#pragma once

enum class ThemeMode {
    System,
    Light,
    Dark,
};

enum class ToolKind {
    Select,
    Wall,
    Floor,
    Ceiling,
    Roof,
    Column,
    Beam,
    Door,
    Window,
    Line,
    Arc,
    Move,
    Rotate,
    Orbit,
    Pan,
    Zoom,
    Paint,
    Measure,
    Section,
    Copy,
};

[[nodiscard]] const char* ThemeModeLabel(ThemeMode mode);
[[nodiscard]] const char* ToolLabel(ToolKind tool);
[[nodiscard]] const char* ToolButtonLabel(ToolKind tool);
[[nodiscard]] const char* ToolIconName(ToolKind tool);
[[nodiscard]] const char* ToolHintText(ToolKind tool);
