#pragma once

class CLogo : public CStage {
public:
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95)
    enum class VIDEO_STATE : int32_t {
        VIDEO_STATE_UNAVAILABLE = 0x0,
        VIDEO_STATE_OPENING = 0x1,
        VIDEO_STATE_READY = 0x2,
        VIDEO_STATE_PLAYING = 0x3,
        VIDEO_STATE_FADEOUT = 0x4,
        VIDEO_STATE_END = 0x5,
    };
#endif

#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95)
    _com_ptr_t<_com_IIID<IWzGr2DLayer, &IID_IUnknown>> m_pLayerBackground;
#endif
    _com_ptr_t<_com_IIID<IWzGr2DLayer, &IID_IUnknown>> m_pLayerMain;
    _com_ptr_t<_com_IIID<IWzProperty, &IID_IUnknown>> m_pLogoProp;
    int m_nLogoCount;
    unsigned int m_dwTickInitial;
    unsigned int m_dwClick;
    int m_bLogoSoundPlayed;
    int m_bWZInit;
    int m_bNXFadeIn;
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95) || defined(REGION_JMS)
    int m_bNXFadeOut;
#endif
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95)
    int m_bVideoMode;
    CLogo::VIDEO_STATE m_videoState;
#endif
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 111)
    int dummy1;
#endif

    CLogo();

    void Init();

    void InitNXLogo();

    void LogoEnd();

    void ForcedEnd();

    void Update() override;

    void OnKey(unsigned int, unsigned int) override;

    int OnSetFocus(int) override;

    void OnMouseButton(unsigned int, unsigned int, int, int) override;

    int OnMouseMove(int, int) override;

    int OnMouseWheel(int, int, int) override;

    void OnDraggableMove(int, int *, int, int) override;

    void SetEnable(int) override;

    int IsEnabled() override;

    void SetShow(int) override;

    int IsShown() override;

    int GetAbsLeft() override;

    int GetAbsTop() override;

    void ClearToolTip() override;

    void OnIMEModeChange(char) override;

    void OnIMEResult(const char *) override;

    void OnIMEComp(const char *, ZArray<unsigned long> *, unsigned int, int,
                            ZList<ZXString<char> > *, int, int, int) override;

    const CRTTI *GetRTTI() override;

    int IsKindOf(const CRTTI *) override;
};

// CLogo: Alloc(sizeof(CLogo)) in CWvsApp::SetUp, then the real ctor runs into it -> our struct
// must be >= the real per-version size or it overruns the heap. Real (size sweep):
// v83/v84/v87 = 0x38, v111 = 0x4C (v95/JMS TBD).
#if defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 111
static_assert(sizeof(CLogo) >= 0x4C, "CLogo too small for GMS v111 (need >= 0x4C) -> SetUp heap overflow");
#elif defined(REGION_GMS) && BUILD_MAJOR_VERSION == 95
static_assert(sizeof(CLogo) == 0x48, "CLogo must match the v95 PDB base exactly (0x48)");
#elif defined(REGION_GMS)
static_assert(sizeof(CLogo) >= 0x38, "CLogo too small (need >= 0x38) -> SetUp heap overflow");
#endif

