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
    MemEdit::WriteInt(C_SLIDE_NOTICE + 0x3E + 1, m_nGameWidth);
    //push 800
    MemEdit::WriteInt(C_SLIDE_NOTICE_SET_MSG + 0x5B9 + 1, m_nGameWidth);
    //push 800
    MemEdit::WriteInt(C_SLIDE_NOTICE_SET_MSG + 0x709 + 2, m_nGameWidth);

    //push 600
    MemEdit::WriteInt(C_SLIDE_NOTICE_ON_CREATE + 0x33 + 1, m_nGameHeight);
    //push 800 ; CWnd::GetCanvas
    MemEdit::WriteInt(C_SLIDE_NOTICE_ON_CREATE + 0x38 + 1, m_nGameWidth);

    //push -300
    MemEdit::WriteInt(C_WND_MAN + 0x243 + 1, floor(-m_nGameHeight / 2));
    //push -400 ;
    MemEdit::WriteInt(C_WND_MAN + 0x249 + 1, floor(-m_nGameWidth / 2));

    //lea eax,[esi+eax-600]
    MemEdit::WriteInt(C_WND_ON_MOVE_WND + 0x1199 + 3, m_nGameHeight);
    //mov ecx,800
    MemEdit::WriteInt(C_WND_ON_MOVE_WND + 0x1205 + 1, m_nGameWidth);
    //lea eax,[ecx+eax-800]
    MemEdit::WriteInt(C_WND_ON_MOVE_WND + 0x1311 + 3, m_nGameWidth);
    //mov ecx,600	; IWzVector2D::RelMove
    MemEdit::WriteInt(C_WND_ON_MOVE_WND + 0x137B + 1, m_nGameHeight);

    //mov edi,600
    MemEdit::WriteInt(C_ANIMATION_DISPLAYER_REGISTER_FADE_IN_OUT_ANIMATION + 0xAE + 1, m_nGameHeight);
    //mov esi,800 ; CreateWnd
    MemEdit::WriteInt(C_ANIMATION_DISPLAYER_REGISTER_FADE_IN_OUT_ANIMATION + 0xB4 + 1, m_nGameWidth);
    //push -300
    MemEdit::WriteInt(C_ANIMATION_DISPLAYER_REGISTER_FADE_IN_OUT_ANIMATION + 0x1E4 + 1, floor(-m_nGameHeight / 2));
    //push -400 ; RelMove?
    MemEdit::WriteInt(C_ANIMATION_DISPLAYER_REGISTER_FADE_IN_OUT_ANIMATION + 0x1E9 + 1, floor(-m_nGameWidth / 2));

    //mov [ebx+2364],300
    MemEdit::WriteInt(C_INPUT_SYSTEM_INIT + 0x1E3 + 6, floor(m_nGameHeight / 2));
    //mov [esi],400
    MemEdit::WriteInt(C_INPUT_SYSTEM_INIT + 0x1DD + 2, floor(m_nGameWidth / 2));
    //mov eax,600
    MemEdit::WriteInt(C_INPUT_SYSTEM_SET_CURSOR_POS + 0x2A + 1, m_nGameHeight);
    //mov eax,800
    MemEdit::WriteInt(C_INPUT_SYSTEM_SET_CURSOR_POS + 0x11 + 1, m_nGameWidth);
    //mov ecx,600
    MemEdit::WriteInt(C_INPUT_SYSTEM_UPDATE_MOUSE + 0x124 + 1, m_nGameHeight);
    //mov ecx,800
    MemEdit::WriteInt(C_INPUT_SYSTEM_UPDATE_MOUSE + 0x10B + 1, m_nGameWidth);
    //push -300
    MemEdit::WriteInt(C_INPUT_SYSTEM_SET_CURSOR_VECTOR_POS + 0x92 + 2, floor(-m_nGameHeight / 2));
    //push -400
    MemEdit::WriteInt(C_INPUT_SYSTEM_SET_CURSOR_VECTOR_POS + 0x9E + 2, floor(-m_nGameWidth / 2));

    //mov eax,599
    MemEdit::WriteInt(C_UI_TOOLTIP_MAKE_LAYER + 0x19E + 1, m_nGameWidth - 1);
    //mov eax,799
    MemEdit::WriteInt(C_UI_TOOLTIP_MAKE_LAYER + 0x18B + 1, m_nGameWidth - 1);

    //lea eax,[eax+ecx-797] ;
    MemEdit::WriteInt(C_TEMPORARY_STAT_VIEW_SHOW_TOOLTIP + 0x48 + 3, -m_nGameWidth + 3);
    //lea eax,[eax+edx-797]
    MemEdit::WriteInt(C_TEMPORARY_STAT_VIEW_FIND_ICON + 0x32 + 3, -m_nGameWidth + 3);

    //sub ebx,277
    MemEdit::WriteInt(C_TEMPORARY_STAT_VIEW_ADJUST_POSITION + 0xE7 + 2, (m_nGameHeight / 2) - 23);
    //lea eax,[eax+esi+397] ;
    MemEdit::WriteInt(C_TEMPORARY_STAT_VIEW_ADJUST_POSITION + 0x105 + 3, (m_nGameWidth / 2) - 3);
    //sub ebx,277
    MemEdit::WriteInt(C_TEMPORARY_STAT_VIEW_ADJUST_POSITION + 0x1F0 + 2, (m_nGameHeight / 2) - 23);
    //lea eax,[eax+esi+397] ;
    MemEdit::WriteInt(C_TEMPORARY_STAT_VIEW_ADJUST_POSITION + 0x20E + 3, (m_nGameWidth / 2) - 3);

    //mov eax,800
    MemEdit::WriteInt(C_UI_TOOLTIP_SET_TOOLTIP_EQUIP_2 + 0x1E1 + 1, m_nGameWidth);
    //mov eax,600
    MemEdit::WriteInt(C_UI_TOOLTIP_SET_TOOLTIP_EQUIP_2 + 0x1FD + 1, m_nGameHeight);

    //mov ecx,600
    MemEdit::WriteInt(C_UI_CONTEXT_MENU + 0x145 + 1, m_nGameHeight);
    //cmp edi,800
    MemEdit::WriteInt(C_UI_CONTEXT_MENU + 0x15A + 2, m_nGameWidth);
    //mov edx,700
    MemEdit::WriteInt(C_UI_CONTEXT_MENU + 0x162 + 1, m_nGameWidth - 100);

    //mov edx,600
    MemEdit::WriteInt(C_UTIL_DLG_EX + 0x47 + 1, m_nGameHeight);
    //mov edx,800
    MemEdit::WriteInt(C_UTIL_DLG_EX + 0x3A + 1, m_nGameWidth);

    //mov eax,600
    MemEdit::WriteInt(C_REGISTER_SALE_ENTRY_DLG_ON_CREATE + 0x2D5 + 1, m_nGameHeight);
    //mov eax,800
    MemEdit::WriteInt(C_REGISTER_SALE_ENTRY_DLG_ON_CREATE + 0x2E5 + 1, m_nGameWidth);
    //mov eax,600
    MemEdit::WriteInt(C_REGISTER_SALE_ENTRY_DLG_ON_CREATE + 0x2D5 + 1, m_nGameHeight);
    //mov eax,800
    MemEdit::WriteInt(C_REGISTER_SALE_ENTRY_DLG_ON_CREATE + 0x2E5 + 1, m_nGameWidth);

    // Other dialogs OnCreate that need confirmation.
    //5AAACC

