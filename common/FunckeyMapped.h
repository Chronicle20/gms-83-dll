#pragma once

/*
00000000 FUNCKEY_MAPPED  struc; (sizeof = 0x5, copyof_1423)
00000000 nType           db ?
00000001 nID             dd ?
00000005 FUNCKEY_MAPPED  ends
*/
#pragma pack(push, 1)
class FUNCKEY_MAPPED
{
    unsigned __int8 nType;
    int nID;
};
#pragma pack(pop)