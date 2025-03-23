class CONFIG_SYSOPT
{
    int nSysOpt_Video;
    int nSysOpt_BGMVol;
    int bSysOpt_BGMMute;
    int nSysOpt_SEVol;
    int bSysOpt_SEMute;
    int nSysOpt_ScreenShot;
    int nSysOpt_MouseSpeed;
    int nSysOpt_HPFlash;
    int nSysOpt_MPFlash;
    int bSysOpt_Tremble;
    int nSysOpt_MobInfo;
#if defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95
    int bSysOpt_LargeScreen;
    int bSysOpt_WindowedMode;
#endif
    int bSysOpt_Minimap_Normal;
};
