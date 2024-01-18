#include "pch.h"

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

// void __thiscall CConfig::ApplySysOpt(CConfig *this, CONFIG_SYSOPT *pSysOpt, int bApplyVideo)
void CConfig::ApplySysOpt(int* pSysOpt, int bApplyVideo) {
    ((VOID(_fastcall * )(CConfig * , PVOID, int* pSysOpt, int bApplyVideo))
    0x0049EA33)(this, NULL, pSysOpt, bApplyVideo);
}

void CConfig::CheckExecPathReg(ZXString<char> sModulePath) {
    ((VOID(_fastcall * )(CConfig * , PVOID, ZXString<char> sModulePath))
    0x0049CCF3)(this, NULL, sModulePath);
}