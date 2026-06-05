#pragma once
#include <cstdint>
#include <mutex>
#include <vector>

namespace custom_ui_host {

using HotkeyId = std::uint32_t;
using WindowHandle = std::uint32_t;

struct HotkeyBinding {
    HotkeyId id;
    unsigned int vk;
    unsigned int mods; // CUSTOM_UI_MOD_* bitmask
    WindowHandle target;
};

class HotkeyRegistry {
  public:
    // Returns 0 on rejection (denylist hit, vk already mapped on game,
    // already-bound conflict). Returns non-zero opaque id on success.
    HotkeyId Bind(unsigned int vk, unsigned int mods, WindowHandle target);
    bool Unbind(HotkeyId id);
    const HotkeyBinding* Lookup(unsigned int vk, unsigned int mods) const;
    void Clear();
    std::size_t Size() const;

  private:
    static bool IsDenylisted(unsigned int vk);

    mutable std::mutex mu_;
    std::vector<HotkeyBinding> bindings_;
    HotkeyId next_id_ = 1;
};

} // namespace custom_ui_host
