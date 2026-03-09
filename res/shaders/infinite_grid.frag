#version 330 core

in vec3 v_world_position;

uniform vec3 u_grid_origin;
uniform float u_cell_size;
uniform float u_major_step;
uniform float u_fade_start;
uniform float u_fade_end;
uniform vec3 u_minor_color;
uniform vec3 u_major_color;
uniform vec3 u_axis_x_color;
uniform vec3 u_axis_y_color;
uniform float u_axis_emphasis;

out vec4 frag_color;

float GridFactor(vec2 world_coord, float spacing)
{
    vec2 scaled = world_coord / spacing;
    vec2 grid = abs(fract(scaled - 0.5) - 0.5) / max(fwidth(scaled), vec2(0.0001));
    float line = min(grid.x, grid.y);
    return 1.0 - min(line, 1.0);
}

void main()
{
    float minor = GridFactor(v_world_position.xy, u_cell_size);
    float major = GridFactor(v_world_position.xy, u_cell_size * u_major_step);

    float axis_x = (1.0 - min(abs(v_world_position.x) / max(fwidth(v_world_position.x), 0.0001), 1.0)) * u_axis_emphasis;
    float axis_y = (1.0 - min(abs(v_world_position.y) / max(fwidth(v_world_position.y), 0.0001), 1.0)) * u_axis_emphasis;

    float distance_from_center = length(v_world_position.xy - u_grid_origin.xy);
    float fade = 1.0 - smoothstep(u_fade_start, u_fade_end, distance_from_center);

    vec3 color = u_minor_color;
    float alpha = minor * 0.26;

    color = mix(color, u_major_color, major);
    alpha = max(alpha, major * 0.55);

    color = mix(color, u_axis_x_color, axis_x);
    alpha = max(alpha, axis_x * 0.9);

    color = mix(color, u_axis_y_color, axis_y);
    alpha = max(alpha, axis_y * 0.9);

    alpha *= fade;

    if (alpha <= 0.002) {
        discard;
    }

    frag_color = vec4(color, alpha);
}
