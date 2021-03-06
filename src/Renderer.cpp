#include <Renderer.h>

void Renderer::registerTexture(const std::string& id, Texture* texture){
    if (this->textures.count(id) == 0){
        this->textures[id] = texture;
		std::cout << "Registered Texture " << id << std::endl;
    } else {
        std::cerr << "ERROR: Texture " << id << " already registered." << std::endl;
    }
}

void Renderer::registerShader(const std::string& id, Shader* shader){
    if (this->shaders.count(id) == 0){
        this->shaders[id] = shader;
		std::cout << "Registered Shader " << id << " (ID: " << shader->programID() << ")" << std::endl;
    } else {
        std::cerr << "ERROR: Shader " << id << " already registered." << std::endl;
    }
}

void Renderer::registerMesh(const std::string& id, OBJMesh* mesh){
    if (this->meshes.count(id) == 0){
        this->meshes[id] = mesh;
		std::cout << "Registered Mesh " << id << std::endl;
    } else {
        std::cerr << "ERORR: Mesh " << id << " already registered." << std::endl;
    }
}

void Renderer::registerMaterial(const std::string& id, Material* material){
    if (this->materials.count(id) == 0){
        this->materials[id] = material;
		std::cout << "Registered Material " << id << " (shader: " << material->shader->programID() << ")" << std::endl;
    } else {
        std::cerr << "ERROR: Material " << id << " already registered." << std::endl;
    }
}

void Renderer::registerCamera(const std::string& id, Camera* camera){
    if (this->cameras.count(id) == 0){
        this->cameras[id] = camera;
		std::cout << "Registered Camera " << id << std::endl;
    } else {
        std::cerr << "ERROR: Camera " << id << " already registered." << std::endl;
    }
}

void Renderer::registerSkybox(const std::string& id, Skybox* skybox){
    if (this->skyboxes.count(id) == 0){
        this->skyboxes[id] = skybox;
		std::cout << "Registered Skybox " << id << std::endl;
    } else {
        std::cerr << "ERROR: Skybox " << id << " already registered." << std::endl;
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
        std::cerr << "ERROR: Camera " << id << " not registered" << std::endl;
    }
}

void Renderer::setSkybox(const std::string& id){
    try {
        this->skybox = this->skyboxes.at(id);
    }
    catch (const std::out_of_range& oor) {
        std::cerr << "ERROR: Skybox " << id << " not registered" << std::endl;
    }
}

void Renderer::drawLine(const glm::vec3& origin, const glm::vec3& end, const glm::vec3& color, bool drawOver) {
	glm::mat4 VP = this->mainCamera->projectionViewMatrix();

	Shader* lineShader = this->shaders.at("line");
	lineShader->bind();

	float line[6];
	memcpy(line, &origin[0], 3 * sizeof(float));
	memcpy(line + 3, &end[0], 3 * sizeof(float));

	glDisable(GL_CULL_FACE);

	if (drawOver) glDisable(GL_DEPTH_TEST);

	GLuint lineVBO, lineVAO;

	glGenBuffers(1, &lineVBO);
	glGenVertexArrays(1, &lineVAO);

	glBindVertexArray(lineVAO);
	glBindBuffer(GL_ARRAY_BUFFER, lineVBO);

	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), line, GL_STREAM_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	lineShader->setUniform("u_MVP", VP);
	lineShader->setUniform("u_color", color);

	glDrawArrays(GL_LINES, 0, 2);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDeleteBuffers(1, &lineVBO);
	glDeleteVertexArrays(1, &lineVAO);

	glEnable(GL_CULL_FACE);
	if (drawOver) glEnable(GL_DEPTH_TEST);
}

void Renderer::deleteObjects() {
	// TODO: this doesn't quite work
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

Renderer::Renderer(GLFWwindow* window) {
	std::cout << "Initializing renderer..." << std::endl << std::endl;

	this->window = window;

	this->resetData();

	// Get line width parameters from opengl
	float lineWidthBounds[2];
	glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, lineWidthBounds);
	this->lineWidthMax = lineWidthBounds[1];
	glLineWidth(lineWidthBounds[0]);

	std::cout << "Renderer initialized." << std::endl;
}

