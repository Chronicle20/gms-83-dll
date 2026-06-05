#include "hotkey_registry.h"

#include <gtest/gtest.h>

using custom_ui_host::HotkeyRegistry;

TEST(HotkeyRegistry, DenylistedKeysReject) {
    HotkeyRegistry reg;
    // VK_ESCAPE=0x1B, VK_RETURN=0x0D, VK_TAB=0x09, arrows 0x25..0x28,
    // mouse 0x01..0x04.
    EXPECT_EQ(reg.Bind(0x1B, 0, /*target=*/42), 0u); // VK_ESCAPE
    EXPECT_EQ(reg.Bind(0x0D, 0, 42), 0u);            // VK_RETURN
    EXPECT_EQ(reg.Bind(0x09, 0, 42), 0u);            // VK_TAB
    EXPECT_EQ(reg.Bind(0x25, 0, 42), 0u);            // VK_LEFT
    EXPECT_EQ(reg.Bind(0x26, 0, 42), 0u);            // VK_UP
    EXPECT_EQ(reg.Bind(0x27, 0, 42), 0u);            // VK_RIGHT
    EXPECT_EQ(reg.Bind(0x28, 0, 42), 0u);            // VK_DOWN
    EXPECT_EQ(reg.Bind(0x01, 0, 42), 0u);            // VK_LBUTTON
    EXPECT_EQ(reg.Bind(0x02, 0, 42), 0u);            // VK_RBUTTON
    EXPECT_EQ(reg.Bind(0x04, 0, 42), 0u);            // VK_MBUTTON
}

TEST(HotkeyRegistry, BindAndLookup) {
    HotkeyRegistry reg;
    auto id = reg.Bind(/*vk=*/0x77, /*mods=*/0, /*target=*/123); // VK_F8
    ASSERT_NE(id, 0u);
    auto* b = reg.Lookup(0x77, 0);
    ASSERT_NE(b, nullptr);
    EXPECT_EQ(b->target, 123u);
    EXPECT_EQ(b->vk, 0x77u);
}

TEST(HotkeyRegistry, BindRejectsConflict) {
    HotkeyRegistry reg;
    auto id1 = reg.Bind(0x77, 0, 1);
    ASSERT_NE(id1, 0u);
    auto id2 = reg.Bind(0x77, 0, 2); // same vk+mods, different target
    EXPECT_EQ(id2, 0u);              // second is rejected
    auto* b = reg.Lookup(0x77, 0);
    ASSERT_NE(b, nullptr);
    EXPECT_EQ(b->target, 1u); // first still wins
}

TEST(HotkeyRegistry, SameVkDifferentModsCoexist) {
    HotkeyRegistry reg;
    auto id1 = reg.Bind(0x77, 0, 1);
    auto id2 = reg.Bind(0x77, /*Ctrl=*/0x02, 2);
    EXPECT_NE(id1, 0u);
    EXPECT_NE(id2, 0u);
    EXPECT_EQ(reg.Lookup(0x77, 0)->target, 1u);
    EXPECT_EQ(reg.Lookup(0x77, 0x02)->target, 2u);
}

TEST(HotkeyRegistry, UnbindRemoves) {
    HotkeyRegistry reg;
    auto id = reg.Bind(0x77, 0, 1);
    ASSERT_NE(id, 0u);
    EXPECT_TRUE(reg.Unbind(id));
    EXPECT_EQ(reg.Lookup(0x77, 0), nullptr);
    EXPECT_FALSE(reg.Unbind(id)); // double unbind is idempotent failure
}

TEST(HotkeyRegistry, ClearWipesAll) {
    HotkeyRegistry reg;
    reg.Bind(0x77, 0, 1);
    reg.Bind(0x78, 0, 2);
    EXPECT_EQ(reg.Size(), 2u);
    reg.Clear();
    EXPECT_EQ(reg.Size(), 0u);
}
