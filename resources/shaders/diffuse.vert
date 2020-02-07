// Normal diffuse shader

#version 330 core

// Uniforms
uniform mat4 MVP;
uniform mat4 VP;

uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;

uniform float Ns;
uniform float Tr;

// Mesh attributes
layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_norm;
layout (location = 2) in vec2 in_texCoord;

// out
out mediump vec3 v_norm;
out mediump vec2 v_texCoord;

void main(){
	v_texCoord = in_texCoord;
	gl_Position = MVP * vec4(in_pos, 1.0f);
}
