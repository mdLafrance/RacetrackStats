#pragma once

#include <stdio.h>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iostream>

namespace Utils 
{
	typedef struct { // D:/foo/bar/baz.py
		std::string directory; // D:/foo/bar
		std::string file;      // baz
		std::string extension; // py
	} FileInfo;

	std::vector<std::string> split(const std::string& s, char delimiter);
	std::string getFileNameNoExtension(const std::string& target);
	Utils::FileInfo getFileInfo(const std::string& target);
	long getFileSize(const std::string& target);

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

class VariableFloatArray {
	int size;
	int sizeOfData;
	float* data;

public:
	void add(const float& target);
	void push_back(const float& target); // For compatability with vector for internal testing

	int getSize();

	float at(const int& target);
	void clear();

	float& operator[](const int& index);

	VariableFloatArray();
	VariableFloatArray(const int& size);
	~VariableFloatArray();
};
