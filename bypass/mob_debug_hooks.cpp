#include "pch.h"

#include "mob_debug_hooks.h"

#include "hooker.h"
#include "logger.h"

// =====================================================================
// TEMP DEBUG (task-008): v79 mob re-entry crash.
//
// Repro: enter a map with mobs (fine) -> cash shop -> exit back to that map.
// The already-present mobs are re-created and the client AVs.
//
// Root cause (from the v79 IDB, GMS_v79_1_DEVM): the stock CMob virtual method
// sub_63A0D2 @0x63A0D2 reads a WZ-resource com_ptr at CMob+0x118 and, when it is
// NULL, computes base 0 and dereferences [0x250] with no null guard:
//     mov eax,[ecx+118h]; ... and edx,eax; cmp [edx+250h],edi   ; edx==0 -> AV
// On a fresh spawn CMob+0x118 is populated; on re-entry it is NULL.
//
// None of our loaded DLLs touch CMob, so this is stock behaviour exposed by the
// re-entry path, not a struct we write. These hooks (v79 only, hardcoded
// addresses) log which mob hits it and guard the null deref so we can see
// whether the client survives past the crash.
// =====================================================================

#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION == 79)

// void __thiscall CMob::Init(CMob* this, DWORD* oid, CInPacket* pkt)
typedef VOID(__thiscall* _CMob_Init_t)(void* pThis, void* oid, void* pkt);
static _CMob_Init_t _CMob_Init = nullptr;

VOID __fastcall CMob_Init_Hook(void* pThis, PVOID edx, void* oid, void* pkt) {
    DWORD* p = reinterpret_cast<DWORD*>(pThis);
    Log("[mobdbg] CMob::Init  IN this=%p oid=%p tmpl=%p res0x118=%p", pThis, oid, (void*)p[98], (void*)p[70]);
    _CMob_Init(pThis, oid, pkt);
    Log("[mobdbg] CMob::Init OUT this=%p res0x118=%p", pThis, (void*)p[70]);
}

// int __thiscall sub_63A0D2(CMob* this, int a1, int a2, int a3, int a4)  (vtable @0xA30C28)
// Reads CMob+0x118 (WZ resource); AVs when it is NULL.
typedef int(__thiscall* _MobAttr_t)(void* pThis, int a1, int a2, int a3, int a4);
static _MobAttr_t _MobAttr = nullptr;

int __fastcall MobAttr_Hook(void* pThis, PVOID edx, int a1, int a2, int a3, int a4) {
    DWORD* p = reinterpret_cast<DWORD*>(pThis);
    if (p[70] == 0) {
        // The dereference the stock code is about to do would AV at 0x250.
        // Treat a missing resource as the "no attr" case and skip.
        Log("[mobdbg] sub_63A0D2 res0x118=NULL this=%p tmpl=%p -- SKIPPING (would AV)", pThis, (void*)p[98]);
        return 6;
    }
    return _MobAttr(pThis, a1, a2, a3, a4);
}

#endif

BOOL InstallMobDebugHooks() {
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION == 79)
    INITMAPLEHOOK(_CMob_Init, _CMob_Init_t, CMob_Init_Hook, 0x006312A7);
    INITMAPLEHOOK(_MobAttr, _MobAttr_t, MobAttr_Hook, 0x0063A0D2);
#endif
    return TRUE;
}
