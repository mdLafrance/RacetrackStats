#include <CSV.h>

CSV::CSV(const std::string& target) {
    Utils::StopWatch sw;

    FILE* f = fopen(target.c_str(), "r");

    this->numberOfFields = 0;
    this->numberOfLines = 0;

    if (!f) {
        std::cerr << "ERROR: Couldn't open cvs file " << target << " for reading." << std::endl;
        this->data = nullptr;
        return;
    }

    std::cout << "Loading CSV file: " << target << ' ';

    sw.start();

    char line[2048];

    fgets(line, sizeof(line), f);

    rewind(f);

    // Count number of named fields for file
    this->numberOfFields = Utils::stringCount(&line[0], ',') + 1;

    // Count number of lines
    while (fgets(line, sizeof(line), f)) {
        ++this->numberOfLines;
    }

    --this->numberOfLines; // Dont want to include line defining types

    this->data = new char* [this->numberOfFields * this->numberOfLines];

    rewind(f);

    int i = 0; // Position in line
    int lineNumber = 0;
    int lineOffset = 0;
    
    char c; // Current character

    int wordLength = 0;
    char word[256];

    // Parse first line into data fields
    fgets(line, sizeof(line), f);

    std::string line_s = std::string(line);

    line_s = line_s.substr(0, line_s.size() - 1); // Chop \n

    std::string dataName;
    std::string dataType;

    for (std::string t : Utils::split(line_s, ',')) {
        auto dataAndType = CSV::splitNameAndType(t);

        this->dataOffsets[dataAndType.first] = i++;
        this->dataTypes[dataAndType.first] = dataAndType.second;
    }

    i = 0;

    // Parse rest of data by line
    while (fgets(line, sizeof(line), f)) {
        i = 0;

        wordLength = 0;

        lineOffset = lineNumber * this->numberOfFields;

        c = line[i];

        int wordCount = 0;

        while (c != '\0' && c != '\n') {
            if (c == ',') {
                if (wordLength == 0) {
                    *(this->data + lineOffset++) = nullptr;
					goto nextword;
                }
                word[wordLength] = '\0';
                *(this->data + lineOffset) = new char[wordLength+1];
                strcpy((*(this->data + lineOffset)), &word[0]);
				wordLength = 0;
                ++lineOffset;
                goto nextword;
            }
            word[wordLength++] = c;
nextword:
            c = line[++i];
        }

        // Last word
        if (wordLength == 0) {
            *(this->data + lineOffset++) = nullptr;
        }
        else {
			word[wordLength] = '\0';
			*(this->data + lineOffset) = new char[wordLength+1];
			strcpy((*(this->data + lineOffset)), &word[0]);
			wordLength = 0;
			++lineOffset;
        }

        ++lineNumber;
    }

    fclose(f);

    std::cout << " (" << sw.lap_s() << ")" << std::endl;
}

CSV::~CSV(){
    for (int i = 0; i < (this->numberOfFields * this->numberOfLines); i++) {
		delete *(this->data + i);
    }

    delete[] this->data;
}

std::string CSV::getData(const std::string& type, const int& line) const {
    if (this->dataOffsets.count(type) == 1) {
		int offset = this->dataOffsets.at(type);
		return *(this->data + (line * this->numberOfFields) + offset);
    }
    else {
        std::cerr << "ERROR: Couldn't access data field <" << type << "> because it is not present in the csv." << std::endl;
		return "";
    }
}

std::string CSV::getData(const int& index, const int& line) const {
    if (!(0 <= index <= this->numberOfFields)) {
        throw std::out_of_range("Index is out of range.");
    }

    return *(this->data + (line * this->numberOfFields) + index);
}

bool CSV::hasData(const std::string& type) {
    return this->dataTypes.count(type) == 1;
}
    
std::pair<std::string, std::string> CSV::splitNameAndType(const std::string& line) {
    if (line.find_first_of('[') == std::string::npos) {
        return std::pair<std::string, std::string>(line, "");
    }

    const char* c = line.c_str();

    int length = line.size();

    int braceLocation;

    for (int i = length-1;i >=0 ; i--){
        if (c[i] == '['){
            braceLocation = i;
            break;
        }
    }

    return std::pair<std::string, std::string>(
        Utils::trimWhitespace(line.substr(0, braceLocation - 1)), 
        Utils::trimWhitespace(line.substr(braceLocation, length))
    );
}
