struct DIDEVICEINSTANCEA
{
    unsigned int dwSize;
    _GUID guidInstance;
    _GUID guidProduct;
    unsigned int dwDevType;
    char tszInstanceName[260];
    char tszProductName[260];
    _GUID guidFFDriver;
    unsigned __int16 wUsagePage;
    unsigned __int16 wUsage;
};
