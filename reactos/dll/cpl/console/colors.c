/*
 * PROJECT:         ReactOS Console Configuration DLL
 * LICENSE:         GPL - See COPYING in the top level directory
 * FILE:            dll/cpl/console/colors.c
 * PURPOSE:         Colors dialog
 * PROGRAMMERS:     Johannes Anderwald (johannes.anderwald@reactos.org)
 *                  Hermes Belusca-Maito (hermes.belusca@sfr.fr)
 */

#include "console.h"

#define NDEBUG
#include <debug.h>

static DWORD ActiveStaticControl = 0;

static BOOL
PaintStaticControls(
    IN HWND hDlg,
    IN PCONSOLE_STATE_INFO pConInfo,
    IN LPDRAWITEMSTRUCT drawItem)
{
    HBRUSH hBrush;
    DWORD index;

    index = min(drawItem->CtlID - IDC_STATIC_COLOR1,
                ARRAYSIZE(pConInfo->ColorTable) - 1);

    hBrush = CreateSolidBrush(pConInfo->ColorTable[index]);
    if (!hBrush) return FALSE;

    FillRect(drawItem->hDC, &drawItem->rcItem, hBrush);
    DeleteObject(hBrush);

    if (ActiveStaticControl == index)
        DrawFocusRect(drawItem->hDC, &drawItem->rcItem);

    return TRUE;
}

