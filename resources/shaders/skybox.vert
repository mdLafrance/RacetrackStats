#version 330 core
layout (location = 0) in vec3 in_pos;

out mediump vec3 TexCoords;

uniform mat4 MVP;

void main()
{
    TexCoords = in_pos;
	vec4 pos =  MVP * vec4(in_pos, 1.0f);
    gl_Position = pos;
}  