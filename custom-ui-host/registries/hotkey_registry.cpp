#include "hotkey_registry.h"

namespace custom_ui_host {

bool HotkeyRegistry::IsDenylisted(unsigned int vk) {
    switch (vk) {
        case 0x01:  // VK_LBUTTON
        case 0x02:  // VK_RBUTTON
        case 0x04:  // VK_MBUTTON
        case 0x09:  // VK_TAB
        case 0x0D:  // VK_RETURN
        case 0x1B:  // VK_ESCAPE
        case 0x25:  // VK_LEFT
        case 0x26:  // VK_UP
        case 0x27:  // VK_RIGHT
        case 0x28:  // VK_DOWN
            return true;
        default:
            return false;
    }
}

HotkeyId HotkeyRegistry::Bind(unsigned int vk, unsigned int /*mods*/,
                              WindowHandle /*target*/) {
    if (IsDenylisted(vk)) return 0;
    return 0;  // not implemented yet — other failure paths in next task
}

bool HotkeyRegistry::Unbind(HotkeyId /*id*/) { return false; }
const HotkeyBinding *HotkeyRegistry::Lookup(unsigned int /*vk*/,
                                            unsigned int /*mods*/) const {
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
