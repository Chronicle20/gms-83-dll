#include "pch.h"

CLogo::CLogo() {
    ((VOID(_fastcall * )(CLogo * , PVOID))
    0x0062ECE2)(this, NULL);
}

const CRTTI *CLogo::GetRTTI(IUIMsgHandler *) {
    return ((const CRTTI *(_fastcall
    * )(CLogo * , PVOID))
    0x0062ED26)(this, NULL);
}

int CLogo::IsKindOf(IUIMsgHandler *, const CRTTI *pRTTI) {
    return ((int (_fastcall * )(CLogo * , PVOID,
    const CRTTI *pRTTI))
    0x0062ED2C)(this, NULL, pRTTI);
}

void CLogo::Update() {
    ((VOID(_fastcall * )(CLogo * , PVOID))
    0x005F4C16)(this, NULL);
}

void CLogo::OnIMEComp(IUIMsgHandler *, const char *, ZArray<unsigned long> *, unsigned int, int,
                      ZList<ZXString<char>> *, int, int, int) {
}

void CLogo::OnIMEResult(IUIMsgHandler *, const char *) {
}

void CLogo::OnIMEModeChange(IUIMsgHandler *, char) {
}

void CLogo::ClearToolTip(IUIMsgHandler *) {
}

int CLogo::GetAbsTop(IUIMsgHandler *) {
    return 0;
}

int CLogo::GetAbsLeft(IUIMsgHandler *) {
    return 0;
}

int CLogo::IsShown(IUIMsgHandler *) {
    return 1;
}

void CLogo::SetShow(IUIMsgHandler *, int) {
}

int CLogo::IsEnabled(IUIMsgHandler *) {
    return 1;
}

void CLogo::SetEnable(IUIMsgHandler *, int) {
}

void CLogo::OnDraggableMove(IUIMsgHandler *, int, int *, int, int) {
}

int CLogo::OnMouseWheel(IUIMsgHandler *, int, int, int) {
    return 0;
}

int CLogo::OnMouseMove(IUIMsgHandler *, int, int) {
    return 0;
}

void CLogo::OnMouseButton(IUIMsgHandler *, unsigned int msg, unsigned int wParam, int rx, int ry) {
    ((VOID(_fastcall * )(CLogo * , PVOID, unsigned int, unsigned int, int, int))
    0x0062F2A1)(this, NULL, msg, wParam, rx, ry);
}

int CLogo::OnSetFocus(IUIMsgHandler *, int bFocus) {
    return ((int(_fastcall * )(CLogo * , PVOID, int))
    0x0062ED20)(this, NULL, bFocus);
}

void CLogo::OnKey(IUIMsgHandler *, unsigned int wParam, unsigned int lParam) {
    ((VOID(_fastcall * )(CLogo * , PVOID, unsigned int, unsigned int))
    0x0062F27A)(this, NULL, wParam, lParam);
}
