#include "app/application.h"

#include "event/input.h"
#include "style/ui_style.h"
#include "ui/container_blur.h"
#include "ui/workspace_model.h"
#include "ui/workspace_widgets.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <string>
#include <string_view>
#include <utility>

#include <windows.h>

#include <fmt/format.h>

#include <SDL3/SDL_properties.h>
#include <SDL3/SDL_video.h>

#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl3.h>
#include <imgui.h>

namespace {

constexpr auto *kWindowsThemeRegistryPath =
    LR"(Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)";
constexpr auto *kWindowsThemeValueName = L"AppsUseLightTheme";
constexpr float kSelectionMarqueeActivationThreshold = 6.0f;
constexpr std::array<const char *, 4> kPerspectiveModes = {
    "Perspective",
    "Plan",
    "Front",
    "Side",
};
constexpr std::array<const char *, 3> kUnits = {
    "Meters",
    "Centimeters",
    "Millimeters",
};
constexpr wchar_t kNativeMenuAppPropName[] = L"WinRtSketchWorld.Application";
constexpr wchar_t kNativeMenuPrevWndProcPropName[] =
    L"WinRtSketchWorld.NativeMenuPrevWndProc";

enum NativeMenuCommand : unsigned int {
  kNativeMenuFileExit = 1001,
  kNativeMenuViewInspector = 1101,
  kNativeMenuViewGrid = 1102,
  kNativeMenuViewAxes = 1103,
  kNativeMenuViewSectionBox = 1104,
  kNativeMenuViewResetCamera = 1105,
  kNativeMenuThemeSystem = 1201,
  kNativeMenuThemeLight = 1202,
  kNativeMenuThemeDark = 1203,
  kNativeMenuHelpSystem = 1301,
  kNativeMenuHelpRefresh = 1302,
};

HWND NativeWindowHandle(SDL_Window *window) {
  if (window == nullptr) {
    return nullptr;
  }

  return static_cast<HWND>(SDL_GetPointerProperty(
      SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WIN32_HWND_POINTER,
      nullptr));
}

void AppendNativeMenuItem(HMENU menu, unsigned int command_id,
                          wchar_t const *label) {
  AppendMenuW(menu, MF_STRING, static_cast<UINT_PTR>(command_id), label);
}

LRESULT CALLBACK NativeMenuWindowProc(HWND hwnd, UINT message, WPARAM wparam,
                                      LPARAM lparam) {
  auto *application =
      static_cast<Application *>(GetPropW(hwnd, kNativeMenuAppPropName));
  auto previous_window_proc = reinterpret_cast<WNDPROC>(
      GetPropW(hwnd, kNativeMenuPrevWndProcPropName));

  if (message == WM_COMMAND && application != nullptr) {
    SDL_Event event{};
    event.type = SDL_EVENT_USER;
    event.user.code = static_cast<Sint32>(LOWORD(wparam));
    event.user.data1 = application;
    SDL_PushEvent(&event);
    return 0;
  }

  if (previous_window_proc != nullptr) {
    return CallWindowProcW(previous_window_proc, hwnd, message, wparam,
                           lparam);
  }

  return DefWindowProcW(hwnd, message, wparam, lparam);
}

char const *PerspectiveIconName(int index) {
  switch (index) {
  case 0:
    return "Cube";
  case 1:
    return "ResponsiveLayoutLibrary";
  case 2:
    return "BoundingBox";
  case 3:
    return "Section";
  default:
    return "Cube";
  }
}

SDL_FRect MakeNormalizedRect(SDL_FPoint const &a, SDL_FPoint const &b) {
  const float min_x = std::min(a.x, b.x);
  const float min_y = std::min(a.y, b.y);
  return SDL_FRect{
      min_x,
      min_y,
      std::abs(a.x - b.x),
      std::abs(a.y - b.y),
  };
}

ImVec4 Mix(ImVec4 const &a, ImVec4 const &b, float t) {
  return ImVec4(
      a.x + ((b.x - a.x) * t), a.y + ((b.y - a.y) * t),
      a.z + ((b.z - a.z) * t), a.w + ((b.w - a.w) * t));
}

bool IsSelectionRectVisible(SDL_FRect const &rect) {
  return rect.w >= kSelectionMarqueeActivationThreshold ||
         rect.h >= kSelectionMarqueeActivationThreshold;
}

bool DetectSystemPrefersDarkTheme() {
  DWORD value = 1;
  DWORD size = sizeof(value);
  const LSTATUS status = RegGetValueW(
      HKEY_CURRENT_USER, kWindowsThemeRegistryPath, kWindowsThemeValueName,
      RRF_RT_REG_DWORD, nullptr, &value, &size);
  return status == ERROR_SUCCESS ? value == 0 : false;
}

std::size_t BrowserSectionIndex(workspace_ui::BrowserSectionId id) {
  switch (id) {
  case workspace_ui::BrowserSectionId::Scene:
    return 0;
  case workspace_ui::BrowserSectionId::FloorPlans:
    return 1;
  case workspace_ui::BrowserSectionId::Views3D:
    return 2;
  case workspace_ui::BrowserSectionId::Elevations:
    return 3;
  case workspace_ui::BrowserSectionId::Sections:
    return 4;
  default:
    return 0;
  }
}

SDL_Rect ToViewportPixels(SDL_FRect const &viewport_rect, ImVec2 display_size,
                          int pixel_width, int pixel_height) {
  const float scale_x = display_size.x > 0.0f
                            ? static_cast<float>(pixel_width) / display_size.x
                            : 1.0f;
  const float scale_y = display_size.y > 0.0f
                            ? static_cast<float>(pixel_height) / display_size.y
                            : 1.0f;
  const int x = static_cast<int>(std::round(viewport_rect.x * scale_x));
  const int y = static_cast<int>(std::round(
      (display_size.y - (viewport_rect.y + viewport_rect.h)) * scale_y));
  const int w = static_cast<int>(std::round(viewport_rect.w * scale_x));
  const int h = static_cast<int>(std::round(viewport_rect.h * scale_y));
  return SDL_Rect{
      std::max(0, x),
      std::max(0, y),
      std::max(0, w),
      std::max(0, h),
  };
}

} // namespace

int Application::Run() {
  if (!Initialize()) {
    SDL_Log("Initialize failed: %s", last_error_.c_str());
    Shutdown();
    return 1;
  }

  while (running_) {
    SDL_Event event{};
    while (SDL_PollEvent(&event)) {
      ProcessEvent(event);
    }

    if (SDL_GetWindowFlags(window_) & SDL_WINDOW_MINIMIZED) {
      SDL_Delay(10);
      continue;
    }

    SyncThemeWithSystem();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    DrawInterface();
    ImGui::Render();

    int pixel_width = 0;
    int pixel_height = 0;
    SDL_GetWindowSizeInPixels(window_, &pixel_width, &pixel_height);
    const SDL_Rect viewport_pixels = ToViewportPixels(
        viewport_rect_, ImGui::GetIO().DisplaySize, pixel_width, pixel_height);
    world_renderer_.Render(gl_, camera_, scene_, world_style_, pixel_width,
                           pixel_height, viewport_pixels, show_grid_,
                           show_axes_, show_guide_cube_);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window_);
  }

  Shutdown();
  return 0;
}

bool Application::Initialize() {
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
    last_error_ = SDL_GetError();
    return false;
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  main_scale_ = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
  const ui_style::UiMetrics ui_metrics = ui_style::BuildUiMetrics(main_scale_);
  menu_bar_height_ = ui_metrics.menu_bar_height;
  command_bar_height_ = ui_metrics.command_bar_height;
  status_bar_height_ = ui_metrics.status_bar_height;
  tray_width_ = ui_metrics.tray_width;

  const SDL_WindowFlags window_flags =
      SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN |
      SDL_WINDOW_HIGH_PIXEL_DENSITY;

  int initial_window_width = static_cast<int>(ui_metrics.initial_window_size.x);
  int initial_window_height =
      static_cast<int>(ui_metrics.initial_window_size.y);
  SDL_Rect usable_bounds{};
  if (SDL_GetDisplayUsableBounds(SDL_GetPrimaryDisplay(), &usable_bounds)) {
    initial_window_width =
        std::min(initial_window_width,
                 usable_bounds.w - static_cast<int>(48.0f * main_scale_));
    initial_window_height =
        std::min(initial_window_height,
                 usable_bounds.h - static_cast<int>(48.0f * main_scale_));
  }

  window_ = SDL_CreateWindow("WinRT Sketch World", initial_window_width,
                             initial_window_height, window_flags);
  if (window_ == nullptr) {
    last_error_ = SDL_GetError();
    return false;
  }

  SDL_SetWindowMinimumSize(
      window_,
      static_cast<int>(std::min(ui_metrics.minimum_window_size.x,
                                static_cast<float>(initial_window_width))),
      static_cast<int>(std::min(ui_metrics.minimum_window_size.y,
                                static_cast<float>(initial_window_height))));

  gl_context_ = SDL_GL_CreateContext(window_);
  if (gl_context_ == nullptr) {
    last_error_ = SDL_GetError();
    return false;
  }

  SDL_GL_MakeCurrent(window_, gl_context_);
  SDL_GL_SetSwapInterval(1);
  SDL_SetWindowPosition(window_, SDL_WINDOWPOS_CENTERED,
                        SDL_WINDOWPOS_CENTERED);
  SDL_ShowWindow(window_);
  InstallNativeMenuBar();

  if (!gl_.Load(last_error_)) {
    return false;
  }

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.IniFilename = nullptr;

  base_path_ = ResolveBasePath();
  ui_style::LoadUiFonts(main_scale_, base_path_ / "res" / "fonts");

  ImGui_ImplSDL3_InitForOpenGL(window_, gl_context_);
  ImGui_ImplOpenGL3_Init("#version 330 core");

  if (!icon_cache_.Initialize(base_path_ / "res" / "icons", last_error_)) {
    return false;
  }
  ResetLayout();
  ApplyThemeMode(theme_mode_, true);
  RefreshSnapshot();
  SetActiveTool(ToolKind::Select, true);
  SetStatusMessage("Ready. Arch workspace shell active. F1 opens Help & System "
                   "Information.");
  running_ = true;
  return true;
}

