#include "ui/container_blur.h"

#include <algorithm>

namespace workspace_ui {
namespace {

struct BlurContainerPalette {
  ImVec4 shadow{};
  ImVec4 outer_glow{};
  ImVec4 fill_base{};
  ImVec4 fill_top{};
  ImVec4 fill_bottom{};
  ImVec4 border{};
  ImVec4 highlight{};
  float blur_radius = 20.0f;
  float shadow_offset = 6.0f;
  int blur_layers = 6;
};

ImVec4 WithAlpha(ImVec4 color, float alpha) {
  color.w = alpha;
  return color;
}

ImVec4 Mix(ImVec4 const &a, ImVec4 const &b, float t) {
  return ImVec4(a.x + ((b.x - a.x) * t), a.y + ((b.y - a.y) * t),
                a.z + ((b.z - a.z) * t), a.w + ((b.w - a.w) * t));
}

BlurContainerPalette PaletteForKind(BlurContainerKind kind, bool dark_theme,
                                    float scale) {
  const float scaled = std::max(0.5f, scale);

  if (!dark_theme) {
    BlurContainerPalette palette{
        .shadow = ImVec4(0.0f, 0.0f, 0.0f, 0.0f),
        .outer_glow = ImVec4(1.0f, 1.0f, 1.0f, 0.035f),
        .fill_base = ImVec4(0.98f, 0.985f, 0.99f, 0.62f),
        .fill_top = ImVec4(1.0f, 1.0f, 1.0f, 0.16f),
        .fill_bottom = ImVec4(0.86f, 0.90f, 0.95f, 0.10f),
        .border = ImVec4(0.82f, 0.86f, 0.90f, 0.52f),
        .highlight = ImVec4(1.0f, 1.0f, 1.0f, 0.22f),
        .blur_radius = 10.0f * scaled,
        .shadow_offset = 0.0f,
        .blur_layers = 3,
    };
    return palette;
  }

  BlurContainerPalette palette{
      .shadow = ImVec4(0.00f, 0.00f, 0.00f, 0.0f),
      .outer_glow = ImVec4(0.34f, 0.44f, 0.54f, 0.045f),
      .fill_base = ImVec4(0.06f, 0.08f, 0.11f, 0.74f),
      .fill_top = ImVec4(0.22f, 0.28f, 0.35f, 0.11f),
      .fill_bottom = ImVec4(0.01f, 0.02f, 0.03f, 0.16f),
      .border = ImVec4(0.30f, 0.38f, 0.46f, 0.34f),
      .highlight = ImVec4(0.90f, 0.95f, 1.0f, 0.08f),
      .blur_radius = 12.0f * scaled,
      .shadow_offset = 0.0f,
      .blur_layers = 3,
  };

  switch (kind) {
  case BlurContainerKind::Rail:
    palette.fill_base = ImVec4(0.02f, 0.03f, 0.05f, 0.84f);
    palette.fill_top = ImVec4(0.18f, 0.23f, 0.30f, 0.08f);
    palette.fill_bottom = ImVec4(0.00f, 0.00f, 0.00f, 0.18f);
    palette.blur_radius = 9.0f * scaled;
    break;
  case BlurContainerKind::Sidebar:
    palette.fill_base = ImVec4(0.05f, 0.07f, 0.10f, 0.78f);
    palette.fill_top = ImVec4(0.22f, 0.29f, 0.36f, 0.10f);
    palette.fill_bottom = ImVec4(0.00f, 0.00f, 0.00f, 0.16f);
    palette.blur_radius = 10.0f * scaled;
    break;
  case BlurContainerKind::Toolbar:
    palette.fill_base = ImVec4(0.10f, 0.13f, 0.18f, 0.72f);
    palette.fill_top = ImVec4(0.30f, 0.38f, 0.46f, 0.14f);
    palette.fill_bottom = ImVec4(0.00f, 0.00f, 0.00f, 0.12f);
    palette.outer_glow = ImVec4(0.45f, 0.56f, 0.68f, 0.05f);
    palette.blur_radius = 10.0f * scaled;
    break;
  case BlurContainerKind::Inspector:
    palette.fill_base = ImVec4(0.05f, 0.07f, 0.10f, 0.80f);
    palette.fill_top = ImVec4(0.20f, 0.26f, 0.33f, 0.10f);
    palette.fill_bottom = ImVec4(0.00f, 0.00f, 0.00f, 0.16f);
    palette.blur_radius = 10.0f * scaled;
    break;
  case BlurContainerKind::Dock:
    palette.fill_base = ImVec4(0.08f, 0.10f, 0.13f, 0.76f);
    palette.fill_top = ImVec4(0.25f, 0.31f, 0.38f, 0.10f);
    palette.fill_bottom = ImVec4(0.00f, 0.00f, 0.00f, 0.12f);
    palette.blur_radius = 9.0f * scaled;
    break;
  case BlurContainerKind::Bubble:
    palette.fill_base = ImVec4(0.08f, 0.10f, 0.13f, 0.80f);
    palette.fill_top = ImVec4(0.26f, 0.33f, 0.40f, 0.10f);
    palette.fill_bottom = ImVec4(0.00f, 0.00f, 0.00f, 0.12f);
    palette.blur_radius = 8.0f * scaled;
    break;
  default:
    break;
  }

  return palette;
}

void DrawSoftBlur(ImDrawList *draw_list, ImVec2 min, ImVec2 max,
                  BlurContainerPalette const &palette, float rounding) {
  for (int layer = palette.blur_layers; layer >= 1; --layer) {
    const float t = static_cast<float>(layer) /
                    static_cast<float>(std::max(1, palette.blur_layers));
    const float expand = palette.blur_radius * t;
    const float glow_alpha = palette.outer_glow.w * (t * t);
    const float shadow_alpha = palette.shadow.w * (t * t * 0.9f);

    const ImVec2 glow_min(min.x - expand, min.y - expand);
    const ImVec2 glow_max(max.x + expand, max.y + expand);
    draw_list->AddRectFilled(
        glow_min, glow_max,
        ImGui::ColorConvertFloat4ToU32(WithAlpha(palette.outer_glow, glow_alpha)),
        rounding + expand);

    if (shadow_alpha > 0.001f) {
      const ImVec2 shadow_min(min.x - expand,
                              min.y - expand + (palette.shadow_offset * t));
      const ImVec2 shadow_max(max.x + expand,
                              max.y + expand + (palette.shadow_offset * t));
      draw_list->AddRectFilled(
          shadow_min, shadow_max,
          ImGui::ColorConvertFloat4ToU32(
              WithAlpha(palette.shadow, shadow_alpha)),
          rounding + expand);
    }
  }
}

} // namespace

void PushBlurContainerWindowStyle() {
  ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
  ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
}

void PopBlurContainerWindowStyle() { ImGui::PopStyleColor(2); }

void DrawBlurContainerBackdrop(BlurContainerKind kind, bool dark_theme,
                               float scale, float rounding) {
  ImDrawList *draw_list = ImGui::GetBackgroundDrawList();
  const ImVec2 min = ImGui::GetWindowPos();
  const ImVec2 max(min.x + ImGui::GetWindowSize().x,
                   min.y + ImGui::GetWindowSize().y);
  const BlurContainerPalette palette = PaletteForKind(kind, dark_theme, scale);

  DrawSoftBlur(draw_list, min, max, palette, rounding);

  draw_list->AddRectFilled(
      min, max, ImGui::ColorConvertFloat4ToU32(palette.fill_base), rounding);

  const float height = max.y - min.y;
  const float top_band = std::clamp(height * 0.42f, 18.0f * scale, 56.0f * scale);
  const float bottom_band =
      std::clamp(height * 0.28f, 14.0f * scale, 42.0f * scale);

  draw_list->AddRectFilled(
      min, ImVec2(max.x, min.y + top_band),
      ImGui::ColorConvertFloat4ToU32(palette.fill_top), rounding,
      ImDrawFlags_RoundCornersTop);
  draw_list->AddRectFilled(
      ImVec2(min.x, max.y - bottom_band), max,
      ImGui::ColorConvertFloat4ToU32(palette.fill_bottom), rounding,
      ImDrawFlags_RoundCornersBottom);

  const ImVec4 inner_line = Mix(palette.border, palette.highlight, 0.35f);
  draw_list->AddRect(
      min, max, ImGui::ColorConvertFloat4ToU32(palette.border), rounding, 0,
      std::max(1.0f, 1.0f * scale));
  draw_list->AddLine(
      ImVec2(min.x + (10.0f * scale), min.y + (1.0f * scale)),
      ImVec2(max.x - (10.0f * scale), min.y + (1.0f * scale)),
      ImGui::ColorConvertFloat4ToU32(palette.highlight),
      std::max(1.0f, 1.0f * scale));
  draw_list->AddLine(
      ImVec2(min.x + (10.0f * scale), max.y - (1.0f * scale)),
      ImVec2(max.x - (10.0f * scale), max.y - (1.0f * scale)),
      ImGui::ColorConvertFloat4ToU32(WithAlpha(inner_line, inner_line.w * 0.55f)),
      std::max(1.0f, 1.0f * scale));
}

} // namespace workspace_ui
