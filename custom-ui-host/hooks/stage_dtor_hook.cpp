#include "pch.h"

#include "hooks/stage_dtor_hook.h"
#include "hooks/stage_restore.h"

#include "runtime/custom_ui_wnd.h"

#include "hooker.h"
#include "logger.h"
#include "memory_map.h"

namespace custom_ui_host {

std::atomic<bool> g_pending_restore{false};

namespace {

typedef void(__thiscall* StageDtor_t)(CStage*);
StageDtor_t _StageDtor = nullptr;

void __fastcall StageDtor_Hook(CStage* self, void* /*edx*/) {
    static bool s_logged = false;
    if (!s_logged) { s_logged = true; Log("custom-ui-host: StageDtor hook first fire"); }
    SnapshotAndSuspendVisibleWindows();
    g_pending_restore.store(true);
    _StageDtor(self);
}

} // namespace

BOOL InstallStageDtorHook() {
    INITMAPLEHOOK_OR_RETURN(_StageDtor, StageDtor_t, &StageDtor_Hook, C_STAGE_DTOR);
    return TRUE;
}

} // namespace custom_ui_host
