#include "pch.h"
#include "memory_map.h"

CConfig *CConfig::GetInstance() {
    return reinterpret_cast<CConfig *>(*(void **) C_CONFIG_GET_INSTANCE);
}

INT CConfig::GetPartnerCode() {
    return ((INT(_fastcall * )(CConfig * , PVOID))
    C_CONFIG_GET_PARTNER_CODE)(this, NULL);
}

void CConfig::ApplySysOpt(int *pSysOpt, int bApplyVideo) {
    ((VOID(_fastcall * )(CConfig * , PVOID, int * pSysOpt, int
    bApplyVideo))
    C_CONFIG_APPLY_SYS_OPT)(this, NULL, pSysOpt, bApplyVideo);
}

void CConfig::CheckExecPathReg(ZXString<char> sModulePath) {
    ((VOID(_fastcall * )(CConfig * , PVOID, ZXString<char>
    sModulePath))
    C_CONFIG_CHECK_EXEC_PATH_REG)(this, NULL, sModulePath);
}