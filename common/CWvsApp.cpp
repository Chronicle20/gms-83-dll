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

typedef VOID(__thiscall *_CWvsApp__InitializePCOM_t)(CWvsApp *pThis);
_CWvsApp__InitializePCOM_t _CWvsApp__InitializePCOM = reinterpret_cast<_CWvsApp__InitializePCOM_t>(0x00AD9498);

void CWvsApp::InitializePCOM() {
    _CWvsApp__InitializePCOM(this);
}

typedef VOID(__thiscall *_CWvsApp__CreateMainWindow_t)(CWvsApp *pThis);
_CWvsApp__CreateMainWindow_t _CWvsApp__CreateMainWindow = reinterpret_cast<_CWvsApp__CreateMainWindow_t>(0x00AD94C7);

void CWvsApp::CreateMainWindow() {
    _CWvsApp__CreateMainWindow(this);
}

typedef VOID(__thiscall *_CWvsApp__ConnectLogin_t)(CWvsApp *pThis);
_CWvsApp__ConnectLogin_t _CWvsApp__ConnectLogin = reinterpret_cast<_CWvsApp__ConnectLogin_t>(0x00AD96D0);

void CWvsApp::ConnectLogin() {
    _CWvsApp__ConnectLogin(this);
}

typedef VOID(__thiscall *_CWvsApp__InitializeResMan_t)(CWvsApp *pThis);
_CWvsApp__InitializeResMan_t _CWvsApp__InitializeResMan = reinterpret_cast<_CWvsApp__InitializeResMan_t>(0x00AD98A5);

void CWvsApp::InitializeResMan() {
    _CWvsApp__InitializeResMan(this);
}

typedef VOID(__thiscall *_CWvsApp__InitializeGr2D_t)(CWvsApp *pThis);
_CWvsApp__InitializeGr2D_t _CWvsApp__InitializeGr2D = reinterpret_cast<_CWvsApp__InitializeGr2D_t>(0x00ADA8D7);

void CWvsApp::InitializeGr2D() {
    _CWvsApp__InitializeGr2D(this);
}

typedef VOID(__thiscall *_CWvsApp__InitializeInput_t)(CWvsApp *pThis);
_CWvsApp__InitializeInput_t _CWvsApp__InitializeInput = reinterpret_cast<_CWvsApp__InitializeInput_t>(0x00ADACA5);

void CWvsApp::InitializeInput() {
    _CWvsApp__InitializeInput(this);
}

typedef VOID(__thiscall *_CWvsApp__InitializeSound_t)(CWvsApp *pThis);
_CWvsApp__InitializeSound_t _CWvsApp__InitializeSound = reinterpret_cast<_CWvsApp__InitializeSound_t>(0x00ADADBA);

void CWvsApp::InitializeSound() {
    _CWvsApp__InitializeSound(this);
}

typedef VOID(__thiscall *_CWvsApp__InitializeGameData_t)(CWvsApp *pThis, PVOID first);
_CWvsApp__InitializeGameData_t _CWvsApp__InitializeGameData = reinterpret_cast<_CWvsApp__InitializeGameData_t>(0x00ADAE26);

void CWvsApp::InitializeGameData() {
    // feels like this needs to be unvirtualized
    _CWvsApp__InitializeGameData(this, nullptr);
}

typedef VOID(__thiscall *_CWvsApp__CreateWndManager_t)(CWvsApp *pThis);
_CWvsApp__CreateWndManager_t _CWvsApp__CreateWndManager = reinterpret_cast<_CWvsApp__CreateWndManager_t>(0x00AD982B);

void CWvsApp::CreateWndManager() {
    _CWvsApp__CreateWndManager(this);
}

ZXString<char> * CWvsApp::GetCmdLine(ZXString<char> *result, int nArg) {
    return ((ZXString<char> *(_fastcall * )(CWvsApp * , PVOID, ZXString<char> *result, int nArg))
    0x00ADB951)(this, nullptr, result, nArg);
}

typedef VOID(__cdecl *_CWvsApp__Dir_BackSlashToSlash_t)(char *string);
_CWvsApp__Dir_BackSlashToSlash_t _CWvsApp__Dir_BackSlashToSlash = reinterpret_cast<_CWvsApp__Dir_BackSlashToSlash_t>(0x00ADBB97);

void CWvsApp::Dir_BackSlashToSlash(char *string) {
    _CWvsApp__Dir_BackSlashToSlash(string);
}

typedef VOID(__cdecl *_CWvsApp__Dir_upDir_t)(char *string);
_CWvsApp__Dir_upDir_t _CWvsApp__Dir_upDir = reinterpret_cast<_CWvsApp__Dir_upDir_t>(0x00ADBC19);

void CWvsApp::Dir_upDir(char *string) {
    _CWvsApp__Dir_upDir(string);
}

typedef VOID(__cdecl *_CWvsApp__Dir_SlashToBackSlash_t)(char *string);
_CWvsApp__Dir_SlashToBackSlash_t _CWvsApp__Dir_SlashToBackSlash = reinterpret_cast<_CWvsApp__Dir_SlashToBackSlash_t>(0x00ADBBD8);

void CWvsApp::Dir_SlashToBackSlash(char *string) {
    _CWvsApp__Dir_SlashToBackSlash(string);
}

char *CWvsApp::GetExceptionFileName() {
    return ((char *(_cdecl * )())
    0x00ADBF06)();
}
