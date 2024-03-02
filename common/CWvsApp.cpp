#include "pch.h"
#include "CWvsApp.h"


CWvsApp *CWvsApp::GetInstance() {
    return reinterpret_cast<CWvsApp *>(*(void **) 0x00CD5C40);
}

void CWvsApp::ISMsgProc(unsigned int message, unsigned int wParam, int lParam) {
    ((VOID(_fastcall * )(CWvsApp * , PVOID, unsigned int message, unsigned int wParam, int lParam))
    0x00ADBE51)(this, nullptr, message, wParam, lParam);
}

//TODO I think this doesnt exist
void CWvsApp::InitializeAuth() {
    ((VOID(_fastcall * )(CWvsApp * , PVOID))
    0x00A8A5AD)(this, nullptr);
}

void CWvsApp::InitializePCOM() {
    ((VOID(_fastcall * )(CWvsApp * , PVOID))
    0x00AD9498)(this, nullptr);
}

void CWvsApp::CreateMainWindow() {
    ((VOID(_fastcall * )(CWvsApp * , PVOID))
    0x00AD94C7)(this, nullptr);
}

void CWvsApp::ConnectLogin() {
    ((VOID(_fastcall * )(CWvsApp * , PVOID))
    0x00AD96D0)(this, nullptr);
}

void CWvsApp::InitializeResMan() {
    ((VOID(_fastcall * )(CWvsApp * , PVOID))
    0x00AD98A5)(this, nullptr);
}

void CWvsApp::InitializeGr2D() {
    ((VOID(_fastcall * )(CWvsApp * , PVOID))
    0x00ADA8D7)(this, nullptr);
}

void CWvsApp::InitializeInput() {
    ((VOID(_fastcall * )(CWvsApp * , PVOID))
    0x00ADACA5)(this, nullptr);
}

void CWvsApp::InitializeSound() {
    ((VOID(_fastcall * )(CWvsApp * , PVOID))
    0x00ADADBA)(this, nullptr);
}

void CWvsApp::InitializeGameData() {
    // feels like this needs to be unvirtualized
    ((VOID(_fastcall * )(CWvsApp * , PVOID))
    0x00ADAFA1)(this, nullptr);
}

void CWvsApp::CreateWndManager() {
    ((VOID(_fastcall * )(CWvsApp * , PVOID))
    0x00AD982B)(this, nullptr);
}

ZXString<char> * CWvsApp::GetCmdLine(ZXString<char> *result, int nArg) {
    return ((ZXString<char> *(_fastcall * )(CWvsApp * , PVOID, ZXString<char> *result, int nArg))
    0x00ADB951)(this, nullptr, result, nArg);
}

void CWvsApp::Dir_BackSlashToSlash(char *string) {
    ((VOID * *(_fastcall * )(char *))
    0x00ADBB97)(string);
}

void CWvsApp::Dir_upDir(char *string) {
    ((VOID * *(_fastcall * )(char *))
    0x00ADBC19)(string);
}

void CWvsApp::Dir_SlashToBackSlash(char *string) {
    ((VOID * *(_fastcall * )(char *))
    0x00ADBBD8)(string);
}

char *CWvsApp::GetExceptionFileName() {
    return ((char *(_cdecl * )())
    0x00ADBF06)();
}
