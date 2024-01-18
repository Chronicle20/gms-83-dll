#include "pch.h"

CWvsApp *CWvsApp::GetInstance() {
    return reinterpret_cast<CWvsApp *>(*(void **) 0x00BE7B38);
}