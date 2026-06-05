# v83.1 CWndMan / CFuncKeyMappedMan — Calling-Convention & Struct-Return Findings

Target IDB: `MapleStory_dump.exe` (GMS **v83.1**, "v83_Me"), image base `0x400000`, MCP port 13337.
All findings anchored to decompiler + disassembler evidence from that IDB. Read-only investigation; IDB not modified.

---

## Investigation 1 — Three CWndMan-related functions

### (1a) `0x009E40F1` — `CWndMan::ProcessKey`  ✅ CONFIRMED `__thiscall`

IDA mangled name on the function: `?ProcessKey@CWndMan@@QAEJIIJ@Z`
- `QAE` = public member, `__thiscall`.
- `J` return = `long`.
- Args `I I J` = `unsigned int, unsigned int, long` (3 explicit args after `this`).

**Prologue (this in ECX):**
```
9e40f1  mov eax, offset loc_AE6F7C      ; SEH scope setup
9e40f6  call __EH_prolog
9e40fb  push ebx / push esi / push edi
9e40fe  mov edi, [ebp+arg_8]            ; reads stack arg (lParam)
9e4101  mov esi, ecx                    ; <-- ECX captured as `this`
9e4103  mov ecx, edi
```
ECX is used as the `this` pointer (`mov esi, ecx`, later `[esi+88h]`, `CWndMan::SetFocus(esi,...)`). Confirms **thiscall**.

**Epilogue (callee-cleaned):**
```
9e428e  retn 0Ch                        ; cleans 12 bytes = 3 stack dwords
```
`retn 0Ch` + `this` in ECX ⇒ thiscall with exactly 3 stack args.

**Decompiled signature (IDA): ** `int __thiscall CWndMan::ProcessKey(IUIMsgHandler *this, int msg, void *vk, int lParam)`.
IDA typed arg2 as `void*` because of the `a1 == 145` / `== 0x7B` comparisons, but the mangled name says it is `unsigned int`.

**VERDICT — the H2 hook / common thunk assumption is CORRECT:**
```cpp
long __thiscall CWndMan::ProcessKey(CWndMan* this, unsigned int msg, unsigned int vk, long lParam);
```
- Convention: `__thiscall` (this in ECX), callee-cleaned `retn 0Ch`.
- Arg count: 3 explicit + `this`.
- Returns `long` (EAX); always returns 0 in observed paths.

> NOTE on the committed `common/CWndMan.cpp` thunk that called ProcessKey as
> `__thiscall (this, nullptr, msg, vk, lParam)`: that signature has FOUR explicit
> args (an extra leading `nullptr`). That is WRONG — ProcessKey takes only
> `(msg, vk, lParam)`. The extra `nullptr` arg would corrupt the stack and
> mismatch the `retn 0Ch` cleanup. Use the 3-arg form above.

---

### (1b) `0x009E43FF` — RegisterUIWindow  ✅ CONFIRMED `__cdecl` FREE/STATIC function (single arg)

IDA has this UNNAMED (`sub_9E43FF`). It is **not** a `CWndMan` instance method.

**Prologue (NO ecx-as-this; arg from stack):**
```
9e43ff  push ebx
9e4400  mov ebx, [esp+4+arg_0]          ; <-- single arg read from STACK, not ECX
9e4404  cmp dword ptr [ebx+18h], 0      ; operates on the passed window ptr
```
ECX is never read as `this`. The single argument (the window/CWnd pointer) comes off the stack.

**Body** operates on a global list head `off_BF1648` / `dword_BF1654` (the global UI window registry), not on instance fields of a CWndMan.

**Epilogue (caller-cleaned):**
```
9e44b9  retn                            ; PLAIN retn = caller cleans = __cdecl
```

**VERDICT:**
```cpp
int __cdecl RegisterUIWindow(CWnd* pWnd);   // single arg, plain retn (cdecl)
```
- Convention: `__cdecl` (all args on stack, caller-cleaned).
- Arg count: 1 (the window pointer).
- Returns `int` (an HRESULT-ish value from a vtbl[180] call; <0 triggers `_com_issue_errorex`).
- Operates on the GLOBAL window list at `off_BF1648`, NOT a CWndMan `this`.

> NOTE: the committed thunk that modeled this as `__thiscall CWndMan::Register(this, nullptr, pWnd)`
> is MIS-TYPED. It is a cdecl free function `(pWnd)`. No `this`, no leading `nullptr`.

---

### (1c) `0x009E44BA` — RemoveWindow / UnregisterUIWindow  ✅ CONFIRMED `__cdecl` (single arg)

