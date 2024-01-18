#include <unknwn.h>

#undef  INTERFACE
#define INTERFACE   IDirectInput8A

DECLARE_INTERFACE_(IDirectInput8A, IUnknown) {
    BEGIN_INTERFACE

// *** IUnknown methods ***
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void **ppv) PURE;

    STDMETHOD_(ULONG, AddRef)(THIS) PURE;

    STDMETHOD_(ULONG, Release)(THIS) PURE;

// ** IDirectInput8A methods ***
    STDMETHOD(CreateDevice)(THIS_ const _GUID *, IDirectInputDevice8A **, IUnknown *) PURE;
    STDMETHOD(EnumDevices)(THIS_ unsigned int, int (__stdcall *)(const DIDEVICEINSTANCEA *, void *), void *, unsigned int) PURE;
    STDMETHOD(GetDeviceStatus)(THIS_ const _GUID *) PURE;
    STDMETHOD(RunControlPanel)(THIS_ HWND__ *, unsigned int) PURE;
    STDMETHOD(Initialize)(THIS_ HINSTANCE__ *, unsigned int) PURE;
    STDMETHOD(FindDevice)(THIS_ const _GUID *, const char *, _GUID *) PURE;
    STDMETHOD(EnumDevicesBySemantics)(THIS_ const char *, int *, int (__stdcall *)(const DIDEVICEINSTANCEA *, IDirectInputDevice8A *, unsigned int, unsigned int, void *), void *, unsigned int) PURE;
    STDMETHOD(ConfigureDevices)(THIS_ int (__stdcall *)(IUnknown *, void *), int *, unsigned int, void *) PURE;
    END_INTERFACE
};