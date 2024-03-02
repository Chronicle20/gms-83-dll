#include "pch.h"

CLogo::CLogo() {
    ((VOID(_fastcall * )(CLogo * , PVOID))
    0x006A0BC2)(this, nullptr);
}

const CRTTI *CLogo::GetRTTI() {
    return ((const CRTTI *(_fastcall
    * )(CLogo * , PVOID))
    0x006A0BFD)(this, nullptr);
}

int CLogo::IsKindOf(const CRTTI *pRTTI) {
    return ((int (_fastcall * )(CLogo * , PVOID,
    const CRTTI *pRTTI))
    0x006A0C03)(this, nullptr, pRTTI);
}

void CLogo::Update() {
    ((VOID(_fastcall * )(CLogo * , PVOID))
    0x006A1069)(this, nullptr);
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
    0x006A1054)(this, nullptr, msg, wParam, rx, ry);
}

int CLogo::OnSetFocus(int bFocus) {
    return ((int(_fastcall * )(CLogo * , PVOID, int))
    0x006A0BF7)(this, nullptr, bFocus);
}

void CLogo::OnKey(unsigned int wParam, unsigned int lParam) {
    ((VOID(_fastcall * )(CLogo * , PVOID, unsigned int, unsigned int))
    0x006A102D)(this, nullptr, wParam, lParam);
}
