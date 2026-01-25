// ExampleLocalAdapter.h
#pragma once
#include "ISyncLocalAdapter.h"

class ExampleLocalAdapter : public ISyncLocalAdapter {
public:
    explicit ExampleLocalAdapter(/* DatabaseManager& db */) { /* tut */ }

    bool EnsureSyncColumns(const std::string& table) override;
    nlohmann::json GetChangedRows(const std::string& table, const std::string& sinceIsoUtc) override;
    bool UpsertRows(const std::string& table, const nlohmann::json& rows) override;
    bool MarkDeleted(const std::string& table, const std::vector<std::string>& sync_ids) override;
    std::string GetLastSyncStampIsoUtc() override;
    void        SetLastSyncStampIsoUtc(const std::string& isoUtc) override;

private:
    // DatabaseManager& m_db;
    // yardÄ±mcÄ± SQL/ADO fonksiyonlarÄ±nâ€¦
};
