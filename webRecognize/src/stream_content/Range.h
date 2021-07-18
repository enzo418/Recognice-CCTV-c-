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

std::vector<std::string> splitString(const std::string& str, const std::string& delim) {
	std::vector<std::string> tokens;
	size_t prev = 0, pos = 0;
	do {
		pos = str.find(delim, prev);
		if (pos == std::string::npos) pos = str.length();
		std::string token = str.substr(prev, pos-prev);
        if (!token.empty()) {
            tokens.push_back(token);
        }
		prev = pos + delim.length();
	} while (pos < str.length() && prev < str.length());

	return tokens;
}

Range getRange(const std::string& range_s, const long& filesize) {
    // read: https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Range
    Range range;
    size_t mpos = range_s.find('-');
    auto parts = splitString(range_s, "-");
    if (mpos == 0) {
        // <unit>=-<suffix-length>
        // <suffix-length> is 
        // an integer indicating the number of units at the end of the file to return.
        range.start = filesize - std::stol(parts[0]);
        range.end = filesize - 1;
    } else if (mpos == range_s.length() - 1) {
        // <unit>=<range-start>-
        range.start = std::stol(parts[0]);
        range.end = filesize - 1;
    } else {
        // <unit>=<range-start>-<range-end>
        range.start = std::stol(parts[0]);
        range.end = std::stol(parts[1]);
    }
    return range;
}

inline void getRanges(const std::string& rangesHeader, std::vector<Range>& outranges, const long& filesize) {
    auto ranges = splitString(rangesHeader, ", ");
    for(auto& range : ranges) {
        outranges.push_back(getRange(range, filesize));
    }
}

bool parseRangeHeader(const std::string& rangesHeader, const long& filesize, std::vector<Range>& ranges) {    
    if (rangesHeader.substr(0, 6) == "bytes=") {
        getRanges(rangesHeader.substr(6, rangesHeader.length() - 1), ranges, filesize);
        return true;
    } else {        
        return false;
    }
}

std::string getRangeStringForResponse(Range r, const long& filesize) {
    // bytes start-end/total
    return "bytes " + std::to_string(r.start) + "-" + std::to_string(r.end) + "/" + std::to_string(filesize);
}
