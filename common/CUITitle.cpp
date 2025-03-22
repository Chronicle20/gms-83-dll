#include "pch.h"

CUITitle *CUITitle::GetInstance() {
    return reinterpret_cast<CUITitle *>(*(void **) C_UI_TITLE_GET_INSTANCE);
}