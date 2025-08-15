#version 330

uniform sampler2D prev_texture;
uniform sampler2D light_texture;
uniform float time;
uniform float darken_screen_factor;

in vec2 texcoord;

layout(location = 0) out vec4 color;


void main()
{
    vec4 in_color = texture(prev_texture, texcoord);
    vec4 light_color = texture(light_texture, texcoord);
    in_color.a = in_color.a + (light_color.a) * 0.5;
    color = in_color;
}