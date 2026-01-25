/////////////////////////////
// App.h
//
// NOTE: Bu proje artık Ribbon kullanmıyor.

#ifndef APP_H
#define APP_H

#include "Mainfrm.h"

/////////////////////////////////////////////////////////////////
// CRealtyApp manages the application. It initializes the
// Win32++ framework when it is constructed, and creates the main
// frame window when it runs.
class CRealtyApp : public CWinApp
{
public:
    CRealtyApp() = default;
    virtual ~CRealtyApp() override = default;

    CMainFrame& GetMainFrame() { return m_frame; }

protected:
    virtual BOOL InitInstance() override;

private:
    CRealtyApp(const CRealtyApp&) = delete;
    CRealtyApp& operator=(const CRealtyApp&) = delete;

    CMainFrame m_frame;
};

// ---------------------------------------------------------------------------
//  InitInstance implementation
//
//  Not: Bazı projelerde App.cpp dosyası solution'a ekli olmayabiliyor.
//  Bu durumda InitInstance link aşamasında "unresolved external" hatası verir.
//  Bu hatayı kalıcı olarak engellemek için InitInstance'ı header içinde inline
//  olarak da sağlıyoruz.
//
//  Eğer App.cpp derleniyorsa, orada REALTYAPP_NO_INLINE_INITINSTANCE tanımlanır
//  ve bu inline implementasyon devre dışı kalır.
// ---------------------------------------------------------------------------
#ifndef REALTYAPP_NO_INLINE_INITINSTANCE
inline BOOL CRealtyApp::InitInstance()
{
    // Uygulama başlangıcı
    m_frame.Create();   // throws a CWinException on failure
    return TRUE;
}
#endif

inline CRealtyApp* GetRealtyApp() { return static_cast<CRealtyApp*>(GetApp()); }

#endif
