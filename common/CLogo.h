#pragma once

class CLogo : public CStage {
public:
    CLogo();

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

