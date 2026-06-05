# v83.1 Custom-UI Controls: Buttons, Edits, Labels, and Cloned-Vtable Overrides

Target IDB confirmed via `list_instances`: `E:\Programs\Nexon\IDBs_v9\GMS\v83_Me\MapleStory_dump.exe.i64`,
binary `MapleStory_dump.exe`, base `0x400000`, active instance port 13337. All addresses below are
absolute (base 0x400000). Every claim is anchored to decompiler/disasm evidence.

> TL;DR for the framework:
> - **Button text does not exist.** `CCtrlButton` is image-only; its "caption" is a WZ UOL image
>   (Normal/Pressed/Disabled/MouseOver/KeyFocused frames). There is no SetText. To get a labeled
>   button you supply a button-image UOL, or you draw your own text on top via Draw.
> - **`CCtrlEdit::CreateCtrl` is the inherited `CCtrlWnd::CreateCtrl` (`0x004DFBFE`)** — vtable slot 2.
>   Initial text via `CCtrlEdit::SetText` (`0x004CC512`); read back via `CCtrlEdit::GetText`
>   (`0x00471353`) -> `ZXString<char>`.
> - **No label/static class exists.** No `CCtrlStatic`/`CCtrlText`. Labels must be drawn in `Draw`
>   (slot 11) using `IWzCanvas::DrawTextA` (`0x004277AD`).
> - **Clicks dispatch to the WINDOW's slot-8 `OnButtonClicked(UINT nControlId)`** — the single arg
>   is the control id you assigned in CreateCtrl, NOT the CCtrlButton*. One window-level override
>   catches all buttons. No per-button vtable clones needed. Confirmed by `CUIStat::OnButtonClicked`.

---

## Q1. CCtrlButton::CreateCtrl (`0x004BFFFB`)

### Exact signature / convention
Mangled name `?CreateCtrl@CCtrlButton@@UAEXPAVCWnd@@IJJJPAX@Z` decodes to:

```c
// __thiscall, virtual, returns void
void __thiscall CCtrlButton::CreateCtrl(
        CCtrlButton *this,
        CWnd  *pParent,      // PAVCWnd@@  parent window
        UINT   nId,          // I         control id (echoed back to OnButtonClicked)
        LONG   x,            // J
        LONG   y,            // J
        LONG   nDecClickArea,// J         -> this->m_nDecClickArea (click-area shrink; OnCreate passes 2)
        void  *pCreateParam);// PAX       CCtrlButton::CREATEPARAM*
```

The prior memmap guess of `(CWnd*, UINT, LONG, LONG, LONG, void*)` is **correct** (6 params after `this`).
The 5th LONG is the dec-click-area, not width/height. **Width/height are NOT passed here** — the button
sizes itself from the loaded image (see below).

### What it does (evidence, 0x4C000D..0x4C01E6)
1. Copies three leading bytes of the CREATEPARAM into the control:
   - `this->m_bAcceptFocus = param[0]`  (`/*0x4c0012*/`)
   - `this->m_bDrawBack    = param[1]`  (`/*0x4c0018*/`)
   - `this->m_bAnimateOnce = param[2]`  (`/*0x4c0020*/`)
2. `this->m_nDecClickArea = a6` (`/*0x4c002f*/`).
3. Reads the **button-image UOL** ZXString at `param+3` (byte offset 12) and calls
   `CCtrlButton::SetButtonImage(this, uol)` (`sub_41CF93(&v22, v8+3)` then `SetButtonImage`,
   `/*0x4c0065*//*0x4c006c*/`).
4. Iterates 4 `m_apPropButton[i]` IWzProperty arrays to compute max width/height of the image frames
   (`sub_40B920`/`sub_40B947` = width/height accessors, `/*0x4c0144*//*0x4c017a*/`), storing
   `v28`=maxW, `v29`=maxH.
5. Tail-calls the base `CCtrlWnd::CreateCtrl(this, pParent, nId, x, y, maxW, maxH, pCreateParam)`
   (`/*0x4c01e6*/`) — so the button's geometry width/height come from the **image**, not args.

### CREATEPARAM layout (`CCtrlButton::CREATEPARAM`)
From the field reads above and the cleaner caller `sub_4C1E0B` (an animated-button helper) which does
`sub_41468C(a7 + 3)` to fetch the UOL string (`/*0x4c1e91*/`):

| Offset | Type | Meaning |
|-------:|------|---------|
| +0  | BYTE  | m_bAcceptFocus |
| +1  | BYTE  | m_bDrawBack |
| +2  | BYTE  | m_bAnimateOnce |
| +3  | (pad) | — |
| +4 .. +11 | — | (bytes 4..11 unread by CreateCtrl; treat as zero) |
| +12 (dword[3]) | `ZXString<unsigned short>` (wide-char ptr) | **button-image UOL** (e.g. `UI/UIWindow.img/...`) |

`OnCreate` (the game's own close-button path, `/*0x92c4f6*//*0x92c518*/`) builds a 12-byte local
(`memset(v32,0,12)`), then puts the close-button UOL ZXString as the param's dword[3] (it actually
passes `v32` whose dword[3] was set up from the `v33`/`SP_..._BTCLOSE` string). The takeaway: a minimal
CREATEPARAM is **16 bytes**: three bools at 0/1/2, then a wide-string pointer at offset 12.

### How button TEXT/label is set
**There is none.** `SetButtonImage` (`0x004C01FF`) loads the image and pulls sub-images by string keys
`SP_1433_NORMAL`, `SP_1435_PRESSED`, `SP_1437_DISABLED`, `SP_1439_MOUSEOVER`, `SP_1441_KEYFOCUSED`
(`/*0x4c02f2*//*0x4c0363*//*0x4c03d4*//*0x4c0445*//*0x4c04be*/`) into `m_apPropButton[0..4]`. The button
is rendered purely from these images. A text caption must be (a) baked into the image UOL, or (b) drawn
by us on top in the window's `Draw`.

