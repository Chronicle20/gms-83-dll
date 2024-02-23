#include "pch.h"
#include "CWvsApp.h"


CWvsApp *CWvsApp::GetInstance() {
    return reinterpret_cast<CWvsApp *>(*(void **) 0x00C9A228);
}

void CWvsApp::ISMsgProc(unsigned int message, unsigned int wParam, int lParam) {
    ((VOID(_fastcall * )(CWvsApp * , PVOID, unsigned int message, unsigned int wParam, int lParam))
    0x00A8DC7D)(this, NULL, message, wParam, lParam);
}

void CWvsApp::InitializeAuth() {
    ((VOID(_fastcall * )(CWvsApp * , PVOID))
    0x00A8A5AD)(this, NULL);
}

void CWvsApp::InitializePCOM() {
    ((VOID(_fastcall * )(CWvsApp * , PVOID))
    0x00A8A162)(this, NULL);
}

void CWvsApp::CreateMainWindow() {
    ((VOID(_fastcall * )(CWvsApp * , PVOID))
    0x00A8A191)(this, NULL);
}

void CWvsApp::ConnectLogin() {
    ((VOID(_fastcall * )(CWvsApp * , PVOID))
    0x00A8A3A8)(this, NULL);
}

void CWvsApp::InitializeResMan() {
    ((VOID(_fastcall * )(CWvsApp * , PVOID))
    0x00A8A6A3)(this, NULL);
}

void CWvsApp::InitializeGr2D() {
    ((VOID(_fastcall * )(CWvsApp * , PVOID))
    0x00A8B61A)(this, NULL);
}

void CWvsApp::InitializeInput() {
    ((VOID(_fastcall * )(CWvsApp * , PVOID))
    0x00A8BA1B)(this, NULL);
}

void CWvsApp::InitializeSound() {
    ((VOID(_fastcall * )(CWvsApp * , PVOID))
    0x00A8BFF6)(this, NULL);
}

void CWvsApp::InitializeGameData() {
    ((VOID(_fastcall * )(CWvsApp * , PVOID))
    0x00A8C8E7)(this, NULL);
}

void CWvsApp::CreateWndManager() {
    ((VOID(_fastcall * )(CWvsApp * , PVOID))
    0x00A8A533)(this, NULL);
}

ZXString<char> * CWvsApp::GetCmdLine(ZXString<char> *result, int nArg) {
    return ((ZXString<char> *(_fastcall * )(CWvsApp * , PVOID, ZXString<char> *result, int nArg))
    0x00A8D77D)(this, NULL, result, nArg);
}

void CWvsApp::Dir_BackSlashToSlash(char *string) {
    ((VOID * *(_fastcall * )(char *))
    0x00A8D9C3)(string);
}

void CWvsApp::Dir_upDir(char *string) {
    ((VOID * *(_fastcall * )(char *))
    0x00A8DA45)(string);
}

void CWvsApp::Dir_SlashToBackSlash(char *string) {
    ((VOID * *(_fastcall * )(char *))
    0x00A8DA04)(string);
}

char *CWvsApp::GetExceptionFileName() {
    return ((char *(_cdecl * )())
    0x00A8DCF6)();
}
