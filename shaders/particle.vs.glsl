#version 330

// Input attributes
in vec3 in_position;
in vec2 in_texcoord;
layout (location = 2) in mat3 in_transform;

out vec2 texcoord;

// Application data
uniform mat3 projection;

void main()
{
	vec3 pos = projection * in_transform * vec3(in_position.xy, 1.0);
	texcoord = in_texcoord;
	gl_Position = vec4(pos.xy, in_position.z, 1.0);
}