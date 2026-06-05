#include "pch.h"

#include "runtime/text_draw.h"

#include "logger.h"
#include "memory_map.h"

#include <cstdio>
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

// --- Stage B: WorldMap 9-slice border ---------------------------------------
// Cached border-piece canvases (IWzCanvas*), one per WZ node. Index -> role:
// 0=top-left 1=top 2=top-right 3=left 4=right 5=bottom-left 6=bottom 7=bottom-right.
void* g_border[8] = {};
int g_border_w[8] = {};
int g_border_h[8] = {};
bool g_border_attempted = false;
int g_border_base = -1; // index into kBorderBases that resolved

// UOL base-path candidates for the border pieces (the WZ tree nesting was
// reported ambiguously). We try each on piece 0 and keep the one that loads.
const char* const kBorderBases[] = {
    "UI/UIWindow.img/WorldMap/Border",
    "UI/Basic.img/WorldMap/Border",
};

// Load a WZ canvas by UOL path. resman GetObjectA(uol) -> variant -> GetUnknown
// -> QI to IWzCanvas. Returns an AddRef'd IWzCanvas* (cached for the session) or
// null. COM-throwing, so the whole thing is guarded.
void* LoadUICanvas(const char* uolAnsi) {
    void* resman = *reinterpret_cast<void**>(C_RESMAN_INSTANCE_PTR);
    if (!resman)
        return nullptr;
    void* canvas = nullptr;
    try {
        BStr uol;
        MakeBStr(&uol, uolAnsi); // GetObjectA consumes (releases) this bstr
        Variant vParam;
        MakeVariantMissing(&vParam);
        Variant vAux;
        MakeVariantMissing(&vAux);
        Variant result;
        std::memset(&result, 0, sizeof(result));
        reinterpret_cast<void*(__fastcall*)(void*, void*, void*, void*, void*, void*)>(C_RESMAN_GET_OBJECT_A)(
            resman, nullptr, &result, uol.m_Data, &vParam, &vAux);
        void* unk =
            reinterpret_cast<void*(__fastcall*)(void*, void*, int, int)>(C_VARIANT_GET_UNKNOWN)(&result, nullptr, 0, 0);
        if (unk)
            reinterpret_cast<int(__fastcall*)(void*, void*, void*)>(C_QI_CANVAS)(&canvas, nullptr, &unk);
    } catch (...) {
        return nullptr;
    }
    return canvas;
}

int CanvasWidth(void* c) {
    return reinterpret_cast<int(__fastcall*)(void*, void*)>(C_CANVAS_GET_WIDTH)(c, nullptr);
}
int CanvasHeight(void* c) {
    return reinterpret_cast<int(__fastcall*)(void*, void*)>(C_CANVAS_GET_HEIGHT)(c, nullptr);
}

// Loads the 8 border pieces once (caching them). Returns true if at least the
// first piece resolved. Logs each piece's dimensions for validation.
bool InitBorder() {
    if (g_border_attempted)
        return g_border[0] != nullptr;
    g_border_attempted = true;

    char uol[160];
    for (int b = 0; b < 2 && g_border_base < 0; ++b) {
        std::snprintf(uol, sizeof(uol), "%s/0", kBorderBases[b]);
        void* c = LoadUICanvas(uol);
        if (c) {
            g_border[0] = c;
            g_border_base = b;
        }
    }
    if (g_border_base < 0) {
        Log("custom-ui-host: border: no WorldMap/Border base path resolved -- frame stays plain");
        return false;
    }
    for (int i = 1; i < 8; ++i) {
        std::snprintf(uol, sizeof(uol), "%s/%d", kBorderBases[g_border_base], i);
        g_border[i] = LoadUICanvas(uol);
    }
    for (int i = 0; i < 8; ++i) {
        if (g_border[i]) {
            try {
                g_border_w[i] = CanvasWidth(g_border[i]);
                g_border_h[i] = CanvasHeight(g_border[i]);
            } catch (...) {
            }
        }
        Log("custom-ui-host: border[%d] (%s/%d) canvas=%p %dx%d", i, kBorderBases[g_border_base], i, g_border[i],
            g_border_w[i], g_border_h[i]);
    }
    return true;
}

