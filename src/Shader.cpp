#include <Shader.h>
const char* vShaderSource =
"#version 330 core\n"
"layout(location = 0) in vec3 vPos;\n"
"layout(location = 1) in vec3 vNorm;\n"
"layout(location = 2) in vec2 vTex;\n"
"uniform mat4 u_MVP;\n"
"out vec2 texCoord;\n"
"void main() {\n"
"   texCoord = vTex;\n"
"	gl_Position = u_MVP * vec4(vPos.x, vPos.y, vPos.z, 1.0);\n"
"}\0";

const char* fShaderSourceBasic =
"#version 330 core\n"
"in vec2 texCoord;\n"
"out vec4 FragColor;\n"
"uniform sampler2D tex;\n"
"void main(){\n"
"	//FragColor = texture(tex, texCoord);\n"
"	FragColor = vec4(0.1f, 0.0f, 0.8f, 1.0f);\n"
"}\n\0";

const char* fShaderSourceTexture =
"#version 330 core\n"
"in vec2 texCoord;\n"
"out vec4 FragColor;\n"
"uniform sampler2D tex;\n"
"void main(){\n"
"	FragColor = texture(tex, texCoord);\n"
"	//FragColor = vec4(0.1f, 0.0f, 0.8f, 1.0f);\n"
"}\n\0";

void Shader::bind() {
	glUseProgram(this->shaderProgram);
}

void Shader::setUniform4x4f(const std::string& name, const glm::mat4& target) const {
	glUniformMatrix4fv(glad_glGetUniformLocation(this->shaderProgram, "u_MVP"), 1, GL_FALSE, &target[0][0]);
}

unsigned int Shader::programID() {
	return this->shaderProgram;
}

bool getGlShaderStatus(int program) {
	glValidateProgram(program);

	int success;

	glGetProgramiv(program, GL_VALIDATE_STATUS, &success);

	if (!success) {
		std::cerr << "Shader program " << program << " failed to compile: " << std::endl;

		int length;
		char* status = new char[512];

		glGetShaderInfoLog(program, 512, &length, status);

		std::cout << status << std::endl;
	}

	return (bool)success;
}

Shader::Shader(const std::string& vertexShaderTarget, const std::string& fragmentShaderTarget) {
	// Load and link an opengl shader from the vertex and fragment shaders specified as parameters.
	// Params should be paths to the files.

	std::ifstream vertexShaderSourceFile, fragmentShaderSourceFile;
	std::string vs_string, fs_string, vertexShortName, fragmentShortName;
	const char* vs_source;
	const char* fs_source;
	bool vertexCompiled, fragmentCompiled, linkSuccessful;

	int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	std::cout << "Loading vertex shader (" << vertexShader << "): " << vertexShaderTarget << std::endl;

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

	vertexCompiled = getGlShaderStatus(vertexShader);

	// Create fragment shader

	int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	std::cout << "Loading fragment shader (" << fragmentShader << "): " << fragmentShaderTarget << std::endl;

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

	fragmentCompiled = getGlShaderStatus(fragmentShader);

	// Link shaders

	this->shaderProgram = glCreateProgram();

	std::cout << "Creating shader program (" << this->shaderProgram << ")" << std::endl;

	if (vertexCompiled && fragmentCompiled) {
		glAttachShader(this->shaderProgram, vertexShader);
		glAttachShader(this->shaderProgram, fragmentShader);
		glLinkProgram(this->shaderProgram);

		linkSuccessful = getGlShaderStatus(this->shaderProgram);
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
