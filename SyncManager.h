// SyncManager.h
#pragma once

#include <string>
#include <vector>
#include <map>
#include "ISyncLocalAdapter.h"
#include "ICloudProvider.h"
#include <nlohmann/json.hpp>











class SyncManager
{
public:
    using json = nlohmann::json;
    using ProgressCallback = std::function<void(const std::string&, int)>;
    
    SyncManager(ISyncLocalAdapter& local, ICloudProvider& cloud);

  // ✅ PULL: Cloud'dan veri indir (Cloud → App)
    bool PullFromCloud(const std::string& table, const std::string& collection);

    // ✅ PUSH: App'ten veriyi cloud'a gönder (App → Cloud)
    bool PushToCloud(const std::string& table, const std::string& collection);
    void DownloadEverything();

    // Çift yönlü senkronizasyon (Push + Pull)
    bool Sync(const std::string& table, const std::string& collection);

    // Birden fazla tablo / koleksiyonu ardışık senkronize eder.
    bool SyncMany(const std::vector<std::pair<std::string, std::string>>& pairs);
    
    // Çoklu tablo senkronizasyonu (İsteğe bağlı progress callback eklendi)
    bool SyncMany(const std::vector<std::pair<std::string, std::string>>& pairs, 
                  ProgressCallback onProgress = nullptr);
    
    // Anlık insert/update bildirimi (tek satır)
    void NotifyUpsert(const std::string& collection, const json& row);

    // Anlık delete bildirimi (tek sync_id)
    void NotifyDelete(const std::string& collection, const std::string& sync_id);
    json RecordToJson(const CString& tableName, void* recordData);

private:
    ISyncLocalAdapter& m_local;
    ICloudProvider& m_cloud;

    // Aynı sync_id'ye sahip yerel ve bulut kayıtları arasında
    // "Updated_At" alanına göre en yeni olanı seçer.
    json ResolveConflicts(const json& localRows, const json& cloudRows);

    // ISO 8601 string'ler arasında max (lexicographic)
    static std::string MaxIso(const std::string& a, const std::string& b);
};
