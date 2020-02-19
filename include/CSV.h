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
    static std::pair<std::string, std::string> splitNameAndType(const std::string& line);

    std::vector<std::string> getOrderedData() const;

    std::string getData(const std::string& type, const int& line) const;
    std::string getData(const int& index, const int& line) const;

    // TODO: These buffer functions could probably be faster if the data was packed in terms of type instead of by line

    // Fill buffer with data of <type> from times in the range [start, end]
    int getBatchData(const std::string& type, const int& start, const int& end, char** dataBuffer);
    // Same, except convert char representations into floats as we fill buffer
    int getBatchDataAsFloat(const std::string& type, const int& start, const int& end, float* dataBuffer);

    std::string getMetric(const std::string& type);

    int getNumberOfDataTypes() const;
    int getNumberOfTimePoints() const;

    bool hasData(const std::string& type);

    CSV(const std::string& targt);
    ~CSV();
};