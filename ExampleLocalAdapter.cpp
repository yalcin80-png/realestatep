// ExampleLocalAdapter.cpp
#include "stdafx.h"
#include "ExampleLocalAdapter.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

bool ExampleLocalAdapter::EnsureSyncColumns(const std::string& table) {
    // TODO: tabloya (yoksa) ÅŸu alanlarÄ± ekle:
    // sync_id (TEXT/GUID), Updated_At(TEXT ISO8601), deleted(BOOLEAN default 0)
    // Access iÃ§in: ALTER TABLE ... ADD COLUMN, ve mevcut kayÄ±tlara default deÄŸer atama.
    return true;
}

json ExampleLocalAdapter::GetChangedRows(const std::string& table, const std::string& sinceIsoUtc) {
    // TODO: SELECT * FROM table WHERE Updated_At > ?;
    // SonuÃ§larÄ± JSON arrayâ€™e dÃ¶nÃ¼ÅŸtÃ¼r (her satÄ±r -> obje)
    return json::array();
}

bool ExampleLocalAdapter::UpsertRows(const std::string& table, const json& rows) {
    // TODO: her row iÃ§in:
    // IF EXISTS(sync_id) THEN UPDATE SET â€¦, Updated_At=? ELSE INSERT â€¦
    return true;
}

bool ExampleLocalAdapter::MarkDeleted(const std::string& table, const std::vector<std::string>& sync_ids) {
    // TODO: UPDATE table SET deleted=1, Updated_At=? WHERE sync_id IN (...)
    return true;
}

std::string ExampleLocalAdapter::GetLastSyncStampIsoUtc() {
    // TODO: app_settings tablosundan veya settings.jsonâ€™dan oku
    return std::string{};
}

void ExampleLocalAdapter::SetLastSyncStampIsoUtc(const std::string& isoUtc) {
    // TODO: kaydet
}
