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

    std::string getData(const std::string& type, const int& line) const;
    std::string getData(const int& index, const int& line) const;
    std::string getMetric(const std::string& type);

    int numberOfTimePoints() const;

    bool hasData(const std::string& type);

    CSV(const std::string& targt);
    ~CSV();
};