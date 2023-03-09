#include "Shared.hpp"

#include <Windows.h>

#define IDLE_EXPORT extern "C" __declspec(dllexport)

IDLE_EXPORT LRESULT CALLBACK IdleProc(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
    HWND hWndMsg = FindWindowExW(HWND_MESSAGE, NULL, LPSZMSGWNDNAME, NULL);
    PostMessageW(hWndMsg, WM_IDLEFOCUS, (WPARAM)0, (LPARAM)0);

    return CallNextHookEx(0L, nCode, wParam, lParam);
}