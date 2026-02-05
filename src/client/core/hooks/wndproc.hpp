#pragma once

#include <windows.h>
#include <functional>

class WndProcHook
{
public:
    explicit WndProcHook(HWND hwnd) : hwnd_(hwnd) {}
    ~WndProcHook() = default;

    WndProcHook(const WndProcHook&) = delete;
    WndProcHook& operator=(const WndProcHook&) = delete;

    bool Initialize();
    void Shutdown();
    void EnsureInstalled();

    std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> OnMessage;

private:
    static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HWND hwnd_ = nullptr;

    // The REAL window proc before we ever hooked (GTA's original proc).
    WNDPROC baseProc_ = nullptr;

    // The proc currently installed when we last ensured (often SA-MP proc).
    WNDPROC nextProc_ = nullptr;

    DWORD lastEnsureTick_ = 0;

    static inline WndProcHook* s_self_ = nullptr;
};