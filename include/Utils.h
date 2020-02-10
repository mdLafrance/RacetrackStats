#pragma once

#include <stdio.h>

#include <string>
#include <vector>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iostream>

static const char DIRECTORY_SEPARATOR = '/';

namespace Utils 
{
	//float PI = 3.14159265359;
	//float DEG2RAD = 0.0174533;
	//float RAD2DEG = 57.2958;

	typedef struct { // D:/foo/bar/baz.py
		std::string directory; // D:/foo/bar
		std::string file;      // baz
		std::string extension; // py
	} FileInfo;

	// String manipulation utility
	std::vector<std::string> split(const std::string& s, char delimiter);
	std::string getFileNameNoExtension(const std::string& target);
	bool hasEnding(const std::string& a, const std::string& b);
	std::string replace(const std::string& s, const char& a, const char& b);

	// File utility
	Utils::FileInfo getFileInfo(const std::string& target);
	long getFileSize(const std::string& target);

	// Opengl Callbacks
	void glfwErrorCallbackFunction(int ecode, const char* info);

	class StopWatch {
		std::chrono::steady_clock::time_point origin;
		std::chrono::steady_clock::time_point latest;

	public:
		std::chrono::milliseconds lap();
		std::string lap_s();

		std::chrono::milliseconds total();
		std::string total_s();

		void start();

		StopWatch();
		~StopWatch();
	};
}