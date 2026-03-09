#include "ui/workspace_widgets.h"

#include "style/ui_style.h"

#include <algorithm>
#include <cfloat>
#include <string>

namespace workspace_ui {
namespace {

struct ButtonFrame {
    ImVec2 min{};
    ImVec2 max{};
    bool pressed = false;
    bool hovered = false;
    bool held = false;
};

ImVec4 Mix(ImVec4 const& a, ImVec4 const& b, float t)
{
    return ImVec4(
        a.x + ((b.x - a.x) * t),
        a.y + ((b.y - a.y) * t),
        a.z + ((b.z - a.z) * t),
        a.w + ((b.w - a.w) * t)
    );
}

ImVec4 WithAlpha(ImVec4 color, float alpha)
{
    color.w = alpha;
    return color;
}

ImVec2 MeasureText(ImFont* font, float font_size, char const* text)
{
    if (text == nullptr || *text == '\0') {
        return ImVec2(0.0f, 0.0f);
    }

    if (font == nullptr) {
        return ImGui::CalcTextSize(text);
    }

    return font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, text);
}

void AddText(ImDrawList* draw_list, ImFont* font, float font_size, ImVec2 pos,
             ImU32 color, char const* text)
{
    if (text == nullptr || *text == '\0') {
        return;
    }

    if (font != nullptr) {
        draw_list->AddText(font, font_size, pos, color, text);
        return;
    }

    draw_list->AddText(pos, color, text);
}

ButtonFrame BeginButtonFrame(char const* id, ImVec2 size)
{
    ImGui::InvisibleButton(id, size);
    return ButtonFrame{
        ImGui::GetItemRectMin(),
        ImGui::GetItemRectMax(),
        ImGui::IsItemClicked(),
        ImGui::IsItemHovered(),
        ImGui::IsItemActive(),
    };
}

void DrawFrame(ImDrawList* draw_list, ButtonFrame const& frame, ImVec4 fill,
               ImVec4 border, float rounding, float thickness)
{
    draw_list->AddRectFilled(frame.min, frame.max,
                             ImGui::ColorConvertFloat4ToU32(fill), rounding);
    draw_list->AddRect(frame.min, frame.max,
                       ImGui::ColorConvertFloat4ToU32(border), rounding, 0,
                       thickness);
}

void DrawFittedIcon(ImDrawList* draw_list, IconTexture const& icon, ImVec2 min,
                    ImVec2 max, ImVec4 tint)
{
    if (!icon.IsValid()) {
        return;
    }

    const float width = max.x - min.x;
    const float height = max.y - min.y;
    const float icon_width = icon.size.x > 0.0f ? icon.size.x : 1.0f;
    const float icon_height = icon.size.y > 0.0f ? icon.size.y : 1.0f;
    const float scale = std::min(width / icon_width, height / icon_height);
    const ImVec2 draw_size(icon_width * scale, icon_height * scale);
    const ImVec2 draw_min(
        min.x + ((width - draw_size.x) * 0.5f),
        min.y + ((height - draw_size.y) * 0.5f)
    );
    const ImVec2 draw_max(draw_min.x + draw_size.x, draw_min.y + draw_size.y);
    draw_list->AddImage(icon.texture_id, draw_min, draw_max, ImVec2(0.0f, 0.0f),
                        ImVec2(1.0f, 1.0f),
                        ImGui::ColorConvertFloat4ToU32(tint));
}

ImVec4 AccentFill(bool hovered)
{
    ImGuiStyle const& style = ImGui::GetStyle();
    const ImVec4 base = style.Colors[ImGuiCol_ButtonActive];
    const ImVec4 hover = style.Colors[ImGuiCol_ButtonHovered];
    return hovered ? Mix(base, hover, 0.55f) : base;
}

float RelativeLuminance(ImVec4 const& color)
{
    return (0.2126f * color.x) + (0.7152f * color.y) + (0.0722f * color.z);
}

ImVec4 AccentContentColor()
{
    return RelativeLuminance(AccentFill(false)) > 0.60f
        ? ImVec4(0.08f, 0.10f, 0.13f, 1.0f)
        : ImVec4(0.99f, 0.98f, 0.96f, 1.0f);
}

ImVec4 HoveredContentColor(float t)
{
    ImGuiStyle const& style = ImGui::GetStyle();
    return Mix(style.Colors[ImGuiCol_Text], AccentContentColor(), t);
}

ImVec4 AccentEdgeColor(bool hovered)
{
    return Mix(AccentFill(hovered), AccentContentColor(), 0.30f);
}

void MaybeShowTooltip(char const* tooltip)
{
    if (tooltip != nullptr && *tooltip != '\0' && ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", tooltip);
    }
}

float TileRadius(float scale)
{
    return 10.0f * scale;
}

float ButtonRadius(float scale)
{
    return 8.0f * scale;
}

float MediumTextSize(float scale)
{
    return 10.4f * scale;
}

void DrawChevron(ImDrawList* draw_list, ImVec2 center, float size,
                 bool expanded, ImU32 color, float thickness)
{
    const float half = size * 0.5f;
    if (expanded) {
        draw_list->AddLine(ImVec2(center.x - half, center.y - (half * 0.4f)),
                           ImVec2(center.x, center.y + (half * 0.4f)), color,
                           thickness);
        draw_list->AddLine(ImVec2(center.x, center.y + (half * 0.4f)),
                           ImVec2(center.x + half, center.y - (half * 0.4f)),
                           color, thickness);
        return;
    }

    draw_list->AddLine(ImVec2(center.x - (half * 0.3f), center.y - half),
                       ImVec2(center.x + (half * 0.3f), center.y), color,
                       thickness);
    draw_list->AddLine(ImVec2(center.x + (half * 0.3f), center.y),
                       ImVec2(center.x - (half * 0.3f), center.y + half),
                       color, thickness);
}

bool BrowserSectionShowsAction(BrowserSection const& section)
{
    switch (section.id) {
    case BrowserSectionId::FloorPlans:
    case BrowserSectionId::Views3D:
    case BrowserSectionId::Sections:
        return true;
    default:
        return false;
    }
}

bool DrawSquareIconButton(SvgIconCache& icons, char const* id,
                          char const* icon_name, char const* label,
                          char const* tooltip, bool active, ImVec2 size,
                          float scale, bool show_label)
{
    ImGuiStyle const& style = ImGui::GetStyle();
    ButtonFrame const frame = BeginButtonFrame(id, size);
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec4 fill = active
        ? AccentFill(frame.hovered)
        : (frame.hovered
               ? WithAlpha(Mix(style.Colors[ImGuiCol_ButtonHovered],
                               style.Colors[ImGuiCol_WindowBg], 0.58f),
                           0.16f)
               : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImVec4 border = active
        ? WithAlpha(AccentEdgeColor(frame.hovered), 0.52f)
        : (frame.hovered
               ? WithAlpha(style.Colors[ImGuiCol_Border], 0.24f)
               : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    if (fill.w > 0.0f || border.w > 0.0f) {
        DrawFrame(draw_list, frame, fill, border, TileRadius(scale),
                  std::max(1.0f, 1.1f * scale));
    }

    IconTexture const icon =
        icons.Get(icon_name != nullptr ? icon_name : "", static_cast<int>(18.0f * scale));
    const ImVec4 icon_tint = active
        ? AccentContentColor()
        : (frame.hovered ? HoveredContentColor(0.18f)
                         : Mix(style.Colors[ImGuiCol_Text], style.Colors[ImGuiCol_TextDisabled], 0.18f));

    if (show_label) {
        const float icon_edge = 13.0f * scale;
        const float icon_left = frame.min.x + ((size.x - icon_edge) * 0.5f);
        const float icon_top = frame.min.y + (6.0f * scale);
        DrawFittedIcon(draw_list, icon,
                       ImVec2(icon_left, icon_top),
                       ImVec2(icon_left + icon_edge, icon_top + icon_edge),
                       icon_tint);

        ImFont* font = ui_style::LabelFont();
        const float font_size = MediumTextSize(scale);
        const ImVec2 text_size = MeasureText(font, font_size, label);
        const ImVec2 text_pos(
            frame.min.x + ((size.x - text_size.x) * 0.5f),
            frame.max.y - text_size.y - (6.0f * scale)
        );
        AddText(draw_list, font, font_size, text_pos,
                ImGui::ColorConvertFloat4ToU32(icon_tint), label);
    } else {
        const float icon_edge = std::clamp(
            std::min(size.x, size.y) - (16.0f * scale),
            14.0f * scale,
            20.0f * scale
        );
        const float icon_left = frame.min.x + ((size.x - icon_edge) * 0.5f);
        const float icon_top = frame.min.y + ((size.y - icon_edge) * 0.5f);
        DrawFittedIcon(draw_list, icon,
                       ImVec2(icon_left, icon_top),
                       ImVec2(icon_left + icon_edge, icon_top + icon_edge),
                       icon_tint);
    }

    MaybeShowTooltip(tooltip != nullptr ? tooltip : label);
    return frame.pressed;
}

bool DrawHorizontalButton(SvgIconCache& icons, char const* id, char const* icon_name,
                          char const* label, char const* tooltip, bool active,
                          ImVec2 size, float scale, bool compact)
{
    ImGuiStyle const& style = ImGui::GetStyle();
    ButtonFrame const frame = BeginButtonFrame(id, size);
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec4 fill = active
        ? AccentFill(frame.hovered)
        : (frame.hovered
               ? WithAlpha(Mix(style.Colors[ImGuiCol_ButtonHovered],
                               style.Colors[ImGuiCol_WindowBg], 0.60f),
                           0.16f)
               : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImVec4 border = active
        ? WithAlpha(AccentEdgeColor(frame.hovered), 0.46f)
        : (frame.hovered
               ? WithAlpha(style.Colors[ImGuiCol_Border], 0.22f)
               : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    if (fill.w > 0.0f || border.w > 0.0f) {
        DrawFrame(draw_list, frame, fill, border, ButtonRadius(scale),
                  std::max(1.0f, 1.0f * scale));
    }

    const float left_padding = compact ? (8.0f * scale) : (10.0f * scale);
    const float icon_size = compact ? (13.0f * scale) : (15.0f * scale);
    const float text_gap = compact ? (6.0f * scale) : (7.0f * scale);
    const ImVec4 content_color = active
        ? AccentContentColor()
        : (frame.hovered ? HoveredContentColor(0.18f)
                         : Mix(style.Colors[ImGuiCol_Text], style.Colors[ImGuiCol_TextDisabled], 0.12f));

    IconTexture const icon = icons.Get(icon_name != nullptr ? icon_name : "",
                                       static_cast<int>(icon_size + (4.0f * scale)));
    const ImVec2 icon_min(frame.min.x + left_padding,
                          frame.min.y + ((size.y - icon_size) * 0.5f));
    const ImVec2 icon_max(icon_min.x + icon_size, icon_min.y + icon_size);
    DrawFittedIcon(draw_list, icon, icon_min, icon_max, content_color);

    if (label != nullptr && *label != '\0') {
        ImFont* font = active ? ui_style::LabelFont() : ui_style::BodyFont();
        const float font_size = MediumTextSize(scale);
        const ImVec2 text_size = MeasureText(font, font_size, label);
        const ImVec2 text_pos(icon_max.x + text_gap,
                              frame.min.y + ((size.y - text_size.y) * 0.5f) - (0.5f * scale));
        AddText(draw_list, font, font_size, text_pos,
                ImGui::ColorConvertFloat4ToU32(content_color), label);
    }

    MaybeShowTooltip(tooltip != nullptr ? tooltip : label);
    return frame.pressed;
}

} // namespace

void PushFontIfAvailable(ImFont* font)
{
    if (font != nullptr) {
        ImGui::PushFont(font);
    }
}

void PopFontIfAvailable(ImFont* font)
{
    if (font != nullptr) {
        ImGui::PopFont();
    }
}

bool DrawToolButton(char const* label, bool active, ImVec2 size)
{
    PushFontIfAvailable(ui_style::LabelFont());
    if (active) {
        ui_style::PushActiveToolButtonStyle();
    }

    const bool pressed = ImGui::Button(label, size);

    if (active) {
        ui_style::PopActiveToolButtonStyle();
    }
    PopFontIfAvailable(ui_style::LabelFont());

    return pressed;
}

bool DrawChipButton(char const* label, bool active, ImVec2 size)
{
    ImGuiStyle const& style = ImGui::GetStyle();
    PushFontIfAvailable(ui_style::LabelFont());
    if (active) {
        ImGui::PushStyleColor(ImGuiCol_Button, AccentFill(false));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, AccentFill(true));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, AccentFill(false));
        ImGui::PushStyleColor(ImGuiCol_Text, AccentContentColor());
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button,
                              WithAlpha(Mix(style.Colors[ImGuiCol_Button],
                                            style.Colors[ImGuiCol_WindowBg], 0.18f),
                                        0.94f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                              WithAlpha(Mix(style.Colors[ImGuiCol_ButtonHovered],
                                            style.Colors[ImGuiCol_Button], 0.18f),
                                        0.98f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                              WithAlpha(style.Colors[ImGuiCol_ButtonHovered], 1.0f));
    }

    const bool pressed = ImGui::Button(label, size);
    ImGui::PopStyleColor(active ? 4 : 3);
    PopFontIfAvailable(ui_style::LabelFont());
    return pressed;
}

bool DrawGhostButton(char const* label, ImVec2 size)
{
    ImGuiStyle const& style = ImGui::GetStyle();
    PushFontIfAvailable(ui_style::LabelFont());
    ImGui::PushStyleColor(ImGuiCol_Button,
                          WithAlpha(Mix(style.Colors[ImGuiCol_Button],
                                        style.Colors[ImGuiCol_WindowBg], 0.30f),
                                    0.90f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                          WithAlpha(Mix(style.Colors[ImGuiCol_ButtonHovered],
                                        style.Colors[ImGuiCol_Button], 0.10f),
                                    0.98f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                          WithAlpha(style.Colors[ImGuiCol_ButtonHovered], 1.0f));
    const bool pressed = ImGui::Button(label, size);
    ImGui::PopStyleColor(3);
    PopFontIfAvailable(ui_style::LabelFont());
    return pressed;
}

bool DrawIconGhostButton(SvgIconCache& icons, char const* id, char const* icon_name,
                         char const* tooltip, ImVec2 size, float scale)
{
    return DrawSquareIconButton(icons, id, icon_name, "", tooltip, false, size,
                                scale, false);
}

bool DrawIconRailButton(SvgIconCache& icons, char const* id, char const* icon_name,
                        char const* label, bool active, ImVec2 size, float scale)
{
    return DrawSquareIconButton(icons, id, icon_name, label, label, active, size,
                                scale, true);
}

bool DrawToolbarIconButton(SvgIconCache& icons, char const* id,
                           char const* icon_name, char const* tooltip,
                           bool active, ImVec2 size, float scale)
{
    ImGuiStyle const& style = ImGui::GetStyle();
    ButtonFrame const frame = BeginButtonFrame(id, size);
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    const ImVec4 fill = active
        ? AccentFill(frame.hovered)
        : (frame.hovered
               ? WithAlpha(Mix(style.Colors[ImGuiCol_ButtonHovered],
                               style.Colors[ImGuiCol_WindowBg], 0.45f),
                           0.54f)
               : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    const ImVec4 border = active
        ? WithAlpha(AccentEdgeColor(frame.hovered), 0.30f)
        : (frame.hovered
               ? WithAlpha(style.Colors[ImGuiCol_Border], 0.42f)
               : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    if (fill.w > 0.0f || border.w > 0.0f) {
        DrawFrame(draw_list, frame, fill, border, ButtonRadius(scale),
                  std::max(1.0f, 1.0f * scale));
    }

    const float icon_edge = std::clamp(
        std::min(size.x, size.y) - (14.0f * scale),
        13.0f * scale,
        16.0f * scale
    );
    const float icon_left = frame.min.x + ((size.x - icon_edge) * 0.5f);
    const float icon_top = frame.min.y + ((size.y - icon_edge) * 0.5f);
    DrawFittedIcon(draw_list, icons.Get(icon_name != nullptr ? icon_name : "",
                                        static_cast<int>(16.0f * scale)),
                   ImVec2(icon_left, icon_top),
                   ImVec2(icon_left + icon_edge, icon_top + icon_edge),
                   active ? AccentContentColor()
                          : (frame.hovered
                                 ? HoveredContentColor(0.18f)
                                 : Mix(style.Colors[ImGuiCol_Text],
                                       style.Colors[ImGuiCol_TextDisabled], 0.08f)));
    MaybeShowTooltip(tooltip);
    return frame.pressed;
}

bool DrawToolbarChipButton(SvgIconCache& icons, char const* id,
                           char const* icon_name, char const* label, bool active,
                           ImVec2 size, float scale)
{
    ImGuiStyle const& style = ImGui::GetStyle();
    ButtonFrame const frame = BeginButtonFrame(id, size);
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    const ImVec4 fill = active
        ? WithAlpha(AccentFill(frame.hovered), frame.hovered ? 0.10f : 0.07f)
        : (frame.hovered
               ? WithAlpha(Mix(style.Colors[ImGuiCol_ButtonHovered],
                               style.Colors[ImGuiCol_WindowBg], 0.40f),
                           0.50f)
               : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    const ImVec4 border = active
        ? WithAlpha(AccentFill(false), 0.62f)
        : (frame.hovered
               ? WithAlpha(style.Colors[ImGuiCol_Border], 0.36f)
               : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    if (fill.w > 0.0f || border.w > 0.0f) {
        DrawFrame(draw_list, frame, fill, border, ButtonRadius(scale),
                  std::max(1.0f, 1.0f * scale));
    }

    const float left_padding = 9.0f * scale;
    const float icon_size = 13.0f * scale;
    const float text_gap = 6.0f * scale;
    const ImVec4 content_color = active
        ? AccentFill(frame.hovered)
        : (frame.hovered
               ? HoveredContentColor(0.18f)
               : Mix(style.Colors[ImGuiCol_Text], style.Colors[ImGuiCol_TextDisabled], 0.10f));

    IconTexture const icon = icons.Get(icon_name != nullptr ? icon_name : "",
                                       static_cast<int>(icon_size + (3.0f * scale)));
    const ImVec2 icon_min(frame.min.x + left_padding,
                          frame.min.y + ((size.y - icon_size) * 0.5f));
    const ImVec2 icon_max(icon_min.x + icon_size, icon_min.y + icon_size);
    DrawFittedIcon(draw_list, icon, icon_min, icon_max, content_color);

    if (label != nullptr && *label != '\0') {
        ImFont* font = active ? ui_style::LabelFont() : ui_style::BodyFont();
        const float font_size = MediumTextSize(scale);
        const ImVec2 text_size = MeasureText(font, font_size, label);
        const ImVec2 text_pos(icon_max.x + text_gap,
                              frame.min.y + ((size.y - text_size.y) * 0.5f) - (0.5f * scale));
        AddText(draw_list, font, font_size, text_pos,
                ImGui::ColorConvertFloat4ToU32(content_color), label);
    }

    MaybeShowTooltip(label);
    return frame.pressed;
}

bool DrawIconActionTile(SvgIconCache& icons, char const* id, char const* icon_name,
                        char const* label, char const* caption, bool active,
                        ImVec2 size, float scale)
{
    ImGuiStyle const& style = ImGui::GetStyle();
    ButtonFrame const frame = BeginButtonFrame(id, size);
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec4 fill = active
        ? WithAlpha(AccentFill(frame.hovered), 0.24f)
        : WithAlpha(Mix(style.Colors[ImGuiCol_Button], style.Colors[ImGuiCol_WindowBg],
                        frame.hovered ? 0.06f : 0.18f),
                    frame.hovered ? 0.94f : 0.88f);
    ImVec4 border = active
        ? WithAlpha(AccentFill(false), 0.85f)
        : WithAlpha(style.Colors[ImGuiCol_Border], frame.hovered ? 0.60f : 0.34f);
    DrawFrame(draw_list, frame, fill, border, TileRadius(scale),
              std::max(1.0f, 1.0f * scale));

    const ImVec4 icon_tint = active
        ? AccentFill(true)
        : Mix(style.Colors[ImGuiCol_Text], style.Colors[ImGuiCol_TextDisabled],
              frame.hovered ? 0.02f : 0.18f);
    DrawFittedIcon(draw_list, icons.Get(icon_name != nullptr ? icon_name : "",
                                        static_cast<int>(20.0f * scale)),
                   ImVec2(frame.min.x + (14.0f * scale), frame.min.y + (10.0f * scale)),
                   ImVec2(frame.max.x - (14.0f * scale), frame.min.y + (30.0f * scale)),
                   icon_tint);

    ImFont* label_font = ui_style::LabelFont();
    ImFont* caption_font = ui_style::CaptionFont();
    const float label_size = MediumTextSize(scale);
    const float caption_size = MediumTextSize(scale);
    const ImVec2 label_dims = MeasureText(label_font, label_size, label);
    const ImVec2 caption_dims = MeasureText(caption_font, caption_size, caption);
    const ImVec2 label_pos(frame.min.x + ((size.x - label_dims.x) * 0.5f),
                           frame.min.y + (33.0f * scale));
    const ImVec2 caption_pos(frame.min.x + ((size.x - caption_dims.x) * 0.5f),
                             frame.min.y + (47.0f * scale));
    AddText(draw_list, label_font, label_size, label_pos,
            ImGui::ColorConvertFloat4ToU32(Mix(style.Colors[ImGuiCol_Text],
                                               style.Colors[ImGuiCol_TextDisabled],
                                               active ? 0.0f : 0.08f)),
            label);
    AddText(draw_list, caption_font, caption_size, caption_pos,
            ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_TextDisabled]),
            caption);

    MaybeShowTooltip(label);
    return frame.pressed;
}

bool DrawDockIconButton(SvgIconCache& icons, char const* id, char const* icon_name,
                        char const* tooltip, bool active, ImVec2 size,
                        float scale)
{
    ImGuiStyle const& style = ImGui::GetStyle();
    ButtonFrame const frame = BeginButtonFrame(id, size);
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    const ImVec4 fill = active
        ? AccentFill(frame.hovered)
        : (frame.hovered
               ? WithAlpha(Mix(style.Colors[ImGuiCol_ButtonHovered],
                               style.Colors[ImGuiCol_WindowBg], 0.42f),
                           0.52f)
               : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    const ImVec4 border = active
        ? WithAlpha(AccentEdgeColor(frame.hovered), 0.30f)
        : (frame.hovered
               ? WithAlpha(style.Colors[ImGuiCol_Border], 0.38f)
               : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    if (fill.w > 0.0f || border.w > 0.0f) {
        DrawFrame(draw_list, frame, fill, border, ButtonRadius(scale),
                  std::max(1.0f, 1.0f * scale));
    }
    const float icon_edge = std::clamp(
        std::min(size.x, size.y) - (14.0f * scale),
        13.0f * scale,
        16.0f * scale
    );
    const float icon_left = frame.min.x + ((size.x - icon_edge) * 0.5f);
    const float icon_top = frame.min.y + ((size.y - icon_edge) * 0.5f);
    DrawFittedIcon(draw_list, icons.Get(icon_name != nullptr ? icon_name : "",
                                        static_cast<int>(16.0f * scale)),
                   ImVec2(icon_left, icon_top),
                   ImVec2(icon_left + icon_edge, icon_top + icon_edge),
                   active ? AccentContentColor()
                          : (frame.hovered
                                 ? HoveredContentColor(0.18f)
                                 : Mix(style.Colors[ImGuiCol_Text], style.Colors[ImGuiCol_TextDisabled], 0.08f)));
    MaybeShowTooltip(tooltip);
    return frame.pressed;
}

bool DrawTogglePill(char const* id, bool* value, ImVec2 size)
{
    ImGui::InvisibleButton(id, size);
    const bool clicked = ImGui::IsItemClicked();
    if (clicked) {
        *value = !*value;
    }

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImGuiStyle const& style = ImGui::GetStyle();
    const ImVec2 min = ImGui::GetItemRectMin();
    const ImVec2 max = ImGui::GetItemRectMax();
    const float radius = size.y * 0.5f;
    const ImVec4 off_track = WithAlpha(
        Mix(style.Colors[ImGuiCol_FrameBg], style.Colors[ImGuiCol_Border], 0.25f), 1.0f);
    const ImU32 track_color = ImGui::ColorConvertFloat4ToU32(
        *value ? AccentFill(ImGui::IsItemHovered()) : off_track);
    const ImU32 knob_color = ImGui::ColorConvertFloat4ToU32(
        Mix(style.Colors[ImGuiCol_Text], ImVec4(1.0f, 1.0f, 1.0f, 1.0f), 0.72f));
    draw_list->AddRectFilled(min, max, track_color, radius);

    const float knob_radius = radius - 3.0f;
    const float knob_x = *value ? (max.x - radius) : (min.x + radius);
    draw_list->AddCircleFilled(ImVec2(knob_x, min.y + radius), knob_radius,
                               knob_color);
    return clicked;
}

bool DrawBrowserSectionHeader(SvgIconCache& icons, BrowserSection const& section,
                              bool expanded, float scale)
{
    (void)icons;
    ImGuiStyle const& style = ImGui::GetStyle();
    const float width = std::max(1.0f, ImGui::GetContentRegionAvail().x);
    ButtonFrame const frame =
        BeginButtonFrame("##section", ImVec2(width, 28.0f * scale));
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    if (frame.hovered || frame.held) {
        draw_list->AddRectFilled(
            frame.min, frame.max,
            ImGui::ColorConvertFloat4ToU32(
                WithAlpha(Mix(style.Colors[ImGuiCol_HeaderHovered],
                              style.Colors[ImGuiCol_WindowBg], 0.40f),
                          0.44f)),
            6.0f * scale
        );
    }

    const ImU32 chevron_color = ImGui::ColorConvertFloat4ToU32(
        Mix(style.Colors[ImGuiCol_Text], style.Colors[ImGuiCol_TextDisabled],
            0.16f));
    DrawChevron(draw_list,
                ImVec2(frame.min.x + (10.0f * scale),
                       frame.min.y + (14.0f * scale)),
                7.0f * scale, expanded, chevron_color,
                std::max(1.0f, 1.15f * scale));

    ImFont* title_font = ui_style::LabelFont();
    AddText(draw_list, title_font, MediumTextSize(scale),
            ImVec2(frame.min.x + (22.0f * scale), frame.min.y + (8.0f * scale)),
            ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_Text]),
            section.title.c_str());

    if (BrowserSectionShowsAction(section)) {
        char const* marker = "+";
        const ImVec2 marker_size =
            MeasureText(title_font, MediumTextSize(scale), marker);
        AddText(draw_list, title_font, MediumTextSize(scale),
                ImVec2(frame.max.x - marker_size.x - (8.0f * scale),
                       frame.min.y + ((28.0f * scale - marker_size.y) * 0.5f)),
                ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_TextDisabled]),
                marker);
    }
    return frame.pressed;
}

bool DrawBrowserItemRow(SvgIconCache& icons, BrowserItem const& item, float scale)
{
    ImGuiStyle const& style = ImGui::GetStyle();
    const float width = std::max(1.0f, ImGui::GetContentRegionAvail().x);
    ButtonFrame const frame =
        BeginButtonFrame("##item", ImVec2(width, 28.0f * scale));
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec4 fill = item.selected
        ? WithAlpha(AccentFill(frame.hovered), 0.28f)
        : ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    ImVec4 border = item.selected
        ? WithAlpha(AccentEdgeColor(frame.hovered), 0.60f)
        : (frame.hovered
               ? WithAlpha(style.Colors[ImGuiCol_Border], 0.18f)
               : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    if (fill.w > 0.0f || border.w > 0.0f) {
        DrawFrame(draw_list, frame, fill, border, 6.0f * scale,
                  std::max(1.0f, 1.0f * scale));
    }

    ImVec4 icon_color = item.selected
                                    ? AccentContentColor()
                                    : (item.accent ? AccentFill(false)
                                    : Mix(style.Colors[ImGuiCol_Text],
                                          style.Colors[ImGuiCol_TextDisabled],
                                          0.08f));
    DrawFittedIcon(draw_list, icons.Get(item.icon_name,
                                        static_cast<int>(12.0f * scale)),
                   ImVec2(frame.min.x + (8.0f * scale), frame.min.y + (8.0f * scale)),
                   ImVec2(frame.min.x + (20.0f * scale), frame.min.y + (20.0f * scale)),
                   icon_color);

    AddText(draw_list, item.selected ? ui_style::LabelFont() : ui_style::BodyFont(),
            MediumTextSize(scale),
            ImVec2(frame.min.x + (28.0f * scale), frame.min.y + (8.0f * scale)),
            ImGui::ColorConvertFloat4ToU32(
                item.selected ? AccentContentColor()
                              : style.Colors[ImGuiCol_Text]),
            item.label.c_str());
    return frame.pressed;
}

void DrawViewportGizmo(SDL_FRect const& viewport_rect, float scale, bool dark_theme)
{
    if (viewport_rect.w <= 0.0f || viewport_rect.h <= 0.0f) {
        return;
    }

    ImDrawList* draw_list = ImGui::GetForegroundDrawList();
    const ImVec2 center(viewport_rect.x + viewport_rect.w - (78.0f * scale),
                        viewport_rect.y + (78.0f * scale));
    const ImVec2 z_tip(center.x, center.y - (34.0f * scale));
    const ImVec2 x_tip(center.x + (34.0f * scale), center.y - (6.0f * scale));
    const ImVec2 y_tip(center.x - (28.0f * scale), center.y - (10.0f * scale));
    const ImU32 ring_color =
        dark_theme ? IM_COL32(23, 26, 31, 220) : IM_COL32(255, 255, 255, 210);

    draw_list->AddCircleFilled(center, 34.0f * scale, ring_color);
    draw_list->AddLine(center, z_tip, IM_COL32(52, 121, 255, 255), 3.0f * scale);
    draw_list->AddLine(center, x_tip, IM_COL32(217, 128, 64, 255), 3.0f * scale);
    draw_list->AddLine(center, y_tip, IM_COL32(130, 220, 36, 255), 3.0f * scale);
    draw_list->AddCircleFilled(center, 6.0f * scale, IM_COL32(244, 245, 247, 220));
    draw_list->AddCircleFilled(z_tip, 9.0f * scale, IM_COL32(52, 121, 255, 255));
    draw_list->AddCircleFilled(x_tip, 9.0f * scale, IM_COL32(217, 128, 64, 255));
    draw_list->AddCircleFilled(y_tip, 9.0f * scale, IM_COL32(130, 220, 36, 255));
    draw_list->AddText(ImVec2(z_tip.x - (4.0f * scale), z_tip.y - (6.0f * scale)),
                       IM_COL32(240, 244, 248, 255), "Z");
    draw_list->AddText(ImVec2(x_tip.x - (4.0f * scale), x_tip.y - (6.0f * scale)),
                       IM_COL32(32, 34, 37, 255), "X");
    draw_list->AddText(ImVec2(y_tip.x - (4.0f * scale), y_tip.y - (6.0f * scale)),
                       IM_COL32(32, 34, 37, 255), "Y");
}

void DrawViewportOverlay(SDL_FRect const& viewport_rect, float scale,
                         bool dark_theme, ViewportOverlayModel const& model)
{
    if (viewport_rect.w <= 0.0f || viewport_rect.h <= 0.0f) {
        return;
    }

    ImDrawList* draw_list = ImGui::GetForegroundDrawList();
    const ImU32 border_color =
        dark_theme ? IM_COL32(70, 76, 88, 220) : IM_COL32(188, 194, 205, 220);
    const ImU32 panel_bg =
        dark_theme ? IM_COL32(10, 12, 16, 210) : IM_COL32(247, 247, 249, 210);
    const ImU32 chip_bg = IM_COL32(217, 128, 64, 235);
    const ImU32 text_color =
        dark_theme ? IM_COL32(244, 245, 247, 255) : IM_COL32(31, 35, 39, 255);
    const ImU32 subtext_color =
        dark_theme ? IM_COL32(163, 170, 180, 255) : IM_COL32(98, 106, 118, 255);
    const float rounding = 18.0f * scale;
    const ImVec2 min(viewport_rect.x, viewport_rect.y);
    const ImVec2 max(viewport_rect.x + viewport_rect.w,
                     viewport_rect.y + viewport_rect.h);

    draw_list->AddRect(min, max, border_color, rounding, 0,
                       std::max(1.0f, 1.25f * scale));

    const ImVec2 header_min(viewport_rect.x + (18.0f * scale),
                            viewport_rect.y + (18.0f * scale));
    const ImVec2 header_max(header_min.x + (260.0f * scale),
                            header_min.y + (64.0f * scale));
    draw_list->AddRectFilled(header_min, header_max, panel_bg, 16.0f * scale);
    AddText(draw_list, ui_style::HeadingFont(), MediumTextSize(scale),
            ImVec2(header_min.x + (14.0f * scale),
                   header_min.y + (9.0f * scale)),
            text_color, model.title.c_str());
    AddText(draw_list, ui_style::CaptionFont(), MediumTextSize(scale),
            ImVec2(header_min.x + (14.0f * scale),
                   header_min.y + (33.0f * scale)),
            subtext_color, model.subtitle.c_str());

    const auto draw_chip = [&](ImVec2 pos, std::string const& label, ImU32 fill,
                               ImU32 label_color) {
        const ImVec2 text_size = MeasureText(ui_style::LabelFont(), MediumTextSize(scale),
                                             label.c_str());
        const ImVec2 chip_max(pos.x + text_size.x + (22.0f * scale),
                              pos.y + (26.0f * scale));
        draw_list->AddRectFilled(pos, chip_max, fill, 13.0f * scale);
        AddText(draw_list, ui_style::LabelFont(), MediumTextSize(scale),
                ImVec2(pos.x + (11.0f * scale), pos.y + (6.0f * scale)),
                label_color, label.c_str());
        return chip_max.x;
    };

    float chip_x = header_min.x;
    const float chip_y = header_max.y + (12.0f * scale);
    chip_x = draw_chip(ImVec2(chip_x, chip_y), model.level_chip, chip_bg,
                       IM_COL32(249, 247, 244, 255)) +
             (8.0f * scale);
    chip_x = draw_chip(
                 ImVec2(chip_x, chip_y), model.mode_chip,
                 dark_theme ? IM_COL32(21, 24, 30, 224)
                            : IM_COL32(231, 236, 241, 236),
                 text_color) +
             (8.0f * scale);
    draw_chip(ImVec2(chip_x, chip_y), model.right_chip,
              dark_theme ? IM_COL32(21, 24, 30, 224)
                         : IM_COL32(231, 236, 241, 236),
              text_color);

    const ImVec2 footer_min(viewport_rect.x + (18.0f * scale),
                            viewport_rect.y + viewport_rect.h -
                                (54.0f * scale));
    const ImVec2 footer_max(
        std::min(viewport_rect.x + viewport_rect.w - (18.0f * scale),
                 footer_min.x + (320.0f * scale)),
        viewport_rect.y + viewport_rect.h - (18.0f * scale));
    draw_list->AddRectFilled(footer_min, footer_max, panel_bg, 14.0f * scale);
    AddText(draw_list, ui_style::CaptionFont(), MediumTextSize(scale),
            ImVec2(footer_min.x + (12.0f * scale),
                   footer_min.y + (10.0f * scale)),
            subtext_color, model.footer.c_str());
}

void DrawShortcutRow(char const* keys, char const* action)
{
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(keys);
    ImGui::TableNextColumn();
    ImGui::TextWrapped("%s", action);
}

void DrawSystemRow(char const* label, std::string const& value)
{
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::TextDisabled("%s", label);
    ImGui::TableNextColumn();
    ImGui::TextWrapped("%s", value.c_str());
}

} // namespace workspace_ui
