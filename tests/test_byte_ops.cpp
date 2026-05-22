#include "byte_ops.h"

#include <array>
#include <cstring>

#include <gtest/gtest.h>

TEST(ByteOps, FillNopFillsAllBytesWith90) {
    std::array<unsigned char, 5> buf{};
    ms::byte_ops::fill_nop(buf.data(), buf.size());
    for (auto b : buf) {
        EXPECT_EQ(b, 0x90);
    }
}

TEST(ByteOps, FillNopZeroLengthIsNoOpAndAcceptsNullptr) {
    // The contract says nullptr is allowed when n == 0.
    ms::byte_ops::fill_nop(nullptr, 0);
    // Also verify it doesn't write past the end of a non-null buf.
    std::array<unsigned char, 4> guard{0xAA, 0xBB, 0xCC, 0xDD};
    ms::byte_ops::fill_nop(guard.data(), 0);
    EXPECT_EQ(guard[0], 0xAA);
    EXPECT_EQ(guard[1], 0xBB);
    EXPECT_EQ(guard[2], 0xCC);
    EXPECT_EQ(guard[3], 0xDD);
}

TEST(ByteOps, CopyRoundTrip) {
    std::array<unsigned char, 7> src{1, 2, 3, 4, 5, 6, 7};
    std::array<unsigned char, 7> dst{};
    ms::byte_ops::copy(dst.data(), src.data(), src.size());
    EXPECT_EQ(std::memcmp(src.data(), dst.data(), src.size()), 0);
}

TEST(ByteOps, CopyZeroLengthIsNoOp) {
    std::array<unsigned char, 3> dst{0xAA, 0xBB, 0xCC};
    std::array<unsigned char, 3> src{0x11, 0x22, 0x33};
    ms::byte_ops::copy(dst.data(), src.data(), 0);
    EXPECT_EQ(dst[0], 0xAA);
    EXPECT_EQ(dst[1], 0xBB);
    EXPECT_EQ(dst[2], 0xCC);
}
