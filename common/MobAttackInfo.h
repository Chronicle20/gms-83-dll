#pragma once

class MobAttackInfo : ZRefCounted {
    int nType;
    int bInactive;
    int nConMP;
    int nPAD;
    int bMagicAttack;
    int nBulletNumber;
    int nMagicElemAttr;
    int bJumpAttack;
    int ___u9[10];
    int nBulletSpeed;
    int bDeadlyAttack;
    int bTremble;
    _bstr_t sEffect;
    _bstr_t sHit;
    _bstr_t sBall;
    _bstr_t sAreaWarning;
    int bHitAttach;
    int tEffectAfter;
    int tAttackAfter;
    int bDoFirst;
    int nMPBurn;
    int bKnockBack;
    int bFacingAttatch;
    int tRandDelayAttack;
    int bRush;
    int bSpeicalAttack;
};
