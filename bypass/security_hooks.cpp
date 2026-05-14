#include "pch.h"

#include "security_hooks.h"

#include "hooker.h"
#include "logger.h"
#include <memedit.h>

typedef VOID(__thiscall* _CSecurityClient__OnPacket_t)(CSecurityClient* pThis, CInPacket* iPacket);
typedef INT(__cdecl* _DR__check_t)();
typedef VOID(__thiscall* _CeTracer__Run_t)(int* pThis);
typedef VOID(__cdecl* _SendHSLog_t)(char a1);

VOID __fastcall CSecurityClient__OnPacket_Hook(CSecurityClient* pThis, PVOID edx, CInPacket* iPacket) {
    Log("CSecurityClient::OnPacket.");
}

INT __cdecl DR__check_Hook() {
    return 0;
}

VOID __fastcall CeTracer__Run_Hook(int* pThis, PVOID edx) {
}

VOID __fastcall SendHSLog_Hook(void* ecx, void* edx, char a1) {
}

BOOL InstallSecurityHooks() {
    // The pre-refactor MainProc interleaved CSecurityClient byte patches
    // BEFORE CWvsApp::CallUpdate / ConnectLogin. The TU split groups them
    // under security_hooks. INITMAPLEHOOK_OR_RETURN inside each
    // installer short-circuits THIS installer on failure; the caller
    // (bypass_main.cpp::MainProc) then short-circuits MainProc itself.

#if defined(REGION_JMS)
    // TODO: lift this and the OnPacket+0x19 NOP below into a proper
    // INITMAPLEHOOK_OR_RETURN-installed CSecurityClient::OnPacket hook.
    if (C_SECURITY_CLIENT_ON_PACKET_RET_STUB != 0) {
        constexpr BYTE retStub[] = {0xC3};
        MemEdit::WriteBytes(C_SECURITY_CLIENT_ON_PACKET_RET_STUB, retStub);
    }
#endif

#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 87) || defined(REGION_JMS)
    HOOKTYPEDEF_C(DR__check);
    INITMAPLEHOOK_OR_RETURN(_DR__check, _DR__check_t, DR__check_Hook, DR_CHECK);
#endif

#if defined(REGION_JMS)
    if (C_SECURITY_CLIENT_ON_PACKET_CHECK != 0) {
        constexpr BYTE nopPair[] = {0x90, 0x90};
        MemEdit::WriteBytes(C_SECURITY_CLIENT_ON_PACKET_CHECK + C_SECURITY_CLIENT_ON_PACKET_CHECK_OFFSET,
                            nopPair);
    }
#endif

    HOOKTYPEDEF_C(CSecurityClient__OnPacket);
    INITMAPLEHOOK_OR_RETURN(_CSecurityClient__OnPacket, _CSecurityClient__OnPacket_t,
                            CSecurityClient__OnPacket_Hook, C_SECURITY_CLIENT_ON_PACKET);

#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95)
    HOOKTYPEDEF_C(CeTracer__Run);
    INITMAPLEHOOK_OR_RETURN(_CeTracer__Run, _CeTracer__Run_t, CeTracer__Run_Hook, CE_TRACER_RUN);
#endif

#if defined(REGION_GMS)
    HOOKTYPEDEF_C(SendHSLog);
    INITMAPLEHOOK_OR_RETURN(_SendHSLog, _SendHSLog_t, SendHSLog_Hook, SEND_HS_LOG);
#endif

    return TRUE;
}