IDA mangled name: `?RemoveWindow@CWndMan@@KAHPAVCWnd@@@Z`
- `KA` = **static** member function (`K` = static; static members carry NO `this` and use `__cdecl`).
- `H` return = `int`.
- `PAVCWnd@@` = single arg `CWnd*`.

**Prologue (arg from stack; ecx set locally, not incoming this):**
```
9e44ba  push esi
9e44bb  push 0
9e44bd  lea eax, [esp+8+arg_0]          ; <-- takes ADDRESS of the stack arg
9e44c1  mov esi, offset off_BF1648      ; global list head (loaded locally)
9e44c6  push eax
9e44c7  mov ecx, esi                    ; ecx = &global list (NOT incoming this)
9e44c9  call sub_9E4D55
```
The incoming arg is the stack `arg_0` (the window ptr). ECX is loaded *inside* the function with the address of the global list `off_BF1648` to call helper sub_9E4D55 (a thiscall helper on the list), so the incoming convention has no `this`.

**Epilogue (caller-cleaned):**
```
9e44db  retn                            ; PLAIN retn = __cdecl
```

**VERDICT:**
```cpp
int __cdecl CWndMan::RemoveWindow(CWnd* pWnd);   // static member -> cdecl, plain retn
```
- Convention: `__cdecl` (static member; single stack arg, caller-cleaned).
- Arg count: 1 (the window pointer; passed BY VALUE on the stack, function takes its address internally).
- Returns `_DWORD*`/`int`.
- Operates on the GLOBAL list `off_BF1648`, NOT a per-instance CWndMan `this`.

> NOTE: the committed thunk modeling this as `__thiscall CWndMan::Unregister(this, nullptr, pWnd)`
> is MIS-TYPED. It is a cdecl (static) function `(pWnd)`.

---

### Reconciliation summary (the known conflict)

| Addr | Believed | TRUE convention | TRUE signature |
|------|----------|-----------------|----------------|
| 0x9E40F1 | ProcessKey thiscall | ✅ `__thiscall`, `retn 0Ch` | `long __thiscall ProcessKey(CWndMan*, uint msg, uint vk, long lParam)` |
| 0x9E43FF | RegisterUIWindow cdecl(window) | ✅ `__cdecl`, plain `retn` | `int __cdecl RegisterUIWindow(CWnd*)` |
| 0x9E44BA | RemoveWindow cdecl(window) | ✅ `__cdecl` (static member), plain `retn` | `int __cdecl CWndMan::RemoveWindow(CWnd*)` |

The committed `common/CWndMan.cpp` thunk that wrapped Register/Unregister as
`__thiscall CWndMan` methods `(this, nullptr, pWnd)` and ProcessKey as
`(this, nullptr, msg, vk, lParam)` is **mis-typed on all three** w.r.t. the extra
`nullptr`/`this`. Register and Unregister are cdecl free/static functions taking
only `(pWnd)`. ProcessKey is thiscall taking only `(msg, vk, lParam)`.

**How to call from a thunk:**
- ProcessKey: thiscall surrogate — pass `this` in ECX, push `lParam, vk, msg` (right-to-left); callee cleans. In a C++ thunk just declare it `long(__thiscall*)(void* self, unsigned, unsigned, long)`.
- RegisterUIWindow / RemoveWindow: plain cdecl — `int(__cdecl*)(void* pWnd)`. No `this`, no extra arg.

---

## Investigation 2 — `CFuncKeyMappedMan::FuncKeyMapped(int vk)`

### Result: there is NO standalone `FuncKeyMapped` function in v83.1 — it is fully INLINED.

The memory map had it as `0` because it does not exist as a discrete function in this
build. There is no `?FuncKeyMapped@CFuncKeyMappedMan@@...@Z` symbol (verified by
`func_query` filter `*FuncKeyMapped*` and `*FUNCKEY_MAPPED*` — the only struct-typed
match is `?LoadFuncKeyMapped@CConfig@@QAEHPAUFUNCKEY_MAPPED@@@Z` at `0x49DF79`, which
takes a `FUNCKEY_MAPPED*` out-param, NOT a `CFuncKeyMappedMan` accessor).

### What the accessor actually compiles to (inlined array index)

Every caller indexes the `FUNCKEY_MAPPED[]` array embedded in the singleton directly.
The singleton instance pointer is `dword_BED5A0` (= `0x00BED5A0`, matches the brief).

