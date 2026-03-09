#include "ui/workspace_model.h"

#include <fmt/format.h>

namespace workspace_ui {
namespace {

constexpr const char *kProjectId = "arch-09f3-b2c1-winrt-shell";

char const *WorkspaceLabel(int active_workspace_index) {
  switch (active_workspace_index) {
  case 0:
    return "Home";
  case 1:
    return "Projects";
  case 2:
    return "Template";
  case 3:
    return "Design";
  case 4:
    return "Library";
  case 5:
    return "Material";
  case 6:
    return "Review";
  default:
    return "Home";
  }
}

std::string SelectionLabel(SelectionSnapshot const &selection) {
  if (!selection.valid) {
    return "Selection set idle";
  }

  return fmt::format("{} {:.0f} x {:.0f}",
                     selection.crossing ? "Crossing" : "Window",
                     selection.rect.w, selection.rect.h);
}

} // namespace

BrowserModel BuildBrowserModel(int active_workspace_index, ToolKind active_tool,
                               std::string const &status_message,
                               SelectionSnapshot const &selection,
                               WinRtSnapshot const &snapshot,
                               WorldSceneStats const &stats, bool show_grid,
                               bool show_axes, bool show_section_box) {
  (void)active_tool;
  (void)selection;
  (void)snapshot;
  (void)stats;
  (void)show_grid;
  (void)show_axes;
  (void)show_section_box;
  BrowserModel model;
  model.project_name = "Test";
  model.project_role = WorkspaceLabel(active_workspace_index);
  model.workspace_label = "Qreasee workspace shell";
  model.project_id = kProjectId;
  model.session_title = WorkspaceLabel(active_workspace_index);
  model.session_note = status_message;

  model.rail_items = {
      RailItem{"Home", "HM", "Home", active_workspace_index == 0},
      RailItem{"Projects", "FL", "FolderProject", active_workspace_index == 1},
      RailItem{"Template", "VW", "ResponsiveLayoutLibrary",
               active_workspace_index == 2},
      RailItem{"Design", "AR", "PaletteDesign", active_workspace_index == 3},
      RailItem{"Library", "LB", "Cube", active_workspace_index == 4},
      RailItem{"Material", "MT", "ColorBucketMaterial",
               active_workspace_index == 5},
      RailItem{"Review", "RV", "eyeglasses", active_workspace_index == 6},
  };

  model.sections = {
      BrowserSection{
          .id = BrowserSectionId::Scene,
          .title = "Scene",
          .caption = "",
          .icon_name = "Cube",
          .items =
              {
                  BrowserItem{"Line", "", "Line", false, false},
                  BrowserItem{"Line", "", "Line", false, false},
                  BrowserItem{"Line", "", "Line", false, false},
                  BrowserItem{"Polygon", "", "Octagon", false, false},
                  BrowserItem{"Polygon", "", "Octagon", false, false},
              },
      },
      BrowserSection{
          .id = BrowserSectionId::FloorPlans,
          .title = "Floor Plans",
          .caption = "",
          .icon_name = "ResponsiveLayoutLibrary",
          .items =
              {
                  BrowserItem{"Level 0", "", "ResponsiveLayoutLibrary",
                              active_workspace_index == 2, false},
              },
      },
      BrowserSection{
          .id = BrowserSectionId::Views3D,
          .title = "3D Views",
          .caption = "",
          .icon_name = "Cube",
          .items =
              {
                  BrowserItem{"Default", "", "Cube",
                              active_workspace_index == 3, false},
              },
      },
      BrowserSection{
          .id = BrowserSectionId::Elevations,
          .title = "Elevations (Building Elevation)",
          .caption = "",
          .icon_name = "eyeglasses",
          .items =
              {
                  BrowserItem{"Building Elevation", "", "eyeglasses", false,
                              false},
              },
      },
      BrowserSection{
          .id = BrowserSectionId::Sections,
          .title = "Section (Building Elevation)",
          .caption = "",
          .icon_name = "Section",
          .items =
              {
                  BrowserItem{"No sections yet", "", "Section", false, false},
              },
      },
  };

  return model;
}

InspectorModel
BuildInspectorModel(ToolKind active_tool, ThemeMode theme_mode,
                    OrbitCamera const &camera, SDL_FRect const &viewport_rect,
                    WinRtSnapshot const &snapshot, WorldSceneStats const &stats,
                    WorldVisualStyle const &style, bool show_grid,
                    bool show_axes, bool show_section_box) {
  const Vec3 target = camera.Target();

  InspectorModel model;
  model.architecture_tools = {
      ToolActionItem{ToolKind::Wall, "Wall", "Perimeter shell",
                     ToolIconName(ToolKind::Wall)},
      ToolActionItem{ToolKind::Floor, "Floor", "Level slab",
                     ToolIconName(ToolKind::Floor)},
      ToolActionItem{ToolKind::Ceiling, "Ceiling", "Upper finish",
                     ToolIconName(ToolKind::Ceiling)},
      ToolActionItem{ToolKind::Roof, "Roof", "Roof canopy",
                     ToolIconName(ToolKind::Roof)},
      ToolActionItem{ToolKind::Column, "Column", "Structural grid",
                     ToolIconName(ToolKind::Column)},
      ToolActionItem{ToolKind::Beam, "Beam", "Span frame",
                     ToolIconName(ToolKind::Beam)},
      ToolActionItem{ToolKind::Door, "Door", "Entry opening",
                     ToolIconName(ToolKind::Door)},
      ToolActionItem{ToolKind::Window, "Window", "Facade opening",
                     ToolIconName(ToolKind::Window)},
  };

  model.camera_rows = {
      InfoRow{"Mode", ToolLabel(active_tool), true},
      InfoRow{"Distance", fmt::format("{:.1f} m", camera.Distance())},
      InfoRow{"Yaw", fmt::format("{:.1f} deg", camera.YawDegrees())},
      InfoRow{"Pitch", fmt::format("{:.1f} deg", camera.PitchDegrees())},
      InfoRow{"Target", fmt::format("{:.1f}, {:.1f}, {:.1f}", target.x,
                                    target.y, target.z)},
  };

  model.navigation_rows = {
      InfoRow{"Theme", ThemeModeLabel(theme_mode), true},
      InfoRow{"Grid", show_grid ? "Visible" : "Hidden"},
      InfoRow{"Axes", show_axes ? "Visible" : "Hidden"},
      InfoRow{"Section Box", show_section_box ? "Visible" : "Hidden"},
      InfoRow{"Viewport",
              fmt::format("{:.0f} x {:.0f}", viewport_rect.w, viewport_rect.h)},
  };

  model.project_rows = {
      InfoRow{"Levels", fmt::format("{}", stats.levels)},
      InfoRow{"Walls", fmt::format("{}", stats.walls)},
      InfoRow{"Columns", fmt::format("{}", stats.columns)},
      InfoRow{"Openings", fmt::format("{}", stats.openings)},
      InfoRow{"Views", fmt::format("{}", stats.saved_views)},
      InfoRow{"Last Sync", snapshot.last_updated},
  };

  model.swatches = {
      MaterialSwatch{"Shell", style.shell_primary},
      MaterialSwatch{"Core", style.shell_secondary},
      MaterialSwatch{"Glass", style.glass},
      MaterialSwatch{"Accent", style.accent},
      MaterialSwatch{"Section", style.section},
  };

  return model;
}

ViewportOverlayModel
BuildViewportOverlayModel(ToolKind active_tool, ThemeMode theme_mode,
                          std::string const &status_message,
                          SelectionSnapshot const &selection,
                          WorldSceneStats const &stats) {
  return ViewportOverlayModel{
      .title = "Arch Workspace",
      .subtitle = fmt::format("{} levels / {} saved views", stats.levels,
                              stats.saved_views),
      .level_chip = "Level 00 / Ground",
      .mode_chip = ToolLabel(active_tool),
      .right_chip = ThemeModeLabel(theme_mode),
      .footer = selection.valid ? SelectionLabel(selection) : status_message,
  };
}

} // namespace workspace_ui
