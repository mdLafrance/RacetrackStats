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

Shader::Shader(const std::string& target) {
	// TODO: ACTUAL SHADER LOADING FROM SOURCE FILE

	const char* fShaderSource = fShaderSourceTexture;
	if (target == "default") {
		fShaderSource = fShaderSourceBasic;
	}

	// Compile and bind vertex shader
	// build and compile our shader program
	// ------------------------------------
	// vertex shader
	int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vShaderSource, NULL);
	glCompileShader(vertexShader);
	// check for shader compile errors
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// fragment shader
	int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fShaderSource, NULL);
	glCompileShader(fragmentShader);
	// check for shader compile errors
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// link shaders
	this->shaderProgram = glCreateProgram();
	glAttachShader(this->shaderProgram, vertexShader);
	glAttachShader(this->shaderProgram, fragmentShader);
	glLinkProgram(this->shaderProgram);
	// check for linking errors
	glGetProgramiv(this->shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(this->shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	int linkSuccess;
	char linkInfoLog[512];

	glGetProgramiv(this->shaderProgram, GL_LINK_STATUS, &linkSuccess);
	if (!linkSuccess) {
		glGetProgramInfoLog(this->shaderProgram, 512, NULL, linkInfoLog);
		std::cout << "Link errors: " << linkInfoLog << std::endl;
	}
}

Shader::~Shader() {
	glDeleteShader(this->shaderProgram);
}
