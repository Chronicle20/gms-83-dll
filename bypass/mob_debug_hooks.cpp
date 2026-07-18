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
    // m_pvc is CMob+0x11C (= p[71]); the resolver reads it. p[72]=m_pvcActive(0x120).
    Log("[mobdbg] CMob::Init  IN this=%p oid=%p tmpl=%p m_pvc(0x11C)=%p", pThis, oid, (void*)p[98], (void*)p[71]);
    _CMob_Init(pThis, oid, pkt);
    Log("[mobdbg] CMob::Init OUT this=%p m_pvc(0x11C)=%p m_pvcActive(0x120)=%p", pThis, (void*)p[71], (void*)p[72]);
}

// static CVecCtrlMob* __cdecl CVecCtrlMob::CreateInstance()  @0x90C422
// If this returns NULL during the bulk re-spawn, the QI into m_pvc leaves it NULL.
typedef void*(__cdecl* _CVecCtrlMob_CreateInstance_t)();
static _CVecCtrlMob_CreateInstance_t _CVecCtrlMob_CreateInstance = nullptr;

void* __cdecl CVecCtrlMob_CreateInstance_Hook() {
    void* r = _CVecCtrlMob_CreateInstance();
    if (!r)
        Log("[mobdbg] CVecCtrlMob::CreateInstance returned NULL");
    return r;
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
    // pThis == CMob+4. p[70] = m_pvc (CMob+0x11C) -- the controller the stock
    // resolver reads. p[69] = the first controller (CMob+0x118).
    //
    // Both v79 and v95 create two CVecCtrlMob in CMob::Init. The difference is
    // which one becomes m_pvc: v95 makes m_pvc the FIRST controller (assigned
    // before its SetActive fires this resolver); v79 makes m_pvc the SECOND
    // controller (assigned AFTER the first controller's SetActive). So on a
    // populated-map re-entry -- where the mob is moving and SetActive resolves a
    // move action during Init -- v79 reads m_pvc before it exists (NULL) -> AV.
    if (p[70] != 0) {
        // m_pvc present -> stock path is correct.
        return _CMob_OnResolveMoveAction(pThis, a2, a3, a4, a5);
    }
    if (p[69] != 0) {
        // m_pvc not created yet, but the first controller is (created + positioned
        // before SetActive). Substitute it so the stock resolver reads a live
        // controller -- exactly what v95 does (resolve against the first controller).
        // Restore afterwards; the real m_pvc is assigned moments later in Init.
        DWORD saved = p[70];
        p[70] = p[69];
        int r = _CMob_OnResolveMoveAction(pThis, a2, a3, a4, a5);
        p[70] = saved;
        return r;
    }
    // Neither controller present (not expected) -> template-only fallback: the
    // stock "no attr" path (v6=0), foothold derived from the mob template.
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
    INITMAPLEHOOK(_CVecCtrlMob_CreateInstance, _CVecCtrlMob_CreateInstance_t, CVecCtrlMob_CreateInstance_Hook,
                  0x0090C422);
#endif
    return TRUE;
}