void Renderer::resetData() {
	// Create and register default assets.

	// Default camera is perspective camera
	Camera* defaultCam = new Camera();

	Shader* defaultShader = new Shader("default", "default");

	Shader* diffuseShader = new Shader(
		std::string(WorldState.projectRoot) + "/resources/shaders/diffuse.vert", 
		std::string(WorldState.projectRoot) + "/resources/shaders/diffuse.frag"
	);

	Shader* lineShader = new Shader(
		std::string(WorldState.projectRoot) + "/resources/shaders/line.vert", 
		std::string(WorldState.projectRoot) + "/resources/shaders/line.frag"
	);

	Shader* skyboxShader = new Shader(
		std::string(WorldState.projectRoot) + "/resources/shaders/skybox.vert", 
		std::string(WorldState.projectRoot) + "/resources/shaders/skybox.frag"
	);

	Texture* defaultTexture = new Texture("default");

	Material* defaultMaterial = new Material("default");
	defaultMaterial->shader = defaultShader;

	std::vector<std::string> skyboxFaces = {
		std::string(WorldState.projectRoot) + "/resources/textures/default_skybox+X.png",
		std::string(WorldState.projectRoot) + "/resources/textures/default_skybox-X.png",
		std::string(WorldState.projectRoot) + "/resources/textures/default_skybox+Y.png",
		std::string(WorldState.projectRoot) + "/resources/textures/default_skybox-Y.png",
		std::string(WorldState.projectRoot) + "/resources/textures/default_skybox+Z.png",
		std::string(WorldState.projectRoot) + "/resources/textures/default_skybox-Z.png",
	};

	Skybox* defaultSkybox = new Skybox(skyboxFaces);

	this->registerShader("diffuse", diffuseShader);
	this->registerShader("line", lineShader);
	this->registerShader("skybox", skyboxShader);
	this->registerShader("default", defaultShader);

	this->registerCamera("default", defaultCam);
	this->registerTexture("default", defaultTexture);
	this->registerMaterial("default", defaultMaterial);
	this->registerSkybox("default", defaultSkybox);

	this->setMainCamera("default");
	this->setSkybox("default");

	this->numOfLights = 0;

	this->frameCount = 0;
}

Renderer::~Renderer() {
	this->deleteObjects();
}

void Renderer::loadScene(const std::string& target) {
	std::ifstream f;
	f.open(target);

	if (!f.is_open()) {
		std::cerr << "ERROR: Could not open target scene file: " << target << std::endl;
		return;
	}

	this->loading = true;

	Utils::StopWatch timer;

	std::cout << std::endl << "Loading Scene " << target << "..." << std::endl;

	this->scene.name = Utils::getFileInfo(target).file;
	this->scene.path = target;
	this->scene.files = std::vector<std::string>();

	int lines = Utils::getLines(target);
	std::string line;
	this->progress = 0.0f;
	float progressStep = 1.0f / lines;

	int lineNumber = 0;

	timer.start();

	while (std::getline(f, line)) {
		++lineNumber;

		this->progress += progressStep;

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
				std::cerr << "ERROR: Too many lights attempting to be created for scene. (Max light count is " << MAX_LIGHT_COUNT << ")" << std::endl;
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

		// Define ambient light for the scene (rgb)
		else if (lineType == "ambient") {
			::WorldState.ambientLight[0] = std::atof(tokens[1].c_str());
			::WorldState.ambientLight[1] = std::atof(tokens[2].c_str());
			::WorldState.ambientLight[2] = std::atof(tokens[3].c_str());
		}

		// Define a mesh to be loaded into the scene (only obj for now)
		else if (lineType == "mesh") {
			std::string fullFileName = std::string(WorldState.trackDataRoot)+ "/" + tokens[tokens.size() - 1]; // last item in the line is the name of the mat file

			std::cout << "Loading " << fullFileName << std::endl;

			std::vector<OBJMesh*> meshes = OBJ::load(fullFileName);

			for (OBJMesh* m : meshes) {
				this->registerMesh(m->getMeshName(), m);
			}
		}

		// Define a material file to be loaded into the scene
		else if (lineType == "material"){
			std::string fullFileName = std::string(WorldState.trackDataRoot)+ "/" + tokens[tokens.size() - 1]; // last item in the line is the name of the mat file

			// Determine if the material is to be loaded with any flags
			uint32_t materialFlags = 0;

	 		if (tokens.size() > 2) { // more than 'material foo/bar/material.mtl' indicates flags are present on the line
				std::vector<std::string> flags(tokens.begin() + 1, tokens.end() - 1);

				for (std::string flag : flags) { // In case more flags in the future (ex half res textures, no mipmaps etc.)
					if (flag == "-t") { // Material has transparency
						materialFlags |= MATERIAL_TRANSPARENT;
					}
				}
			}

			std::cout << "Loading " << fullFileName << std::endl;

			std::vector<Material*> materials = MTL::load(fullFileName);

			for (Material* m : materials) {
				m->addFlag(materialFlags);
				m->shader = this->shaders.at("diffuse");
				
				this->registerMaterial(m->name, m);
			}
		}

		else {
			std::cerr << "ERROR: Unsupported definition (line " << lineNumber << ") : " << line << std::endl;
		}
	}

	// All necessary files loaded, now link all resources into "Objects" to be rendered.
	Object* o;
	OBJMesh* m;

	for (auto p : this->meshes){
		o = this->newObject(p.first); // generates new transform

		m = p.second;
		m->generateBuffers();

		o->mesh = m;

		// Link loaded materials to mesh triangles
		// Map access is super slow, so caching the pointer here to shave a lot of time off in tick()
		for (int i = 0; i < m->faceMaterials.size(); i++){
			OBJ::FaceMaterials* fm = m->faceMaterials.data() + i;
			try {
				fm->material = this->materials.at(fm->materialName);
			}
			catch (const std::out_of_range & oor) {
				fm->material = this->materials.at("default");
				std::cerr << "ERROR: Couldn't find material <" << fm->materialName << "> for mesh <" << m->getMeshName() << ">" << std::endl;
			}
		}
	}

	// Now that all necessary Objects have been generated with their meshes, link parents 
	// This step needs to happen after the meshes are organized into objects, so that they have the relevant transforms to parent
	for (Object* o : this->objects){
		std::string targetParent = o->mesh->getDefaultParentName();

		if (targetParent != "") {
			Object* parentObject = this->getObject(targetParent);
			
			if (parentObject == nullptr) {
				std::cerr << "ERROR: Can't find parent " << "<" << targetParent << ">" << " for object " << o->name << std::endl;
			} else {
				o->transform->setParent(parentObject->transform);
			}
		}
	}

	std::cout << "Finished loading scene " << this->scene.name << " (" << timer.lap_s() << ")" << std::endl;

	this->loading = false;
}

