#include <Camera.h>

void Camera::setFOV(const float& fov) {
	this->FOV = fov;
}

void Camera::setFarClipPlane(const float& fcp) {
	assert(fcp > CAMERA_DEFAULT_NEAR_CLIP_PLANE && "Far clip plane is intersecting near clip plane");

	this->farClipPlane = fcp;
}

CameraType Camera::getType() {
	return this->type;
}

glm::mat4 Camera::projectionViewMatrix(const bool& local) {
	glm::mat4 camTransform = local ? this->transform->getLocalMatrix() : this->transform->getMatrix();

	glm::vec3 e = glm::vec3(glm::column(camTransform, 3));
	glm::vec3 x = camTransform * glm::vec4(1, 0, 0, 0);
	glm::vec3 y = camTransform * glm::vec4(0, 1, 0, 0);
	glm::vec3 z = camTransform * glm::vec4(0, 0, 1, 0);

	glm::mat4 view = {
		glm::vec4(x[0], y[0], z[0], 0),
		glm::vec4(x[1], y[1], z[1], 0),
		glm::vec4(x[2], y[2], z[2], 0),
		glm::vec4(-glm::dot(x, e), -glm::dot(y, e), -glm::dot(z, e), 1)

	};

	glm::mat4 proj;

	if (this->type == CameraType::Orthographic) {
		int halfX = WorldState.windowX / 2;
		int halfY = WorldState.windowY / 2;

		proj = glm::ortho(-halfX, halfX, -halfY, halfY, -100, 1000);
	}

	if (this->type == CameraType::Perspective) {
		proj = glm::perspective(this->FOV, (float)WorldState.rendererX / (float)WorldState.rendererY, CAMERA_DEFAULT_NEAR_CLIP_PLANE, this->farClipPlane);
	}

	return proj * view;
}

Camera::Camera(const CameraType& type) {
	this->type = type;

	if (type == CameraType::Perspective) {
		this->FOV = CAMERA_DEFAULT_FOV;
	}

	this->farClipPlane = CAMERA_DEFAULT_FAR_CLIP_PLANE;

	this->transform = new Transform();
}

Camera::Camera(const float& fov, const float& zMin, const float& zMax) {
	// Constructor for perspective camera
	this->type = CameraType::Perspective;

	this->FOV = fov;
	this->farClipPlane = zMax;

	this->transform = new Transform();
}

Camera::Camera(const float& xMin, const float& xMax, const float& yMin, const float& yMax, const float& zMin, const float& zMax) {
	// Perspective for orthographic camera 

	this->type = CameraType::Orthographic;

	this->transform = new Transform();
}

Camera::~Camera() {
	delete this->transform;
}
