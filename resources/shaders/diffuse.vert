// Normal diffuse shader

#version 330 core

// Uniforms
uniform mat4 M;
uniform mat4 VP;
uniform mat4 MVP;

uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;

uniform float Ns;
uniform float Tr;

uniform vec3 cameraForward;

// Mesh attributes
layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_norm;
layout (location = 2) in vec2 in_texCoord;

out mediump vec3 v_pos;
out mediump vec3 v_norm;
out mediump vec2 v_texCoord;

out mediump vec3 v_norm_world;

void main(){
	v_pos = in_pos;
	v_norm = in_norm;
	v_texCoord = in_texCoord;
	
	v_norm_world = (M * vec4(in_norm, 0)).xyz;

	gl_Position = VP * M * vec4(in_pos, 1);
}
