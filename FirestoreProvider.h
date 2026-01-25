#pragma once

#include "stdafx.h"
#include "ICloudProvider.h"
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

class FirestoreProvider : public ICloudProvider
{
public:
    using json = nlohmann::json;

    FirestoreProvider(const std::string& accessToken,
        const std::string& projectId,
        const std::string& database = "(default)");

    virtual ~FirestoreProvider() = default;

    // ICloudProvider arayüzünden gelenler
    json PullChanges(const std::string& collection,
        const std::string& sinceIsoUtc) override;

    bool PushUpserts(const std::string& collection,
        const json& rows) override;

    bool PushDeletes(const std::string& collection,
        const json& sync_ids) override;

    std::string NowIsoUtc() override;

private:
    std::string m_token;
    std::string m_project;
    std::string m_database;

    // Yardımcı: Hata loglamak için
    void LogError(const std::string& prefix, const cpr::Response& r);
    void LogDebug(const std::string& msg);

    // Yardımcı: Dönüşümler
    static json RowToFirestore(const json& row);
    static json FirestoreToRow(const json& doc);
};