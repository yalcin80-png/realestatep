#include "stdafx.h"
#include "SyncProgressDlg.h"

SyncProgressDlg::SyncProgressDlg(SyncManager* pSyncMgr)
    : CDialog(IDD_SYNC_PROGRESS), m_pSyncMgr(pSyncMgr)
{
}

SyncProgressDlg::~SyncProgressDlg()
{
    if (m_workerThread.joinable())
        m_workerThread.join();
}

BOOL SyncProgressDlg::OnInitDialog()
{
    // 1. Kontrolleri Bağla
    // Eğer resource ID'ler yanlışsa burada hata verir. Resource.h ile Resource.rc uyumlu mu?
    if (!m_progBar.AttachDlgItem(IDC_SYNC_PROGRESS_BAR, *this))
    {
        MessageBox(L"Progress Bar bulunamadı! Resource ID kontrol edin.", L"Hata", MB_ICONERROR);
        EndDialog(IDCANCEL);
        return FALSE;
    }

    m_lblStatus.AttachDlgItem(IDC_SYNC_STATUS_TEXT, *this);

    m_progBar.SetRange(0, 100);
    m_progBar.SetPos(0);
    m_lblStatus.SetWindowText(L"Bağlantı kontrol ediliyor...");

    // 2. Thread Başlat
    m_workerThread = std::thread(&SyncProgressDlg::ThreadWorker, this);

    return TRUE;
}

void SyncProgressDlg::ThreadWorker()
{
    // Güvenlik kontrolü
    if (!m_pSyncMgr) {
        this->PostMessage(WM_SYNC_FINISH, 0, 0);
        return;
    }

    // Callback fonksiyonu (UI Güncelleme)
    auto callback = [this](const std::string& msg, int percent) {
        // Log çıktısı verelim (DebugView ile izleyebilirsiniz)
        std::string debugMsg = "[Sync] " + msg + "\n";
        OutputDebugStringA(debugMsg.c_str());

        // UI'ya mesaj gönder
        std::wstring wMsg(msg.begin(), msg.end());
        CString* pStr = new CString(wMsg.c_str());
        this->PostMessage(WM_SYNC_UPDATE, (WPARAM)percent, (LPARAM)pStr);

        // Çok hızlı geçmemesi için suni gecikme
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        };

    // --- SENKRONİZASYON BAŞLIYOR ---

    // 1. Tablo Listesi (Veritabanınızdaki gerçek tablo adları olmalı)
    std::vector<std::pair<std::string, std::string>> tables = {
            {"Customer",     "Customer"},
            {"HomeTbl",      "HomeTbl"},
            {"Land",         "Land"},
            {"Field",        "Field"},
            {"Vineyard",     "Vineyard"},
            {"Villa",        "Villa"},
            {"Commercial",   "Commercial"},
            {"Offers",       "Offers"},

            {"Appointments", "Appointments"} // <--- YENİ: Bunu ekleyin
    };

    try {
        // SyncManager'ı çalıştır
        bool success = m_pSyncMgr->SyncMany(tables, callback);

        if (!success) {
            OutputDebugString(L"[Sync] SyncMany başarısız döndü!\n");
        }
    }
    catch (const std::exception& e) {
        std::string err = "[Sync] HATA: ";
        err += e.what();
        OutputDebugStringA(err.c_str());
    }

    // İşlem bitti, pencereyi kapat
    this->PostMessage(WM_SYNC_FINISH, 0, 0);
}

INT_PTR SyncProgressDlg::DialogProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
    case WM_SYNC_UPDATE:
    {
        int percent = (int)wparam;
        CString* pMsg = (CString*)lparam;
        if (m_progBar.IsWindow()) m_progBar.SetPos(percent);
        if (pMsg && m_lblStatus.IsWindow()) {
            m_lblStatus.SetWindowText(*pMsg);
            delete pMsg;
        }
        return TRUE;
    }
    case WM_SYNC_FINISH:
        EndDialog(IDOK);
        return TRUE;
    }
    return DialogProcDefault(msg, wparam, lparam);
}