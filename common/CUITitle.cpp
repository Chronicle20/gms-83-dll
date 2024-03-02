#include "pch.h"

CUITitle *CUITitle::GetInstance() {
    return reinterpret_cast<CUITitle *>(*(void **) 0x00CD79A0);
}