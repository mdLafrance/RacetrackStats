#include <CSV.h>

CSV::CSV(const std::string& target) {
    Utils::StopWatch stopwatch;

    FILE* f = fopen(target.c_str(), "r");

    this->numberOfFields = 0;
    this->numberOfLines = 0;

    if (!f) {
        std::cerr << "ERROR: Couldn't open cvs file " << target << " for reading." << std::endl;
        this->data = nullptr;
        return;
    }

    std::cout << "Loading CSV file: " << target << std::endl;

    stopwatch.start();

    char line[2048];

    int i = 0; // Position in line
    int lineNumber = 0;
    int lineOffset = 0;
    
    char c; // Current character

    int wordLength = 0;
    char word[256];

    // Parse first line for named data fields
    fgets(line, sizeof(line), f);

    std::string line_s = std::string(line);

    line_s = line_s.substr(0, line_s.size() - 1); // Chop \n

    std::string dataName;
    std::string dataType;

    int nullCounter = 0; // Create unique NULL<number> name for null named fields. (is there ever going to be more than 1?)

    // For each comma separated name of data field
    for (std::string t : Utils::split(line_s, ',')) {
        std::pair<std::string, std::string> dataAndType;

        // If data type is not given, identify it with unique null name, just in case
        if (t.size() == 0){
            dataAndType.first = "NULL" + std::to_string(nullCounter++);
            dataAndType.second = "";
        } else {
            dataAndType = CSV::splitNameAndType(t);        
        }

        this->dataOffsets[dataAndType.first] = i++;
        this->dataTypes[dataAndType.first] = dataAndType.second;

        ++this->numberOfFields;
    }

    // Count number of lines, not including first line defining the types
    while (fgets(line, sizeof(line), f)) {
        ++this->numberOfLines;
    }

    rewind(f);
    fgets(line, sizeof(line), f); // eat first line we've already parsed

    this->data = new char* [this->numberOfFields * this->numberOfLines];

    // Copy the word that was being recorded into the data buffer
    auto writeWord = [&](){
        word[wordLength] = '\0';
        *(this->data + lineOffset) = new char[wordLength+1];
        strcpy((*(this->data + lineOffset)), &word[0]);
        wordLength = 0;
        ++lineOffset;
    };

    i = 0;

    // Parse rest of data by line
    while (fgets(line, sizeof(line), f)) {
        i = 0;

        wordLength = 0;

        lineOffset = lineNumber * this->numberOfFields;

        c = line[i];

        int wordCount = 0;

        // The next loop cycles over the line recording characters
        // If a ',' is hit, this indicates the end of the last word, so save that one to the data buffer if it's not empty

        while (c != '\0' && c != '\n') {
            if (c == ',') {
                if (wordLength == 0) {
                    *(this->data + lineOffset++) = nullptr;
					goto nextword;
                }
                writeWord();
                goto nextword;
            }
            word[wordLength++] = c;
nextword:
            c = line[++i];
        }

        // Last word in line
        if (wordLength == 0) {
            *(this->data + lineOffset++) = nullptr;
        }
        else {
            writeWord();
        }

        ++lineNumber;
    }

    fclose(f);
    
    std::cout << "Loaded " << this->numberOfFields << " data fields over " << this->numberOfLines-1 << " time points. " << " (" << stopwatch.lap_s() << ")" << std::endl;
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

        char* ptr = *(this->data + (line * this->numberOfFields) + offset);

        if (ptr == nullptr){
            return "";
        } else {
            return ptr;
        }
    }
    else {
        std::cerr << "ERROR: Couldn't access data field <" << type << "> because it is not present in the csv." << std::endl;
		return "";
    }
}

float CSV::getDataAsFloat(const std::string& type, const int& line) const {
	return std::atof(this->getData(type, line).c_str());
}

std::string CSV::getData(const int& index, const int& line) const {
    if (!(0 <= index <= this->numberOfFields)) {
        throw std::out_of_range("Index is out of range.");
    }

	char* ptr = *(this->data + (line * this->numberOfFields) + index);

	if (ptr == nullptr){
		return "";
	} else {
		return ptr;
	}
}

int CSV::getBatchData(const std::string& type, const int& start, const int& end, char** dataBuffer) {
    if (!this->hasData(type)) {
        std::cerr << "ERROR: Requesting batch fetch for non-existent data type: " << type << std::endl;
        return -1;
    }

    if (end < start || start < 0 || end > this->numberOfLines) {
        throw std::range_error("Illegal bounds.");
    }

    int index = this->dataOffsets.at(type);
    int j = 0;

    for (int i = start; i < end; i++) {
        *(dataBuffer + j++) = *(this->data + (i * this->numberOfFields) + index);
    }

    return 0;
}

int CSV::getBatchDataAsFloat(const std::string& type, const int& start, const int& end, float* dataBuffer) {
    if (!this->hasData(type)) {
        std::cerr << "ERROR: Requesting batch fetch for non-existent data type: " << type << std::endl;
        return -1;
    }

    if (end < start || start < 0 || end > this->numberOfLines) {
        throw std::range_error("Illegal bounds.");
    }

    int index = this->dataOffsets.at(type);
    int j = 0;

    for (int i = start; i < end; i++) {
        *(dataBuffer + j++) = std::atof(*(this->data + (i * this->numberOfFields) + index));
    }

    return 0;
}

std::vector<std::string> CSV::getOrderedData() const {
    std::vector<std::string> data(this->numberOfFields, "NULL");

    for (std::pair<std::string, int> p : this->dataOffsets) {
        data[p.second] = p.first;
    }

    return data;
}

std::string CSV::getMetric(const std::string& type) {
    if (!(this->dataTypes.count(type) == 1)) {
        std::cerr << "ERROR: Type: " << type << " is not present in the CSV." << std::endl;
        return "";
    }

    return this->dataTypes.at(type);
}

int CSV::getOffset(const std::string& type) {
	return this->dataOffsets[type];
}

bool CSV::hasData(const std::string& type) {
	return this->dataOffsets.find(type) != this->dataOffsets.end();
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
