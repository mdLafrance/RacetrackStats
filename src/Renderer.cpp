#include <Renderer.h>

std::string vec3ToString(const glm::vec3& v) {
	char s[32];
	sprintf(s, "[%.3f, %.3f, %.3f]", v[0], v[1], v[2]);

	return std::string(s);
}

void Renderer::registerTexture(const std::string& id, Texture* texture){
    if (this->textures.count(id) == 0){
        this->textures[id] = texture;
		std::cout << "Registered Texture " << id << std::endl;
    } else {
        std::cerr << "Texture " << id << " already registered." << std::endl;
    }
}

void Renderer::registerShader(const std::string& id, Shader* shader){
    if (this->shaders.count(id) == 0){
        this->shaders[id] = shader;
		std::cout << "Registered Shader " << id << std::endl;
    } else {
        std::cerr << "Shader " << id << " already registered." << std::endl;
    }
}

void Renderer::registerMesh(const std::string& id, OBJMesh* mesh){
    if (this->meshes.count(id) == 0){
        this->meshes[id] = mesh;
		std::cout << "Registered Mesh " << id << " (Material " << mesh->getDefaultMaterialName() << ")" << std::endl;
    } else {
        std::cerr << "Mesh " << id << " already registered." << std::endl;
    }
}

void Renderer::registerMaterial(const std::string& id, Material* material){
    if (this->materials.count(id) == 0){
        this->materials[id] = material;
		std::cout << "Registered Material " << id << std::endl;
    } else {
        std::cerr << "Material " << id << " already registered." << std::endl;
    }
}

void Renderer::registerCamera(const std::string& id, Camera* camera){
    if (this->cameras.count(id) == 0){
        this->cameras[id] = camera;
		std::cout << "Registered Camera " << id << std::endl;
    } else {
        std::cerr << "Camera " << id << " already registered." << std::endl;
    }
}

Camera* Renderer::getMainCamera() {
	return this->mainCamera;
}

void Renderer::setMainCamera(const std::string& id){
    try {
        this->mainCamera = this->cameras.at(id);
    }
    catch (const std::out_of_range& oor) {
        std::cerr << "Camera " << id << " not registered" << std::endl;
    }
}

void Renderer::drawLine(const glm::vec3& origin, const glm::vec3& end, const glm::vec4& color) {
	// Draw a line from origin to end with given color

	float vertices[6] = {
		origin[0], origin[1], origin[2],
		end[0], end[1], end[2]
	};

	float triangle[] = {
		-0.5, 0.5, 0,
		-0.5, -0.5, 0,
		0.5, 0, 0

	};

	Shader* shader = this->shaders.at("line");
	shader->bind();
	shader->setUniformMatrix4fv("MVP", glm::mat4());

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STREAM_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	glDeleteBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Renderer::deleteObjects() {
	for (auto k : this->textures) {
		delete k.second;
	}

	for (auto k : this->shaders) {
		delete k.second;
	}

	for (auto k : this->meshes) {
		delete k.second;
	}

	for (auto k : this->materials) {
		delete k.second;
	}

	for (auto k : this->cameras) {
		delete k.second;
	} 

	for (auto k : this->objects){
		delete k.second;
	}
}

Renderer::Renderer(GLFWwindow* window) {
	this->window = window;

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "Failed to initialize GLAD\n";
	}

	this->resetData();

	//glEnable(GL_CULL_FACE);
	glDisable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glCullFace(GL_CW);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	Texture* test = new Texture("D:/Hacking/RacetrackStats/Mosport/texture/mosport_objs_2.dds");

	std::cout << "Renderer initialized." << std::endl;
}

void Renderer::resetData() {
	// Create and register default assets.
	// Default is persp cam
	Camera* defaultCam = new Camera(45.0f, 1.0f /*WINDOW_DEFAULT_X/WINDOW_DEFAULT_Y*/, 0, 2000);
	defaultCam->transform->setTranslation(glm::vec3(0, 0, -100));

	Shader* defaultShader = new Shader("default", "default");
	Shader* diffuseShader = new Shader(
		std::string(WorldState.projectRoot) + "/resources/shaders/diffuse.vert", 
		std::string(WorldState.projectRoot) + "/resources/shaders/diffuse.frag"
	);
	Shader* lineShader = new Shader("default", "line");

	Texture* defaultTexture = new Texture("default");

	Material* defaultMaterial = new Material("default");
	defaultMaterial->shader = defaultShader;

	this->registerCamera("default", defaultCam);
	this->registerShader("default", defaultShader);
	this->registerShader("diffuse", diffuseShader);
	this->registerShader("line", lineShader);
	this->registerTexture("default", defaultTexture);
	this->registerMaterial("default", defaultMaterial);

	this->setMainCamera("default");

	this->numOfLights = 0;
}

