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

typedef VOID(__thiscall *_CLogo__Update_t)(CLogo *pThis);
_CLogo__Update_t _CLogo__Update = reinterpret_cast<_CLogo__Update_t>(0x006A1069);

void CLogo::Update() {
    _CLogo__Update(this);
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

typedef VOID(__thiscall *_CLogo__OnMouseButton_t)(CLogo *pThis, unsigned int msg, unsigned int wParam, int rx, int ry);
_CLogo__OnMouseButton_t _CLogo__OnMouseButton = reinterpret_cast<_CLogo__OnMouseButton_t>(0x006A1054);

void CLogo::OnMouseButton(unsigned int msg, unsigned int wParam, int rx, int ry) {
    _CLogo__OnMouseButton(this, msg, wParam, rx, ry);
}

typedef INT(__thiscall *_CLogo__OnSetFocus_t)(CLogo *pThis, int bFocus);
_CLogo__OnSetFocus_t _CLogo__OnSetFocus = reinterpret_cast<_CLogo__OnSetFocus_t>(0x006A0BF7);

int CLogo::OnSetFocus(int bFocus) {
    return _CLogo__OnSetFocus(this, bFocus);
}

typedef VOID(__thiscall *_CLogo__OnKey_t)(CLogo *pThis, unsigned int wParam, unsigned int lParam);
_CLogo__OnKey_t _CLogo__OnKey = reinterpret_cast<_CLogo__OnKey_t>(0x006A102D);

void CLogo::OnKey(unsigned int wParam, unsigned int lParam) {
    _CLogo__OnKey(this, wParam, lParam);
}
