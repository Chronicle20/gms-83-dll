#include "pch.h"

void CBattleRecordMan::CreateInstance() {
    Log("CBattleRecordMan::CreateInstance");
    reinterpret_cast<void (__cdecl *)()>(C_BATTLE_RECORD_MAN_CREATE_INSTANCE)();
}