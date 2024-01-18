#undef  INTERFACE
#define INTERFACE   IWzGr2D

DECLARE_INTERFACE_(IWzGr2D, IUnknown) {
    BEGIN_INTERFACE

// *** IUnknown methods ***
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void **ppv) PURE;

    STDMETHOD_(ULONG, AddRef)(THIS) PURE;

    STDMETHOD_(ULONG, Release)(THIS) PURE;

// ** IWzGr2D methods ***
    STDMETHOD(Initialize)(THIS_ unsigned int, unsigned int, tagVARIANT, tagVARIANT, tagVARIANT) PURE;

    STDMETHOD(Uninitialize)(THIS) PURE;

    STDMETHOD(GetnextRenderTime)(THIS_ int *) PURE;

    STDMETHOD(UpdateCurrentTime)(THIS_ int) PURE;

    STDMETHOD(RenderFrame)(THIS) PURE;

    STDMETHOD(SetFrameSkip)(THIS) PURE;

    STDMETHOD(ToggleFpsPanel)(THIS) PURE;

    STDMETHOD(DisableFpsPanel)(THIS) PURE;

    STDMETHOD(Getwidth)(THIS_ unsigned int *) PURE;

    STDMETHOD(Getheight)(THIS_ unsigned int *) PURE;

    STDMETHOD(PutscreenResolution)(THIS_ unsigned int, unsigned int) PURE;

    STDMETHOD(Getbpp)(THIS_ unsigned int *) PURE;

    STDMETHOD(GetrefreshRate)(THIS_ unsigned int *) PURE;

    STDMETHOD(Getfps100)(THIS_ unsigned int *) PURE;

    STDMETHOD(GetcurrentTime)(THIS_ int *) PURE;

    STDMETHOD(GetfullScreen)(THIS_ int *) PURE;

    STDMETHOD(PutfullScreen)(THIS_ int) PURE;

    STDMETHOD(GetbackColor)(THIS_ unsigned int *) PURE;

    STDMETHOD(PutbackColor)(THIS_ unsigned int) PURE;

    STDMETHOD(GetredTone)(THIS_ IWzVector2D **) PURE;

    STDMETHOD(GetgreenBlueTone)(THIS_ IWzVector2D **) PURE;

    STDMETHOD(Getcenter)(THIS_ IWzVector2D **) PURE;

    STDMETHOD(GetSnapshot)(THIS_ tagVARIANT, unsigned int, int, int) PURE;

    STDMETHOD(CreateLayer)(THIS_ int, int, unsigned int, unsigned int, int, tagVARIANT, tagVARIANT,
                           IWzGr2DLayer **) PURE;

    STDMETHOD(AdjustCenter)(THIS_ int, int) PURE;

    STDMETHOD(TakeScreenShot)(THIS_ wchar_t *, int) PURE;

    STDMETHOD(SetVideoMode)(THIS_ int) PURE;

    STDMETHOD(SetVideoPath)(THIS_ wchar_t *, int, int) PURE;

    STDMETHOD(PlayVideo)(THIS) PURE;

    STDMETHOD(PauseVideo)(THIS_ int) PURE;

    STDMETHOD(StopVideo)(THIS) PURE;

    STDMETHOD(GetvideoStatus)(THIS_ int *) PURE;

    STDMETHOD(PutvideoVolume)(THIS_ int) PURE;

    END_INTERFACE
};