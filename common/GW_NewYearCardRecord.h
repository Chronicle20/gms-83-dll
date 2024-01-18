#include "ZArray.h"

struct GW_NewYearCardRecord {
    unsigned int m_dwSN;
    unsigned int m_dwSenderID;
    char m_sSenderName[13];
    int m_bSenderDiscardCard;
    _FILETIME m_dateSent;
    unsigned int m_dwReceiverID;
    char m_sReceiverName[13];
    int m_bReceiverDiscardCard;
    int m_bReceiverReceivedCard;
    _FILETIME m_dateReceived;
    char m_sContent[121];
};
