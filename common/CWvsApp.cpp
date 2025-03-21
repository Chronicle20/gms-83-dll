#include "pch.h"
#include "memory_map.h"

CWvsApp *CWvsApp::GetInstance() {
    return reinterpret_cast<CWvsApp *>(*(void **) C_WVS_APP_GET_INSTANCE);
}

void CWvsApp::ISMsgProc(unsigned int message, unsigned int wParam, int lParam) {
    ((VOID(_fastcall * )(CWvsApp * , PVOID, unsigned int message, unsigned int wParam, int lParam))
    C_WVS_APP_IS_MSG_PROC)(this, NULL, message, wParam, lParam);
}

void CWvsApp::InitializeAuth() {
    ((VOID(_fastcall * )(CWvsApp * , PVOID))
    C_WVS_APP_INITIALIZE_AUTH)(this, NULL);
}

void CWvsApp::InitializePCOM() {
    ((VOID(_fastcall * )(CWvsApp * , PVOID))
    C_WVS_APP_INITIALIZE_PCOM)(this, NULL);
}

void CWvsApp::CreateMainWindow() {
    ((VOID(_fastcall * )(CWvsApp * , PVOID))
    C_WVS_APP_CREATE_MAIN_WINDOW)(this, NULL);
}

void CWvsApp::ConnectLogin() {
    ((VOID(_fastcall * )(CWvsApp * , PVOID))
    C_WVS_APP_CONNECT_LOGIN)(this, NULL);
}

void CWvsApp::InitializeResMan() {
    ((VOID(_fastcall * )(CWvsApp * , PVOID))
    C_WVS_APP_INITIALIZE_RES_MAN)(this, NULL);
}

void CWvsApp::InitializeGr2D() {
    ((VOID(_fastcall * )(CWvsApp * , PVOID))
    C_WVS_APP_INITIALIZE_GR2D)(this, NULL);
}

void CWvsApp::InitializeInput() {
    ((VOID(_fastcall * )(CWvsApp * , PVOID))
    C_WVS_APP_INITIALIZE_INPUT)(this, NULL);
}

void CWvsApp::InitializeSound() {
    ((VOID(_fastcall * )(CWvsApp * , PVOID))
    C_WVS_APP_INITIALIZE_SOUND)(this, NULL);
}

void CWvsApp::InitializeGameData() {
    ((VOID(_fastcall * )(CWvsApp * , PVOID))
    C_WVS_APP_INITIALIZE_GAME_DATA)(this, NULL);
}

void CWvsApp::CreateWndManager() {
    ((VOID(_fastcall * )(CWvsApp * , PVOID))
    C_WVS_APP_CREATE_WND_MANAGER)(this, NULL);
}

void CWvsApp::Dir_BackSlashToSlash(char *string) {
    ((VOID * *(_fastcall * )(char *))
            C_WVS_APP_DIR_BACK_SLASH_TO_SLASH)(string);
}

void CWvsApp::Dir_upDir(char *string) {
    ((VOID * *(_fastcall * )(char *))
            C_WVS_APP_DIR_UP_DIR)(string);
}

void CWvsApp::Dir_SlashToBackSlash(char *string) {
    ((VOID * *(_fastcall * )(char *))
    C_WVS_APP_DIR_SLASH_TO_BACK_SLASH)(string);
}
