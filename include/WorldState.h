#pragma once

#include <string>

// Small struct shared once accross runtime to track world state necessary to the renderer
struct _WorldState {
	int windowX;
	int windowY;

	int rendererX;
	int rendererY;

	float ambientLight[3];

	std::string projectRoot;
	std::string trackDataRoot;
};
