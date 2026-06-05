#pragma once
#include <cstdint>
#include <mutex>
#include <unordered_map>

// __cdecl is an MSVC/x86 calling-convention annotation. On GCC/Clang
// (Linux host unit-test builds) it does not exist; define it away so the
// header compiles on both targets without forking the type.
#if !defined(_MSC_VER) && !defined(__cdecl)
#define __cdecl
#endif

namespace custom_ui_host {

using HandlerId = std::uint32_t;

using PacketHandlerFn = void(__cdecl*)(unsigned short opcode, const unsigned char* payload, unsigned int payloadLen,
                                       void* user);

class PacketRegistry {
  public:
    PacketRegistry(unsigned short range_min, unsigned short range_max);

    // Returns 0 if opcode is out of range or already registered.
    HandlerId Register(unsigned short opcode, PacketHandlerFn fn, void* user);
    bool Unregister(HandlerId id);

    // Out-of-range opcodes and absent handlers are silent no-ops.
    void Dispatch(unsigned short opcode, const unsigned char* payload, unsigned int payloadLen);

    std::size_t Size() const;

  private:
    struct Entry {
        HandlerId id;
        unsigned short opcode;
        PacketHandlerFn fn;
        void* user;
    };

    mutable std::mutex mu_;
    std::unordered_map<unsigned short, Entry> by_opcode_;
    HandlerId next_id_ = 1;
    unsigned short range_min_;
    unsigned short range_max_;
};

} // namespace custom_ui_host
