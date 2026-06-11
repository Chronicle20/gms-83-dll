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

VOID __fastcall CeTracer__Run_Hook(int* pThis, PVOID edx) {}

VOID __fastcall SendHSLog_Hook(void* ecx, void* edx, char a1) {}

#if defined(REGION_GMS) && BUILD_MAJOR_VERSION == 84
// --- TEMPORARY DIAGNOSTIC (v84): log-only call-NULL probe -----------------
// The v84 client hangs ~1s into the field, right after it takes monster control,
// in the movement/ZtlSecureTear decrypt path (a call-through-NULL that NMCO's
// handler catches and then hangs on). We only want to know WHERE it hangs on the
// fixed-server build. A plain VEH won't fire (NMCO routes the fault around the
// VEH chain via a dispatcher hook), so we hook the registered SetUnhandled-
// ExceptionFilter target (0x7BB9AA) -- the point NMCO DOES route through -- log
// the faulting context + .text return chain, then call the original so it hangs
// exactly as it would. No resume/defang. Remove once localized.
typedef LONG(__stdcall* _TLEFProbe_t)(EXCEPTION_POINTERS*);
static _TLEFProbe_t _TLEFProbe = nullptr;
static volatile LONG g_probeHits = 0;

static LONG __stdcall TLEFProbe_Hook(EXCEPTION_POINTERS* ep) {
    const EXCEPTION_RECORD* er = ep->ExceptionRecord;
    if (er->ExceptionCode == EXCEPTION_ACCESS_VIOLATION && er->ExceptionAddress == nullptr) {
        const LONG n = InterlockedIncrement(&g_probeHits);
        if (n <= 3) {
            const CONTEXT* ctx = ep->ContextRecord;
            const DWORD esp = ctx->Esp;
            const DWORD ret = !IsBadReadPtr(reinterpret_cast<void*>(esp), 4) ? *reinterpret_cast<DWORD*>(esp) : 0;
            const DWORD vtbl = (ctx->Ecx && !IsBadReadPtr(reinterpret_cast<void*>(ctx->Ecx), 4))
                                   ? *reinterpret_cast<DWORD*>(ctx->Ecx)
                                   : 0;
            Log("PROBE> call-NULL #%ld ret=%08X ECX=%08X [ECX]=%08X EAX=%08X EDX=%08X EBX=%08X ESI=%08X EDI=%08X "
                "EBP=%08X",
                n, ret, ctx->Ecx, vtbl, ctx->Eax, ctx->Edx, ctx->Ebx, ctx->Esi, ctx->Edi, ctx->Ebp);
            if (!IsBadReadPtr(reinterpret_cast<void*>(esp), 0x400)) {
                const DWORD* sp = reinterpret_cast<const DWORD*>(esp);
                int found = 0;
                for (unsigned i = 0; i < 0x100 && found < 16; ++i) {
                    const DWORD v = sp[i];
                    if (v < 0x00401000 || v >= 0x00AC0000) // maplestory code range only
                        continue;
                    const BYTE* p = reinterpret_cast<const BYTE*>(v);
                    char k = '?'; // classify the call that would have returned here
                    if (!IsBadReadPtr(reinterpret_cast<void*>(v - 6), 6)) {
                        if (p[-5] == 0xE8) k = 'C';      // call rel32
                        else if (p[-2] == 0xFF) k = 'v'; // call [reg]
                        else if (p[-3] == 0xFF) k = 'v'; // call [reg+disp8]
                        else if (p[-6] == 0xFF) k = 'm'; // call [mem]
                        else k = '.';
                    }
                    if (k == '?' || k == '.')
                        continue;
                    Log("PROBE>   stk[+%03X]=%08X (%c)", i * 4, v, k);
                    ++found;
                }
            }
        }
    }
    return _TLEFProbe ? _TLEFProbe(ep) : EXCEPTION_CONTINUE_SEARCH;
}
#endif

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
        MemEdit::WriteBytes(C_SECURITY_CLIENT_ON_PACKET_CHECK + C_SECURITY_CLIENT_ON_PACKET_CHECK_OFFSET, nopPair);
    }
#endif

    HOOKTYPEDEF_C(CSecurityClient__OnPacket);
    INITMAPLEHOOK_OR_RETURN(_CSecurityClient__OnPacket, _CSecurityClient__OnPacket_t, CSecurityClient__OnPacket_Hook,
                            C_SECURITY_CLIENT_ON_PACKET);

#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95)
    HOOKTYPEDEF_C(CeTracer__Run);
    INITMAPLEHOOK_OR_RETURN(_CeTracer__Run, _CeTracer__Run_t, CeTracer__Run_Hook, CE_TRACER_RUN);
#endif

#if defined(REGION_GMS)
    HOOKTYPEDEF_C(SendHSLog);
    INITMAPLEHOOK_OR_RETURN(_SendHSLog, _SendHSLog_t, SendHSLog_Hook, SEND_HS_LOG);
#endif

#if defined(REGION_GMS) && BUILD_MAJOR_VERSION == 84
    // TEMPORARY diagnostic probe (log-only; see TLEFProbe_Hook above). Hardcoded
    // 0x007BB9AA = TopLevelExceptionFilter entry. Best-effort so a failure can't
    // abort MainProc. Remove once the v84 in-field hang is localized.
    INITMAPLEHOOK(_TLEFProbe, _TLEFProbe_t, TLEFProbe_Hook, 0x007BB9AA);
#endif

    return TRUE;
}
