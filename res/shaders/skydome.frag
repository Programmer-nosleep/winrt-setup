#version 330 core

in vec3 v_position;
in vec2 v_screen_uv;

uniform vec3 u_sky_low_color;
uniform vec3 u_sky_base_blue;
uniform vec3 u_ground_gray;
uniform float u_use_screen_gradient;

out vec4 frag_color;

void main()
{
    // Perspective mode: Use local position Z (Z-up world)
    // Ortho mode: Use screen Y (v_screen_uv.y)
    float t = mix(
        smoothstep(-2.0, 2.0, v_position.z),
        smoothstep(0.498, 0.502, v_screen_uv.y),
        u_use_screen_gradient
    );

    // Sky gradient intensity
    float sky_factor = mix(
        clamp(v_position.z / 500.0, 0.0, 1.0),
        clamp((v_screen_uv.y - 0.5) * 2.0, 0.0, 1.0),
        u_use_screen_gradient
    );

    vec3 sky = mix(u_sky_low_color, u_sky_base_blue, sky_factor);
    vec3 ground = u_ground_gray;
    vec3 color = mix(ground, sky, t);

    frag_color = vec4(color, 1.0);
}
