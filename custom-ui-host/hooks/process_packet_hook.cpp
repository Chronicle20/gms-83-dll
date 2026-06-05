#include "pch.h"

#include "hooks/process_packet_hook.h"

#include "abi/abi_globals.h"
#include "runtime/host_config.h"

#include "hooker.h"
#include "logger.h"
#include "memory_map.h"

namespace custom_ui_host {

namespace {

typedef void(__thiscall *ProcessPacket_t)(CClientSocket *, CInPacket *);
ProcessPacket_t _ProcessPacket = nullptr;

void __fastcall ProcessPacket_Hook(CClientSocket *self, void * /*edx*/,
                                   CInPacket *iPacket) {
    if (!iPacket) {
        _ProcessPacket(self, iPacket);
        return;
    }
    const unsigned int saved_offset = iPacket->m_uOffset;
    const unsigned short opcode = iPacket->Decode2();

    if (opcode >= g_config.inbound_op_min && opcode <= g_config.inbound_op_max) {
        // Custom range: dispatch to registered handler with the payload
        // bytes that follow the opcode. Vanilla path NOT invoked.
        const unsigned char *payload =
            reinterpret_cast<const unsigned char *>(
                iPacket->m_aRecvBuff.GetTailPosition()) +
            iPacket->m_uOffset;
        const unsigned int payloadLen =
            iPacket->m_uDataLen > iPacket->m_uOffset
                ? iPacket->m_uDataLen - iPacket->m_uOffset
                : 0u;
        if (g_packets) g_packets->Dispatch(opcode, payload, payloadLen);
        return;
    }

    // Outside custom range: restore cursor and run vanilla.
    iPacket->m_uOffset = saved_offset;
    _ProcessPacket(self, iPacket);
}

} // namespace

BOOL InstallProcessPacketHook() {
    INITMAPLEHOOK_OR_RETURN(_ProcessPacket, ProcessPacket_t,
                            &ProcessPacket_Hook,
                            C_CLIENT_SOCKET_PROCESS_PACKET);
    return TRUE;
}

} // namespace custom_ui_host
