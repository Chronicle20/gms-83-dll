#pragma once

#include "IDraggable.h"
#include "ZList.h"
#include "CRTTI.h"

class IUIMsgHandler {
public:
    virtual ~IUIMsgHandler() = default;

    virtual void *OnKey(IUIMsgHandler *, unsigned int, unsigned int) = 0;

    virtual int *OnSetFocus(IUIMsgHandler *, int) = 0;

    virtual void *OnMouseButton(IUIMsgHandler *, unsigned int, unsigned int, int, int) = 0;

    virtual int *OnMouseMove(IUIMsgHandler *, int, int) = 0;

    virtual int *OnMouseWheel(IUIMsgHandler *, int, int, int) = 0;

    virtual void *OnMouseEnter(IUIMsgHandler *, int) = 0;

    virtual void *OnDraggableMove(IUIMsgHandler *, int, int *, int, int) = 0;

    virtual void *SetEnable(IUIMsgHandler *, int) = 0;

    virtual int *IsEnabled(IUIMsgHandler *) = 0;

    virtual void *SetShow(IUIMsgHandler *, int) = 0;

    virtual int *IsShown(IUIMsgHandler *) = 0;

    virtual int *GetAbsLeft(IUIMsgHandler *) = 0;

    virtual int *GetAbsTop(IUIMsgHandler *) = 0;

    virtual void *ClearToolTip(IUIMsgHandler *) = 0;

    virtual void *OnIMEModeChange(IUIMsgHandler *, char) = 0;

    virtual void *OnIMEResult(IUIMsgHandler *, const char *) = 0;

    virtual void *OnIMEComp(IUIMsgHandler *, const char *, ZArray<unsigned long> *, unsigned int, int,
                            ZList<ZXString<char> > *, int, int, int) = 0;

    virtual const CRTTI *GetRTTI(IUIMsgHandler *) = 0;

    virtual int *IsKindOf(IUIMsgHandler *, const CRTTI *) = 0;
};