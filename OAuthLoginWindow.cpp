#include "stdafx.h"
#include "OAuthLoginWindow.h"
#include <atlbase.h>
#include <atlconv.h>
#include <string>
#include <algorithm>

// Standart dışı olabilir diye manuel ekliyoruz
#ifndef DISPID_NAVIGATEERROR
#define DISPID_NAVIGATEERROR 271
#endif

OAuthLoginWindow::OAuthLoginWindow() : m_pConnPoint(nullptr), m_cookie(0), m_events(nullptr) {}

OAuthLoginWindow::~OAuthLoginWindow()
{
    if (m_pConnPoint) {
        if (m_cookie) m_pConnPoint->Unadvise(m_cookie);
        m_pConnPoint->Release();
    }
    if (m_events) m_events->Release();
}

// IUnknown & IDispatch
HRESULT OAuthLoginWindow::BrowserEvents::QueryInterface(REFIID riid, void** ppv)
{
    if (!ppv) return E_POINTER;
    if (riid == IID_IUnknown || riid == IID_IDispatch) {
        *ppv = static_cast<IDispatch*>(this); AddRef(); return S_OK;
    }
    return E_NOINTERFACE;
}

// ------------------- EVENT HANDLER (BURASI KRİTİK) -------------------
HRESULT OAuthLoginWindow::BrowserEvents::Invoke(
    DISPID dispidMember, REFIID, LCID, WORD, DISPPARAMS* pDispParams, VARIANT*, EXCEPINFO*, UINT*)
{
    // 1. Durum: Sayfa Yüklenmeden Önce (BeforeNavigate2)
    if (dispidMember == DISPID_BEFORENAVIGATE2 && pDispParams && pDispParams->cArgs >= 6)
    {
        VARIANT* vUrl = &pDispParams->rgvarg[5]; // URL
        VARIANT* vCancel = &pDispParams->rgvarg[0]; // Cancel

        if (vUrl->vt & VT_BYREF) vUrl = vUrl->pvarVal;
        if (vCancel->vt & VT_BYREF) vCancel = vCancel->pvarVal;

        if (vUrl->vt == VT_BSTR)
        {
            std::wstring url(vUrl->bstrVal);
            // URL bizim kodumuzu içeriyor mu?
            if (m_parent && m_parent->ExtractCodeFromUrl(url))
            {
                // Evet! Navigasyonu durdur ve başarılı sayfasını aç.
                if (vCancel->vt == VT_BOOL || vCancel->vt == (VT_BOOL | VT_BYREF))
                    *(vCancel->pboolVal) = VARIANT_TRUE;

                m_parent->ShowSuccessPage();
                return S_OK;
            }
        }
    }
    // 2. Durum: Bağlantı Hatası Olduğunda (NavigateError)
    // localhost'a bağlanamayınca burası tetiklenir.
    else if (dispidMember == DISPID_NAVIGATEERROR && pDispParams && pDispParams->cArgs >= 4)
    {
        VARIANT* vUrl = &pDispParams->rgvarg[3]; // Hata veren URL
        VARIANT* vCancel = &pDispParams->rgvarg[0]; // İptal

        if (vUrl->vt & VT_BYREF) vUrl = vUrl->pvarVal;
        if (vCancel->vt & VT_BYREF) vCancel = vCancel->pvarVal;

        if (vUrl->vt == VT_BSTR)
        {
            std::wstring url(vUrl->bstrVal);

            // Hata veren URL, bizim kodlu URL mi?
            if (m_parent && m_parent->ExtractCodeFromUrl(url))
            {
                // Evet! Bu bir hata değil, zaferdir.
                // Hata sayfasını engelle.
                if (vCancel->vt == VT_BOOL || vCancel->vt == (VT_BOOL | VT_BYREF))
                    *(vCancel->pboolVal) = VARIANT_TRUE;

                m_parent->ShowSuccessPage();
                return S_OK;
            }
        }
    }
    return S_OK;
}

// Pencere Oluşturma
int OAuthLoginWindow::OnCreate(CREATESTRUCT& cs)
{
    if (CWnd::OnCreate(cs) == -1) return -1;
    if (!m_browser.Create(*this)) return -1;

    // Pencere boyutuna yay
    CRect rc; rc=GetClientRect();
    m_browser.SetWindowPos(NULL, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW);

    IWebBrowser2* pWB = m_browser.GetIWebBrowser2();
    if (pWB)
    {
        // Event Sink Bağla
        m_events = new BrowserEvents(this);
        m_events->AddRef();
        CComPtr<IConnectionPointContainer> cpc;
        if (SUCCEEDED(pWB->QueryInterface(IID_IConnectionPointContainer, (void**)&cpc))) {
            if (SUCCEEDED(cpc->FindConnectionPoint(DIID_DWebBrowserEvents2, &m_pConnPoint))) {
                m_pConnPoint->Advise(static_cast<IDispatch*>(m_events), &m_cookie);
            }
        }
        // Sessiz Mod (Script hatalarını gösterme)
        pWB->put_Silent(VARIANT_TRUE);
    }
    return 0;
}

void OAuthLoginWindow::OnInitialUpdate()
{
    CWnd::OnInitialUpdate();
    if (!m_authUrl.empty()) {
        CString url(m_authUrl.c_str());
        m_browser.Navigate(url);
    }
}

void OAuthLoginWindow::OnDestroy()
{
    if (m_browser.IsWindow()) m_browser.Destroy();
    CWnd::OnDestroy();
}

// URL Analizi
bool OAuthLoginWindow::ExtractCodeFromUrl(const std::wstring& url)
{
    std::wstring urlLower = url;
    std::transform(urlLower.begin(), urlLower.end(), urlLower.begin(), ::towlower);

    // 1. localhost veya 127.0.0.1 mi?
    bool isLocal = (urlLower.find(L"localhost") != std::wstring::npos) ||
        (urlLower.find(L"127.0.0.1") != std::wstring::npos);

    // 2. code= parametresi var mı?
    bool hasCode = (urlLower.find(L"code=") != std::wstring::npos);

    if (isLocal && hasCode)
    {
        size_t pos = urlLower.find(L"code=");
        std::wstring code = url.substr(pos + 5);

        // Fazlalıkları kes (&scope=... vs)
        size_t endPos = code.find_first_of(L"&");
        if (endPos != std::wstring::npos) code = code.substr(0, endPos);

        // Kodu UTF-8'e çevir ve gönder
        std::string utf8Code;
        int len = WideCharToMultiByte(CP_UTF8, 0, code.c_str(), -1, NULL, 0, NULL, NULL);
        if (len > 0) {
            utf8Code.resize(len);
            WideCharToMultiByte(CP_UTF8, 0, code.c_str(), -1, &utf8Code[0], len, NULL, NULL);
            if (!utf8Code.empty() && utf8Code.back() == '\0') utf8Code.pop_back();
        }

        if (m_onCodeReceived) m_onCodeReceived(utf8Code);
        return true;
    }
    return false;
}

void OAuthLoginWindow::ShowSuccessPage()
{
    CString html = L"about:<!DOCTYPE html><html><body style='font-family:Segoe UI;text-align:center;padding-top:50px;'><h1 style='color:green;'>Giriş Başarılı!</h1><p>Lütfen bekleyiniz...</p></body></html>";
    m_browser.Navigate(html);
}