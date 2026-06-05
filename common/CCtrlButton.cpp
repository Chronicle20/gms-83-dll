#include "pch.h"

CCtrlButton::CCtrlButton() {
    // Game-side ctor at C_CTRL_BUTTON_CTOR is nullary (verified v83.1:
    // ??0CCtrlButton@@QAE@XZ). Geometry/params are applied later via the
    // virtual CreateCtrl (0x004BFFFB), wired by the framework's control-
    // creation layer (Phase 6), not here.
    reinterpret_cast<void(__fastcall *)(CCtrlButton *, void *)>(
        C_CTRL_BUTTON_CTOR)(this, nullptr);
}
