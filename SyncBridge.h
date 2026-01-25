// SyncBridge.h
#pragma once

#include "ISyncInterface.h"
#include "SyncManager.h"
#include <mutex>
#include <string>
#include <nlohmann/json.hpp>
#include <cpr/cpr.h>
#include "LoginManager.h"
#include <thread>
#include <atomic>

class SyncBridge
{
public:
    static SyncBridge& GetInstance()
    {
        static SyncBridge instance;
        return instance;
    }

    // Bağlantı durumu
    static bool IsOnline() { return m_online; }
    static void SetOnline(bool state) { m_online = state; }

    // Ekleme / Güncelleme bildirimleri
    void NotifyInsert(const std::string& table, const CString& syncId);
    void NotifyUpdate(const std::string& table, const CString& syncId);

    // İsteğe bağlı: Silme bildirimi (kullanmak istersen)
    void NotifyDelete(const std::string& table, const CString& syncId);

    // Çevrimdışı durumda yapılacak işlemleri sıraya alır.
    void QueueLocalChange(const std::string& table,
        const CString& syncId,
        const std::string& action);
    void HandleChange(const std::string& table,
        const CString& syncId,
        const std::string& action);
    std::mutex m_fileMutex;   // offline log için
    std::mutex m_netMutex;    // clou
    // Firestore yapılandırması
    void Initialize(const std::string& projectId, LoginManager* auth);
    void ProcessOfflineQueue();

    // Ağ durumunu izlemeyi başlat / durdur
    void StartNetworkMonitor(int intervalMs = 5000);
    void StopNetworkMonitor();

private:
    SyncBridge() = default;
    ~SyncBridge();

    SyncBridge(const SyncBridge&) = delete;
    SyncBridge& operator=(const SyncBridge&) = delete;

    static std::string CStringToUtf8(const CString& str);
    bool SendFirestoreRequest(const std::string& table,
        const CString& syncId,
        const std::string& action);

private:
    inline static bool m_online = true;

    std::string m_projectId;
    LoginManager* m_auth = nullptr;
    std::mutex m_mutex;

    // Network monitor
    std::thread m_monitorThread;
    std::atomic<bool> m_monitorRunning{ false };
    int m_monitorIntervalMs{ 5000 };
};
