#pragma once
#include <comip.h>

///
/// Maplestory types that are common between many version will go in here.
/// Eventually we can use preprocessor macros to construct these based on version.
/// But for now I'm adding ones I figure are constant between all (32-bit) versions.
///

#include "asserts.h"
#include "logger.h"

#include "ZRef.h"
#include "ZRefCounted.h"
#include "TSecType.h"
#include "ZArray.h"
#include "ZList.h"
#include "ZMap.h"
#include "ZXString.h"
#include "ZPair.h"

typedef const struct
{
    unsigned int magic;
    void* object;
    void* info; // _s__ThrowInfo
} CxxThrowExceptionObject;

struct Ztl_bstr_t : _bstr_t
{
};

struct Ztl_variant_t : _variant_t
{
};

struct RANGE
{
    int low;
    int high;
};

struct UINT128
{
    unsigned int m_data[4];
};

typedef DWORD __POSITION;