void Application::Shutdown() {
  DestroyNativeMenuBar();
  world_renderer_.Destroy(gl_);
  icon_cache_.Clear();

  if (ImGui::GetCurrentContext() != nullptr) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
  }

  if (gl_context_ != nullptr) {
    SDL_GL_DestroyContext(gl_context_);
    gl_context_ = nullptr;
  }

  if (window_ != nullptr) {
    SDL_DestroyWindow(window_);
    window_ = nullptr;
  }

  SDL_Quit();
}

void Application::ProcessEvent(SDL_Event const &event) {
  ImGui_ImplSDL3_ProcessEvent(&event);

  switch (event.type) {
  case SDL_EVENT_QUIT:
    running_ = false;
    return;
  case SDL_EVENT_USER:
    if (event.user.data1 == this &&
        HandleNativeMenuCommand(static_cast<unsigned int>(event.user.code))) {
      return;
    }
    break;
  case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
    if (event.window.windowID == SDL_GetWindowID(window_)) {
      running_ = false;
      return;
    }
    break;
  case SDL_EVENT_WINDOW_FOCUS_LOST:
  case SDL_EVENT_WINDOW_MOUSE_LEAVE:
    camera_.CancelInteractions();
    CancelSelectionMarquee();
    break;
  case SDL_EVENT_KEY_DOWN:
    if (!event.key.repeat) {
      HandleKeyboardShortcut(event.key);
    }
    break;
  default:
    break;
  }

  const bool selection_consumed = HandleSelectionMarqueeEvent(event);
  const bool allow_camera_input =
      !selection_consumed &&
      (camera_.IsInteracting() || !ImGui::GetIO().WantCaptureMouse);
  if (allow_camera_input) {
    SDL_Event camera_event = event;
    if (active_tool_ == ToolKind::Orbit) {
      if (camera_event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
          camera_event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
        if (camera_event.button.button == SDL_BUTTON_LEFT) {
          camera_event.button.button = SDL_BUTTON_RIGHT;
        }
      }
    } else if (active_tool_ == ToolKind::Pan) {
      if (camera_event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
          camera_event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
        if (camera_event.button.button == SDL_BUTTON_LEFT) {
          camera_event.button.button = SDL_BUTTON_MIDDLE;
        }
      }
    }
    camera_.HandleEvent(camera_event);
  }
}

void Application::HandleKeyboardShortcut(SDL_KeyboardEvent const &event) {
  if (event.key == InputConstants::kOpenHelp) {
    show_help_modal_ = true;
    SetStatusMessage("Help & System Information opened.");
    return;
  }

  if (event.key == InputConstants::kRefreshSystemInfo) {
    RefreshSnapshot();
    return;
  }

  if (event.key == InputConstants::kToggleInspector) {
    inspector_visible_ = !inspector_visible_;
    SetStatusMessage(inspector_visible_ ? "Default Tray shown."
                                        : "Default Tray hidden.");
    return;
  }

  if (event.key == InputConstants::kResetCamera) {
    camera_.Reset();
    perspective_index_ = 0;
    SetStatusMessage("Camera reset to home view.");
    return;
  }

  if (event.key == InputConstants::kFrontView) {
    camera_.SetFrontView();
    perspective_index_ = 2;
    SetStatusMessage("Front view activated.");
    return;
  }

  if (event.key == InputConstants::kRightView) {
    camera_.SetRightView();
    perspective_index_ = 3;
    SetStatusMessage("Right view activated.");
    return;
  }

  if (event.key == InputConstants::kIsometricView) {
    camera_.SetIsometricView();
    perspective_index_ = 0;
    SetStatusMessage("Isometric view activated.");
    return;
  }

  if (event.key == InputConstants::kTopView) {
    camera_.SetTopView();
    perspective_index_ = 1;
    SetStatusMessage("Top view activated.");
    return;
  }

  if (InputConstants::HasModifier(event.mod,
                                  InputConstants::kSceneToggleModifier)) {
    if (event.key == InputConstants::kToggleGrid) {
      show_grid_ = !show_grid_;
      SetStatusMessage(show_grid_ ? "Ground grid enabled."
                                  : "Ground grid hidden.");
      return;
    }
    if (event.key == InputConstants::kToggleAxes) {
      show_axes_ = !show_axes_;
      SetStatusMessage(show_axes_ ? "World axes enabled."
                                  : "World axes hidden.");
      return;
    }
    if (event.key == InputConstants::kToggleGuide) {
      show_guide_cube_ = !show_guide_cube_;
      SetStatusMessage(show_guide_cube_ ? "Guide cube enabled."
                                        : "Guide cube hidden.");
      return;
    }
  }
}

void Application::DrawInterface() {
  UpdateNativeMenuBar();
  DrawMenuBar();
  UpdateViewportRect();
  DrawCommandBar();
  DrawInspector();
  DrawStatusBar();
  DrawHelpModal();
  DrawSelectionMarquee();
}

