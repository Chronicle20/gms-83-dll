#pragma once

// PartyRaidTeam: a 4-byte enum in IDA local types (Type #330), but no enum members
// were exposed by IDA's list_local_types output -- keeping as typedef-to-int preserves
// the 4-byte slot in CWvsContext::m_nTeamForPartyRaid.
// TODO: replace with `enum PartyRaidTeam : int { ... };` once values are known
// (look at CField_PartyRaid::SetTeam or similar accessors for the constants).
typedef int PartyRaidTeam;
