// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

//#ifndef PCH_H
//#define PCH_H

#include "framework.h"

#include <atlstr.h>

#include "comip.h"
#include "comdef.h"

#include "ZXString.h"
#include "ZArray.h"

#include "IDraggable.h"
#include "IUIMsgHandler.h"
#include "IWzSerialize.h"
#include "IWzShape2D.h"
#include "IWzVector2D.h"
#include "IWzGr2DLayer.h"
#include "IWzGr2D.h"
#include "IWzCanvas.h"

#include "CActionMan.h"
#include "CClientSocket.h"
#include "CConfig.h"
#include "CLogin.h"
#include "COutPacket.h"
#include "CSystemInfo.h"
#include "CUITitle.h"
#include "CWndMan.h"
#include "CWvsApp.h"
#include "CWvsContext.h"
#include "TSingleton.h"