#include "pch.h"
#include "memory_map.h"

CLogo::CLogo() {
    ((VOID(_fastcall * )(CLogo * , PVOID))
    C_LOGO)(this, NULL);
}

const CRTTI *CLogo::GetRTTI() {
    return ((const CRTTI *(_fastcall
    * )(CLogo * , PVOID))
    C_LOGO_GET_RTTI)(this, NULL);
}

int CLogo::IsKindOf(const CRTTI *pRTTI) {
    return ((int (_fastcall * )(CLogo * , PVOID,
    const CRTTI *pRTTI))
    C_LOGO_IS_KIND_OF)(this, NULL, pRTTI);
}

void CLogo::Update() {
    ((VOID(_fastcall * )(CLogo * , PVOID))
    C_LOGO_UPDATE)(this, NULL);
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
    C_LOGO_ON_MOUSE_BUTTON)(this, NULL, msg, wParam, rx, ry);
}

int CLogo::OnSetFocus(int bFocus) {
    return ((int(_fastcall * )(CLogo * , PVOID, int))
    C_LOGO_ON_SET_FOCUS)(this, NULL, bFocus);
}

void CLogo::OnKey(unsigned int wParam, unsigned int lParam) {
    ((VOID(_fastcall * )(CLogo * , PVOID, unsigned int, unsigned int))
    C_LOGO_ON_KEY)(this, NULL, wParam, lParam);
}