INT_PTR CALLBACK
ColorsProc(HWND hDlg,
           UINT uMsg,
           WPARAM wParam,
           LPARAM lParam)
{
    DWORD colorIndex;
    COLORREF color;

    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            /* Set the valid range of the colour indicators */
            SendDlgItemMessageW(hDlg, IDC_UPDOWN_COLOR_RED  , UDM_SETRANGE, 0, (LPARAM)MAKELONG(255, 0));
            SendDlgItemMessageW(hDlg, IDC_UPDOWN_COLOR_GREEN, UDM_SETRANGE, 0, (LPARAM)MAKELONG(255, 0));
            SendDlgItemMessageW(hDlg, IDC_UPDOWN_COLOR_BLUE , UDM_SETRANGE, 0, (LPARAM)MAKELONG(255, 0));

            /* Select by default the screen background option */
            CheckRadioButton(hDlg, IDC_RADIO_SCREEN_TEXT, IDC_RADIO_POPUP_BACKGROUND, IDC_RADIO_SCREEN_BACKGROUND);
            SendMessageW(hDlg, WM_COMMAND, IDC_RADIO_SCREEN_BACKGROUND, 0);

            return TRUE;
        }

        case WM_DRAWITEM:
        {
            LPDRAWITEMSTRUCT drawItem = (LPDRAWITEMSTRUCT)lParam;

            if (drawItem->CtlID >= IDC_STATIC_COLOR1 && drawItem->CtlID <= IDC_STATIC_COLOR16)
                return PaintStaticControls(hDlg, ConInfo, drawItem);
            else if (drawItem->CtlID == IDC_STATIC_SCREEN_COLOR)
                return PaintText(drawItem, ConInfo, Screen);
            else if (drawItem->CtlID == IDC_STATIC_POPUP_COLOR)
                return PaintText(drawItem, ConInfo, Popup);

            break;
        }

        case WM_NOTIFY:
        {
            switch (((LPNMHDR)lParam)->code)
            {
                case PSN_APPLY:
                {
                    ApplyConsoleInfo(hDlg);
                    return TRUE;
                }

                case UDN_DELTAPOS:
                {
                    LPNMUPDOWN lpnmud = (LPNMUPDOWN)lParam;

                    /* Get the current color */
                    colorIndex = ActiveStaticControl;
                    color = ConInfo->ColorTable[colorIndex];

                    if (lpnmud->hdr.idFrom == IDC_UPDOWN_COLOR_RED)
                    {
                        lpnmud->iPos = min(max(lpnmud->iPos + lpnmud->iDelta, 0), 255);
                        color = RGB(lpnmud->iPos, GetGValue(color), GetBValue(color));
                    }
                    else if (lpnmud->hdr.idFrom == IDC_UPDOWN_COLOR_GREEN)
                    {
                        lpnmud->iPos = min(max(lpnmud->iPos + lpnmud->iDelta, 0), 255);
                        color = RGB(GetRValue(color), lpnmud->iPos, GetBValue(color));
                    }
                    else if (lpnmud->hdr.idFrom == IDC_UPDOWN_COLOR_BLUE)
                    {
                        lpnmud->iPos = min(max(lpnmud->iPos + lpnmud->iDelta, 0), 255);
                        color = RGB(GetRValue(color), GetGValue(color), lpnmud->iPos);
                    }
                    else
                    {
                        break;
                    }

                    ConInfo->ColorTable[colorIndex] = color;
                    InvalidateRect(GetDlgItem(hDlg, IDC_STATIC_COLOR1 + colorIndex), NULL, TRUE);
                    InvalidateRect(GetDlgItem(hDlg, IDC_STATIC_SCREEN_COLOR), NULL, TRUE);
                    InvalidateRect(GetDlgItem(hDlg, IDC_STATIC_POPUP_COLOR) , NULL, TRUE);

                    PropSheet_Changed(GetParent(hDlg), hDlg);
                    break;
                }
            }

            break;
        }

        case WM_COMMAND:
        {
            /* NOTE: both BN_CLICKED and STN_CLICKED == 0 */
            if (HIWORD(wParam) == BN_CLICKED || HIWORD(wParam) == STN_CLICKED)
            {
                switch (LOWORD(wParam))
                {
                case IDC_RADIO_SCREEN_TEXT:
                {
                    /* Get the color of the screen foreground */
                    colorIndex = TextAttribFromAttrib(ConInfo->ScreenAttributes);
                    color = ConInfo->ColorTable[colorIndex];

                    /* Set the values of the colour indicators */
                    SetDlgItemInt(hDlg, IDC_EDIT_COLOR_RED  , GetRValue(color), FALSE);
                    SetDlgItemInt(hDlg, IDC_EDIT_COLOR_GREEN, GetGValue(color), FALSE);
                    SetDlgItemInt(hDlg, IDC_EDIT_COLOR_BLUE , GetBValue(color), FALSE);

                    InvalidateRect(GetDlgItem(hDlg, IDC_STATIC_COLOR1 + ActiveStaticControl), NULL, TRUE);
                    ActiveStaticControl = colorIndex;
                    InvalidateRect(GetDlgItem(hDlg, IDC_STATIC_COLOR1 + ActiveStaticControl), NULL, TRUE);
                    InvalidateRect(GetDlgItem(hDlg, IDC_STATIC_SCREEN_COLOR), NULL, TRUE);
                    InvalidateRect(GetDlgItem(hDlg, IDC_STATIC_POPUP_COLOR) , NULL, TRUE);
                    break;
                }

                case IDC_RADIO_SCREEN_BACKGROUND:
                {
                    /* Get the color of the screen background */
                    colorIndex = BkgdAttribFromAttrib(ConInfo->ScreenAttributes);
                    color = ConInfo->ColorTable[colorIndex];

                    /* Set the values of the colour indicators */
                    SetDlgItemInt(hDlg, IDC_EDIT_COLOR_RED  , GetRValue(color), FALSE);
                    SetDlgItemInt(hDlg, IDC_EDIT_COLOR_GREEN, GetGValue(color), FALSE);
                    SetDlgItemInt(hDlg, IDC_EDIT_COLOR_BLUE , GetBValue(color), FALSE);

                    InvalidateRect(GetDlgItem(hDlg, IDC_STATIC_COLOR1 + ActiveStaticControl), NULL, TRUE);
                    ActiveStaticControl = colorIndex;
                    InvalidateRect(GetDlgItem(hDlg, IDC_STATIC_COLOR1 + ActiveStaticControl), NULL, TRUE);
                    InvalidateRect(GetDlgItem(hDlg, IDC_STATIC_SCREEN_COLOR), NULL, TRUE);
                    InvalidateRect(GetDlgItem(hDlg, IDC_STATIC_POPUP_COLOR) , NULL, TRUE);
                    break;
                }

                case IDC_RADIO_POPUP_TEXT:
                {
                    /* Get the color of the popup foreground */
                    colorIndex = TextAttribFromAttrib(ConInfo->PopupAttributes);
                    color = ConInfo->ColorTable[colorIndex];

                    /* Set the values of the colour indicators */
                    SetDlgItemInt(hDlg, IDC_EDIT_COLOR_RED  , GetRValue(color), FALSE);
                    SetDlgItemInt(hDlg, IDC_EDIT_COLOR_GREEN, GetGValue(color), FALSE);
                    SetDlgItemInt(hDlg, IDC_EDIT_COLOR_BLUE , GetBValue(color), FALSE);

                    InvalidateRect(GetDlgItem(hDlg, IDC_STATIC_COLOR1 + ActiveStaticControl), NULL, TRUE);
                    ActiveStaticControl = colorIndex;
                    InvalidateRect(GetDlgItem(hDlg, IDC_STATIC_COLOR1 + ActiveStaticControl), NULL, TRUE);
                    InvalidateRect(GetDlgItem(hDlg, IDC_STATIC_SCREEN_COLOR), NULL, TRUE);
                    InvalidateRect(GetDlgItem(hDlg, IDC_STATIC_POPUP_COLOR) , NULL, TRUE);
                    break;
                }

                case IDC_RADIO_POPUP_BACKGROUND:
                {
                    /* Get the color of the popup background */
                    colorIndex = BkgdAttribFromAttrib(ConInfo->PopupAttributes);
                    color = ConInfo->ColorTable[colorIndex];

                    /* Set the values of the colour indicators */
                    SetDlgItemInt(hDlg, IDC_EDIT_COLOR_RED  , GetRValue(color), FALSE);
                    SetDlgItemInt(hDlg, IDC_EDIT_COLOR_GREEN, GetGValue(color), FALSE);
                    SetDlgItemInt(hDlg, IDC_EDIT_COLOR_BLUE , GetBValue(color), FALSE);

                    InvalidateRect(GetDlgItem(hDlg, IDC_STATIC_COLOR1 + ActiveStaticControl), NULL, TRUE);
                    ActiveStaticControl = colorIndex;
                    InvalidateRect(GetDlgItem(hDlg, IDC_STATIC_COLOR1 + ActiveStaticControl), NULL, TRUE);
                    InvalidateRect(GetDlgItem(hDlg, IDC_STATIC_SCREEN_COLOR), NULL, TRUE);
                    InvalidateRect(GetDlgItem(hDlg, IDC_STATIC_POPUP_COLOR) , NULL, TRUE);
                    break;
                }
                }

                if (IDC_STATIC_COLOR1 <= LOWORD(wParam) && LOWORD(wParam) <= IDC_STATIC_COLOR16)
                {
                    colorIndex = LOWORD(wParam) - IDC_STATIC_COLOR1;

                    /* If the same static control was re-clicked, don't take it into account */
                    if (colorIndex == ActiveStaticControl)
                        break;

                    color = ConInfo->ColorTable[colorIndex];

                    SetDlgItemInt(hDlg, IDC_EDIT_COLOR_RED  , GetRValue(color), FALSE);
                    SetDlgItemInt(hDlg, IDC_EDIT_COLOR_GREEN, GetGValue(color), FALSE);
                    SetDlgItemInt(hDlg, IDC_EDIT_COLOR_BLUE , GetBValue(color), FALSE);

                    if (IsDlgButtonChecked(hDlg, IDC_RADIO_SCREEN_TEXT))
                    {
                        ConInfo->ScreenAttributes = MakeAttrib(colorIndex, BkgdAttribFromAttrib(ConInfo->ScreenAttributes));
                    }
                    else if (IsDlgButtonChecked(hDlg, IDC_RADIO_SCREEN_BACKGROUND))
                    {
                        ConInfo->ScreenAttributes = MakeAttrib(TextAttribFromAttrib(ConInfo->ScreenAttributes), colorIndex);
                    }
                    else if (IsDlgButtonChecked(hDlg, IDC_RADIO_POPUP_TEXT))
                    {
                        ConInfo->PopupAttributes = MakeAttrib(colorIndex, BkgdAttribFromAttrib(ConInfo->PopupAttributes));
                    }
                    else if (IsDlgButtonChecked(hDlg, IDC_RADIO_POPUP_BACKGROUND))
                    {
                        ConInfo->PopupAttributes = MakeAttrib(TextAttribFromAttrib(ConInfo->PopupAttributes), colorIndex);
                    }

                    InvalidateRect(GetDlgItem(hDlg, IDC_STATIC_COLOR1 + ActiveStaticControl), NULL, TRUE);
                    ActiveStaticControl = colorIndex;
                    InvalidateRect(GetDlgItem(hDlg, IDC_STATIC_COLOR1 + ActiveStaticControl), NULL, TRUE);
                    InvalidateRect(GetDlgItem(hDlg, IDC_STATIC_SCREEN_COLOR), NULL, TRUE);
                    InvalidateRect(GetDlgItem(hDlg, IDC_STATIC_POPUP_COLOR) , NULL, TRUE);

                    PropSheet_Changed(GetParent(hDlg), hDlg);
                    break;
                }
            }
            else if (HIWORD(wParam) == EN_KILLFOCUS)
            {
                switch (LOWORD(wParam))
                {
                case IDC_EDIT_COLOR_RED:
                {
                    DWORD red;

                    /* Get the current color */
                    colorIndex = ActiveStaticControl;
                    color = ConInfo->ColorTable[colorIndex];

                    red = GetDlgItemInt(hDlg, IDC_EDIT_COLOR_RED, NULL, FALSE);
                    red = min(max(red, 0), 255);

                    color = RGB(red, GetGValue(color), GetBValue(color));

                    ConInfo->ColorTable[colorIndex] = color;
                    InvalidateRect(GetDlgItem(hDlg, IDC_STATIC_COLOR1 + colorIndex), NULL, TRUE);
                    InvalidateRect(GetDlgItem(hDlg, IDC_STATIC_SCREEN_COLOR), NULL, TRUE);
                    InvalidateRect(GetDlgItem(hDlg, IDC_STATIC_POPUP_COLOR) , NULL, TRUE);

                    PropSheet_Changed(GetParent(hDlg), hDlg);
                    break;
                }

                case IDC_EDIT_COLOR_GREEN:
                {
                    DWORD green;

                    /* Get the current color */
                    colorIndex = ActiveStaticControl;
                    color = ConInfo->ColorTable[colorIndex];

                    green = GetDlgItemInt(hDlg, IDC_EDIT_COLOR_GREEN, NULL, FALSE);
                    green = min(max(green, 0), 255);

                    color = RGB(GetRValue(color), green, GetBValue(color));

                    ConInfo->ColorTable[colorIndex] = color;
                    InvalidateRect(GetDlgItem(hDlg, IDC_STATIC_COLOR1 + colorIndex), NULL, TRUE);
                    InvalidateRect(GetDlgItem(hDlg, IDC_STATIC_SCREEN_COLOR), NULL, TRUE);
                    InvalidateRect(GetDlgItem(hDlg, IDC_STATIC_POPUP_COLOR) , NULL, TRUE);

                    PropSheet_Changed(GetParent(hDlg), hDlg);
                    break;
                }

                case IDC_EDIT_COLOR_BLUE:
                {
                    DWORD blue;

                    /* Get the current color */
                    colorIndex = ActiveStaticControl;
                    color = ConInfo->ColorTable[colorIndex];

                    blue = GetDlgItemInt(hDlg, IDC_EDIT_COLOR_BLUE, NULL, FALSE);
                    blue = min(max(blue, 0), 255);

                    color = RGB(GetRValue(color), GetGValue(color), blue);

                    ConInfo->ColorTable[colorIndex] = color;
                    InvalidateRect(GetDlgItem(hDlg, IDC_STATIC_COLOR1 + colorIndex), NULL, TRUE);
                    InvalidateRect(GetDlgItem(hDlg, IDC_STATIC_SCREEN_COLOR), NULL, TRUE);
                    InvalidateRect(GetDlgItem(hDlg, IDC_STATIC_POPUP_COLOR) , NULL, TRUE);

                    PropSheet_Changed(GetParent(hDlg), hDlg);
                    break;
                }
                }
            }

            break;
        }

        default:
            break;
    }

    return FALSE;
}
