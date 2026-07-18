#pragma once

#include <windows.h>

// TEMP DEBUG (task-008): trace the v79 mob re-entry crash (cash-shop return ->
// re-spawn existing mobs -> AV). Remove once diagnosed.
BOOL InstallMobDebugHooks();
