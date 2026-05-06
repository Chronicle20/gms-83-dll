#pragma once

template <typename TBullet>
class BulletContainer
{
public:
    ZList<ZRef<TBullet>> m_lList;
};