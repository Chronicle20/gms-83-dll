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
// Root cause (from the v79 IDB, GMS_v79_1_DEVM): the stock virtual method
// CMob::OnResolveMoveAction @0x63A0D2 (IVecCtrlOwner vtable slot 1; named from
// the v95 PDB) reads the mob's vector-controller com_ptr CMob::m_pvc (CMob+0x11C,
// = [ecx+0x118] since this==CMob+4) and, when it is NULL, computes base 0 and
// dereferences it with no null guard:
//     mov eax,[ecx+118h]; ... and edx,eax; cmp [edx+250h],edi   ; edx==0 -> AV
// v95 has the SAME unguarded read (Nexon never fixed it); both rely on m_pvc
// being assigned before the resolver runs. Re-entering a populated map re-spawns
// every mob in a burst, so m_pvc is still NULL when the resolver is called.
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

// long __thiscall CMob::OnResolveMoveAction(this, long, long, long, CVecCtrl*)  @0x63A0D2
// this == CMob+4 (the IVecCtrlOwner sub-object), so [ecx+0x118] is CMob+0x11C.
// Reads that WZ resource; AVs when it is NULL (missing the guard SetShoeAttr has).
typedef int(__thiscall* _CMob_OnResolveMoveAction_t)(void* pThis, int a1, int a2, int a3, int a4);
static _CMob_OnResolveMoveAction_t _CMob_OnResolveMoveAction = nullptr;

// _ZtlSecureFuse<long> @0x004146AA: long __cdecl(long const* const, unsigned)
typedef long(__cdecl* _ZtlSecureFuse_long_t)(const void*, unsigned);
static const _ZtlSecureFuse_long_t _ZtlSecureFuse_long = reinterpret_cast<_ZtlSecureFuse_long_t>(0x004146AA);

int __fastcall CMob_OnResolveMoveAction_Hook(void* pThis, PVOID edx, int a2, int a3, int a4, int a5) {
    DWORD* p = reinterpret_cast<DWORD*>(pThis);
    if (p[70] != 0) {
        // Resource present -> stock path is correct.
        return _CMob_OnResolveMoveAction(pThis, a2, a3, a4, a5);
    }
    // Resource (CMob+0x11C) NULL: the stock CMob::OnResolveMoveAction @0x63A0D2 would compute base 0 and
    // deref [0x250] -> AV (it lacks the null-guard its sibling CMob::SetShoeAttr
    // has). Reimplement it faithfully with the guard: a null resource means the
    // "resource+0x250" test is 0, i.e. v6=0, and the foothold value is derived
    // from the mob template. This matches the stock-intended behaviour for an
    // absent resource instead of returning a blind constant. task-008.
    const int v6 = 0; // null resource -> no attr flag
    int v7 = (p[168] && p[98]) ? static_cast<int>(p[98]) : static_cast<int>(p[97]);
    int v8 = _ZtlSecureFuse_long(reinterpret_cast<const void*>(v7 + 64), *reinterpret_cast<unsigned*>(v7 + 72));
    if (!v8)
        return a2 ? ((a2 < 0) | 4) : (a4 | 4);
    int v9 = v8 - 1;
    if (v9) {
        int v10 = v9 - 1;
        if (v10) {
            if (v10 != 1)
                return 0; // v8 >= 4
            // v8 == 3
            return a2 ? ((a2 < 0) | (2 * (v6 ? 16 : 6))) : (a4 | (2 * (v6 ? 16 : 6)));
        }
        // v8 == 2
        if (!*reinterpret_cast<int*>(a5 + 272))
            return a2 ? ((a2 < 0) | 6) : (a4 | 6);
    }
    // v8 == 1, or (v8 == 2 and *(a5+272)) -> LABEL_25
    if (a2)
        return (a2 < 0) | (2 * (v6 ? 16 : 1));
    return a4 | 4;
}

#endif

BOOL InstallMobDebugHooks() {
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION == 79)
    INITMAPLEHOOK(_CMob_Init, _CMob_Init_t, CMob_Init_Hook, 0x006312A7);
    INITMAPLEHOOK(_CMob_OnResolveMoveAction, _CMob_OnResolveMoveAction_t, CMob_OnResolveMoveAction_Hook, 0x0063A0D2);
#endif
    return TRUE;
}