### Concrete call recipe (place a button)
```c
// 1. allocate + construct
CCtrlButton* btn = (CCtrlButton*)operator_new(0x5A4);   // size from OnCreate alloc /*0x92c4c9*/
CCtrlButton__ctor(btn);                                 // 0x004258E4 (nullary)

// 2. build CREATEPARAM (>=16 bytes, zeroed)
struct { BYTE acceptFocus, drawBack, animOnce, _pad; DWORD _r[2]; const wchar_t* imageUOL; } cp = {0};
cp.acceptFocus = 1;
cp.imageUOL    = L"UI/UIWindow.img/...";   // a button image node in the WZ

// 3. create on parent CUIWnd  (vtable slot 8 == byte +32 of the button vtable)
((CreateCtrl_t)btn->vtbl[8])(btn, parentCUIWnd, /*nId*/ 1234, x, y, /*nDecClickArea*/ 0, &cp);

// 4. register with the window's child list so Draw/hit-test see it:
//    CWnd::RegisterCtrl-equivalent is sub_4284FF(&parent[108 bytes], btn) — see OnCreate /*0x92c4f6*/
```
Note: the call MUST go through the button vtable slot (virtual), exactly as OnCreate does
(`(*(**(this+112)+32))(...)`, `/*0x92c518*/`). `(vtbl+32)/4 = slot 8` is the CreateCtrl slot in
`CCtrlButton`'s vtable (its vtable starts at the address stored at `*btn` after the ctor).

---

## Q2. CCtrlEdit::CreateCtrl, SetText, GetText

### CreateCtrl = inherited CCtrlWnd::CreateCtrl (`0x004DFBFE`), vtable slot 2
There is **no** `CCtrlEdit::CreateCtrl` — only two CreateCtrl functions exist in the binary
(`CCtrlButton::CreateCtrl` 0x4BFFFB, `CCtrlWnd::CreateCtrl` 0x4DFBFE). Reading the `CCtrlEdit` vtable
`off_AF2BA8` (bytes via get_bytes) the dword at **slot 2 = `0x004DFBFE`** = `CCtrlWnd::CreateCtrl`.
So CCtrlEdit uses the base unchanged.

Mangled `?CreateCtrl@CCtrlWnd@@UAEXPAVCWnd@@IJJJJPAX@Z`:
```c
void __thiscall CCtrlWnd::CreateCtrl(
        CCtrlWnd *this,
        CWnd *pParent,   // PAVCWnd@@
        UINT  nId,       // I
        LONG  x,         // J
        LONG  y,         // J
        LONG  w,         // J   <-- width IS a parameter here (unlike CCtrlButton)
        LONG  h,         // J   <-- height
        void *pParam);   // PAX
```
(`CCtrlWnd::CreateCtrl` decompile: `this[5]=a2`(parent), creates an IWzVector2D and a child layer at
(a4=x, a5=y) sized by (a6=w, a7=h) via the canvas/layer create calls `/*0x4dfd40*/`; stores
`this[7]=w, this[8]=h`, then calls virtual slot 4 `this->vtbl[4](this, a8)` `/*0x4dfd9b*/`.)

### CCtrlEdit::CREATEPARAM layout (ctor `0x004C8D5F`)
The edit param is a richer struct (built by `CCtrlEdit::CREATEPARAM::CREATEPARAM`):

| Offset (dword idx) | Init value | Meaning (inferred) |
|---:|---|---|
| +0  | `ZXString<char>` (= WindowName "") | initial text buffer |
| +4 (idx1) | 0 | flag |
| +8 (idx2) | -2 | flag/style |
| +12 (idx3)| 0 | |
| +16 (idx4)| 0 | |
| +20 (idx5)| 0 | (m_sFontName ptr; set to SP_5527_ARIAL bstr) `/*0x4c8db7*/` |
| +24 (idx6)| 12 | **font size = 12** `/*0x4c8dde*/` |
| +28 (idx7)| 0xFF000000 | **text color (ARGB, opaque black)** `/*0x4c8de5*/` |
| +32..+52 (idx8..13) | 0 | reserved |

So a default CREATEPARAM gives a 12pt Arial, black, empty edit. You typically construct it, optionally
tweak font/size/color, then call CreateCtrl(parent, id, x, y, w, h, &param).

### Place an edit box at (x,y,w,h) with initial text
```c
CCtrlEdit* ed = (CCtrlEdit*)operator_new(sizeofCCtrlEdit);
CCtrlEdit__ctor(ed);                              // 0x004C9C72 (nullary)

CCtrlEdit::CREATEPARAM cp;
CCtrlEdit__CREATEPARAM__ctor(&cp);                // 0x004C8D5F  (defaults: Arial 12, black)
// (optional) set cp text/color/size here

((CreateCtrl_t)ed->vtbl[2])(ed, parentCUIWnd, /*nId*/ 2000, x, y, w, h, &cp);  // slot 2 == 0x4DFBFE
CCtrlEdit__SetText(ed, "initial");                // 0x004CC512  (ANSI char*)
// register with parent child list (sub_4284FF as for buttons)
```

### Read the current text back (for CustomUI_GetEditText)
```c
ZXString<char> out;                               // 4-byte handle (ptr to ref-counted buffer)
CCtrlEdit__GetText(ed, &out);                     // 0x00471353  -> out
const char* s = out.m_pStr;                       // backing buffer; copy out, then release ZXString
```
`CCtrlEdit::GetText` (`0x00471353`) simply does `*out = 0; ZXString<char>::operator=(out, this+13)`
(`/*0x471368*/`) — the live text lives at dword index 13 (byte offset 52) inside CCtrlEdit, and SetText
writes the same field (`v4 = this + 13`, `/*0x4cc525*/`). So `CustomUI_GetEditText` = call GetText into
a local `ZXString<char>`, read `.m_pStr`, copy, release.

---

## Q3. Labels — NO dedicated class; draw manually

Searched functions for `CCtrlStatic`, `CCtrlText` -> **zero results**. No static/label control class
exists in v83.1. The only "control" classes are `CCtrlWnd` (base), `CCtrlButton`, `CCtrlEdit` (plus
list/scroll variants). **Labels must be rendered manually** by overriding the window's `Draw`
(slot 11) and calling a text primitive.

