#pragma once

#ifndef ISYNCLOCALADAPTER_H
#define ISYNCLOCALADAPTER_H


#include <string>
#include <vector>
#include <nlohmann/json.hpp>

class ISyncLocalAdapter {
public:
    virtual ~ISyncLocalAdapter() = default;

    virtual bool EnsureSyncColumns(const std::string& table) = 0;
    virtual nlohmann::json GetChangedRows(const std::string& table, const std::string& sinceIsoUtc) = 0;
    virtual bool UpsertRows(const std::string& table, const nlohmann::json& rows) = 0;
    virtual bool MarkDeleted(const std::string& table, const std::vector<std::string>& sync_ids) = 0;

    virtual std::string GetLastSyncStampIsoUtc() = 0;
    virtual void        SetLastSyncStampIsoUtc(const std::string& isoUtc) = 0;
};


#endif
