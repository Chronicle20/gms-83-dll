#include "CRand32.h"

struct CalcDamage {
    CRand32 m_RndGenForCharacter;
    CRand32 m_RndForCheckDamageMiss;
    CRand32 m_RndForMortalBlow;
    CRand32 m_RndForSummoned;
    CRand32 m_RndForMob;
    CRand32 m_RndGenForMob;
};
