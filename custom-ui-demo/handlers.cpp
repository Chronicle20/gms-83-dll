#include "pch.h"

#include "demo_main.h"

#include "logger.h"

#include <cstdio>

namespace custom_ui_demo {

void OnPing(CustomUI_WindowHandle, CustomUI_CtrlId, void*) {
    Log("custom-ui-demo: OnPing -> SendPacket(0x0F00)");
    g_abi.SendPacket(0x0F00, nullptr, 0);
}

void OnPong(unsigned short opcode, const unsigned char* payload, unsigned int len, void*) {
    int seq;
    if (len >= 4) {
        seq = *reinterpret_cast<const int*>(payload);
    } else {
        seq = ++g_ping_count;
    }
    char buf[64];
    std::snprintf(buf, sizeof(buf), "Server says: pong %d", seq);
    Log("custom-ui-demo: OnPong opcode=0x%04X seq=%d", opcode, seq);
    g_abi.SetLabelText(g_window, g_label, buf);
}

} // namespace custom_ui_demo