void Application::DrawMenuBar() {
  const ui_style::UiMetrics ui_metrics = ui_style::BuildUiMetrics(main_scale_);
  ImGuiIO &io = ImGui::GetIO();
  ImGuiStyle const &style = ImGui::GetStyle();
  menu_bar_height_ = 0.0f;
  const workspace_ui::SelectionSnapshot selection_snapshot{
      .valid = last_marquee_valid_,
      .crossing = last_marquee_crossing_,
      .rect = last_marquee_rect_,
  };
  const workspace_ui::BrowserModel browser_model =
      workspace_ui::BuildBrowserModel(active_workspace_index_, active_tool_,
                                      status_message_, selection_snapshot,
                                      snapshot_, scene_.stats, show_grid_,
                                      show_axes_, show_guide_cube_);

  const float margin = ui_metrics.chrome_margin;
  const float gap = ui_metrics.chrome_gap;
  const float chrome_height = io.DisplaySize.y - (margin * 2.0f);
  const ImGuiWindowFlags panel_flags = ImGuiWindowFlags_NoDecoration |
                                       ImGuiWindowFlags_NoMove |
                                       ImGuiWindowFlags_NoResize |
                                       ImGuiWindowFlags_NoSavedSettings;
  const ImVec4 active_text = style.Colors[ImGuiCol_Text];
  const ImVec4 muted_text = style.Colors[ImGuiCol_TextDisabled];
  const float footer_offset = 30.0f * main_scale_;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, ui_metrics.panel_rounding);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                      ImVec2(8.0f * main_scale_, 12.0f * main_scale_));
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                      ImVec2(4.0f * main_scale_, 8.0f * main_scale_));
  workspace_ui::PushBlurContainerWindowStyle();
  ImGui::SetNextWindowPos(ImVec2(margin, margin), ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(ui_metrics.nav_rail_width, chrome_height),
                           ImGuiCond_Always);
  ImGui::Begin("##layout-rail", nullptr, panel_flags);
  workspace_ui::DrawBlurContainerBackdrop(workspace_ui::BlurContainerKind::Rail,
                                          resolved_dark_theme_, main_scale_,
                                          ui_metrics.panel_rounding);

  ImGui::SetCursorPosY(8.0f * main_scale_);
  for (std::size_t index = 0; index < browser_model.rail_items.size();
       ++index) {
    auto const &item = browser_model.rail_items[index];
    const std::string button_id = fmt::format("##rail-{}", index);
    if (workspace_ui::DrawIconRailButton(
            icon_cache_, button_id.c_str(), item.icon_name.c_str(),
            item.label.c_str(), item.active, ui_metrics.rail_button_size,
            main_scale_)) {
      active_workspace_index_ = static_cast<int>(index);
      SetStatusMessage(fmt::format("{} workspace selected.", item.label));
    }
    ImGui::Dummy(ImVec2(0.0f, 4.0f * main_scale_));
  }

  ImGui::SetCursorPosY(std::max(
      ImGui::GetCursorPosY(),
      ImGui::GetWindowHeight() - ui_metrics.rail_button_size.y -
          (12.0f * main_scale_)));
  if (workspace_ui::DrawIconGhostButton(
          icon_cache_, "##workspace-settings", "Settings",
          "Open workspace settings",
          ui_metrics.rail_button_size, main_scale_)) {
    inspector_visible_ = true;
    inspector_tab_ = 1;
    SetStatusMessage("Workspace settings opened.");
  }

  ImGui::End();
  workspace_ui::PopBlurContainerWindowStyle();
  ImGui::PopStyleVar(3);

  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, ui_metrics.panel_rounding);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                      ImVec2(18.0f * main_scale_, 18.0f * main_scale_));
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                      ImVec2(0.0f, 8.0f * main_scale_));
  workspace_ui::PushBlurContainerWindowStyle();
  ImGui::SetNextWindowPos(
      ImVec2(margin + ui_metrics.nav_rail_width + gap, margin),
      ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(ui_metrics.left_panel_width, chrome_height),
                           ImGuiCond_Always);
  ImGui::Begin("##layout-scene-browser", nullptr, panel_flags);
  workspace_ui::DrawBlurContainerBackdrop(
      workspace_ui::BlurContainerKind::Sidebar, resolved_dark_theme_,
      main_scale_, ui_metrics.panel_rounding);

  workspace_ui::PushFontIfAvailable(ui_style::HeadingFont());
  ImGui::TextUnformatted(browser_model.project_name.c_str());
  workspace_ui::PopFontIfAvailable(ui_style::HeadingFont());
  ImGui::SameLine();
  ImGui::SetCursorPosX(ImGui::GetWindowWidth() - (46.0f * main_scale_));
  if (workspace_ui::DrawIconGhostButton(
          icon_cache_, "##sidebar-menu", "Menu", "Workspace menu",
          ImVec2(28.0f * main_scale_, 28.0f * main_scale_), main_scale_)) {
    SetStatusMessage("Workspace menu is not wired yet.");
  }
  ImGui::Dummy(ImVec2(0.0f, 8.0f * main_scale_));

  if (workspace_ui::DrawIconGhostButton(
          icon_cache_, "##sidebar-search", "Search", "Search views",
          ImVec2(20.0f * main_scale_, 20.0f * main_scale_), main_scale_)) {
    SetStatusMessage("Search is not wired yet.");
  }
  ImGui::SameLine();
  ImGui::SetCursorPosY(ImGui::GetCursorPosY() - (1.0f * main_scale_));
  ImGui::TextColored(active_text, "Views");
  ImGui::SameLine();
  ImGui::TextColored(muted_text, "Level");
  ImGui::Dummy(ImVec2(0.0f, 12.0f * main_scale_));

  for (auto const &section : browser_model.sections) {
    const std::size_t section_index = BrowserSectionIndex(section.id);
    ImGui::PushID(static_cast<int>(section_index));
    if (workspace_ui::DrawBrowserSectionHeader(
            icon_cache_, section, browser_sections_expanded_[section_index],
            main_scale_)) {
      browser_sections_expanded_[section_index] =
          !browser_sections_expanded_[section_index];
    }

    if (!browser_sections_expanded_[section_index]) {
      ImGui::Dummy(ImVec2(0.0f, 4.0f * main_scale_));
      ImGui::PopID();
      continue;
    }

    ImGui::Indent(14.0f * main_scale_);
    for (std::size_t item_index = 0; item_index < section.items.size();
         ++item_index) {
      ImGui::PushID(static_cast<int>(item_index));
      auto const &item = section.items[item_index];
      if (workspace_ui::DrawBrowserItemRow(icon_cache_, item, main_scale_)) {
        switch (section.id) {
        case workspace_ui::BrowserSectionId::Scene:
          if (item.icon_name == "Line") {
            SetActiveTool(ToolKind::Line);
          } else {
            SetActiveTool(ToolKind::Roof);
          }
          SetStatusMessage(fmt::format("{} primitive selected.", item.label));
          break;
        case workspace_ui::BrowserSectionId::FloorPlans:
          camera_.SetTopView();
          perspective_index_ = 1;
          active_workspace_index_ = 2;
          SetStatusMessage(fmt::format("{} activated.", item.label));
          break;
        case workspace_ui::BrowserSectionId::Views3D:
          camera_.SetIsometricView();
          perspective_index_ = 0;
          active_workspace_index_ = 3;
          SetStatusMessage(fmt::format("{} activated.", item.label));
          break;
        case workspace_ui::BrowserSectionId::Elevations:
          camera_.SetFrontView();
          perspective_index_ = 2;
          SetStatusMessage("Building elevation activated.");
          break;
        case workspace_ui::BrowserSectionId::Sections:
          if (item.label == "No sections yet") {
            SetStatusMessage("No sections available yet.");
          } else {
            show_guide_cube_ = true;
            SetActiveTool(ToolKind::Section, true);
            SetStatusMessage(fmt::format("{} review opened.", item.label));
          }
          break;
        default:
          SetStatusMessage(fmt::format("{} selected.", item.label));
          break;
        }
      }
      ImGui::PopID();
    }
    ImGui::Unindent(14.0f * main_scale_);
    ImGui::Dummy(ImVec2(0.0f, 8.0f * main_scale_));
    ImGui::PopID();
  }

  ImGui::SetCursorPosY(std::max(
      ImGui::GetCursorPosY(),
      ImGui::GetWindowHeight() - footer_offset));
  ImGui::TextDisabled("Project ID: %s", browser_model.project_id.c_str());
  ImGui::SameLine();
  ImGui::SetCursorPosX(ImGui::GetWindowWidth() - (54.0f * main_scale_));
  ImGui::TextDisabled("Views");

  ImGui::End();
  workspace_ui::PopBlurContainerWindowStyle();
  ImGui::PopStyleVar(3);
}

