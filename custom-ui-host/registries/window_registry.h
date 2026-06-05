#pragma once
#include <cstdint>
#include <functional>
#include <mutex>
#include <unordered_map>

namespace custom_ui_host {

// Forward-declared; defined in runtime/custom_ui_wnd.h (Phase 5). Tests
// supply their own stub definition via the same forward-declared type.
struct CustomUIWnd;

using WindowHandle = std::uint32_t;

class WindowRegistry {
public:
    WindowHandle Register(CustomUIWnd *w);
    bool Unregister(WindowHandle h);
    CustomUIWnd *Lookup(WindowHandle h) const;
    void ForEach(const std::function<void(CustomUIWnd *)> &fn) const;
    std::size_t Size() const;

private:
    mutable std::mutex mu_;
    std::unordered_map<WindowHandle, CustomUIWnd *> by_handle_;
    WindowHandle next_handle_ = 1;
};

} // namespace custom_ui_host
