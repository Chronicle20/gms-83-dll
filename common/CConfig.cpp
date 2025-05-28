#include "pch.h"
#include "memory_map.h"

CConfig::CConfig() {
    Log("CConfig::CConfig");
    reinterpret_cast<void (__fastcall *)(CConfig *, void *)>(C_CONFIG)(this, nullptr);
};

CConfig *CConfig::GetInstance() {
    Log("CConfig::GetInstance");
    return reinterpret_cast<CConfig *>(*reinterpret_cast<void **>(C_CONFIG_INSTANCE_ADDR));
}

INT CConfig::GetPartnerCode() {
    Log("CConfig::GetPartnerCode");
    return reinterpret_cast<INT(__fastcall *)(CConfig *, void *)>(C_CONFIG_GET_PARTNER_CODE)(this, nullptr);
}

void CConfig::ApplySysOpt(int *pSysOpt, int bApplyVideo) {
    Log("CConfig::ApplySysOpt");
    reinterpret_cast<void (__fastcall *)(CConfig *, void *, int *, int)>(
            C_CONFIG_APPLY_SYS_OPT)(this, nullptr, pSysOpt, bApplyVideo);
}

void CConfig::CheckExecPathReg(ZXString<char> sModulePath) {
    Log("CConfig::CheckExecPathReg");
    reinterpret_cast<void (__fastcall *)(CConfig *, void *, ZXString<char>)>(
            C_CONFIG_CHECK_EXEC_PATH_REG)(this, nullptr, sModulePath);
}