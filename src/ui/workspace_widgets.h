#pragma once

#include <string>

#include <SDL3/SDL.h>
#include <imgui.h>

#include "ui/svg_icon_cache.h"
#include "ui/workspace_model.h"

namespace workspace_ui {

void PushFontIfAvailable(ImFont* font);
void PopFontIfAvailable(ImFont* font);

bool DrawToolButton(char const* label, bool active, ImVec2 size);
bool DrawChipButton(char const* label, bool active, ImVec2 size);
bool DrawGhostButton(char const* label, ImVec2 size);
bool DrawIconGhostButton(SvgIconCache& icons, char const* id, char const* icon_name,
                         char const* tooltip, ImVec2 size, float scale);
bool DrawIconRailButton(SvgIconCache& icons, char const* id, char const* icon_name,
                        char const* label, bool active, ImVec2 size, float scale);
bool DrawToolbarIconButton(SvgIconCache& icons, char const* id,
                           char const* icon_name, char const* tooltip,
                           bool active, ImVec2 size, float scale);
bool DrawToolbarChipButton(SvgIconCache& icons, char const* id,
                           char const* icon_name, char const* label, bool active,
                           ImVec2 size, float scale);
bool DrawIconActionTile(SvgIconCache& icons, char const* id, char const* icon_name,
                        char const* label, char const* caption, bool active,
                        ImVec2 size, float scale);
bool DrawDockIconButton(SvgIconCache& icons, char const* id, char const* icon_name,
                        char const* tooltip, bool active, ImVec2 size,
                        float scale);
bool DrawTogglePill(char const* id, bool* value, ImVec2 size);
bool DrawBrowserSectionHeader(SvgIconCache& icons, BrowserSection const& section,
                              bool expanded, float scale);
bool DrawBrowserItemRow(SvgIconCache& icons, BrowserItem const& item,
                        float scale);
void DrawViewportGizmo(SDL_FRect const& viewport_rect, float scale, bool dark_theme);
void DrawViewportOverlay(
    SDL_FRect const& viewport_rect,
    float scale,
    bool dark_theme,
    ViewportOverlayModel const& model
);
void DrawShortcutRow(char const* keys, char const* action);
void DrawSystemRow(char const* label, std::string const& value);

} // namespace workspace_ui
