#include <Shader.h>

// Default shaders
const char* vShaderSource =
"// Default shader, just diffuse shading;\n"
"#version 330 core\n"
"// Uniforms\n"
"uniform mat4 M;\n"
"uniform mat4 MVP;\n"
"// Mesh attributes\n"
"layout(location = 0) in vec3 in_pos;\n"
"layout(location = 1) in vec3 in_norm;\n"
"layout(location = 2) in vec2 in_texCoord;\n"
"out mediump vec3 v_norm;\n"
"out mediump vec3 v_norm_world;\n"
"void main() {\n"
"	v_norm = in_norm;\n"
"	v_norm_world = (M * vec4(in_norm, 0)).xyz;\n"
"	gl_Position = MVP * vec4(in_pos, 1);\n"
"}\n\0";

const char* fShaderSourceBasic =
"// Default diffuse shader\n"
"#version 330 core\n"
"// Uniforms\n"
"uniform mat4 M;\n"
"uniform mat4 MVP;\n"
"// Lights\n"
"uniform int numOfLights;\n"
"uniform mat3 lights[8];\n"
"// Passed mesh attributes\n"
"in mediump vec3 v_norm;\n"
"in mediump vec3 v_norm_world;\n"
"out vec4 FragColor;\n"
"void main() {\n"
"	vec3 diffuse = vec3(1.0f, 1.0f, 1.0f);\n"
"	vec3 Iout;\n"
"	for (int i = 0; i < numOfLights; i++) {\n"
"		if (lights[i][2][0] == 0) {\n"
"			vec3 light_v = -1 * lights[i][1];\n"
"			float NdotL = dot(v_norm_world, normalize(light_v));\n"
"			if (NdotL <= 0.0) continue; // Light is shining on back face, continue\n"
"			// Unpack settings for each light\n"
"			vec3 light_color = normalize(lights[i][0]);\n"
"			vec3 light_values = lights[i][2];\n"
"			float intensity = light_values[1];\n"
"			// Diffuse \n"
"			Iout += NdotL * intensity * vec3(diffuse.x * light_color.x, diffuse.y * light_color.y, diffuse.z * light_color.z);\n"
"		}\n"
"	}\n"
"	FragColor = vec4(Iout, 1.0f);\n"
"}\n\0";

void Shader::bind() {
	glUseProgram(this->shaderProgram);
}

void Shader::setUniform(const char* name, const float& val) {
	glUniform1f(glad_glGetUniformLocation(this->shaderProgram, name), val);
}

void Shader::setUniform(const char* name, const unsigned int& val) {
	glUniform1i(glad_glGetUniformLocation(this->shaderProgram, name), val);
}

void Shader::setUniform(const char* name, const glm::vec3& val) {
	glUniform3fv(glad_glGetUniformLocation(this->shaderProgram, name), 1, &val[0]);
}

void Shader::setUniform(const char* name, const glm::mat4& val) {
	glUniformMatrix4fv(glad_glGetUniformLocation(this->shaderProgram, name), 1, GL_FALSE, &val[0][0]);
}

void Shader::setLights(const int& count, const glm::mat3* m0) {
	glUniform1i(glad_glGetUniformLocation(this->shaderProgram, "numOfLights"), count);

	glUniformMatrix3fv(glad_glGetUniformLocation(this->shaderProgram, "lights"), count, GL_FALSE, &(*m0)[0][0]);
}

std::string Shader::loadShaderSource(const std::string& target, const bool& followIncludes) {
	std::string source;
	std::string line;

	std::ifstream f(target);
	
	if (!f.is_open()) {
		std::cerr << "ERROR: Couldn't load source [" << target << ']' << std::endl;
		return source;
	}

	while (std::getline(f, line)) {
		if (Utils::hasStart(line, "#include")) {
			if (followIncludes) {
				// Don't have access to build settings, so only support relative includes
				if ((line.find('<') != std::string::npos) && (line.find('>') != std::string::npos)) {
					std::cerr << "ERROR: Shaders only support relative includes." << std::endl;
					continue;
				}
				
				int quote0 = line.find('\"') + 1;
				int quote1 = line.rfind('\"');

				std::string includeTarget = line.substr(quote0, quote1 - quote0);

				source += Shader::loadShaderSource((Utils::getFileInfo(target).directory + DIRECTORY_SEPARATOR + includeTarget).c_str(), false); // Only follow one level of includes
			}
		} else {
			source += line + '\n';
		}
	}

	return source;
}

