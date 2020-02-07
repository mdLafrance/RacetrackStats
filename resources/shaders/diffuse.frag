#version 330 core

// Uniforms
uniform mat4 MVP;
uniform mat4 VP;

uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;

uniform sampler2D map_Kd;
uniform sampler2D map_Ks;
uniform sampler2D map_norm;

uniform float Ns;
uniform float Tr;

uniform int numOfLights;
uniform mat3 lights[8];

// Inputs
in mediump vec3 v_norm;
in mediump vec2 v_texCoord;

out vec4 FragColor;

void main() {
	vec3 normal = normalize(v_norm);
	

	FragColor = vec4(Kd, 1.0f) + texture(map_Kd, v_texCoord);
}