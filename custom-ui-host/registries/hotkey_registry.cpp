#include "hotkey_registry.h"

namespace custom_ui_host {

bool HotkeyRegistry::IsDenylisted(unsigned int vk) {
    switch (vk) {
    case 0x01: // VK_LBUTTON
    case 0x02: // VK_RBUTTON
    case 0x04: // VK_MBUTTON
    case 0x09: // VK_TAB
    case 0x0D: // VK_RETURN
    case 0x1B: // VK_ESCAPE
    case 0x25: // VK_LEFT
    case 0x26: // VK_UP
    case 0x27: // VK_RIGHT
    case 0x28: // VK_DOWN
        return true;
    default:
        return false;
    }
}

HotkeyId HotkeyRegistry::Bind(unsigned int vk, unsigned int mods, WindowHandle target) {
    if (IsDenylisted(vk))
        return 0;
    std::lock_guard<std::mutex> g(mu_);
    for (const auto& b : bindings_) {
        if (b.vk == vk && b.mods == mods)
            return 0;
    }
    HotkeyBinding nb{next_id_++, vk, mods, target};
    bindings_.push_back(nb);
    return nb.id;
}

bool HotkeyRegistry::Unbind(HotkeyId id) {
    std::lock_guard<std::mutex> g(mu_);
    for (auto it = bindings_.begin(); it != bindings_.end(); ++it) {
        if (it->id == id) {
            bindings_.erase(it);
            return true;
        }
    }
    return false;
}

const HotkeyBinding* HotkeyRegistry::Lookup(unsigned int vk, unsigned int mods) const {
    std::lock_guard<std::mutex> g(mu_);
    for (const auto& b : bindings_) {
        if (b.vk == vk && b.mods == mods)
            return &b;
    }
    return nullptr;
}
void HotkeyRegistry::Clear() {
    std::lock_guard<std::mutex> g(mu_);
    bindings_.clear();
    next_id_ = 1;
}
std::size_t HotkeyRegistry::Size() const {
    std::lock_guard<std::mutex> g(mu_);
    return bindings_.size();
}

} // namespace custom_ui_host
