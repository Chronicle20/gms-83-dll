#pragma once
#include "abi/custom_ui_abi.h"

namespace custom_ui_demo {

struct ResolvedAbi {
    unsigned int (__cdecl *GetAbiVersion)();
    int (__cdecl *IsReady)();
    CustomUI_WindowHandle (__cdecl *CreateWindow)(const char *, int, int, int,
                                                  int, void *);
    int (__cdecl *ShowWindow)(CustomUI_WindowHandle);
    int (__cdecl *HideWindow)(CustomUI_WindowHandle);
    int (__cdecl *DestroyWindow)(CustomUI_WindowHandle);
    CustomUI_CtrlId (__cdecl *AddLabel)(CustomUI_WindowHandle, int, int,
                                        const char *);
    CustomUI_CtrlId (__cdecl *AddButton)(CustomUI_WindowHandle, int, int, int,
                                         int, const char *,
                                         CustomUI_OnClickFn);
    int (__cdecl *SetLabelText)(CustomUI_WindowHandle, CustomUI_CtrlId,
                                const char *);
    CustomUI_HotkeyId (__cdecl *BindHotkey)(unsigned int, unsigned int,
                                            CustomUI_WindowHandle);
    int (__cdecl *SendPacket)(unsigned short, const void *, unsigned int);
    CustomUI_HandlerId (__cdecl *RegisterPacketHandler)(
        unsigned short, CustomUI_PacketHandlerFn, void *);
};

extern ResolvedAbi g_abi;
extern CustomUI_WindowHandle g_window;
extern CustomUI_CtrlId g_label;
extern int g_ping_count;

void OnPing(CustomUI_WindowHandle, CustomUI_CtrlId, void *);
void OnPong(unsigned short, const unsigned char *, unsigned int, void *);

} // namespace custom_ui_demo
