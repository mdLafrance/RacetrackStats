// Normal diffuse shader

#version 330 core

// Uniforms
uniform mat4 MVP;
uniform mat4 MV;

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
out mediump vec3 v_pos;
out mediump vec2 v_texCoord;

void main(){
	vec4 in_pos4fv = vec4(in_pos, 1);

	v_texCoord = in_texCoord;

	// View coordinate converted normals and positions, for phong calc in frag
	v_norm = vec3(MV * vec4(in_norm, 0));
	v_pos = vec3(MV * in_pos4fv);

	gl_Position = MVP * in_pos4fv;
}
