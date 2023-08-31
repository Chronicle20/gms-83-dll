#pragma once
#include "../common/hooker.h"

/*
	Put all your MapleAPI typedefs in here and link them in MapleAPI.cpp
	If you don't link them in MapleAPI.cpp you will get import errors
*/

/// <summary>
/// Example cdecl typedef. stdcall will use the same format.
/// </summary>
typedef void(__cdecl* _ExampleFunc_cdecl_t)(void* pArg1, int nArg2);
extern _ExampleFunc_cdecl_t _ExampleFunc_cdecl;

/// <summary>
/// Example __thiscall typedef
/// Because this calling convention is only used for member calls, we need to swap it with __fastcall to make it work with our hooks.
/// I'll save you details, but in a nutshell every __fastcall needs at least two arguments.
/// The first argument is pointer to the member class and the second argument is a null pointer to make the calling convention swap work.
/// See more information here: https://guidedhacking.com/threads/thiscall-member-function-hooking.4036/
/// </summary>
typedef int(__fastcall* _ExampleFunc_thiscall_t)(void* pThis, void* edx, int nArg1);
extern _ExampleFunc_thiscall_t _ExampleFunc_thiscall;


/* alternatively, for a shorter solution, use this macro to the same effect as the typedefs above */
/* this expands to the same format as the above examples */
HOOKTYPEDEF_H(void, __stdcall, ExampleFunc_macro, void* pArg1, int nArg2); // but remember, you still need to link it in MapleAPI.cpp