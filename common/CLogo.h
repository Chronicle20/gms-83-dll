#pragma once

class CLogo : public CStage {
public:
    CLogo();

    void Update() override;

    void OnKey(IUIMsgHandler *, unsigned int, unsigned int) override;

    int OnSetFocus(IUIMsgHandler *, int) override;

    void OnMouseButton(IUIMsgHandler *, unsigned int, unsigned int, int, int) override;

    int OnMouseMove(IUIMsgHandler *, int, int) override;

    int OnMouseWheel(IUIMsgHandler *, int, int, int) override;

    void OnDraggableMove(IUIMsgHandler *, int, int *, int, int) override;

    void SetEnable(IUIMsgHandler *, int) override;

    int IsEnabled(IUIMsgHandler *) override;

    void SetShow(IUIMsgHandler *, int) override;

    int IsShown(IUIMsgHandler *) override;

    int GetAbsLeft(IUIMsgHandler *) override;

    int GetAbsTop(IUIMsgHandler *) override;

    void ClearToolTip(IUIMsgHandler *) override;

    void OnIMEModeChange(IUIMsgHandler *, char) override;

    void OnIMEResult(IUIMsgHandler *, const char *) override;

    void OnIMEComp(IUIMsgHandler *, const char *, ZArray<unsigned long> *, unsigned int, int,
                            ZList<ZXString<char> > *, int, int, int) override;

    const CRTTI *GetRTTI(IUIMsgHandler *) override;

    int IsKindOf(IUIMsgHandler *, const CRTTI *) override;
};

