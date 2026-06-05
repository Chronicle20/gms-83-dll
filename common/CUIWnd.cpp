#include "pch.h"

CUIWnd::CUIWnd(int nUIType, int closeType, int closeX, int closeY,
               const unsigned short *sBackgrndUOL, int nBackgrndX,
               int nBackgrndY) {
    // Game-side ctor at C_UI_WND_CTOR. Real v83.1 signature recovered from
    // IDA: (nUIType, closeType, closeX, closeY, sBackgrndUOL, nBackgrndX,
    // nBackgrndY); __thiscall, retn 1Ch. NOT the (x,y,w,h,name) form the
    // plan originally assumed.
    reinterpret_cast<void(__fastcall *)(CUIWnd *, void *, int, int, int, int,
                                        const unsigned short *, int, int)>(
        C_UI_WND_CTOR)(this, nullptr, nUIType, closeType, closeX, closeY,
                       sBackgrndUOL, nBackgrndX, nBackgrndY);
}