Object* Renderer::newObject(const std::string& name) {
	if (this->getObject(name) != nullptr) {
		std::cerr << "ERROR: Object " << name << " has already been created." << std::endl;
		return nullptr;
	}

	Object* object = new Object(name);
	
	object->transform = new Transform();

	this->objects.push_back(object);

	return object;
}

// TODO: Changed this from std map because it was causing serious performance issues while iterating through elements
// In future, maybe associate a map with this for access, or have this vector be sorted
Object* Renderer::getObject(const std::string& name) {
	for (Object* o : this->objects) {
		if (o->name == name) return o;
	}

	return nullptr;
}

void Renderer::tick(const double& dTime) {
	++this->frameCount;

	// Get light matrices (fast if they havent changed)
	for (int i = 0; i < this->numOfLights; i++) {
		*(this->lightMatrices + i) = (this->lights + i)->getMatrix();
	}

	Transform* camTransform = this->mainCamera->transform;

	// VP matrix for this frame
	glm::mat4 VP = this->mainCamera->projectionViewMatrix();

	// matrix which only captures the translation to the camera's position 
	glm::mat4 cameraPositionTransform = glm::translate(glm::vec3(glm::column(camTransform->getMatrix(), 3)));

	if (this->skybox != nullptr){
		// Force a perspective VP for skybox drawing
		glm::mat4 _VP = glm::perspective(this->mainCamera->getFOV(), (float)WorldState.rendererX / WorldState.rendererY, 0.01f, 1000.0f) * this->mainCamera->viewMatrix();

		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);

		Shader* skyboxShader = this->shaders.at("skybox");

		skyboxShader->bind();

		skyboxShader->setUniform("MVP", _VP * cameraPositionTransform);

		this->skybox->draw();

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
	}

    Shader* shader;
    OBJMesh* mesh;

	glm::mat4 objectTransform;

    for (Object* object : this->objects) {
    	mesh = object->mesh;

		objectTransform = object->transform->getMatrix();
    
		mesh->bind();

    	for (int i = 0; i < mesh->faceMaterials.size(); i++){
			OBJ::FaceMaterials& mat = mesh->faceMaterials[i];

			mat.material->bind(); // Binds shader

			shader = mat.material->shader;

    	    shader->setLights(this->numOfLights, this->lightMatrices);
    	    shader->setUniform("Ka", { WorldState.ambientLight[0], WorldState.ambientLight[1], WorldState.ambientLight[2] });

    	    shader->setUniform("M", objectTransform);
    	    shader->setUniform("VP", VP);
    	    shader->setUniform("MVP", VP * objectTransform);
    
    		mesh->drawRange(mat.range[0], mat.range[1] - mat.range[0]);
    	}
    }

	// Draw queued lines (if any)
	// TODO: optimization: line shader should be bound once for all these lines
	for (LineData line : this->linesToDraw){
		this->drawLine(line.origin, line.end, line.color, line.drawOver);
	}

	this->linesToDraw.clear();
}
 
void Renderer::addLine(const glm::vec3& origin, const glm::vec3& end, const glm::vec3& color, bool drawOver) {
	this->linesToDraw.push_back({ origin, end, color, drawOver });
}

void Renderer::setLineWidth(const float& w) {
	if (1 <= w <= this->lineWidthMax) glLineWidth(w);
}