// Blit a loaded IWzCanvas image onto the destination canvas at (x,y), native
// size, fully opaque. The blit-image method is COM vtable slot 32 (+0x80),
// __stdcall(this, x, y, img, <VT_I4 255 alpha spread as 4 dwords>).
void BlitCanvas(void* dstCanvas, void** dstVtbl, int x, int y, void* img) {
    if (!img)
        return;
    Variant alpha;
    std::memset(&alpha, 0, sizeof(alpha));
    *reinterpret_cast<unsigned short*>(alpha.bytes) = 3;      // vt = VT_I4
    *reinterpret_cast<unsigned long*>(alpha.bytes + 8) = 255; // lVal = 255 (full alpha)
    const unsigned long* a = reinterpret_cast<const unsigned long*>(alpha.bytes);
    reinterpret_cast<long(__stdcall*)(void*, int, int, void*, unsigned long, unsigned long, unsigned long,
                                      unsigned long)>(dstVtbl[32])(dstCanvas, x, y, img, a[0], a[1], a[2], a[3]);
}

// Compose the WorldMap 9-slice border onto dstCanvas for a w x h dialog: four
// corners at the corners, the four edges tiled along each side. Pieces:
// 0=TL 1=top 2=TR 3=left 4=right 5=BL 6=bottom 7=BR.
void DrawBorder(void* dstCanvas, void** dstVtbl, int w, int h) {
    const int wTL = g_border_w[0], hTL = g_border_h[0];
    const int wTR = g_border_w[2], hTR = g_border_h[2];
    const int wBL = g_border_w[5], hBL = g_border_h[5];
    const int wBR = g_border_w[7], hBR = g_border_h[7];

    // Corners.
    BlitCanvas(dstCanvas, dstVtbl, 0, 0, g_border[0]);
    BlitCanvas(dstCanvas, dstVtbl, w - wTR, 0, g_border[2]);
    BlitCanvas(dstCanvas, dstVtbl, 0, h - hBL, g_border[5]);
    BlitCanvas(dstCanvas, dstVtbl, w - wBR, h - hBR, g_border[7]);

    // Top edge (piece 1) tiled horizontally between the top corners.
    if (g_border[1] && g_border_w[1] > 0)
        for (int x = wTL; x < w - wTR; x += g_border_w[1])
            BlitCanvas(dstCanvas, dstVtbl, x, 0, g_border[1]);
    // Bottom edge (piece 6) tiled along the bottom, aligned to the piece bottom.
    if (g_border[6] && g_border_w[6] > 0)
        for (int x = wBL; x < w - wBR; x += g_border_w[6])
            BlitCanvas(dstCanvas, dstVtbl, x, h - g_border_h[6], g_border[6]);
    // Left edge (piece 3) tiled vertically between the left corners.
    if (g_border[3] && g_border_h[3] > 0)
        for (int y = hTL; y < h - hBL; y += g_border_h[3])
            BlitCanvas(dstCanvas, dstVtbl, 0, y, g_border[3]);
    // Right edge (piece 4) tiled along the right, aligned to the piece right.
    if (g_border[4] && g_border_h[4] > 0)
        for (int y = hTR; y < h - hBR; y += g_border_h[4])
            BlitCanvas(dstCanvas, dstVtbl, w - g_border_w[4], y, g_border[4]);
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

    // Load the WorldMap border pieces once (cached).
    const bool haveBorder = InitBorder();

    void* canvas_storage = nullptr;
    reinterpret_cast<GetCanvasFn>(C_WND_GET_CANVAS)(cuiwnd_self, nullptr, &canvas_storage);
    void* canvas = *reinterpret_cast<void**>(&canvas_storage);
    if (!canvas)
        return;

    // IWzCanvas::FillRect = raw COM vtable slot 35 (+0x8C), __stdcall(this, x, y,
    // w, h, argb): confirmed in CUIToolTip::MakeLayer (`mov edx,[esi]; push args;
    // push canvas; call [edx+8Ch]`). Colour is ARGB -- alpha 0x00 is fully
    // TRANSPARENT (an earlier 0x00xxxxxx body drew nothing), so use 0xFF alpha.
    void** vtbl = *reinterpret_cast<void***>(canvas);
    auto fill = reinterpret_cast<long(__stdcall*)(void*, int, int, int, int, unsigned int)>(vtbl[35]);
    constexpr unsigned int kBodyColor = 0xFFECE6D8;   // opaque light parchment panel
    constexpr unsigned int kBorderColor = 0xFF2A2A2A; // fallback border (no WZ art)

    // Opaque body fill, then the WorldMap 9-slice border art on top. If the art
    // failed to load, fall back to a plain 1px FillRect border.
    fill(canvas, 0, 0, w, h, kBodyColor);
    if (haveBorder) {
        DrawBorder(canvas, vtbl, w, h);
    } else {
        fill(canvas, 0, 0, w, 1, kBorderColor);     // top
        fill(canvas, 0, h - 1, w, 1, kBorderColor); // bottom
        fill(canvas, 0, 0, 1, h, kBorderColor);     // left
        fill(canvas, w - 1, 0, 1, h, kBorderColor); // right
    }

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
