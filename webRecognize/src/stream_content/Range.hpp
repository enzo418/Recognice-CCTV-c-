#pragma once
#include <iostream>
#include <limits>
#include <list>
#include <vector>
#include <sstream>

struct Range {
    long start;
    long end;
    size_t length() const {
        return end - start + 1;
    }
};

std::vector<std::string> splitString(const std::string& str, const std::string& delim);

Range getRange(const std::string& range_s, const long& filesize);

inline void getRanges(const std::string& rangesHeader, std::vector<Range>& outranges, const long& filesize);

bool parseRangeHeader(const std::string& rangesHeader, const long& filesize, std::vector<Range>& ranges);

std::string getRangeStringForResponse(Range r, const long& filesize);