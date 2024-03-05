#include "pch.h"

CConfig *CConfig::GetInstance() {
    return reinterpret_cast<CConfig *>(*(void **) 0x00CD5690);
}

CConfig::CConfig() {
    ((VOID(_fastcall * )(CConfig * , PVOID))
    0x004B8CA5)(this, nullptr);
};

// int __thiscall CConfig::GetPartnerCode(CConfig *this)
//typedef INT(__fastcall *_CConfig__GetPartnerCode_t)(CConfig *pThis, PVOID edx);

//_CConfig__GetPartnerCode_t _CConfig__GetPartnerCode = reinterpret_cast<_CConfig__GetPartnerCode_t>(0x005F6CFB);
// TODO does not appear to exist in JMS 185
INT CConfig::GetPartnerCode() {
    return ((INT(_fastcall * )(CConfig * , PVOID))
    0x0062E3F4)(this, nullptr);
}

typedef VOID(__thiscall *_CConfig__ApplySysOpt_t)(CConfig *pThis, int *pSysOpt, int bApplyVideo);

_CConfig__ApplySysOpt_t _CConfig__ApplySysOpt = reinterpret_cast<_CConfig__ApplySysOpt_t>(0x004BB741);

// void __thiscall CConfig::ApplySysOpt(CConfig *this, CONFIG_SYSOPT *pSysOpt, int bApplyVideo)
void CConfig::ApplySysOpt(int *pSysOpt, int bApplyVideo) {
    _CConfig__ApplySysOpt(this, pSysOpt, bApplyVideo);
}

typedef VOID(__thiscall *_CConfig__CheckExecPathReg_t)(CConfig *pThis, ZXString<char> sModulePath);
_CConfig__CheckExecPathReg_t _CConfig__CheckExecPathReg = reinterpret_cast<_CConfig__CheckExecPathReg_t>(0x004B98CD);

void CConfig::CheckExecPathReg(ZXString<char> sModulePath) {
    _CConfig__CheckExecPathReg(this, sModulePath);
}