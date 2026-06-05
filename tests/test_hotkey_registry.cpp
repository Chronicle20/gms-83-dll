#include "hotkey_registry.h"

#include <gtest/gtest.h>

using custom_ui_host::HotkeyRegistry;

TEST(HotkeyRegistry, DenylistedKeysReject) {
    HotkeyRegistry reg;
    // VK_ESCAPE=0x1B, VK_RETURN=0x0D, VK_TAB=0x09, arrows 0x25..0x28,
    // mouse 0x01..0x04.
    EXPECT_EQ(reg.Bind(0x1B, 0, /*target=*/42), 0u);  // VK_ESCAPE
    EXPECT_EQ(reg.Bind(0x0D, 0, 42), 0u);             // VK_RETURN
    EXPECT_EQ(reg.Bind(0x09, 0, 42), 0u);             // VK_TAB
    EXPECT_EQ(reg.Bind(0x25, 0, 42), 0u);             // VK_LEFT
    EXPECT_EQ(reg.Bind(0x26, 0, 42), 0u);             // VK_UP
    EXPECT_EQ(reg.Bind(0x27, 0, 42), 0u);             // VK_RIGHT
    EXPECT_EQ(reg.Bind(0x28, 0, 42), 0u);             // VK_DOWN
    EXPECT_EQ(reg.Bind(0x01, 0, 42), 0u);             // VK_LBUTTON
    EXPECT_EQ(reg.Bind(0x02, 0, 42), 0u);             // VK_RBUTTON
    EXPECT_EQ(reg.Bind(0x04, 0, 42), 0u);             // VK_MBUTTON
}
