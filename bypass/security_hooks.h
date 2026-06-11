#pragma once
#include <Windows.h>

BOOL InstallSecurityHooks();

// v84 only: keep our anti-tamper VEH at the front of the chain (call per frame).
// Defined under the v84 gate in security_hooks.cpp; callers gate the call site.
void RearmAntiTamperVeh();
