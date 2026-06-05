#include "packet_registry.h"

namespace custom_ui_host {

PacketRegistry::PacketRegistry(unsigned short range_min,
                               unsigned short range_max)
    : range_min_(range_min), range_max_(range_max) {}

HandlerId PacketRegistry::Register(unsigned short opcode, PacketHandlerFn fn,
                                   void *user) {
    if (opcode < range_min_ || opcode > range_max_) return 0;
    std::lock_guard<std::mutex> g(mu_);
    if (by_opcode_.count(opcode) != 0) return 0;
    Entry e{next_id_++, opcode, fn, user};
    by_opcode_.emplace(opcode, e);
    return e.id;
}

bool PacketRegistry::Unregister(HandlerId id) {
    std::lock_guard<std::mutex> g(mu_);
    for (auto it = by_opcode_.begin(); it != by_opcode_.end(); ++it) {
        if (it->second.id == id) {
            by_opcode_.erase(it);
            return true;
        }
    }
    return false;
}

void PacketRegistry::Dispatch(unsigned short opcode,
                              const unsigned char *payload,
                              unsigned int payloadLen) {
    if (opcode < range_min_ || opcode > range_max_) return;
    PacketHandlerFn fn = nullptr;
    void *user = nullptr;
    {
        std::lock_guard<std::mutex> g(mu_);
        auto it = by_opcode_.find(opcode);
        if (it == by_opcode_.end()) return;
        fn = it->second.fn;
        user = it->second.user;
    }
    // Dispatch outside the lock so a misbehaving handler can't deadlock
    // re-entering Register/Unregister.
    fn(opcode, payload, payloadLen, user);
}

std::size_t PacketRegistry::Size() const {
    std::lock_guard<std::mutex> g(mu_);
    return by_opcode_.size();
}

} // namespace custom_ui_host
