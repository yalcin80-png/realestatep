// SyncBridge.cpp  (FIXED & HARDENED)

#include "stdafx.h"
#include "resource.h"
#include "SyncBridge.h"

#include <chrono>
#include <thread>
#include <fstream>
#include <sstream>
#include <mutex>
#include <filesystem>

#include <WinInet.h>
#pragma comment(lib, "Wininet.lib")

using json = nlohmann::json;

static const wchar_t* OFFLINE_LOG_PATH = L"C:\\EmlakCRM\\offline_sync.log";

// ---------------------------------------------------------
void SyncBridge::Initialize(const std::string& projectId, LoginManager* auth)
{
    m_projectId = projectId;
    m_auth = auth;
}

// ---------------------------------------------------------
void SyncBridge::NotifyInsert(const std::string& table, const CString& syncId)
{
    HandleChange(table, syncId, "insert");
}

void SyncBridge::NotifyUpdate(const std::string& table, const CString& syncId)
{
    HandleChange(table, syncId, "update");
}

void SyncBridge::NotifyDelete(const std::string& table, const CString& syncId)
{
    HandleChange(table, syncId, "delete");
}

// ---------------------------------------------------------
void SyncBridge::HandleChange(const std::string& table,
    const CString& syncId,
    const std::string& action)
{
    if (!m_auth) return;

    if (!IsOnline() || !SendFirestoreRequest(table, syncId, action))
        QueueLocalChange(table, syncId, action);
}

// ---------------------------------------------------------
void SyncBridge::QueueLocalChange(const std::string& table,
    const CString& syncId,
    const std::string& action)
{
    std::lock_guard<std::mutex> lock(m_fileMutex);

    std::filesystem::create_directories("C:\\EmlakCRM");

    std::ofstream out("C:\\EmlakCRM\\offline_sync.log", std::ios::app | std::ios::binary);
    if (!out.is_open()) return;

    out << table << "|"
        << CStringToUtf8(syncId) << "|"
        << action << "\n";
}

// ---------------------------------------------------------
std::string SyncBridge::CStringToUtf8(const CString& str)
{
    if (str.IsEmpty()) return {};

    int len = WideCharToMultiByte(CP_UTF8, 0, str, -1, nullptr, 0, nullptr, nullptr);
    std::string out(len - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, str, -1, out.data(), len - 1, nullptr, nullptr);
    return out;
}

// ---------------------------------------------------------
bool SyncBridge::SendFirestoreRequest(const std::string& table,
    const CString& syncId,
    const std::string& action)
{
    std::lock_guard<std::mutex> netLock(m_netMutex);

    std::string token = m_auth->GetAccessToken();
    if (token.empty()) return false;

    std::string syncIdUtf8 = CStringToUtf8(syncId);

    std::string baseUrl =
        "https://firestore.googleapis.com/v1/projects/" + m_projectId +
        "/databases/(default)/documents/" + table + "/" + syncIdUtf8;

    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm tm{}; gmtime_s(&tm, &now);

    char ts[32]{};
    strftime(ts, sizeof(ts), "%Y-%m-%dT%H:%M:%SZ", &tm);

    json body = {
        { "fields", {
            { "sync_id",    { { "stringValue", syncIdUtf8 } } },
            { "action",     { { "stringValue", action } } },
            { "Updated_At", { { "timestampValue", ts } } }
        }}
    };

    cpr::Response r = cpr::Patch(
        cpr::Url{ baseUrl },
        cpr::Bearer{ token },
        cpr::Header{ {"Content-Type","application/json"} },
        cpr::Body{ body.dump() }
    );

    if (r.status_code == 404)
    {
        r = cpr::Post(
            cpr::Url{ "https://firestore.googleapis.com/v1/projects/" + m_projectId +
                      "/databases/(default)/documents/" + table +
                      "?documentId=" + syncIdUtf8 },
            cpr::Bearer{ token },
            cpr::Header{ {"Content-Type","application/json"} },
            cpr::Body{ body.dump() }
        );
    }

    return (r.status_code >= 200 && r.status_code < 300);
}

// ---------------------------------------------------------
void SyncBridge::ProcessOfflineQueue()
{
    if (!IsOnline()) return;

    std::lock_guard<std::mutex> fileLock(m_fileMutex);

    std::ifstream in("C:\\EmlakCRM\\offline_sync.log", std::ios::binary);
    if (!in.is_open()) return;

    std::vector<std::string> failed;
    std::string line;

    while (std::getline(in, line))
    {
        std::stringstream ss(line);
        std::string table, syncId, action;

        std::getline(ss, table, '|');
        std::getline(ss, syncId, '|');
        std::getline(ss, action);

        if (!SendFirestoreRequest(table, CString(syncId.c_str()), action))
            failed.push_back(line);
    }
    in.close();

    std::ofstream out("C:\\EmlakCRM\\offline_sync.log", std::ios::trunc | std::ios::binary);
    for (auto& l : failed)
        out << l << "\n";
}

// ---------------------------------------------------------
void SyncBridge::StartNetworkMonitor(int intervalMs)
{
    if (m_monitorRunning.load()) return;

    m_monitorRunning.store(true);
    m_monitorIntervalMs = intervalMs;

    m_monitorThread = std::thread([this]()
        {
            while (m_monitorRunning.load())
            {
                DWORD flags = 0;
                bool online = InternetGetConnectedState(&flags, 0);

                if (online != IsOnline())
                {
                    SetOnline(online);
                    if (online) ProcessOfflineQueue();
                }

                std::this_thread::sleep_for(
                    std::chrono::milliseconds(m_monitorIntervalMs));
            }
        });
}

// ---------------------------------------------------------
void SyncBridge::StopNetworkMonitor()
{
    m_monitorRunning.store(false);
    if (m_monitorThread.joinable())
        m_monitorThread.join();
}

// ---------------------------------------------------------
SyncBridge::~SyncBridge()
{
    StopNetworkMonitor();
}
