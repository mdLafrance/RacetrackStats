#include <Utils.h>

namespace Utils
{
	std::vector<std::string> split(const std::string& s, char delimiter) {
		std::vector<std::string> splitString;

		std::string word;

		for (int i = 0; i < s.length(); i++) {
			if (s[i] == delimiter) {
				if (!word.empty()) {
					splitString.push_back(word);
					word = "";
				}
				continue;
			}

			word += s[i];
		}

		if (!word.empty()) {
			splitString.push_back(word);
		}

		return splitString;
	}

	std::string getFileNameNoExtension(const std::string& target) {
		std::string s = target;
		std::replace(s.begin(), s.end(), '\\', '/');
		std::string noExtension = s.substr(0, s.find_last_of('.'));
		return Utils::split(noExtension, '/').back();
	}

	Utils::FileInfo getFileInfo(const std::string& target) {
		Utils::FileInfo fi;

		std::string s = target;
		std::replace(s.begin(), s.end(), '\\', '/');

		int lastSlash, lastDot, offset;

		lastSlash = s.find_last_of('/');
		lastDot = s.find_last_of('.');
		offset = lastDot - lastSlash - 1;

		fi.directory = lastSlash != -1 ? s.substr(0, lastSlash) : ".";
		fi.file = s.substr(lastSlash + 1, offset);
		fi.extension = lastDot != -1 ? s.substr(lastDot + 1) : "";

		return fi;
	}

	long getFileSize(const std::string& target) {
		std::ifstream f;
		f.open(target);

		if (!f.is_open()) {
			std::cerr << "Couldn't open file " << target << " to check file size." << std::endl;
		}

		f.seekg(0, std::ios_base::end);

		return f.tellg();
	}
	
	void StopWatch::start() {
		this->origin = std::chrono::steady_clock::now();
		this->latest = std::chrono::steady_clock::now();
	}

	std::chrono::milliseconds StopWatch::lap() {
		std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

		auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - latest);

		this->latest = now;

		return diff;
	}

	std::string StopWatch::lap_s() {
		long int count = this->lap().count();

		std::stringstream ss;

		std::string s = std::to_string(count / 1000);
		std::string ms = std::to_string(count % 1000);

		// Left justify with 0's so no ambiguity between 0.1s and 0.10s
		for (int i = 0; i < (ms.size() - 3); i++) {
			ms = '0' + ms;
		}

		ss << s << "." << ms << "s";

		return ss.str();
	}

	std::chrono::milliseconds StopWatch::total() {
		std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

		auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->origin);

		return diff;
	}

	std::string StopWatch::total_s() {
		long int count = this->total().count();

		std::stringstream ss;

		std::string s = std::to_string(count / 1000);
		std::string ms = std::to_string(count % 1000);

		// Left justify with 0's so no ambiguity between 0.1s and 0.10s
		for (int i = 0; i < (ms.size() - 3); i++) {
			ms = '0' + ms;
		}

		ss << s << "." << ms << "s";

		return ss.str();
	}

	StopWatch::StopWatch() {}
	StopWatch::~StopWatch() {}
}

void VariableFloatArray::add(const float& target) {
	*(this->data + this->size++) = target;
	
	if (this->size >= this->sizeOfData) {
		float* old = this->data;
		int oldSize = this->sizeOfData;

		this->sizeOfData = 2 * oldSize;
		this->data = new float[this->sizeOfData];

		memcpy(this->data, old, oldSize);

		delete[] old;

		//std::cout << "Total size expanded to " << this->sizeOfData << std::endl;
	}
}

void VariableFloatArray::push_back(const float& target) {
	this->add(target);
}

int VariableFloatArray::getSize()
{
	return this->size;
}

void VariableFloatArray::clear() {
	this->size = 0;
}

float VariableFloatArray::at(const int& target) {
	if (target <= 0 || target >= this->size) {
		throw std::out_of_range("Target value out of range.");
	}

	return *(this->data + target);
}

float& VariableFloatArray::operator[](const int& index) {
	if (index < 0 || index >= this->size) {
		throw std::out_of_range("Index out of range.");
	}

	return *(this->data + index);
}

VariableFloatArray::VariableFloatArray() {
	this->size = 0;
	this->sizeOfData = 64;
	this->data = new float[64];
}

VariableFloatArray::VariableFloatArray(const int& dataSize) {
	if (dataSize < 1) {
		throw std::out_of_range("Non Positive Values Not Accepted.");
	}

	this->size = 0;
	this->sizeOfData = dataSize;
	this->data = new float[dataSize];
}

VariableFloatArray::~VariableFloatArray() {
	delete[] this->data;
}
