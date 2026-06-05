# v83.1 CUIWnd Construction / Geometry / Teardown Protocol

Target IDB confirmed via `get_metadata`: `D:\Programs\Nexon\MapleStory\v83_Me\MapleStory_dump.exe`,
base `0x400000`, module `MapleStory_dump.exe`, md5 `80ff438ced539b831f0d2ed95099275d`. This is the
v83 GMS dump. All addresses below are absolute (base 0x400000).

> TL;DR for the framework: there are **two** CUIWnd ctors. The 7-arg "wide-string" ctor at
> `0x0092C17F` *always* builds a UOL string and calls `CWnd::SetBackgrnd` — passing it a real
> resource is expected. The other ctor, `??0CUIWnd@@QAE@HHHHHHH@Z` at **`0x0092C0BF`**, is a
> pure-int ctor that **never touches a string and never loads a background** — it is the safe
> base for a custom, programmatically-drawn window. Geometry is set *after* construction via
> `CWnd::CreateWnd` (`0x009DE4D2`), which accepts explicit l/t/w/h. Teardown is the virtual
> `~CUIWnd` at `0x0092C272` (plain `(this)`), normally reached through the scalar-deleting-dtor
> thunk `??_GCUIWnd` at `0x0092C15B` with signature `(this, char flags)`.

---

## Q1. Ctor signature (`0x0092C17F`) and null-background safety

### Mangled name / arg count
`??0CUIWnd@@QAE@HHHHPBGHHH@Z` decodes to params: `int, int, int, int, const unsigned short*, int, int, int`
i.e. 4 ints, a wide-char pointer, then 3 ints. The decompiler signature:

```c
void __thiscall CUIWnd::CUIWnd(
        CUIWnd *this,
        int nUIType,           // -> [this+0x588] m_nUIType
        int closeType,         // -> [this+0x58C] m_nBtCloseType
        int closeX,            // -> [this+0x590] m_nBtCloseX
        int closeY,            // -> [this+0x594] m_nBtCloseY
        const wchar_t *sBackgrndUOL,
        int nBackgrndX,        // passed to CWnd::SetBackgrnd
        int nBackgrndY,        // passed to CWnd::SetBackgrnd
        _bstr_t::Data_t *bMultiBg)   // see "retn 1Ch" note below
```

Body (decompiled, abridged with addresses):

```c
CWnd::CWnd(this);                                    // 0x92C192 base ctor
this->m_pBtClose[1]    = 0;                           // 0x92C19C [this+0x70]
CUIToolTip::CUIToolTip(&this->m_uiToolTip);           // 0x92C1A6 [this+0x74]
this->m_abOption       = 0;                           // 0x92C1AB [this+0x5A8]
this->m_sBackgrndUOL   = 0;                           // 0x92C1B1 [this+0x5AC]
this->m_nUIType        = nUIType;                     // 0x92C1BA [this+0x588]
this->m_nBtCloseType   = closeType;                   // 0x92C1C3 [this+0x58C]
this->m_nBtCloseX      = closeX;                       // 0x92C1CC [this+0x590]
this->__vftable        = &off_B3CE10;                 // 0x92C1D5 CUIWnd primary vftable
this->IUIMsgHandler::__vftable = &off_B3CDC4;         // 0x92C1DB
this->ZRefCounted::__vftable    = &off_B3CDC0;        // 0x92C1E2
this->m_nBtCloseY      = closeY;                       // 0x92C1E9 [this+0x594]
this->m_nBackgrndX     = 0;                            // 0x92C1EF [this+0x598]
// build UOL string from StringPool fmt SP_2167 and sBackgrndUOL:
m_pStr = StringPool::GetStringW(.., SP_2167_UI_UIWINDOWIMG_S_BACKGRND)->_m_pStr; // 0x92C211
ZXString<ushort>::Format(&v19, m_pStr, sBackgrndUOL); // 0x92C21F   <-- DEREFERENCES sBackgrndUOL
_bstr_t::_bstr_t(&v15, v13);                           // 0x92C248
CWnd::SetBackgrnd(this, v15.m_Data, nBackgrndX, nBackgrndY); // 0x92C24F
```

