#version 330 core
layout (location = 0) in vec3 v_Pos;

out vec3 TexCoords;

uniform mat4 MVP;

void main()
{
    TexCoords = v_Pos;
    gl_Position = MVP * vec4(v_Pos, 1.0);
}  