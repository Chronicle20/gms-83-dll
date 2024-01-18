#undef  INTERFACE
#define INTERFACE   IDirectInputDevice8A

DECLARE_INTERFACE_(IDirectInputDevice8A, IUnknown) {
    BEGIN_INTERFACE

// *** IUnknown methods ***
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void **ppv) PURE;

    STDMETHOD_(ULONG, AddRef)(THIS) PURE;

    STDMETHOD_(ULONG, Release)(THIS) PURE;

// ** IDirectInputDevice8A methods ***
    END_INTERFACE
};