Renderer::~Renderer() {
	delete[] this->lights;
	delete[] this->lightMatrices;

	this->deleteObjects();
}

void Renderer::loadScene(const std::string& target) {
	std::ifstream f;
	f.open(target);

	if (!f.is_open()) {
		std::cerr << "Could not open target scene for loading: " << target << std::endl;
		return;
	}

	std::cout << "Loading Scene " << target << "..." << std::endl;

	this->scene.name = Utils::getFileInfo(target).file;
	this->scene.path = target;
	this->scene.files = std::vector<std::string>();

	std::string line;

	while (std::getline(f, line)) {
		if (line == "" || line[0] == '#') continue;

		std::vector<std::string> tokens = Utils::split(line, ' ');
		std::string lineType = tokens[0];

		// Name of scene
		if (lineType == "name") {
			int nameLength = line.size() - 5; // "name " is 5 chars
			this->scene.name = line.substr(5, nameLength);
		}
		// Light to be used in scene
		else if (lineType == "light") {
			if (this->numOfLights >= MAX_LIGHT_COUNT) {
				std::cerr << "Too many lights attempting to be created for scene. (Max light count is " << MAX_LIGHT_COUNT << ")" << std::endl;
				continue;
			}

			std::string word;

			int k = 1;

			Light light = {tokens[k++] == "directional" ? LightType::DIRECTIONAL : LightType::POINT};

			while (k < tokens.size()){
				word = tokens[k++];	

				if (word == "-p" || word == "-d" || word == "-v"){
					light.setPointDirection({ std::atof(tokens[k++].c_str()), std::atof(tokens[k++].c_str()), std::atof(tokens[k++].c_str()) });
				}

				if (word == "-c"){
					light.setColor({ std::atof(tokens[k++].c_str()), std::atof(tokens[k++].c_str()), std::atof(tokens[k++].c_str()) });
				}

				if (word == "-i"){
					light.setIntensity(std::atof(tokens[k++].c_str()));
				}

				if (word == "-i" || word == "-f"){
					light.setK(std::atof(tokens[k].c_str()));
				}
			}

			std::cout << "Created new " << tokens[1] << " light." << std::endl;

			this->lights[this->numOfLights++] = light;
		}
		else if (lineType == "ambient") {
			::WorldState.ambientLight[0] = std::atof(tokens[1].c_str());
			::WorldState.ambientLight[1] = std::atof(tokens[2].c_str());
			::WorldState.ambientLight[2] = std::atof(tokens[3].c_str());
		}
		// Line is just path to some file to be included in the scene
		else { 
			Utils::FileInfo fi = Utils::getFileInfo(line);

			if (fi.extension == "mtl"){ // Load mtl library file
				this->scene.files.push_back(line);
				this->loadMaterialLibrary(line);
			} else if (fi.extension == "obj") { // Load obj file
				this->scene.files.push_back(line);
				this->loadOBJ(line);
			} else if (fi.extension == "vert") { // Load vertex shader
				std::cout << "Loading vertex shader...";
			} else if (fi.extension == "frag") { // Load fragment shader
				std::cout << "Loading fragment shader...";
			} else {
				std::cerr << "Unsupported file type " << fi.extension << " for file " << line << std::endl;
			}
		}
	}

	// All necessary files loaded, now link all resources into Objects to be rendered.

	for (auto p : this->meshes){
		Object* o = this->newObject(p.first);

		o->mesh = p.second;
	}

	// Now that all necessary Objects have been generated with their meshes, link parents and materials
	Object* o;
	for (auto p : this->objects){
		o = p.second;
		std::string targetParent = o->mesh->getDefaultParentName();
		std::string targetMaterial = o->mesh->getDefaultMaterialName();

		if (targetParent != "") {
			try {
				o->transform->setParent(this->objects.at(targetParent)->transform);
			}
			catch (const std::out_of_range & oor) {
				std::cerr << "Can't find parent " << "<" << targetParent << ">" << " for object " << p.first << std::endl;
			}
		}

		if (targetMaterial != ""){
			try {
				o->material = this->materials.at(targetMaterial);
			} catch (const std::out_of_range& oor){
				std::cerr << "Can't find material " << "<" << targetMaterial << "> for object " << p.first << std::endl;
			}
		} else {
			o->material = this->materials.at("default");
		}
	}
}

