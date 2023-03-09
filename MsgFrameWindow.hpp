#include "FrameData.hpp"

#include <Windows.h>

#define WM_THREADQUIT   WM_USER+0x0
#define WM_CREATEFRAME  WM_USER+0x1
#define WM_CLEARFRAMES  WM_USER+0x2
#define WM_UPDATEIMAGE  WM_USER+0x3

struct ThreadCreateInfo
{
    HANDLE hEvent;
    HINSTANCE hInstance;
    FrameData *pFrameData;
};

DWORD WINAPI FrameWindowThreadProc(_In_ LPVOID lpParameter);