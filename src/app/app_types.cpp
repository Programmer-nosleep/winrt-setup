#include "app/app_types.h"

const char* ThemeModeLabel(ThemeMode mode)
{
    switch (mode) {
    case ThemeMode::System:
        return "System";
    case ThemeMode::Light:
        return "Light";
    case ThemeMode::Dark:
        return "Dark";
    default:
        return "Unknown";
    }
}

const char* ToolLabel(ToolKind tool)
{
    switch (tool) {
    case ToolKind::Select:
        return "Select";
    case ToolKind::Wall:
        return "Wall";
    case ToolKind::Floor:
        return "Floor";
    case ToolKind::Ceiling:
        return "Ceiling";
    case ToolKind::Roof:
        return "Roof";
    case ToolKind::Column:
        return "Column";
    case ToolKind::Beam:
        return "Beam";
    case ToolKind::Door:
        return "Door";
    case ToolKind::Window:
        return "Window";
    case ToolKind::Line:
        return "Line";
    case ToolKind::Arc:
        return "Arc";
    case ToolKind::Move:
        return "Move";
    case ToolKind::Rotate:
        return "Rotate";
    case ToolKind::Orbit:
        return "Orbit";
    case ToolKind::Pan:
        return "Pan";
    case ToolKind::Zoom:
        return "Zoom";
    case ToolKind::Paint:
        return "Material";
    case ToolKind::Measure:
        return "Measure";
    case ToolKind::Section:
        return "Section";
    case ToolKind::Copy:
        return "Copy";
    default:
        return "Tool";
    }
}

const char* ToolButtonLabel(ToolKind tool)
{
    switch (tool) {
    case ToolKind::Select:
        return "SEL";
    case ToolKind::Wall:
        return "WAL";
    case ToolKind::Floor:
        return "FLR";
    case ToolKind::Ceiling:
        return "CLG";
    case ToolKind::Roof:
        return "ROF";
    case ToolKind::Column:
        return "COL";
    case ToolKind::Beam:
        return "BEM";
    case ToolKind::Door:
        return "DOR";
    case ToolKind::Window:
        return "WIN";
    case ToolKind::Line:
        return "LIN";
    case ToolKind::Arc:
        return "ARC";
    case ToolKind::Move:
        return "MOV";
    case ToolKind::Rotate:
        return "ROT";
    case ToolKind::Orbit:
        return "ORB";
    case ToolKind::Pan:
        return "PAN";
    case ToolKind::Zoom:
        return "ZOM";
    case ToolKind::Paint:
        return "MAT";
    case ToolKind::Measure:
        return "DIM";
    case ToolKind::Section:
        return "SEC";
    case ToolKind::Copy:
        return "CPY";
    default:
        return "TOOL";
    }
}

const char* ToolIconName(ToolKind tool)
{
    switch (tool) {
    case ToolKind::Select:
        return "NavigationArrow";
    case ToolKind::Wall:
        return "Line";
    case ToolKind::Floor:
        return "ResponsiveLayoutLibrary";
    case ToolKind::Ceiling:
        return "Grid";
    case ToolKind::Roof:
        return "Octagon";
    case ToolKind::Column:
        return "BoundingBox";
    case ToolKind::Beam:
        return "Vector";
    case ToolKind::Door:
        return "NavigationArrow";
    case ToolKind::Window:
        return "Rectangle";
    case ToolKind::Line:
        return "Line";
    case ToolKind::Arc:
        return "BezierCurve";
    case ToolKind::Move:
        return "Move";
    case ToolKind::Rotate:
        return "Rotate";
    case ToolKind::Orbit:
        return "Orbit";
    case ToolKind::Pan:
        return "BackHand";
    case ToolKind::Zoom:
        return "BoundingBox";
    case ToolKind::Paint:
        return "ColorBucketMaterial";
    case ToolKind::Measure:
        return "Dimensions";
    case ToolKind::Section:
        return "Section";
    case ToolKind::Copy:
        return "Copy";
    default:
        return "Cube";
    }
}

const char* ToolHintText(ToolKind tool)
{
    switch (tool) {
    case ToolKind::Select:
        return "LMB drag creates a window or crossing marquee. RMB orbit, MMB pan, wheel zoom.";
    case ToolKind::Wall:
        return "Wall authoring shell is active. Layout mirrors the arch workspace, but placement is still visual only.";
    case ToolKind::Floor:
        return "Floor authoring shell is active. Use this mode as a planning context for slabs and levels.";
    case ToolKind::Ceiling:
        return "Ceiling authoring shell is active. Detailed ceiling editing is not wired yet.";
    case ToolKind::Roof:
        return "Roof authoring shell is active. Use it to inspect the upper massing layer.";
    case ToolKind::Column:
        return "Column authoring shell is active. Structural placement is not implemented yet.";
    case ToolKind::Beam:
        return "Beam authoring shell is active. Structural editing remains a placeholder.";
    case ToolKind::Door:
        return "Door placement shell is active. Openings are preview-only for now.";
    case ToolKind::Window:
        return "Window placement shell is active. Curtain wall editing is not implemented yet.";
    case ToolKind::Line:
        return "Line tool shell is selected. Geometry editing is not implemented yet.";
    case ToolKind::Arc:
        return "Arc tool shell is selected. Use it as UI context only for now.";
    case ToolKind::Move:
        return "Move tool shell is selected. View navigation remains active with mouse controls.";
    case ToolKind::Rotate:
        return "Rotate tool shell is selected. Camera presets remain available in the sidebar.";
    case ToolKind::Orbit:
        return "Orbit tool context selected. RMB orbit, MMB pan, wheel zoom.";
    case ToolKind::Pan:
        return "Pan tool context selected. MMB pan remains the direct navigation gesture.";
    case ToolKind::Zoom:
        return "Zoom tool context selected. Mouse wheel controls zoom.";
    case ToolKind::Paint:
        return "Material tool shell is selected. Viewport material preview is available in the tray.";
    case ToolKind::Measure:
        return "Measure tool shell is selected. Measurement tools are not wired yet.";
    case ToolKind::Section:
        return "Section review shell is active. Use the section box toggle to inspect the massing cut.";
    case ToolKind::Copy:
        return "Copy shell is active. Duplication workflows are not implemented yet.";
    default:
        return "Ready.";
    }
}
