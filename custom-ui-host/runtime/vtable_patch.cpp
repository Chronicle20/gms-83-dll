#include "pch.h"

namespace custom_ui_host {

// Cloned CUIWnd vtable pointer. Null until Task 5.3 initialises it.
// Defined here so the extern declaration in custom_ui_wnd.cpp resolves.
void *g_cloned_cuiwnd_vtable = nullptr;

}  // namespace custom_ui_host
