#pragma once

#include <array>
#include <filesystem>
#include <string>

#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

#include "app/app_types.h"
#include "platform/winrt_info.h"
#include "render/gl_api.h"
#include "render/world_renderer.h"
#include "ui/svg_icon_cache.h"
#include "world/orbit_camera.h"
#include "world/world_scene.h"

class Application {
public:
    int Run();

private:
    bool Initialize();
    void Shutdown();
    void ProcessEvent(SDL_Event const& event);
    void HandleKeyboardShortcut(SDL_KeyboardEvent const& event);
    void DrawInterface();
    void DrawMenuBar();
    void DrawCommandBar();
    void DrawInspector();
    void DrawStatusBar();
    void DrawHelpModal();
    void UpdateViewportRect();
    bool HandleSelectionMarqueeEvent(SDL_Event const& event);
    void DrawSelectionMarquee() const;
    void CancelSelectionMarquee();
    void InstallNativeMenuBar();
    void DestroyNativeMenuBar();
    void UpdateNativeMenuBar() const;
    bool HandleNativeMenuCommand(unsigned int command_id);
    void ResetLayout();
    void ReloadRenderer(bool quiet = false);
    void ApplyThemeMode(ThemeMode mode, bool quiet = false);
    void SyncThemeWithSystem();
    void RefreshSnapshot();
    bool ConfirmExit();
    void SetActiveTool(ToolKind tool, bool quiet = false);
    void SetStatusMessage(std::string message);

    [[nodiscard]] std::filesystem::path ResolveBasePath() const;
    [[nodiscard]] float ResolvedTrayWidth() const;
    [[nodiscard]] SDL_FRect WorkspaceChromeRect() const;
    [[nodiscard]] bool IsPointInsideViewport(float x, float y) const;
    [[nodiscard]] SDL_FPoint ClampPointToViewport(SDL_FPoint point) const;
    [[nodiscard]] SDL_FRect CurrentSelectionRect() const;

    SDL_Window* window_ = nullptr;
    SDL_GLContext gl_context_ = nullptr;
    float main_scale_ = 1.0f;
    bool running_ = false;
    bool inspector_visible_ = true;
    bool show_help_modal_ = false;
    bool show_grid_ = true;
    bool show_axes_ = true;
    bool show_guide_cube_ = false;
    bool resolved_dark_theme_ = false;
    ThemeMode theme_mode_ = ThemeMode::System;
    ToolKind active_tool_ = ToolKind::Select;
    int active_workspace_index_ = 0;
    int inspector_tab_ = 0;
    int perspective_index_ = 0;
    int units_index_ = 0;
    float tolerance_value_ = 0.30f;
    bool dimension_snap_ = true;
    bool angle_snap_ = false;
    bool parallel_snap_ = true;
    bool perpendicular_snap_ = false;
    std::array<bool, 5> browser_sections_expanded_{true, true, true, true, true};
    Uint64 last_theme_sync_ticks_ = 0;
    float menu_bar_height_ = 0.0f;
    float command_bar_height_ = 0.0f;
    float status_bar_height_ = 0.0f;
    float tray_width_ = 0.0f;
    SDL_FRect viewport_rect_{};
    SDL_FPoint marquee_origin_{};
    SDL_FPoint marquee_current_{};
    SDL_FRect last_marquee_rect_{};
    bool marquee_active_ = false;
    bool marquee_visible_ = false;
    bool last_marquee_valid_ = false;
    bool last_marquee_crossing_ = false;

    GlApi gl_{};
    WorldRenderer world_renderer_{};
    OrbitCamera camera_{};
    WorldScene scene_{};
    WorldVisualStyle world_style_{};
    WinRtSnapshot snapshot_{};
    workspace_ui::SvgIconCache icon_cache_{};
    std::filesystem::path base_path_{};
    std::string status_message_{"Ready"};
    std::string last_error_{};
    void* native_hwnd_ = nullptr;
    void* native_menu_ = nullptr;
};
