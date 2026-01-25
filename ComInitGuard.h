#pragma once
#ifndef _WIN32
#error ComInitGuard is Windows-only
#endif

#include <windows.h>

// Simple RAII guard for COM initialization on the current thread.
// NOTE: COM is thread-affine. ADO uses COM, so every thread that touches ADO must be initialized.
class ComInitGuard
{
public:
    explicit ComInitGuard(DWORD coinit = COINIT_APARTMENTTHREADED) noexcept
    {
        hr_ = ::CoInitializeEx(nullptr, coinit);
    }

    ~ComInitGuard() noexcept
    {
        if (hr_ == S_OK || hr_ == S_FALSE)
            ::CoUninitialize();
    }

    HRESULT Result() const noexcept { return hr_; }

private:
    HRESULT hr_{ E_FAIL };
};