void Application::DrawCommandBar() {
  const ui_style::UiMetrics ui_metrics = ui_style::BuildUiMetrics(main_scale_);
  ImGuiStyle const &style = ImGui::GetStyle();
  command_bar_height_ = 0.0f;
  if (viewport_rect_.w <= 0.0f || viewport_rect_.h <= 0.0f) {
    return;
  }

  const SDL_FRect chrome_rect = WorkspaceChromeRect();
  if (chrome_rect.w <= 0.0f || chrome_rect.h <= 0.0f) {
    return;
  }

  const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
                                 ImGuiWindowFlags_NoMove |
                                 ImGuiWindowFlags_NoSavedSettings;
  const float item_spacing = 8.0f * main_scale_;
  const float horizontal_padding = 12.0f * main_scale_;
  const float toolbar_width =
      (horizontal_padding * 2.0f) +
      (ui_metrics.top_toolbar_button_size.x * 9.0f) + (item_spacing * 8.0f);

  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, ui_metrics.panel_rounding);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                      ImVec2(horizontal_padding, 5.0f * main_scale_));
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                      ImVec2(item_spacing, 0.0f));
  workspace_ui::PushBlurContainerWindowStyle();
  float toolbar_x = chrome_rect.x;
  if (chrome_rect.w > toolbar_width) {
    toolbar_x += (chrome_rect.w - toolbar_width) * 0.5f;
  }
  ImGui::SetNextWindowPos(
      ImVec2(toolbar_x, chrome_rect.y + (4.0f * main_scale_)),
      ImGuiCond_Always);
  ImGui::SetNextWindowSize(
      ImVec2(toolbar_width, ui_metrics.top_toolbar_height),
      ImGuiCond_Always);
  ImGui::Begin("##floating-top-toolbar", nullptr, flags);
  workspace_ui::DrawBlurContainerBackdrop(
      workspace_ui::BlurContainerKind::Toolbar, resolved_dark_theme_,
      main_scale_, ui_metrics.panel_rounding);

  if (workspace_ui::DrawToolbarIconButton(
          icon_cache_, "##toolbar-undo", "Undo", "Undo", false,
          ui_metrics.top_toolbar_button_size, main_scale_)) {
    SetStatusMessage("Undo stack is not wired yet.");
  }
  ImGui::SameLine();
  if (workspace_ui::DrawToolbarIconButton(
          icon_cache_, "##toolbar-redo", "Redo", "Redo", false,
          ui_metrics.top_toolbar_button_size, main_scale_)) {
    SetStatusMessage("Redo stack is not wired yet.");
  }
  ImGui::SameLine();
  if (workspace_ui::DrawToolbarIconButton(
          icon_cache_, "##toolbar-view", "eyeglasses", "View",
          perspective_index_ == 0,
          ui_metrics.top_toolbar_button_size,
          main_scale_)) {
    camera_.SetIsometricView();
    perspective_index_ = 0;
    SetStatusMessage("Perspective workspace view activated.");
  }
  ImGui::SameLine();
  if (workspace_ui::DrawToolbarIconButton(
          icon_cache_, "##toolbar-plan", "BackHand", "Plan",
          perspective_index_ == 1,
          ui_metrics.top_toolbar_button_size,
          main_scale_)) {
    camera_.SetTopView();
    perspective_index_ = 1;
    SetStatusMessage("Plan workspace view activated.");
  }
  ImGui::SameLine();
  if (workspace_ui::DrawToolbarIconButton(
          icon_cache_, "##toolbar-orbit", ToolIconName(ToolKind::Orbit),
          "Orbit camera", active_tool_ == ToolKind::Orbit,
          ui_metrics.top_toolbar_button_size, main_scale_)) {
    SetActiveTool(ToolKind::Orbit);
  }
  ImGui::SameLine();
  if (workspace_ui::DrawToolbarIconButton(
          icon_cache_, "##toolbar-grid", "Grid", "Toggle grid", show_grid_,
          ui_metrics.top_toolbar_button_size, main_scale_)) {
    show_grid_ = !show_grid_;
    SetStatusMessage(show_grid_ ? "Ground grid enabled."
                                : "Ground grid hidden.");
  }
  ImGui::SameLine();
  if (workspace_ui::DrawToolbarIconButton(
          icon_cache_, "##toolbar-section", ToolIconName(ToolKind::Section),
          "Toggle section box", show_guide_cube_,
          ui_metrics.top_toolbar_button_size, main_scale_)) {
    show_guide_cube_ = !show_guide_cube_;
    if (show_guide_cube_) {
      SetActiveTool(ToolKind::Section, true);
    }
    SetStatusMessage(show_guide_cube_ ? "Section box enabled."
                                      : "Section box hidden.");
  }
  ImGui::SameLine();
  if (workspace_ui::DrawToolbarIconButton(
          icon_cache_, "##toolbar-move", ToolIconName(ToolKind::Move),
          "Move tool", active_tool_ == ToolKind::Move,
          ui_metrics.top_toolbar_button_size, main_scale_)) {
    SetActiveTool(ToolKind::Move);
  }
  ImGui::SameLine();
  if (workspace_ui::DrawToolbarIconButton(
          icon_cache_, "##toolbar-copy", ToolIconName(ToolKind::Copy),
          "Copy tool", active_tool_ == ToolKind::Copy,
          ui_metrics.top_toolbar_button_size, main_scale_)) {
    SetActiveTool(ToolKind::Copy);
  }

  ImGui::End();
  workspace_ui::PopBlurContainerWindowStyle();
  ImGui::PopStyleVar(3);

  ImDrawList *draw_list = ImGui::GetForegroundDrawList();
  const ImVec4 handle_fill = resolved_dark_theme_
                                 ? ImVec4(0.19f, 0.25f, 0.31f, 0.88f)
                                 : ImVec4(0.88f, 0.91f, 0.95f, 0.92f);
  const ImVec4 handle_stroke =
      resolved_dark_theme_
          ? Mix(style.Colors[ImGuiCol_TextDisabled], style.Colors[ImGuiCol_Text], 0.28f)
          : Mix(style.Colors[ImGuiCol_TextDisabled], style.Colors[ImGuiCol_Text], 0.45f);
  const ImVec2 handle_min(toolbar_x + ((toolbar_width - (28.0f * main_scale_)) * 0.5f),
                          chrome_rect.y + ui_metrics.top_toolbar_height +
                              (4.0f * main_scale_));
  const ImVec2 handle_max(handle_min.x + (28.0f * main_scale_),
                          handle_min.y + (16.0f * main_scale_));
  draw_list->AddRectFilled(handle_min, handle_max,
                           ImGui::ColorConvertFloat4ToU32(handle_fill),
                           7.0f * main_scale_);
  const ImVec2 center((handle_min.x + handle_max.x) * 0.5f,
                      handle_min.y + (8.0f * main_scale_));
  draw_list->AddLine(ImVec2(center.x - (4.0f * main_scale_),
                            center.y - (1.0f * main_scale_)),
                     ImVec2(center.x, center.y + (3.0f * main_scale_)),
                     ImGui::ColorConvertFloat4ToU32(handle_stroke),
                     std::max(1.0f, 1.1f * main_scale_));
  draw_list->AddLine(ImVec2(center.x, center.y + (3.0f * main_scale_)),
                     ImVec2(center.x + (4.0f * main_scale_),
                            center.y - (1.0f * main_scale_)),
                     ImGui::ColorConvertFloat4ToU32(handle_stroke),
                     std::max(1.0f, 1.1f * main_scale_));
}

