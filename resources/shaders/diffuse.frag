#version 330 core

// Same flags values as in Material.h
#define MATERIAL_USE_map_Kd 1
#define MATERIAL_USE_map_Ks 2
#define MATERIAL_USE_map_norm 4

// Uniforms
uniform mat4 MVP;
uniform mat4 VP;

uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;

uniform sampler2D map_Kd;
uniform sampler2D map_Ks;
uniform sampler2D map_norm;

// misc flags which can be provided by the material
uniform int flags;

uniform float Ns; // Spec exponent
uniform float Tr; // Transparency (1 is transparent)

uniform int numOfLights;
uniform mat3 lights[8];

// Inputs
in mediump vec3 v_norm;
in mediump vec3 v_pos;
in mediump vec2 v_texCoord;

out vec4 FragColor;

void main() {
	vec3 light_v, light_color, light_values, Iin, Ia, diffuse, specular, N, V, R, Iout;

	V = normalize(-1.0f * v_pos);

	// diffuse, spec, and normal can be driven by texture instead of unfiform value if flags are set in material
	if ((flags & MATERIAL_USE_map_Kd) != 0){
		diffuse = vec3(texture(map_Kd, v_texCoord));
	} else {
		diffuse = Kd;
	}

	if ((flags & MATERIAL_USE_map_Ks) != 0){
		specular = vec3(texture(map_Ks, v_texCoord));
	} else {
		specular = Ks;
	}

	if ((flags & MATERIAL_USE_map_norm) != 0){
		N = normalize(vec3(texture(map_norm, v_texCoord))); // Maybe this needs to be converted to vcs in vert shader as well?
	} else {
		N = normalize(v_norm);
	}

	Iout = Ka; // Accumulated output intensity

	float NdotL, RdotV;

	for (int i = 0; i < numOfLights; i++){
		light_v = lights[i][1];
		NdotL = dot(N, light_v);

		if (NdotL <= 0.0) continue; // Light is shining on back face, continue

		// Unpack settings for each light
		light_color = lights[i][0];
		light_values = lights[i][2];

		// Diffuse 
		Iout += NdotL * vec3(diffuse.x * light_color.x, diffuse.y * light_color.y, diffuse.z * light_color.z);

		// Spec
		R = (2.0 * NdotL) * N - light_v;
		RdotV = dot(R, V);
		
		if (RdotV > 0.0){
			Iout += pow(RdotV, Ns) * vec3(specular.x * light_color.x, specular.y * light_color.y, specular.z * light_color.z);
		}
	}

	FragColor = vec4(Iout, 1.0f - Tr);
}