### Does it copy-from / dereference `sBackgrndUOL`?
**Yes.** At `0x92C21F` the ctor calls `ZXString<ushort>::Format(out, fmtFromStringPool, sBackgrndUOL)`.
`SP_2167_UI_UIWINDOWIMG_S_BACKGRND` is a `%s`-style format string (the same id is reused by
`CUIWnd::OnCreate` at `0x92C338` to format a `UI/UIWindow.img/<name>/backgrnd` UOL). So `sBackgrndUOL`
is consumed as the `%s` argument and **will be dereferenced as a wide string**.

**Null/empty handling:** passing `nullptr` is **not safe for this ctor** — `Format` will dereference
it as a `%s` wide pointer (undefined/crash). `CWnd::SetBackgrnd` (`0x009E0AB2`) *does* tolerate a null
`_bstr_t::Data_t` at the very end (it only skips the trailing `Release` when `a2 == 0`), but by then
`GetObjectA` has already been called with the (garbage) formatted UOL string and the format step has
already touched the null pointer. **Conclusion: do not call `0x0092C17F` with a null/empty background.**
Use the int-only ctor at `0x0092C0BF` instead (see Q2).

### `retn 1Ch` reconciliation
The function ends `retn 1Ch` = 28 bytes = **7 caller-pushed dwords** (plus `this` in ecx → `__thiscall`).
The decompiler/mangled name show an 8th param `bMultiBg` (`_bstr_t::Data_t*` at `ebp+0x40`), but every
caller passes an **uninitialized local** there (e.g. `CUIPartySearch` at `0x877623`:
`CUIWnd::CUIWnd(a1, 22, 3, 283, 8, aPa, 0, 0, v8)` where `v8` is `_bstr_t::Data_t *v8; // [esp+0h]`).
That slot is the `_bstr_t` return-by-value temporary the ctor builds internally, **not** a real 8th
argument. So the real ABI is **7 stack dwords (nUIType, closeType, closeX, closeY, sBackgrndUOL,
nBackgrndX, nBackgrndY) + `this` in ecx, `retn 1Ch`** — consistent with the disassembly.

---

## Q2. What `nUIType` / `closeType` control, and safe values

### `nUIType` (`[this+0x588]`)
`nUIType` is the window's **type id / config key**, not a layout driver. It is used:
- as the StringPool sub-key when formatting the background UOL (ctor `0x92C211`), and
- as the key into `CConfig::GetUIWndPos(nUIType, &x, &y, &option)` to fetch the **saved screen
  position** for this window (`CUIWnd::CreateUIWndPosSaved` at `0x00801A29`, line `GetUIWndPos(.., this->m_nUIType, ..)`).

So `nUIType` ties the window to a persisted position slot and a WZ resource key. For a custom window
that does **not** want a persisted position or a WZ background, `nUIType` only needs to be a value that
`CConfig::GetUIWndPos` handles benignly. Observed real values from callers (xrefs to both ctors):

| UI                | ctor used | nUIType | closeType | closeX | closeY | nBackgrnd(args 5..7) |
|-------------------|-----------|---------|-----------|--------|--------|----------------------|
| CUIEquip          | 0x92C0BF  | 1       | 3         | 155/188| 6      | 1, 0, 0              |
| CUIPartySearch    | 0x92C17F  | 22      | 3         | 283    | 8      | "Pa"(wstr), 0, 0     |
| CUIItemMaker      | 0x92C17F  | 23      | 3         | 292    | 6      | "Ma"(wstr), 0, 0     |

### `closeType` (`[this+0x58C]`) — controls the close button
This is read in `CUIWnd::OnCreate` (vtable slot 13, `0x0092C2E8`) as `*(this+1420)` (= `[this+0x58C]`):

