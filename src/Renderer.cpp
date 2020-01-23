#include <Renderer.h>

void Renderer::getWindowDimensions(int* width, int* height){
    *(width) = this->windowX;
    *(height) = this->windowY;
}

void Renderer::updateWindowDimensions(int width, int height){
    this->windowX = width;
    this->windowY = height;
}

void Renderer::updateWindowDimensions(GLFWwindow* window, int width, int height){
    this->windowX = width;
    this->windowY = height;
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

void Renderer::setMainCamera(const std::string& id){
    try {
        this->mainCamera = this->cameras.at(id);
    }
    catch (const std::out_of_range& oor) {
        std::cerr << "Camera " << id << " not registered" << std::endl;
    }
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
}

void Renderer::start() {
	/*
		Main Render loop.
		Renders available meshes, handles input.
	*/
	glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	std::cout << "Renderer starting..." << std::endl;

	this->meshes = OBJ::load("D:/projects/repos/GLTest/models/alien.obj");
	//this->meshes = OBJ::load("D:/projects/repos/GLTest/test/mosport_vehicles.obj");

	//auto spaceship = OBJ::load("D:/projects/repos/GLTest/models/spaceship.obj");

	for (auto m : meshes) {
		this->registerMesh(m.first, m.second);
	}

	OBJMesh* mesh;
	std::string materialName;
	Material* material;

	bool isWireframe = false;

	float rotation[2] = { 0.0f, 0.0f };

	float scale = 1.0f;

	float speed = 2.0f;
	float rspeed = 0.05f;
	float scaleSpeed = 1.1f;

	while (!glfwWindowShouldClose(this->window)) {
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(this->window, true);
		}

		float inputs[2] = { 0.0f, 0.0f };

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			inputs[1] += speed;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			inputs[0] -= speed;
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			inputs[1] -= speed;
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			inputs[0] += speed;
		}
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
			rotation[1] -= rspeed;
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
			rotation[0] -= rspeed;
		}
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
			rotation[1] += rspeed;
		}
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
			rotation[0] += rspeed;
		}
		if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS) {
			scale /= scaleSpeed;
		}
		if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS) {
			scale *= scaleSpeed;
		}
		if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
			if (isWireframe) {
				isWireframe = false;
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
			else {
				isWireframe = true;
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
		}

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		auto mat = glm::rotate(rotation[0], glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(rotation[1], glm::vec3(1.0f, 0.0f, 0.0f)) * glm::scale(glm::vec3(scale, scale, scale));

		for (auto m : this->meshes) {
			mesh = m.second;
			materialName = mesh->getDefaultMaterialName();

			std::string target = "default";
			if (this->materials.count(materialName) == 1) {
				target = materialName;
			}

			material = this->materials.at(target);
			
			this->textures.at(material->texture)->bind();
			
			Shader* shader = this->shaders.at(material->shader);
			shader->bind();

			glm::mat4 mvp = this->mainCamera->projectionViewMatrix() * mat;

			shader->setUniform4x4f("u_MVP", mvp);

			mesh->draw();
		}

		glfwSwapBuffers(this->window);
		glfwPollEvents();
	}
}

Renderer::Renderer(GLFWwindow* window) {
	this->window = window;

	glfwGetWindowSize(window, &this->windowX, &this->windowY);
	std::cout << "Window with width height: " << this->windowX << ' ' << this->windowY << std::endl;
	
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "Failed to initialize GLAD\n";
	}

	glViewport(0, 0, this->windowX, this->windowY);

	//glfwSetFramebufferSizeCallback(window, &this->updateWindowDimensions);

	// Create and register default assets.
	Camera* defaultCam = new Camera(0, this->windowX, 0, this->windowY, -600, 600);
	this->registerCamera("default", defaultCam);
	this->setMainCamera("default");

	Shader* defaultShader = new Shader("default");
	Texture* defaultTexture = new Texture("default");
	Material* defaultMaterial = new Material("default", "default", "default");

	this->registerShader("default", defaultShader);
	this->registerTexture("default", defaultTexture);
	this->registerMaterial("default", defaultMaterial);

	std::cout << "Renderer initialized." << std::endl;
}

void Renderer::tick(const double& dTime) {
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	int translation[2] = { 0,0 };

	// Collect input
	if (glfwGetKey(this->window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(this->window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		translation[1] = 1;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		translation[0] = -1;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		translation[1] = -1;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		translation[0] = 1;
	}

	this->mainCamera->translate(translation[0], translation[1], 0);

	// For now, only use default material/shaders

	Shader* shader = this->shaders.at("default");
	shader->bind();

	glm::mat4 VP = this->mainCamera->projectionViewMatrix();

	glm::mat4 transform = glm::rotate(0.8f * (float)(glfwGetTime()), glm::vec3(0.f, 1.0f, 0.f));

	shader->setUniform4x4f("MVP", VP * transform);

	std::string objectName;
	Object* object;
	for (auto p : this->objects) {
		objectName = p.first;
		object = p.second;

		object->mesh->draw();
	}

	// Draw registered objects
	// for (auto p : this->meshes) {
	// 	OBJMesh* mesh = p.second;
	// 	Material* mat = this->materials.at("default");
	// 	Shader* shader = this->shaders.at(mat->shader);
	// 	Texture* tex = this->textures.at(mat->texture);
	// 	shader->bind();
	// 	tex->bind();

	// 	// TODO: Move this to transform component for objects
	// 	glm::mat4 transform = glm::rotate(0.8f * (float)(glfwGetTime()), glm::vec3(0.f, 1.0f, 0.f));

	// 	glm::mat4 proj = this->mainCamera->projectionViewMatrix();

	// 	shader->setUniform4x4f("MVP", proj * transform);

	// 	mesh->draw();
	// }

	glfwSwapBuffers(this->window);
	glfwPollEvents();
}

Renderer::~Renderer() {}

void Renderer::loadScene(const std::string& target) {
	std::ifstream f;
	f.open(target);

	if (!f.is_open()) {
		std::cerr << "Could not open target scene for loading: " << target << std::endl;
		return;
	}

	std::cout << std::endl << "Loading Scene " << target << "..." << std::endl;

	this->scene.name = Utils::getFileInfo(target).file;
	this->scene.path = target;
	this->scene.files = std::vector<std::string>();

	std::string line;

	while (std::getline(f, line)) {
		if (line == "" || line[0] == '#') continue;

		std::vector<std::string> tokens = Utils::split(line, ' ');
		std::string lineType = tokens[0];

		// Other line specs deprecated
		// Leaving this functionality in for later

		if (lineType == "name"){
			int nameLength = line.size() - 5; // "name " is 5 chars
	 		this->scene.name = line.substr(5, nameLength);		
		// Line is just path to some file to be included in the scene
		} else { 
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
		// o->material = this->materials.at(p.second->getDefaultMaterialName);
		o->material = this->materials.at("default");
	}

	// Now that all necessary Objects have been generated, link parents

	Object* o;
	for (auto p : this->objects){
		o = p.second;
		std::string targetParent = o->mesh->getDefaultParentName();

		if (targetParent != "") {
			try {
				o->transform->setParent(this->objects.at(targetParent)->transform);
			}
			catch (const std::out_of_range & oor) {
				std::cerr << "Can't find parent " << "<" << targetParent << ">" << " for object " << p.first << std::endl;
			}
		}
	}
}

void Renderer::loadMaterialLibrary(const std::string& target) {
	std::cout << "Loading material library " << target << std::endl;
	Utils::FileInfo fi = Utils::getFileInfo(target);
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

	this->objects[name] = object;

	return object;
}
