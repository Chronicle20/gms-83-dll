#include "pch.h"

CLogo::CLogo() {
    Log("CLogo::CLogo");
    reinterpret_cast<void (__fastcall *)(CLogo *, void *)>(C_LOGO)(this, nullptr);
}

const CRTTI *CLogo::GetRTTI() {
    return reinterpret_cast<const CRTTI *(__fastcall *)(CLogo *, void *)>(C_LOGO_GET_RTTI)(this, nullptr);
}

int CLogo::IsKindOf(const CRTTI *pRTTI) {
    return reinterpret_cast<int (__fastcall *)(CLogo *, void *, const CRTTI *)>(C_LOGO_IS_KIND_OF)(this, nullptr,
                                                                                                   pRTTI);
}

void CLogo::Init() {
    Log("CLogo::Init");
    reinterpret_cast<void (__fastcall *)(CLogo *, void *)>(C_LOGO_INIT)(this, nullptr);
}

void CLogo::InitNXLogo() {
    reinterpret_cast<void (__thiscall *)(CLogo *)>(C_LOGO_INIT_NX_LOGO)(this);
}

void CLogo::Update() {
    reinterpret_cast<void (__fastcall *)(CLogo *, void *)>(C_LOGO_UPDATE)(this, nullptr);
}

void CLogo::LogoEnd() {
    Log("CLogo::ForcedEnd");
    reinterpret_cast<void (__fastcall *)(CLogo *, void *)>(C_LOGO_LOGO_END)(this, nullptr);
}

void CLogo::ForcedEnd() {
    Log("CLogo::ForcedEnd");
    reinterpret_cast<void (__fastcall *)(CLogo *, void *)>(C_LOGO_FORCED_END)(this, nullptr);
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
    reinterpret_cast<void (__fastcall *)(CLogo *, void *, unsigned int, unsigned int, int, int)>(
            C_LOGO_ON_MOUSE_BUTTON)(this, nullptr, msg, wParam, rx, ry);
}

int CLogo::OnSetFocus(int bFocus) {
    return reinterpret_cast<int (__fastcall *)(CLogo *, void *, int)>(
            C_LOGO_ON_SET_FOCUS)(this, nullptr, bFocus);
}

void CLogo::OnKey(unsigned int wParam, unsigned int lParam) {
    reinterpret_cast<void (__fastcall *)(CLogo *, void *, unsigned int, unsigned int)>(
            C_LOGO_ON_KEY)(this, nullptr, wParam, lParam);
}