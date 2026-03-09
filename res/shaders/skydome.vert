#version 330 core

layout (location = 0) in vec3 a_position;

uniform mat4 u_view_projection;

out vec3 v_position;
out vec2 v_screen_uv;

void main()
{
    v_position = a_position;
    gl_Position = u_view_projection * vec4(a_position, 1.0);
    v_screen_uv = gl_Position.xy / gl_Position.w * 0.5 + 0.5;
}
