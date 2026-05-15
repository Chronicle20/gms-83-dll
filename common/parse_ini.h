#pragma once

#include <functional>
#include <map>
#include <string>
#include <vector>

namespace ms::ini {

struct Parsed {
    // "section.key" -> ordered values (last-wins is the consumer's choice).
    std::map<std::string, std::vector<std::string>> entries;
};

using LogSink = std::function<void(const char*)>;

// Returns true on successful open + parse. Malformed lines and duplicate
// keys are reported via `sink` (if non-null) but do not fail the parse.
// Missing file is the only hard failure.
bool Parse(const std::string& path, Parsed& out, const LogSink& sink = nullptr);

} // namespace ms::ini
