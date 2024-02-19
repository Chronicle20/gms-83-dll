#pragma once

#include "CRTTI.h"

class IUIMsgHandler {
public:
    virtual void OnKey(unsigned int, unsigned int) = 0;

    virtual int OnSetFocus(int) = 0;

    virtual void OnMouseButton(unsigned int, unsigned int, int, int) = 0;

    virtual int OnMouseMove(int, int) = 0;

    virtual int OnMouseWheel(int, int, int) = 0;

    virtual void OnMouseEnter(int) = 0;

    virtual void OnDraggableMove(int, int *, int, int) = 0;

    virtual void SetEnable(int) = 0;

    virtual int IsEnabled() = 0;

    virtual void SetShow(int) = 0;

    virtual int IsShown() = 0;

    virtual int GetAbsLeft() = 0;

    virtual int GetAbsTop() = 0;

    virtual void ClearToolTip() = 0;

    virtual void OnIMEModeChange(char) = 0;

    virtual void OnIMEResult(const char *) = 0;

    virtual void OnIMEComp(const char *, ZArray<unsigned long> *, unsigned int, int,
                            ZList<ZXString<char> > *, int, int, int) = 0;

    virtual const CRTTI *GetRTTI() = 0;

    virtual int IsKindOf(const CRTTI *) = 0;
};