```c
v11 = *(this + 1420);          // 0x92C3BF  m_nBtCloseType
if ( v11 ) {                   // 0x92C3C7  ONLY build a close button if nonzero
    v12 = v11 - 1;             // switch on closeType
    // 1 -> SP_1821 BtUIClose      (0x92C4A4)
    // 2 -> SP_1549 BtUIClose2     (0x92C479)
    // 3 -> SP_5577 UI/Basic BtClose (0x92C44E)  <-- most common (Equip/PartySearch/ItemMaker)
    // 4 -> SP_2168 UI/UIWindow BtClose (0x92C40F)
    // other -> generic CCtrlButton (LABEL_19, 0x92C4C9)
    CCtrlButton::CCtrlButton(...);
}
// if closeType == 0: the entire close-button block is skipped -> NO close button
```

**Recommendation for a custom window with no WZ background:**
- **Use the int-only ctor `0x0092C0BF`**, not `0x0092C17F` (the int ctor never formats/loads a string).
- `nUIType`: pick an id that is harmless to `CConfig::GetUIWndPos`. Reusing an existing benign slot
  (e.g. `1`, as CUIEquip does) is the most evidence-backed choice; a never-saved id returns a default
  position. If you set geometry yourself via `CreateWnd` (Q3) the saved-pos lookup is irrelevant.
- `closeType = 0` → **no close button** (confirmed: OnCreate skips the block). If you want the standard
  close button instead, use `3`.
- `closeX`, `closeY` are the close-button pixel offsets inside the window; with `closeType = 0` they are
  unused, so `0, 0` is fine.
- For the int ctor, args 5..7 are `nBackgrnd (resource id), nBackgrndX, nBackgrndY` stored at
  `[this+0x598/0x59C/0x5A0]`. Pass `0, 0, 0`; with our own `OnCreate`/`Draw` override no WZ canvas is loaded.

The int-only ctor body (`0x0092C0BF`) confirms it does **none** of the string/SetBackgrnd work:

```c
CWnd::CWnd(this);
this->m_nUIType      = nUIType;     // [this+0x588]
this->m_nBtCloseType = closeType;   // [this+0x58C]
this->m_nBtCloseX    = closeX;      // [this+0x590]
this->m_nBtCloseY    = closeY;      // [this+0x594]
this->m_nBackgrndX   = nBackgrnd;   // [this+0x598]  (arg5 stored as a plain int / res id)
[this+0x59C]         = nBackgrndX;  // arg6
[this+0x5A0]         = nBackgrndY;  // arg7 (m_bPosSave region)
this->__vftable      = &off_B3CE10; // same CUIWnd vftable, retn 1Ch
```

---

## Q3. How the screen rectangle (x, y, w, h) is established

**The ctor does NOT set geometry. Geometry is established after construction by `CWnd::CreateWnd`.**

`CWnd::CreateWnd` — `0x009DE4D2`, `__thiscall`:

```c
void __thiscall CWnd::CreateWnd(CWnd *this,
        int l, int t, int *w, int h,   // left, top, width, height  (w typed int* by IDA, value is the width)
        int z,                          // z-order
        int bScreenCoord,               // 1 = screen coords
        int pData,
        int bSetFocus);
```

Evidence it carries the rect:
- `CreateWnd` calls the layer-creating virtual `(this->IGObj::__vftable[8])(this, l, t, w, h, z, bScreenCoord, pData)`
  at `0x9DE515`. Slot 8 is `CWnd::PreCreateWnd` (`0x009DE7FB`).
- In `PreCreateWnd`: `if (a4 && a5)` (width && height nonzero) it creates a layer **sized to (w,h)** via
  `IWzGr2D::CreateLayer(.., a4, a5, ..)` (`0x9DE87B`); it positions the layer's `lt` to `(a2,a3)` = `(l,t)`
  (`0x9DEAC0`); and finally stores `m_width = a4` / `m_height = a5` (`0x9DEB3D`/`0x9DEB44`).
- Back in `CreateWnd`, when no background layer exists it likewise sets `this->m_width = w` / `this->m_height = h`
  (`0x9DE528`/`0x9DE536`) and creates the layer.

So a custom `(x, y, w, h)` **is** honored programmatically, **even with no background canvas**: pass
nonzero width/height to `CreateWnd` and it builds an empty sized layer.

