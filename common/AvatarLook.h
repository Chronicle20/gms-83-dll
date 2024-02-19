#pragma once

/*
00000000 AvatarLook      struc ; (sizeof=0x205, copyof_1995)
00000000 baseclass_0     ZRefCounted ?
0000000C nGender         db ?
0000000D nSkin           dd ?
00000011 nFace           dd ?
00000015 nWeaponStickerID dd ?
00000019 anHairEquip     dd 60 dup(?)
00000109 anUnseenEquip   dd 60 dup(?)
000001F9 anPetID         dd 3 dup(?) 
00000205 AvatarLook      ends
*/
struct AvatarLook : ZRefCounted
{
    unsigned __int8 nGender;
    int nSkin;
    int nFace;
    int nWeaponStickerID;
    int anHairEquip[60];
    int anUnseenEquip[60];
    int nPetID[3];
};