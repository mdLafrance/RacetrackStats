#include <Utils.h>

// TODO: some of these could stand to get cleaned up a bit

namespace Utils
{
	std::vector<std::string> split(const std::string& s, char delimiter) {
		std::vector<std::string> splitString;

		std::string word = "";

		for (int i = 0; i < s.length(); i++) {
			if (s[i] == delimiter) {
				splitString.push_back(word);
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
	
	bool hasEnding (std::string const &fullString, std::string const &ending) {
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
			std::cerr << "Couldn't open file " << target << " to check file size." << std::endl;
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

	void log(const std::string& message, const unsigned char& logLevel){
		if (logLevel & __log_flags__) std::cout << message << std::endl;
	}

	void setLogFlags(const unsigned char& flags){
		::__log_flags__ = flags;
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