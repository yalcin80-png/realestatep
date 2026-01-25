#pragma once
#include "ISyncLocalAdapter.h"
#include <string>
#include <vector>
#include <comdef.h>
#include "AllGlobalHeader.h"

// ADO nesneleri için import (Eğer projenizde stdafx.h içinde varsa burayı yorum satırı yapabilirsiniz)
// Genelde bu satır AdoLocalAdapter.cpp veya stdafx.h içinde olur. 
// Derleme hatası alırsanız burayı açın:
// #import "C:\Program Files\Common Files\System\ADO\msado15.dll" no_namespace rename("EOF", "EndOfFile")

class AdoLocalAdapter : public ISyncLocalAdapter
{
public:
    // TEK VE BİRLEŞİK CONSTRUCTOR (Varsayılan parametreli)
    // Bu satır sayesinde hem boş hem de dolu çağırabilirsiniz.
    AdoLocalAdapter(const std::string& connStr = "");

    ~AdoLocalAdapter();

    // Temel Fonksiyonlar
    bool Exec(const std::string& sql);
    nlohmann::json QueryJson(const std::string& sql);

    // Interface (ISyncLocalAdapter) Fonksiyonları
    bool EnsureSyncColumns(const std::string& table) override;
    nlohmann::json GetChangedRows(const std::string& table, const std::string& sinceIsoUtc) override;
    bool UpsertRows(const std::string& table, const nlohmann::json& rows) override;
    bool MarkDeleted(const std::string& table, const std::vector<std::string>& sync_ids) override;

    std::string GetLastSyncStampIsoUtc() override;
    void SetLastSyncStampIsoUtc(const std::string& isoUtc) override;

    // Ekstra Overloadlar
    std::string GetLastSyncStampIsoUtc(const std::string& table);
    void SetLastSyncStampIsoUtc(const std::string& table, const std::string& stamp);

private:
    _ConnectionPtr m_conn;
    std::string m_connStr;

    // Varsayılan bağlantı metni
    std::string GetDefaultConnStr();
};