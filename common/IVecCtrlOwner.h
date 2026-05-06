#pragma once

interface IVecCtrlOwner {
public:
    virtual ~IVecCtrlOwner() = default;

    virtual int *GetType(IVecCtrlOwner *) = 0;
    virtual int *OnResolveMoveAction(IVecCtrlOwner *, int, int, int, int *) = 0;
    virtual void *OnLayerZChanged(IVecCtrlOwner *, int *) = 0;
    virtual ZRef<CAttrShoe> *GetShoeAttr(IVecCtrlOwner *, ZRef<CAttrShoe> *) = 0;
    virtual tagPOINT *GetPos(IVecCtrlOwner *) = 0;
    virtual tagPOINT *GetPosPrev(IVecCtrlOwner *) = 0;
    virtual int *GetZMass(IVecCtrlOwner *) = 0;
    virtual CRTTI *GetRTTI(IVecCtrlOwner *) = 0;
    virtual int *IsKindOf(IVecCtrlOwner *, const CRTTI *) = 0;
};