Real-world wrappers (both end in `CreateWnd`):
- `CUIWnd::CreateUIWndPosSaved` (`0x00801A29`): fetches saved (x,y) from `CConfig::GetUIWndPos(nUIType,…)`
  and calls `CWnd::CreateWnd(this, x, y, width, 304, 10, 1, 0, 1)` (width/height hardcoded per UI).
- `CUIPartySearch`/`CUIItemMaker` ctors: derive w/h from the loaded background canvas
  (`sub_40B920`=GetWidth, `sub_40B947`=GetHeight on the canvas at `[this+0x68]`) then call
  `CWnd::CreateWnd(this, x, y, w, h, 10, 1, 0, 1)`.

**Relevant member offsets on CWnd** (from `CWnd` ctor `0x009DE383` and CreateWnd/PreCreateWnd):
- `m_dwWndKey` set in CreateWnd (`0x9DE4EE`).
- `m_rcInvalidated` RECT (cleared in CreateWnd at `0x9DE4F6`).
- `m_pLayer` COM ptr — the visual layer (`m_pInterface` checked throughout).
- `m_width` / `m_height` — set by PreCreateWnd at `0x9DEB3D`/`0x9DEB44` and by CreateWnd at `0x9DE528`/`0x9DE536`.
- `m_bScreenCoord` set in PreCreateWnd (`0x9DE81B`).

**Mechanism to use:** after constructing the object, call
`CWnd::CreateWnd(this, x, y, w, h, /*z=*/10, /*bScreenCoord=*/1, /*pData=*/0, /*bSetFocus=*/1)`.
This both sizes/positions the window's layer **and** registers it for display (it calls
`sub_9E43FF` = RegisterUIWindow internally — see Q6). There is no separate "MoveWindow"/"SetRect"
that geometry must come through; `CreateWnd` is the single entry that establishes the rectangle.
Geometry is **not** background-canvas-only.

---

## Q4. Vtable facts (`C_UI_WND_VFTABLE`)

The CUIWnd primary (IGObj-base) vftable is the value the ctor stores at `[this+0]`:
**`off_B3CE10` = `0x00B3CE10`.** (Both ctors store it; the base `CWnd` ctor first stores `off_B3F078`,
then CUIWnd overrides it.) The two secondary vftables stored at `[this+4]` / `[this+8]` are
`off_B3CDC4` (IUIMsgHandler) and `off_B3CDC0` (ZRefCounted).

Walking `0x00B3CE10` (dword pointers, until they leave code/.rdata range):

| slot | address      | function                                   |
|------|--------------|--------------------------------------------|
| 0    | 0x009E067E   | CWnd::Update                               |
| 1    | 0x004243FE   | CWnd::OnDragDrop                           |
| 2    | 0x009DE7FB   | CWnd::PreCreateWnd                         |
| 3    | 0x0042444B   | (CWnd thunk/virtual)                       |
| 4    | 0x0092C544   | CUIWnd override (sub_92C544)               |
| 5    | 0x009DEB57   | CWnd::OnMoveWnd                            |
| 6    | 0x009E00A6   | CWnd::OnEndMoveWnd                         |
| 7    | 0x0042444E   | (CWnd thunk/virtual)                       |
| 8    | 0x0092C5AE   | CUIWnd::OnButtonClicked                    |
| 9    | 0x0092C5E4   | CUIWnd override (sub_92C5E4)               |
| 10   | 0x009E03A6   | CWndMan::UpdateWindowPosition              |
| 11   | 0x009E0502   | CWnd::Draw                                 |
| 12   | 0x00424403   | (CWnd thunk/virtual)                       |
| 13   | 0x0092C2E8   | CUIWnd::OnCreate                           |
| --   | 0xFDE04000   | NOT a code pointer → end of vftable        |

**Total slot count = 14** (indices 0..13). The dword at `0x00B3CE48` is `0xFDE04000`, which is not a
valid code address, so the vftable terminates after slot 13.

