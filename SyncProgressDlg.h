#pragma once

#include "SyncManager.h"
#include "resource.h"           // Resource ID'leri için
#include <thread>

// Custom Mesajlar
#define WM_SYNC_UPDATE  (WM_USER + 100)
#define WM_SYNC_FINISH  (WM_USER + 101)

class SyncProgressDlg : public CDialog
{
public:
    // Constructor'da resource ID'yi alıyoruz
    SyncProgressDlg(SyncManager* pSyncMgr);
    virtual ~SyncProgressDlg();

protected:
    // Win32++ Override'ları
    virtual BOOL OnInitDialog() override;
    virtual INT_PTR DialogProc(UINT msg, WPARAM wparam, LPARAM lparam) override; // Mesajları burada yakalayacağız

private:
    SyncManager* m_pSyncMgr;
    CProgressBar m_progBar;
    CStatic      m_lblStatus;
    std::thread  m_workerThread;

    void ThreadWorker();
};