void Application::DrawInspector() {
  if (!inspector_visible_) {
    return;
  }

  const ui_style::UiMetrics ui_metrics = ui_style::BuildUiMetrics(main_scale_);
  ImGuiStyle const &style = ImGui::GetStyle();
  const ImVec4 accent_color = style.Colors[ImGuiCol_ButtonActive];
  const workspace_ui::InspectorModel inspector_model =
      workspace_ui::BuildInspectorModel(
          active_tool_, theme_mode_, camera_, viewport_rect_, snapshot_,
          scene_.stats, world_style_, show_grid_, show_axes_, show_guide_cube_);

  ImGuiIO &io = ImGui::GetIO();
  const float top = ui_metrics.chrome_margin;
  const float height = io.DisplaySize.y - (ui_metrics.chrome_margin * 2.0f);
  const float tray_width = ResolvedTrayWidth();

  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, ui_metrics.panel_rounding);
  workspace_ui::PushBlurContainerWindowStyle();
  ImGui::SetNextWindowPos(
      ImVec2(io.DisplaySize.x - tray_width - ui_metrics.chrome_margin, top),
      ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(tray_width, height), ImGuiCond_Always);

  const ImGuiWindowFlags flags =
      ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

  ImGui::Begin("##layout-right-tray", nullptr, flags);
  workspace_ui::DrawBlurContainerBackdrop(
      workspace_ui::BlurContainerKind::Inspector, resolved_dark_theme_,
      main_scale_, ui_metrics.panel_rounding);

  workspace_ui::DrawToolbarIconButton(
      icon_cache_, "##inspector-badge", "Cube", "Workspace inspector", true,
      ui_metrics.right_header_button_size, main_scale_);
  ImGui::SameLine();
  ImGui::AlignTextToFramePadding();
  workspace_ui::PushFontIfAvailable(ui_style::HeadingFont());
  ImGui::TextUnformatted("Viewport");
  workspace_ui::PopFontIfAvailable(ui_style::HeadingFont());
  ImGui::SameLine();
  ImGui::SetCursorPosX(ImGui::GetWindowWidth() - (210.0f * main_scale_));
  ImGui::SetNextItemWidth(128.0f * main_scale_);
  if (ImGui::BeginCombo("##perspective",
                        kPerspectiveModes[perspective_index_])) {
    for (int index = 0; index < static_cast<int>(kPerspectiveModes.size());
         ++index) {
      if (ImGui::Selectable(kPerspectiveModes[index],
                            perspective_index_ == index)) {
        perspective_index_ = index;
        if (index == 0) {
          camera_.SetIsometricView();
        } else if (index == 1) {
          camera_.SetTopView();
        } else if (index == 2) {
          camera_.SetFrontView();
        } else {
          camera_.SetRightView();
        }
        SetStatusMessage(
            fmt::format("{} view activated.", kPerspectiveModes[index]));
      }
    }
    ImGui::EndCombo();
  }
  ImGui::SameLine();
  if (workspace_ui::DrawIconGhostButton(
          icon_cache_, "##inspector-settings", "Settings",
          "Open workspace settings",
          ui_metrics.right_header_button_size, main_scale_)) {
    show_help_modal_ = true;
    SetStatusMessage("Workspace settings opened.");
  }
  ImGui::SameLine();
  if (workspace_ui::DrawGhostButton("X", ui_metrics.right_header_button_size)) {
    inspector_visible_ = false;
  }

  workspace_ui::PushFontIfAvailable(ui_style::CaptionFont());
  ImGui::SeparatorText("Workspace");
  ImGui::TextUnformatted("Units");
  ImGui::SameLine();
  ImGui::SetCursorPosX(ImGui::GetWindowWidth() - (138.0f * main_scale_));
  ImGui::SetNextItemWidth(120.0f * main_scale_);
  if (ImGui::BeginCombo("##units", kUnits[units_index_])) {
    for (int index = 0; index < static_cast<int>(kUnits.size()); ++index) {
      if (ImGui::Selectable(kUnits[index], units_index_ == index)) {
        units_index_ = index;
      }
    }
    ImGui::EndCombo();
  }

  ImGui::TextUnformatted("Tolerance");
  ImGui::SameLine();
  ImGui::SetCursorPosX(ImGui::GetWindowWidth() - (100.0f * main_scale_));
  ImGui::SetNextItemWidth(82.0f * main_scale_);
  ImGui::DragFloat("##tolerance", &tolerance_value_, 0.01f, 0.01f, 5.0f,
                   "%.2f");

  ImGui::TextUnformatted("Theme");
  ImGui::SameLine();
  ImGui::SetCursorPosX(ImGui::GetWindowWidth() - (138.0f * main_scale_));
  ImGui::SetNextItemWidth(120.0f * main_scale_);
  if (ImGui::BeginCombo("##tray-theme", ThemeModeLabel(theme_mode_))) {
    if (ImGui::Selectable("System", theme_mode_ == ThemeMode::System)) {
      ApplyThemeMode(ThemeMode::System);
    }
    if (ImGui::Selectable("Light", theme_mode_ == ThemeMode::Light)) {
      ApplyThemeMode(ThemeMode::Light);
    }
    if (ImGui::Selectable("Dark", theme_mode_ == ThemeMode::Dark)) {
      ApplyThemeMode(ThemeMode::Dark);
    }
    ImGui::EndCombo();
  }

  const auto draw_toggle_row = [&](const char *label, bool *value,
                                   const char *id) {
    ImGui::TextUnformatted(label);
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() -
                         ui_metrics.inspector_toggle_size.x -
                         (18.0f * main_scale_));
    if (workspace_ui::DrawTogglePill(id, value,
                                     ui_metrics.inspector_toggle_size)) {
      SetStatusMessage(
          fmt::format("{} {}.", label, *value ? "enabled" : "disabled"));
    }
  };

  draw_toggle_row("Dimension Snap", &dimension_snap_, "##dimension-snap");
  draw_toggle_row("Angle Snap", &angle_snap_, "##angle-snap");
  draw_toggle_row("Parallel Snap", &parallel_snap_, "##parallel-snap");
  draw_toggle_row("Perpendicular Snap", &perpendicular_snap_,
                  "##perpendicular-snap");

  ImGui::SeparatorText("Panels");
  workspace_ui::PopFontIfAvailable(ui_style::CaptionFont());

  if (workspace_ui::DrawToolbarChipButton(icon_cache_, "##inspector-tab-tools",
                                          "Cube", "Tools", inspector_tab_ == 0,
                                          ui_metrics.inspector_tab_button_size,
                                          main_scale_)) {
    inspector_tab_ = 0;
  }
  ImGui::SameLine();
  if (workspace_ui::DrawToolbarChipButton(
          icon_cache_, "##inspector-tab-camera", ToolIconName(ToolKind::Orbit),
          "Camera", inspector_tab_ == 1, ui_metrics.inspector_tab_button_size,
          main_scale_)) {
    inspector_tab_ = 1;
  }
  ImGui::SameLine();
  if (workspace_ui::DrawToolbarChipButton(
          icon_cache_, "##inspector-tab-mats", ToolIconName(ToolKind::Paint),
          "Mats", inspector_tab_ == 2, ui_metrics.inspector_tab_button_size,
          main_scale_)) {
    inspector_tab_ = 2;
  }
  ImGui::SameLine();
  if (workspace_ui::DrawToolbarChipButton(
          icon_cache_, "##inspector-tab-project", "FolderProject", "Project",
          inspector_tab_ == 3, ui_metrics.inspector_tab_button_size,
          main_scale_)) {
    inspector_tab_ = 3;
  }

  const auto apply_camera_preset = [&](char const *label) {
    if (std::string_view(label) == "ISO") {
      camera_.SetIsometricView();
      perspective_index_ = 0;
    } else if (std::string_view(label) == "TOP") {
      camera_.SetTopView();
      perspective_index_ = 1;
    } else if (std::string_view(label) == "FRNT") {
      camera_.SetFrontView();
      perspective_index_ = 2;
    } else if (std::string_view(label) == "BACK") {
      camera_.SetBackView();
      perspective_index_ = 2;
    } else if (std::string_view(label) == "LEFT") {
      camera_.SetLeftView();
      perspective_index_ = 3;
    } else if (std::string_view(label) == "RGHT") {
      camera_.SetRightView();
      perspective_index_ = 3;
    }
    SetStatusMessage(fmt::format("{} camera preset activated.", label));
  };

  if (inspector_tab_ == 0) {
    for (std::size_t index = 0;
         index < inspector_model.architecture_tools.size(); ++index) {
      auto const &tool = inspector_model.architecture_tools[index];
      const std::string tile_id = fmt::format("##arch-tool-{}", index);
      if (workspace_ui::DrawIconActionTile(
              icon_cache_, tile_id.c_str(), tool.icon_name.c_str(),
              tool.label.c_str(), tool.caption.c_str(),
              active_tool_ == tool.tool, ui_metrics.right_action_tile_size,
              main_scale_)) {
        SetActiveTool(tool.tool);
      }
      if (index % 2 == 0 &&
          index + 1 < inspector_model.architecture_tools.size()) {
        ImGui::SameLine();
      }
    }
  } else if (inspector_tab_ == 1) {
    const auto preset_icon_name = [](char const *label) {
      if (std::string_view(label) == "ISO") {
        return PerspectiveIconName(0);
      }
      if (std::string_view(label) == "TOP") {
        return PerspectiveIconName(1);
      }
      if (std::string_view(label) == "FRNT" ||
          std::string_view(label) == "BACK") {
        return PerspectiveIconName(2);
      }
      return PerspectiveIconName(3);
    };
    const std::array<const char *, 6> preset_labels = {"ISO",  "TOP",  "FRNT",
                                                       "BACK", "LEFT", "RGHT"};
    for (std::size_t index = 0; index < preset_labels.size(); ++index) {
      const std::string preset_id = fmt::format("##camera-preset-{}", index);
      if (workspace_ui::DrawToolbarChipButton(
              icon_cache_, preset_id.c_str(),
              preset_icon_name(preset_labels[index]), preset_labels[index],
              false, ui_metrics.camera_preset_button_size, main_scale_)) {
        apply_camera_preset(preset_labels[index]);
      }
      if (index % 3 != 2 && index + 1 < preset_labels.size()) {
        ImGui::SameLine();
      }
    }
    ImGui::SeparatorText("Navigation");
    for (auto const &row : inspector_model.navigation_rows) {
      if (row.accent) {
        ImGui::TextColored(accent_color, "%s",
                           row.label.c_str());
      } else {
        ImGui::TextDisabled("%s", row.label.c_str());
      }
      ImGui::SameLine();
      ImGui::SetCursorPosX(ImGui::GetWindowWidth() - (132.0f * main_scale_));
      ImGui::TextWrapped("%s", row.value.c_str());
    }
    ImGui::SeparatorText("Camera");
    for (auto const &row : inspector_model.camera_rows) {
      if (row.accent) {
        ImGui::TextColored(accent_color, "%s",
                           row.label.c_str());
      } else {
        ImGui::TextDisabled("%s", row.label.c_str());
      }
      ImGui::SameLine();
      ImGui::SetCursorPosX(ImGui::GetWindowWidth() - (132.0f * main_scale_));
      ImGui::TextWrapped("%s", row.value.c_str());
    }
  } else if (inspector_tab_ == 2) {
    ImGui::SeparatorText("Material Palette");
    for (std::size_t index = 0; index < inspector_model.swatches.size();
         ++index) {
      auto const &swatch = inspector_model.swatches[index];
      ImGui::ColorButton(
          fmt::format("##swatch-{}", index).c_str(),
          ImVec4(swatch.color[0], swatch.color[1], swatch.color[2], 1.0f),
          ImGuiColorEditFlags_NoTooltip, ui_metrics.material_swatch_size);
      ImGui::SameLine();
      ImGui::Text("%s", swatch.label.c_str());
    }
    ImGui::SeparatorText("Hint");
    ImGui::TextWrapped("%s", ToolHintText(active_tool_));
  } else {
    ImGui::SeparatorText("Project");
    for (auto const &row : inspector_model.project_rows) {
      if (row.accent) {
        ImGui::TextColored(accent_color, "%s",
                           row.label.c_str());
      } else {
        ImGui::TextDisabled("%s", row.label.c_str());
      }
      ImGui::SameLine();
      ImGui::SetCursorPosX(ImGui::GetWindowWidth() - (148.0f * main_scale_));
      ImGui::TextWrapped("%s", row.value.c_str());
    }
  }

  ImGui::SeparatorText("Selection");
  if (marquee_active_ && marquee_visible_) {
    const SDL_FRect rect = CurrentSelectionRect();
    ImGui::Text("%s marquee",
                marquee_current_.x < marquee_origin_.x ? "Crossing" : "Window");
    ImGui::Text("Rect %.0f x %.0f", rect.w, rect.h);
  } else if (last_marquee_valid_) {
    ImGui::Text("%s marquee", last_marquee_crossing_ ? "Crossing" : "Window");
    ImGui::Text("Last %.0f x %.0f", last_marquee_rect_.w, last_marquee_rect_.h);
  } else {
    ImGui::TextDisabled("No selection active");
  }

  ImGui::SeparatorText("System");
  ImGui::Text("Theme mode  %s", ThemeModeLabel(theme_mode_));
  ImGui::Text("Viewport    %.0f x %.0f", viewport_rect_.w, viewport_rect_.h);
  if (!last_error_.empty()) {
    ImGui::Separator();
    ImGui::TextDisabled("Last error");
    ImGui::TextWrapped("%s", last_error_.c_str());
  }

  ImGui::End();
  workspace_ui::PopBlurContainerWindowStyle();
  ImGui::PopStyleVar();
}

