#include "MsgFrameWindow.hpp"
#include "Shared.hpp"

#include <vector>
#include <objidl.h>
#include <Gdiplus.h>
#include <cassert>

struct WindowFrame
{
    HWND hWnd;
    POINT pt, ptDir;
};

static LPCWSTR g_lpszWindowName = L"Idleg";
static const Gdiplus::Color g_Alpha(0, 0, 0, 0);
static const POINT g_ptZero{};
static const BLENDFUNCTION g_blendFunc = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };

static std::vector<WindowFrame> g_hWndFrames;
static SIZE g_ptSz;
static MONITORINFO mi{ sizeof(MONITORINFO) };

static HDC g_hDCCompat{};
static HBITMAP g_hBitmapImage{};

static ULONG_PTR g_GdiplusToken{};
static HINSTANCE g_hInst{};

static ATOM RegisterIdlegWindow();
static LRESULT IdlegWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static void LoadNewImage(_In_z_ LPCWSTR lpszImageFilename);
static void CreateNewFrame(ATOM aClassName);
static void DestroyFrames();
static void UpdateFrames();
static void DestroyThread();

DWORD WINAPI FrameWindowThreadProc(_In_ LPVOID lpParameter)
{
    MSG Msg;
    ThreadCreateInfo* pTInfo = reinterpret_cast<ThreadCreateInfo*>(lpParameter);
    Gdiplus::GdiplusStartupInput gdiStartupInput{};
    ATOM aClassName{};
    FrameData *pFrameData = pTInfo->pFrameData;

    g_hInst = pTInfo->hInstance;
    aClassName = RegisterIdlegWindow();
    assert(aClassName);

    assert(Gdiplus::GdiplusStartup(&g_GdiplusToken, &gdiStartupInput, NULL) == Gdiplus::Ok);
    LoadNewImage(pFrameData->lpszFilename_);

    GetMonitorInfoW(MonitorFromPoint({0, 0}, MONITOR_DEFAULTTOPRIMARY), &mi);
    
    PeekMessage(&Msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
    SetEvent(pTInfo->hEvent);

    while (true)
    {
        if (PeekMessageW(&Msg, NULL, 0, 0, PM_REMOVE))
        {

            if (WM_THREADQUIT == Msg.message)
            {
                DestroyThread();
                SetEvent(reinterpret_cast<HANDLE>(Msg.lParam));
                break;
            }
            else if (WM_CREATEFRAME == Msg.message)
                CreateNewFrame(aClassName);
            else if (WM_CLEARFRAMES == Msg.message)
                DestroyFrames();
            else
                LoadNewImage(pFrameData->lpszFilename_);
        }
        else
            UpdateFrames();
    }
    return (DWORD)Msg.wParam;
}

static ATOM RegisterIdlegWindow()
{
    WNDCLASSEXW wcex{};
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.hInstance = g_hInst;
    wcex.lpfnWndProc = IdlegWindowProc;
    wcex.lpszClassName = g_lpszWindowName;

    return RegisterClassExW(&wcex);
}

static void LoadNewImage(_In_z_ LPCWSTR lpszImageFilename)
{
    Gdiplus::Bitmap image(lpszImageFilename);
    
    assert(image.GetLastStatus() == Gdiplus::Status::Ok);
    g_ptSz.cx = min(image.GetWidth(), BFRM_DEFAULT);
    g_ptSz.cy = min(image.GetHeight(), BFRM_DEFAULT);

    if (g_hDCCompat)
    {
        HBITMAP hbmpTemp{};
        image.GetHBITMAP(g_Alpha, &hbmpTemp);

        DeleteObject(SelectObject(g_hDCCompat, hbmpTemp));
    }
    else
    {
        HDC hDCScreen = GetDC(NULL);
        assert(hDCScreen);
        g_hDCCompat = CreateCompatibleDC(hDCScreen);
        image.GetHBITMAP(g_Alpha, &g_hBitmapImage);

        g_hBitmapImage = (HBITMAP)SelectObject(g_hDCCompat, g_hBitmapImage);
        assert(g_hBitmapImage);
        ReleaseDC(NULL, g_hDCCompat);
    }
}

static void CreateNewFrame(ATOM aClassName)
{
    WindowFrame winFrame{};
    winFrame.hWnd =  CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED,
            (LPCWSTR)(DWORD64)aClassName,
            NULL,
            WS_VISIBLE,
            0, 0,
            g_ptSz.cx, g_ptSz.cy,
            NULL,
            NULL,
            g_hInst,
            NULL);

    winFrame.ptDir.x = 1;
    winFrame.ptDir.y = 1;
    // Set random location on some corner on screen
    g_hWndFrames.push_back(winFrame);
}

static void DestroyFrames()
{
    for (WindowFrame winFrame : g_hWndFrames)
        DestroyWindow(winFrame.hWnd);
    g_hWndFrames.clear();
}

static void UpdateFrames()
{
    HDC hDCScreen = GetDC(NULL);

    for (WindowFrame& winFrame : g_hWndFrames)
    {
        if (winFrame.pt.x+g_ptSz.cx == mi.rcMonitor.right)
            winFrame.ptDir.x = -1;
        else if (winFrame.pt.x == 0)
            winFrame.ptDir.x = 1;
                
        if (winFrame.pt.y+g_ptSz.cy == mi.rcMonitor.bottom)
            winFrame.ptDir.y = -1;
        else if (winFrame.pt.y == 0)
            winFrame.ptDir.y = 1;

        winFrame.pt.x += winFrame.ptDir.x;
        winFrame.pt.y += winFrame.ptDir.y;
        
        UpdateLayeredWindow(winFrame.hWnd, hDCScreen, &(winFrame.pt),
            (SIZE*)&g_ptSz,
            g_hDCCompat,
            (POINT*)&g_ptZero,
            0,
            const_cast<BLENDFUNCTION*>(&g_blendFunc),
            ULW_ALPHA);
    }
    ReleaseDC(NULL, hDCScreen);
    Sleep(5);
}

static void DestroyThread()
{
    DestroyFrames();
    if (g_hDCCompat &&
        g_hBitmapImage)
    {
        DeleteObject(SelectObject(g_hDCCompat, g_hBitmapImage));
        DeleteDC(g_hDCCompat);
    }
    Gdiplus::GdiplusShutdown(g_GdiplusToken);
}

static LRESULT IdlegWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
    {
        DWORD dwStyles = (DWORD)GetWindowLongW(hWnd, GWL_STYLE);
        SetWindowLongW(hWnd, GWL_STYLE, dwStyles & ~WS_OVERLAPPEDWINDOW);
        SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOOWNERZORDER | SWP_FRAMECHANGED
            | SWP_NOSIZE | SWP_NOMOVE);
    }
    return 0;
    }
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}