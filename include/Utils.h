#pragma once

#include <stdio.h>

// The dirent.h header is supplied on POSIX machines, using a third party one in windows (used to browse filesystem)
#ifdef _WIN32
#include <dirent/dirent.h>
#else
#include <dirent.h>
#endif // _WIN32

#include <string>
#include <vector>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iostream>
#include <type_traits>

#include <imgui.h>
#include <glm/glm.hpp>

static const char DIRECTORY_SEPARATOR = '/';

// Operators

// glm
glm::vec3 operator *(float f, glm::vec3 v); // float * vector
std::ostream& operator <<(std::ostream& os, const glm::vec3& v); // print vec3

// imgui
ImVec2 operator +(const ImVec2& a, const ImVec2& b); // add vectors
std::ostream& operator <<(std::ostream& os, const ImVec2& vec); // Print vector

// Centralized utility functions
namespace Utils 
{
	static const float PI = 3.14159265359;
	static const float DEG2RAD = 0.0174533;
	static const float RAD2DEG = 57.2958;

	struct FileInfo { // D:/foo/bar/baz.py
		std::string directory; // D:/foo/bar
		std::string file;      // baz
		std::string extension; // py
	};

	// Numerical utility
	template <typename T>
	T clamp(const T& x, const T& min, const T& max) {
		if (x < min) return min;
		if (x > max) return max;
		return x;
	}

	template <typename T>
	inline int sign(const T& x) {
		assert(std::is_arithmetic<T>::value && "Type must be a numerical type");
		return x >= 0 ? 1 : -1;
	}

	template <typename T>
	void findMaxMin(const T* data, const int& sizeOfData, T* min, T* max) {
		assert((data != nullptr) && (sizeOfData > 0) && (min != nullptr) && (max != nullptr) && "Invalid parameters supplied");

		T x, _min, _max;

		_min = _max = *data;

		for (int i = 0; i < sizeOfData; i++) {
			x = *(data + i);

			if (x < _min) _min = x;
			if (x > _max) _max = x;
		}

		*min = _min;
		*max = _max;
	}

	// String manipulation utility
	std::vector<std::string> split(const std::string& s, char delimiter, const bool& removeEmpty = true);
	std::string getFileNameNoExtension(const std::string& target);
	std::string replace(const std::string& s, const char& a, const char& b);
	std::string trimWhitespace(const std::string& s);
	bool hasStart(const std::string& s, const std::string& start);
	bool hasEnding(const std::string& a, const std::string& b);
	int stringCount(char* s, const char& c);

	// File utility
	Utils::FileInfo getFileInfo(const std::string& target);
	long getFileSize(const std::string& target);
	long getLines(const std::string& taret);
	std::vector<std::string> getFilesInDirectory(const char* target);

	// Opengl Callbacks
	void glfwErrorCallbackFunction(int ecode, const char* info);

	// Utility class to time lengths of functions 
	// TODO: Maybe template this to for different levels of accuracy?
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

	struct CSVvector {
		std::string dataField; // Name of the data field that will drive the vector
		glm::vec3 origin; // Origin of the vector (relative to car local coordinates)
		glm::vec3 direction; // Direction of the vector (relative to the car local coordinates)
		glm::vec3 color; // Color of the vector
		bool doNormalize;
	};

	struct CSVgraph {
		std::string dataField; // Name of the data field that will drive the graph
		glm::vec3 color; // Color of the line
		int graph; // Which graph display this data field should be displayed on. Defaults to the first available

		// These get calculated after being loaded
		float min;
		float max;
	};

	struct CSVDataDisplaySettings {
		std::string path; // Path to the file where this originated

		// Special fields which will drive the positional data of the car
		std::string longitude;
		std::string latitude;
		std::string elevation;
		std::string heading;

		std::vector<CSVvector> vectors;
		std::vector<CSVgraph> graphs;
	};

	CSVDataDisplaySettings loadDisplaySettings(const std::string& target);
}