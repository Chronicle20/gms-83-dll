#include "pch.h"

typedef VOID(__cdecl *_CAnimationDisplayer__CreateInstance_t)();
_CAnimationDisplayer__CreateInstance_t _CAnimationDisplayer__CreateInstance = reinterpret_cast<_CAnimationDisplayer__CreateInstance_t>(0x00ADCD21);

void CAnimationDisplayer::CreateInstance() {
    _CAnimationDisplayer__CreateInstance();
}