**Destructor slot:** the scalar deleting destructor is **NOT** in this interface vftable (slot 0 is
`CWnd::Update`, not a dtor — these IGObj-style interface vtables don't follow the MSVC "dtor at slot 0"
convention). Destruction is reached through the separate scalar-deleting-dtor thunk `??_GCUIWnd`
(`0x0092C15B`, see Q5), which is referenced by every CUIWnd subclass's own deleting dtor, not via this
vftable. So: `C_UI_WND_VFTABLE = 0x00B3CE10`, `C_UI_WND_VTABLE_SLOT_COUNT = 14`, and there is **no dtor
slot index within this vftable**.

To override drawing for a custom window, the slot of interest is **slot 11 (`CWnd::Draw`, `0x009E0502`)**;
to suppress WZ-background/close-button creation, **slot 13 (`OnCreate`, `0x0092C2E8`)** is the hook point.

---

## Q5. Dtor signature

Two distinct functions:

1. **`CUIWnd::~CUIWnd`** — `??1CUIWnd@@UAE@XZ` at **`0x0092C272`**. `UAE@XZ` = virtual `__thiscall`,
   **no arguments**:
   ```c
   void __thiscall CUIWnd::~CUIWnd(CUIWnd *this);   // 0x0092C272
   ```
   Body restores the three vftables, releases `m_sBackgrndUOL` (`[this+0x5AC]`), `RemoveAll` on the
   ZArray (`[this+0x5A8]`), destroys the tooltip (`[this+0x74]`), releases the close-button ref
   (`[this+0x64]`), then chains to `CWnd::~CWnd` (`0x009DE438`).

2. **Scalar deleting destructor `??_GCUIWnd`** — `sub_92C15B` at **`0x0092C15B`**, `__thiscall`,
   takes `(this, char flags)`:
   ```c
   int *__thiscall CUIWnd_scalar_deleting_dtor(CUIWnd *this, char flags); // 0x0092C15B
   //   CUIWnd::~CUIWnd(this);
   //   if (flags & 1) ZAllocEx<ZAllocAnonSelector>::Free(allocator, this);
   //   return this;
   ```

**Resolution of the plan's 2-arg/3-arg inconsistency:** the *real* destructor `~CUIWnd` is the
**plain 1-arg** form `(this)` at `0x0092C272`. The **2-arg** `(this, char flags)` form is the scalar
deleting destructor thunk at `0x0092C15B`. There is no 3-arg dtor. Call convention:
- If the framework allocated the CUIWnd with the game's `ZAllocEx` allocator and wants it freed too,
  call the deleting dtor: `(*(0x0092C15B))(this, 1)`.
- If the buffer is owned by the framework (e.g. placement-style or your own allocator), call the plain
  dtor `(*(0x0092C272))(this)` and free the buffer yourself, **or** call the deleting dtor with
  `flags = 0` to run the dtor without freeing.

---

## Q6. Show / Hide

`CWndMan::RegisterUIWindow` (`0x009E43FF`) and `CWndMan::RemoveWindow` (`0x009E44BA`, the
"UnregisterUIWindow") are the correct add/remove-from-display calls.

- `RegisterUIWindow(window)` (`0x009E43FF`): inserts the window pointer into the `CWndMan` z-ordered
  list (`off_BF1648`) keyed by the layer z (`sub_44337D` on `[window+24]` = `m_pLayer`), then calls the
  layer's virtual `(*(*layer+180))(layer, z)` to make it visible. **Takes a single `__cdecl` arg** (the
  window pointer).
- `RemoveWindow(window)` (`0x009E44BA`): finds and removes the window from that same list.

**Important:** you normally do **not** call `RegisterUIWindow` directly. `CWnd::CreateWnd`
(`0x009DE4D2`) already calls `sub_9E43FF` (= RegisterUIWindow) at `0x9DE78D` after building the layer,
and also fires `OnCreate` (slot 13) via the layer setup and sets focus if `bSetFocus`. So **`CreateWnd`
both sizes and shows the window**. To hide/destroy, call `CWndMan::RemoveWindow(0x009E44BA)` then the
dtor. No separate "visible flag" needs to be poked for the basic show path; the layer's z/visibility is
managed by the Register/Remove pair plus the `CreateLayer` done in PreCreateWnd.

