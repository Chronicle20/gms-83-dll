#include "pch.h"

#include "key_mapped_hooks.h"

#include "hooker.h"
#include "logger.h"

typedef VOID(__thiscall* _CFuncKeyMappedMan__CFuncKeyMappedMan_t)(CFuncKeyMappedMan* pThis);

VOID __fastcall CFuncKeyMappedMan__CFuncKeyMappedMan_Hook(CFuncKeyMappedMan* pThis, PVOID edx) {
    Log("CFuncKeyMappedMan::CFuncKeyMappedMan.");

    void** instance = reinterpret_cast<void**>(C_FUNC_KEY_MAPPED_MAN_INSTANCE_ADDR);
    *instance = pThis;

    *(void**)pThis = (void*)C_FUNC_KEY_MAPPED_MAN_VFTABLE;
    memcpy(pThis->m_aFuncKeyMapped, reinterpret_cast<void*>(DEFAULT_FKM_INSTANCE_ADDR),
           sizeof(pThis->m_aFuncKeyMapped));
    memcpy(pThis->m_aFuncKeyMapped_Old, reinterpret_cast<void*>(DEFAULT_FKM_INSTANCE_ADDR),
           sizeof(pThis->m_aFuncKeyMapped_Old));
    memcpy(pThis->m_aQuickslotKeyMapped, reinterpret_cast<void*>(DEFAULT_QKM_INSTANCE_ADDR),
           sizeof(pThis->m_aQuickslotKeyMapped));
    memcpy(pThis->m_aQuickslotKeyMapped_Old, reinterpret_cast<void*>(DEFAULT_QKM_INSTANCE_ADDR),
           sizeof(pThis->m_aQuickslotKeyMapped_Old));

    pThis->m_nPetConsumeItemID = 0;
    pThis->m_nPetConsumeMPItemID = 0;
#if defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 111 || defined(REGION_JMS)
    pThis->dummy1 = 0;
#endif
#if defined(REGION_JMS)
    pThis->dummy2 = 0;
#endif
}

BOOL InstallKeyMappedHooks() {
    HOOKTYPEDEF_C(CFuncKeyMappedMan__CFuncKeyMappedMan);
    INITMAPLEHOOK_OR_RETURN(_CFuncKeyMappedMan__CFuncKeyMappedMan, _CFuncKeyMappedMan__CFuncKeyMappedMan_t,
                            CFuncKeyMappedMan__CFuncKeyMappedMan_Hook, C_FUNC_KEY_MAPPED_MAN);
    return TRUE;
}
