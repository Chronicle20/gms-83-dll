#include "ZMap.h"
#include "ZXString.h"

struct CSimpleStrMap {
    ZMap<ZXString<char>, ZXString<char>, ZXString<char> > m_mValues;
};
