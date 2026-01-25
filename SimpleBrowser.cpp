#include "stdafx.h"
#include "SimpleBrowser.h"
#include <shlobj.h> 

#pragma comment(lib, "WebView2Loader.dll.lib")
#include "json.hpp"
using json = nlohmann::json;
using namespace Microsoft::WRL;

// WebView2 ExecuteScript sonucu JSON string olarak döner.
// Burada gerçek string'e decode ederiz (\u003C, \n, \\" vs.).
static std::wstring DecodeWebView2JsonString(LPCWSTR resultObjectAsJson)
{
    if (!resultObjectAsJson) return L"";

    std::wstring wjson(resultObjectAsJson);
    if (wjson.empty()) return L"";

    // Wide -> UTF-8
    int len = WideCharToMultiByte(CP_UTF8, 0, wjson.data(), (int)wjson.size(), NULL, 0, NULL, NULL);
    if (len <= 0) return L"";
    std::string utf8(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wjson.data(), (int)wjson.size(), utf8.data(), len, NULL, NULL);

    try {
        // JSON string -> std::string (UTF-8)
        json j = json::parse(utf8);
        if (!j.is_string()) return L"";
        std::string s = j.get<std::string>();

        // UTF-8 -> Wide
        int wlen = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), NULL, 0);
        if (wlen <= 0) return L"";
        std::wstring out(wlen, 0);
        MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), &out[0], wlen);
        return out;
    }
    catch (...) {
        return L"";
    }
}

CSimpleBrowser::CSimpleBrowser() {}

CSimpleBrowser::~CSimpleBrowser() noexcept {
    if (m_controller) m_controller->Close();
}

void CSimpleBrowser::InitBrowser(HWND hParent)
{
    Create(hParent);

    wchar_t tempPath[MAX_PATH];
    ::GetTempPath(MAX_PATH, tempPath);
    std::wstring userDataFolder = std::wstring(tempPath) + L"SahibindenBrowserData";

    CreateCoreWebView2EnvironmentWithOptions(nullptr, userDataFolder.c_str(), nullptr,
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [this](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
                if (FAILED(result)) {
                    ::MessageBox(NULL, _T("WebView2 Runtime Başlatılamadı!"), _T("Hata"), MB_ICONERROR);
                    return result;
                }
                env->CreateCoreWebView2Controller(GetHwnd(), Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                    [this](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
                        if (controller != nullptr) {
                            m_controller = controller;
                            m_controller->get_CoreWebView2(&m_webView);
                            if (m_webView) {
                                Microsoft::WRL::ComPtr<ICoreWebView2Settings> settings;
                                m_webView->get_Settings(&settings);
                                if (settings) {
                                    settings->put_IsScriptEnabled(TRUE);
                                    settings->put_AreDefaultScriptDialogsEnabled(TRUE);
                                }
                            }
                            RECT bounds;
                            ::GetClientRect(GetHwnd(), &bounds);
                            m_controller->put_Bounds(bounds);

                            if (!m_pendingUrl.IsEmpty()) {
                                std::wstring wUrl(m_pendingUrl);
                                m_webView->Navigate(wUrl.c_str());
                                m_pendingUrl.Empty();
                            }
                        }
                        return S_OK;
                    }).Get());
                return S_OK;
            }).Get());
}

void CSimpleBrowser::Navigate(const CString& url)
{
    if (m_webView) {
        std::wstring wUrl(url);
        m_webView->Navigate(wUrl.c_str());
    }
    else {
        m_pendingUrl = url;
    }
}

void CSimpleBrowser::GetSourceCode(SourceCallback callback)
{
    if (!m_webView) { callback(L""); return; }
    m_webView->ExecuteScript(L"document.documentElement.outerHTML",
        Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
            [callback](HRESULT errorCode, LPCWSTR resultObjectAsJson) -> HRESULT {
                callback(DecodeWebView2JsonString(resultObjectAsJson));
                return S_OK;
            }).Get());
}

