#include <Utils.h>

// TODO: some of these could stand to get cleaned up a bit

glm::vec3 operator *(float f, glm::vec3 v) {
	return glm::vec3(f * v[0], f * v[1], f * v[2]);
}

std::ostream& operator <<(std::ostream& os, const glm::vec3& v) {
	os << '[' << v[0] << ", " << v[1] << ", " << v[2] << ']';
	return os;
}

ImVec2 operator +(const ImVec2& a, const ImVec2& b) {
	return ImVec2(a[0] + b[0], a[1] + b[1]);
}

std::ostream& operator <<(std::ostream& os, const ImVec2& vec) {
	os << '[' << vec[0] << ", " << vec[1] << ']';
	return os;
}

namespace Utils
{
	std::vector<std::string> split(const std::string& s, char delimiter, const bool& removeEmpty) {
		std::vector<std::string> splitString;

		std::string word = "";

		for (int i = 0; i < s.length(); i++) {
			if (s[i] == delimiter) {
				if (word.size() > 0 || removeEmpty == false) splitString.push_back(word);
				word = "";
				continue;
			}

			word += s[i];
		}

		if (!word.empty()) {
			splitString.push_back(word);
		}

		return splitString;
	}
	bool hasStart(const std::string& s, const std::string& start) { 
		return s.rfind(start, 0) == 0; 
	};
	
	bool hasEnding (const std::string &fullString, const std::string &ending) {
		if (fullString.length() >= ending.length()) {
			return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
		} else {
			return false;
		}
	}

	std::string replace(const std::string& s, const char& a, const char& b){
		std::string newString(s.size(), ' ');

		char c;
		for (int i = 0; i < s.size(); i++){
			c = s[i];
			newString[i] = c == a ? b : c;
		}

		return newString;
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
			std::cerr << "ERROR: Couldn't open file " << target << " to check file size." << std::endl;
		}

		f.seekg(0, std::ios_base::end);

		return f.tellg();
	}

	long getLines(const std::string& target){
		FILE* f = fopen(target.c_str(), "r");

		if (!f){
			return 0;
		}

		char line[1024];

		long count = 0;

		while (fgets(line, sizeof(line), f)) ++count;

		fclose(f);

		return count;
	}

	std::vector<std::string> getFilesInDirectory(const char* target) {
		std::vector<std::string> files;

		struct dirent* ent;
		DIR* dir = opendir(target);

		if (dir != NULL) {
			ent = readdir(dir);

			while (ent != NULL) {
				if (ent->d_type == DT_REG) {
					files.push_back(ent->d_name);
				}

				ent = readdir(dir);
			}

			closedir(dir);
		}
		else {
			std::cerr << "ERROR: Couldn't open directory " << target << std::endl;
		}

		return files;
	}

	int stringCount(char* s, const char& c){
		int count = 0;

		for (int i = 0;; i++){
			if (*(s+i) == '\0') break;
			if (*(s+i) == c) ++count;
		}

		return count;
	}

	std::string trimWhitespace(const std::string& s){
		int firstCharacter = 0;
		int lastCharacter = s.size();

		for (int i = 0; i < s.size(); i++){
			if (s[i] != ' ') break;

			++firstCharacter;
		}

		for (int i = s.size(); i >= 0; i--){
			if (s[i] != ' ') break;

			--lastCharacter;
		}

		return s.substr(firstCharacter, lastCharacter);
	}

	void Utils::glfwErrorCallbackFunction(int ecode, const char* info) {
		std::cerr << "--------------------------------------------------------" << std::endl;
		std::cerr << "ERROR CODE " << ecode << std::endl;
		std::cerr << info << std::endl;
		std::cerr << "--------------------------------------------------------" << std::endl;
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

	CSVDataDisplaySettings loadDisplaySettings(const std::string& target) {
		CSVDataDisplaySettings data;

		std::ifstream f(target);

		if (!f.is_open()) {
			std::cerr << "ERROR: Couldn't load display settings file: " << target << std::endl;
			return data;
		}

		data.path = target;

		std::cout << "Loading CSV data display config file " << target << std::endl;

		std::string line;
		std::string lineType;
		std::vector<std::string> tokens;

		while (getline(f, line)) {
			if (line.size() == 0 || line[0] == '#') continue; // comment line

			tokens = Utils::split(line, ' ');

			if (tokens.size() == 0) continue; // empty line

			lineType = tokens[0];

			if (lineType == "longitude") data.longitude = tokens[1];
			else if (lineType == "latitude") data.latitude = tokens[1];
			else if (lineType == "elevation") data.elevation= tokens[1];
			else if (lineType == "heading") data.heading= tokens[1];

			else if (lineType == "vector") {
				// vector  x y z  x y z  r g b  <data field>

				data.vectors.push_back({ // New CSVvector struct
					tokens[10], // data field
					{std::atof(tokens[1].c_str()), std::atof(tokens[2].c_str()), std::atof(tokens[3].c_str())}, // origin
					glm::normalize(glm::vec3(std::atof(tokens[4].c_str()), std::atof(tokens[5].c_str()), std::atof(tokens[6].c_str()))), // direction
					{std::atof(tokens[7].c_str()), std::atof(tokens[8].c_str()), std::atof(tokens[9].c_str())} // color
				});
			}

			else if (lineType == "graph") {
				// graph <data field> r g b (optional int number of target graph)

				data.graphs.push_back({ // New CSVgraph struct
					tokens[1],
					{std::atof(tokens[2].c_str()), std::atof(tokens[3].c_str()), std::atof(tokens[4].c_str())}, // origin
					tokens.size() == 6 ? std::atoi(tokens[5].c_str()) : -1 // optional int graph target arg
				});
			}

			else {
				std::cerr << "ERROR while reading " << target << " : unkown line \"" << line << "\"" << std::endl;
			}
		}

		std::cout << "Finished loading config file." << std::endl;

		return data;
	}
}