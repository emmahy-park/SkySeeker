#version 330

uniform sampler2D screen_texture;
uniform sampler2D shadow_texture;
uniform float time;
uniform float darken_screen_factor;

in vec2 texcoord;

layout(location = 0) out vec4 color;

vec4 vignette(vec4 in_color) 
{

	// M4 Creative component: Shadow
	vec4 shadow_color = texture(shadow_texture, texcoord);
	in_color = mix(in_color, vec4(0.0, 0.0, 0.0, 1.0f), 0.5f - (0.5f)*(shadow_color.a));
	return in_color;
}

// darken the screen, i.e., fade to black
vec4 fade_color(vec4 in_color) 
{
	if (darken_screen_factor > 0)
		in_color -= darken_screen_factor * vec4(0.8, 0.8, 0.8, 0.5);
	return in_color;
}

void main()
{
    vec4 in_color = texture(screen_texture, texcoord);
	 in_color = vignette(in_color);
    color = fade_color(in_color);
}