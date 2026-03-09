#pragma once

#include <array>
#include <vector>

constexpr float kWorldSkyDomeRadius = 1200.0f;

struct WorldVertex {
    float position[3];
    float color[3];
};

struct VertexRange {
    int first = 0;
    int count = 0;
};

struct WorldVisualStyle {
    std::array<float, 4> sky_color{};
    std::array<float, 4> sky_top_color{};
    std::array<float, 4> ground_color{};
    std::array<float, 3> grid_minor{};
    std::array<float, 3> grid_major{};
    std::array<float, 3> axis_x{};
    std::array<float, 3> axis_y{};
    std::array<float, 3> axis_z{};
    std::array<float, 3> guide{};
    std::array<float, 3> shell_primary{};
    std::array<float, 3> shell_secondary{};
    std::array<float, 3> glass{};
    std::array<float, 3> accent{};
    std::array<float, 3> section{};
    float horizon_ratio = 0.66f;
};

struct WorldSceneStats {
    int levels = 0;
    int slabs = 0;
    int walls = 0;
    int columns = 0;
    int openings = 0;
    int section_cuts = 0;
    int saved_views = 0;
};

struct WorldScene {
    std::vector<WorldVertex> vertices;
    VertexRange massing_mesh{};
    VertexRange glass_mesh{};
    VertexRange floor_outlines{};
    VertexRange grid{};
    VertexRange section_lines{};
    VertexRange axes{};
    VertexRange guide_cube{};
    WorldSceneStats stats{};
};

WorldVisualStyle BuildWorldVisualStyle(bool dark_theme);
WorldScene CreateSketchWorld(WorldVisualStyle const& style);
