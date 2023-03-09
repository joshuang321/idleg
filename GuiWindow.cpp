#include "BFrame.hpp"
#include "FrameData.hpp"
#include "Shared.hpp"

#include "resource.h"

#include <Windows.h>
#include <CommCtrl.h>
#include <cassert>

#define WINDOW_WIDTH    300
#define WINDOW_HEIGHT   325
#define WINDOW_STYLE    WS_VISIBLE | WS_CAPTION | WS_SYSMENU

#define CTL_OFFSET      11
#define CTL_BOFFSET     7
#define CTL_BOFFSET2    11
#define CHF_OFFSET      30

#define IDM_CHOOSEFILE  0x100
#define IDM_OK          0x101
#define IDM_CANCEL      0x102


LPCWSTR g_lpszWindowName = L"idleg-gui";

HFONT g_hStdFont{};
HINSTANCE g_hInst{};

HWND CreateAppWindow(void);
LRESULT MainWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void CreateAppFont(void);

void CreateWindowText(HWND hWndParent, int x, int y, _In_z_ LPCWSTR lpszString);
HWND CreateUpDownControl(HWND hWndParent, int y, int nMin, int nMax, int nInitial);
void CreateChooseFileControl(HWND hWndParent, int y);
HWND CreateBouncingFrameControl(HWND hWndParent, int y);
void CreateCommandsControl(HWND hWndParent);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nShowCmd);

    const INITCOMMONCONTROLSEX icex{ sizeof(INITCOMMONCONTROLSEX), ICC_UPDOWN_CLASS };
    MSG Msg;
    Gdiplus::GdiplusStartupInput gdiStartupInput{};
    ULONG_PTR gdiplusToken{};

    g_hInst = hInstance;
    assert(Gdiplus::GdiplusStartup(&gdiplusToken, &gdiStartupInput, NULL) == Gdiplus::Ok);
    assert(InitCommonControlsEx(&icex));

    HWND hWnd = CreateAppWindow();
    assert(hWnd);

    while(GetMessageW(&Msg, NULL, 0, 0))
    {
        TranslateMessage(&Msg);
        DispatchMessageW(&Msg);
    }

    Gdiplus::GdiplusShutdown(gdiplusToken);
    return 0;
}

