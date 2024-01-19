#undef  INTERFACE
#define INTERFACE   IWzGr2DLayer

DECLARE_INTERFACE_(IWzGr2DLayer, IWzVector2D) {
    BEGIN_INTERFACE

// *** IWzShape2D methods ***
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void **ppv) PURE;

    STDMETHOD_(ULONG, AddRef)(THIS) PURE;

    STDMETHOD_(ULONG, Release)(THIS) PURE;

    STDMETHOD(GetpersistentUOL)(THIS_  wchar_t **) PURE;

    STDMETHOD(Serialize)(THIS_ IWzArchive *) PURE;

    STDMETHOD(Getitem)(THIS_ tagVARIANT, tagVARIANT *) PURE;

    STDMETHOD(GetNewEnum)(THIS_ IUnknown **) PURE;

    STDMETHOD(Getcount)(THIS_ unsigned int *) PURE;

    STDMETHOD(Getx)(THIS_ int *) PURE;

    STDMETHOD(Putx)(THIS_ int) PURE;

    STDMETHOD(Gety)(THIS_ int *) PURE;

    STDMETHOD(Puty)(THIS_ int) PURE;

    STDMETHOD(Getx2)(THIS_ int *) PURE;

    STDMETHOD(Putx2)(THIS_ int) PURE;

    STDMETHOD(Gety2)(THIS_ int *) PURE;

    STDMETHOD(Puty2)(THIS_ int) PURE;

    STDMETHOD(Move)(THIS_ int, int) PURE;

    STDMETHOD(Offset)(THIS_ int, int) PURE;

    STDMETHOD(Scale)(THIS_ int, int, int, int, int, int) PURE;

    STDMETHOD(Insert)(THIS_ tagVARIANT, tagVARIANT) PURE;

    STDMETHOD(Remove)(THIS_ tagVARIANT, tagVARIANT *) PURE;

    STDMETHOD(Init)(THIS_ int, int) PURE;

    STDMETHOD(GetcurrentTime)(THIS_ int *) PURE;

    STDMETHOD(PutcurrentTime)(THIS_ int) PURE;

    STDMETHOD(Getorigin)(THIS_ tagVARIANT *) PURE;

    STDMETHOD(Putorigin)(THIS_ tagVARIANT) PURE;

    STDMETHOD(Getrx)(THIS_ int *) PURE;

    STDMETHOD(Putrx)(THIS_ int) PURE;

    STDMETHOD(Getry)(THIS_ int *) PURE;

    STDMETHOD(Putry)(THIS_ int) PURE;

    STDMETHOD(Geta)(THIS_ long double *) PURE;

    STDMETHOD(Getra)(THIS_ long double *) PURE;

    STDMETHOD(Putra)(THIS_ long double) PURE;

    STDMETHOD(GetflipX)(THIS_ int *) PURE;

    STDMETHOD(put_flipX)(THIS_ int) PURE;

    STDMETHOD(GetSnapshot)(THIS_ int *, int *, int *, int *, int *, int *, long double *, long double *, tagVARIANT) PURE;

    STDMETHOD(RelMove)(THIS_ int, int, tagVARIANT, tagVARIANT) PURE;

    STDMETHOD(RelOffset)(THIS_ int, int, tagVARIANT, tagVARIANT) PURE;

    STDMETHOD(Ratio)(THIS_ IWzVector2D *, int, int, int, int) PURE;

    STDMETHOD(WrapClip)(THIS_ tagVARIANT, int, int, unsigned int, unsigned int, tagVARIANT) PURE;

    STDMETHOD(Rotate)(THIS_ long double, tagVARIANT) PURE;

    STDMETHOD(GetlooseLevel)(THIS_ unsigned int *) PURE;

    STDMETHOD(PutlooseLevel)(THIS_ unsigned int) PURE;

    STDMETHOD(Fly)(THIS_ tagVARIANT *, int) PURE;

// ** IWzGr2DLayer methods ***

    STDMETHOD(Getz)(THIS_ int *) PURE;

    STDMETHOD(Putz)(THIS_ int) PURE;

    STDMETHOD(Getwidth)(THIS_ int *) PURE;

    STDMETHOD(Putwidth)(THIS_ int) PURE;

    STDMETHOD(Getheight)(THIS_ int *) PURE;

    STDMETHOD(Putheight)(THIS_ int) PURE;

    STDMETHOD(Getlt)(THIS_ IWzVector2D **) PURE;

    STDMETHOD(Getrb)(THIS_ IWzVector2D **) PURE;

    STDMETHOD(InterlockedOffset)(THIS_ int, int, int, int) PURE;

    STDMETHOD(Getflip)(THIS_ int *) PURE;

    STDMETHOD(Putflip)(THIS_ int) PURE;

    STDMETHOD(Getcolor)(THIS_ unsigned int *) PURE;

    STDMETHOD(Putcolor)(THIS_ unsigned int) PURE;

    STDMETHOD(Getalpha)(THIS_ IWzVector2D **) PURE;

    STDMETHOD(GetredTone)(THIS_ IWzVector2D **) PURE;

    STDMETHOD(GetgreenBlueTone)(THIS_ IWzVector2D **) PURE;

    STDMETHOD(Getblend)(THIS_ LAYER_BLENDTYPE *) PURE;

    STDMETHOD(Putblend)(THIS_ LAYER_BLENDTYPE) PURE;

    STDMETHOD(Getoverlay)(THIS_ tagVARIANT *) PURE;

    STDMETHOD(Putoverlay)(THIS_ tagVARIANT) PURE;

    STDMETHOD(Getcanvas)(THIS_ tagVARIANT, IWzCanvas **) PURE;

    STDMETHOD(InsertCanvas)(THIS_ IWzCanvas *, tagVARIANT, tagVARIANT, tagVARIANT, tagVARIANT, tagVARIANT, tagVARIANT *) PURE;

    STDMETHOD(RemoveCanvas)(THIS_ tagVARIANT, IWzCanvas **) PURE;

    STDMETHOD(ShiftCanvas)(THIS_ tagVARIANT) PURE;

    STDMETHOD(Animate)(THIS_ GR2D_ANITYPE, tagVARIANT, tagVARIANT) PURE;

    STDMETHOD(GetanimationState)(THIS_ int *) PURE;

    STDMETHOD(Getvisible)(THIS_ int *) PURE;

    STDMETHOD(Putvisible)(THIS_ int) PURE;

    END_INTERFACE
};
