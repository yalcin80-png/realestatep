#include "stdafx.h"
#include "FirestoreProvider.h"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <mutex>
#include <iostream>

// Windows.h genelde stdafx.h içinde gelir ama
// OutputDebugStringA için emin olmak adına:
#ifndef _WINDOWS_
#include <windows.h>
#endif

using json = nlohmann::json;

static std::mutex g_firestoreNetMutex;

namespace
{
    // Firestore için RFC 3339 formatı (Zaman dilimi UTC 'Z' ile bitmeli)
    std::string IsoNowUtcHelper()
    {
        using namespace std::chrono;
        auto now = system_clock::now();
        auto t = system_clock::to_time_t(now);
        std::tm tm{};

        // Windows için güvenli gmtime
        gmtime_s(&tm, &t);

        char buf[32]{};
        // Format: YYYY-MM-DDTHH:MM:SSZ
        strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tm);
        return std::string(buf);
    }
}

// ============================================================================
//  CTOR
// ============================================================================
FirestoreProvider::FirestoreProvider(const std::string& accessToken,
    const std::string& projectId,
    const std::string& database)
    : m_token(accessToken)
    , m_project(projectId)
    , m_database(database)
{
}
// SyncManager.cpp - Yardımcı Fonksiyon (ToUtf8 altına ekleyin)

//static std::string FormatToFirestoreDate(const CString& rawDate)
//{
//    std::string s = ToUtf8(rawDate);
//    if (s.empty()) return "1970-01-01T00:00:00Z"; // Boşsa varsayılan tarih
//
//    // Zaten uygun formatta mı? (2024-05-20T14:30:00Z gibi)
//    if (s.find('T') != std::string::npos && s.back() == 'Z') return s;
//
//    // Basit düzeltme: "2024-01-01 12:00:00" -> "2024-01-01T12:00:00Z"
//    for (auto& c : s) {
//        if (c == ' ') c = 'T';
//    }
//    // Sona 'Z' (UTC) ekle
//    if (s.back() != 'Z') s += 'Z';
//
//    return s;
//}
// ============================================================================
//  Helper: Log Error
// ============================================================================
void FirestoreProvider::LogError(const std::string& prefix, const cpr::Response& r)
{
    std::stringstream ss;
    ss << "[FirestoreProvider] " << prefix << " HTTP Error: " << r.status_code << "\n";
    ss << "Body: " << r.text << "\n"; // Hata detayını (Missing Index vs.) görmek için kritik
    OutputDebugStringA(ss.str().c_str());
}

void FirestoreProvider::LogDebug(const std::string& msg)
{
    std::string s = "[FirestoreProvider] " + msg + "\n";
    OutputDebugStringA(s.c_str());
}

// ============================================================================
//  Time Helper
// ============================================================================
std::string FirestoreProvider::NowIsoUtc()
{
    return IsoNowUtcHelper();
}

// ============================================================================
//  Local Row -> Firestore Document
// ============================================================================
FirestoreProvider::json FirestoreProvider::RowToFirestore(const json& row)
{
    json fields = json::object();

    for (auto it = row.begin(); it != row.end(); ++it)
    {
        const auto& key = it.key();
        const auto& val = it.value();

        // Firestore tiplerine çevrim
        if (val.is_boolean())
            fields[key] = { {"booleanValue", val.get<bool>()} };

        else if (val.is_number_integer())
            fields[key] = { {"integerValue", std::to_string(val.get<long long>())} }; // Firestore int64 ister, string olarak gider

        else if (val.is_number_float())
            fields[key] = { {"doubleValue", val.get<double>()} };

        else if (val.is_string())
        {
            // Özel alan kontrolü: Updated_At timestampValue olmalı
            if (key == "Updated_At")
                fields[key] = { {"timestampValue", val.get<std::string>()} };
            else
                fields[key] = { {"stringValue", val.get<std::string>()} };
        }
        else if (val.is_null())
            fields[key] = { {"nullValue", nullptr} };

        else
            // Karmaşık tipler veya array'ler string'e dökülür
            fields[key] = { {"stringValue", val.dump()} };
    }

    return json{ {"fields", fields} };
}

// ============================================================================
//  Firestore Document -> Local Row
// ============================================================================
FirestoreProvider::json FirestoreProvider::FirestoreToRow(const json& doc)
{
    json row = json::object();

    if (!doc.contains("fields")) return row;

    for (auto it = doc["fields"].begin(); it != doc["fields"].end(); ++it)
    {
        const auto& v = it.value();
        std::string k = it.key();

        if (v.contains("stringValue")) row[k] = v["stringValue"];
        else if (v.contains("booleanValue")) row[k] = v["booleanValue"];
        else if (v.contains("integerValue"))
        {
            // String olarak gelen sayıyı parse et
            try { row[k] = std::stoll(v["integerValue"].get<std::string>()); }
            catch (...) { row[k] = 0; }
        }
        else if (v.contains("doubleValue")) row[k] = v["doubleValue"];
        else if (v.contains("timestampValue")) row[k] = v["timestampValue"];
        else if (v.contains("nullValue")) row[k] = nullptr;
    }

    // sync_id'yi doküman isminden (name) çıkar
    if (doc.contains("name"))
    {
        auto& full = doc["name"].get_ref<const std::string&>();
        auto pos = full.find_last_of('/');
        if (pos != std::string::npos)
            row["sync_id"] = full.substr(pos + 1);
    }

    return row;
}









 //============================================================================
 // Pull (Cloud -> Local) SERVER SIDE FILTER
 //============================================================================
