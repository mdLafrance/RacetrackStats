#version 330 core

layout(location = 0) in vec3 in_Pos;

uniform mat4 u_MVP;
uniform mediump vec3 u_color;

void main() {
	// gl_Position = vec4(in_Pos, 1.0f);
	gl_Position = u_MVP * vec4(in_Pos.xyz, 1.0);
};