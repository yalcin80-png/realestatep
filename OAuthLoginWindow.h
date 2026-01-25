

#ifndef OATHLOGINMNGR
#define OATHLOGINMNGR

#pragma once
#include "stdafx.h"
#include <exdisp.h>          // IWebBrowser2, DIID_DWebBrowserEvents2
#include <ocidl.h>           // IConnectionPointContainer, IConnectionPoint
#include <functional>
#include <string>
#include <exdispid.h>  // DISPID_BEFORENAVIGATE2  👈 BUNU EKLE
class OAuthLoginWindow : public CWnd
{
public:
    OAuthLoginWindow();
    virtual ~OAuthLoginWindow();

    void SetAuthUrl(const std::string& url) { m_authUrl = url; }

    void SetOnCodeReceivedCallback(const std::function<void(const std::string&)>& cb)
    {
        m_onCodeReceived = cb;
    }
    void ShowSuccessPage();
protected:
    virtual int  OnCreate(CREATESTRUCT& cs) override;
    virtual void OnInitialUpdate() override;
    virtual void OnDestroy() override;

private:
    // ---- WebBrowser + Event Sink ----
    CWebBrowser        m_browser;        // Win32++ WebBrowser kontrolü
    IConnectionPoint* m_pConnPoint = nullptr;
    DWORD              m_cookie = 0;

    class BrowserEvents : public IDispatch
    {
    public:
        explicit BrowserEvents(OAuthLoginWindow* parent) : m_parent(parent) {}

        // IUnknown
        ULONG STDMETHODCALLTYPE AddRef() override { return 1; }
        ULONG STDMETHODCALLTYPE Release() override { return 1; }
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override;

        // IDispatch
        HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT*) override { return E_NOTIMPL; }
        HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT, LCID, ITypeInfo**) override { return E_NOTIMPL; }
        HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID, LPOLESTR*, UINT, LCID, DISPID*) override { return E_NOTIMPL; }

        HRESULT STDMETHODCALLTYPE Invoke(
            DISPID dispidMember,
            REFIID riid,
            LCID lcid,
            WORD wFlags,
            DISPPARAMS* pDispParams,
            VARIANT* pVarResult,
            EXCEPINFO* pExcepInfo,
            UINT* puArgErr) override;

    private:
        OAuthLoginWindow* m_parent;
    };

    BrowserEvents* m_events = nullptr;

    // ---- OAuth bilgileri / callback ----
    std::string                               m_authUrl;
    std::function<void(const std::string&)>   m_onCodeReceived;

    bool ExtractCodeFromUrl(const std::wstring& url);
};



#endif  // OATHLOGINMNGR