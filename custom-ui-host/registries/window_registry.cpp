#include "window_registry.h"

namespace custom_ui_host {

WindowHandle WindowRegistry::Register(CustomUIWnd* w) {
    std::lock_guard<std::mutex> g(mu_);
    WindowHandle h = next_handle_++;
    by_handle_.emplace(h, w);
    return h;
}

bool WindowRegistry::Unregister(WindowHandle h) {
    std::lock_guard<std::mutex> g(mu_);
    return by_handle_.erase(h) != 0;
}

CustomUIWnd* WindowRegistry::Lookup(WindowHandle h) const {
    std::lock_guard<std::mutex> g(mu_);
    auto it = by_handle_.find(h);
    return it == by_handle_.end() ? nullptr : it->second;
}

void WindowRegistry::ForEach(const std::function<void(CustomUIWnd*)>& fn) const {
    std::lock_guard<std::mutex> g(mu_);
    for (const auto& kv : by_handle_)
        fn(kv.second);
}

std::size_t WindowRegistry::Size() const {
    std::lock_guard<std::mutex> g(mu_);
    return by_handle_.size();
}

} // namespace custom_ui_host
