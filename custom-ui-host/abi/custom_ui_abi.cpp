#include "pch.h"

#include "abi/abi_globals.h"
#include "abi/custom_ui_abi.h"

#include "host_globals.h"
#include "runtime/custom_ui_wnd.h"
#include "runtime/host_config.h"

#include "logger.h"
#include "memory_map.h"

#include <cstring>
#include <new>
#include <string>

namespace custom_ui_host {

std::unique_ptr<WindowRegistry> g_windows;
std::unique_ptr<HotkeyRegistry> g_hotkeys;
std::unique_ptr<PacketRegistry> g_packets;

void InitAbiGlobals() {
    g_windows = std::make_unique<WindowRegistry>();
    g_hotkeys = std::make_unique<HotkeyRegistry>();
    g_packets = std::make_unique<PacketRegistry>(g_config.inbound_op_min, g_config.inbound_op_max);
}

namespace {

bool ReadyOrLog(const char* site) {
    if (custom_ui_host::g_double_load.load()) {
        Log("custom-ui-host: %s called on double-load instance -- ignoring", site);
        return false;
    }
    if (!custom_ui_host::g_ready.load()) {
        Log("custom-ui-host: %s called before ready -- ignoring", site);
        return false;
    }
    return true;
}

} // namespace
} // namespace custom_ui_host

