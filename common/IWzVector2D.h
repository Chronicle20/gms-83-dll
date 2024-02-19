#undef  INTERFACE
#define INTERFACE   IWzVector2D

DECLARE_INTERFACE_(IWzVector2D, IWzShape2D) {
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

// ** IWzVector2D methods ***

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

    END_INTERFACE
};
