#pragma once

#include <filesystem>

#include <imgui.h>

namespace ui_style {

struct UiMetrics {
    float menu_bar_height;
    float command_bar_height;
    float status_bar_height;
    float chrome_margin;
    float chrome_gap;
    float nav_rail_width;
    float left_panel_width;
    float tray_width;
    float tray_min_width;
    float tray_max_width;
    float panel_rounding;
    float inner_rounding;
    float top_toolbar_width;
    float top_toolbar_height;
    float bottom_dock_width;
    float bottom_dock_height;
    float bottom_hud_width;
    float bottom_hud_height;
    float viewport_gizmo_radius;
    float toolbar_group_spacing;
    float toolbar_separator_spacing;
    float toolbar_theme_combo_width;
    float toolbar_right_padding;
    float status_right_padding;
    float shortcut_table_key_width;
    float system_table_field_width;
    ImVec2 initial_window_size;
    ImVec2 minimum_window_size;
    ImVec2 rail_button_size;
    ImVec2 top_toolbar_button_size;
    ImVec2 dock_button_size;
    ImVec2 right_header_button_size;
    ImVec2 right_action_tile_size;
    ImVec2 inspector_tab_button_size;
    ImVec2 inspector_toggle_size;
    ImVec2 tool_button_size;
    ImVec2 toolbar_tray_toggle_button_size;
    ImVec2 toolbar_help_button_size;
    ImVec2 camera_home_button_size;
    ImVec2 camera_preset_button_size;
    ImVec2 camera_top_button_size;
    ImVec2 material_swatch_size;
    ImVec2 help_modal_size;
    ImVec2 help_refresh_button_size;
    ImVec2 help_close_button_size;
};

struct SelectionMarqueeStyle {
    ImU32 border_color;
    ImU32 fill_color;
    ImU32 text_color;
    float border_thickness;
    float label_padding;
    float label_lift;
};

[[nodiscard]] UiMetrics BuildUiMetrics(float scale);
[[nodiscard]] SelectionMarqueeStyle BuildSelectionMarqueeStyle(float scale, bool dark_theme, bool crossing);
[[nodiscard]] ImFont* BodyFont();
[[nodiscard]] ImFont* HeadingFont();
[[nodiscard]] ImFont* LabelFont();
[[nodiscard]] ImFont* CaptionFont();

void ApplyUiTheme(float scale, bool dark_theme);
void LoadUiFonts(float scale, std::filesystem::path const& fonts_directory);
void PushActiveToolButtonStyle();
void PopActiveToolButtonStyle();

} // namespace ui_style