//    MemEdit::WriteInt(0x005AADAA + 1, m_nGameHeight);//mov eax,600
//    MemEdit::WriteInt(0x005AADBA + 1, m_nGameWidth);    //mov eax,800 ; RelMove?

    //mov eax,600
    MemEdit::WriteInt(C_WORLD_MAP_DLG_ON_CREATE + 0x2DE + 1, m_nGameHeight);
    //mov eax,800
    MemEdit::WriteInt(C_WORLD_MAP_DLG_ON_CREATE + 0x2EE + 1, m_nGameWidth);

    //mov eax,600
    MemEdit::WriteInt(C_REGISTER_AUCTION_ENTRY_DLG_ON_CREATE + 0x2DE + 1, m_nGameHeight);
    //mov eax,800
    MemEdit::WriteInt(C_REGISTER_AUCTION_ENTRY_DLG_ON_CREATE + 0x2EE + 1, m_nGameWidth);

    //mov eax,600
    MemEdit::WriteInt(C_ITC_WND_ITEM_DLG_ON_CREATE + 0x2DE + 1, m_nGameHeight);
    //mov eax,800
    MemEdit::WriteInt(C_ITC_WND_ITEM_DLG_ON_CREATE + 0x2EE + 1, m_nGameWidth);

    //mov eax,600
    MemEdit::WriteInt(C_ITC_BID_AUCTION_DLG_ON_CREATE + 0x2DE + 1, m_nGameHeight);
    //mov eax,800
    MemEdit::WriteInt(C_ITC_BID_AUCTION_DLG_ON_CREATE + 0x2EE + 1, m_nGameWidth);

    //mov eax,600
    MemEdit::WriteInt(C_UI_ADMIN_SHOP_WISH_LIST_SET_LAYER + 0x2F4 + 1, m_nGameHeight);
    //mov eax,800
    MemEdit::WriteInt(C_UI_ADMIN_SHOP_WISH_LIST_SET_LAYER + 0x302 + 1, m_nGameWidth);

    //mov eax,600 ;
    MemEdit::WriteInt(0x008A1000 + 0x2F4 + 1, m_nGameHeight);
    //mov eax,800 ; IWzVector2D::RelMove
    MemEdit::WriteInt(0x008A1000 + 0x302 + 1, m_nGameWidth);

    //push 600
    MemEdit::WriteInt(C_LOGO_DRAW_NX_LOGO + 0x61 + 1, m_nGameHeight);
    //push 800 ; IWzGr2DLayer::Getcanvas
    MemEdit::WriteInt(C_LOGO_DRAW_NX_LOGO + 0x66 + 1, m_nGameWidth);

    //push 600
    MemEdit::WriteInt(C_LOGO_DRAW_WZ_LOGO + 0x61 + 1, m_nGameHeight);
    //push 800 ; IWzGr2DLayer::Getcanvas
    MemEdit::WriteInt(C_LOGO_DRAW_WZ_LOGO + 0x66 + 1, m_nGameWidth);

    //mov eax,600
    MemEdit::WriteInt(C_CONFIRM_PURCHASE_DLG_SET_LAYER + 0x325 + 1, m_nGameHeight);
    //mov eax,800
    MemEdit::WriteInt(C_CONFIRM_PURCHASE_DLG_SET_LAYER + 0x333 + 1, m_nGameWidth);

    //mov ecx,600
    MemEdit::WriteInt(C_UI_ART_SPEAKER_SAMPLE + 0x1B8 + 1, m_nGameHeight);
    //cmp edi,800
    MemEdit::WriteInt(C_UI_ART_SPEAKER_SAMPLE + 0x1CD + 2, m_nGameWidth);
    //mov edx,700 ;
    MemEdit::WriteInt(C_UI_ART_SPEAKER_SAMPLE + 0x1D5 + 1, m_nGameWidth - 100);

    //push 800
    MemEdit::WriteInt(C_CTRL_SELECTOR_ON_MOUSE_BUTTON + 0xF6 + 1, m_nGameWidth);
    //push 800
    MemEdit::WriteInt(C_CTRL_SELECTOR_ON_MOUSE_BUTTON + 0x15C + 1, m_nGameWidth);
    //push 800 ; StringPool#1443 (BtMouseCilck)
    MemEdit::WriteInt(C_CTRL_SELECTOR_ON_MOUSE_BUTTON + 0x1A9 + 1, m_nGameWidth);

    //push 600
    MemEdit::WriteInt(C_FIELD_LIMITED_VIEW_INIT + 0xB6 + 1, m_nGameHeight);
    //mov edi,800
    MemEdit::WriteInt(C_FIELD_LIMITED_VIEW_INIT + 0xBB + 1, m_nGameWidth);
    //push 600 ; RelMove?
    MemEdit::WriteInt(C_FIELD_LIMITED_VIEW_INIT + 0x132 + 1, m_nGameWidth);

    //mov [ebp-16],800 ; CreateWnd
    MemEdit::WriteInt(C_CTRL_EDIT_CREATE_IME_CAND_WND + 0x199 + 1, m_nGameWidth);
    //cmp ecx,600
    MemEdit::WriteInt(C_CTRL_EDIT_CREATE_IME_CAND_WND + 0x2FE + 2, m_nGameHeight);
    //mov eax,800 ; CreateWnd
    MemEdit::WriteInt(C_CTRL_EDIT_CREATE_IME_CAND_WND + 0x2E9 + 1, m_nGameWidth);
    //cmp ecx,600
    MemEdit::WriteInt(C_CTRL_EDIT_CREATE_IME_CAND_WND + 0x2FE + 1, m_nGameHeight);
    //mov eax,800 ; CreateWnd
    MemEdit::WriteInt(C_CTRL_EDIT_CREATE_IME_CAND_WND + 0x2E9 + 1, m_nGameWidth);

    // Something MonsterBook related.
    //cmp edx,600
    MemEdit::WriteInt(0x0085F303 + 0x3E + 2, m_nGameHeight);
    //mov eax, 231h
    MemEdit::WriteInt(0x0085F303 + 0x5E + 1, m_nGameHeight - 39);
    //cmp edx,800
    MemEdit::WriteInt(0x0085F303 + 0x69 + 2, m_nGameWidth);
    //mov ecx,720
    MemEdit::WriteInt(0x0085F303 + 0x71 + 1, m_nGameWidth - 80);

    //mov     ebx, 400
    MemEdit::WriteInt(C_MAP_LOADABLE_RESTORE_VIEW_RANGE + 0x70 + 1, m_nGameWidth / 2);
    //add     eax, 300
    MemEdit::WriteInt(C_MAP_LOADABLE_RESTORE_VIEW_RANGE + 0xD7 + 1, m_nGameHeight / 2);
    //sub     eax, 300
    MemEdit::WriteInt(C_MAP_LOADABLE_RESTORE_VIEW_RANGE + 0x19E + 1, m_nGameHeight / 2);

    //push    578
    MemEdit::WriteInt(C_UI_STATUS_BAR + 0x273 + 1, m_nGameHeight);
    //push    800
    MemEdit::WriteInt(C_UI_STATUS_BAR + 0x278 + 1, m_nGameWidth);


    MemEdit::WriteInt(C_UI_STATUS_BAR_ON_CREATE + 0x1B9E + 1, m_nGameHeight - 22);
    MemEdit::WriteInt(C_UI_STATUS_BAR_ON_CREATE + 0x1BA3 + 1, m_nGameWidth);
    MemEdit::WriteInt(C_UI_STATUS_BAR_ON_CREATE + 0x1E42 + 1, m_nGameHeight - 22);
    MemEdit::WriteInt(C_UI_STATUS_BAR_ON_CREATE + 0x1E47 + 1, m_nGameWidth);

    MemEdit::WriteInt(C_UI_STATUS_BAR_ON_CREATE + 0x15E1 + 1, m_nGameHeight - (600-533));
    MemEdit::WriteInt(C_UI_STATUS_BAR_ON_CREATE + 0x2DFC + 1, m_nGameHeight - (600-543));
    MemEdit::WriteInt(C_UI_STATUS_BAR_ON_CREATE + 0x2EA4 + 1, m_nGameHeight - (600-543));
    MemEdit::WriteInt(C_UI_STATUS_BAR_ON_CREATE + 0x2F6D + 1, m_nGameHeight - (600-543));
    MemEdit::WriteInt(C_UI_STATUS_BAR_ON_CREATE + 0x3035 + 1, m_nGameHeight - (600-543));
    MemEdit::WriteInt(C_UI_STATUS_BAR_ON_CREATE + 0x30BC + 1, m_nGameHeight - (600-515));
    MemEdit::WriteInt(C_UI_STATUS_BAR_ON_CREATE + 0x3143 + 1, m_nGameHeight - (600-515));
    MemEdit::WriteInt(C_UI_STATUS_BAR_ON_CREATE + 0x31CA + 1, m_nGameHeight - (600-515));
    MemEdit::WriteInt(C_UI_STATUS_BAR_ON_CREATE + 0x3251 + 1, m_nGameHeight - (600-515));
    MemEdit::WriteInt(C_UI_STATUS_BAR_ON_CREATE + 0x32D8 + 1, m_nGameHeight - (600-515));
    MemEdit::WriteInt(C_UI_STATUS_BAR_ON_CREATE + 0x33D4 + 1, m_nGameHeight - (600-515));
    MemEdit::WriteInt(C_UI_STATUS_BAR_ON_CREATE + 0x34E4 + 1, m_nGameHeight - (600-515));

    MemEdit::WriteInt(C_UI_STATUS_BAR_SET_STATUS_VALUE + 0x3D  + 1, m_nGameHeight - (600-578));
    MemEdit::WriteInt(C_UI_STATUS_BAR_SET_STATUS_VALUE + 0x42  + 1, m_nGameWidth);
    MemEdit::WriteInt(C_UI_STATUS_BAR_SET_STATUS_VALUE + 0x9B  + 1, m_nGameHeight - (600-554));
    MemEdit::WriteInt(C_UI_STATUS_BAR_SET_STATUS_VALUE + 0x119  + 1, m_nGameHeight - (600-545));
    MemEdit::WriteInt(C_UI_STATUS_BAR_SET_STATUS_VALUE + 0x1B8  + 1, m_nGameHeight - (600-560));

    // CUIStatusBar::MakeCtrlEdit
    MemEdit::WriteInt(0x008DFB36 + 0x6F  + 1, m_nGameHeight - (600-520));
    MemEdit::WriteInt(0x008DFB36 + 0xDA  + 1, m_nGameHeight - (600-515));

    //CUIStatusBar::ToggleQuickSlot
    MemEdit::WriteInt(0x008DF4B1 + 0x2D1  + 2, m_nGameHeight - (600-533));
    MemEdit::WriteInt(0x008DF4B1 + 0x452 + 1, m_nGameHeight - (600-515));

    // CUIStatusBar::ToggleMaxMinButton
    MemEdit::WriteInt(0x008DF9C2 + 0xAD  + 1, m_nGameHeight - (600-519));
    MemEdit::WriteInt(0x008DF9C2 + 0x13F  + 1, m_nGameHeight - (600-519));

    // CUIStatusBar::SetChatType
    MemEdit::WriteInt(0x008D4A6D + 0x1DA  + 1, m_nGameHeight - (600-513));
    MemEdit::WriteInt(0x008D4A6D + 0x4C  + 2, m_nGameHeight - (600-507));
    MemEdit::WriteInt(0x008D4A6D + 0x8E  + 1, m_nGameHeight - (600-509));
    MemEdit::WriteInt(0x008D4A6D + 0x100  + 1, m_nGameHeight - (600-510));
    MemEdit::WriteInt(0x008D4A6D + 0x1B2  + 1, m_nGameHeight - (600-510));

    MemEdit::WriteInt(0x008D3B2F + 0x444  + 1, m_nGameHeight - (71+22));
    MemEdit::WriteInt(0x008D3B2F + 0x4B6  + 1, m_nGameHeight - (71+22));
    MemEdit::WriteInt(0x008D3B2F + 0x529  + 1, m_nGameHeight - (63+22));
    MemEdit::WriteInt(0x008D3B2F + 0x59F  + 1, m_nGameHeight - (59+22));
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