#pragma once

/*
00000000 AvatarData      struc ; (sizeof=0x2FE, copyof_4916)
00000000 characterStat   GW_CharacterStat ?
000000F9 avatarLook      AvatarLook ?
000002FE AvatarData      ends
*/
struct AvatarData
{
    GW_CharacterStat characterStat;
    AvatarLook avatarLook;
};