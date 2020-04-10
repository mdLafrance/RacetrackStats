#include <Shader.h>

// Default shaders
const char* vShaderSource =
"#version 330 core\n"
"layout(location = 0) in vec3 vPos;\n"
"layout(location = 1) in vec3 vNorm;\n"
"layout(location = 2) in vec2 vTex;\n"
"uniform mat4 MVP;\n"
"out vec2 texCoord;\n"
"void main() {\n"
"   texCoord = vTex;\n"
"	gl_Position = MVP * vec4(vPos.x, vPos.y, vPos.z, 1.0);\n"
"}\0";

const char* fShaderSourceBasic =
"#version 330 core\n"
"in vec2 texCoord;\n"
"out vec4 FragColor;\n"
"uniform sampler2D tex;\n"
"void main(){\n"
"	FragColor = vec4(0.8, 0.8, 0.8, 1.0f);\n"
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

Shader::Shader(const std::string& vertexShaderTarget, const std::string& fragmentShaderTarget) {
	// Load and link an opengl shader from the vertex and fragment shaders specified as parameters.
	// Params should be paths to the files.

	std::ifstream vertexShaderSourceFile, fragmentShaderSourceFile;
	std::string vs_string, fs_string, vertexShortName, fragmentShortName;
	const char* vs_source;
	const char* fs_source;
	int vertexCompiled, fragmentCompiled, linkSuccessful;

	int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	std::cout << "Loading vertex shader (ID: " << vertexShader << "): [" << vertexShaderTarget << ']' << std::endl;

	if (vertexShaderTarget == "default") {
		vs_source = vShaderSource;
	} else {
		vertexShaderSourceFile.open(vertexShaderTarget);

		if (vertexShaderSourceFile.good()) {
			// Copy file into string
			vs_string.assign(std::istreambuf_iterator<char>(vertexShaderSourceFile),
							 std::istreambuf_iterator<char>());

			vs_source = vs_string.c_str();
		}
		else {
			std::cerr << "Couldn't open " << vertexShaderTarget << " for reading." << std::endl;
			vs_source = vShaderSource;
		}
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
		fragmentShaderSourceFile.open(fragmentShaderTarget);

		if (fragmentShaderSourceFile.good()) {
			// Copy file into string
			fs_string.assign(std::istreambuf_iterator<char>(fragmentShaderSourceFile),
							 std::istreambuf_iterator<char>());

			fs_source = fs_string.c_str();
		}
		else {
			std::cerr << "Couldn't open " << fragmentShaderTarget << " for reading." << std::endl;
			fs_source = fShaderSourceBasic;
		}
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
		vertexShortName = Utils::getFileInfo(vertexShaderTarget).file;
		fragmentShortName = Utils::getFileInfo(fragmentShaderTarget).file;
		std::cout << "Successfully loaded shader with vertex/fragment: " << vertexShortName << " " << fragmentShortName << std::endl;
	}
}

Shader::~Shader() {
	glDeleteShader(this->shaderProgram);
}
