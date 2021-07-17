#pragma once
#include <iostream>
#include <limits>
#include <list>
#include <vector>
#include <sstream>

// a quick test. This code comes from godbolt
// https://github.com/mattgodbolt/seasocks/blob/b1b641527d7db967cd583a3f73157a1d991e26d1/src/main/c/Connection.cpp

std::vector<std::string> split(const std::string& input, char splitChar) {
    if (input.empty()) {
        return {};
    }

    std::vector<std::string> result;
    size_t pos = 0;
    size_t newPos;
    while ((newPos = input.find(splitChar, pos)) != std::string::npos) {
        result.push_back(input.substr(pos, newPos - pos));
        pos = newPos + 1;
    }
    result.push_back(input.substr(pos));
    return result;
}

struct Range {
    long start;
    long end;
    size_t length() const {
        return end - start + 1;
    }
};

bool parseRange(const std::string& rangeStr, Range& range) {
    size_t minusPos = rangeStr.find('-');
    if (minusPos == std::string::npos) {
        std::cout << "Bad range: '" << rangeStr << "'";
        return false;
    }
    if (minusPos == 0) {
        // A range like "-500" means 500 bytes from end of file to end.
        range.start = std::stoi(rangeStr);
        range.end = std::numeric_limits<long>::max();
        return true;
    } else {
        range.start = std::stoi(rangeStr.substr(0, minusPos));
        if (minusPos == rangeStr.size() - 1) {
            range.end = std::numeric_limits<long>::max();
        } else {
            range.end = std::stoi(rangeStr.substr(minusPos + 1));
        }
        return true;
    }
    return false;
}

bool parseRanges(const std::string& range, std::list<Range>& ranges) {
    static const std::string expectedPrefix = "bytes=";
    if (range.length() < expectedPrefix.length() || range.substr(0, expectedPrefix.length()) != expectedPrefix) {
        std::cout << "Bad range request prefix: '" << range << "'";
        return false;
    }
    auto rangesText = split(range.substr(expectedPrefix.length()), ',');
    for (auto& it : rangesText) {
        Range r;
        if (!parseRange(it, r)) {
            return false;
        }
        ranges.push_back(r);
    }
    return !ranges.empty();
}


// Sends HTTP 200 or 206, content-length, and range info as needed. Returns the actual file ranges
// needing sending.
std::list<Range> processRangesForStaticData(const std::list<Range>& origRanges, long fileSize, std::string& rangeHeaderOut) {
    if (origRanges.empty()) {
        // Easy case: a non-range request.
        // res->writeStatus("200 OK")->writeHeader("Content-Length", fileSize);
        return {Range{0, fileSize - 1}};
    }

    // Partial content request.
    // bufferResponseAndCommonHeaders(ResponseCode::PartialContent);
    int contentLength = 0;
    std::ostringstream rangeLine;
    std::list<Range> sendRanges;
    for (auto actualRange : origRanges) {
        if (actualRange.start < 0) {
            actualRange.start += fileSize;
        }
        if (actualRange.start >= fileSize) {
            actualRange.start = fileSize - 1;
        }
        if (actualRange.end >= fileSize) {
            actualRange.end = fileSize - 1;
        }
        contentLength += actualRange.length();
        sendRanges.push_back(actualRange);
        rangeLine << actualRange.start << "-" << actualRange.end;
    }
    rangeLine << "/" << fileSize;

    std::cout << "Range writeline parsed: " << rangeLine.str() << std::endl;

    rangeHeaderOut = rangeLine.str();
    // bufferLine(rangeLine.str());
    // bufferLine("Content-Length: " + toString(contentLength));
    return sendRanges;
}