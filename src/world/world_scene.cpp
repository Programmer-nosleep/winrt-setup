#include "world/world_scene.h"

#include <array>

#include "utils/math3d.h"

namespace {

struct Point3 {
    float x;
    float y;
    float z;
};

void PushVertex(
    std::vector<WorldVertex>& vertices,
    Point3 const& point,
    std::array<float, 3> const& color
)
{
    vertices.push_back(WorldVertex{{point.x, point.y, point.z}, {color[0], color[1], color[2]}});
}

void PushTriangle(
    std::vector<WorldVertex>& vertices,
    Point3 const& a,
    Point3 const& b,
    Point3 const& c,
    std::array<float, 3> const& color
)
{
    PushVertex(vertices, a, color);
    PushVertex(vertices, b, color);
    PushVertex(vertices, c, color);
}

void AddQuad(
    std::vector<WorldVertex>& vertices,
    Point3 const& a,
    Point3 const& b,
    Point3 const& c,
    Point3 const& d,
    std::array<float, 3> const& color
)
{
    PushTriangle(vertices, a, b, c, color);
    PushTriangle(vertices, a, c, d, color);
}

void AddBox(
    std::vector<WorldVertex>& vertices,
    Point3 min,
    Point3 max,
    std::array<float, 3> const& color
)
{
    const Point3 p000{min.x, min.y, min.z};
    const Point3 p001{min.x, min.y, max.z};
    const Point3 p010{min.x, max.y, min.z};
    const Point3 p011{min.x, max.y, max.z};
    const Point3 p100{max.x, min.y, min.z};
    const Point3 p101{max.x, min.y, max.z};
    const Point3 p110{max.x, max.y, min.z};
    const Point3 p111{max.x, max.y, max.z};

    AddQuad(vertices, p001, p101, p111, p011, color);
    AddQuad(vertices, p000, p010, p110, p100, color);
    AddQuad(vertices, p000, p001, p011, p010, color);
    AddQuad(vertices, p100, p110, p111, p101, color);
    AddQuad(vertices, p010, p011, p111, p110, color);
    AddQuad(vertices, p000, p100, p101, p001, color);
}

void PushLine(
    std::vector<WorldVertex>& vertices,
    float ax,
    float ay,
    float az,
    float bx,
    float by,
    float bz,
    float r,
    float g,
    float b
)
{
    vertices.push_back(WorldVertex{{ax, ay, az}, {r, g, b}});
    vertices.push_back(WorldVertex{{bx, by, bz}, {r, g, b}});
}

void PushDashedLine(
    std::vector<WorldVertex>& vertices,
    Point3 const& start,
    Point3 const& end,
    std::array<float, 3> const& color,
    float dash_length,
    float gap_length
)
{
    const Vec3 from{start.x, start.y, start.z};
    const Vec3 to{end.x, end.y, end.z};
    const Vec3 delta = to - from;
    const float total_length = Length(delta);
    if (total_length <= 0.0001f) {
        return;
    }

    const Vec3 direction = delta / total_length;
    const float stride = std::max(0.05f, dash_length + gap_length);
    for (float distance = 0.0f; distance < total_length; distance += stride) {
        const float segment_end = std::min(total_length, distance + dash_length);
        const Vec3 a = from + (direction * distance);
        const Vec3 b = from + (direction * segment_end);
        PushLine(vertices, a.x, a.y, a.z, b.x, b.y, b.z,
                 color[0], color[1], color[2]);
    }
}

VertexRange AddGrid(std::vector<WorldVertex>& vertices, WorldVisualStyle const& style)
{
    constexpr int kGridExtent = 36;
    const int start = static_cast<int>(vertices.size());

    for (int index = -kGridExtent; index <= kGridExtent; ++index) {
        const bool major_line = (index % 6) == 0;
        const auto& color = major_line ? style.grid_major : style.grid_minor;
        PushLine(
            vertices,
            static_cast<float>(index), -static_cast<float>(kGridExtent), -0.01f,
            static_cast<float>(index), static_cast<float>(kGridExtent), -0.01f,
            color[0], color[1], color[2]
        );
        PushLine(
            vertices,
            -static_cast<float>(kGridExtent), static_cast<float>(index), -0.01f,
            static_cast<float>(kGridExtent), static_cast<float>(index), -0.01f,
            color[0], color[1], color[2]
        );
    }

    return VertexRange{
        .first = start,
        .count = static_cast<int>(vertices.size()) - start,
    };
}

VertexRange AddMassing(
    std::vector<WorldVertex>& vertices,
    WorldVisualStyle const& style,
    WorldSceneStats& stats
)
{
    const int start = static_cast<int>(vertices.size());

    AddBox(vertices, Point3{-10.5f, -7.5f, -0.15f}, Point3{10.5f, 7.5f, 0.18f}, style.shell_secondary);
    AddBox(vertices, Point3{-8.5f, -5.5f, 3.20f}, Point3{8.5f, 5.5f, 3.44f}, style.shell_secondary);
    AddBox(vertices, Point3{-9.0f, -6.0f, 6.40f}, Point3{9.0f, 6.0f, 6.62f}, style.shell_primary);

    AddBox(vertices, Point3{-10.5f, -7.5f, 0.18f}, Point3{-10.15f, 7.5f, 3.20f}, style.shell_primary);
    AddBox(vertices, Point3{10.15f, -7.5f, 0.18f}, Point3{10.5f, 7.5f, 3.20f}, style.shell_primary);
    AddBox(vertices, Point3{-10.15f, -7.5f, 0.18f}, Point3{10.15f, -7.15f, 3.20f}, style.shell_primary);
    AddBox(vertices, Point3{-10.15f, 7.15f, 0.18f}, Point3{10.15f, 7.5f, 3.20f}, style.shell_primary);
    AddBox(vertices, Point3{-1.10f, -7.15f, 0.18f}, Point3{-0.75f, 4.25f, 3.20f}, style.shell_primary);
    AddBox(vertices, Point3{0.75f, -4.25f, 0.18f}, Point3{1.10f, 7.15f, 3.20f}, style.shell_primary);
    AddBox(vertices, Point3{-8.5f, -5.5f, 3.44f}, Point3{-8.15f, 5.5f, 6.40f}, style.shell_primary);
    AddBox(vertices, Point3{8.15f, -5.5f, 3.44f}, Point3{8.5f, 5.5f, 6.40f}, style.shell_primary);
    AddBox(vertices, Point3{-8.15f, -5.5f, 3.44f}, Point3{8.15f, -5.15f, 6.40f}, style.shell_primary);
    AddBox(vertices, Point3{-8.15f, 5.15f, 3.44f}, Point3{8.15f, 5.5f, 6.40f}, style.shell_primary);
    AddBox(vertices, Point3{-2.5f, -1.0f, 3.44f}, Point3{2.5f, 1.0f, 6.40f}, style.shell_primary);

    const std::array<Point3, 8> column_mins = {
        Point3{-7.50f, -4.60f, 0.18f}, Point3{-2.40f, -4.60f, 0.18f},
        Point3{2.40f, -4.60f, 0.18f}, Point3{7.10f, -4.60f, 0.18f},
        Point3{-7.50f, 4.00f, 0.18f}, Point3{-2.40f, 4.00f, 0.18f},
        Point3{2.40f, 4.00f, 0.18f}, Point3{7.10f, 4.00f, 0.18f},
    };
    for (Point3 const& min : column_mins) {
        AddBox(vertices, min, Point3{min.x + 0.40f, min.y + 0.40f, 6.15f}, style.accent);
    }

    stats.levels = 2;
    stats.slabs = 3;
    stats.walls = 10;
    stats.columns = static_cast<int>(column_mins.size());
    stats.openings = 10;
    stats.section_cuts = 2;
    stats.saved_views = 6;

    return VertexRange{
        .first = start,
        .count = static_cast<int>(vertices.size()) - start,
    };
}

VertexRange AddGlass(std::vector<WorldVertex>& vertices, WorldVisualStyle const& style)
{
    const int start = static_cast<int>(vertices.size());
    AddBox(vertices, Point3{-9.80f, -7.20f, 0.90f}, Point3{-9.45f, -3.20f, 2.55f}, style.glass);
    AddBox(vertices, Point3{9.45f, 3.20f, 0.90f}, Point3{9.80f, 7.20f, 2.55f}, style.glass);
    AddBox(vertices, Point3{-8.00f, 5.18f, 4.10f}, Point3{8.00f, 5.52f, 5.65f}, style.glass);
    AddBox(vertices, Point3{-8.00f, -5.52f, 4.10f}, Point3{8.00f, -5.18f, 5.65f}, style.glass);
    return VertexRange{
        .first = start,
        .count = static_cast<int>(vertices.size()) - start,
    };
}

VertexRange AddFloorOutlines(std::vector<WorldVertex>& vertices, WorldVisualStyle const& style)
{
    const int start = static_cast<int>(vertices.size());

    const auto draw_rect = [&](float min_x, float min_y, float max_x, float max_y, float z, std::array<float, 3> const& color) {
        PushLine(vertices, min_x, min_y, z, max_x, min_y, z, color[0], color[1], color[2]);
        PushLine(vertices, max_x, min_y, z, max_x, max_y, z, color[0], color[1], color[2]);
        PushLine(vertices, max_x, max_y, z, min_x, max_y, z, color[0], color[1], color[2]);
        PushLine(vertices, min_x, max_y, z, min_x, min_y, z, color[0], color[1], color[2]);
    };

    draw_rect(-10.5f, -7.5f, 10.5f, 7.5f, 0.20f, style.accent);
    draw_rect(-8.5f, -5.5f, 8.5f, 5.5f, 3.46f, style.section);
    draw_rect(-9.0f, -6.0f, 9.0f, 6.0f, 6.64f, style.section);
    draw_rect(-2.5f, -1.0f, 2.5f, 1.0f, 3.46f, style.accent);

    return VertexRange{
        .first = start,
        .count = static_cast<int>(vertices.size()) - start,
    };
}

VertexRange AddSectionLines(std::vector<WorldVertex>& vertices, WorldVisualStyle const& style)
{
    const int start = static_cast<int>(vertices.size());

    PushLine(vertices, 0.0f, -8.5f, 0.0f, 0.0f, 8.5f, 0.0f, style.section[0], style.section[1], style.section[2]);
    PushLine(vertices, 0.0f, -8.5f, 6.8f, 0.0f, 8.5f, 6.8f, style.section[0], style.section[1], style.section[2]);
    PushLine(vertices, -9.5f, 0.0f, 0.0f, 9.5f, 0.0f, 0.0f, style.accent[0], style.accent[1], style.accent[2]);
    PushLine(vertices, -9.5f, 0.0f, 6.8f, 9.5f, 0.0f, 6.8f, style.accent[0], style.accent[1], style.accent[2]);
    return VertexRange{
        .first = start,
        .count = static_cast<int>(vertices.size()) - start,
    };
}

VertexRange AddAxes(std::vector<WorldVertex>& vertices, WorldVisualStyle const& style)
{
    const int start = static_cast<int>(vertices.size());
    constexpr float kPositiveLength = 12.0f;
    constexpr float kNegativeLength = 12.0f;
    constexpr float kDashLength = 0.5f;
    constexpr float kGapLength = 0.3f;

    PushLine(vertices, 0.0f, 0.0f, 0.0f, kPositiveLength, 0.0f, 0.0f,
             style.axis_x[0], style.axis_x[1], style.axis_x[2]);
    PushLine(vertices, 0.0f, 0.0f, 0.0f, 0.0f, kPositiveLength, 0.0f,
             style.axis_y[0], style.axis_y[1], style.axis_y[2]);
    PushLine(vertices, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 10.0f,
             style.axis_z[0], style.axis_z[1], style.axis_z[2]);

    PushDashedLine(vertices, Point3{0.0f, 0.0f, 0.0f},
                   Point3{-kNegativeLength, 0.0f, 0.0f}, style.axis_x,
                   kDashLength, kGapLength);
    PushDashedLine(vertices, Point3{0.0f, 0.0f, 0.0f},
                   Point3{0.0f, -kNegativeLength, 0.0f}, style.axis_y,
                   kDashLength, kGapLength);
    PushDashedLine(vertices, Point3{0.0f, 0.0f, 0.0f},
                   Point3{0.0f, 0.0f, -10.0f}, style.axis_z,
                   kDashLength, kGapLength);
    return VertexRange{
        .first = start,
        .count = static_cast<int>(vertices.size()) - start,
    };
}

VertexRange AddGuideCube(std::vector<WorldVertex>& vertices, WorldVisualStyle const& style)
{
    constexpr float min_x = -11.2f;
    constexpr float max_x = 11.2f;
    constexpr float min_y = -8.2f;
    constexpr float max_y = 8.2f;
    constexpr float top = 6.9f;
    const int start = static_cast<int>(vertices.size());

    PushLine(vertices, min_x, min_y, 0.0f, max_x, min_y, 0.0f, style.guide[0], style.guide[1], style.guide[2]);
    PushLine(vertices, max_x, min_y, 0.0f, max_x, max_y, 0.0f, style.guide[0], style.guide[1], style.guide[2]);
    PushLine(vertices, max_x, max_y, 0.0f, min_x, max_y, 0.0f, style.guide[0], style.guide[1], style.guide[2]);
    PushLine(vertices, min_x, max_y, 0.0f, min_x, min_y, 0.0f, style.guide[0], style.guide[1], style.guide[2]);

    PushLine(vertices, min_x, min_y, top, max_x, min_y, top, style.guide[0], style.guide[1], style.guide[2]);
    PushLine(vertices, max_x, min_y, top, max_x, max_y, top, style.guide[0], style.guide[1], style.guide[2]);
    PushLine(vertices, max_x, max_y, top, min_x, max_y, top, style.guide[0], style.guide[1], style.guide[2]);
    PushLine(vertices, min_x, max_y, top, min_x, min_y, top, style.guide[0], style.guide[1], style.guide[2]);

    PushLine(vertices, min_x, min_y, 0.0f, min_x, min_y, top, style.guide[0], style.guide[1], style.guide[2]);
    PushLine(vertices, max_x, min_y, 0.0f, max_x, min_y, top, style.guide[0], style.guide[1], style.guide[2]);
    PushLine(vertices, max_x, max_y, 0.0f, max_x, max_y, top, style.guide[0], style.guide[1], style.guide[2]);
    PushLine(vertices, min_x, max_y, 0.0f, min_x, max_y, top, style.guide[0], style.guide[1], style.guide[2]);

    return VertexRange{
        .first = start,
        .count = static_cast<int>(vertices.size()) - start,
    };
}

} // namespace

