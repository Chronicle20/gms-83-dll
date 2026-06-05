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
    Log("custom-ui-host: InitLabelFont begin");

    // The game's COM helpers (PcCreateObject::IWzFont, IWzFont::Create) THROW a
    // _com_error on failure rather than returning a code, so each step is
    // wrapped to pinpoint which one fails and to keep the throw out of the
    // game's render loop. This runs once (guarded above).

    // Step A: resolve the COM class-hint. The factory's FIRST arg is NOT a face
    // name; the client pulls it from StringPool[SP_1410_CANVASFONT] (idx 0x582,
    // cf. sub_461CA8). GetStringW struct-returns a ZXString<ushort> (single
    // m_pStr) into our 4-byte slot; the pool string is persistent so we don't
    // release the ref.
    constexpr unsigned int kStringPoolCanvasFont = 0x582;
    const wchar_t* fontHint = nullptr;
    try {
        void* pool = reinterpret_cast<void*(__cdecl*)()>(C_STRING_POOL_GET_INSTANCE)();
        reinterpret_cast<void*(__fastcall*)(void*, void*, void*, unsigned int)>(C_STRING_POOL_GET_STRING_W)(
            pool, nullptr, &fontHint, kStringPoolCanvasFont);
    } catch (...) {
        Log("custom-ui-host: InitLabelFont: StringPool resolve THREW");
        return false;
    }
    Log("custom-ui-host: InitLabelFont: SP_1410 hint=[%ls] ptr=%p", fontHint ? fontHint : L"(null)",
        reinterpret_cast<const void*>(fontHint));
    if (!fontHint)
        return false;

    // Step B: allocate the IWzFont COM object from the hint.
    void* font = nullptr;
    try {
        reinterpret_cast<PcCreateIWzFontFn>(C_PC_CREATE_IWZFONT)(fontHint, &font, 0);
    } catch (...) {
        Log("custom-ui-host: InitLabelFont: PcCreateObject::IWzFont THREW (hint rejected)");
        return false;
    }
    Log("custom-ui-host: InitLabelFont: IWzFont created=%p", font);
    if (!font)
        return false;

    // Step C: configure face/size/color. "Arial" is the FACE (== StringPool
    // [SP_5527_ARIAL]); opaque black is the game's default label colour.
    BStr face;
    MakeBStr(&face, "Arial");
    Variant style;
    MakeVariantMissing(&style); // optional style omitted -> regular weight
    long hr = 0;
    try {
        hr = reinterpret_cast<IWzFontCreateFn>(C_IWZFONT_CREATE)(font, nullptr, face.m_Data, /*height*/ 12,
                                                                 /*argbColor*/ 0xFF000000u, &style);
    } catch (...) {
        Log("custom-ui-host: InitLabelFont: IWzFont::Create THREW");
        return false;
    }
    Log("custom-ui-host: InitLabelFont: Create hr=0x%08lX", static_cast<unsigned long>(hr));
    if (hr < 0)
        return false;

    g_label_font = font;
    Log("custom-ui-host: InitLabelFont ok");
    return true;
}

void DrawLabel(void* cuiwnd_self, int x, int y, const char* utf8) {
    if (!utf8)
        return;
    if (!g_label_font) {
        if (!InitLabelFont()) // font unavailable -> skip text this frame
            return;
    }

    // One-time step logging so a crash in the draw path is localized by the
    // last line printed (the per-call calling conventions are unverified).
    static bool s_dbg = true;
    const bool dbg = s_dbg;
    if (dbg)
        Log("custom-ui-host: DrawLabel: begin x=%d y=%d", x, y);

    // 1. window-layer canvas (owned _com_ptr_t<IWzCanvas>). The storage's first
    //    pointer is the raw IWzCanvas*.
    void* canvas_storage = nullptr;
    reinterpret_cast<GetCanvasFn>(C_WND_GET_CANVAS)(cuiwnd_self, nullptr, &canvas_storage);
    void* raw_canvas = *reinterpret_cast<void**>(&canvas_storage);
    if (dbg)
        Log("custom-ui-host: DrawLabel: canvas=%p", raw_canvas);
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
    if (dbg)
        Log("custom-ui-host: DrawLabel: pre DrawTextA text=%p font=%p", text.m_Data, g_label_font);
    reinterpret_cast<DrawTextAFn>(C_DRAW_TEXT_A)(raw_canvas, nullptr, x, y, text.m_Data, g_label_font, &vAlpha, &vTab);
    if (dbg)
        Log("custom-ui-host: DrawLabel: post DrawTextA");

    // 5. release the canvas ref (COM Release == vtable slot +8).
    void** canvas_vtbl = *reinterpret_cast<void***>(raw_canvas);
    auto release = reinterpret_cast<unsigned long(__fastcall*)(void*, void*)>(canvas_vtbl[2]);
    release(raw_canvas, nullptr);
    if (dbg) {
        Log("custom-ui-host: DrawLabel: done (released canvas)");
        s_dbg = false;
    }
}

} // namespace custom_ui_host
