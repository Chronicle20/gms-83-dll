#include "pch.h"
#include "memory_map.h"

CConfig::CConfig() {
    Log("CConfig::CConfig");
    ((VOID(_fastcall * )(CConfig * , PVOID))
    C_CONFIG)(this, nullptr);
};

CConfig *CConfig::GetInstance() {
    Log("CConfig::GetInstance");
    return reinterpret_cast<CConfig *>(*(void **) C_CONFIG_GET_INSTANCE);
}

INT CConfig::GetPartnerCode() {
    Log("CConfig::GetPartnerCode");
    return ((INT(_fastcall * )(CConfig * , PVOID))
    C_CONFIG_GET_PARTNER_CODE)(this, nullptr);
}

void CConfig::ApplySysOpt(int *pSysOpt, int bApplyVideo) {
    Log("CConfig::ApplySysOpt");
    ((VOID(_fastcall * )(CConfig * , PVOID, int * pSysOpt, int
    bApplyVideo))
    C_CONFIG_APPLY_SYS_OPT)(this, nullptr, pSysOpt, bApplyVideo);
}

void CConfig::CheckExecPathReg(ZXString<char> sModulePath) {
    Log("CConfig::CheckExecPathReg");
    ((VOID(_fastcall * )(CConfig * , PVOID, ZXString<char>
    sModulePath))
    C_CONFIG_CHECK_EXEC_PATH_REG)(this, nullptr, sModulePath);
}