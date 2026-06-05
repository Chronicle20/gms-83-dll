#pragma once
#include <atomic>

namespace custom_ui_host {
extern std::atomic<bool> g_pending_restore;
}
