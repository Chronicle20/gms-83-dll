// custom_ui_abi.h — public C ABI for the custom-ui-host framework.
// Consumer DLLs include this header AND link nothing; the ABI is resolved
// at runtime via LoadLibrary("custom-ui-host.dll") + GetProcAddress.
//
// ABI version: 1.0.0 (encoded as 0x00010000 by CustomUI_GetAbiVersion).
// All exports are __cdecl extern "C" __declspec(dllexport). Resolve by
// name, never by ordinal. Strings are UTF-8 at the boundary.
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef int CustomUI_WindowHandle;   // 0 = invalid
typedef int CustomUI_CtrlId;
typedef int CustomUI_HotkeyId;
typedef int CustomUI_HandlerId;

typedef void(__cdecl *CustomUI_OnClickFn)(CustomUI_WindowHandle w,
                                          CustomUI_CtrlId c, void *user);

typedef void(__cdecl *CustomUI_PacketHandlerFn)(unsigned short opcode,
                                                const unsigned char *payload,
                                                unsigned int payloadLen,
                                                void *user);

#define CUSTOM_UI_MOD_SHIFT 0x01
#define CUSTOM_UI_MOD_CTRL  0x02
#define CUSTOM_UI_MOD_ALT   0x04

/* Lifecycle */
__declspec(dllexport) unsigned int __cdecl CustomUI_GetAbiVersion(void);
__declspec(dllexport) int          __cdecl CustomUI_IsReady(void);

/* Windows */
__declspec(dllexport) CustomUI_WindowHandle __cdecl
    CustomUI_CreateWindow(const char *title, int x, int y, int w, int h,
                          void *user);
__declspec(dllexport) int __cdecl CustomUI_ShowWindow(CustomUI_WindowHandle h);
__declspec(dllexport) int __cdecl CustomUI_HideWindow(CustomUI_WindowHandle h);
__declspec(dllexport) int __cdecl CustomUI_DestroyWindow(CustomUI_WindowHandle h);

/* Controls */
__declspec(dllexport) CustomUI_CtrlId __cdecl
    CustomUI_AddLabel(CustomUI_WindowHandle h, int x, int y, const char *text);
__declspec(dllexport) CustomUI_CtrlId __cdecl
    CustomUI_AddButton(CustomUI_WindowHandle h, int x, int y, int w, int h_,
                       const char *text, CustomUI_OnClickFn onClick);
__declspec(dllexport) CustomUI_CtrlId __cdecl
    CustomUI_AddEdit(CustomUI_WindowHandle h, int x, int y, int w, int h_,
                     const char *initialText);
__declspec(dllexport) int __cdecl
    CustomUI_SetLabelText(CustomUI_WindowHandle h, CustomUI_CtrlId c,
                          const char *text);
__declspec(dllexport) int __cdecl
    CustomUI_GetEditText(CustomUI_WindowHandle h, CustomUI_CtrlId c, char *buf,
                         int bufLen);

/* Hotkeys */
__declspec(dllexport) CustomUI_HotkeyId __cdecl
    CustomUI_BindHotkey(unsigned int vk, unsigned int modifiers,
                        CustomUI_WindowHandle target);
__declspec(dllexport) int __cdecl
    CustomUI_UnbindHotkey(CustomUI_HotkeyId id);

/* Packets */
__declspec(dllexport) int __cdecl
    CustomUI_SendPacket(unsigned short opcode, const void *payload,
                        unsigned int len);
__declspec(dllexport) CustomUI_HandlerId __cdecl
    CustomUI_RegisterPacketHandler(unsigned short opcode,
                                   CustomUI_PacketHandlerFn fn, void *user);
__declspec(dllexport) int __cdecl
    CustomUI_UnregisterPacketHandler(CustomUI_HandlerId id);

/* Debug */
__declspec(dllexport) void __cdecl CustomUI_DumpRegistries(void);

#ifdef __cplusplus
}
#endif
