#include "Client.h"
#include "memedit.h"
#include "AddyLocations.h"

int Client::m_nGameHeight = 768;
int Client::m_nGameWidth = 1024;
int nStatusBarY = Client::m_nGameHeight - 578;

__declspec(naked) void AdjustStatusBar() {
    __asm {
            push nStatusBarY
            push ebx // horizontal position; 0
            mov ecx, esi
            jmp dword ptr[dwStatusBarPosRetn]
    }
}

__declspec(naked) void AdjustStatusBarBG() {
    __asm {
            push nStatusBarY
            movsd
            push 0
            jmp dword ptr[dwStatusBarBackgroundPosRetn]
    }
}

__declspec(naked) void AdjustStatusBarInput() {
    __asm {
            push nStatusBarY
            push edi
            lea ecx,[esi + 0x0CD0]
            jmp dword ptr[dwStatusBarInputPosRetn]
    }
}

__declspec(naked) void PositionLoginDlg() {
    __asm {
            push 0x000000B4
            push 400
            push - 48    // y
            push - 185    // x
            jmp dword ptr[dwLoginCreateDlgRtn]
    }
}

__declspec(naked) void PositionLoginUsername() {
    __asm {
            push 0x0F
            push 0x00000084
            push 127    // y
            push 0        // x
            jmp dword ptr[dwLoginUsernameRtn]
    }
}

__declspec(naked) void PositionLoginPassword() {
    __asm {
            push 0x0F
            push 0x78
            push 127    // y
            push 272    // x
            jmp dword ptr[dwLoginPasswordRtn]
    }
}

void Client::EnableNewIGCipher() {
    const int nCipherHash = m_nIGCipherHash;
    MemEdit::WriteInt(dwIGCipherHash + 3, nCipherHash);
    MemEdit::WriteInt(dwIGCipherVirtual1 + 3, nCipherHash);
    MemEdit::WriteInt(dwIGCipherVirtual2 + 3, nCipherHash);
    MemEdit::WriteInt(dwIGCipherDecrypt + 3, nCipherHash);
    MemEdit::WriteInt(dwIGCipherDecryptStr + 3, nCipherHash);
}