FirestoreProvider::json FirestoreProvider::PullChanges(
    const std::string& collection,
    const std::string& sinceIsoUtc)
{
    std::lock_guard<std::mutex> lock(g_firestoreNetMutex);

    // TRACE ile Output penceresinden takip edebilirsiniz
    std::string debugMsg = "[Firestore] PULL Istegi: " + collection + " Zaman: " + sinceIsoUtc + "\n";
    OutputDebugStringA(debugMsg.c_str());

    json body =
    {
        {"structuredQuery", {
            {"from", {{{"collectionId", collection}}}},
            {"where", {
                {"fieldFilter", {
                    {"field", {{"fieldPath","Updated_At"}}},
                    {"op","GREATER_THAN"},
                    {"value", {{"timestampValue", sinceIsoUtc}}}
                }}
            }}
        }}
    };

    auto resp = cpr::Post(
        cpr::Url{ "https://firestore.googleapis.com/v1/projects/" + m_project +
                  "/databases/" + m_database + "/documents:runQuery" },
        cpr::Header{
            {"Authorization","Bearer " + m_token},
            {"Content-Type","application/json"}
        },
        cpr::Body{ body.dump() },
        cpr::VerifySsl{ false }
    );

    json out = json::array();

    // 1. HATA KONTROLÜ
    if (resp.status_code != 200)
    {
        std::string err = "HTTP HATA: " + std::to_string(resp.status_code) + "\nCevap: " + resp.text;
        ::MessageBoxA(nullptr, err.c_str(), "Firestore Hatasi", MB_ICONERROR);
        return out;
    }

    try
    {
        auto arr = json::parse(resp.text);

        // 2. VERİ KONTROLÜ
        if (arr.is_array())
        {
            // Eğer dizi boşsa veya sadece readTime varsa veri yok demektir.
            // Amaç: Hiç 'document' içeren eleman var mı?
            bool veriBulundu = false;

            for (auto& it : arr)
            {
                if (it.contains("document")) {
                    out.push_back(FirestoreToRow(it["document"]));
                    veriBulundu = true;
                }
            }

            if (!veriBulundu) {
                OutputDebugStringA(("[Firestore] " + collection + " icin sunucudan 0 kayit dondu (Guncel veri yok).\n").c_str());
            }
            else {
                OutputDebugStringA(("[Firestore] " + collection + " icin " + std::to_string(out.size()) + " kayit indirildi.\n").c_str());
            }
        }
    }
    catch (const std::exception& e)
    {
        ::MessageBoxA(nullptr, e.what(), "JSON Parse Hatasi", MB_ICONERROR);
    }

    return out;
}
// ============================================================================
//  Push Upserts (AUTO CREATE + UPDATE)
// ============================================================================
bool FirestoreProvider::PushUpserts(const std::string& collection, const json& rows)
{
    std::lock_guard<std::mutex> lock(g_firestoreNetMutex);

    if (!rows.is_array() || rows.empty()) return true;

    // Firestore batch limitleri vardır (maks 500 işlem). Güvenlik için 400.
    const size_t batchSize = 400;

    for (size_t i = 0; i < rows.size(); i += batchSize)
    {
        json writes = json::array();

        for (size_t j = i; j < rows.size() && j < i + batchSize; ++j)
        {
            const auto& row = rows[j];
            if (!row.contains("sync_id")) continue;

            std::string sid = row["sync_id"];

            // Satırı Firestore formatına çevir
            json doc = RowToFirestore(row);

            // Dokümanın tam yolu: projects/{id}/databases/{db}/documents/{coll}/{docId}
            std::string name =
                "projects/" + m_project +
                "/databases/" + m_database +
                "/documents/" + collection + "/" + sid;

            // Upsert (Update varsa güncelle, yoksa oluştur) mantığı
            writes.push_back({
                {"update", {
                    {"name", name},
                    {"fields", doc["fields"]}
                }}
                });
        }

        if (writes.empty()) continue;

        json body = { {"writes", writes} };

        auto resp = cpr::Post(
            cpr::Url{ "https://firestore.googleapis.com/v1/projects/" + m_project +
                      "/databases/" + m_database + "/documents:commit" },
            cpr::Header{
                {"Authorization","Bearer " + m_token},
                {"Content-Type","application/json"}
            },
            cpr::Body{ body.dump() },
            cpr::VerifySsl{ false } // SSL Hatasını önlemek için
        );

        // BURAYI EKLEYİN:
        if (resp.status_code != 200) {
            std::string err = "[Firestore Error] Code: " + std::to_string(resp.status_code) + "\n";
            err += "Response Body: " + resp.text + "\n"; // Google'ın cevabı burada yazar
            OutputDebugStringA(err.c_str());
        }

        if (resp.status_code != 200)
        {
            LogError("PushUpserts", resp);
            return false;
        }
    }

    return true;
}

// ============================================================================
//  Push Deletes (SOFT DELETE)
// ============================================================================
bool FirestoreProvider::PushDeletes(const std::string& collection, const json& sync_ids)
{
    // Soft Delete: Kaydı silmek yerine "deleted = true" olarak güncelliyoruz.
    // Bu sayede diğer cihazlar da silindiğini anlayıp kendi verilerini güncelleyebilir.
    json rows = json::array();

    for (auto& sid : sync_ids)
    {
        rows.push_back({
            {"sync_id", sid},
            {"deleted", true},
            {"Updated_At", NowIsoUtc()}
            });
    }

    // Upsert mantığı aynı olduğu için PushUpserts fonksiyonunu yeniden kullanıyoruz.
    return PushUpserts(collection, rows);
}