WorldVisualStyle BuildWorldVisualStyle(bool dark_theme)
{
    const std::array<float, 4> sky_low = {0.741f, 0.894f, 0.961f, 1.0f};
    const std::array<float, 4> sky_top = {0.592f, 0.835f, 0.933f, 1.0f};
    const std::array<float, 4> ground = {0.733f, 0.761f, 0.796f, 1.0f};

    if (dark_theme) {
        return WorldVisualStyle{
            .sky_color = sky_low,
            .sky_top_color = sky_top,
            .ground_color = ground,
            .grid_minor = {0.62f, 0.66f, 0.71f},
            .grid_major = {0.52f, 0.56f, 0.61f},
            .axis_x = {0.92f, 0.24f, 0.25f},
            .axis_y = {0.20f, 0.82f, 0.26f},
            .axis_z = {0.16f, 0.34f, 0.92f},
            .guide = {0.86f, 0.52f, 0.24f},
            .shell_primary = {0.81f, 0.81f, 0.81f},
            .shell_secondary = {0.62f, 0.62f, 0.62f},
            .glass = {0.88f, 0.90f, 0.94f},
            .accent = {0.85f, 0.50f, 0.25f},
            .section = {0.90f, 0.55f, 0.28f},
            .horizon_ratio = 0.66f,
        };
    }

    return WorldVisualStyle{
        .sky_color = sky_low,
        .sky_top_color = sky_top,
        .ground_color = ground,
        .grid_minor = {0.66f, 0.70f, 0.75f},
        .grid_major = {0.56f, 0.60f, 0.65f},
        .axis_x = {0.92f, 0.24f, 0.25f},
        .axis_y = {0.20f, 0.82f, 0.26f},
        .axis_z = {0.16f, 0.34f, 0.92f},
        .guide = {0.83f, 0.49f, 0.21f},
        .shell_primary = {0.78f, 0.78f, 0.78f},
        .shell_secondary = {0.62f, 0.62f, 0.62f},
        .glass = {0.87f, 0.89f, 0.93f},
        .accent = {0.83f, 0.49f, 0.21f},
        .section = {0.89f, 0.54f, 0.28f},
        .horizon_ratio = 0.66f,
    };
}

WorldScene CreateSketchWorld(WorldVisualStyle const& style)
{
    WorldScene scene;
    scene.vertices.reserve(3000);
    scene.massing_mesh = AddMassing(scene.vertices, style, scene.stats);
    scene.glass_mesh = AddGlass(scene.vertices, style);
    scene.floor_outlines = AddFloorOutlines(scene.vertices, style);
    scene.grid = AddGrid(scene.vertices, style);
    scene.section_lines = AddSectionLines(scene.vertices, style);
    scene.axes = AddAxes(scene.vertices, style);
    scene.guide_cube = AddGuideCube(scene.vertices, style);
    return scene;
}
