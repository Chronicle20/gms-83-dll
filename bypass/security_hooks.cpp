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
// --- v84 movement anti-cheat call-NULL defang ----------------------------
// v84's CVecCtrl movement update validates ZtlSecureTear-encrypted coordinates
// through a Themida-protected routine that, when it detects the bypass, jumps
// to a deliberately-NULL pointer (EIP=0, C0000005 DEP-execute) on the first
// movement frame -> NMCO's exception handler then hangs the process. We register
// a vectored handler that runs FIRST, recognizes exactly that trap (and nothing
// else, so Themida's own exception-based control flow is untouched), logs the
// faulting context once per occurrence, and resumes past the trapped call.
static PVOID g_hAntiTamperVeh = nullptr;
static volatile LONG g_antiTamperHits = 0;
static volatile LONG g_avSeen = 0; // any C0000005 our VEH is reached for (ordering probe)

static LONG CALLBACK AntiTamperTrapVeh(EXCEPTION_POINTERS* ep) {
    const EXCEPTION_RECORD* er = ep->ExceptionRecord;

    if (er->ExceptionCode != EXCEPTION_ACCESS_VIOLATION)
        return EXCEPTION_CONTINUE_SEARCH;

    // Ordering probe: prove whether our handler is even reached for AVs, and
    // where they land. If we see other AVs but never the at-0 trap, something
    // (NMCO) is intercepting it before the VEH chain (dispatcher hook).
    const LONG seen = InterlockedIncrement(&g_avSeen);
    const DWORD info0 = er->NumberParameters >= 1 ? static_cast<DWORD>(er->ExceptionInformation[0]) : 0xFFFFFFFF;
    if (seen <= 40)
        Log("VEH> saw AV #%ld at=%p info0=%lu (0=read 1=write 8=exec)", seen, er->ExceptionAddress, info0);

    // The trap: executing at address 0.
    if (er->ExceptionAddress != nullptr)
        return EXCEPTION_CONTINUE_SEARCH;
    if (er->NumberParameters < 2 || er->ExceptionInformation[0] != 8 /* execute */)
        return EXCEPTION_CONTINUE_SEARCH;

    CONTEXT* ctx = ep->ContextRecord;
    const DWORD esp = ctx->Esp;

    // The faulting `call 0` already pushed the return address; [ESP] is it.
    DWORD retaddr = 0;
    if (!IsBadReadPtr(reinterpret_cast<void*>(esp), 4))
        retaddr = *reinterpret_cast<DWORD*>(esp);

    // ECX is `this` for the trapped thiscall (the moving object's CVecCtrl).
    DWORD vtbl = 0;
    if (ctx->Ecx && !IsBadReadPtr(reinterpret_cast<void*>(ctx->Ecx), 4))
        vtbl = *reinterpret_cast<DWORD*>(ctx->Ecx);

    const LONG n = InterlockedIncrement(&g_antiTamperHits);
    if (n <= 24) {
        Log("VEH> call-NULL trap #%ld ret=%08X EAX=%08X ECX(this)=%08X [ECX]vtbl=%08X "
            "EDX=%08X EBX=%08X ESI=%08X EDI=%08X EBP=%08X",
            n, retaddr, ctx->Eax, ctx->Ecx, vtbl, ctx->Edx, ctx->Ebx, ctx->Esi, ctx->Edi, ctx->Ebp);
    }

    // Skip the trapped call: pop the pushed return address, resume after it.
    if (retaddr) {
        ctx->Eip = retaddr;
        ctx->Esp = esp + 4;
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

void InstallAntiTamperVeh() {
    if (g_hAntiTamperVeh)
        return;
    g_hAntiTamperVeh = AddVectoredExceptionHandler(1 /* call first */, AntiTamperTrapVeh);
    Log("VEH> v84 anti-tamper call-NULL defang installed: %p", g_hAntiTamperVeh);
}

// Re-register at the FRONT of the VEH chain. NMCO installs its own handler
// after us at field entry, pushing itself ahead; re-arming each frame keeps us
// first. Add-then-remove so there is never a window with no handler installed.
void RearmAntiTamperVeh() {
    PVOID prev = g_hAntiTamperVeh;
    g_hAntiTamperVeh = AddVectoredExceptionHandler(1, AntiTamperTrapVeh);
    if (prev)
        RemoveVectoredExceptionHandler(prev);
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
    // v84's movement anti-cheat (CVecCtrl + ZtlSecureTear, Themida-protected)
    // retaliates against the bypass with a deliberate call-through-NULL on the
    // first movement update. Install a vectored handler that logs the trap
    // context and skips the call so the field survives. See InstallAntiTamperVeh.
    InstallAntiTamperVeh();
#endif

    return TRUE;
}
