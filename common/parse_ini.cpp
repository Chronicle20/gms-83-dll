#include "parse_ini.h"

#include <algorithm>
#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <fstream>

namespace ms::ini {

namespace {

std::string TrimLeft(std::string s) {
    auto it = std::find_if(s.begin(), s.end(), [](unsigned char c) { return !std::isspace(c); });
    s.erase(s.begin(), it);
    return s;
}

std::string TrimRight(std::string s) {
    auto it = std::find_if(s.rbegin(), s.rend(), [](unsigned char c) { return !std::isspace(c); });
    s.erase(it.base(), s.end());
    return s;
}

std::string Trim(std::string s) {
    return TrimRight(TrimLeft(std::move(s)));
}

// Strip everything from the first ; or # to end of line. [Section] is not a comment.
std::string StripComment(const std::string& line) {
    auto pos = line.find_first_of(";#");
    return (pos == std::string::npos) ? line : line.substr(0, pos);
}

void Tell(const LogSink& sink, const char* fmt, ...) {
    if (!sink)
        return;
    char buf[256];
    va_list args;
    va_start(args, fmt);
    std::vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    sink(buf);
}

} // namespace

bool Parse(const std::string& path, Parsed& out, const LogSink& sink) {
    std::ifstream inputFile(path);
    if (!inputFile.is_open()) {
        Tell(sink, "Failed to open INI file: %s", path.c_str());
        return false;
    }

    std::string line;
    std::string currentSection;
    int lineNo = 0;
    while (std::getline(inputFile, line)) {
        ++lineNo;
        std::string trimmed = Trim(StripComment(line));
        if (trimmed.empty())
            continue;

        if (trimmed.front() == '[' && trimmed.back() == ']') {
            currentSection = trimmed.substr(1, trimmed.size() - 2);
            continue;
        }

        auto pos = trimmed.find('=');
        if (pos == std::string::npos) {
            Tell(sink, "INI: malformed line %d (no '='): %s", lineNo, trimmed.c_str());
            continue;
        }
        std::string key = Trim(trimmed.substr(0, pos));
        std::string value = Trim(trimmed.substr(pos + 1));
        if (key.empty()) {
            Tell(sink, "INI: malformed line %d (empty key): %s", lineNo, trimmed.c_str());
            continue;
        }
        out.entries[currentSection + "." + key].push_back(std::move(value));
    }
    return true;
}

} // namespace ms::ini
