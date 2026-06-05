#pragma once

namespace custom_ui_host {

// Cloned CUIWnd vtable. Null until InitCustomUIWndVtable() runs; CustomUIWnd::
// Create patches a placed window's vptr to this once it is set.
extern void *g_cloned_cuiwnd_vtable;

// Clones the 14-slot CUIWnd vtable and overrides slot 8 (OnButtonClicked),
// slot 11 (Draw) and slot 13 (OnCreate). Also initialises the label font
// (non-fatal on failure). Returns false only if the vtable could not be cloned
// (slot count 0 or allocation failure).
bool InitCustomUIWndVtable();

}  // namespace custom_ui_host
