#include "CConfig.h"

CConfig *CConfig::GetInstance() {
    return reinterpret_cast<CConfig *>(*(void **) 0x00BEBF9C);
}


// int __thiscall CConfig::GetPartnerCode(CConfig *this)
//typedef INT(__fastcall *_CConfig__GetPartnerCode_t)(CConfig *pThis, PVOID edx);

//_CConfig__GetPartnerCode_t _CConfig__GetPartnerCode = reinterpret_cast<_CConfig__GetPartnerCode_t>(0x005F6CFB);
INT CConfig::GetPartnerCode() {
    return ((INT(_fastcall * )(CConfig * , PVOID))
    0x005F6CFB)(this, NULL);
}