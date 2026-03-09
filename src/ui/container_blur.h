#pragma once

#include <imgui.h>

namespace workspace_ui {

enum class BlurContainerKind {
  Rail,
  Sidebar,
  Toolbar,
  Inspector,
  Dock,
  Bubble,
};

void PushBlurContainerWindowStyle();
void PopBlurContainerWindowStyle();

void DrawBlurContainerBackdrop(BlurContainerKind kind, bool dark_theme,
                               float scale, float rounding);

} // namespace workspace_ui
