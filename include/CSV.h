#pragma once

#include <stdio.h>
#include <string>
#include <map>

#include <Utils.h>

class CSV {
    int numberOfLines;
    int numberOfFields;

    char** data;

    std::map<std::string, int> dataOffsets;
    std::map<std::string, std::string> dataTypes;

public:
    static std::pair<std::string, std::string> splitNameAndType(const std::string& line); // Utility function

    std::vector<std::string> getOrderedData() const; // Get vector of types in order of appearance in file

    // Get a single value of a type at a time point
    std::string getData(const std::string& type, const int& line) const; // ex. getData("Speed", 2); gets speed value at time point 2
    std::string getData(const int& index, const int& line) const;
	float getDataAsFloat(const std::string& type, const int& line) const; // Same behavior, but cast to float

	// Interpret data for a type as float, and get the max and min over the range
	void getDataMinMax(const std::string& type, float* min, float* max);

    // TODO: These buffer functions could probably be faster if the data was packed in terms of type instead of by line

    // Fill buffer with data of <type> from times in the range [start, end]
    int getBatchData(const std::string& type, const int& start, const int& end, char** dataBuffer);
    // Same, except convert char representations into floats as we fill buffer
    int getBatchDataAsFloat(const std::string& type, const int& start, const int& end, float* dataBuffer);

    std::string getMetric(const std::string& type); // ex. Speed in the file could be defined as <Speed [m/s]>. getMetric("Speed") returns "[m/s]"

	int getNumberOfDataTypes() const { return this->numberOfFields; };
	int getNumberOfTimePoints() const { return this->numberOfLines; };

	int getOffset(const std::string& type);

    bool hasData(const std::string& type);

    CSV(const std::string& targt); // Load a given .csv file 
    ~CSV();
};