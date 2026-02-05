#include "wndproc.hpp"
#include "system/logger.hpp"

bool WndProcHook::Initialize()
{
    if (!hwnd_) {
        LOG_FATAL("WndProcHook: invalid HWND.");
        return false;
    }

    s_self_ = this;

    ::SetLastError(0);
    auto prev = reinterpret_cast<WNDPROC>(
        ::SetWindowLongPtr(hwnd_, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&StaticWndProc))
    );

    if (!prev && ::GetLastError() != 0) {
        LOG_FATAL("Failed to install WndProc hook (err={}).", ::GetLastError());
        s_self_ = nullptr;
        return false;
    }

    baseProc_ = prev; // real original at time of first hook
    nextProc_ = prev; // chain target initially

    LOG_DEBUG("WndProc hook installed successfully.");
    return true;
}

void WndProcHook::EnsureInstalled()
{
    if (!hwnd_)
        return;

    const DWORD now = ::GetTickCount();
    if (now - lastEnsureTick_ < 250)
        return;

    lastEnsureTick_ = now;

    const auto current = reinterpret_cast<WNDPROC>(::GetWindowLongPtr(hwnd_, GWLP_WNDPROC));
    if (!current)
        return;

    if (current == reinterpret_cast<WNDPROC>(&StaticWndProc))
        return;

    nextProc_ = current;

    ::SetLastError(0);
    ::SetWindowLongPtr(hwnd_, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&StaticWndProc));

    LOG_WARN("[WndProcHook] WndProc was replaced -> hook reinstalled (nextProc={}).", (void*)nextProc_);
}

void WndProcHook::Shutdown()
{
    if (!hwnd_)
        return;

    const auto current = reinterpret_cast<WNDPROC>(::GetWindowLongPtr(hwnd_, GWLP_WNDPROC));

    if (current == reinterpret_cast<WNDPROC>(&StaticWndProc))
    {
        // Restore to whatever was current when we last ensured (usually SA-MP proc).
        auto restore = nextProc_ ? nextProc_ : baseProc_;
        if (restore)
            ::SetWindowLongPtr(hwnd_, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(restore));
    }

    hwnd_ = nullptr;
    baseProc_ = nullptr;
    nextProc_ = nullptr;
    s_self_ = nullptr;

    LOG_DEBUG("WndProc hook uninstalled successfully.");
}

LRESULT CALLBACK WndProcHook::StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    auto* self = s_self_;

    // If SA-MP's proc calls back into our proc (because it saved us as "prev"),
    // we must NOT call nextProc_ again, or we recurse infinitely.
    static thread_local int depth = 0;
    const bool reentered = (depth++ > 0);

    // Only run our handler
    if (!reentered && self && self->OnMessage)
    {
        const LRESULT result = self->OnMessage(hwnd, msg, wParam, lParam);
        if (result != 0) { 
            depth--; 
            return result; 
        }
    }

    WNDPROC target = nullptr;

    if (self)
        target = reentered ? self->baseProc_ : self->nextProc_;

    const LRESULT out = ::CallWindowProc(target ? target : ::DefWindowProc, hwnd, msg, wParam, lParam);

    depth--;
    return out;
}