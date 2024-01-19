#pragma once

/*
00000000 SPSet           struc; (sizeof = 0xC, align = 0x4, copyof_1762)
00000000 _ZtlSecureTear_nJobLevel db 2 dup(? )
00000002 _ZtlSecureTear_nJobLevel_CS dd ?
00000006 _ZtlSecureTear_nSP db 2 dup(? )
00000008 _ZtlSecureTear_nSP_CS dd ?
0000000C SPSet           ends
*/
struct SPSet
{
    unsigned __int8 _ZtlSecureTear_nJobLevel[2];
    __unaligned __declspec(align(1)) unsigned int _ZtlSecureTear_nJobLevel_CS;
    unsigned __int8 _ZtlSecureTear_nSP[2];
    unsigned int _ZtlSecureTear_nSP_CS;
};