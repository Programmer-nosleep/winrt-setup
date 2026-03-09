#include "style/ui_style.h"

#include <array>
#include <filesystem>
#include <string>

namespace ui_style {
namespace {

ImFont* g_body_font = nullptr;
ImFont* g_heading_font = nullptr;
ImFont* g_label_font = nullptr;
ImFont* g_caption_font = nullptr;
constexpr float kMediumFontSize = 13.25f;

UiMetrics BaseUiMetrics()
{
    return UiMetrics{
        .menu_bar_height = 24.0f,
        .command_bar_height = 44.0f,
        .status_bar_height = 28.0f,
        .chrome_margin = 16.0f,
        .chrome_gap = 14.0f,
        .nav_rail_width = 72.0f,
        .left_panel_width = 320.0f,
        .tray_width = 328.0f,
        .tray_min_width = 288.0f,
        .tray_max_width = 388.0f,
        .panel_rounding = 12.0f,
        .inner_rounding = 8.0f,
        .top_toolbar_width = 452.0f,
        .top_toolbar_height = 44.0f,
        .bottom_dock_width = 436.0f,
        .bottom_dock_height = 44.0f,
        .bottom_hud_width = 286.0f,
        .bottom_hud_height = 82.0f,
        .viewport_gizmo_radius = 34.0f,
        .toolbar_group_spacing = 16.0f,
        .toolbar_separator_spacing = 14.0f,
        .toolbar_theme_combo_width = 120.0f,
        .toolbar_right_padding = 16.0f,
        .status_right_padding = 20.0f,
        .shortcut_table_key_width = 140.0f,
        .system_table_field_width = 190.0f,
        .initial_window_size = ImVec2(1460.0f, 860.0f),
        .minimum_window_size = ImVec2(1180.0f, 760.0f),
        .rail_button_size = ImVec2(48.0f, 50.0f),
        .top_toolbar_button_size = ImVec2(34.0f, 34.0f),
        .dock_button_size = ImVec2(34.0f, 34.0f),
        .right_header_button_size = ImVec2(28.0f, 28.0f),
        .right_action_tile_size = ImVec2(100.0f, 72.0f),
        .inspector_tab_button_size = ImVec2(74.0f, 28.0f),
        .inspector_toggle_size = ImVec2(36.0f, 20.0f),
        .tool_button_size = ImVec2(56.0f, 26.0f),
        .toolbar_tray_toggle_button_size = ImVec2(84.0f, 26.0f),
        .toolbar_help_button_size = ImVec2(72.0f, 26.0f),
        .camera_home_button_size = ImVec2(64.0f, 26.0f),
        .camera_preset_button_size = ImVec2(76.0f, 28.0f),
        .camera_top_button_size = ImVec2(60.0f, 26.0f),
        .material_swatch_size = ImVec2(52.0f, 28.0f),
        .help_modal_size = ImVec2(760.0f, 620.0f),
        .help_refresh_button_size = ImVec2(160.0f, 32.0f),
        .help_close_button_size = ImVec2(96.0f, 32.0f),
    };
}

ImVec2 Scale(ImVec2 value, float scale)
{
    return ImVec2(value.x * scale, value.y * scale);
}

float RelativeLuminance(ImVec4 const& color)
{
    return (0.2126f * color.x) + (0.7152f * color.y) + (0.0722f * color.z);
}

template <std::size_t N>
ImFont* AddFirstAvailableFont(
    std::array<std::filesystem::path, N> const& paths,
    float size
)
{
    ImGuiIO& io = ImGui::GetIO();
    for (auto const& path : paths) {
        if (!std::filesystem::exists(path)) {
            continue;
        }

        const std::string path_string = path.string();
        if (ImFont* font = io.Fonts->AddFontFromFileTTF(path_string.c_str(), size); font != nullptr) {
            return font;
        }
    }

    return nullptr;
}

} // namespace

UiMetrics BuildUiMetrics(float scale)
{
    UiMetrics metrics = BaseUiMetrics();
    metrics.menu_bar_height *= scale;
    metrics.command_bar_height *= scale;
    metrics.status_bar_height *= scale;
    metrics.chrome_margin *= scale;
    metrics.chrome_gap *= scale;
    metrics.nav_rail_width *= scale;
    metrics.left_panel_width *= scale;
    metrics.tray_width *= scale;
    metrics.tray_min_width *= scale;
    metrics.tray_max_width *= scale;
    metrics.panel_rounding *= scale;
    metrics.inner_rounding *= scale;
    metrics.top_toolbar_width *= scale;
    metrics.top_toolbar_height *= scale;
    metrics.bottom_dock_width *= scale;
    metrics.bottom_dock_height *= scale;
    metrics.bottom_hud_width *= scale;
    metrics.bottom_hud_height *= scale;
    metrics.viewport_gizmo_radius *= scale;
    metrics.toolbar_group_spacing *= scale;
    metrics.toolbar_separator_spacing *= scale;
    metrics.toolbar_theme_combo_width *= scale;
    metrics.toolbar_right_padding *= scale;
    metrics.status_right_padding *= scale;
    metrics.shortcut_table_key_width *= scale;
    metrics.system_table_field_width *= scale;
    metrics.initial_window_size = Scale(metrics.initial_window_size, scale);
    metrics.minimum_window_size = Scale(metrics.minimum_window_size, scale);
    metrics.rail_button_size = Scale(metrics.rail_button_size, scale);
    metrics.top_toolbar_button_size = Scale(metrics.top_toolbar_button_size, scale);
    metrics.dock_button_size = Scale(metrics.dock_button_size, scale);
    metrics.right_header_button_size = Scale(metrics.right_header_button_size, scale);
    metrics.right_action_tile_size = Scale(metrics.right_action_tile_size, scale);
    metrics.inspector_tab_button_size = Scale(metrics.inspector_tab_button_size, scale);
    metrics.inspector_toggle_size = Scale(metrics.inspector_toggle_size, scale);
    metrics.tool_button_size = Scale(metrics.tool_button_size, scale);
    metrics.toolbar_tray_toggle_button_size = Scale(metrics.toolbar_tray_toggle_button_size, scale);
    metrics.toolbar_help_button_size = Scale(metrics.toolbar_help_button_size, scale);
    metrics.camera_home_button_size = Scale(metrics.camera_home_button_size, scale);
    metrics.camera_preset_button_size = Scale(metrics.camera_preset_button_size, scale);
    metrics.camera_top_button_size = Scale(metrics.camera_top_button_size, scale);
    metrics.material_swatch_size = Scale(metrics.material_swatch_size, scale);
    metrics.help_modal_size = Scale(metrics.help_modal_size, scale);
    metrics.help_refresh_button_size = Scale(metrics.help_refresh_button_size, scale);
    metrics.help_close_button_size = Scale(metrics.help_close_button_size, scale);
    return metrics;
}

SelectionMarqueeStyle BuildSelectionMarqueeStyle(float scale, bool dark_theme, bool crossing)
{
    return SelectionMarqueeStyle{
        .border_color = crossing
            ? (dark_theme ? IM_COL32(78, 201, 140, 255) : IM_COL32(20, 138, 90, 255))
            : (dark_theme ? IM_COL32(217, 128, 64, 255) : IM_COL32(188, 102, 33, 255)),
        .fill_color = crossing
            ? (dark_theme ? IM_COL32(78, 201, 140, 42) : IM_COL32(20, 138, 90, 34))
            : (dark_theme ? IM_COL32(217, 128, 64, 38) : IM_COL32(188, 102, 33, 32)),
        .text_color = dark_theme ? IM_COL32(240, 244, 248, 255) : IM_COL32(20, 28, 38, 255),
        .border_thickness = 1.5f * scale,
        .label_padding = 6.0f * scale,
        .label_lift = 20.0f * scale,
    };
}

void ApplyUiTheme(float scale, bool dark_theme)
{
    ImGuiStyle& style = ImGui::GetStyle();
    style = ImGuiStyle();
    if (dark_theme) {
        ImGui::StyleColorsDark();
    } else {
        ImGui::StyleColorsLight();
    }

    style.ScaleAllSizes(scale);
    style.FontScaleDpi = 1.0f;
    style.WindowRounding = 12.0f;
    style.ChildRounding = 10.0f;
    style.FrameRounding = 8.0f;
    style.PopupRounding = 10.0f;
    style.ScrollbarRounding = 8.0f;
    style.GrabRounding = 8.0f;
    style.TabRounding = 8.0f;
    style.WindowPadding = ImVec2(16.0f, 14.0f);
    style.FramePadding = ImVec2(10.0f, 7.0f);
    style.ItemSpacing = ImVec2(8.0f, 8.0f);
    style.ItemInnerSpacing = ImVec2(7.0f, 6.0f);
    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 0.0f;
    style.FrameBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.TabBorderSize = 0.0f;

    auto& colors = style.Colors;
    if (dark_theme) {
        colors[ImGuiCol_Text] = ImVec4(0.94f, 0.95f, 0.98f, 1.0f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.52f, 0.56f, 0.63f, 1.0f);
        colors[ImGuiCol_WindowBg] = ImVec4(0.05f, 0.06f, 0.08f, 0.95f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.07f, 0.08f, 0.10f, 0.98f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.09f, 0.12f, 0.98f);
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.05f, 0.06f, 0.08f, 0.96f);
        colors[ImGuiCol_Border] = ImVec4(0.15f, 0.18f, 0.23f, 1.0f);
        colors[ImGuiCol_Separator] = ImVec4(0.14f, 0.17f, 0.22f, 1.0f);
        colors[ImGuiCol_Button] = ImVec4(0.10f, 0.11f, 0.15f, 1.0f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.14f, 0.17f, 0.22f, 1.0f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.85f, 0.50f, 0.25f, 1.0f);
        colors[ImGuiCol_Header] = ImVec4(0.09f, 0.11f, 0.14f, 1.0f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.14f, 0.17f, 0.22f, 1.0f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.85f, 0.50f, 0.25f, 1.0f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.09f, 0.10f, 0.13f, 1.0f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.14f, 0.18f, 1.0f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.17f, 0.20f, 0.26f, 1.0f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.05f, 0.06f, 0.08f, 1.0f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.05f, 0.06f, 0.08f, 1.0f);
        colors[ImGuiCol_Tab] = ImVec4(0.09f, 0.11f, 0.14f, 1.0f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.14f, 0.17f, 0.22f, 1.0f);
        colors[ImGuiCol_TabActive] = ImVec4(0.85f, 0.50f, 0.25f, 1.0f);
        colors[ImGuiCol_CheckMark] = ImVec4(0.94f, 0.70f, 0.36f, 1.0f);
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.06f, 0.08f, 1.0f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.17f, 0.20f, 0.26f, 1.0f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.22f, 0.26f, 0.31f, 1.0f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.85f, 0.50f, 0.25f, 1.0f);
    } else {
        colors[ImGuiCol_Text] = ImVec4(0.16f, 0.19f, 0.24f, 1.0f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.45f, 0.49f, 0.56f, 1.0f);
        colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.95f, 0.96f, 0.96f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.97f, 0.98f, 0.99f, 0.98f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.98f, 0.99f, 1.00f, 0.98f);
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.92f, 0.93f, 0.95f, 0.98f);
        colors[ImGuiCol_Border] = ImVec4(0.82f, 0.85f, 0.89f, 1.0f);
        colors[ImGuiCol_Separator] = ImVec4(0.81f, 0.84f, 0.88f, 1.0f);
        colors[ImGuiCol_Button] = ImVec4(0.89f, 0.91f, 0.94f, 1.0f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.84f, 0.88f, 0.92f, 1.0f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.83f, 0.49f, 0.22f, 1.0f);
        colors[ImGuiCol_Header] = ImVec4(0.89f, 0.91f, 0.94f, 1.0f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.84f, 0.88f, 0.92f, 1.0f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.83f, 0.49f, 0.22f, 1.0f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.96f, 0.97f, 0.98f, 1.0f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.91f, 0.93f, 0.96f, 1.0f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.86f, 0.90f, 0.95f, 1.0f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.92f, 0.93f, 0.95f, 1.0f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.92f, 0.93f, 0.95f, 1.0f);
        colors[ImGuiCol_Tab] = ImVec4(0.89f, 0.91f, 0.94f, 1.0f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.84f, 0.88f, 0.92f, 1.0f);
        colors[ImGuiCol_TabActive] = ImVec4(0.83f, 0.49f, 0.22f, 1.0f);
        colors[ImGuiCol_CheckMark] = ImVec4(0.83f, 0.49f, 0.22f, 1.0f);
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.92f, 0.93f, 0.95f, 1.0f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.77f, 0.81f, 0.88f, 1.0f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.70f, 0.76f, 0.86f, 1.0f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.83f, 0.49f, 0.22f, 1.0f);
    }
}

