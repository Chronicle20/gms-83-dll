#include "pch.h"
#include "MapleAPI.h"

/*
	Link your externs in here.
*/

// example linking
_ExampleFunc_cdecl_t _ExampleFunc_cdecl;
_ExampleFunc_thiscall_t _ExampleFunc_thiscall;
// end example linking

/* this expands to the same format as the above examples */
HOOKTYPEDEF_C(ExampleFunc_macro);