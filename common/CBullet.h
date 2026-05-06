class  CBullet : ZRefCounted
{
    _com_ptr_t<_com_IIID<IWzVector2D,&IID_IUnknown> > m_pOrigin;
    _com_ptr_t<_com_IIID<IWzGr2DLayer,&IID_IUnknown> > m_pLayer;
    int m_bClockwise;
    int m_tStart;
    int m_tEnd;
    tagPOINT m_ptStart;
    tagPOINT m_ptEnd;
    _com_ptr_t<_com_IIID<IWzVector2D,&IID_IUnknown> > m_pVecTarget;
};