### Text-draw primitive: `IWzCanvas::DrawTextA` (`0x004277AD`)
Mangled `?DrawTextA@IWzCanvas@@QAEKJJVZtl_bstr_t@@PAUIWzFont@@ABVZtl_variant_t@@2@Z`:
```c
DWORD __thiscall IWzCanvas::DrawTextA(
        IWzCanvas *pCanvas,
        LONG nLeft,                 // x
        LONG nTop,                  // y
        Ztl_bstr_t sText,           // text (wide internally: pulls sText->m_wstr /*0x4277c7*/)
        IWzFont  *pFont,            // font COM object
        Ztl_variant_t vAlpha,       // 16-byte VARIANT: color/alpha
        Ztl_variant_t vTabOrg);     // 16-byte VARIANT: tab origin (pass empty)
```
Internally it forwards to canvas vtable slot at byte +152 (`(*(*this+152))(...)`, `/*0x4277f6*/`) with
the wide string, font, and the two variants expanded to 4 dwords each. Errors -> `_com_issue_errorex`.

To get the window's canvas inside our Draw override, use `CWnd::GetCanvas` (`0x00425C4C`) — exactly as
`CWnd::Draw` itself does (`/*0x9e0552*/`). For a font, the game obtains an `IWzFont` from the graphics
engine (same way CCtrlEdit's CREATEPARAM stores `SP_5527_ARIAL`/size/color). For the framework, the
simplest robust path: in our Draw override, get the canvas, build a `Ztl_bstr_t` from the label text and
a color variant, and call `DrawTextA`. (If acquiring an `IWzFont` proves fiddly, the alternative is to
let buttons/edits carry the visible text and keep static labels minimal — see decision below.)

Related higher-level helpers if useful: `DrawTextSepartedLine` (`0x008E40E4`) for multi-line wrapped text.

---

## Q4. OnCreate slot 13 (`0x0092C2E8`)

### Signature / convention
```c
int __thiscall CUIWnd::OnCreate(CUIWnd *this, int _unused, const wchar_t *pBackgrndArg);
```
(Decompiler shows `(int this, int _48, wchar_t *a3)`; `__thiscall`, returns int — actually returns the
result of `ZXString::_Release(&a3)` `/*0x92c533*/`, value irrelevant. It is a virtual called during
window creation.)

### What it does (this is exactly what we want to suppress)
1. If `*(this+1432) != 0` (a "has background" flag), it builds a background UOL string and either:
   - formats `SP_2167_UI_UIWINDOWIMG_S_BACKGRND` with a number (`/*0x92c34d*/`), or uses the passed
     `a3` arg, then for `m_nUIType==11` overrides with `SP_3742_UI_GUILDBBSIMG_GUILDBBS_BACKGRND`
     (`/*0x92c39b*/`), then calls `sub_92C708(this)` which **loads/sets the background**
     (`/*0x92c3ba*/`).
2. If `*(this+1420) != 0` (a "close-button type" selector), it selects one of several close-button
   image UOLs (`SP_1821_BTUICLOSE`, `SP_1549_BTUICLOSE2`, `SP_5577_BASICIMG_BTCLOSE`,
   `SP_2168_S_BTCLOSE` depending on the type, `/*0x92c4a4*/.../*0x92c428*/`), allocates a
   `CCtrlButton` (`Alloc 0x5A4` + `CCtrlButton::CCtrlButton`, `/*0x92c4c9*//*0x92c4e5*/`), registers it
   (`sub_4284FF`, `/*0x92c4f6*/`), and calls the button's `CreateCtrl` with **id 1000** at
   `(this+1424, this+1428)` (`/*0x92c518*/`). Id **1000** is exactly what slot-8
   `OnButtonClicked` checks to close the window (`if (a2==1000) UI_Close`, see Q5).

So OnCreate's side effects are: (a) load WZ background, (b) create a close button with id 1000. **Both
are things our custom window wants to suppress.**

### Is a no-op override safe?
**Yes.** Evidence:
- The background only loads when `*(this+1432)` is set and close button only when `*(this+1420)` is set.
  Both are member flags configured at construction. Our pure-int ctor path (`0x0092C0BF`, per the
  construction note) does not require these.
- `OnCreate` does **no** child-list/layer setup beyond the background+close-button. The window's layer
  and geometry are established by `CWnd::CreateWnd` (`0x009DE4D2`), independent of OnCreate. Nothing in
  CreateWnd reads outputs that only OnCreate produces; OnCreate is a post-creation hook for visuals.
- A no-op leaves `this+1452` (the background UOL ZXString) at its ctor value (0/empty), which is exactly
  the state CWnd::Draw checks for (`m_pBackgrnd.m_pInterface == NULL` -> draws nothing, see Q6).

**Recommendation: override slot 13 with a no-op** that returns 0. Exact override signature:
```c
int __fastcall CustomUI_OnCreate(void* this, void* /*edx*/, int unused, const wchar_t* pArg) { return 0; }
// (__thiscall in source terms; under MSVC thiscall the hidden 'this' is ecx, no edx use)
```
If you want to be maximally conservative and still allow a programmatic background later, the override
can be a no-op now and add explicit `CWnd::SetBackgrnd` calls only when desired. No minimal mandatory
work is required for a blank custom window.

---

## Q5. OnButtonClicked slot 8 (`0x0092C5AE`)

### Signature / convention
Mangled `?OnButtonClicked@CUIWnd@@UAEXI@Z`:
```c
void __thiscall CUIWnd::OnButtonClicked(CUIWnd *this, UINT nControlId);   // virtual, __thiscall
```
Base impl: `if (nControlId == 1000) CWvsContext::UI_Close(ctx, this[354]);` (`/*0x92c5b6*//*0x92c5c4*/`).
i.e. the default only handles the close button (id 1000).

### How a click reaches it / what identifies the button
**The argument is the control id (UINT)** that you assigned in `CreateCtrl` — NOT the CCtrlButton
pointer. Proof: every derived override has signature `...@@UAEXI@Z` and `switch`es on the id. Concrete
example `CUIStat::OnButtonClicked` (`0x008C522F`):
```c
switch (a2) {
  case 0x7D0: ... SendAbilityUpRequest(2048); break;   // id 2000
  case 0x7D6: ToggleDetail(); break;                   // id 2006
  ...
  default: CUIWnd::OnButtonClicked(this, a2);           // forward unknown ids to base /*0x8c5291*/
}
```
The button's press handling (in `CCtrlButton`) calls back into the **parent window's** virtual
`OnButtonClicked(nId)` using the id stored when the control was created. The 20+ xrefs to 0x0092C5AE are
all derived-class overrides forwarding their default case to the base — confirming this is the standard
window-level dispatch.

### Confirmation for the framework
**Overriding slot 8 in the cloned WINDOW vtable catches clicks for ALL buttons on that window.** You do
**not** need per-instance `CCtrlButton` vtable clones. Map the click to your ControlEntry **by control
id** (the UINT you passed as `nId` to CreateCtrl). Recommended scheme: assign each control a unique id at
add-time, store id->callback in the window's control table, and in the override look up the id.

Exact override signature:
```c
void __fastcall CustomUI_OnButtonClicked(CustomWnd* this, void* /*edx*/, UINT nControlId)
{
    // look up nControlId in this->controls[]; invoke its callback.
    // forward unhandled ids: ((OnButtonClicked_t)0x0092C5AE)(this, nControlId);  // base close handling
}
```
(Forwarding to base is optional — only needed if you also use id 1000 for a close button.)

---

## Q6. Draw slot 11 (`0x009E0502`)

### Signature / convention
Mangled `?Draw@CWnd@@...` -> `CWnd::Draw(CWnd *this, const tagRECT *pClip)`, `__thiscall`, returns
void*/ignored. (Decompile: `void* __thiscall CWnd::Draw(CWnd* this, const tagRECT* a2)`.)

### Does CWnd::Draw render child controls?
**No.** `CWnd::Draw` only draws **the window's own background**: if `m_pBackgrnd.m_pInterface` is set it
blits the background layer at `(m_nBackgrndX, m_nBackgrndY)` via canvas slot +128 (`/*0x9e0587*/`); else
if `m_pOverlabLayer` is set it draws that; otherwise it does nothing. It does **not** iterate child
controls.

However, child controls (buttons/edits created via CreateCtrl) are drawn through their **own layers** —
`CCtrlWnd::CreateCtrl` creates each control as a child IWzGr2D layer parented under the window's layer
(`PcCreateObject::IWzVector2D` + layer create at `/*0x4dfd40*/`). The graphics engine composites those
child layers automatically; the controls render themselves through the layer tree, not through the
parent's `Draw`. So:

**Buttons and edits show up WITHOUT us overriding Draw** (they have their own layers). We only need to
override `Draw` (slot 11) when we want to paint **custom label text** on the window surface (Q3). When we
do override it, we should still draw any intended background ourselves (or call the base) — but for a
transparent custom window with manually drawn labels, the override just gets the canvas
(`CWnd::GetCanvas` 0x00425C4C) and calls `DrawTextA` (0x004277AD) per label.

Override signature (only if drawing labels):
```c
void __fastcall CustomUI_Draw(CustomWnd* this, void* /*edx*/, const tagRECT* pClip)
{
    // optionally: ((Draw_t)0x009E0502)(this, pClip);   // base bg (no-op if no bg set)
    // for each label: GetCanvas -> DrawTextA(canvas, x, y, bstr(text), font, colorVariant, emptyVariant)
}
```

---

## (a) Control-creation recipe (pseudocode)

```c
// ---- add a button (image-only; text must be baked in the image or drawn in Draw) ----
ControlEntry* CustomUI_AddButton(CustomWnd* w, UINT id, int x, int y, const wchar_t* imageUOL) {
    CCtrlButton* b = new(0x5A4) CCtrlButton;  ctor 0x004258E4;
    struct ButtonParam { BYTE acceptFocus, drawBack, animOnce, pad; DWORD r0, r1;
                         const wchar_t* uol; } p = {1,0,0,0,0,0, imageUOL};   // 16 bytes
    ((CreateCtrl_t)b->vtbl[8])(b, (CWnd*)w, id, x, y, /*decClickArea*/0, &p);
    register_child(w, b);                      // sub_4284FF on w's child list (offset +108)
    return store_entry(w, id, b, /*kind*/BUTTON);
}

// ---- add an edit ----
ControlEntry* CustomUI_AddEdit(CustomWnd* w, UINT id, int x,int y,int cw,int ch, const char* initText) {
    CCtrlEdit* e = new CCtrlEdit;  ctor 0x004C9C72;
    CCtrlEdit::CREATEPARAM cp;  ctor 0x004C8D5F;          // Arial 12, black, empty
    ((CreateCtrl_t)e->vtbl[2])(e, (CWnd*)w, id, x, y, cw, ch, &cp);   // slot2 == 0x004DFBFE
    if (initText) CCtrlEdit::SetText(e, initText);        // 0x004CC512
    register_child(w, e);
    return store_entry(w, id, e, EDIT);
}

// ---- read edit text ----
int CustomUI_GetEditText(CCtrlEdit* e, char* dst, int cap) {
    ZXString<char> tmp = {0};
    CCtrlEdit::GetText(e, &tmp);                          // 0x00471353
    strncpy(dst, tmp.m_pStr ? tmp.m_pStr : "", cap);
    ZXString<char>::_Release(&tmp);
    return strlen(dst);
}
```
`CreateCtrl_t` = `void(__thiscall*)(void* thisCtrl, CWnd* parent, UINT id, LONG x, LONG y, LONG a, LONG b, void* param)`
— note CCtrlButton's slot-8 variant has 6 args (no width/height), CCtrlWnd/Edit's slot-2 variant has 7.

## (b) Label strategy decision

**No native label control exists -> draw labels manually.** Override the cloned window's `Draw`
(slot 11) and call `IWzCanvas::DrawTextA` (`0x004277AD`) once per label, using the window canvas from
`CWnd::GetCanvas` (`0x00425C4C`). Keep a per-window list of (text, x, y, color, font) label entries.
The one open dependency is acquiring an `IWzFont`; mirror how `CCtrlEdit::CREATEPARAM` resolves
`SP_5527_ARIAL` + size + ARGB color, or reuse an existing font object obtained from the graphics engine.
(If font acquisition is undesirable, fall back to putting visible text on image-buttons / read-only edits.)

## (c) Cloned-vtable override set

| Slot | Addr (base) | Override? | Override signature |
|----:|---|---|---|
| 8  | 0x0092C5AE (OnButtonClicked) | **YES, always** | `void __thiscall(CustomWnd*, UINT nControlId)` — dispatch by id to your callbacks |
| 11 | 0x009E0502 (Draw) | **Only if drawing labels** | `void __thiscall(CustomWnd*, const tagRECT* pClip)` — GetCanvas + DrawTextA per label |
| 13 | 0x0092C2E8 (OnCreate) | **YES, always** | `int __thiscall(CustomWnd*, int unused, const wchar_t* arg) { return 0; }` (no-op: suppress WZ bg + close button) |

Buttons/edits draw via their own child layers, so slot 11 does NOT need overriding to show controls —
only for custom label text. No per-control vtable clones are required (slot 8 dispatch is id-based).

## (d) NEW addresses for the memory map (with suggested symbol names)

| Address | Suggested symbol | Notes |
|---|---|---|
| 0x004DFBFE | `CCtrlWnd::CreateCtrl` | `?CreateCtrl@CCtrlWnd@@UAEXPAVCWnd@@IJJJJPAX@Z`; base CreateCtrl; **also CCtrlEdit's CreateCtrl** (vtable slot 2). 7 args: parent,id,x,y,w,h,param |
| 0x004C01FF | `CCtrlButton::SetButtonImage` | loads Normal/Pressed/Disabled/MouseOver/KeyFocused frames from a UOL |
| 0x004C9C72 | `CCtrlEdit::CCtrlEdit` | nullary ctor (already known per prompt) |
| 0x004C8D5F | `CCtrlEdit::CREATEPARAM::CREATEPARAM` | default-init edit param (Arial 12, ARGB 0xFF000000) |
| 0x00471B56 | `CCtrlEdit::CREATEPARAM::~CREATEPARAM` | param dtor |
| 0x004CC512 | `CCtrlEdit::SetText` | `?SetText@CCtrlEdit@@QAEXPBD@Z`; ANSI `char*`; text stored at edit dword[13] |
| 0x00471353 | `CCtrlEdit::GetText` | `?GetText@CCtrlEdit@@QAE?AV?$ZXString@D@@XZ`; returns `ZXString<char>` (for CustomUI_GetEditText) |
| 0x004277AD | `IWzCanvas::DrawTextA` | label text primitive: (x,y,Ztl_bstr_t,IWzFont*,vColor,vTabOrg) |
| 0x00425C4C | `CWnd::GetCanvas` | obtain window canvas inside Draw override |
| 0x008E40E4 | `DrawTextSepartedLine` (free fn) | optional multi-line/wrapped text helper |
| 0xAF2BA8 | `CCtrlEdit::`vftable` | vtable base; slot 2 = CreateCtrl (=0x4DFBFE) |
| 0x0092C708 | `CUIWnd::LoadBackgrnd` (guess) `sub_92C708` | background loader invoked by OnCreate (suppressed by our no-op) |
| 0x004284FF | `CWnd::RegisterCtrl` (guess) `sub_4284FF` | adds a control to the window's child list (offset +108); used by OnCreate when creating the close button |

### Confirmed prior-map facts (unchanged)
- C_UI_WND_VFTABLE @ 0xB3CE10: slot 8 = 0x0092C5AE (OnButtonClicked), slot 11 = 0x009E0502 (Draw),
  slot 13 = 0x0092C2E8 (OnCreate) — all re-verified by reading the vtable bytes.
- CCtrlButton nullary ctor 0x004258E4; CCtrlButton::CreateCtrl 0x004BFFFB (slot 8 of button vtable).

---

## Text drawing recipe (font + DrawTextA)

This resolves the one open dependency from Q3/(b): how to acquire an `IWzFont*` and how to marshal the
`DrawTextA` arguments. All anchored to two concrete in-game text-drawing sites that mirror exactly what
our `Draw` override must do: `CCtrlCheckBox::Draw` (`0x004C1A83`) and `CUIToolTip::DrawTextCenter`
(`0x008F466D`), plus the font factory in `CUIToolTip::CUIToolTip` (`0x008E49B5`).

### 1. `IWzCanvas::DrawTextA` (`0x004277AD`) — exact signature + marshaling

Mangled `?DrawTextA@IWzCanvas@@QAEKJJVZtl_bstr_t@@PAUIWzFont@@ABVZtl_variant_t@@2@Z`:

```c
// __thiscall, returns HRESULT (KJ.. => unsigned long / HRESULT)
DWORD __thiscall IWzCanvas::DrawTextA(
        IWzCanvas      *pCanvas,   // ecx (raw interface, NOT the _com_ptr_t wrapper)
        LONG            nLeft,      // x, window/layer-relative (see coord note)
        LONG            nTop,       // y
        Ztl_bstr_t      sText,      // BY VALUE: a 4-byte struct { _bstr_t::Data_t* m_Data; }
        IWzFont        *pFont,      // raw IWzFont* (NOT the _com_ptr_t)
        const Ztl_variant_t &vAlpha,// BY REF: 16-byte VARIANT — opacity/color (see below)
        const Ztl_variant_t &vTabOrg);// BY REF: 16-byte VARIANT — tab origin (pass a default/0 variant)
```

Decompile evidence (`0x004277AD`): it reads `sText->m_wstr` (`/*0x4277c7*/`), then forwards to canvas
vtable **byte +152** with `(this, x, y, wstr, pFont, *vAlpha[0..3], *vTabOrg[0..3], &vTabOrg)`
(`/*0x4277f6*/`), checks HRESULT via `_com_issue_errorex`, and **Releases the bstr Data_t on return**
(`/*0x427816*/`) — so DrawTextA consumes one reference of the bstr you pass (matches the callers, which
build a fresh `_bstr_t` per call).

**Marshaling the text arg (`Ztl_bstr_t` from `const char*`):**
`Ztl_bstr_t`/`_bstr_t` is a single-pointer struct (`_bstr_t::Data_t* m_Data`). Build it with the
`_bstr_t(const char*)` ctor `??0_bstr_t@@QAE@PBD@Z` at **`0x00425ADD`** (`__thiscall`, ecx=&dest,
arg=ANSI char*; allocates the Data_t and converts to wide). Pass the resulting 4-byte struct by value
(i.e. push its `m_Data`). DrawTextA Releases it internally, so do **not** Release it again yourself — the
callers (`CCtrlCheckBox::Draw`, `DrawTextCenter`) construct a `_bstr_t` immediately before the call and
never release it afterward. Evidence: checkbox `_bstr_t::_bstr_t(&v22, v17); ... DrawTextA(..., v22.m_Data, ...)`
(`/*0x4c1c04*//*0x4c1c1f*/`).

**Marshaling the color/opacity arg (`vAlpha`):**
This is a `Ztl_variant_t` (a 16-byte `VARIANT`), **not** a packed ARGB int. The RGB text color lives on
the **font** (`IWzFont::Create`'s `uColor`, see §2). The `vAlpha` variant is a 0–255 **opacity**:
- `CCtrlCheckBox::Draw` builds it with `sub_402FAB(&pvarg, 255, 3)` (`/*0x4c1aee*/`) = VT_I4 (vt=3),
  value 255 (fully opaque).
- Build it with the `Ztl_variant_t` int ctor **`sub_402FAB`** at **`0x00402FAB`**:
  `_DWORD* __thiscall Ztl_variant_t::ctor_i4(VARIANT* this, int value, short vt)` — call with
  `(&v, alpha0to255, 3)`. (vt must be 3=VT_I4, 10=VT_ERROR, or 11=VT_BOOL; anything else throws.)

**Marshaling `vTabOrg`:** pass a default-initialized `Ztl_variant_t` (VT_EMPTY / zeroed 16 bytes). The
callers use a `VariantInit`'d / `ZComVariantCopy`'d empty variant; `sub_402FAB(&v, 0, 3)` (VT_I4 0) also
works. It is the tab-stop origin; 0 disables tabbing.

> So: **color = the font's `uColor` (ARGB); the DrawTextA color arg is only the alpha/opacity (0-255)
> as a VT_I4 variant.** To draw "ARGB 0xAARRGGBB" text: bake RRGGBB into the font's `uColor` and pass
> AA (or 255) as the opacity variant. The font's `uColor` itself is a full 0xAARRGGBB (the engine uses
> 0xFF000000 for default black, 0xFFFFFFFF for white — see §2).

### 2. `IWzFont` acquisition — factory call recipe

The CCtrlEdit CREATEPARAM ctor (`0x004C8D5F`) does **not** create an `IWzFont`; it only stores the face
name bstr (`SP_5527_ARIAL`), size (12), and color (`0xFF000000`) as plain data — the actual font object
is built later. The real font construction is a **two-step** pattern, proven in `CUIToolTip::CUIToolTip`
(`0x008E49B5`, repeated ~20× to fill its font cache):

```c
// Step A: allocate a blank IWzFont COM object into a _com_ptr_t<IWzFont> slot.
//   PcCreateObject::IWzFont(LPCWSTR /*ignored class hint*/, IWzFont** ppOut, int aggregate=0)
//   addr 0x00463670 ; internally PCOMCreateObject(hint, &IID_IWzFont(=dword_BD83C0), ppOut, 0).
IWzFont* pFont = NULL;
PcCreateObject_IWzFont(faceNameW, &pFont, 0);          // 0x00463670  (cdecl: push hint, &pFont, 0)

// Step B: configure it. IWzFont::Create sets face/height/color/style in one call.
//   ?Create@IWzFont@@QAEJVZtl_bstr_t@@KKABVZtl_variant_t@@@Z  addr 0x0046341A
//   HRESULT __thiscall IWzFont::Create(IWzFont* this,
//                          Ztl_bstr_t sName,             // face, e.g. "Arial"  (by value, bstr)
//                          unsigned long uHeight,        // point size, e.g. 12
//                          unsigned long uColor,         // ARGB 0xAARRGGBB  (0xFFFFFFFF / 0xFF000000)
//                          const Ztl_variant_t& sStyle); // style variant (bold/italic flags packed)
pFont->Create(bstr("Arial"), 12, 0xFF000000, styleVariant);   // 0x0046341A
```

Evidence (`CUIToolTip` ctor disasm): `PcCreateObject::IWzFont` is called per slot with a `GetStringW`
face name (`/*0x8e4b1b*/` etc.), then for the styled font it pushes `sStyle, uColor=0xFFFFFFFF,
uHeight=0x0C, sName` and calls `IWzFont::Create` (`/*0x8e4c8d*//*0x8e4cbe*/`). `sub_4284A8` (`0x004284A8`)
just dereferences the `_com_ptr_t` to get the raw `IWzFont*` `this` for the Create call.

**Style variant (`sStyle`):** built by `sub_4626B3` (`0x004626B3`) which makes a VT_BYREF/array-style
variant (vt=8) wrapping a style string from `StringPool::GetBSTR` (the tooltip uses `SP` id `0x583`).
For a plain regular font you can pass a default/empty `Ztl_variant_t` (the engine treats empty as
"no extra style"); to match the game exactly, wrap a style bstr via `sub_4626B3`. Bold/italic are encoded
in this style string, not as separate args.

**Extracting the raw `IWzFont*` to hand to DrawTextA:** the font is stored as a `_com_ptr_t<IWzFont>`
(4-byte slot). Dereference it (`*slot`) to get the raw `IWzFont*` — that's what `sub_4284A5`
(`0x004284A5`, `return *this;`) does in `CCtrlCheckBox::Draw` (`v23 = sub_4284A5(p_m_pFont)`,
`/*0x4c1bf4*/`) before passing `v23` as DrawTextA's `pFont`.

**Caching:** there is no global font singleton you can borrow directly; fonts are owned per-object
(CCtrlEdit stores `m_pFont`/`m_pFontDisabled`; CUIToolTip caches ~22 fonts at `this+0x424..0x470`). For
the custom-UI framework, **create one `IWzFont` once per (face,size,color,style) and cache it on the
CustomWnd**, then reuse its raw pointer across Draw calls. Release it (COM `Release`, vtable slot +8)
when the window is destroyed.

### 3. `CWnd::GetCanvas` (`0x00425C4C`) — signature + lifetime rule

```c
// __thiscall; out-param is a _com_ptr_t<IWzCanvas> (4-byte). Returns the same out ptr.
_com_ptr_t<IWzCanvas>* __thiscall CWnd::GetCanvas(CWnd* this, _com_ptr_t<IWzCanvas>* pOut /*, [unused]*/);
```

Behavior (decompile `0x00425C4C`): picks the window's layer (`this[8]` if set, else `this[6]`), calls
`IWzGr2DLayer::GetCanvas` (`0x00425D2E`) on it, then `ZComAPI::ZComVariantCopy`. It returns an
`IWzCanvas` **`_com_ptr_t` (a ref-counted COM pointer)** for the window's layer.

**Lifetime rule (IMPORTANT):** GetCanvas returns an **owned (AddRef'd) reference**. You MUST Release it
when done. Both reference call-sites do this: `CCtrlCheckBox::Draw` ends with `if (v31) (*(*v31+8))(v31);`
(`/*0x4c1c62*/`) — i.e. call COM `Release` (vtable slot +8) on the canvas pointer at end of Draw. The raw
`IWzCanvas*` to pass to DrawTextA is `*pOut` (deref the `_com_ptr_t`), obtained via `sub_414576`
(`0x00414576`, deref-with-null-check) in the checkbox path. The canvas **is valid inside Draw** (slot 11)
— that is exactly where the engine's own control Draws obtain and use it.

> Note: `CCtrlWnd::GetCanvas` (`0x004C0690`) is the control-level variant used by `CCtrlCheckBox::Draw`;
> for a **window** (CUIWnd) Draw override, use `CWnd::GetCanvas` (`0x00425C4C`). Same return/lifetime
> contract (owned `_com_ptr_t<IWzCanvas>`; Release at end).

### 4. Draw-override ordering — confirmed sound

In the cloned slot-11 `Draw(CUIWnd* self, const tagRECT* rc)`:

1. **(optional) Call the original `CWnd::Draw` (`0x009E0502`) first** to paint the window's own
   background. This is a no-op if no background is set (per Q6), so it is safe and correct to call it
   first; child control layers composite independently of Draw (per Q6), so ordering vs. children does
   not matter.
2. `GetCanvas(self, &canvas)` — get the window-layer canvas (owned ref).
3. For each text item: build `_bstr_t(text)`, build the opacity variant `sub_402FAB(&vA, 255, 3)`, build
   an empty `vTabOrg` variant, then `DrawTextA(rawCanvas, x, y, bstr.m_Data, rawFont, vA, vTabOrg)`.
   DrawTextA consumes the bstr ref (do not release it).
4. **Release the canvas** (`(*(*canvas+8))(canvas)`), and `ZComVariantCopy`/clear the variants as the
   engine does (or just let stack `Ztl_variant_t` dtors run in C++).

This matches the engine's own control-Draw structure exactly (`CCtrlCheckBox::Draw`).

**Coordinate space:** `(nLeft, nTop)` are **layer/window-relative** coordinates (the canvas belongs to
the window's own layer), NOT screen coordinates. Evidence: `CCtrlCheckBox::Draw` passes `rx+2`, `ry+2`
where `rx/ry` are the control-relative draw offsets the parent passes in; `DrawTextCenter` passes
`(width - textWidth)/2` — pure local coordinates. So draw labels at coordinates relative to the window's
top-left (0,0 = window origin).

### 5. String / bstr / variant helper addresses

| Address | Symbol | Signature / use |
|---|---|---|
| `0x00425ADD` | `_bstr_t::_bstr_t(const char*)` (`??0_bstr_t@@QAE@PBD@Z`) | build the `Ztl_bstr_t` text arg from ANSI `char*` (converts to wide). One ref consumed by DrawTextA. |
| `0x00402FAB` | `Ztl_variant_t::ctor_i4` (`sub_402FAB`) | build a VT_I4 variant: `(VARIANT* this, int value, short vt=3)`. Use for the opacity arg (value=alpha 0-255) and for an empty tab-origin (value=0). |
| `0x004626B3` | `Ztl_variant_t::make_style` (`sub_4626B3`) | build the font style variant (vt=8) from a style bstr; for the `IWzFont::Create` `sStyle` arg. Empty variant = regular. |
| `0x004284A5` / `0x00414576` / `0x004284A8` | `_com_ptr_t::deref` (`sub_4284A5`/`sub_414576`/`sub_4284A8`) | dereference a `_com_ptr_t` to its raw interface pointer (font / canvas). `0x414576` is the null-checked variant. |
| `0x00402EA5` | `_bstr_t::Data_t::Release` | release a bstr Data_t (only if you build a bstr you do NOT pass to DrawTextA). |

### Copy-pasteable C++ pseudocode

```c
// One-time per (face,size,argbColor): create + cache a font on the CustomWnd.
IWzFont* CustomUI_MakeFont(const wchar_t* face, unsigned size, unsigned int argb /*0xAARRGGBB*/) {
    IWzFont* pFont = NULL;
    ((void(__cdecl*)(const wchar_t*, IWzFont**, int))0x00463670)(face, &pFont, 0); // PcCreateObject::IWzFont
    Ztl_variant_t style; variant_init_empty(&style);              // regular style; or 0x004626B3 for bold/italic
    Ztl_bstr_t name; ((void(__thiscall*)(Ztl_bstr_t*,const char*))0x00425ADD)(&name, /*ansi face*/);
    ((long(__thiscall*)(IWzFont*, Ztl_bstr_t, unsigned long, unsigned long, const Ztl_variant_t&))
        0x0046341A)(pFont, name, size, argb, style);              // IWzFont::Create
    return pFont;   // raw owned IWzFont* — Release (vtbl+8) when the window is destroyed
}

// Draw one label inside the slot-11 Draw override. 'rawFont' is the cached raw IWzFont*.
// argb's ALPHA byte is applied as opacity; its RGB came from the font's uColor at Create time.
void DrawLabel(CUIWnd* self, int x, int y, const char* utf8, unsigned int argb) {
    // 1. window-layer canvas (owned _com_ptr_t<IWzCanvas>)
    void* canvas = NULL;                                          // _com_ptr_t<IWzCanvas>
    ((void*(__thiscall*)(CUIWnd*, void**))0x00425C4C)(self, &canvas);   // CWnd::GetCanvas
    IWzCanvas* rawCanvas = *(IWzCanvas**)&canvas;

    // 2. text bstr (DrawTextA consumes one ref — do NOT release it)
    Ztl_bstr_t text;
    ((void(__thiscall*)(Ztl_bstr_t*, const char*))0x00425ADD)(&text, utf8);   // _bstr_t(const char*)

    // 3. opacity variant (VT_I4 = alpha 0..255) and empty tab-origin variant
    Ztl_variant_t vAlpha, vTab;                                   // each 16 bytes
    ((void(__thiscall*)(void*, int, short))0x00402FAB)(&vAlpha, (argb >> 24) & 0xFF, 3);
    ((void(__thiscall*)(void*, int, short))0x00402FAB)(&vTab, 0, 3);

    // 4. draw  (font carries RGB; rawFont is the cached IWzFont*)
    ((unsigned long(__thiscall*)(IWzCanvas*, long, long, Ztl_bstr_t, IWzFont*,
                                 const Ztl_variant_t&, const Ztl_variant_t&))
        0x004277AD)(rawCanvas, x, y, text, self->cachedFont, vAlpha, vTab);   // IWzCanvas::DrawTextA

    // 5. release the canvas ref (COM Release == vtable slot +8). Variants: let dtors clear, or ZComVariantCopy.
    if (rawCanvas) (*(void(__thiscall**)(IWzCanvas*))(*(void**)rawCanvas + 8))(rawCanvas);
}

// In the override: optionally paint base background first, then labels.
void __fastcall CustomUI_Draw(CustomWnd* self, void* /*edx*/, const tagRECT* rc) {
    ((void(__thiscall*)(void*, const tagRECT*))0x009E0502)(self, rc);   // original CWnd::Draw (bg; no-op if none)
    for (each label L in self->labels)
        DrawLabel((CUIWnd*)self, L.x, L.y, L.text, L.argb);
}
```

### NEW memory-map symbols (names + addresses)

| Address | Suggested symbol | Notes |
|---|---|---|
| `0x0046341A` | `IWzFont::Create` | `?Create@IWzFont@@QAEJVZtl_bstr_t@@KKABVZtl_variant_t@@@Z`; `(Ztl_bstr_t face, ulong height, ulong argbColor, const Ztl_variant_t& style)` → HRESULT. The real font factory. |
| `0x00463670` | `PcCreateObject::IWzFont` | `?IWzFont@PcCreateObject@@IAEXXZ`; allocates a blank `IWzFont` COM object: `(LPCWSTR hint, IWzFont** ppOut, int aggregate)`. |
| `0x00425ADD` | `_bstr_t::_bstr_t(const char*)` | `??0_bstr_t@@QAE@PBD@Z`; build `Ztl_bstr_t` text arg from ANSI. |
| `0x00402FAB` | `Ztl_variant_t::ctor_i4` | `sub_402FAB`; VT_I4 variant builder `(this, value, vt=3)`; used for the opacity arg + empty tab-origin. |
| `0x004626B3` | `Ztl_variant_t::make_style` | `sub_4626B3`; style variant (vt=8) for `IWzFont::Create`. |
| `0x004284A5` | `_com_ptr_t::deref` | `sub_4284A5` (`return *this`); raw IWzFont* from `_com_ptr_t`. |
| `0x00414576` | `_com_ptr_t::deref_checked` | `sub_414576`; null-checked deref → raw `IWzCanvas*`. |
| `0x004284A8` | `_com_ptr_t::deref` (font) | `sub_4284A8`; deref used before `IWzFont::Create`. |
| `0x00425D2E` | `IWzGr2DLayer::GetCanvas` | layer-level canvas getter called by `CWnd::GetCanvas`. |
| `0x004C1A83` | `CCtrlCheckBox::Draw` | reference text-draw site (GetCanvas → DrawTextA → Release pattern). |
| `0x008F466D` | `CUIToolTip::DrawTextCenter` | second reference text-draw site (`(width-textWidth)/2` local coords). |
| `0x008E49B5` | `CUIToolTip::CUIToolTip` | font-cache construction site (PcCreateObject::IWzFont + IWzFont::Create). |

> Color recap: the **font** holds the ARGB text color (`IWzFont::Create` `uColor`, e.g. 0xFF000000 black /
> 0xFFFFFFFF white). `DrawTextA`'s color argument is a **VT_I4 opacity variant (0-255)**, not a packed
> ARGB int. Bake RGB into the font; pass alpha as the variant. Still uncertain only: the exact bold/italic
> bit encoding inside the style string for `IWzFont::Create` (regular works with an empty style variant;
> the engine sources its style strings from StringPool ids).