HWND CreateAppWindow(void)
{
    RECT rcIdeal{ 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    WNDCLASSEXW wcex{};
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.lpfnWndProc = MainWindowProc;
    wcex.hInstance = g_hInst;
    wcex.hIcon = LoadIconW(g_hInst, MAKEINTRESOURCEW(IDI_APP));
    wcex.hCursor = LoadCursorW(NULL, MAKEINTRESOURCEW(IDC_ARROW));
    wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wcex.lpszClassName = g_lpszWindowName;
    wcex.hIconSm = LoadIconW(g_hInst, MAKEINTRESOURCEW(IDI_APP));

    ATOM aClassName = RegisterClassExW(&wcex);
    assert(aClassName);
    assert(AdjustWindowRect(&rcIdeal, WINDOW_STYLE, FALSE));

    return CreateWindowExW(0, (LPCWSTR)(ULONG_PTR)aClassName ,g_lpszWindowName, WINDOW_STYLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        (int)rcIdeal.right -rcIdeal.left, (int)rcIdeal.bottom - rcIdeal.top,
        NULL, NULL,
        g_hInst, NULL);
}

LRESULT MainWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static BFrame* pbFrame{};
    static HWND hWndBouncingFrame{}, 
        hWndEditSeconds{}, hWndEditNumFrames{};
    
    switch (uMsg)
    {
    case WM_CREATE:
    {
        FrameData frameData{ L"okayeg.png", NBFRM_DEFAULT, SEC_DEFAULT };
        GetFrameData(&frameData);
        CreateAppFont();
        CreateWindowText(hWnd, CTL_OFFSET, CTL_OFFSET+4, L"Timeout (in seconds):");
        hWndEditSeconds = CreateUpDownControl(hWnd, CTL_OFFSET, TMO_SEC_MIN, TMO_SEC_MAX, frameData.nSeconds);
        
        CreateWindowText(hWnd, CTL_OFFSET, CTL_OFFSET + CTL_BOFFSET + 23, L"Max number of bouncing frames:");
        hWndEditNumFrames = CreateUpDownControl(hWnd, CTL_OFFSET + CTL_BOFFSET + 23, NFRM_MIN, NFRM_MAX, frameData.nFrames);
        
        CreateWindowText(hWnd, CTL_OFFSET, CTL_OFFSET + CTL_BOFFSET + CHF_OFFSET + 50, L"Bouncing Frame");
        CreateChooseFileControl(hWnd, CTL_OFFSET + CTL_BOFFSET + CHF_OFFSET + 46);
        hWndBouncingFrame = CreateBouncingFrameControl(hWnd, CTL_OFFSET + CTL_BOFFSET + CTL_BOFFSET2 + CHF_OFFSET + 69);
        
        CreateCommandsControl(hWnd);
        pbFrame = new BFrame{ frameData.lpszFilename_ };
    }
        return 0;
    
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDM_CHOOSEFILE:
            pbFrame->ChangeFilename(hWnd);
            InvalidateRect(hWndBouncingFrame, NULL, TRUE);
            break;

        case IDM_OK:
        {
            WCHAR text[4];
            FrameData frameData;
            COPYDATASTRUCT cpyDS { 1, sizeof(FrameData), &frameData };

            pbFrame->getFilename(frameData.lpszFilename_);
            GetWindowTextW(hWndEditNumFrames, text, 4);
            
            frameData.nFrames = _wtoi(text);
            frameData.nFrames = max(frameData.nFrames, NFRM_MIN);
            frameData.nFrames = min(frameData.nFrames, NFRM_MAX);
            
            GetWindowTextW(hWndEditSeconds, text, 4);
            frameData.nSeconds = _wtoi(text);
            frameData.nSeconds = max(frameData.nSeconds, TMO_SEC_MIN);
            frameData.nSeconds = min(frameData.nSeconds, TMO_SEC_MAX);

            SaveFrameData(&frameData);

            HWND hWndMsg = FindWindowExW(HWND_MESSAGE, NULL, LPSZMSGWNDNAME, NULL);
            if (hWndMsg)
                SendMessageW(hWndMsg, WM_COPYDATA, (WPARAM)hWnd, reinterpret_cast<LPARAM>(&cpyDS));
        }
        case IDM_CANCEL:
            PostQuitMessage(0);
            break;
        }
        return 0;

    case WM_DRAWITEM:
        pbFrame->PaintWindow(reinterpret_cast<LPDRAWITEMSTRUCT>(lParam));
        return 0;

    case WM_CTLCOLORSTATIC: return (LRESULT)GetStockObject(WHITE_BRUSH);
        
    case WM_DESTROY:
        PostQuitMessage(0);
        if (pbFrame)
            delete pbFrame;
        if (g_hStdFont)
            DeleteObject(g_hStdFont);
        return 0;
    }
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

void CreateAppFont(void)
{
    const LOGFONTW lgStdFont = { .lfHeight=-12, .lfWeight=400, .lfOutPrecision=3, .lfClipPrecision=2,
        .lfQuality=1, .lfPitchAndFamily=34, .lfFaceName = L"MS Shell Dlg" };
    if (!g_hStdFont)
    {
        g_hStdFont = CreateFontIndirectW(&lgStdFont);
        assert(g_hStdFont);
    }
}

void CreateWindowText(HWND hWndParent, int x, int y, _In_z_ LPCWSTR lpszString)
{
    assert(g_hStdFont);
    HWND hWndText = CreateWindowExW(0, WC_STATICW, lpszString, WS_VISIBLE | WS_CHILD,
        x, y,
        0, 0,
        hWndParent, NULL,
        g_hInst, NULL);
    assert(hWndText);

    SIZE szWnd;
    HDC hDC = GetDC(hWndText);
    HFONT hTempFont = (HFONT)SelectObject(hDC, g_hStdFont);
    assert(GetTextExtentPoint32W(hDC, lpszString, (int)wcslen(lpszString), &szWnd));
    SelectObject(hDC, hTempFont);
    ReleaseDC(hWndText, hDC);
    assert(SetWindowPos(hWndText, NULL, 0, 0, szWnd.cx, szWnd.cy, SWP_NOMOVE | SWP_NOZORDER));
    SendMessageW(hWndText, WM_SETFONT, (WPARAM)g_hStdFont, (LPARAM)TRUE);
}