void Renderer::loadMaterialLibrary(const std::string& target) {
	std::cout << "Loading material library " << target << std::endl;
	Utils::FileInfo fi = Utils::getFileInfo(target);

	std::map<std::string, Material*> materials = Material::load(target);

	for (auto p : materials) {
		p.second->shader = this->shaders.at("diffuse");
		this->registerMaterial(p.first, p.second);
	}
}

void Renderer::loadOBJ(const std::string& target){
	std::cout << "Loading OBJ file " << target << std::endl;

	std::map<std::string, OBJMesh*> meshes = OBJ::load(target);

	for (auto p : meshes){
		this->registerMesh(p.first, p.second);
	}
}

Object* Renderer::newObject(const std::string& name) {
	if (this->objects.count(name) != 0) {
		std::cerr << "Object " << name << " already exists." << std::endl;
		return nullptr;
	}

	Object* object = new Object(name);
	
	object->transform = new Transform();
	object->material = this->materials.at("default");

	this->objects[name] = object;

	return object;
}

void Renderer::tick(const double& dTime) {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float translation[3] = { 0,0,0 };
	float rotation[2] = { 0,0 };

	float translateSpeed = 1;
	float rotationSpeed = 0.05f;

	if (!std::getenv("MSI")) {
		translateSpeed *= 0.2;
		rotationSpeed *= 0.06;
	}

	// TRANSLATION
	if (glfwGetKey(this->window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(this->window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		translation[1] = translateSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		translation[0] = -translateSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		translation[1] = -translateSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		translation[0] = translateSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		translation[2] = translateSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		translation[2] = -translateSpeed;
	}

	// ROTATION 
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		rotation[1] = rotationSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		rotation[0] = rotationSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		rotation[1] = -rotationSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		rotation[0] = -rotationSpeed;
	}

	// Generate gpu-friendly matrix representation for lights 
	for (int i = 0; i < this->numOfLights; i++) {
		*(this->lightMatrices + i) = (this->lights + i)->getMatrix();
	}

	// Calcuate new MVP for camera on this frame
	Transform* camTransform = this->mainCamera->transform;

	glm::vec3 worldUp = glm::inverse(camTransform->getMatrix()) * glm::vec4(0, 1, 0, 0); // did this on a hunch, why does it work??

	camTransform->rotate(-rotation[0], worldUp);
	camTransform->rotate(-rotation[1], glm::vec3(1,0,0));

	glm::vec3 dx, dy, dz;
	if (abs(translation[0]) > 0.01) {
		dx = translation[0] * camTransform->right();
		dx.y = 0;
		dx = glm::normalize(dx);
	}
	else {
		dx = { 0,0,0 };
	}

	dy = translation[2] * glm::vec3(0, 1, 0);

	if (abs(translation[1]) > 0.01) {
		dz = -1.0f * translation[1] * camTransform->forward();
		dz.y = 0;
		dz = normalize(dz);
	}
	else {
		dz = { 0,0,0 };
	}

	camTransform->translate(dx + dy + dz);

	glm::mat4 VP = this->mainCamera->projectionViewMatrix();

	Object* object;
	Shader* shader;

	//TODO: sort objects by material, then render by material
	for (auto p : this->objects) {
		object = p.second;

		// Binding Material binds associated Shader, which sets its own internal uniforms
		object->material->bind();

		shader = object->material->shader;

		// Override mateiral ambient with world ambient for now
		glUniform3fv(glad_glGetUniformLocation(shader->programID(), "Ka"), 1, &WorldState.ambientLight[0]);
		//glUniform3fv(glad_glGetUniformLocation(shader->programID(), "cameraForward"), 1, &this->mainCamera->transform->forward()[0]);

		shader->setUniformMatrix4fv("MV", VP);
		shader->setUniformMatrix4fv("MVP", VP * object->transform->getMatrix());

		shader->setLights(this->numOfLights, this->lightMatrices);

		object->mesh->draw();
	}

	glfwSwapBuffers(this->window);
	glfwPollEvents();
}