void LoadUiFonts(float scale, std::filesystem::path const& fonts_directory)
{
    ImGuiIO& io = ImGui::GetIO();
    const std::array<std::filesystem::path, 2> regular_fonts = {
        fonts_directory / "segoeui.ttf",
        fonts_directory / "verdana.ttf",
    };
    io.Fonts->Clear();
    g_body_font = AddFirstAvailableFont(regular_fonts, kMediumFontSize * scale);

    if (g_body_font == nullptr) {
        g_body_font = io.Fonts->AddFontDefault();
    }
    g_caption_font = g_body_font;
    g_label_font = g_body_font;
    g_heading_font = g_body_font;

    io.FontDefault = g_body_font;
}

ImFont* BodyFont()
{
    return g_body_font;
}

ImFont* HeadingFont()
{
    return g_heading_font;
}

ImFont* LabelFont()
{
    return g_label_font;
}

ImFont* CaptionFont()
{
    return g_caption_font;
}

void PushActiveToolButtonStyle()
{
    ImGuiStyle const& style = ImGui::GetStyle();
    const ImVec4 active = style.Colors[ImGuiCol_ButtonActive];
    const ImVec4 hovered = style.Colors[ImGuiCol_ButtonHovered];
    const ImVec4 text = RelativeLuminance(active) > 0.60f
        ? ImVec4(0.08f, 0.10f, 0.13f, 1.0f)
        : ImVec4(0.97f, 0.98f, 1.0f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, active);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hovered);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, active);
    ImGui::PushStyleColor(ImGuiCol_Text, text);
}

void PopActiveToolButtonStyle()
{
    ImGui::PopStyleColor(4);
}

} // namespace ui_style
