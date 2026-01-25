#pragma once
#include "AdoLocalAdapter.h"

class AccessLocalAdapter final : public AdoLocalAdapter {
public:
    explicit AccessLocalAdapter(const std::wstring& connectionString);

    bool EnsureSyncColumns(const std::string& table) override;

    std::vector<Row> GetChangedRows(const std::string& table,
                                    const std::string& lastSyncIsoUtc) override;

    bool ApplyRemoteChanges(const std::string& table,
                            const std::vector<Row>& rows) override;

    bool MarkAsSynced(const std::string& table,
                      const std::vector<std::string>& syncIds,
                      const std::string& newStampIsoUtc) override;
};
