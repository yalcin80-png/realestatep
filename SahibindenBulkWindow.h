#pragma once

#include "wxx_wincore.h"
#include "wxx_controls.h"
#include "wxx_stdcontrols.h" // CButton, CEdit, CListBox için gerekli

#include <vector>
#include <string>
#include <atomic>

#include <cstdint>

// CSimpleBrowser'ın da Win32++ CWnd'den türetildiği varsayılmıştır.
#include "SimpleBrowser.h" 

// Global: veri değişti (bulk import vs.) -> ana view yenile
#ifndef UWM_DATA_CHANGED
  #define UWM_DATA_CHANGED        (WM_APP + 203)
#endif

class CSahibindenBulkWindow : public Win32xx::CWnd
{
public:
    CSahibindenBulkWindow();
    virtual ~CSahibindenBulkWindow() noexcept;

    // NOTE:
    // Win32++'ta CWnd::Create(...) sanal fonksiyonunun imzası bu sınıfta
    // ihtiyaç duyduğumuz "Create(HWND owner)" ile aynı değildir.
    // Bu yüzden burada Create adını kullanmak override çakışmasına yol açar.
    // Çözüm: farklı isim kullan.
    HWND CreateBulkWindow(HWND hOwner);

protected:
    // Win32++ Message Loop Override
    virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

    // Olay İşleyicileri
    virtual int OnCreate(CREATESTRUCT& cs) override;
    virtual void OnDestroy() override;
    //void OnSize(int width, int height);
    void OnCommand(UINT id);

private:
    enum class State {
        Idle,
        LoadList,
        WaitListReady,
        LoadAd,
        WaitAdReady,
        SaveAd,
        NextAd,
        NextListPage,
        PausedBot,
        PausedUser,
        Done
    };

    void CreateControls();
    void LayoutControls(int cx, int cy);

    // Checkpoint / Rapor
    void LoadCheckpoint();
    void SaveCheckpoint(bool force = false);
    void ClearCheckpoint();

    std::wstring GetCheckpointPath() const;
    std::wstring GetDailyReportPath() const;
    static std::wstring GetExeDir();

    void ApplySpeedPreset(int presetIndex);
    void UpdateTitleStats();

    void LogLine(const Win32xx::CString& s);
    void Start();
    void Stop();

    void Tick();
    void RequestListingLinks();
    void NavigateToAd();
    void RequestAdJson();
    void RequestAdHtmlAndSave(const std::wstring& jsonText);
    void RequestNextPage();

    static std::wstring Trim(const std::wstring& s);

private:
    // UI Kontrolleri (Win32++ Sınıfları)
    Win32xx::CEdit      m_editUrl;
    Win32xx::CComboBox  m_comboSpeed;
    Win32xx::CButton    m_btnStart;
    Win32xx::CButton    m_btnStop;
    Win32xx::CButton    m_btnPause;
    Win32xx::CListBox   m_listLog;
    CSimpleBrowser      m_browser;

    // Owner (genelde CMainFrame) - import sonrası UI refresh tetiklemek için
    HWND m_ownerHwnd = nullptr;

    // Durum Değişkenleri
    std::atomic<bool> m_running{ false };
    std::atomic<bool> m_userPaused{ false };
    State m_state = State::Idle;
    State m_userResumeState = State::Idle;
    UINT_PTR m_timer = 0;

    std::vector<std::wstring> m_adUrls;
    size_t m_adIndex = 0;

    std::wstring m_currentListUrl;
    std::wstring m_nextListUrl;
    int m_pageCount = 0;
    int m_maxPages = 9999;

    DWORD m_stateStartTick = 0;

    // Throttling/Backoff (ban riskini azaltmak icin)
    DWORD m_nextActionTick = 0;
    int   m_adsSinceLongPause = 0;
    int   m_blockCount = 0;
    State m_resumeState = State::Idle;

    // Ayarlar (insan gibi gezinme)
    int   m_longPauseEveryAds = 10;
    DWORD m_longPauseMinMs = 30000;
    DWORD m_longPauseMaxMs = 90000;

    DWORD m_listReadyMinMs = 2000;
    DWORD m_listReadyMaxMs = 4500;
    DWORD m_adReadyMinMs   = 3000;
    DWORD m_adReadyMaxMs   = 6500;
    DWORD m_betweenAdsMinMs = 4500;
    DWORD m_betweenAdsMaxMs = 12000;

    DWORD m_backoffBaseMs = 60000; // 60s (engel yakalaninca)
    DWORD m_backoffMaxMs  = 1800000; // 30dk cap

    // Profil (UI)
    int   m_speedPreset = 1; // 0=Yavas, 1=Normal, 2=CokYavas

    // Gunluk kota
    int   m_dailyQuotaMax = 150;
    int   m_dailyCount = 0;
    std::wstring m_dailyDate; // YYYY-MM-DD

    // Istatistik
    int   m_statFoundLinks = 0;
    int   m_statSavedOk = 0;
    int   m_statSavedFail = 0;
    int   m_statBackoffCount = 0;
    int   m_statLongPauseCount = 0;

    // Checkpoint throttling
    DWORD m_lastCheckpointTick = 0;
    DWORD m_checkpointEveryMs = 5000; // 5 sn

    bool  m_checkpointLoaded = false;
    bool  m_checkpointWasRunning = false;

    void ScheduleNextAction(DWORD delayMs);
    DWORD RandRange(DWORD minMs, DWORD maxMs);
    void PauseWithBackoff(const Win32xx::CString& reason, State resumeState);

};