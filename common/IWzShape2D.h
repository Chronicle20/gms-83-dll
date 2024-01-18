#undef  INTERFACE
#define INTERFACE   IWzShape2D

DECLARE_INTERFACE_(IWzShape2D, IWzSerialize) {
    BEGIN_INTERFACE

// *** IWzSerialize methods ***
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void **ppv) PURE;

    STDMETHOD_(ULONG, AddRef)(THIS) PURE;

    STDMETHOD_(ULONG, Release)(THIS) PURE;

    STDMETHOD(GetpersistentUOL)(THIS_  wchar_t **) PURE;

    STDMETHOD(Serialize)(THIS_ IWzArchive *) PURE;

// ** IWzShape2D methods ***

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

    END_INTERFACE
};
