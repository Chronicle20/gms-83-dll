#include "pch.h"

typedef VOID(__cdecl *_CQuickslotKeyMappedMan__CreateInstance_t)();
_CQuickslotKeyMappedMan__CreateInstance_t _CQuickslotKeyMappedMan__CreateInstance = reinterpret_cast<_CQuickslotKeyMappedMan__CreateInstance_t>(0x00ADCFDF);

void CQuickslotKeyMappedMan::CreateInstance() {
    _CQuickslotKeyMappedMan__CreateInstance();
}