Shader::Shader(const std::string& vertexShaderTarget, const std::string& fragmentShaderTarget) {
	// Load and link an opengl shader from the vertex and fragment shaders specified as parameters.
	// Params should be paths to the files.

	std::string vs_string, fs_string;
	const char* vs_source;
	const char* fs_source;
	int vertexCompiled, fragmentCompiled, linkSuccessful;

	int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	std::cout << "Loading vertex shader (ID: " << vertexShader << "): [" << vertexShaderTarget << ']' << std::endl;

	if (vertexShaderTarget == "default") {
		vs_source = vShaderSource;
	} else {
		vs_string = Shader::loadShaderSource(vertexShaderTarget).c_str(); // NOTE: this is assigned to vs_string var out of scope so the internal char array doesn't get cleaned up
		vs_source = vs_string.c_str();
	}

	glShaderSource(vertexShader, 1, &vs_source, NULL);
	glCompileShader(vertexShader);

	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vertexCompiled);

	if (!vertexCompiled) {
		char status[512];

		glGetShaderInfoLog(vertexShader, 512, NULL, status);

		std::cerr << "Vertex Shader " << vertexShader << " failed to compile: \n" << status << std::endl;
	}
	else {
		std::cout << "Vertex Shader Compiled." << std::endl;
	}

	// Create fragment shader

	int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	std::cout << "Loading fragment shader (ID: " << fragmentShader << "): [" << fragmentShaderTarget << ']' << std::endl;

	if (fragmentShaderTarget == "default") {
		fs_source = fShaderSourceBasic;
	} else {
		fs_string = Shader::loadShaderSource(fragmentShaderTarget).c_str();
		fs_source = fs_string.c_str();
	}

	glShaderSource(fragmentShader, 1, &fs_source, NULL);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fragmentCompiled);

	if (!fragmentCompiled) {
		char status[512];

		glGetShaderInfoLog(fragmentShader, 512, NULL, status);

		std::cerr << "Fragment Shader " << fragmentShader << " failed to compile: \n" << status << std::endl;
	}
	else {
		std::cout << "Fragment Shader Compiled." << std::endl;
	}

	// Link shaders

	this->shaderProgram = glCreateProgram();

	std::cout << "Creating shader program (ID: " << this->shaderProgram << ")" << std::endl;

	if (vertexCompiled && fragmentCompiled) {
		glAttachShader(this->shaderProgram, vertexShader);
		glAttachShader(this->shaderProgram, fragmentShader);
		glLinkProgram(this->shaderProgram);

		glGetProgramiv(this->shaderProgram, GL_LINK_STATUS, &linkSuccessful);

		if (!linkSuccessful) {
			char status[512];

			glGetProgramInfoLog(this->shaderProgram, 512, NULL, status);

			std::cerr << "Shader program " << this->shaderProgram << " failed to link: \n" << status << std::endl;
		}
	}
	else {
		std::cerr << "All shaders did not successfully compile, cannot link." << std::endl;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	if (vertexCompiled && fragmentCompiled && linkSuccessful) {
		std::string vertexShortName = Utils::getFileInfo(vertexShaderTarget).file;
		std::string fragmentShortName = Utils::getFileInfo(fragmentShaderTarget).file;
		std::cout << "Successfully loaded shader with vertex/fragment: " << vertexShortName << " " << fragmentShortName << std::endl;
	}
}

Shader::~Shader() {
	glDeleteShader(this->shaderProgram);
}
