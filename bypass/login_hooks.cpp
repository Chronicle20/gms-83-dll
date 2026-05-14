#include "pch.h"

#include "login_hooks.h"

#include "hooker.h"
#include "logger.h"

typedef INT(__thiscall* _CLogin__SendCheckPasswordPacket_t)(CLogin* pThis, char* sID, char* sPasswd);

INT __fastcall CLogin__SendCheckPasswordPacket_Hook(CLogin* pThis, PVOID edx,
                                                    char* sID, char* sPasswd) {
    Log("CLogin::SendCheckPasswordPacket. ID [%s]. bRequestSent [%d].",
        sID, pThis->m_bRequestSent);
    if (pThis->m_bRequestSent) {
        return 0;
    }
    pThis->m_bRequestSent = 1;
    pThis->m_WorldItem.RemoveAll();
    pThis->m_aBalloon.RemoveAll();

    auto systemInfo = CSystemInfo();
    systemInfo.Init();
    auto cOutPacket = COutPacket(1);

    ZXString<char> tempString = ZXString<char>(sID, 0xFFFFFFFF);
    cOutPacket.EncodeStr(tempString);

    ZXString<char> tempString2 = ZXString<char>(sPasswd, 0xFFFFFFFF);
    cOutPacket.EncodeStr(tempString2);

    cOutPacket.EncodeBuffer(systemInfo.GetMachineId(), 16);
    int gameRoomClient = systemInfo.GetGameRoomClient();
    Log("GRC %d, GRC PTR %d", gameRoomClient, &gameRoomClient);
    cOutPacket.Encode4(gameRoomClient);
    cOutPacket.Encode1(CWvsApp::GetInstance()->m_nGameStartMode);
    cOutPacket.Encode1(0);
    cOutPacket.Encode1(0);
#if defined(REGION_GMS)
    cOutPacket.Encode4(CConfig::GetInstance()->GetPartnerCode());
#endif
    CClientSocket::GetInstance()->SendPacket(&cOutPacket);
#if defined(REGION_JMS)
    CWvsContext::GetInstance()->unk1.Assign(sID, 0xFFFFFFFF);
#endif
    CUITitle* cuiTitle = CUITitle::GetInstance();
    if (cuiTitle) {
        cuiTitle->ClearToolTip();
    }
    // m_aSendBuff.RemoveAll() removed per §4.3 — dtor handles it.
    return 1;
}

BOOL InstallLoginHooks() {
    HOOKTYPEDEF_C(CLogin__SendCheckPasswordPacket);
    INITMAPLEHOOK_OR_RETURN(_CLogin__SendCheckPasswordPacket, _CLogin__SendCheckPasswordPacket_t,
                            CLogin__SendCheckPasswordPacket_Hook, C_LOGIN_SEND_CHECK_PASSWORD_PACKET);
    return TRUE;
}
