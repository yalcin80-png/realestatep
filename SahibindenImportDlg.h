#pragma once
#include "wxx_wincore.h"
#include "resource.h"
#include "SimpleBrowser.h" // Modüler Browser Sınıfımız

class CSahibindenImportDlg : public CDialog
{
public:
    CSahibindenImportDlg(HWND hParent = NULL);
    virtual ~CSahibindenImportDlg() {}
    void ApplyClipStyles();

protected:
    // Dialog Başlatıldığında
    virtual BOOL OnInitDialog();
    void ProcessJsonContent(const std::wstring& jsonText);
    // YENİ: Hem JSON hem HTML verisini alıp birleştirir.
    // Buton Tıklamaları
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

    // Pencere Kapatma
    virtual void OnCancel();

    // Pencere Boyutlandırma ve Timer mesajları için
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
    enum : UINT {
        // WebView2 init'i dialog acilisindan sonra (message loop baslayinca) yapmak icin.
        WM_APP_INIT_BROWSER = (WM_APP + 0x0210),
        WM_APP_FORCE_ACTIVATE = (WM_APP + 0x0211),
        TIMER_IMPORT_POLL = 1,
    };

    // --- Aksiyonlar ---
    void OnButtonNavigate(); // Linke Git
    void OnButtonImport();   // Veriyi Çek ve Kaydet

    // --- Yardımcılar ---
    void ProcessHtmlContent(const std::wstring& html);
    void LogLine(const CString& text);
    void ResizeBrowser(); // Browser'ı pencere boyutuna uydur

    // Import polling (sayfa tam yuklenmeden JSON bazen bos gelir)
    void BeginImportPoll();
    void PollImportOnce();
    void EndImportPoll();
    std::wstring GetLogFilePath() const;

    // --- Kontroller ---
    CEdit           m_editUrl;      // URL Kutusu
    CEdit           m_editContactName;
    CEdit           m_editContactPhone;
    CButton         m_btnGo;        // Git Butonu
    CButton         m_btnImport;    // İçe Aktar Butonu
    CButton         m_btnPasteClip;
    CListBox        m_listLog;      // Log Ekranı

    // --- Modüler Browser Bileşeni ---
    CSimpleBrowser  m_browser;

    // --- Import state ---
    bool  m_importInProgress{ false };
    int   m_importTry{ 0 };
    DWORD m_importStartTick{ 0 };
    CString m_importUrl;
};