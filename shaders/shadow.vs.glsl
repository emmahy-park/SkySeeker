#version 330

// !!! Simple shader for colouring basic meshes

// Input attributes
in vec3 in_position;

// Application data
uniform mat3 transform;
uniform mat3 projection;
uniform vec2 in_light_position;

out vec4 color;


void main()
{
	vec3 pos = projection * transform * vec3(in_position.xy, 1.f);
	vec3 light_pos = projection * vec3(in_light_position, 1.0);

	gl_Position = vec4(pos.xy - in_position.z*light_pos.xy, 0, 1 - in_position.z);
}
