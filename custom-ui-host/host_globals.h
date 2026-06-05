#pragma once
#include <atomic>

namespace custom_ui_host {

// Set true once MainProc has finished hook installation. Consumer DLLs
// poll CustomUI_IsReady() (which reads this) before issuing other calls.
extern std::atomic<bool> g_ready;

// Set true when the host detects it was loaded twice (mutex
// ERROR_ALREADY_EXISTS). The second instance does not install hooks
// and the ABI shims return error sentinels.
extern std::atomic<bool> g_double_load;

} // namespace custom_ui_host
