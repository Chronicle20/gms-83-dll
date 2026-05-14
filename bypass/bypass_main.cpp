#include "pch.h"

#include "app_hooks.h"
#include "security_hooks.h"
#include "login_hooks.h"
#include "socket_hooks.h"
#include "key_mapped_hooks.h"

DWORD WINAPI MainProc(LPVOID lpParam) {
    if (!InstallAppHooks())        return 0;
    if (!InstallSecurityHooks())   return 0;
    if (!InstallLoginHooks())      return 0;
    if (!InstallSocketHooks())     return 0;
    if (!InstallKeyMappedHooks())  return 0;
    return 0;
}
