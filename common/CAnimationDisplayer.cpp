#include "pch.h"

void CAnimationDisplayer::CreateInstance() {
    Log("CAnimationDisplayer::CreateInstance");
    reinterpret_cast<void (__fastcall *)()>(C_ANIMATION_DISPLAYER_CREATE_INSTANCE)();
}