void Application::DrawStatusBar() {
  const ui_style::UiMetrics ui_metrics = ui_style::BuildUiMetrics(main_scale_);
  status_bar_height_ = 0.0f;
  if (viewport_rect_.w <= 0.0f || viewport_rect_.h <= 0.0f) {
    return;
  }
  const SDL_FRect chrome_rect = WorkspaceChromeRect();
  if (chrome_rect.w <= 0.0f || chrome_rect.h <= 0.0f) {
    return;
  }

  const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
                                 ImGuiWindowFlags_NoMove |
                                 ImGuiWindowFlags_NoSavedSettings;
  const std::array<ToolKind, 9> bottom_tools = {
      ToolKind::Select, ToolKind::Line,  ToolKind::Arc,
      ToolKind::Window, ToolKind::Roof,  ToolKind::Move,
      ToolKind::Section, ToolKind::Paint, ToolKind::Copy,
  };
  const float item_spacing = 8.0f * main_scale_;
  const float horizontal_padding = 12.0f * main_scale_;
  const float dock_width =
      (horizontal_padding * 2.0f) +
      (ui_metrics.dock_button_size.x * static_cast<float>(bottom_tools.size())) +
      (item_spacing * static_cast<float>(bottom_tools.size() - 1));
  const float dock_x =
      chrome_rect.x + std::max(0.0f, (chrome_rect.w - dock_width) * 0.5f);
  const float dock_y = chrome_rect.y + chrome_rect.h -
                       ui_metrics.bottom_dock_height - (4.0f * main_scale_);

  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, ui_metrics.panel_rounding);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                      ImVec2(horizontal_padding, 5.0f * main_scale_));
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                      ImVec2(item_spacing, 0.0f));
  workspace_ui::PushBlurContainerWindowStyle();

  ImGui::SetNextWindowPos(ImVec2(dock_x, dock_y), ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(dock_width, ui_metrics.bottom_dock_height),
                           ImGuiCond_Always);
  ImGui::Begin("##bottom-tool-dock", nullptr, flags);
  workspace_ui::DrawBlurContainerBackdrop(workspace_ui::BlurContainerKind::Dock,
                                          resolved_dark_theme_, main_scale_,
                                          ui_metrics.panel_rounding);

  for (std::size_t index = 0; index < bottom_tools.size(); ++index) {
    const ToolKind tool = bottom_tools[index];
    const std::string button_id = fmt::format("##dock-tool-{}", index);
    if (workspace_ui::DrawDockIconButton(
            icon_cache_, button_id.c_str(), ToolIconName(tool), ToolLabel(tool),
            active_tool_ == tool, ui_metrics.dock_button_size, main_scale_)) {
      SetActiveTool(tool);
    }
    if (index + 1 != bottom_tools.size()) {
      ImGui::SameLine();
    }
  }

  ImGui::End();
  workspace_ui::PopBlurContainerWindowStyle();
  ImGui::PopStyleVar(3);

  const float settings_width = ui_metrics.dock_button_size.x +
                               (18.0f * main_scale_);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, ui_metrics.panel_rounding);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                      ImVec2(9.0f * main_scale_, 5.0f * main_scale_));
  workspace_ui::PushBlurContainerWindowStyle();
  ImGui::SetNextWindowPos(
      ImVec2(dock_x + dock_width + (12.0f * main_scale_), dock_y),
      ImGuiCond_Always);
  ImGui::SetNextWindowSize(
      ImVec2(settings_width, ui_metrics.bottom_dock_height), ImGuiCond_Always);
  ImGui::Begin("##bottom-settings-bubble", nullptr, flags);
  workspace_ui::DrawBlurContainerBackdrop(
      workspace_ui::BlurContainerKind::Bubble, resolved_dark_theme_,
      main_scale_, ui_metrics.panel_rounding);
  if (workspace_ui::DrawToolbarIconButton(
          icon_cache_, "##dock-settings", "Settings",
          inspector_visible_ ? "Hide settings panel" : "Show settings panel",
          inspector_visible_, ui_metrics.dock_button_size, main_scale_)) {
    inspector_visible_ = !inspector_visible_;
    SetStatusMessage(inspector_visible_ ? "Settings panel shown."
                                        : "Settings panel hidden.");
  }
  ImGui::End();
  workspace_ui::PopBlurContainerWindowStyle();
  ImGui::PopStyleVar(2);
}

