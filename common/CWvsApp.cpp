#include "pch.h"

CWvsApp *CWvsApp::GetInstance() {
    return reinterpret_cast<CWvsApp *>(*reinterpret_cast<void **>(C_WVS_APP_GET_INSTANCE));
}

void CWvsApp::ISMsgProc(unsigned int message, unsigned int wParam, int lParam) {
    reinterpret_cast<void (__fastcall *)(CWvsApp *, void *, unsigned int, unsigned int, int)>(
            C_WVS_APP_IS_MSG_PROC)(this, nullptr, message, wParam, lParam);
}

void CWvsApp::InitializeAuth() {
    Log("CWvsApp::InitializeAuth");
    reinterpret_cast<void (__fastcall *)(CWvsApp *, void *)>(
            C_WVS_APP_INITIALIZE_AUTH)(this, nullptr);
}

void CWvsApp::InitializePCOM() {
    Log("CWvsApp::InitializePCOM");
    reinterpret_cast<void (__fastcall *)(CWvsApp *, void *)>(
            C_WVS_APP_INITIALIZE_PCOM)(this, nullptr);
}

void CWvsApp::CreateMainWindow() {
    Log("CWvsApp::CreateMainWindow");
    reinterpret_cast<void (__fastcall *)(CWvsApp *, void *)>(
            C_WVS_APP_CREATE_MAIN_WINDOW)(this, nullptr);
}

void CWvsApp::ConnectLogin() {
    Log("CWvsApp::ConnectLogin");
    reinterpret_cast<void (__fastcall *)(CWvsApp *, void *)>(
            C_WVS_APP_CONNECT_LOGIN)(this, nullptr);
}

void CWvsApp::InitializeResMan() {
    Log("CWvsApp::InitializeResMan");
    reinterpret_cast<void (__fastcall *)(CWvsApp *, void *)>(
            C_WVS_APP_INITIALIZE_RES_MAN)(this, nullptr);
}

void CWvsApp::InitializeGr2D() {
    Log("CWvsApp::InitializeGr2D");
    reinterpret_cast<void (__fastcall *)(CWvsApp *, void *)>(
            C_WVS_APP_INITIALIZE_GR2D)(this, nullptr);
}

void CWvsApp::InitializeInput() {
    Log("CWvsApp::InitializeInput");
    reinterpret_cast<void (__fastcall *)(CWvsApp *, void *)>(
            C_WVS_APP_INITIALIZE_INPUT)(this, nullptr);
}

void CWvsApp::InitializeSound() {
    Log("CWvsApp::InitializeSound");
    reinterpret_cast<void (__fastcall *)(CWvsApp *, void *)>(
            C_WVS_APP_INITIALIZE_SOUND)(this, nullptr);
}

void CWvsApp::InitializeGameData() {
    Log("CWvsApp::InitializeGameData");
    reinterpret_cast<void (__fastcall *)(CWvsApp *, void *)>(
            C_WVS_APP_INITIALIZE_GAME_DATA)(this, nullptr);
}

void CWvsApp::CreateWndManager() {
    Log("CWvsApp::CreateWndManager");
    reinterpret_cast<void (__fastcall *)(CWvsApp *, void *)>(
            C_WVS_APP_CREATE_WND_MANAGER)(this, nullptr);
}

ZXString<char> *CWvsApp::GetCmdLine(ZXString<char> *result, int nArg) {
    return reinterpret_cast<ZXString<char> *(__fastcall *)(CWvsApp *, void *, ZXString<char> *, int)>(
            C_WVS_APP_GET_CMD_LINE)(this, nullptr, result, nArg);
}

void CWvsApp::Dir_BackSlashToSlash(char *string) {
    reinterpret_cast<void (__fastcall *)(char *)>(
            C_WVS_APP_DIR_BACK_SLASH_TO_SLASH)(string);
}

void CWvsApp::Dir_upDir(char *string) {
    reinterpret_cast<void (__fastcall *)(char *)>(
            C_WVS_APP_DIR_UP_DIR)(string);
}

void CWvsApp::Dir_SlashToBackSlash(char *string) {
    reinterpret_cast<void (__fastcall *)(char *)>(
            C_WVS_APP_DIR_SLASH_TO_BACK_SLASH)(string);
}

char *CWvsApp::GetExceptionFileName() {
    return reinterpret_cast<char *(__cdecl *)()>(C_WVS_APP_GET_EXCEPTION_FILE_NAME)();
}
