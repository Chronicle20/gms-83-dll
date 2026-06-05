#include "packet_registry.h"

#include <cstring>
#include <vector>
#include <gtest/gtest.h>

using custom_ui_host::PacketRegistry;

namespace {
struct CallSpy {
    int count = 0;
    unsigned short last_op = 0;
    unsigned int last_len = 0;
    std::vector<unsigned char> last_payload;
};

void __cdecl Handler(unsigned short op, const unsigned char *p,
                     unsigned int n, void *user) {
    auto *s = static_cast<CallSpy *>(user);
    s->count++;
    s->last_op = op;
    s->last_len = n;
    s->last_payload.assign(p, p + n);
}
} // namespace

TEST(PacketRegistry, RegisterAndDispatch) {
    PacketRegistry reg(/*min=*/0x2000, /*max=*/0x20FF);
    CallSpy spy;
    auto id = reg.Register(0x2000, &Handler, &spy);
    ASSERT_NE(id, 0u);
    unsigned char payload[] = {0xAA, 0xBB};
    reg.Dispatch(0x2000, payload, sizeof(payload));
    EXPECT_EQ(spy.count, 1);
    EXPECT_EQ(spy.last_op, 0x2000);
    EXPECT_EQ(spy.last_len, 2u);
    EXPECT_EQ(spy.last_payload[0], 0xAA);
}

TEST(PacketRegistry, OutOfRangeRegisterRejected) {
    PacketRegistry reg(0x2000, 0x20FF);
    CallSpy spy;
    EXPECT_EQ(reg.Register(0x1FFF, &Handler, &spy), 0u);
    EXPECT_EQ(reg.Register(0x2100, &Handler, &spy), 0u);
}

TEST(PacketRegistry, DoubleRegisterRejected) {
    PacketRegistry reg(0x2000, 0x20FF);
    CallSpy spy;
    auto id1 = reg.Register(0x2010, &Handler, &spy);
    auto id2 = reg.Register(0x2010, &Handler, &spy);
    EXPECT_NE(id1, 0u);
    EXPECT_EQ(id2, 0u);
}

TEST(PacketRegistry, DispatchOutsideRangeNoop) {
    PacketRegistry reg(0x2000, 0x20FF);
    CallSpy spy;
    reg.Register(0x2000, &Handler, &spy);
    reg.Dispatch(0x1FFF, nullptr, 0);  // outside range
    reg.Dispatch(0x2001, nullptr, 0);  // in range, no handler
    EXPECT_EQ(spy.count, 0);
}

TEST(PacketRegistry, UnregisterStopsDispatch) {
    PacketRegistry reg(0x2000, 0x20FF);
    CallSpy spy;
    auto id = reg.Register(0x2000, &Handler, &spy);
    EXPECT_TRUE(reg.Unregister(id));
    reg.Dispatch(0x2000, nullptr, 0);
    EXPECT_EQ(spy.count, 0);
    EXPECT_FALSE(reg.Unregister(id));  // idempotent failure
}
