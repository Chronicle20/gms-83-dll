#pragma once


/*
00000000 GW_CharacterStat struc; (sizeof = 0xF9, copyof_1764)
00000000 dwCharacterID   dd ?
00000004 sCharacterName  db 13 dup(? )
00000011 nGender         db ?
00000012 nSkin           db ?
00000013 nFace           dd ?
00000017 nHair           dd ?
0000001B aliPetLockerSN  _LARGE_INTEGER 3 dup(? )
00000033 _ZtlSecureTear_nLevel db 2 dup(? )
00000035 _ZtlSecureTear_nLevel_CS dd ?
00000039 _ZtlSecureTear_nJob dw 2 dup(? )
0000003D _ZtlSecureTear_nJob_CS dd ?
00000041 _ZtlSecureTear_nSTR dw 2 dup(? )
00000045 _ZtlSecureTear_nSTR_CS dd ?
00000049 _ZtlSecureTear_nDEX dw 2 dup(? )
0000004D _ZtlSecureTear_nDEX_CS dd ?
00000051 _ZtlSecureTear_nINT dw 2 dup(? )
00000055 _ZtlSecureTear_nINT_CS dd ?
00000059 _ZtlSecureTear_nLUK dw 2 dup(? )
0000005D _ZtlSecureTear_nLUK_CS dd ?
00000061 _ZtlSecureTear_nHP dd 2 dup(? )
00000069 _ZtlSecureTear_nHP_CS dd ?
0000006D _ZtlSecureTear_nMHP dd 2 dup(? )
00000075 _ZtlSecureTear_nMHP_CS dd ?
00000079 _ZtlSecureTear_nMP dd 2 dup(? )
00000081 _ZtlSecureTear_nMP_CS dd ?
00000085 _ZtlSecureTear_nMMP dd 2 dup(? )
0000008D _ZtlSecureTear_nMMP_CS dd ?
00000091 _ZtlSecureTear_nAP dw 2 dup(? )
00000095 _ZtlSecureTear_nAP_CS dd ?
00000099 _ZtlSecureTear_nSP dw 2 dup(? )
0000009D _ZtlSecureTear_nSP_CS dd ?
000000A1 _ZtlSecureTear_nEXP dd 2 dup(? )
000000A9 _ZtlSecureTear_nEXP_CS dd ?
000000AD _ZtlSecureTear_nPOP dw 2 dup(? )
000000B1 _ZtlSecureTear_nPOP_CS dd ?
000000B5 _ZtlSecureTear_nMoney dd 2 dup(? )
000000BD _ZtlSecureTear_nMoney_CS dd ?
000000C1 _ZtlSecureTear_nTempEXP dd 2 dup(? )
000000C9 _ZtlSecureTear_nTempEXP_CS dd ?
000000CD extendSP        ExtendSP ?
000000E1 _ZtlSecureTear_dwPosMap dd 2 dup(? )
000000E9 _ZtlSecureTear_dwPosMap_CS dd ?
000000ED nPortal         db ?
000000EE nCheckSum       dd ?
000000F2 nItemCountCheckSum db ?
000000F3 nPlaytime       dd ?
000000F7 nSubJob         dw ?
000000F9 GW_CharacterStat ends
*/
struct GW_CharacterStat {
    unsigned int dwCharacterID;
    char sCharacterName[13];
    unsigned __int8 nGender;
    unsigned __int8 nSkin;
    int nFace;
    int nHair;
    _LARGE_INTEGER liPetLockerSN[3];
    unsigned __int8 _ZtlSecureTear_nLevel[2];
    unsigned int _ZtlSecureTear_nLevel_CS;
    __int16 _ZtlSecureTear_nJob[2];
    unsigned int _ZtlSecureTear_nJob_CS;
    __int16 _ZtlSecureTear_nSTR[2];
    unsigned int _ZtlSecureTear_nSTR_CS;
    __int16 _ZtlSecureTear_nDEX[2];
    unsigned int _ZtlSecureTear_nDEX_CS;
    __int16 _ZtlSecureTear_nINT[2];
    unsigned int _ZtlSecureTear_nINT_CS;
    __int16 _ZtlSecureTear_nLUK[2];
    unsigned int _ZtlSecureTear_nLUK_CS;
    int _ZtlSecureTear_nHP[2];
    unsigned int _ZtlSecureTear_nHP_CS;
    int _ZtlSecureTear_nMHP[2];
    unsigned int _ZtlSecureTear_nMHP_CS;
    int _ZtlSecureTear_nMP[2];
    unsigned int _ZtlSecureTear_nMP_CS;
    int _ZtlSecureTear_nMMP[2];
    unsigned int _ZtlSecureTear_nMMP_CS;
    __int16 _ZtlSecureTear_nAP[2];
    unsigned int _ZtlSecureTear_nAP_CS;
    __int16 _ZtlSecureTear_nSP[2];
    unsigned int _ZtlSecureTear_nSP_CS;
    int _ZtlSecureTear_nEXP[2];
    unsigned int _ZtlSecureTear_nEXP_CS;
    __int16 _ZtlSecureTear_nPOP[2];
    unsigned int _ZtlSecureTear_nPOP_CS;
    int _ZtlSecureTear_nMoney[2];
    unsigned int _ZtlSecureTear_nMoney_CS;
    int _ZtlSecureTear_nTempEXP[2];
    unsigned int _ZtlSecureTear_nTempEXP_CS;
    ExtendSP extendSP;
    unsigned int _ZtlSecureTear_dwPosMap[2];
    unsigned int _ZtlSecureTear_dwPosMap_CS;
    unsigned __int8 nPortal;
    int nCheckSum;
    unsigned __int8 nItemCountCheckSum;
    int nPlaytime;
    __int16 nSubJob;
};