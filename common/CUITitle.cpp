#include "pch.h"

CUITitle *CUITitle::GetInstance() {
    return reinterpret_cast<CUITitle *>(*reinterpret_cast<void **>(C_UI_TITLE_GET_INSTANCE));
}