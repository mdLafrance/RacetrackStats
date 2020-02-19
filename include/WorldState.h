#pragma once

struct _WorldState {
	int windowX;
	int windowY;

	float ambientLight[3];

	const char* projectRoot;
	const char* trackDataRoot;
};
