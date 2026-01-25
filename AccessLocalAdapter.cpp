#include "AccessLocalAdapter.h"
#include <sstream>

AccessLocalAdapter::AccessLocalAdapter(const std::wstring& cs)
    : AdoLocalAdapter(cs) {}

bool AccessLocalAdapter::EnsureSyncColumns(const std::string& table) {
    std::wstring t(table.begin(), table.end());

    ExecuteNonQuery(L"ALTER TABLE " + t + L" ADD COLUMN sync_id TEXT(64)");
    ExecuteNonQuery(L"ALTER TABLE " + t + L" ADD COLUMN updated_at TEXT(30)");
    ExecuteNonQuery(L"ALTER TABLE " + t + L" ADD COLUMN deleted BIT DEFAULT 0");

    return true; // Access'te hata varsa catch'e düşer
}

std::vector<Row> AccessLocalAdapter::GetChangedRows(const std::string& table,
                                                    const std::string& lastSyncIsoUtc) {
    std::stringstream ss;
    ss << "SELECT * FROM " << table
       << " WHERE updated_at IS NULL OR updated_at > '" << lastSyncIsoUtc << "'";

    std::wstring sql(ss.str().begin(), ss.str().end());
    return ExecuteQuery(sql);
}

bool AccessLocalAdapter::ApplyRemoteChanges(const std::string& table,
                                            const std::vector<Row>& rows) {
    for (auto& row : rows) {
        // burada istersen UPSERT mantığı kurarız (varsa update, yoksa insert)
        // şu an iskelet
    }
    return true;
}

bool AccessLocalAdapter::MarkAsSynced(const std::string& table,
                                      const std::vector<std::string>& syncIds,
                                      const std::string& stamp) {
    for (auto& id : syncIds) {
        std::stringstream ss;
        ss << "UPDATE " << table << " SET updated_at='" << stamp
           << "' WHERE sync_id='" << id << "'";

        ExecuteNonQuery(std::wstring(ss.str().begin(), ss.str().end()));
    }
    return true;
}
