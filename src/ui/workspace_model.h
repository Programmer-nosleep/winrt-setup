#pragma once

#include <array>
#include <string>
#include <vector>

#include <SDL3/SDL.h>

#include "app/app_types.h"
#include "platform/winrt_info.h"
#include "world/orbit_camera.h"
#include "world/world_scene.h"

namespace workspace_ui {

enum class BrowserSectionId {
    Scene,
    FloorPlans,
    Views3D,
    Elevations,
    Sections,
};

struct RailItem {
    std::string label;
    std::string short_label;
    std::string icon_name;
    bool active = false;
};

struct BrowserItem {
    std::string label;
    std::string meta;
    std::string icon_name;
    bool selected = false;
    bool accent = false;
};

struct BrowserSection {
    BrowserSectionId id = BrowserSectionId::Scene;
    std::string title;
    std::string caption;
    std::string icon_name;
    std::vector<BrowserItem> items;
};

struct BrowserModel {
    std::string project_name;
    std::string project_role;
    std::string workspace_label;
    std::vector<RailItem> rail_items;
    std::vector<BrowserSection> sections;
    std::string session_title;
    std::string session_note;
    std::string project_id;
};

struct ToolActionItem {
    ToolKind tool = ToolKind::Select;
    std::string label;
    std::string caption;
    std::string icon_name;
};

struct InfoRow {
    std::string label;
    std::string value;
    bool accent = false;
};

struct MaterialSwatch {
    std::string label;
    std::array<float, 3> color{};
};

struct InspectorModel {
    std::vector<ToolActionItem> architecture_tools;
    std::vector<InfoRow> camera_rows;
    std::vector<InfoRow> navigation_rows;
    std::vector<InfoRow> project_rows;
    std::vector<MaterialSwatch> swatches;
};

struct SelectionSnapshot {
    bool valid = false;
    bool crossing = false;
    SDL_FRect rect{};
};

struct ViewportOverlayModel {
    std::string title;
    std::string subtitle;
    std::string level_chip;
    std::string mode_chip;
    std::string right_chip;
    std::string footer;
};

[[nodiscard]] BrowserModel BuildBrowserModel(
    int active_workspace_index,
    ToolKind active_tool,
    std::string const& status_message,
    SelectionSnapshot const& selection,
    WinRtSnapshot const& snapshot,
    WorldSceneStats const& stats,
    bool show_grid,
    bool show_axes,
    bool show_section_box
);

[[nodiscard]] InspectorModel BuildInspectorModel(
    ToolKind active_tool,
    ThemeMode theme_mode,
    OrbitCamera const& camera,
    SDL_FRect const& viewport_rect,
    WinRtSnapshot const& snapshot,
    WorldSceneStats const& stats,
    WorldVisualStyle const& style,
    bool show_grid,
    bool show_axes,
    bool show_section_box
);

[[nodiscard]] ViewportOverlayModel BuildViewportOverlayModel(
    ToolKind active_tool,
    ThemeMode theme_mode,
    std::string const& status_message,
    SelectionSnapshot const& selection,
    WorldSceneStats const& stats
);

} // namespace workspace_ui