void Application::DrawHelpModal() {
  if (show_help_modal_) {
    ImGui::OpenPopup("Help & System Information");
    show_help_modal_ = false;
  }

  const ui_style::UiMetrics ui_metrics = ui_style::BuildUiMetrics(main_scale_);
  ImGui::SetNextWindowSize(ui_metrics.help_modal_size, ImGuiCond_Appearing);
  if (!ImGui::BeginPopupModal("Help & System Information", nullptr,
                              ImGuiWindowFlags_NoSavedSettings)) {
    return;
  }

  if (ImGui::BeginTabBar("help-tabs")) {
    if (ImGui::BeginTabItem("Overview")) {
      ImGui::TextWrapped(
          "Shell ini sekarang lebih dekat ke workspace arch milik qreasee-app: "
          "rail kiri, project browser, toolbar melayang, inspector kanan, "
          "viewport fokus, dan dock alat di bawah.");
      ImGui::Spacing();
      ImGui::BulletText("Theme mode: System, Light, Dark.");
      ImGui::BulletText(
          "World sekarang berisi architectural massing, glass band, "
          "outline level, dan section guides.");
      ImGui::BulletText("Project browser kiri meniru pola views, floor plans, "
                        "elevations, dan sections.");
      ImGui::BulletText("Inspector kanan dipisah menjadi Tools, Camera, Mats, "
                        "dan Project.");
      ImGui::BulletText(
          "Select tool tetap punya SelectionMarquee window/crossing.");
      ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Shortcuts")) {
      if (ImGui::BeginTable("shortcut-table", 2,
                            ImGuiTableFlags_RowBg |
                                ImGuiTableFlags_BordersInnerV |
                                ImGuiTableFlags_SizingStretchProp)) {
        ImGui::TableSetupColumn("Shortcut", ImGuiTableColumnFlags_WidthFixed,
                                ui_metrics.shortcut_table_key_width);
        ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();
        workspace_ui::DrawShortcutRow(InputConstants::kOpenHelpLabel,
                                      "Open Help & System Information");
        workspace_ui::DrawShortcutRow(InputConstants::kRefreshSystemInfoLabel,
                                      "Refresh WinRT system information");
        workspace_ui::DrawShortcutRow(InputConstants::kToggleInspectorLabel,
                                      "Toggle Default Tray");
        workspace_ui::DrawShortcutRow(InputConstants::kResetCameraLabel,
                                      "Reset camera to home view");
        workspace_ui::DrawShortcutRow(InputConstants::kFrontViewLabel,
                                      "Front camera preset");
        workspace_ui::DrawShortcutRow(InputConstants::kRightViewLabel,
                                      "Right camera preset");
        workspace_ui::DrawShortcutRow(InputConstants::kIsometricViewLabel,
                                      "Isometric camera preset");
        workspace_ui::DrawShortcutRow(InputConstants::kTopViewLabel,
                                      "Top camera preset");
        workspace_ui::DrawShortcutRow(InputConstants::kToggleGridLabel,
                                      "Toggle ground grid");
        workspace_ui::DrawShortcutRow(InputConstants::kToggleAxesLabel,
                                      "Toggle world axes");
        workspace_ui::DrawShortcutRow(InputConstants::kToggleGuideLabel,
                                      "Toggle guide cube");
        ImGui::EndTable();
      }
      ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("System")) {
      if (ImGui::BeginTable("system-table", 2,
                            ImGuiTableFlags_RowBg |
                                ImGuiTableFlags_BordersInnerV |
                                ImGuiTableFlags_SizingStretchProp)) {
        ImGui::TableSetupColumn("Field", ImGuiTableColumnFlags_WidthFixed,
                                ui_metrics.system_table_field_width);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();
        workspace_ui::DrawSystemRow("Calendar system",
                                    snapshot_.calendar_system);
        workspace_ui::DrawSystemRow("Time zone", snapshot_.time_zone);
        workspace_ui::DrawSystemRow("Last refresh", snapshot_.last_updated);
        workspace_ui::DrawSystemRow("Theme mode", ThemeModeLabel(theme_mode_));
        workspace_ui::DrawSystemRow(
            "Resolved theme",
            resolved_dark_theme_ ? std::string("Dark") : std::string("Light"));
        ImGui::EndTable();
      }
      ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
  }

  ImGui::Separator();
  if (ImGui::Button("Refresh System Info",
                    ui_metrics.help_refresh_button_size)) {
    RefreshSnapshot();
  }
  ImGui::SameLine();
  if (ImGui::Button("Close", ui_metrics.help_close_button_size)) {
    ImGui::CloseCurrentPopup();
  }

  ImGui::EndPopup();
}

void Application::ResetLayout() {
  inspector_visible_ = true;
  show_grid_ = true;
  show_axes_ = true;
  show_guide_cube_ = false;
  inspector_tab_ = 0;
  active_workspace_index_ = 3;
  perspective_index_ = 0;
  browser_sections_expanded_ = {true, true, true, true, true};
  CancelSelectionMarquee();
  last_marquee_valid_ = false;
  camera_.SetIsometricView();
}

void Application::InstallNativeMenuBar() {
  HWND const hwnd = NativeWindowHandle(window_);
  if (hwnd == nullptr || native_menu_ != nullptr) {
    return;
  }

  HMENU const root_menu = CreateMenu();
  HMENU const file_menu = CreatePopupMenu();
  HMENU const view_menu = CreatePopupMenu();
  HMENU const theme_menu = CreatePopupMenu();
  HMENU const help_menu = CreatePopupMenu();
  if (root_menu == nullptr || file_menu == nullptr || view_menu == nullptr ||
      theme_menu == nullptr || help_menu == nullptr) {
    if (help_menu != nullptr) {
      DestroyMenu(help_menu);
    }
    if (theme_menu != nullptr) {
      DestroyMenu(theme_menu);
    }
    if (view_menu != nullptr) {
      DestroyMenu(view_menu);
    }
    if (file_menu != nullptr) {
      DestroyMenu(file_menu);
    }
    if (root_menu != nullptr) {
      DestroyMenu(root_menu);
    }
    return;
  }

  AppendNativeMenuItem(file_menu, kNativeMenuFileExit, L"E&xit");

  AppendNativeMenuItem(view_menu, kNativeMenuViewInspector, L"&Inspector");
  AppendNativeMenuItem(view_menu, kNativeMenuViewGrid, L"&Grid");
  AppendNativeMenuItem(view_menu, kNativeMenuViewAxes, L"&Axes");
  AppendNativeMenuItem(view_menu, kNativeMenuViewSectionBox, L"&Section Box");
  AppendMenuW(view_menu, MF_SEPARATOR, 0, nullptr);
  AppendNativeMenuItem(view_menu, kNativeMenuViewResetCamera, L"&Reset Camera");

  AppendNativeMenuItem(theme_menu, kNativeMenuThemeSystem, L"&System");
  AppendNativeMenuItem(theme_menu, kNativeMenuThemeLight, L"&Light");
  AppendNativeMenuItem(theme_menu, kNativeMenuThemeDark, L"&Dark");

  AppendNativeMenuItem(help_menu, kNativeMenuHelpSystem,
                       L"&Help && System Information");
  AppendNativeMenuItem(help_menu, kNativeMenuHelpRefresh,
                       L"&Refresh System Info");

  AppendMenuW(root_menu, MF_POPUP, reinterpret_cast<UINT_PTR>(file_menu),
              L"&File");
  AppendMenuW(root_menu, MF_POPUP, reinterpret_cast<UINT_PTR>(view_menu),
              L"&View");
  AppendMenuW(root_menu, MF_POPUP, reinterpret_cast<UINT_PTR>(theme_menu),
              L"&Theme");
  AppendMenuW(root_menu, MF_POPUP, reinterpret_cast<UINT_PTR>(help_menu),
              L"&Help");

  SetPropW(hwnd, kNativeMenuAppPropName, this);
  auto previous_window_proc = reinterpret_cast<WNDPROC>(SetWindowLongPtrW(
      hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(NativeMenuWindowProc)));
  SetPropW(hwnd, kNativeMenuPrevWndProcPropName,
           reinterpret_cast<HANDLE>(previous_window_proc));

  SetMenu(hwnd, root_menu);
  ::DrawMenuBar(hwnd);

  native_hwnd_ = hwnd;
  native_menu_ = root_menu;
}

void Application::DestroyNativeMenuBar() {
  auto *hwnd = static_cast<HWND>(native_hwnd_);
  auto *menu = static_cast<HMENU>(native_menu_);
  if (hwnd != nullptr) {
    auto previous_window_proc = reinterpret_cast<WNDPROC>(
        GetPropW(hwnd, kNativeMenuPrevWndProcPropName));
    if (previous_window_proc != nullptr) {
      SetWindowLongPtrW(hwnd, GWLP_WNDPROC,
                        reinterpret_cast<LONG_PTR>(previous_window_proc));
    }
    RemovePropW(hwnd, kNativeMenuPrevWndProcPropName);
    RemovePropW(hwnd, kNativeMenuAppPropName);
    SetMenu(hwnd, nullptr);
    ::DrawMenuBar(hwnd);
  }

  if (menu != nullptr) {
    DestroyMenu(menu);
  }

  native_hwnd_ = nullptr;
  native_menu_ = nullptr;
}

void Application::UpdateNativeMenuBar() const {
  auto *menu = static_cast<HMENU>(native_menu_);
  auto *hwnd = static_cast<HWND>(native_hwnd_);
  if (menu == nullptr) {
    return;
  }

  auto *view_menu = GetSubMenu(menu, 1);
  auto *theme_menu = GetSubMenu(menu, 2);
  if (view_menu != nullptr) {
    CheckMenuItem(view_menu, kNativeMenuViewInspector,
                  MF_BYCOMMAND |
                      (inspector_visible_ ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(view_menu, kNativeMenuViewGrid,
                  MF_BYCOMMAND | (show_grid_ ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(view_menu, kNativeMenuViewAxes,
                  MF_BYCOMMAND | (show_axes_ ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(view_menu, kNativeMenuViewSectionBox,
                  MF_BYCOMMAND |
                      (show_guide_cube_ ? MF_CHECKED : MF_UNCHECKED));
  }

  if (theme_menu != nullptr) {
    unsigned int checked_theme_command = kNativeMenuThemeSystem;
    if (theme_mode_ == ThemeMode::Light) {
      checked_theme_command = kNativeMenuThemeLight;
    } else if (theme_mode_ == ThemeMode::Dark) {
      checked_theme_command = kNativeMenuThemeDark;
    }
    CheckMenuRadioItem(theme_menu, kNativeMenuThemeSystem,
                       kNativeMenuThemeDark, checked_theme_command,
                       MF_BYCOMMAND);
  }

  if (hwnd != nullptr) {
    ::DrawMenuBar(hwnd);
  }
}

bool Application::HandleNativeMenuCommand(unsigned int command_id) {
  switch (command_id) {
  case kNativeMenuFileExit:
    running_ = false;
    return true;
  case kNativeMenuViewInspector:
    inspector_visible_ = !inspector_visible_;
    SetStatusMessage(inspector_visible_ ? "Default Tray shown."
                                        : "Default Tray hidden.");
    return true;
  case kNativeMenuViewGrid:
    show_grid_ = !show_grid_;
    SetStatusMessage(show_grid_ ? "Ground grid enabled."
                                : "Ground grid hidden.");
    return true;
  case kNativeMenuViewAxes:
    show_axes_ = !show_axes_;
    SetStatusMessage(show_axes_ ? "World axes enabled."
                                : "World axes hidden.");
    return true;
  case kNativeMenuViewSectionBox:
    show_guide_cube_ = !show_guide_cube_;
    SetStatusMessage(show_guide_cube_ ? "Section box enabled."
                                      : "Section box hidden.");
    return true;
  case kNativeMenuViewResetCamera:
    camera_.Reset();
    perspective_index_ = 0;
    SetStatusMessage("Camera reset to home view.");
    return true;
  case kNativeMenuThemeSystem:
    ApplyThemeMode(ThemeMode::System);
    return true;
  case kNativeMenuThemeLight:
    ApplyThemeMode(ThemeMode::Light);
    return true;
  case kNativeMenuThemeDark:
    ApplyThemeMode(ThemeMode::Dark);
    return true;
  case kNativeMenuHelpSystem:
    show_help_modal_ = true;
    SetStatusMessage("Help & System Information opened.");
    return true;
  case kNativeMenuHelpRefresh:
    RefreshSnapshot();
    return true;
  default:
    return false;
  }
}

void Application::ReloadRenderer(bool quiet) {
  std::string error;
  if (!world_renderer_.Initialize(gl_, base_path_ / "res" / "shaders", scene_,
                                  error)) {
    last_error_ = error;
    if (!quiet) {
      SetStatusMessage("Renderer reload failed. Check shader files.");
    }
    return;
  }

  last_error_.clear();
  if (!quiet) {
    SetStatusMessage("Renderer reloaded from res/shaders.");
  }
}

void Application::ApplyThemeMode(ThemeMode mode, bool quiet) {
  theme_mode_ = mode;

  bool next_dark_theme = false;
  switch (theme_mode_) {
  case ThemeMode::System:
    next_dark_theme = DetectSystemPrefersDarkTheme();
    break;
  case ThemeMode::Light:
    next_dark_theme = false;
    break;
  case ThemeMode::Dark:
    next_dark_theme = true;
    break;
  }

  resolved_dark_theme_ = next_dark_theme;
  last_theme_sync_ticks_ = SDL_GetTicks();

  if (ImGui::GetCurrentContext() != nullptr) {
    ui_style::ApplyUiTheme(main_scale_, resolved_dark_theme_);
  }

  world_style_ = BuildWorldVisualStyle(resolved_dark_theme_);
  scene_ = CreateSketchWorld(world_style_);
  if (gl_context_ != nullptr) {
    ReloadRenderer(true);
  }

  if (!quiet) {
    SetStatusMessage(fmt::format("Theme mode set to {}. Resolved theme is {}.",
                                 ThemeModeLabel(theme_mode_),
                                 resolved_dark_theme_ ? "Dark" : "Light"));
  }
}

void Application::SyncThemeWithSystem() {
  if (theme_mode_ != ThemeMode::System) {
    return;
  }

  const Uint64 now = SDL_GetTicks();
  if (now - last_theme_sync_ticks_ < 1000) {
    return;
  }

  last_theme_sync_ticks_ = now;
  const bool current_dark_theme = DetectSystemPrefersDarkTheme();
  if (current_dark_theme != resolved_dark_theme_) {
    ApplyThemeMode(ThemeMode::System, true);
    SetStatusMessage(
        fmt::format("Theme synced with Windows. Current mode is {}.",
                    resolved_dark_theme_ ? "Dark" : "Light"));
  }
}

void Application::RefreshSnapshot() {
  snapshot_ = CaptureWinRtSnapshot();
  SetStatusMessage(
      fmt::format("System info refreshed at {}.", snapshot_.last_updated));
}

void Application::SetActiveTool(ToolKind tool, bool quiet) {
  if (tool != ToolKind::Select) {
    CancelSelectionMarquee();
  }
  active_tool_ = tool;
  if (tool == ToolKind::Paint) {
    inspector_tab_ = 2;
  } else if (tool == ToolKind::Orbit || tool == ToolKind::Pan ||
             tool == ToolKind::Zoom || tool == ToolKind::Section) {
    inspector_tab_ = 1;
  } else if (tool == ToolKind::Wall || tool == ToolKind::Floor ||
             tool == ToolKind::Ceiling || tool == ToolKind::Roof ||
             tool == ToolKind::Column || tool == ToolKind::Beam ||
             tool == ToolKind::Door || tool == ToolKind::Window) {
    inspector_tab_ = 0;
  }
  if (!quiet) {
    SetStatusMessage(fmt::format("{} tool selected.", ToolLabel(active_tool_)));
  }
}

void Application::SetStatusMessage(std::string message) {
  status_message_ = std::move(message);
}

std::filesystem::path Application::ResolveBasePath() const {
  char const *raw_base_path = SDL_GetBasePath();
  if (raw_base_path == nullptr) {
    return std::filesystem::current_path();
  }

  return std::filesystem::path(raw_base_path);
}

void Application::UpdateViewportRect() {
  ImGuiIO &io = ImGui::GetIO();
  // Render the world behind the entire window. UI panels now float on top
  // instead of carving the viewport into a smaller center area.
  viewport_rect_.x = 0.0f;
  viewport_rect_.y = 0.0f;
  viewport_rect_.w = std::max(0.0f, io.DisplaySize.x);
  viewport_rect_.h = std::max(0.0f, io.DisplaySize.y);
}

float Application::ResolvedTrayWidth() const {
  const ui_style::UiMetrics ui_metrics = ui_style::BuildUiMetrics(main_scale_);
  return std::clamp(tray_width_, ui_metrics.tray_min_width,
                    ui_metrics.tray_max_width);
}

SDL_FRect Application::WorkspaceChromeRect() const {
  ImGuiIO const &io = ImGui::GetIO();
  const ui_style::UiMetrics ui_metrics = ui_style::BuildUiMetrics(main_scale_);
  const float left = ui_metrics.chrome_margin + ui_metrics.nav_rail_width +
                     ui_metrics.chrome_gap + ui_metrics.left_panel_width +
                     ui_metrics.chrome_gap;
  const float right =
      io.DisplaySize.x - ui_metrics.chrome_margin -
      (inspector_visible_ ? ResolvedTrayWidth() + ui_metrics.chrome_gap : 0.0f);
  const float top = ui_metrics.chrome_margin;
  const float bottom = io.DisplaySize.y - ui_metrics.chrome_margin;
  return SDL_FRect{
      left,
      top,
      std::max(0.0f, right - left),
      std::max(0.0f, bottom - top),
  };
}

bool Application::HandleSelectionMarqueeEvent(SDL_Event const &event) {
  if (active_tool_ != ToolKind::Select) {
    return false;
  }

  switch (event.type) {
  case SDL_EVENT_MOUSE_BUTTON_DOWN:
    if (event.button.button != SDL_BUTTON_LEFT) {
      return false;
    }
    if (ImGui::GetIO().WantCaptureMouse ||
        !IsPointInsideViewport(event.button.x, event.button.y)) {
      return false;
    }
    marquee_active_ = true;
    marquee_visible_ = false;
    marquee_origin_ =
        ClampPointToViewport(SDL_FPoint{event.button.x, event.button.y});
    marquee_current_ = marquee_origin_;
    return true;

  case SDL_EVENT_MOUSE_MOTION:
    if (!marquee_active_) {
      return false;
    }
    marquee_current_ =
        ClampPointToViewport(SDL_FPoint{event.motion.x, event.motion.y});
    marquee_visible_ = IsSelectionRectVisible(CurrentSelectionRect());
    return true;

  case SDL_EVENT_MOUSE_BUTTON_UP:
    if (!marquee_active_ || event.button.button != SDL_BUTTON_LEFT) {
      return false;
    }
    marquee_current_ =
        ClampPointToViewport(SDL_FPoint{event.button.x, event.button.y});
    marquee_visible_ = IsSelectionRectVisible(CurrentSelectionRect());

    if (marquee_visible_) {
      last_marquee_rect_ = CurrentSelectionRect();
      last_marquee_valid_ = true;
      last_marquee_crossing_ = marquee_current_.x < marquee_origin_.x;
      SetStatusMessage(
          fmt::format("{} marquee completed: {:.0f} x {:.0f}.",
                      last_marquee_crossing_ ? "Crossing" : "Window",
                      last_marquee_rect_.w, last_marquee_rect_.h));
    } else {
      last_marquee_valid_ = false;
      SetStatusMessage("Selection click registered.");
    }

    marquee_active_ = false;
    marquee_visible_ = false;
    return true;

  default:
    return false;
  }
}

void Application::DrawSelectionMarquee() const {
  if (!marquee_active_ || !marquee_visible_) {
    return;
  }

  const SDL_FRect rect = CurrentSelectionRect();
  const bool crossing = marquee_current_.x < marquee_origin_.x;
  const ui_style::SelectionMarqueeStyle marquee_style =
      ui_style::BuildSelectionMarqueeStyle(main_scale_, resolved_dark_theme_,
                                           crossing);

  auto *draw_list = ImGui::GetForegroundDrawList();
  const ImVec2 min(rect.x, rect.y);
  const ImVec2 max(rect.x + rect.w, rect.y + rect.h);
  draw_list->AddRectFilled(min, max, marquee_style.fill_color);
  draw_list->AddRect(min, max, marquee_style.border_color, 0.0f, 0,
                     marquee_style.border_thickness);

  const char *label = crossing ? "CROSSING" : "WINDOW";
  const ImVec2 label_pos(
      rect.x + marquee_style.label_padding,
      std::max(viewport_rect_.y + marquee_style.label_padding,
               rect.y - marquee_style.label_lift));
  draw_list->AddText(label_pos, marquee_style.text_color, label);
}

void Application::CancelSelectionMarquee() {
  marquee_active_ = false;
  marquee_visible_ = false;
}

bool Application::IsPointInsideViewport(float x, float y) const {
  return x >= viewport_rect_.x && y >= viewport_rect_.y &&
         x <= viewport_rect_.x + viewport_rect_.w &&
         y <= viewport_rect_.y + viewport_rect_.h;
}

SDL_FPoint Application::ClampPointToViewport(SDL_FPoint point) const {
  point.x = std::clamp(point.x, viewport_rect_.x,
                       viewport_rect_.x + viewport_rect_.w);
  point.y = std::clamp(point.y, viewport_rect_.y,
                       viewport_rect_.y + viewport_rect_.h);
  return point;
}

SDL_FRect Application::CurrentSelectionRect() const {
  return MakeNormalizedRect(marquee_origin_, marquee_current_);
}
