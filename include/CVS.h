#pragma once

#include <stdio.h>
#include <string>
#include <map>

#include <Utils.h>

class CVS {
    int numberOfLines;
    int numberOfFields;

    char** data;

    std::map<std::string, int> dataOffsets;

public:
    char* getData(char* type, const int& line);

    CVS(const std::string& targt);
    ~CVS();
};