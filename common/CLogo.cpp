#include "pch.h"

CLogo::CLogo() {
    ((VOID(_fastcall * )(CLogo * , PVOID))
    0x0062ECE2)(this, NULL);
}

const CRTTI *CLogo::GetRTTI() {
    return ((const CRTTI *(_fastcall
    * )(CLogo * , PVOID))
    0x0062ED26)(this, NULL);
}

int CLogo::IsKindOf(const CRTTI *pRTTI) {
    return ((int (_fastcall * )(CLogo * , PVOID,
    const CRTTI *pRTTI))
    0x0062ED2C)(this, NULL, pRTTI);
}

void CLogo::Update() {
    ((VOID(_fastcall * )(CLogo * , PVOID))
    0x005F4C16)(this, NULL);
}

void CLogo::OnIMEComp(const char *, ZArray<unsigned long> *, unsigned int, int,
                      ZList<ZXString<char>> *, int, int, int) {
}

void CLogo::OnIMEResult(const char *) {
}

void CLogo::OnIMEModeChange(char) {
}

void CLogo::ClearToolTip() {
}

int CLogo::GetAbsTop() {
    return 0;
}

int CLogo::GetAbsLeft() {
    return 0;
}

int CLogo::IsShown() {
    return 1;
}

void CLogo::SetShow(int) {
}

int CLogo::IsEnabled() {
    return 1;
}

void CLogo::SetEnable(int) {
}

void CLogo::OnDraggableMove(int, int *, int, int) {
}

int CLogo::OnMouseWheel(int, int, int) {
    return 0;
}

int CLogo::OnMouseMove(int, int) {
    return 0;
}

void CLogo::OnMouseButton(unsigned int msg, unsigned int wParam, int rx, int ry) {
    ((VOID(_fastcall * )(CLogo * , PVOID, unsigned int, unsigned int, int, int))
    0x0062F2A1)(this, NULL, msg, wParam, rx, ry);
}

int CLogo::OnSetFocus(int bFocus) {
    return ((int(_fastcall * )(CLogo * , PVOID, int))
    0x0062ED20)(this, NULL, bFocus);
}

void CLogo::OnKey(unsigned int wParam, unsigned int lParam) {
    ((VOID(_fastcall * )(CLogo * , PVOID, unsigned int, unsigned int))
    0x0062F27A)(this, NULL, wParam, lParam);
}
