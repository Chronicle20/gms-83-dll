#include "pch.h"

#include "hooks/s_update_hook.h"
#include "hooks/stage_restore.h"

#include "runtime/custom_ui_wnd.h"
#include "runtime/ui_dispatch.h"

#include "hooker.h"
#include "logger.h"
#include "memory_map.h"

namespace custom_ui_host {

namespace {

typedef CWnd**(__cdecl* SUpdate_t)();
SUpdate_t _SUpdate = nullptr;

CWnd** __cdecl SUpdate_Hook() {
    static bool s_logged = false;
    if (!s_logged) { s_logged = true; Log("custom-ui-host: sUpdate hook first fire"); }
    CWnd** rv = _SUpdate();
    if (g_pending_restore.exchange(false)) {
        RestoreSuspendedWindows();
    }
    DrainUIThreadTasks();  // run consumer UI-thread tasks (window/control build)
    return rv;
}

} // namespace

BOOL InstallSUpdateHook() {
    INITMAPLEHOOK_OR_RETURN(_SUpdate, SUpdate_t, &SUpdate_Hook, C_WND_MAN_S_UPDATE);
    return TRUE;
}

} // namespace custom_ui_host
