#version 330 core

// Same flags values as in Material.h
#define MATERIAL_USE_map_Kd   1
#define MATERIAL_USE_map_Ks   1 << 1
#define MATERIAL_USE_map_norm 1 << 2
#define MATERIAL_TRANSPARENT  1 << 3

// Uniforms
uniform mat4 M;
uniform mat4 VP;
uniform mat4 MVP;

uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;

uniform float Ns; // Spec exponent
uniform float Tr; // Transparency (1 is transparent)

uniform sampler2D map_Kd;
uniform sampler2D map_Ks;
uniform sampler2D map_norm;

uniform vec3 cameraForward;

// misc flags which can be provided by the material
uniform int flags;

uniform int numOfLights;
uniform mat3 lights[8];

// Inputs
in mediump vec3 v_norm;
in mediump vec3 v_pos;
in mediump vec3 v_norm_world;
in mediump vec2 v_texCoord;

out vec4 FragColor;

void main() {	
	vec3 V = normalize(-1.0f * v_pos);

	float diffuse_alpha = 1.0f;

	// diffuse, spec, and normal can be driven by texture instead of unfiform value if flags are set in material
	vec3 diffuse = Kd;
	if ((flags & MATERIAL_USE_map_Kd) != 0){
		vec4 tex = texture(map_Kd, v_texCoord);
		diffuse = tex.xyz;
		diffuse_alpha = tex.a;
	}

	// If material is defined to be transparent
	// TODO: this is a hack, need to fix actual transparency pipeline
	if ((flags & MATERIAL_TRANSPARENT) != 0) {
		if (diffuse_alpha <= 0.01f){
			discard;
		}
	} else {
		diffuse_alpha = 1.0f;
	}

	vec3 specular;
	if ((flags & MATERIAL_USE_map_Ks) != 0){
		specular = vec3(texture(map_Ks, v_texCoord));
	} else {
		specular = Ks;
	}

	vec3 N, N_wcs;
	if ((flags & MATERIAL_USE_map_norm) != 0){
		N_wcs = normalize((M * vec4(texture(map_norm, v_texCoord).xys, 0)).xyz);
	} else {
		N_wcs = normalize(v_norm_world);
	}

	vec3 Iout = vec3(Ka.x * diffuse.x, Ka.y * diffuse.y, Ka.z * diffuse.z);  // Accumulated output intensity

	vec3 light_v, light_color, light_values, R;
	float NdotL, RdotV, intensity;

	for (int i = 0; i < numOfLights; i++){
		// 0 in this location indicates directional light
		if (lights[i][2][0] == 0){
			light_v = -1 * lights[i][1];
			NdotL = dot(N_wcs, normalize(light_v));

			if (NdotL <= 0.0) continue; // Light is shining on back face, continue

			// Unpack settings for each light
			light_color = normalize(lights[i][0]);
			light_values = lights[i][2];

			intensity = light_values[1];

			// Diffuse 
			Iout += NdotL * intensity * vec3(diffuse.x * light_color.x, diffuse.y * light_color.y, diffuse.z * light_color.z);

			// // Spec
			// R = ((2.0 * NdotL) * N_wcs) - light_v;
			// RdotV = dot(R, vec3(VP * vec4(cameraForward,1)));
			
			// if (RdotV > 0.0){
			// 	Iout += pow(RdotV, Ns) * vec3(specular.x * light_color.x, specular.y * light_color.y, specular.z * light_color.z);
			// }

		}
	}

	FragColor = vec4(Iout, diffuse_alpha);
}