#include <CVS.h>

CVS::CVS(const std::string& target) {
    Utils::StopWatch sw;

    FILE* f = fopen(target.c_str(), "r");

    this->numberOfFields = 0;
    this->numberOfLines = 0;

    if (!f) {
        std::cerr << "ERROR: Couldn't open cvs file " << target << " for reading." << std::endl;
        this->data = nullptr;
        return;
    }

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

    std::cout << "File has: " << this->numberOfFields << " types." << std::endl;
    std::cout << "Counted " << this->numberOfLines << " lines" << std::endl;

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

    std::vector<std::string> types = Utils::split(line_s, ',');

    for (std::string t : types) {
        std::cout << "Registered <" << t << "> at " << i << std::endl;
        this->dataOffsets[t] = i++;
    }

    i = 0;

    // Parse rest of data by line
    while (fgets(line, sizeof(line), f)) {
        i = 0;

        wordLength = 0;

        lineOffset = lineNumber * this->numberOfFields;

        c = line[i];

        while (c != '\0' && c != '\n') {
            if (c == ',') {
                if (wordLength == 0) {
                    *(this->data + lineOffset++) = nullptr;
					goto nextword;
                }
                word[wordLength] = '\0';
                char* destination = *(this->data + lineOffset++);
                destination = new char[wordLength+1];
                strcpy(destination, &word[0]);
				wordLength = 0;
                goto nextword;
            }
            word[wordLength++] = c;
nextword:
            c = line[++i];
        }
    }

    fclose(f);

    std::cout << "TIME: " << sw.lap_s() << std::endl;
}

CVS::~CVS(){
    for (int i = 0; i < (this->numberOfFields * this->numberOfLines); i++) {
        std::cout << '\r' << i;
        delete[] *(this->data + i);
    }

    delete[] this->data;
}

char* CVS::getData(char* type, const int& line) {
    int offset = this->dataOffsets[type];

    return *(this->data + (line * this->numberOfFields) + offset);
}
