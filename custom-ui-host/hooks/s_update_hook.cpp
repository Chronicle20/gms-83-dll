#include "pch.h"

#include "hooks/s_update_hook.h"
#include "hooks/stage_restore.h"

#include "runtime/custom_ui_wnd.h"

#include "hooker.h"
#include "logger.h"
#include "memory_map.h"

namespace custom_ui_host {

namespace {

typedef CWnd **(__cdecl *SUpdate_t)();
SUpdate_t _SUpdate = nullptr;

CWnd **__cdecl SUpdate_Hook() {
    CWnd **rv = _SUpdate();
    if (g_pending_restore.exchange(false)) {
        RestoreSuspendedWindows();
    }
    return rv;
}

} // namespace

BOOL InstallSUpdateHook() {
    INITMAPLEHOOK_OR_RETURN(_SUpdate, SUpdate_t, &SUpdate_Hook,
                            C_WND_MAN_S_UPDATE);
    return TRUE;
}

} // namespace custom_ui_host
