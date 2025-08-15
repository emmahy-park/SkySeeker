#version 330

// Input attributes
in vec3 in_position;
in vec2 in_texcoord;

// Passed to fragment shader
out vec2 texcoord;
out vec2 fragTexcoord;

// Application data
uniform mat3 transform;
uniform mat3 projection;

uniform vec4 tiletexcoord;

void main()
{
	texcoord = in_texcoord;

	fragTexcoord.x = tiletexcoord.x + texcoord.x * (tiletexcoord.z - tiletexcoord.x);
	fragTexcoord.y = tiletexcoord.y + texcoord.y * (tiletexcoord.w - tiletexcoord.y);

	vec3 pos = projection * transform * vec3(in_position.xy, 1.0);
	gl_Position = vec4(pos.xy, in_position.z, 1.0);
}