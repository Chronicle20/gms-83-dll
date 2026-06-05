#pragma once

#include "registries/hotkey_registry.h"
#include "registries/packet_registry.h"
#include "registries/window_registry.h"

#include "runtime/custom_ui_wnd.h"

#include <memory>

namespace custom_ui_host {

// Host-wide registry singletons. Constructed by InitAbiGlobals() from
// MainProc (after LoadHostConfig() so packet range comes from INI).
extern std::unique_ptr<WindowRegistry> g_windows;
extern std::unique_ptr<HotkeyRegistry> g_hotkeys;
extern std::unique_ptr<PacketRegistry> g_packets;

void InitAbiGlobals();

} // namespace custom_ui_host