---

## Recommended construction sequence (pseudocode)

```c
// Resolved addresses (base 0x400000):
//   CUIWnd int-only ctor : 0x0092C0BF   __thiscall, retn 1Ch (7 args)
//   CWnd::CreateWnd       : 0x009DE4D2   __thiscall (l,t,w,h,z,bScreenCoord,pData,bSetFocus)
//   CWndMan::RemoveWindow : 0x009E44BA   __cdecl(window)
//   CUIWnd::~CUIWnd       : 0x0092C272   __thiscall(this)            (plain dtor)
//   ??_GCUIWnd (del dtor) : 0x0092C15B   __thiscall(this, char flags)
//   CUIWnd vftable        : 0x00B3CE10   (14 slots; slot 11 = Draw, slot 13 = OnCreate)

// 1. Allocate a CUIWnd-sized buffer (sizeof must cover through [this+0x5B0]+; the largest
//    CUIWnd member touched is the ZArray at 0x5A8 / ZXString at 0x5AC, so >= 0x5B0 bytes.
//    Use the game allocator if you intend to free via the deleting dtor.)
CUIWnd* w = (CUIWnd*) AllocWnd(/* >= 0x5B0 */);

// 2. Construct: int-only ctor, NO WZ background, NO close button.
//    nUIType=1 (benign existing slot), closeType=0 (no close btn),
//    closeX=0, closeY=0, nBackgrnd=0, nBackgrndX=0, nBackgrndY=0.
((void(__thiscall*)(CUIWnd*,int,int,int,int,int,int,int))0x0092C0BF)
    (w, /*nUIType*/1, /*closeType*/0, /*closeX*/0, /*closeY*/0,
        /*nBackgrnd*/0, /*nBackgrndX*/0, /*nBackgrndY*/0);

// 2b. (Optional but recommended for a fully custom window) overwrite vtable slots in our own
//     vftable copy so Draw (slot 11) renders our content and OnCreate (slot 13) does NOT try to
//     load a WZ background / close button. Point [w+0] at our patched vftable, or subclass.

// 3. Set geometry + show in one call. CreateWnd builds a sized empty layer (no background needed),
//    positions it at (x,y), stores m_width/m_height, and registers it with CWndMan (shows it).
((void(__thiscall*)(CUIWnd*,int,int,int,int,int,int,int,int))0x009DE4D2)
    (w, /*l*/x, /*t*/y, /*w*/width, /*h*/height,
        /*z*/10, /*bScreenCoord*/1, /*pData*/0, /*bSetFocus*/1);

// ... window is now visible and interactive ...

// 4. Teardown:
//    a) remove from the display/z-order list
((void(__cdecl*)(CUIWnd*))0x009E44BA)(w);
//    b) run the destructor. If the game allocator owns the buffer, use the deleting dtor with flag 1:
((void*(__thiscall*)(CUIWnd*,char))0x0092C15B)(w, /*free*/1);
//       Otherwise run the plain dtor and free yourself:
//    ((void(__thiscall*)(CUIWnd*))0x0092C272)(w);  FreeWnd(w);
```

### Notes / caveats
- **Do not** use the `0x0092C17F` (wide-string) ctor with a null/empty background — it always formats
  and `SetBackgrnd`s the string and will dereference the pointer. The int ctor `0x0092C0BF` is the safe
  background-free path.
- If you keep CUIWnd's stock vftable, `CreateWnd`'s call into `OnCreate` (slot 13, `0x0092C2E8`) will
  attempt to build a background from the `SP_2167` UOL using `m_nUIType`. To guarantee no WZ lookup,
  override slot 13 (OnCreate) and slot 11 (Draw) in a window-specific vftable — that is the intended
  "draw via overridden vtable slot" hook.
- `nUIType`: any value works for raw construction; it only matters for `CConfig::GetUIWndPos` (saved
  position) and the WZ background key. Since we set geometry explicitly via `CreateWnd` and override
  OnCreate, the saved-position path is bypassed.
```
