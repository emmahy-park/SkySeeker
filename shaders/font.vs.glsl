#version 330 core
//layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
layout (location = 0) in vec2 in_position;
layout (location = 1) in vec2 in_texcoord;
out vec2 TexCoords;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(in_position, 0.0, 1.0);
    TexCoords = in_texcoord;
}  