HWND CreateUpDownControl(HWND hWndParent, int y, int nMin, int nMax, int nIntial)
{
    HWND hWndEdit = CreateWindowExW(WS_EX_CLIENTEDGE, WC_EDITW, NULL, WS_VISIBLE | WS_CHILD | WS_BORDER
        | ES_NUMBER | ES_LEFT | ES_READONLY,
        WINDOW_WIDTH - CTL_OFFSET - 74, y, 74, 23,
        hWndParent, NULL,
        g_hInst, NULL);
    assert(hWndEdit);
    SendMessageW(hWndEdit, WM_SETFONT, (WPARAM)g_hStdFont, (LPARAM)TRUE);
    HWND hWndUpDown = CreateWindowExW(0, UPDOWN_CLASSW, NULL, WS_VISIBLE | WS_CHILD |
        UDS_AUTOBUDDY | UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_HOTTRACK,
        0, 0, 0, 0,
        hWndParent, NULL,
        g_hInst, NULL);
    assert(hWndUpDown);
    SendMessageW(hWndUpDown, UDM_SETRANGE, 0, MAKELPARAM(nMin, nMax));
    SendMessageW(hWndUpDown, UDM_SETPOS, 0, (LPARAM)nIntial);

    return hWndEdit;
}

void CreateChooseFileControl(HWND hWndParent, int y)
{
    SIZE szWnd{};
    HWND hWndChooseFile = CreateWindowExW(0, WC_BUTTONW, L"Choose File...", WS_VISIBLE | WS_CHILD | BS_CENTER,
       0, 0, 0, 0,
       hWndParent, (HMENU)IDM_CHOOSEFILE,
       g_hInst, NULL);
    assert(hWndChooseFile);
    SendMessageW(hWndChooseFile, WM_SETFONT, (WPARAM)g_hStdFont, (LPARAM)TRUE);
    SendMessageW(hWndChooseFile, BCM_GETIDEALSIZE, (WPARAM)0, (LPARAM)&szWnd);
    szWnd.cx = (int)((float)szWnd.cx * 1.3);

    SetWindowPos(hWndChooseFile, NULL, WINDOW_WIDTH - CTL_OFFSET - szWnd.cx, y, szWnd.cx, 23,
        SWP_NOZORDER);
}

HWND CreateBouncingFrameControl(HWND hWndParent, int y)
{
    HWND hWndBFrame = CreateWindowExW(0, WC_STATICW, NULL, WS_VISIBLE | WS_CHILD | WS_BORDER |
        SS_OWNERDRAW,
        0, 0, 0, 0,
        hWndParent, NULL,
        g_hInst, NULL);
    assert(hWndBFrame);
    SetWindowPos(hWndBFrame, NULL, (WINDOW_WIDTH - BFRM_DEFAULT)/2, y,
        BFRM_DEFAULT, BFRM_DEFAULT, SWP_NOZORDER);

    return hWndBFrame;
}

void CreateCommandsControl(HWND hWndParent)
{
    SIZE szWnd{};
    HWND hWndOk = CreateWindowExW(0, WC_BUTTONW, L"Ok",  WS_VISIBLE | WS_CHILD | BS_CENTER,
        WINDOW_WIDTH - 150 - CTL_BOFFSET - CTL_OFFSET, WINDOW_HEIGHT - 23 - CTL_OFFSET, 75, 23,
        hWndParent, (HMENU)IDM_OK,
        g_hInst, NULL);
    assert(hWndOk);
    HWND hWndCancel = CreateWindowExW(0, WC_BUTTONW, L"Cancel", WS_VISIBLE | WS_CHILD | BS_CENTER,
        WINDOW_WIDTH - 75 - CTL_OFFSET, WINDOW_HEIGHT - 23 - CTL_OFFSET, 75, 23,
        hWndParent, (HMENU)IDM_CANCEL,
        g_hInst, NULL);
    assert(hWndCancel);
    
    SendMessageW(hWndOk, WM_SETFONT, (WPARAM)g_hStdFont, (LPARAM)TRUE);
    SendMessageW(hWndCancel, WM_SETFONT, (WPARAM)g_hStdFont, (LPARAM)TRUE);
}