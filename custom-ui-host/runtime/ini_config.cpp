#include "pch.h"

#include "host_config.h"

#include "logger.h"
#include "parse_ini.h"

#include <cstdlib>
#include <string>

namespace custom_ui_host {

HostConfig g_config;

namespace {

unsigned short ParseHex16(const std::string &s, unsigned short fallback) {
    if (s.empty()) return fallback;
    try {
        auto v = std::stoul(s, nullptr, 0);
        if (v > 0xFFFF) return fallback;
        return static_cast<unsigned short>(v);
    } catch (...) {
        return fallback;
    }
}

bool ParseBool(const std::string &s, bool fallback) {
    if (s == "true" || s == "True" || s == "1") return true;
    if (s == "false" || s == "False" || s == "0") return false;
    return fallback;
}

const std::string &First(const ms::ini::Parsed &p, const char *key,
                         const std::string &fallback) {
    auto it = p.entries.find(key);
    if (it == p.entries.end() || it->second.empty()) return fallback;
    return it->second.front();
}

} // namespace

void LoadHostConfig() {
    // The host DLL is loaded from the edits/ directory; the INI sits
    // next to it. GetModuleHandle(NULL) is the host process, not the
    // host DLL — we need GetModuleFileNameW on the host's own HMODULE,
    // recoverable via a sentinel symbol address.
    HMODULE self = nullptr;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                       | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       reinterpret_cast<LPCWSTR>(&LoadHostConfig), &self);

    wchar_t path[MAX_PATH] = {};
    if (!self || !GetModuleFileNameW(self, path, MAX_PATH)) {
        Log("custom-ui-host: cannot resolve own module path -- using defaults");
        return;
    }
    // Replace filename with custom-ui-host.ini.
    std::wstring wpath = path;
    auto slash = wpath.find_last_of(L"\\/");
    if (slash == std::wstring::npos) {
        Log("custom-ui-host: module path has no directory separator");
        return;
    }
    wpath.erase(slash + 1);
    wpath += L"custom-ui-host.ini";

    // Convert wpath -> std::string (ASCII path expected on local builds).
    std::string apath(wpath.begin(), wpath.end());

    ms::ini::Parsed p;
    if (!ms::ini::Parse(apath, p, [](const char *msg) {
            Log("custom-ui-host: ini warning: %s", msg);
        })) {
        Log("custom-ui-host: ini missing at %s -- using defaults", apath.c_str());
        return;
    }

    const std::string empty;
    g_config.verbose = ParseBool(First(p, "host.verbose", empty), false);
    g_config.inbound_op_min =
        ParseHex16(First(p, "host.inbound_opcode_min", empty), 0x2000);
    g_config.inbound_op_max =
        ParseHex16(First(p, "host.inbound_opcode_max", empty), 0x20FF);
    g_config.outbound_op_min =
        ParseHex16(First(p, "host.outbound_opcode_min", empty), 0x0F00);
    g_config.outbound_op_max =
        ParseHex16(First(p, "host.outbound_opcode_max", empty), 0x0FFF);

    Log("custom-ui-host: config inbound=[0x%04X..0x%04X] outbound=[0x%04X..0x%04X] verbose=%d",
        g_config.inbound_op_min, g_config.inbound_op_max,
        g_config.outbound_op_min, g_config.outbound_op_max,
        (int)g_config.verbose);
}

} // namespace custom_ui_host
