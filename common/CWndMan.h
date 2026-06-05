#pragma once

struct CUIWnd;

class CWndMan {
public:
  static CWnd** s_Update();
  static void RedrawInvalidatedWindows();

  void RegisterUIWindow(CUIWnd* pWnd);
  void UnregisterUIWindow(CUIWnd* pWnd);
  long ProcessKey(unsigned int msg, unsigned int vk, long lParam);
};
