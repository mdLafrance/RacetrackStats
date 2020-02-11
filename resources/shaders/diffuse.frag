#version 330 core

// Same flags values as in Material.h
#define MATERIAL_USE_map_Kd 1
#define MATERIAL_USE_map_Ks 2
#define MATERIAL_USE_map_norm 4

// Uniforms
uniform mat4 MV;
uniform mat4 MVP;

uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;

uniform sampler2D map_Kd;
uniform sampler2D map_Ks;
uniform sampler2D map_norm;

uniform vec3 cameraForward;

// misc flags which can be provided by the material
uniform int flags;

uniform float Ns; // Spec exponent
uniform float Tr; // Transparency (1 is transparent)

uniform int numOfLights;
uniform mat3 lights[8];

// Inputs
in mediump vec3 v_norm;
in mediump vec3 v_pos;
in mediump vec3 v_pos_world;
in mediump vec2 v_texCoord;

out vec4 FragColor;

void main() {	

	vec3 light_v, light_color, light_values, Iin, Ia, diffuse, specular, N, N_vcs, V, R, Iout;

	V = normalize(-1.0f * v_pos);

	// diffuse, spec, and normal can be driven by texture instead of unfiform value if flags are set in material
	diffuse = Kd;
	if ((flags & MATERIAL_USE_map_Kd) != 0){
		diffuse = vec3(texture(map_Kd, v_texCoord));
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

	N_vcs = normalize(vec3(MV * vec4(N, 0)));

	Iout = vec3(Ka.x * diffuse.x, Ka.y * diffuse.y, Ka.z * diffuse.z);  // Accumulated output intensity

	float NdotL, RdotV, intensity;

	for (int i = 0; i < numOfLights; i++){
		// 0 in this location indicates directional light
		if (lights[i][2][0] == 0){
			light_v = -1 * lights[i][1];
			NdotL = dot(N, normalize(light_v));

			if (NdotL <= 0.0) continue; // Light is shining on back face, continue

			// Unpack settings for each light
			light_color = normalize(lights[i][0]);
			light_values = lights[i][2];

			intensity = light_values[1];

			// Diffuse 
			Iout += NdotL * intensity * vec3(diffuse.x * light_color.x, diffuse.y * light_color.y, diffuse.z * light_color.z);

			continue;
			// Spec
			R = ((2.0 * NdotL) * N) - light_v;
			RdotV = dot(R, vec3(MV* vec4(cameraForward,1)));
			
			// specular not working?
			if (RdotV > 0.0){
				Iout += pow(RdotV, Ns) * vec3(specular.x * light_color.x, specular.y * light_color.y, specular.z * light_color.z);
			}

		}
	}

	FragColor = vec4(Iout, 1.0f - Tr);

}