Inlined pattern (identical across all callers):
```c
// vk normalization seen everywhere: if BYTE2(key)==54 use 42, else BYTE2(key)
v = (BYTE2(key) == 54) ? 42 : BYTE2(key);
p = dword_BED5A0 + 4*v + v + 4;      // == instance + 5*vk + 4
//   *p        -> FUNCKEY_MAPPED.nType (u8 at element+0)
//   *(p + 1)  -> FUNCKEY_MAPPED.nID   (int at element+1)
```
Evidence:
- `CUserLocal::UseFuncKeyMapped` @0x94F2ED: `v5 = dword_BED5A0 + 4*v4 + v4 + 4;` then `*v5` (nType), `*(v5+1)` (nID).
- `CWvsContext::UseFuncKeyMapped` @0xA0773D: `*(dword_BED5A0 + 4*v3 + v3 + 5)` (reads nType at element+0 — note +5 here is +4 base + the element body; both forms resolve to element start = base+4+5*vk).
- `CWvsContext::ProcessBasicUIKey` @0xA07431: `v9 = dword_BED5A0 + 4*v8 + v8 + 4; if (*(v9+1) < 0x1C && *v9 == 4) ...`.

### Array layout — confirmed from the ctor `??0CFuncKeyMappedMan@@QAE@XZ` @0x58DD0D

```
58dd11  lea eax, [esi+4]                 ; dst = this + 4  (array base)
58dd19  mov ebx, 1BDh                    ; size = 0x1BD = 445 bytes
58dd24  mov edi, offset dword_BD8BCC     ; src = default keymap table
58dd39  call _memcpy                     ; memcpy(this+4, default, 445)
```
- Array base offset = **+4** within the `CFuncKeyMappedMan` instance.
- Total bytes = `0x1BD` = 445 = **89 elements × 5 bytes** (vk indices 0..88).
- Element stride = **5 bytes** = `sizeof(FUNCKEY_MAPPED)`.

`FUNCKEY_MAPPED` (verified via `type_inspect`, size 5, packed):
```c
struct FUNCKEY_MAPPED {  // size = 5, packed (no padding)
    unsigned __int8 nType;  // +0
    int             nID;    // +1
};
```

### Struct-return mechanism — N/A in v83.1 (because inlined)

The brief asked how the >4-byte struct return is passed (MSVC hidden return-slot
pointer). **In v83.1 this question is moot: there is no call** — `FuncKeyMapped(vk)`
is never emitted as a function, so there is no EDX:EAX vs hidden-pointer ABI to honor.
The existing thunk that "assumed EDX:EAX return" is calling a function that **does not
exist** in this build.

### How a thunk / port should obtain a FUNCKEY_MAPPED in v83.1

Do NOT try to call a `FuncKeyMapped` function. Replicate the inline access instead:

```cpp
// pMan = *(CFuncKeyMappedMan**)0x00BED5A0  (the singleton instance pointer)
// vk in [0,88]; apply the 54->42 normalization the client uses if you took it from BYTE2(key).
struct FUNCKEY_MAPPED { unsigned char nType; int nID; }; // packed, 5 bytes

inline FUNCKEY_MAPPED FuncKeyMapped(void* pMan, int vk) {
    const unsigned char* arr = (const unsigned char*)pMan + 4; // array base
    const unsigned char* e   = arr + 5 * vk;                   // 5-byte stride
    FUNCKEY_MAPPED r;
    r.nType = e[0];
    r.nID   = *(const int*)(e + 1);   // unaligned read; struct is packed
    return r;
}
```

If a discrete callable is required anyway, use the trivial singleton getter
`sub_801B47` @0x801B47 (`return dword_BED5A0;`) to fetch the instance, then index as above.

> CRITICAL CORRECTION: any thunk assuming a 5-byte struct returned in EDX:EAX from a
> `CFuncKeyMappedMan::FuncKeyMapped` call is invalid for v83.1 — the function was
> inlined out of existence. Read the packed array element directly at
> `instance + 4 + 5*vk` (nType=byte, nID=int-at-+1).

---

## Address reference

| Symbol | Address |
|--------|---------|
| CWndMan::ProcessKey | `0x009E40F1` |
| RegisterUIWindow (cdecl free fn) | `0x009E43FF` |
| CWndMan::RemoveWindow (static, cdecl) | `0x009E44BA` |
| CFuncKeyMappedMan singleton ptr (`dword_BED5A0`) | `0x00BED5A0` |
| CFuncKeyMappedMan ctor | `0x0058DD0D` |
| CFuncKeyMappedMan singleton getter (`return instance`) | `0x00801B47` |
| CFuncKeyMappedMan::FuncKeyMapped | **does not exist (inlined)** |
| CConfig::LoadFuncKeyMapped (takes FUNCKEY_MAPPED*) | `0x0049DF79` |
