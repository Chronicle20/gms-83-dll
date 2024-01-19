#include "pch.h"

CWvsApp *CWvsApp::GetInstance() {
    return reinterpret_cast<CWvsApp *>(*(void **) 0x00BE7B38);
}

void CWvsApp::ISMsgProc(unsigned int message, unsigned int wParam, int lParam) {
    ((VOID(_fastcall * )(CWvsApp * , PVOID, unsigned int message, unsigned int wParam, int lParam))
    0x009F97BC)(this, NULL, message, wParam, lParam);
}

void CWvsApp::InitializeAuth() {
    ((VOID(_fastcall * )(CWvsApp * , PVOID))
    0x009F7097)(this, NULL);
}

void CWvsApp::InitializePCOM() {
    ((VOID(_fastcall * )(CWvsApp * , PVOID))
    0x009F6D77)(this, NULL);
}

void CWvsApp::CreateMainWindow() {
    ((VOID(_fastcall * )(CWvsApp * , PVOID))
    0x009F6D97)(this, NULL);
}

void CWvsApp::ConnectLogin() {
    ((VOID(_fastcall * )(CWvsApp * , PVOID))
    0x009F6F27)(this, NULL);
}

void CWvsApp::InitializeResMan() {
    ((VOID(_fastcall * )(CWvsApp * , PVOID))
    0x009F7159)(this, NULL);
}

void CWvsApp::InitializeGr2D() {
    ((VOID(_fastcall * )(CWvsApp * , PVOID))
    0x009F7A3B)(this, NULL);
}

void CWvsApp::InitializeInput() {
    ((VOID(_fastcall * )(CWvsApp * , PVOID))
    0x009F7CE1)(this, NULL);
}

void CWvsApp::InitializeSound() {
    ((VOID(_fastcall * )(CWvsApp * , PVOID))
    0x009F82BC)(this, NULL);
}

void CWvsApp::InitializeGameData() {
    ((VOID(_fastcall * )(CWvsApp * , PVOID))
    0x009F8B61)(this, NULL);
}

void CWvsApp::CreateWndManager() {
    ((VOID(_fastcall * )(CWvsApp * , PVOID))
    0x009F7034)(this, NULL);
}

void CWvsApp::Dir_BackSlashToSlash(char *string) {
    ((VOID * *(_fastcall * )(char *))
    0x009F95FE)(string);
}

void CWvsApp::Dir_upDir(char *string) {
    ((VOID * *(_fastcall * )(char *))
    0x009F9644)(string);
}

void CWvsApp::Dir_SlashToBackSlash(char *string) {
    ((VOID * *(_fastcall * )(char *))
    0x009F9621)(string);
}
