#include "window_registry.h"

#include <gtest/gtest.h>

namespace custom_ui_host {
struct CustomUIWnd {  // test stub; the real one lands in Phase 5
    int marker;
};
} // namespace custom_ui_host

using custom_ui_host::CustomUIWnd;
using custom_ui_host::WindowRegistry;

TEST(WindowRegistry, RegisterReturnsNonZero) {
    WindowRegistry reg;
    CustomUIWnd w{42};
    auto h = reg.Register(&w);
    EXPECT_NE(h, 0u);
}

TEST(WindowRegistry, LookupReturnsTheWindow) {
    WindowRegistry reg;
    CustomUIWnd w{42};
    auto h = reg.Register(&w);
    EXPECT_EQ(reg.Lookup(h), &w);
    EXPECT_EQ(reg.Lookup(0), nullptr);
    EXPECT_EQ(reg.Lookup(h + 9999), nullptr);
}

TEST(WindowRegistry, UnregisterClearsLookup) {
    WindowRegistry reg;
    CustomUIWnd w{42};
    auto h = reg.Register(&w);
    EXPECT_TRUE(reg.Unregister(h));
    EXPECT_EQ(reg.Lookup(h), nullptr);
    EXPECT_FALSE(reg.Unregister(h));  // double-unreg is idempotent failure
}

TEST(WindowRegistry, HandlesAreUnique) {
    WindowRegistry reg;
    CustomUIWnd a{1}, b{2}, c{3};
    auto ha = reg.Register(&a);
    auto hb = reg.Register(&b);
    auto hc = reg.Register(&c);
    EXPECT_NE(ha, hb);
    EXPECT_NE(hb, hc);
    EXPECT_NE(ha, hc);
}

TEST(WindowRegistry, ForEachIteratesAll) {
    WindowRegistry reg;
    CustomUIWnd a{1}, b{2};
    reg.Register(&a);
    reg.Register(&b);
    int sum = 0;
    reg.ForEach([&](CustomUIWnd *w) { sum += w->marker; });
    EXPECT_EQ(sum, 3);
}
