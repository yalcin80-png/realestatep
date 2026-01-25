#pragma once
#include "wxx_wincore.h"
#include <functional>
#include <string>
#include <wrl.h>
#include "WebView2.h"

// Callback tipi
using SourceCallback = std::function<void(std::wstring)>;

class CSimpleBrowser : public CWnd
{
public:
    CSimpleBrowser();
    virtual ~CSimpleBrowser() noexcept;

    void InitBrowser(HWND hParent);
    void Navigate(const CString& url);

    // HTML Kaynağını Alır
    void GetSourceCode(SourceCallback callback);

    // YENİ: İlanın "data-json" verisini doğrudan çeker (En garantili yöntem)
    void GetListingJSON(SourceCallback callback);


    // YENİ: JavaScript Kodu Çalıştırma (Scroll vb. işlemler için)
    void ExecuteScript(const std::wstring& script, std::function<void()> onComplete = nullptr);

    // YENİ: JavaScript sonucu string olarak döndür (Bulk Import için)
    void ExecuteScriptGetResult(const std::wstring& script, SourceCallback callback);
protected:
    virtual void OnDestroy();
    virtual void OnSize(int width, int height);
    virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    Microsoft::WRL::ComPtr<ICoreWebView2Controller> m_controller;
    Microsoft::WRL::ComPtr<ICoreWebView2> m_webView;
    CString m_pendingUrl;
};