extern "C" {

__declspec(dllexport) unsigned int __cdecl CustomUI_GetAbiVersion(void) {
    return 0x00010000u;
}

__declspec(dllexport) int __cdecl CustomUI_IsReady(void) {
    return custom_ui_host::g_ready.load() ? 1 : 0;
}

__declspec(dllexport) CustomUI_WindowHandle __cdecl CustomUI_CreateWindow(const char* title, int x, int y, int w, int h,
                                                                          void* user) {
    if (!custom_ui_host::ReadyOrLog("CreateWindow"))
        return 0;
    auto* wnd = custom_ui_host::CustomUIWnd::Create(x, y, w, h, title, user);
    if (!wnd)
        return 0;
    auto handle = custom_ui_host::g_windows->Register(wnd);
    wnd->Extras().handle = handle;
    return static_cast<CustomUI_WindowHandle>(handle);
}

__declspec(dllexport) int __cdecl CustomUI_ShowWindow(CustomUI_WindowHandle h) {
    if (!custom_ui_host::ReadyOrLog("ShowWindow"))
        return 0;
    auto* wnd = custom_ui_host::g_windows->Lookup(static_cast<unsigned int>(h));
    if (!wnd)
        return 0;
    wnd->Show();
    return 1;
}

__declspec(dllexport) int __cdecl CustomUI_HideWindow(CustomUI_WindowHandle h) {
    if (!custom_ui_host::ReadyOrLog("HideWindow"))
        return 0;
    auto* wnd = custom_ui_host::g_windows->Lookup(static_cast<unsigned int>(h));
    if (!wnd)
        return 0;
    wnd->Hide();
    return 1;
}

__declspec(dllexport) int __cdecl CustomUI_DestroyWindow(CustomUI_WindowHandle h) {
    if (!custom_ui_host::ReadyOrLog("DestroyWindow"))
        return 0;
    auto* wnd = custom_ui_host::g_windows->Lookup(static_cast<unsigned int>(h));
    if (!wnd)
        return 0;
    custom_ui_host::g_windows->Unregister(static_cast<unsigned int>(h));
    custom_ui_host::CustomUIWnd::Destroy(wnd);
    return 1;
}

/* Controls — Task 6.5. Native CCtrlButton/CCtrlEdit creation + ControlEntry
   population; rendering (Draw override) and click dispatch (slot-8 override)
   are already wired in vtable_patch.cpp. */

namespace {

// CCtrlButton::CREATEPARAM, minimal 16-byte layout per v83_controls.md Q1
// "CREATEPARAM layout": three bools at +0/+1/+2 (m_bAcceptFocus, m_bDrawBack,
// m_bAnimateOnce), then the button-image UOL at +12. CreateCtrl reads param+3
// (byte 12) as a ZXString<unsigned short>; since ZXString<T> is a single
// pointer (m_pStr), a raw const unsigned short* placed there is an equivalent
// by-value ZXString whose backing buffer is our string literal. This matches
// the doc's own recipe (lines 90, 343-344) which uses a raw const wchar_t* at
// offset 12.
struct CtrlButtonCreateParam {
    unsigned char acceptFocus;
    unsigned char drawBack;
    unsigned char animateOnce;
    unsigned char pad;
    unsigned int r0;
    unsigned int r1;
    const unsigned short* uol; // ZXString<unsigned short>::m_pStr (button-image UOL)
};
static_assert(sizeof(CtrlButtonCreateParam) == 16, "CCtrlButton::CREATEPARAM must be 16 bytes (UOL at offset 12)");

// Default button image: a universally-present GMS node with
// normal/mouseOver/pressed/disabled frames.
const unsigned short* const kDefaultButtonUOL = reinterpret_cast<const unsigned short*>(L"UI/Basic.img/BtOK");

} // namespace

__declspec(dllexport) CustomUI_CtrlId __cdecl CustomUI_AddLabel(CustomUI_WindowHandle h, int x, int y,
                                                                const char* text) {
    if (!custom_ui_host::ReadyOrLog("AddLabel"))
        return 0;
    auto* wnd = custom_ui_host::g_windows->Lookup(static_cast<unsigned int>(h));
    if (!wnd)
        return 0;
    auto& fe = wnd->Extras();
    custom_ui_host::CtrlId id = fe.next_ctrl_id++;
    custom_ui_host::ControlEntry entry{};
    entry.id = id;
    entry.kind = custom_ui_host::ControlKind::Label;
    entry.raw = nullptr;
    entry.on_click = nullptr;
    entry.user = nullptr;
    entry.text = text ? text : "";
    entry.draw_x = x;
    entry.draw_y = y;
    fe.controls.push_back(std::move(entry));
    return static_cast<CustomUI_CtrlId>(id);
}

__declspec(dllexport) CustomUI_CtrlId __cdecl CustomUI_AddButton(CustomUI_WindowHandle h, int x, int y, int /*w*/,
                                                                 int /*h_*/, const char* text,
                                                                 CustomUI_OnClickFn onClick) {
    if (!custom_ui_host::ReadyOrLog("AddButton"))
        return 0;
    auto* wnd = custom_ui_host::g_windows->Lookup(static_cast<unsigned int>(h));
    if (!wnd)
        return 0;
    auto& fe = wnd->Extras();

    // 1. Allocate + zero + nullary-construct the CCtrlButton byte buffer.
    void* buf = ::operator new(SIZEOF_C_CTRL_BUTTON_V83_1, std::nothrow);
    if (!buf)
        return 0;
    std::memset(buf, 0, SIZEOF_C_CTRL_BUTTON_V83_1);
    reinterpret_cast<void(__fastcall*)(void*, void*)>(C_CTRL_BUTTON_CTOR)(buf, nullptr);

    // 2. Build the CREATEPARAM (accept focus, default button image).
    CtrlButtonCreateParam param{};
    param.acceptFocus = 1;
    param.uol = kDefaultButtonUOL;

    // 3. CCtrlButton::CreateCtrl(this, parent, id, x, y, decClickArea=0, &param).
    custom_ui_host::CtrlId id = fe.next_ctrl_id++;
    reinterpret_cast<void(__fastcall*)(void*, void*, void*, unsigned int, long, long, long, void*)>(
        C_CTRL_BUTTON_CREATE_CTRL)(buf, nullptr, wnd->GameWnd(), id, x, y, 0, &param);

    // 4. Populate the ControlEntry. Caption text draws on top via the Draw
    //    override; clicks dispatch via slot-8 by matching id.
    custom_ui_host::ControlEntry entry{};
    entry.id = id;
    entry.kind = custom_ui_host::ControlKind::Button;
    entry.raw = buf;
    entry.on_click =
        reinterpret_cast<void(__cdecl*)(custom_ui_host::WindowHandle, custom_ui_host::CtrlId, void*)>(onClick);
    entry.user = nullptr;
    entry.text = text ? text : "";
    entry.draw_x = x;
    entry.draw_y = y;
    fe.controls.push_back(std::move(entry));
    return static_cast<CustomUI_CtrlId>(id);
}

__declspec(dllexport) CustomUI_CtrlId __cdecl CustomUI_AddEdit(CustomUI_WindowHandle h, int x, int y, int w, int h_,
                                                               const char* initialText) {
    if (!custom_ui_host::ReadyOrLog("AddEdit"))
        return 0;
    auto* wnd = custom_ui_host::g_windows->Lookup(static_cast<unsigned int>(h));
    if (!wnd)
        return 0;
    auto& fe = wnd->Extras();

    // 1. Allocate + zero + nullary-construct the CCtrlEdit byte buffer.
    void* buf = ::operator new(SIZEOF_C_CTRL_EDIT_V83_1, std::nothrow);
    if (!buf)
        return 0;
    std::memset(buf, 0, SIZEOF_C_CTRL_EDIT_V83_1);
    reinterpret_cast<void(__fastcall*)(void*, void*)>(C_CTRL_EDIT_CTOR)(buf, nullptr);

    // 2. Build the CCtrlEdit::CREATEPARAM (defaults: Arial 12, black). The doc
    //    (Q2) shows fields through dword idx13 (byte 52); a 64-byte zeroed
    //    local is comfortably larger than the param and is ctor-initialised.
    unsigned char cp[64];
    std::memset(cp, 0, sizeof(cp));
    reinterpret_cast<void(__fastcall*)(void*, void*)>(C_CTRL_EDIT_CREATEPARAM_CTOR)(cp, nullptr);

    // 3. CCtrlWnd::CreateCtrl (CCtrlEdit's inherited slot-2 CreateCtrl):
    //    (this, parent, id, x, y, w, h, &cp).
    custom_ui_host::CtrlId id = fe.next_ctrl_id++;
    reinterpret_cast<void(__fastcall*)(void*, void*, void*, unsigned int, long, long, long, long, void*)>(
        C_CTRL_WND_CREATE_CTRL)(buf, nullptr, wnd->GameWnd(), id, x, y, w, h_, cp);

    // 4. Optional initial text (ANSI).
    if (initialText) {
        reinterpret_cast<void(__fastcall*)(void*, void*, const char*)>(C_CTRL_EDIT_SET_TEXT)(buf, nullptr, initialText);
    }

    // CREATEPARAM dtor (releases its embedded ZXString members).
    reinterpret_cast<void(__fastcall*)(void*, void*)>(C_CTRL_EDIT_CREATEPARAM_DTOR)(cp, nullptr);

    // 5. Populate the ControlEntry. No text: the edit renders itself via its
    //    own child layer.
    custom_ui_host::ControlEntry entry{};
    entry.id = id;
    entry.kind = custom_ui_host::ControlKind::Edit;
    entry.raw = buf;
    entry.on_click = nullptr;
    entry.user = nullptr;
    fe.controls.push_back(std::move(entry));
    return static_cast<CustomUI_CtrlId>(id);
}

__declspec(dllexport) int __cdecl CustomUI_SetLabelText(CustomUI_WindowHandle h, CustomUI_CtrlId c, const char* text) {
    if (!custom_ui_host::ReadyOrLog("SetLabelText"))
        return 0;
    auto* wnd = custom_ui_host::g_windows->Lookup(static_cast<unsigned int>(h));
    if (!wnd)
        return 0;
    auto& fe = wnd->Extras();
    for (custom_ui_host::ControlEntry& entry : fe.controls) {
        if (entry.id == static_cast<custom_ui_host::CtrlId>(c)) {
            entry.text = text ? text : "";
            return 1;
        }
    }
    return 0;
}

__declspec(dllexport) int __cdecl CustomUI_GetEditText(CustomUI_WindowHandle h, CustomUI_CtrlId c, char* buf,
                                                       int bufLen) {
    if (!custom_ui_host::ReadyOrLog("GetEditText"))
        return 0;
    if (!buf || bufLen <= 0)
        return 0;
    auto* wnd = custom_ui_host::g_windows->Lookup(static_cast<unsigned int>(h));
    if (!wnd)
        return 0;
    auto& fe = wnd->Extras();
    void* edit = nullptr;
    for (custom_ui_host::ControlEntry& entry : fe.controls) {
        if (entry.id == static_cast<custom_ui_host::CtrlId>(c) && entry.kind == custom_ui_host::ControlKind::Edit) {
            edit = entry.raw;
            break;
        }
    }
    if (!edit)
        return 0;

    // CCtrlEdit::GetText returns ZXString<char> BY VALUE. Under MSVC __thiscall,
    // a class with a non-trivial copy ctor/dtor is returned via a hidden
    // return-slot pointer passed as the first stack argument (this in ECX, edx
    // in EDX). Mirror common/CWvsApp.cpp CWvsApp::GetCmdLine, the established
    // struct-return wrapper in this codebase. ZXString<char> is a 4-byte handle
    // (m_pStr); copy it out, then release via Empty().
    ZXString<char> out; // default ctor: m_pStr = nullptr
    reinterpret_cast<ZXString<char>*(__fastcall*)(void*, void*, ZXString<char>*)>(C_CTRL_EDIT_GET_TEXT)(edit, nullptr,
                                                                                                        &out);

    const char* src = out.m_pStr ? out.m_pStr : "";
    std::strncpy(buf, src, static_cast<std::size_t>(bufLen) - 1);
    buf[bufLen - 1] = '\0';
    int copied = static_cast<int>(std::strlen(buf));
    out.Empty(); // release the ZXString's backing buffer
    return copied;
}
__declspec(dllexport) CustomUI_HotkeyId __cdecl CustomUI_BindHotkey(unsigned int vk, unsigned int modifiers,
                                                                    CustomUI_WindowHandle target) {
    if (!custom_ui_host::ReadyOrLog("BindHotkey"))
        return 0;

    // Reject if the vanilla func-key map already binds this slot. v83.1
    // inlines FuncKeyMapped as a direct read of m_aFuncKeyMapped[vk]
    // (returned by value); byte 0 of the packed FUNCKEY_MAPPED is nType.
    auto* mgr = CFuncKeyMappedMan::GetInstance();
    if (mgr) {
        constexpr int kMax = 89; // m_aFuncKeyMapped[89] on GMS
        if (vk < static_cast<unsigned int>(kMax)) {
            FUNCKEY_MAPPED fkm = mgr->FuncKeyMapped(static_cast<int>(vk));
            unsigned char nType = *reinterpret_cast<unsigned char*>(&fkm);
            if (nType != 0) {
                Log("custom-ui-host: BindHotkey rejected -- vk=0x%02X already mapped (nType=%u)", vk, nType);
                return 0;
            }
        }
    }

    auto* target_wnd = custom_ui_host::g_windows->Lookup(static_cast<unsigned int>(target));
    if (!target_wnd) {
        Log("custom-ui-host: BindHotkey rejected -- unknown target window %d", target);
        return 0;
    }

    auto id = custom_ui_host::g_hotkeys->Bind(vk, modifiers, static_cast<unsigned int>(target));
    if (id == 0) {
        Log("custom-ui-host: BindHotkey rejected -- vk=0x%02X denied (denylist or already bound)", vk);
        return 0;
    }
    return static_cast<CustomUI_HotkeyId>(id);
}

__declspec(dllexport) int __cdecl CustomUI_UnbindHotkey(CustomUI_HotkeyId id) {
    if (!custom_ui_host::ReadyOrLog("UnbindHotkey"))
        return 0;
    return custom_ui_host::g_hotkeys->Unbind(static_cast<unsigned int>(id)) ? 1 : 0;
}
__declspec(dllexport) int __cdecl CustomUI_SendPacket(unsigned short opcode, const void* payload, unsigned int len) {
    if (!custom_ui_host::ReadyOrLog("SendPacket"))
        return 0;
    if (opcode < custom_ui_host::g_config.outbound_op_min || opcode > custom_ui_host::g_config.outbound_op_max) {
        Log("custom-ui-host: SendPacket rejected -- opcode 0x%04X outside outbound range", opcode);
        return 0;
    }
    COutPacket op(static_cast<INT>(opcode));
    if (payload && len)
        op.EncodeBuffer(payload, len);
    auto* sock = CClientSocket::GetInstance();
    if (!sock) {
        Log("custom-ui-host: SendPacket -- CClientSocket singleton null");
        return 0;
    }
    sock->SendPacket(&op);
    return 1;
}

__declspec(dllexport) CustomUI_HandlerId __cdecl CustomUI_RegisterPacketHandler(unsigned short opcode,
                                                                                CustomUI_PacketHandlerFn fn,
                                                                                void* user) {
    if (!custom_ui_host::ReadyOrLog("RegisterPacketHandler"))
        return 0;
    if (!fn)
        return 0;
    if (opcode < custom_ui_host::g_config.inbound_op_min || opcode > custom_ui_host::g_config.inbound_op_max) {
        Log("custom-ui-host: RegisterPacketHandler rejected -- opcode 0x%04X outside inbound range", opcode);
        return 0;
    }
    return static_cast<CustomUI_HandlerId>(custom_ui_host::g_packets->Register(opcode, fn, user));
}

__declspec(dllexport) int __cdecl CustomUI_UnregisterPacketHandler(CustomUI_HandlerId id) {
    if (!custom_ui_host::ReadyOrLog("UnregisterPacketHandler"))
        return 0;
    return custom_ui_host::g_packets->Unregister(static_cast<unsigned int>(id)) ? 1 : 0;
}
__declspec(dllexport) void __cdecl CustomUI_DumpRegistries(void) {
    if (custom_ui_host::g_windows) {
        Log("custom-ui-host: window registry size=%zu", custom_ui_host::g_windows->Size());
    }
    if (custom_ui_host::g_hotkeys) {
        Log("custom-ui-host: hotkey registry size=%zu", custom_ui_host::g_hotkeys->Size());
    }
    if (custom_ui_host::g_packets) {
        Log("custom-ui-host: packet registry size=%zu", custom_ui_host::g_packets->Size());
    }
}

} // extern "C"
