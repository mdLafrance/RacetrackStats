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
	static const float PI = 3.14159265359;
	static const float DEG2RAD = 0.0174533;
	static const float RAD2DEG = 57.2958;

	typedef struct { // D:/foo/bar/baz.py
		std::string directory; // D:/foo/bar
		std::string file;      // baz
		std::string extension; // py
	} FileInfo;

	// String manipulation utility
	std::vector<std::string> split(const std::string& s, char delimiter);
	std::string getFileNameNoExtension(const std::string& target);
	std::string replace(const std::string& s, const char& a, const char& b);
	std::string trimWhitespace(const std::string& s);
	bool hasEnding(const std::string& a, const std::string& b);
	int stringCount(char* s, const char& c);

	// File utility
	Utils::FileInfo getFileInfo(const std::string& target);
	long getFileSize(const std::string& target);

	// Opengl Callbacks
	void glfwErrorCallbackFunction(int ecode, const char* info);

	// Utility class to time lengths of functions 
	class StopWatch {
		std::chrono::steady_clock::time_point origin;
		std::chrono::steady_clock::time_point latest;

	public:
		std::chrono::milliseconds lap();
		std::string lap_s(); // Return string representation of current lap time 

		std::chrono::milliseconds total();
		std::string total_s(); // Return string representation of current run time

		void start(); // Reset stopwatch values

		StopWatch();
		~StopWatch();
	};
}