#pragma once
#include <Windows.h>

namespace custom_ui_host {
// Temporary diagnostic hooks: log the game's real font-factory + button
// CreateCtrl arguments so we can replicate them. Remove once font/button work.
BOOL InstallDebugHooks();
} // namespace custom_ui_host