void Client::UpdateResolution() {
    nStatusBarY = Client::m_nGameHeight - 578;

    MemEdit::CodeCave(AdjustStatusBar, dwStatusBarVPos, 5);
    MemEdit::CodeCave(AdjustStatusBarBG, dwStatusBarBackgroundVPos, 5);
    MemEdit::CodeCave(AdjustStatusBarInput, dwStatusBarInputVPos, 9);

    MemEdit::WriteInt(dwApplicationHeight + 1, m_nGameHeight);//push 600
    MemEdit::WriteInt(dwApplicationWidth + 1, m_nGameWidth);    //push 800 ; CWvsApp::InitializeGr2D
    MemEdit::WriteInt(dwCursorVectorVPos + 2, floor(-m_nGameHeight / 2));//push -300
    MemEdit::WriteInt(dwCursorVectorHPos + 2,
                      floor(-m_nGameWidth / 2));    //push -400 ; CInputSystem::SetCursorVectorPos
    MemEdit::WriteInt(dwUpdateMouseLimitVPos + 1, m_nGameHeight);//mov ecx,600
    MemEdit::WriteInt(dwUpdateMouseLimitHPos + 1, m_nGameWidth);    //mov ecx,800 ; CInputSystem::UpdateMouse
    MemEdit::WriteInt(dwCursorPosLimitVPos + 1, m_nGameHeight);//mov eax,600
    MemEdit::WriteInt(dwCursorPosLimitHPos + 1, m_nGameWidth);    //mov eax,800 ; CInputSystem::SetCursorPos
    MemEdit::WriteInt(dwViewPortHeight + 3, m_nGameHeight);//lea eax,[esi+eax-600]
    MemEdit::WriteInt(dwViewPortWidth + 3, m_nGameWidth);    //lea eax,[ecx+eax-800]

    MemEdit::WriteInt(dwToolTipLimitVPos + 1, m_nGameWidth - 1); //mov eax,599 ; CUIToolTip::MakeLayer
    MemEdit::WriteInt(dwToolTipLimitHPos + 1, m_nGameWidth - 1); //mov eax,799 ; CUIToolTip::MakeLayer

    MemEdit::WriteInt(dwTempStatToolTipDraw + 3,
                      -m_nGameWidth + 6); //lea eax,[eax+ecx-797] ; CTemporaryStatView::ShowToolTip
    MemEdit::WriteInt(dwTempStatToolTipFind + 3,
                      -m_nGameWidth + 6); //lea eax,[eax+ecx-797] ; CTemporaryStatView::FindIcon
    MemEdit::WriteInt(dwTempStatIconVPos + 2, (m_nGameHeight / 2) - 23);    //sub ebx,277 ; Skill icon buff y-pos
    MemEdit::WriteInt(dwTempStatIconHpos + 3,
                      (m_nGameWidth / 2) - 3);    //lea eax,[eax+esi+397] ; Skill icon buff x-pos
    MemEdit::WriteInt(dwTempStatCoolTimeVPos + 2,
                      (m_nGameHeight / 2) - 23);    //sub ebx,277 ; Skill icon cooltime y-pos
    MemEdit::WriteInt(dwTempStatCoolTimeHPos + 3,
                      (m_nGameWidth / 2) - 3);    //lea eax,[eax+esi+397] ; Skill icon cooltime x-pos

    MemEdit::WriteInt(dwQuickSlotInitVPos + 1, m_nGameHeight - 67);//add eax,533
    MemEdit::WriteInt(dwQuickSlotInitHPos + 1, m_nGameWidth - 153); //push 647
    MemEdit::WriteInt(dwQuickSlotVPos + 2, m_nGameHeight - 67);//add esi,533
    MemEdit::WriteInt(dwQuickSlotHPos + 1, m_nGameWidth - 153); //push 647
    MemEdit::WriteInt(dwQuickSlotCWndVPos + 2, (-m_nGameWidth + 32) / 2); //lea edi,[eax-647]
    MemEdit::WriteInt(dwQuickSlotCWndHPos + 2, -m_nGameWidth + 228); //lea ebx,[eax-427]

    MemEdit::WriteInt(dwByteAvatarMegaHPos + 1,
                      m_nGameWidth + 100); //push 800 ; CAvatarMegaphone::ByeAvatarMegaphone ; IWzVector2D::RelMove
    MemEdit::WriteInt(dwAvatarMegaWidth + 1, m_nGameWidth); //push 800 ; CAvatarMegaphone ; CreateWnd

    MemEdit::WriteInt(0x0043717B + 1, m_nGameHeight);//mov edi,600
    MemEdit::WriteInt(0x00437181 + 1, m_nGameWidth);    //mov esi,800 ; CreateWnd
    MemEdit::WriteInt(0x0053808B + 1, m_nGameHeight);//push 600
    MemEdit::WriteInt(0x00538091 + 1, m_nGameWidth);    //push 800 ; RelMove?
    MemEdit::WriteInt(0x004CC160 + 1, m_nGameWidth);    //mov [ebp-16],800 ; CreateWnd
    MemEdit::WriteInt(0x004CC2C5 + 2, m_nGameHeight);//cmp ecx,600
    MemEdit::WriteInt(0x004CC2B0 + 1, m_nGameWidth);    //mov eax,800 ; CreateWnd
    MemEdit::WriteInt(0x004D59B2 + 1, m_nGameHeight);//mov eax,800
    MemEdit::WriteInt(0x004D599D + 1, m_nGameWidth);    //mov eax,800 ; CreateWnd
    MemEdit::WriteInt(0x0085F36C + 2, m_nGameWidth);    //cmp edx,800
    MemEdit::WriteInt(0x0085F374 + 1, m_nGameWidth - 80);    //mov ecx,720 ; CreateDlg
    MemEdit::WriteInt(0x008EBC58 + 1, m_nGameHeight);//mov eax,600
    MemEdit::WriteInt(0x008EBC3C + 1, m_nGameWidth);    //mov eax,800 ; RelMove?
    MemEdit::WriteInt(0x009966B5 + 1, m_nGameHeight);//mov ecx,600
    MemEdit::WriteInt(0x009966CA + 2, m_nGameWidth);    //cmp edi,800
    MemEdit::WriteInt(0x009966D2 + 1, m_nGameWidth - 100);    //mov edx,700 ; CreateDlg
    MemEdit::WriteInt(0x009A3E7F + 1, m_nGameHeight);//mov edx,600
    MemEdit::WriteInt(0x009A3E72 + 1, m_nGameWidth);    //mov edx,800 ; CreateDlg
    MemEdit::WriteInt(0x0045B898 + 1, m_nGameHeight - 25);    //push 575
    MemEdit::WriteInt(0x0045B97E + 1, m_nGameWidth);    //push 800 ; RelMove?
    MemEdit::WriteInt(0x004D9BD1 + 1, m_nGameWidth);    //push 800
    MemEdit::WriteInt(0x004D9C37 + 1, m_nGameWidth);    //push 800
    MemEdit::WriteInt(0x004D9C84 + 1, m_nGameWidth);    //push 800 ; StringPool#1443 (BtMouseCilck)
    MemEdit::WriteInt(0x005386F0 + 1, m_nGameHeight);//push 600
    MemEdit::WriteInt(0x005386F5 + 1, m_nGameWidth);    //push 800 ; CField::DrawFearEffect
    MemEdit::WriteInt(0x0055B808 + 1, m_nGameHeight);//push 600
    MemEdit::WriteInt(0x0055B80D + 1, m_nGameWidth);    //mov edi,800
    MemEdit::WriteInt(0x0055B884 + 1, m_nGameWidth);    //push 600 ; RelMove?
    MemEdit::WriteInt(0x007E15BE + 1, m_nGameWidth);    //push 800 ; CreateWnd
    MemEdit::WriteInt(0x007E16B9 + 1, m_nGameHeight);//push 600
    MemEdit::WriteInt(0x007E16BE + 1, m_nGameWidth);    //push 800 ; CWnd::GetCanvas
    MemEdit::WriteInt(0x008AA266 + 1, m_nGameHeight);//push 600
    MemEdit::WriteInt(0x008AA26B + 1, m_nGameWidth);    //push 800 ; CreateWnd
    MemEdit::WriteInt(0x009F6E99 + 1, m_nGameHeight);//push 600
    MemEdit::WriteInt(0x009F6EA0 + 1, m_nGameWidth);    //push 800 ; StringPool#1162 (MapleStoryClass)
    MemEdit::WriteInt(0x005A8B46 + 1, m_nGameHeight);//mov eax,600
    MemEdit::WriteInt(0x005A8B56 + 1, m_nGameWidth);    //mov eax,800 ; RelMove?
    MemEdit::WriteInt(0x005A9B42 + 1, m_nGameHeight);//mov eax,600
    MemEdit::WriteInt(0x005A9B52 + 1, m_nGameWidth);    //mov eax,800 ; RelMove?
    MemEdit::WriteInt(0x005AADAA + 1, m_nGameHeight);//mov eax,600
    MemEdit::WriteInt(0x005AADBA + 1, m_nGameWidth);    //mov eax,800 ; RelMove?
    MemEdit::WriteInt(0x005ABC65 + 1, m_nGameHeight);//mov eax,600
    MemEdit::WriteInt(0x005ABC75 + 1, m_nGameWidth);    //mov eax,800 ; RelMove?
    MemEdit::WriteInt(0x005ACB29 + 1, m_nGameHeight);//mov eax,600
    MemEdit::WriteInt(0x005ACB39 + 1, m_nGameWidth);    //mov eax,800 ; RelMove?
    MemEdit::WriteInt(0x005C187E + 1, m_nGameHeight);//mov eax,600
    MemEdit::WriteInt(0x005C188E + 1, m_nGameWidth);    //mov eax,800 ; RelMove?
    MemEdit::WriteInt(0x005C2D62 + 1, m_nGameHeight);//mov eax,600
    MemEdit::WriteInt(0x005C2D72 + 1, m_nGameWidth);    //mov eax,800 ; RelMove?
    MemEdit::WriteInt(0x007CF48F + 1, m_nGameHeight);//mov eax,600 ;
    MemEdit::WriteInt(0x007CF49D + 1, m_nGameWidth);    //mov eax,800 ; IWzVector2D::RelMove
    MemEdit::WriteInt(0x008A12F4 + 1, m_nGameHeight);//mov eax,600 ;
    MemEdit::WriteInt(0x008A1302 + 1, m_nGameWidth);    //mov eax,800 ; IWzVector2D::RelMove
    MemEdit::WriteInt(0x0062FC4A + 1, m_nGameHeight);//push 600
    MemEdit::WriteInt(0x0062FC4F + 1, m_nGameWidth);    //push 800 ; IWzGr2DLayer::Getcanvas
    MemEdit::WriteInt(0x0062FE63 + 1, m_nGameHeight);//push 600
    MemEdit::WriteInt(0x0062FE68 + 1, m_nGameWidth);    //push 800 ; IWzGr2DLayer::Getcanvas
    MemEdit::WriteInt(0x007F257E + 1, m_nGameHeight);//push 600
    MemEdit::WriteInt(0x007F258F + 1, m_nGameWidth);    //push 800 ; CWnd::CreateWnd
    MemEdit::WriteInt(0x0062F9C6 + 1, m_nGameHeight);//push 600
    MemEdit::WriteInt(0x0062F9CB + 1, m_nGameWidth);    //push 800; (UI/Logo/Wizet)
    MemEdit::WriteInt(0x0062F104 + 1, m_nGameHeight);//mov eax,600
    MemEdit::WriteInt(0x0062F109 + 1, m_nGameWidth);    //mov eax,800 ; Rectangle
    MemEdit::WriteInt(0x006406D5 + 1, m_nGameHeight);//mov ecx,600
    MemEdit::WriteInt(0x006406C3 + 1, m_nGameWidth);    //mov ecx,800
    MemEdit::WriteInt(0x0064050A + 1, m_nGameHeight);//mov ecx,600
    MemEdit::WriteInt(0x006404F8 + 1, m_nGameWidth);    //mov ecx,800
    MemEdit::WriteInt(0x00640618 + 1, m_nGameHeight);//mov ecx,600
    MemEdit::WriteInt(0x00640690 + 1, m_nGameHeight);//mov ecx,600
    MemEdit::WriteInt(0x0064061D + 1, m_nGameHeight);//mov ecx,600
    MemEdit::WriteInt(0x0064064B + 1, m_nGameHeight);//mov ecx,600
    MemEdit::WriteInt(0x00640606 + 1, m_nGameWidth);    //mov ecx,800
    MemEdit::WriteInt(0x0064067E + 1, m_nGameWidth);    //mov ecx,800
    MemEdit::WriteInt(0x00640639 + 1, m_nGameWidth);    //mov ecx,800
    MemEdit::WriteInt(0x0064043E + 1, floor(m_nGameWidth / 2));    //mov edi,400
    MemEdit::WriteInt(0x00640443 + 1, floor(m_nGameHeight / 2));    //mov esi,300
    MemEdit::WriteInt(0x00640626 + 1, floor(m_nGameWidth / 2));    //add eax,400 ; bunch of modulus stuff
    MemEdit::WriteInt(0x00641A19 + 3, m_nGameHeight);//mov [ebp+28],600
    MemEdit::WriteInt(0x00641A12 + 3, m_nGameWidth);    //mov [ebp+32],800 ; idk
    MemEdit::WriteInt(0x00641B38 + 3, m_nGameHeight);//mov [ebp-32],600
    MemEdit::WriteInt(0x00641B2E + 3, m_nGameWidth);    //mov [ebp-36],800 ; CAnimationDisplayer::SetCenterOrigin
    MemEdit::WriteInt(0x0046B85C + 1, m_nGameHeight);//mov eax,600
    MemEdit::WriteInt(0x0046B86A + 1, m_nGameWidth);    //mov eax,800 ; IWzVector2D::RelMove
    MemEdit::WriteInt(0x009994D8 + 1, m_nGameHeight);//mov ecx,600
    MemEdit::WriteInt(0x009994ED + 2, m_nGameWidth);    //cmp edi,800
    MemEdit::WriteInt(0x009994F5 + 1, m_nGameWidth - 100);    //mov edx,700 ; CreateDlg
    MemEdit::WriteInt(0x00641FC8 + 1, floor(m_nGameHeight / 2));    //add eax,300  ; VRRight
    MemEdit::WriteInt(0x0064208F + 1, floor(m_nGameHeight / 2));    //sub eax,300
    MemEdit::WriteInt(0x00641F61 + 1, floor(m_nGameWidth / 2));    //mov ebc,400 ;  VRTop
    MemEdit::WriteInt(0x006CD842 + 1, floor(m_nGameWidth / 2));    //push 400 ; RelMove?
    MemEdit::WriteInt(0x0059A0A2 + 6, floor(m_nGameHeight / 2));    //mov [ebx+2364],300
    MemEdit::WriteInt(0x0059A09C + 2, floor(m_nGameWidth / 2));    //mov [esi],400	; CInputSystem::LoadCursorState
    MemEdit::WriteInt(0x0080546C + 1, m_nGameHeight);//mov edi,600
    MemEdit::WriteInt(0x00805459 + 1, m_nGameWidth);    //mov edx,800 ; CUIEventAlarm::CreateEventAlarm
    MemEdit::WriteInt(0x008CFD4B + 1, m_nGameHeight - 22);    //push 578
    MemEdit::WriteInt(0x008CFD50 + 1, m_nGameWidth);    //push 800
    MemEdit::WriteInt(0x0053836D + 1, floor(-m_nGameHeight / 2));//push -300
    MemEdit::WriteInt(0x00538373 + 1, floor(-m_nGameWidth / 2));    //push -400	; RelMove?
    MemEdit::WriteInt(0x0055BB2F + 1, floor(-m_nGameHeight / 2));//push -300
    MemEdit::WriteInt(0x0055BB35 + 1, floor(-m_nGameWidth / 2));    //push -400 ; RelMove?
    MemEdit::WriteInt(0x005F481E + 1, floor(-m_nGameHeight / 2));//push -300
    MemEdit::WriteInt(0x005F4824 + 1, floor(-m_nGameWidth / 2));    //push -400 ; RelMove?
    MemEdit::WriteInt(0x004372B1 + 1, floor(-m_nGameHeight / 2));//push -300
    MemEdit::WriteInt(0x004372B6 + 1, floor(-m_nGameWidth / 2));    //push -400 ; RelMove?
    MemEdit::WriteInt(0x006CE3AB + 1, m_nGameWidth);    //push 800
    MemEdit::WriteInt(0x007E1CB7 + 1, m_nGameWidth);    //push 800
    MemEdit::WriteInt(0x008D82F5 + 1, m_nGameHeight - 22);    //push 578
    MemEdit::WriteInt(0x008D82FA + 1, m_nGameWidth);    //push 800 ; CreateWnd?
    MemEdit::WriteInt(0x00935870 + 1, floor(m_nGameHeight / 2));    //push 300
    MemEdit::WriteInt(0x0093586B + 1, m_nGameWidth);    // push 800 ; RelMove? (Skills)
    MemEdit::WriteInt(0x009DFD5C + 1, m_nGameWidth);    //mov ecx,800
    MemEdit::WriteInt(0x009DFED2 + 1, m_nGameHeight);//mov ecx,600	; IWzVector2D::RelMove
    MemEdit::WriteInt(0x009F6ADD + 1, floor(m_nGameHeight / 2)); //push 300 ; MapleStoryClass
    MemEdit::WriteInt(0x006D50D8 + 1, m_nGameHeight);//push 600
    MemEdit::WriteInt(0x0074BAA9 + 1, m_nGameHeight);//push 600
    MemEdit::WriteInt(0x0074B951 + 1, m_nGameHeight);//push 600
    MemEdit::WriteInt(0x0074B4A2 + 1, m_nGameHeight);//push 600
    MemEdit::WriteInt(0x0074B3B7 + 1, m_nGameHeight);//push 600
    MemEdit::WriteInt(0x006421B3 + 1, m_nGameHeight);//push 600 ; CSoundMan::PlayBGM
    MemEdit::WriteInt(0x0060411C + 1, m_nGameHeight);//push 600
    MemEdit::WriteInt(0x00604126 + 1, floor(-m_nGameHeight / 2));    //push -300
    MemEdit::WriteInt(0x005E3FA0 + 1, m_nGameHeight);//push 600
    MemEdit::WriteInt(0x0059EB49 + 1, m_nGameHeight);//push 600 ; CSoundMan::PlayBGM
    MemEdit::WriteInt(0x008D247B + 1, m_nGameHeight - 33);    //push 567 ; IWzVector2D::RelMove
    MemEdit::WriteInt(0x008DEB93 + 1, m_nGameHeight - 20);    //push 580
    MemEdit::WriteInt(0x008DEE2F + 1, m_nGameHeight - 20);    //push 580
    MemEdit::WriteInt(0x008D2765 + 1, m_nGameHeight - 19);    //push 581
    MemEdit::WriteInt(0x008D29B4 + 1, m_nGameHeight - 19);    //push 581
    MemEdit::WriteInt(0x008D8BFE + 1, m_nGameHeight - 19);    //push 581
    MemEdit::WriteInt(0x008D937E + 1, m_nGameHeight - 19);    //push 581
    MemEdit::WriteInt(0x008D9AC9 + 1, m_nGameHeight - 19);    //push
    MemEdit::WriteInt(0x008D1D50 + 1, m_nGameHeight - 22);    //push 578
    MemEdit::WriteInt(0x008D1D55 + 1, m_nGameWidth);    //push 800
    MemEdit::WriteInt(0x008D1FF4 + 1, m_nGameHeight - 22);    //push 578
    MemEdit::WriteInt(0x008D1FF9 + 1, m_nGameWidth);    //push 800 ; CUIStatusBar
    MemEdit::WriteInt(0x0062F5DF + 1, m_nGameHeight);//push 600
    MemEdit::WriteInt(0x0062F5E4 + 1, m_nGameWidth);    //push 800 ; (UI/Logo/Nexon)
    MemEdit::WriteInt(0x004EDB89 + 1, m_nGameWidth);    //mov ecx,800
    MemEdit::WriteInt(0x004EDB78 + 1, m_nGameHeight);//mov ecx,600 ; CreateWnd
    MemEdit::WriteInt(0x004EDAD8 + 1, m_nGameWidth);    //mov ecx,800
    MemEdit::WriteInt(0x009F7079, m_nGameHeight);    // dd 600
    MemEdit::WriteInt(0x009F707E, m_nGameWidth);    // dd 800
    MemEdit::WriteInt(0x00BE2738, floor(m_nGameWidth / 2));    // dd 400
    MemEdit::WriteInt(0x00BE2DF4, floor(m_nGameHeight / 2));    // dd 300
    MemEdit::WriteInt(0x00BE2DF0, floor(m_nGameWidth / 2));    // dd 400
    MemEdit::WriteInt(0x00640656 + 2, floor(-m_nGameWidth / 2));        //add edi,-400 ;
    MemEdit::WriteInt(0x00641048 + 1, floor(-m_nGameHeight / 2));    //mov esi,-300
    MemEdit::WriteInt(0x00641050 + 1, floor(-m_nGameWidth / 2));        //mov esi,-400 ;
    MemEdit::WriteInt(0x006CE4C6 + 1, floor(-m_nGameWidth / 2));        //push -400 ;
    MemEdit::WriteInt(0x009E2E85 + 1, floor(-m_nGameHeight / 2));    //push -300
    MemEdit::WriteInt(0x009E2E8B + 1, floor(-m_nGameWidth / 2));        //push -400 ;
    MemEdit::WriteInt(0x005F64DE + 1, floor(-m_nGameHeight / 2));    //push -300 ;
    MemEdit::WriteInt(0x0093519A + 1, floor(-m_nGameHeight / 2));    //push -300 ;
    MemEdit::WriteInt(0x00954433 + 1, floor(-m_nGameHeight / 2));    //push -300 ;
    MemEdit::WriteInt(0x00981555 + 1, floor(-m_nGameHeight / 2));    //push -300 ;
    MemEdit::WriteInt(0x00981F7A + 2, floor(-m_nGameHeight / 2));    //push -300 ;
    MemEdit::WriteInt(0x00A448B0 + 2, floor(-m_nGameHeight / 2));    //push -300 ; CWvsPhysicalSpace2D::Load
    MemEdit::WriteInt(0x005F6627 + 1, floor(-m_nGameHeight / 2));    //push -300 ;
    MemEdit::WriteInt(0x0066BACE + 2, floor(-m_nGameWidth / 2));        //and ecx,-400
    MemEdit::WriteInt(0x009B76BD + 3, floor(-m_nGameHeight / 2));    //push -300
    MemEdit::WriteInt(0x009B76CB + 3, floor(m_nGameHeight / 2));        //push 300

    MemEdit::WriteInt(dwSlideNoticeWidth + 1, m_nGameWidth);    //push 800 ; CSlideNotice::CSlideNotice
    MemEdit::WriteInt(dwSlideNoticeSetMsgWidth + 1, m_nGameWidth);    //push 800 ; CSlideNotice::SetMsg
    MemEdit::WriteInt(dwSlideNoticeSetMsgWidth2 + 2, m_nGameWidth);    //push 800 ; CSlideNotice::SetMsg

    MemEdit::WriteInt(0x008D01B2 + 0x2E01 + 1, m_nGameWidth - 227);    // ShopButton X
    MemEdit::WriteInt(0x008D01B2 + 0x2EA9 + 1, m_nGameWidth - 171);    // MTSButton X
    MemEdit::WriteInt(0x008D01B2 + 0x2F72 + 1, m_nGameWidth - 115);    // Menu Button X
    MemEdit::WriteInt(0x008D01B2 + 0x2F72 + 1, m_nGameWidth - 115);    // Menu Button X
    MemEdit::WriteInt(0x008D01B2 + 0x303A + 1, m_nGameWidth - 59);    // Shortcut Button X

    MemEdit::WriteInt(0x008D01B2 + 0x33D9 + 1, m_nGameWidth - 32); // Toggle Quickslot X
    MemEdit::WriteInt(0x008DF4B1 + 0x457 + 1, m_nGameWidth - 32); // Toggle Quickslot X

    MemEdit::WriteInt(0x008D01B2 + 0x32DD + 1, m_nGameWidth - 62); // SetKey X
    MemEdit::WriteInt(0x008D01B2 + 0x3256 + 1, m_nGameWidth - 92); // SkillKey X
    MemEdit::WriteInt(0x008D01B2 + 0x31CF + 1, m_nGameWidth - 122); // StatKey X
    MemEdit::WriteInt(0x008D01B2 + 0x3148 + 1, m_nGameWidth - 152); // InvenKey X
    MemEdit::WriteInt(0x008D01B2 + 0x30C1 + 1, m_nGameWidth - 182); // EquipKey X

    MemEdit::WriteInt(0x008D3B2F  + 0x52F + 1, m_nGameWidth - 227); // Alert GM
    MemEdit::WriteInt(0x008D01B2  + 0x2E01 + 1, m_nGameWidth - 227); // Alert GM
    MemEdit::WriteInt(0x008D01B2  + 0x34E9 + 1, m_nGameWidth - 227); // Alert GM

    MemEdit::WriteInt(0x008D3B2F  + 0x5A5 + 1, m_nGameWidth - 201); // IconMemo X Pos

    MemEdit::WriteInt(0x00849DE2 + 0x5D + 1, m_nGameWidth - 134); // Game Menu X Pos
    MemEdit::WriteInt(0x00849DE2 + 0x57 + 1, m_nGameHeight - 177); // Game Menu Y Pos
    MemEdit::WriteInt(0x0084A560 + 0x5D + 1, m_nGameWidth - 93); // ShortCut Menu X Pos
    MemEdit::WriteInt(0x0084A560 + 0x57 + 1, m_nGameHeight - 282); // ShortCut Menu Y Pos
}

void Client::UpdateLogin() {
    MemEdit::CodeCave(PositionLoginDlg, dwLoginCreateDlg, 14);
    MemEdit::CodeCave(PositionLoginUsername, dwLoginUsername, 11);
    MemEdit::CodeCave(PositionLoginPassword, dwLoginPassword, 8);
    MemEdit::WriteInt(dwLoginInputBackgroundColor + 3, 0xFFF8FAFF); // ARGB value
    MemEdit::WriteByte(dwLoginInputFontColor + 3, 1); // bool: true=black, false=white
    MemEdit::WriteInt(dwLoginLoginBtn + 1, -127); // x-pos
    MemEdit::WriteInt(dwLoginFindPasswordBtn + 1, -127); // x-pos
    MemEdit::WriteInt(dwLoginQuitBtn + 1, -127); // x-pos
    MemEdit::WriteInt(dwLoginFindIDBtn + 1, -127); // x-pos
    MemEdit::WriteByte(dwLoginFindIDBtn + 1, -127); // x-pos
    MemEdit::WriteByte(dwLoginWebHomeBtn + 1, -127); // x-pos
    MemEdit::WriteByte(dwLoginWebRegisterBtn + 1, -127); // x-pos
}
