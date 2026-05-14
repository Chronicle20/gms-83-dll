#pragma once
#include <cstddef>

namespace ms::byte_ops {

// Fills `n` bytes at `dst` with the x86 NOP opcode (0x90). No-op when n == 0.
// `dst` may be nullptr only when n == 0.
void fill_nop(unsigned char* dst, std::size_t n);

// Copies `n` bytes from `src` to `dst` (memcpy wrapper). No-op when n == 0.
void copy(unsigned char* dst, const unsigned char* src, std::size_t n);

} // namespace ms::byte_ops
