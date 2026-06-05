#include "pch.h"

CCtrlEdit::CCtrlEdit() {
    // Game-side ctor at C_CTRL_EDIT_CTOR is nullary (verified v83.1:
    // ??0CCtrlEdit@@QAE@XZ). It chains CCtrlWnd::CCtrlWnd, zeros members,
    // and installs 3 vptrs. Geometry/init-text are applied later via the
    // virtual CreateCtrl, wired by the framework's control-creation layer
    // (Phase 6), not here.
    reinterpret_cast<void(__fastcall *)(CCtrlEdit *, void *)>(
        C_CTRL_EDIT_CTOR)(this, nullptr);
}
