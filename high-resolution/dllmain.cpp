/*
 This file is part of GMS-83-DLL.

 GMS-83-DLL is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

 GMS-83-DLL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
 */

#include "pch.h"

#include <memedit.h>
#include "logger.h"
#include <fstream>
#include <string>
#include <map>
#include <sstream>

std::map<std::string, std::string> parseINI(const std::string &filePath) {
    std::map<std::string, std::string> iniData;

    std::ifstream inputFile(filePath);
    if (!inputFile.is_open()) {
        Log("Failed to open INI file: %s", filePath.c_str());
        return iniData;
    }

    std::string line;
    std::string currentSection;
    while (std::getline(inputFile, line)) {
        if (line.empty()) continue;

        if (line[0] == '[' && line.back() == ']') {
            currentSection = line.substr(1, line.size() - 2);
        } else {
            size_t pos = line.find('=');
            if (pos != std::string::npos) {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);
                iniData[currentSection + "." + key] = value;
            }
        }
    }

    inputFile.close();
    return iniData;
}

VOID __stdcall MainProc() {
    Log("[high-resolution] : Loading .ini");
    std::map<std::string, std::string> iniData = parseINI("edits/high-resolution.ini");
    if (iniData.empty()) {
        return;
    }
    int m_nGameHeight = std::stoi(iniData["Main.Height"]);
    int m_nGameWidth = std::stoi(iniData["Main.Width"]);
    Log("[high-resolution] : Setting game resolution to [%dx%d].", m_nGameWidth, m_nGameHeight);

    //push 600
    MemEdit::WriteInt(C_WVS_APP_INITIALIZE_GR2D + 0xE2 + 1, m_nGameHeight);
    //push 800
    MemEdit::WriteInt(C_WVS_APP_INITIALIZE_GR2D + 0xE8 + 1, m_nGameWidth);

    //push 600
    MemEdit::WriteInt(C_WVS_APP_CREATE_MAIN_WINDOW + 0x102 + 1, m_nGameHeight);
    //push 800
    MemEdit::WriteInt(C_WVS_APP_CREATE_MAIN_WINDOW + 0x109 + 1, m_nGameWidth);

    //push 800
    MemEdit::WriteInt(C_SLIDE_NOTICE +0x3E + 1, m_nGameWidth);
    //push 800
    MemEdit::WriteInt(C_SLIDE_NOTICE_SET_MSG +0x5B9 + 1, m_nGameWidth);
    //push 800
    MemEdit::WriteInt(C_SLIDE_NOTICE_SET_MSG +0x709 + 2, m_nGameWidth);

    //push 600
    MemEdit::WriteInt(C_SLIDE_NOTICE_ON_CREATE +0x33 + 1, m_nGameHeight);
    //push 800 ; CWnd::GetCanvas
    MemEdit::WriteInt(C_SLIDE_NOTICE_ON_CREATE +0x38 + 1, m_nGameWidth);

    //push -300
    MemEdit::WriteInt(C_WND_MAN +0x243 + 1, floor(-m_nGameHeight / 2));
    //push -400 ;
    MemEdit::WriteInt(C_WND_MAN +0x249 + 1, floor(-m_nGameWidth / 2));

    //lea eax,[esi+eax-600]
    MemEdit::WriteInt(C_WND_ON_MOVE_WND +0x1199 + 3, m_nGameHeight);
    //mov ecx,800
    MemEdit::WriteInt(C_WND_ON_MOVE_WND +0x1205 + 1, m_nGameWidth);
    //lea eax,[ecx+eax-800]
    MemEdit::WriteInt(C_WND_ON_MOVE_WND +0x1311 + 3, m_nGameWidth);
    //mov ecx,600	; IWzVector2D::RelMove
    MemEdit::WriteInt(C_WND_ON_MOVE_WND +0x137B + 1, m_nGameHeight);

    //mov edi,600
    MemEdit::WriteInt(C_ANIMATION_DISPLAYER_REGISTER_FADE_IN_OUT_ANIMATION +0xAE + 1, m_nGameHeight);
    //mov esi,800 ; CreateWnd
    MemEdit::WriteInt(C_ANIMATION_DISPLAYER_REGISTER_FADE_IN_OUT_ANIMATION +0xB4 + 1, m_nGameWidth);
    //push -300
    MemEdit::WriteInt(C_ANIMATION_DISPLAYER_REGISTER_FADE_IN_OUT_ANIMATION +0x1E4 + 1, floor(-m_nGameHeight / 2));
    //push -400 ; RelMove?
    MemEdit::WriteInt(C_ANIMATION_DISPLAYER_REGISTER_FADE_IN_OUT_ANIMATION +0x1E9 + 1, floor(-m_nGameWidth / 2));

    //mov [ebx+2364],300
    MemEdit::WriteInt(C_INPUT_SYSTEM_INIT + 0x1E3 + 6, floor(m_nGameHeight / 2));
    //mov [esi],400
    MemEdit::WriteInt(C_INPUT_SYSTEM_INIT + 0x1DD + 2, floor(m_nGameWidth / 2));
    //mov eax,600
    MemEdit::WriteInt(C_INPUT_SYSTEM_SET_CURSOR_POS +0x2A + 1, m_nGameHeight);
    //mov eax,800
    MemEdit::WriteInt(C_INPUT_SYSTEM_SET_CURSOR_POS +0x11 + 1, m_nGameWidth);
    //mov ecx,600
    MemEdit::WriteInt(C_INPUT_SYSTEM_UPDATE_MOUSE +0x124 + 1, m_nGameHeight);
    //mov ecx,800
    MemEdit::WriteInt(C_INPUT_SYSTEM_UPDATE_MOUSE +0x10B + 1, m_nGameWidth);
    //push -300
    MemEdit::WriteInt(C_INPUT_SYSTEM_SET_CURSOR_VECTOR_POS +0x92 + 2, floor(-m_nGameHeight / 2));
    //push -400
    MemEdit::WriteInt(C_INPUT_SYSTEM_SET_CURSOR_VECTOR_POS +0x9E + 2, floor(-m_nGameWidth / 2));

    //mov eax,599
    MemEdit::WriteInt(C_UI_TOOLTIP_MAKE_LAYER +0x19E + 1, m_nGameWidth - 1);
    //mov eax,799
    MemEdit::WriteInt(C_UI_TOOLTIP_MAKE_LAYER +0x18B + 1, m_nGameWidth - 1);

    //lea eax,[eax+ecx-797] ;
    MemEdit::WriteInt(C_TEMPORARY_STAT_VIEW_SHOW_TOOLTIP +0x48 + 3, -m_nGameWidth + 3);

    //sub ebx,277 ;
    MemEdit::WriteInt(C_TEMPORARY_STAT_VIEW_ADJUST_POSITION +0xE7 + 2, (m_nGameHeight / 2) - 23);
    //lea eax,[eax+esi+397] ;
    MemEdit::WriteInt(C_TEMPORARY_STAT_VIEW_ADJUST_POSITION +0x105 + 3, (m_nGameWidth / 2) - 3);
    //sub ebx,277 ;
    MemEdit::WriteInt(C_TEMPORARY_STAT_VIEW_ADJUST_POSITION +0x1F0 + 2, (m_nGameHeight / 2) - 23);
    //lea eax,[eax+esi+397] ;
    MemEdit::WriteInt(C_TEMPORARY_STAT_VIEW_ADJUST_POSITION +0x20E + 3, (m_nGameWidth / 2) - 3);

    //mov eax,800 ; RelMove?
    MemEdit::WriteInt(C_UI_TOOLTIP_SET_TOOLTIP_EQUIP_2 +0x1E1 + 1, m_nGameWidth);
    //mov eax,600
    MemEdit::WriteInt(C_UI_TOOLTIP_SET_TOOLTIP_EQUIP_2 +0x1FD + 1, m_nGameHeight);

    //mov ecx,600
    MemEdit::WriteInt(C_UI_CONTEXT_MENU +0x145 + 1, m_nGameHeight);
    //cmp edi,800
    MemEdit::WriteInt(C_UI_CONTEXT_MENU +0x15A + 2, m_nGameWidth);
    //mov edx,700 ;
    MemEdit::WriteInt(C_UI_CONTEXT_MENU +0x162 + 1, m_nGameWidth - 100);

    //mov edx,600
    MemEdit::WriteInt(C_UTIL_DLG_EX +0x47 + 1, m_nGameHeight);
    //mov edx,800 ;
    MemEdit::WriteInt(C_UTIL_DLG_EX +0x3A + 1, m_nGameWidth);

    //mov eax,600
    MemEdit::WriteInt(C_REGISTER_SALE_ENTRY_DLG_ON_CREATE +0x2D5 + 1, m_nGameHeight);
    //mov eax,800 ; RelMove?
    MemEdit::WriteInt(C_REGISTER_SALE_ENTRY_DLG_ON_CREATE +0x2E5 + 1, m_nGameWidth);
    //mov eax,600
    MemEdit::WriteInt(C_REGISTER_SALE_ENTRY_DLG_ON_CREATE +0x2D5 + 1, m_nGameHeight);
    //mov eax,800 ; RelMove?
    MemEdit::WriteInt(C_REGISTER_SALE_ENTRY_DLG_ON_CREATE +0x2E5 + 1, m_nGameWidth);

    // Other dialogs OnCreate that need confirmation.
//    MemEdit::WriteInt(0x005AADAA + 1, m_nGameHeight);//mov eax,600
//    MemEdit::WriteInt(0x005AADBA + 1, m_nGameWidth);    //mov eax,800 ; RelMove?
//    MemEdit::WriteInt(0x005ABC65 + 1, m_nGameHeight);//mov eax,600
//    MemEdit::WriteInt(0x005ABC75 + 1, m_nGameWidth);    //mov eax,800 ; RelMove?
//    MemEdit::WriteInt(0x005ACB29 + 1, m_nGameHeight);//mov eax,600
//    MemEdit::WriteInt(0x005ACB39 + 1, m_nGameWidth);    //mov eax,800 ; RelMove?
//    MemEdit::WriteInt(0x005C187E + 1, m_nGameHeight);//mov eax,600
//    MemEdit::WriteInt(0x005C188E + 1, m_nGameWidth);    //mov eax,800 ; RelMove?
//    MemEdit::WriteInt(0x005C2D62 + 1, m_nGameHeight);//mov eax,600
//    MemEdit::WriteInt(0x005C2D72 + 1, m_nGameWidth);    //mov eax,800 ; RelMove?
//    MemEdit::WriteInt(0x007CF48F + 1, m_nGameHeight);//mov eax,600 ;
//    MemEdit::WriteInt(0x007CF49D + 1, m_nGameWidth);    //mov eax,800 ; IWzVector2D::RelMove
//    MemEdit::WriteInt(0x008A12F4 + 1, m_nGameHeight);//mov eax,600 ;
//    MemEdit::WriteInt(0x008A1302 + 1, m_nGameWidth);    //mov eax,800 ; IWzVector2D::RelMove
//    MemEdit::WriteInt(0x0062FC4A + 1, m_nGameHeight);//push 600
//    MemEdit::WriteInt(0x0062FC4F + 1, m_nGameWidth);    //push 800 ; IWzGr2DLayer::Getcanvas
//    MemEdit::WriteInt(0x0062FE63 + 1, m_nGameHeight);//push 600
//    MemEdit::WriteInt(0x0062FE68 + 1, m_nGameWidth);    //push 800 ; IWzGr2DLayer::Getcanvas
//    MemEdit::WriteInt(0x0046B85C + 1, m_nGameHeight);//mov eax,600
//    MemEdit::WriteInt(0x0046B86A + 1, m_nGameWidth);    //mov eax,800 ; IWzVector2D::RelMove

    //mov ecx,600
    MemEdit::WriteInt(C_UI_ART_SPEAKER_SAMPLE +0x1B8 + 1, m_nGameHeight);
    //cmp edi,800
    MemEdit::WriteInt(C_UI_ART_SPEAKER_SAMPLE +0x1CD + 2, m_nGameWidth);
    //mov edx,700 ;
    MemEdit::WriteInt(C_UI_ART_SPEAKER_SAMPLE +0x1D5 + 1, m_nGameWidth - 100);

    //CCtrlSelector::OnMouseButton
//    MemEdit::WriteInt(0x004D9BD1 + 1, m_nGameWidth);    //push 800
//    MemEdit::WriteInt(0x004D9C37 + 1, m_nGameWidth);    //push 800
//    MemEdit::WriteInt(0x004D9C84 + 1, m_nGameWidth);    //push 800 ; StringPool#1443 (BtMouseCilck)


    // ?Init@CField_LimitedView@@UAEXPAX@Z
//    MemEdit::WriteInt(0x0055B808 + 1, m_nGameHeight);//push 600
//    MemEdit::WriteInt(0x0055B80D + 1, m_nGameWidth);    //mov edi,800
//    MemEdit::WriteInt(0x0055B884 + 1, m_nGameWidth);    //push 600 ; RelMove?

//    //TODO explain these but they look good
//    MemEdit::WriteInt(0x004CC160 + 1, m_nGameWidth);    //mov [ebp-16],800 ; CreateWnd
//    MemEdit::WriteInt(0x004CC2C5 + 2, m_nGameHeight);//cmp ecx,600
//    MemEdit::WriteInt(0x004CC2B0 + 1, m_nGameWidth);    //mov eax,800 ; CreateWnd
//    MemEdit::WriteInt(0x004D59B2 + 1, m_nGameHeight);//mov eax,800
//    MemEdit::WriteInt(0x004D599D + 1, m_nGameWidth);    //mov eax,800 ; CreateWnd
//
//    // Something MonsterBook related.
//    MemEdit::WriteInt(0x0085F36C + 2, m_nGameWidth);    //cmp edx,800
//    MemEdit::WriteInt(0x0085F374 + 1, m_nGameWidth - 80);    //mov ecx,720 ; CreateDlg
}

// dll entry point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH: {
            DisableThreadLibraryCalls(hModule);
            CreateThread(nullptr,
                         0, (LPTHREAD_START_ROUTINE) &MainProc, nullptr, 0, nullptr);
            break;
        }
    }
    return TRUE;
}