#include "pch.h"

#include "runtime/ui_dispatch.h"
#include "runtime/seh_dispatch.h"

#include "logger.h"

#include <mutex>
#include <utility>
#include <vector>

namespace custom_ui_host {

namespace {
std::mutex g_ui_mu;
std::vector<std::pair<UIThreadFn, void *>> g_ui_tasks;
} // namespace

void EnqueueUIThreadTask(UIThreadFn fn, void *user) {
    if (!fn) return;
    std::lock_guard<std::mutex> g(g_ui_mu);
    g_ui_tasks.emplace_back(fn, user);
}

void DrainUIThreadTasks() {
    // Swap the queue out under the lock, then run outside it so a task that
    // enqueues more work (or calls back into the framework) can't deadlock.
    std::vector<std::pair<UIThreadFn, void *>> tasks;
    {
        std::lock_guard<std::mutex> g(g_ui_mu);
        if (g_ui_tasks.empty()) return;
        tasks.swap(g_ui_tasks);
    }
    for (auto &t : tasks) {
        UIThreadFn fn = t.first;
        void *user = t.second;
        SafeDispatch("UI-thread task", [fn, user]() { fn(user); });
    }
}

} // namespace custom_ui_host
