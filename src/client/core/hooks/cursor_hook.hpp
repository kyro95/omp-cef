#pragma once
#include <windows.h>
#include <atomic>

class HookManager;

class CursorHook
{
public:
    static CursorHook& Instance() noexcept
    {
        static CursorHook inst;
        return inst;
    }

    void Initialize(HookManager& hooks);
    void Shutdown(HookManager& hooks);

    void SetForced(bool enabled) noexcept { forced_.store(enabled, std::memory_order_release); }
    bool IsForced() const noexcept { return forced_.load(std::memory_order_acquire); }

    void SetForcedCursor(HCURSOR cur) noexcept { forced_cursor_.store(cur, std::memory_order_release); }
    HCURSOR GetForcedCursor() const noexcept { return forced_cursor_.load(std::memory_order_acquire); }

private:
    CursorHook() = default;

    using SetCursor_t = HCURSOR (WINAPI*)(HCURSOR);

    static HCURSOR WINAPI hkSetCursor(HCURSOR hCursor);

private:
    SetCursor_t orig_set_cursor_ = nullptr;

    std::atomic<bool> forced_{ false };
    std::atomic<HCURSOR> forced_cursor_{ nullptr };
};
