#include "pch.h"

#include "runtime/text_draw.h"

#include "logger.h"
#include "memory_map.h"

#include <cstring>

namespace custom_ui_host {

namespace {

// A Ztl_bstr_t / _bstr_t is a single-pointer struct: { _bstr_t::Data_t* m_Data; }.
// DrawTextA and IWzFont::Create take it BY VALUE (i.e. they receive m_Data as one
// pushed dword). We build it with the game's _bstr_t(const char*) ctor and pass
// the 4-byte storage's first (and only) pointer.
struct BStr {
    void* m_Data;
};

// A Ztl_variant_t is a 16-byte VARIANT. DrawTextA / IWzFont::Create take it BY
// REF (we pass &storage). We build it with the game's VT_I4 ctor.
struct Variant {
    unsigned char bytes[16];
};

// Cached raw IWzFont* for label rendering. Null until InitLabelFont succeeds.
void* g_label_font = nullptr;
// Set once we've tried (and failed) to build the font, so DrawLabel doesn't
// re-attempt every frame and spam the log with the same diagnostics.
bool g_font_init_attempted = false;

// __thiscall surrogate (ecx=this) for _bstr_t::_bstr_t(const char*).
using BStrCtorFn = void(__fastcall*)(void* /*this*/, void* /*edx*/, const char* ansi);

// __cdecl PcCreateObject::IWzFont(LPCWSTR hint, IWzFont** ppOut, int aggregate).
using PcCreateIWzFontFn = void(__cdecl*)(const wchar_t* hint, void** ppOut, int aggregate);

// __thiscall surrogate for IWzFont::Create(Ztl_bstr_t face, ulong height,
// ulong argbColor, const Ztl_variant_t& style). face is passed by value (its
// single m_Data pointer); style is passed by reference.
using IWzFontCreateFn = long(__fastcall*)(void* /*this*/, void* /*edx*/, void* faceData, unsigned long height,
                                          unsigned long argbColor, const void* style);

// __thiscall surrogate for CWnd::GetCanvas(_com_ptr_t<IWzCanvas>* pOut).
using GetCanvasFn = void*(__fastcall*)(void* /*this*/, void* /*edx*/, void** pOut);

// __thiscall surrogate for IWzCanvas::DrawTextA(LONG x, LONG y, Ztl_bstr_t text,
// IWzFont*, const Ztl_variant_t& vAlpha, const Ztl_variant_t& vTabOrg). text is
// by value (its m_Data); the two variants are by reference.
using DrawTextAFn = unsigned long(__fastcall*)(void* /*this*/, void* /*edx*/, long x, long y, void* textData,
                                               void* font, const void* vAlpha, const void* vTabOrg);

void MakeBStr(BStr* out, const char* ansi) {
    out->m_Data = nullptr;
    reinterpret_cast<BStrCtorFn>(C_BSTR_FROM_CSTR)(out, nullptr, ansi);
}

// The COM "optional argument omitted" variant: VT_ERROR (0x000A) with scode
// DISP_E_PARAMNOTFOUND (0x80020004). This is exactly the global `pvargSrc` the
// game copies for IWzFont::Create's style and IWzCanvas::DrawTextA's alpha +
// tab-origin args (cf. sub_4275F0 / sub_461CA8) -- i.e. "use defaults". Passing
// a real VT_I4 instead makes those COM methods reject the arg and throw a
// _com_error (this was why IWzFont::Create threw).
void MakeVariantMissing(Variant* out) {
    std::memset(out, 0, sizeof(*out));
    *reinterpret_cast<unsigned short*>(out->bytes) = 0x000A;         // vt = VT_ERROR
    *reinterpret_cast<unsigned long*>(out->bytes + 8) = 0x80020004u; // scode = DISP_E_PARAMNOTFOUND
}

} // namespace

bool InitLabelFont() {
    if (g_label_font)
        return true;
    if (g_font_init_attempted) // already tried and failed -- don't spam / re-throw every frame
        return false;
    g_font_init_attempted = true;

    // The game's COM helpers (PcCreateObject::IWzFont, IWzFont::Create) THROW a
    // _com_error on failure rather than returning a code, so each step is
    // wrapped both to keep the throw out of the game's render loop and to log
    // which step failed. This runs once (guarded above), so on failure it logs
    // exactly one line rather than spamming every frame.

    // Step A: resolve the COM class-hint. The factory's FIRST arg is NOT a face
    // name; the client pulls it from StringPool[SP_1410_CANVASFONT] (idx 0x582,
    // cf. sub_461CA8). GetStringW struct-returns a ZXString<ushort> (single
    // m_pStr) into our 4-byte slot; the pool string is persistent so we don't
    // release the ref.
    constexpr unsigned int kStringPoolCanvasFont = 0x582; // SP_1410_CANVASFONT (COM class-hint)
    constexpr unsigned int kStringPoolFontFace = 0x1597;  // SP_5527 (the game's UI font face)
    void* pool = nullptr;
    const wchar_t* fontHint = nullptr;
    try {
        pool = reinterpret_cast<void*(__cdecl*)()>(C_STRING_POOL_GET_INSTANCE)();
        reinterpret_cast<void*(__fastcall*)(void*, void*, void*, unsigned int)>(C_STRING_POOL_GET_STRING_W)(
            pool, nullptr, &fontHint, kStringPoolCanvasFont);
    } catch (...) {
        Log("custom-ui-host: InitLabelFont: StringPool resolve threw -- labels disabled");
        return false;
    }
    if (!fontHint) {
        Log("custom-ui-host: InitLabelFont: SP_1410 hint empty -- labels disabled");
        return false;
    }

    // Step B: allocate the IWzFont COM object from the hint.
    void* font = nullptr;
    try {
        reinterpret_cast<PcCreateIWzFontFn>(C_PC_CREATE_IWZFONT)(fontHint, &font, 0);
    } catch (...) {
        Log("custom-ui-host: InitLabelFont: PcCreateObject::IWzFont threw -- labels disabled");
        return false;
    }
    if (!font) {
        Log("custom-ui-host: InitLabelFont: IWzFont null -- labels disabled");
        return false;
    }

    // Step C: configure face/size/color. The FACE must be the game's own UI
    // font, resolved from StringPool[SP_5527] (idx 0x1597) -- the same string
    // get_basic_font / sub_461CA8 use. A literal "Arial" renders as the
    // NPC/chat face instead of the crisp UI face. GetBSTR struct-returns a
    // Ztl_bstr_t (single m_Data) which IWzFont::Create consumes (releases), so
    // we don't release it. We also resolve it as a wide string purely to log
    // the actual font name. Opaque black is the colour; the style/alpha/tab
    // variants must be the COM "omitted optional" variant
    // (VT_ERROR/DISP_E_PARAMNOTFOUND) -- a real VT_I4 makes Create throw.
    const wchar_t* faceName = nullptr;
    try {
        reinterpret_cast<void*(__fastcall*)(void*, void*, void*, unsigned int)>(C_STRING_POOL_GET_STRING_W)(
            pool, nullptr, &faceName, kStringPoolFontFace);
    } catch (...) {
    }
    Log("custom-ui-host: label font face=[%ls]", faceName ? faceName : L"(null)");
    BStr face;
    face.m_Data = nullptr;
    try {
        reinterpret_cast<void*(__fastcall*)(void*, void*, void*, unsigned int)>(C_STRING_POOL_GET_BSTR)(
            pool, nullptr, &face, kStringPoolFontFace);
    } catch (...) {
        Log("custom-ui-host: InitLabelFont: GetBSTR(face) threw -- labels disabled");
        return false;
    }
    Variant style;
    MakeVariantMissing(&style);
    long hr = 0;
    try {
        hr = reinterpret_cast<IWzFontCreateFn>(C_IWZFONT_CREATE)(font, nullptr, face.m_Data, /*height*/ 12,
                                                                 /*argbColor*/ 0xFF000000u, &style);
    } catch (...) {
        Log("custom-ui-host: InitLabelFont: IWzFont::Create threw -- labels disabled");
        return false;
    }
    if (hr < 0) {
        Log("custom-ui-host: InitLabelFont: IWzFont::Create hr=0x%08lX -- labels disabled",
            static_cast<unsigned long>(hr));
        return false;
    }

    g_label_font = font;
    Log("custom-ui-host: label font ready");
    return true;
}

void DrawFrame(void* cuiwnd_self, int w, int h) {
    if (w <= 0 || h <= 0)
        return;

    void* canvas_storage = nullptr;
    reinterpret_cast<GetCanvasFn>(C_WND_GET_CANVAS)(cuiwnd_self, nullptr, &canvas_storage);
    void* canvas = *reinterpret_cast<void**>(&canvas_storage);
    if (!canvas)
        return;

    // IWzCanvas::FillRect = raw COM vtable slot 35 (+0x8C), __stdcall(this, x, y,
    // w, h, argb): confirmed in CUIToolTip::MakeLayer (`mov edx,[esi]; push args;
    // push canvas; call [edx+8Ch]`). Colour is 24-bit RGB (high byte ignored);
    // the same call draws tooltip boxes. Body fill then 1px border lines.
    void** vtbl = *reinterpret_cast<void***>(canvas);
    auto fill = reinterpret_cast<long(__stdcall*)(void*, int, int, int, int, unsigned int)>(vtbl[35]);
    constexpr unsigned int kBodyColor = 0xE6E6E6;   // light grey panel
    constexpr unsigned int kBorderColor = 0x2A2A2A; // dark border
    fill(canvas, 0, 0, w, h, kBodyColor);
    fill(canvas, 0, 0, w, 1, kBorderColor);     // top
    fill(canvas, 0, h - 1, w, 1, kBorderColor); // bottom
    fill(canvas, 0, 0, 1, h, kBorderColor);     // left
    fill(canvas, w - 1, 0, 1, h, kBorderColor); // right

    // release the canvas ref (raw IUnknown::Release, slot 2, __stdcall -- see
    // DrawLabel step 5 for why the convention differs from the C++ wrappers).
    reinterpret_cast<unsigned long(__stdcall*)(void*)>(vtbl[2])(canvas);
}

void DrawLabel(void* cuiwnd_self, int x, int y, const char* utf8) {
    if (!utf8)
        return;
    if (!g_label_font) {
        if (!InitLabelFont()) // font unavailable -> skip text this frame
            return;
    }

    // 1. window-layer canvas (owned _com_ptr_t<IWzCanvas>). The storage's first
    //    pointer is the raw IWzCanvas*.
    void* canvas_storage = nullptr;
    reinterpret_cast<GetCanvasFn>(C_WND_GET_CANVAS)(cuiwnd_self, nullptr, &canvas_storage);
    void* raw_canvas = *reinterpret_cast<void**>(&canvas_storage);
    if (!raw_canvas)
        return;

    // 2. text bstr (DrawTextA consumes one ref -- do NOT release it).
    BStr text;
    MakeBStr(&text, utf8);

    // 3. alpha + tab-origin variants. The game (sub_4275F0) passes the
    //    "omitted optional" variant for both -> default opacity (opaque; RGB
    //    lives on the font) and default tab origin. A real VT_I4 here makes
    //    DrawTextA's inner COM call reject the arg and throw.
    Variant vAlpha;
    MakeVariantMissing(&vAlpha);
    Variant vTab;
    MakeVariantMissing(&vTab);

    // 4. draw.
    reinterpret_cast<DrawTextAFn>(C_DRAW_TEXT_A)(raw_canvas, nullptr, x, y, text.m_Data, g_label_font, &vAlpha, &vTab);

    // 5. release the canvas ref. GetCanvas returned an AddRef'd _com_ptr_t, so
    //    we owe one Release. This is a DIRECT call to the raw COM vtable slot 2
    //    (IUnknown::Release), which is __stdcall -- `this` is pushed on the
    //    stack and the callee cleans it (retn 4). Confirmed in CWnd::Draw:
    //    `mov ecx,[eax]; push eax; call [ecx+8]`. (GetCanvas/DrawTextA are
    //    __thiscall C++ *wrappers*, hence the convention difference.) Calling
    //    Release as __fastcall (this in ecx, 0 stack) left ESP off by 4 -> the
    //    /RTC1 "value of esp was not properly saved" crash.
    void** canvas_vtbl = *reinterpret_cast<void***>(raw_canvas);
    reinterpret_cast<unsigned long(__stdcall*)(void*)>(canvas_vtbl[2])(raw_canvas);
}

} // namespace custom_ui_host
