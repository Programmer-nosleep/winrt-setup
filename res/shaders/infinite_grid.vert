#version 330 core

layout (location = 0) in vec2 a_plane;

uniform mat4 u_view_projection;
uniform vec3 u_grid_origin;
uniform float u_grid_extent;

out vec3 v_world_position;

void main()
{
    vec3 world_position = vec3(
        u_grid_origin.x + (a_plane.x * u_grid_extent),
        u_grid_origin.y + (a_plane.y * u_grid_extent),
        u_grid_origin.z
    );

    v_world_position = world_position;
    gl_Position = u_view_projection * vec4(world_position, 1.0);
}
