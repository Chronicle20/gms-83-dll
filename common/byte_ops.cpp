#include "byte_ops.h"

#include <cstring>

namespace ms::byte_ops {

namespace {
constexpr unsigned char kX86Nop = 0x90;
}

void fill_nop(unsigned char* dst, std::size_t n) {
    if (n == 0)
        return;
    for (std::size_t i = 0; i < n; ++i) {
        dst[i] = kX86Nop;
    }
}

void copy(unsigned char* dst, const unsigned char* src, std::size_t n) {
    if (n == 0)
        return;
    std::memcpy(dst, src, n);
}

} // namespace ms::byte_ops