// --- YENİ FONKSİYON: Doğrudan Veri Çekme ---
void CSimpleBrowser::GetListingJSON(SourceCallback callback)
{
    if (!m_webView) { callback(L""); return; }

    // NOT:
    // gaPageViewTrackingJson@data-json bazen COK BUYUK olabiliyor. WebView2 ExecuteScript sonucunda
    // buyuk string'ler bazi sistemlerde kesilebiliyor (truncated) ve bu da "customVars=1" gibi
    // semptomlara yol aciyor.
    // Cozum: Sayfa icinde JSON'u parse edip, ihtiyacimiz olan kismi (customVars + dmpData)
    // daha KUCUK bir payload olarak geri dondur.
    // Ayrica data-json bazen HTML entity'leri (&quot;) ile gelebiliyor. Onu da decode ediyoruz.

    LPCWSTR script =
        L"(function(){\n"
        L"  try {\n"
        L"    var elem = document.getElementById('gaPageViewTrackingJson');\n"
        L"    if(!elem) return null;\n"
        L"    var raw = elem.getAttribute('data-json');\n"
        L"    if(!raw) return null;\n"
        L"    var obj = null;\n"
        L"    try { obj = JSON.parse(raw); } catch(e) {\n"
        L"      // HTML entity decode (e.g. &quot;)\n"
        L"      var ta = document.createElement('textarea');\n"
        L"      ta.innerHTML = raw;\n"
        L"      obj = JSON.parse(ta.value);\n"
        L"    }\n"
        L"    var out = {\n"
        L"      customVars: (obj && obj.customVars) ? obj.customVars : [],\n"
        L"      dmpData:    (obj && obj.dmpData)    ? obj.dmpData    : [],\n"
        L"      trackPageview: (obj && obj.trackPageview) ? obj.trackPageview : [],\n"
        L"      viewName: (obj && obj.viewName) ? obj.viewName : null,\n"
        L"    };\n"
        L"    return JSON.stringify(out);\n"
        L"  } catch(e2) { return null; }\n"
        L"})()";

    m_webView->ExecuteScript(script,
        Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
            [callback](HRESULT errorCode, LPCWSTR resultObjectAsJson) -> HRESULT {
                callback(DecodeWebView2JsonString(resultObjectAsJson));
                return S_OK;
            }).Get());
}

void CSimpleBrowser::OnSize(int width, int height)
{
    if (m_controller) {
        RECT bounds = { 0, 0, width, height };
        m_controller->put_Bounds(bounds);
    }
}

void CSimpleBrowser::OnDestroy()
{
    if (m_controller) m_controller->Close();
    m_controller = nullptr;
    m_webView = nullptr;
}

LRESULT CSimpleBrowser::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_SIZE: OnSize(LOWORD(lParam), HIWORD(lParam)); break;
    case WM_DESTROY: OnDestroy(); break;
    }
    return CWnd::WndProc(uMsg, wParam, lParam);
}


// [EKSİK OLAN VE LİNKER HATASI VEREN KISIM]
void CSimpleBrowser::ExecuteScript(const std::wstring& script, std::function<void()> onComplete)
{
    if (!m_webView) {
        // Eğer browser hazır değilse callback'i hemen çağırmayalım veya hata verelim
        return;
    }

    m_webView->ExecuteScript(script.c_str(),
        Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
            [onComplete](HRESULT errorCode, LPCWSTR resultObjectAsJson) -> HRESULT {
                // Script çalışması bitti
                if (onComplete) onComplete();
                return S_OK;
            }).Get());
}

void CSimpleBrowser::ExecuteScriptGetResult(const std::wstring& script, SourceCallback callback)
{
    if (!m_webView) { callback(L""); return; }

    m_webView->ExecuteScript(script.c_str(),
        Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
            [callback](HRESULT /*errorCode*/, LPCWSTR resultObjectAsJson) -> HRESULT {
                callback(DecodeWebView2JsonString(resultObjectAsJson));
                return S_OK;
            }).Get());
}