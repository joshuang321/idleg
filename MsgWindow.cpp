#include "MsgFrameWindow.hpp"
#include "Shared.hpp"

#include <Windows.h>
#include <cassert>

static LPCWSTR g_lpszWindowName = LPSZMSGWNDNAME;

static HANDLE g_hFrameWndThread{};
static DWORD g_dwIdThread{};
static UINT_PTR g_uIdTimer{};
static HHOOK g_hllMouseHook{}, g_hllKeyboardHook{};
static FrameData g_FrameData{ L"okayeg.png", SEC_DEFAULT, NBFRM_DEFAULT };
static HINSTANCE g_hInst{};

static HWND CreateMsgWindow();
static LRESULT MsgWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static void InitHooks();
static void UnInitHooks();
static void CreateFrameWindowThread();
static void DestroyFrameWindowThread();

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nShowCmd);

    MSG Msg;
    g_hInst = hInstance;

    InitHooks();
    GetFrameData(&g_FrameData);
    g_FrameData.nSeconds = MILSECONDS(g_FrameData.nSeconds);

    HWND hWnd = CreateMsgWindow();
    assert(hWnd);
    g_uIdTimer = SetTimer(hWnd, (UINT_PTR)1, g_FrameData.nSeconds, NULL);
    assert(g_uIdTimer > 0);

    while (GetMessageW(&Msg, NULL, 0, 0))
    {
        TranslateMessage(&Msg);
        DispatchMessageW(&Msg);
    }
    UnInitHooks();

    return 0;
}

static HWND CreateMsgWindow()
{
    WNDCLASSEXW wcex{};
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.lpfnWndProc = MsgWindowProc;
    wcex.hInstance = g_hInst;
    wcex.lpszClassName = g_lpszWindowName;

    ATOM aClassName = RegisterClassExW(&wcex);
    assert(aClassName);

    return CreateWindowExW(0, (LPCWSTR)(ULONG_PTR)aClassName, g_lpszWindowName, 0 , 0, 0, 0, 0,
        HWND_MESSAGE, NULL,
        g_hInst, NULL);
}

#include <string>

static LRESULT MsgWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static int nCount{};
    switch (uMsg)
    {
    case WM_CREATE:
        CreateFrameWindowThread();
        return 0;
    
    case WM_IDLEFOCUS:
        nCount = 0;
        PostThreadMessageW(g_dwIdThread, WM_CLEARFRAMES, (WPARAM)NULL, (LPARAM)NULL);
        SetTimer(hWnd, g_uIdTimer, g_FrameData.nSeconds, NULL);
        return 0;

    case WM_COPYDATA:
    {
        COPYDATASTRUCT *pCpyDS = reinterpret_cast<COPYDATASTRUCT*>(lParam);
        FrameData* pFrameData = reinterpret_cast<FrameData*>(pCpyDS->lpData);
        g_FrameData.nFrames = pFrameData->nFrames;
        g_FrameData.nSeconds = MILSECONDS(pFrameData->nSeconds);
        wcscpy(g_FrameData.lpszFilename_, pFrameData->lpszFilename_);
        PostThreadMessageW(g_dwIdThread, WM_UPDATEIMAGE, (WPARAM)NULL, (LPARAM)NULL);

        SetTimer(hWnd, g_uIdTimer, g_FrameData.nSeconds, NULL);
    }
        return 0;

    case WM_TIMER:
        if (nCount < g_FrameData.nFrames)
        {
            nCount++;
            PostThreadMessageW(g_dwIdThread, WM_CREATEFRAME, (WPARAM)NULL, (LPARAM)NULL);
        }
        return 0;
    
    case WM_DESTROY:
        DestroyFrameWindowThread();
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

static void InitHooks()
{
    HMODULE hLib = LoadLibraryW(L"idleg-dll.dll");
    assert(hLib);
    HOOKPROC hookProc = reinterpret_cast<HOOKPROC>(GetProcAddress(hLib, "IdleProc"));
    g_hllMouseHook = SetWindowsHookExW(WH_MOUSE_LL, hookProc, hLib, 0);
    g_hllKeyboardHook = SetWindowsHookExW(WH_KEYBOARD_LL, hookProc, hLib, 0);
}

static void UnInitHooks()
{
    UnhookWindowsHookEx(g_hllMouseHook);
    UnhookWindowsHookEx(g_hllKeyboardHook);
}

static void CreateFrameWindowThread()
{
    HANDLE hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
    assert(hEvent);
    ThreadCreateInfo tInfo{ hEvent, g_hInst, &g_FrameData };

    g_hFrameWndThread = CreateThread(NULL, 0, FrameWindowThreadProc, reinterpret_cast<LPVOID>(&tInfo),
        0, &g_dwIdThread);
    assert(g_hFrameWndThread);

    assert(WAIT_OBJECT_0 == WaitForSingleObject(hEvent, INFINITE));
    CloseHandle(hEvent);
}

static void DestroyFrameWindowThread()
{
    HANDLE hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
    assert(hEvent);
    PostThreadMessageW(g_dwIdThread, WM_THREADQUIT, (WPARAM)NULL, (LPARAM)hEvent);

    assert(WAIT_OBJECT_0 == WaitForSingleObject(hEvent, INFINITE));
    CloseHandle(hEvent);
}