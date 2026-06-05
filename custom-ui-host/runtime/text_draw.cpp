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
    void *m_Data;
};

// A Ztl_variant_t is a 16-byte VARIANT. DrawTextA / IWzFont::Create take it BY
// REF (we pass &storage). We build it with the game's VT_I4 ctor.
struct Variant {
    unsigned char bytes[16];
};

// Cached raw IWzFont* for label rendering. Null until InitLabelFont succeeds.
void *g_label_font = nullptr;

// __thiscall surrogate (ecx=this) for _bstr_t::_bstr_t(const char*).
using BStrCtorFn = void(__fastcall *)(void * /*this*/, void * /*edx*/,
                                      const char *ansi);

// __thiscall surrogate for Ztl_variant_t::ctor_i4(int value, short vt).
using VariantCtorI4Fn = void(__fastcall *)(void * /*this*/, void * /*edx*/,
                                           int value, short vt);

// __cdecl PcCreateObject::IWzFont(LPCWSTR hint, IWzFont** ppOut, int aggregate).
using PcCreateIWzFontFn = void(__cdecl *)(const wchar_t *hint, void **ppOut,
                                          int aggregate);

// __thiscall surrogate for IWzFont::Create(Ztl_bstr_t face, ulong height,
// ulong argbColor, const Ztl_variant_t& style). face is passed by value (its
// single m_Data pointer); style is passed by reference.
using IWzFontCreateFn = long(__fastcall *)(void * /*this*/, void * /*edx*/,
                                           void *faceData, unsigned long height,
                                           unsigned long argbColor,
                                           const void *style);

// __thiscall surrogate for CWnd::GetCanvas(_com_ptr_t<IWzCanvas>* pOut).
using GetCanvasFn = void *(__fastcall *)(void * /*this*/, void * /*edx*/,
                                         void **pOut);

// __thiscall surrogate for IWzCanvas::DrawTextA(LONG x, LONG y, Ztl_bstr_t text,
// IWzFont*, const Ztl_variant_t& vAlpha, const Ztl_variant_t& vTabOrg). text is
// by value (its m_Data); the two variants are by reference.
using DrawTextAFn = unsigned long(__fastcall *)(
    void * /*this*/, void * /*edx*/, long x, long y, void *textData, void *font,
    const void *vAlpha, const void *vTabOrg);

void MakeBStr(BStr *out, const char *ansi) {
    out->m_Data = nullptr;
    reinterpret_cast<BStrCtorFn>(C_BSTR_FROM_CSTR)(out, nullptr, ansi);
}

void MakeVariantI4(Variant *out, int value) {
    std::memset(out, 0, sizeof(*out));
    reinterpret_cast<VariantCtorI4Fn>(C_VARIANT_CTOR_I4)(out, nullptr, value, 3);
}

}  // namespace

bool InitLabelFont() {
    if (g_label_font) return true;

    // Step A: allocate a blank IWzFont COM object.
    void *font = nullptr;
    reinterpret_cast<PcCreateIWzFontFn>(C_PC_CREATE_IWZFONT)(L"Arial", &font, 0);
    if (!font) {
        Log("custom-ui-host: PcCreateObject::IWzFont returned null -- label "
            "rendering disabled");
        return false;
    }

    // Step B: configure face/size/color. Empty style variant = regular weight.
    BStr face;
    MakeBStr(&face, "Arial");
    Variant style;
    MakeVariantI4(&style, 0);

    long hr = reinterpret_cast<IWzFontCreateFn>(C_IWZFONT_CREATE)(
        font, nullptr, face.m_Data, /*height*/ 12,
        /*argbColor*/ 0xFFFFFFFFu, &style);
    if (hr < 0) {
        Log("custom-ui-host: IWzFont::Create failed hr=0x%08lX -- label "
            "rendering disabled",
            static_cast<unsigned long>(hr));
        return false;
    }

    g_label_font = font;
    return true;
}

void DrawLabel(void *cuiwnd_self, int x, int y, const char *utf8) {
    if (!g_label_font || !utf8) return;

    // 1. window-layer canvas (owned _com_ptr_t<IWzCanvas>). The storage's first
    //    pointer is the raw IWzCanvas*.
    void *canvas_storage = nullptr;
    reinterpret_cast<GetCanvasFn>(C_WND_GET_CANVAS)(cuiwnd_self, nullptr,
                                                    &canvas_storage);
    void *raw_canvas = *reinterpret_cast<void **>(&canvas_storage);
    if (!raw_canvas) return;

    // 2. text bstr (DrawTextA consumes one ref -- do NOT release it).
    BStr text;
    MakeBStr(&text, utf8);

    // 3. opacity variant (VT_I4, alpha 255 = fully opaque; RGB lives on the
    //    font) and an empty tab-origin variant.
    Variant vAlpha;
    MakeVariantI4(&vAlpha, 255);
    Variant vTab;
    MakeVariantI4(&vTab, 0);

    // 4. draw.
    reinterpret_cast<DrawTextAFn>(C_DRAW_TEXT_A)(
        raw_canvas, nullptr, x, y, text.m_Data, g_label_font, &vAlpha, &vTab);

    // 5. release the canvas ref (COM Release == vtable slot +8).
    void **canvas_vtbl = *reinterpret_cast<void ***>(raw_canvas);
    auto release = reinterpret_cast<unsigned long(__fastcall *)(void *, void *)>(
        canvas_vtbl[2]);
    release(raw_canvas, nullptr);
}

}  // namespace custom_ui_host
