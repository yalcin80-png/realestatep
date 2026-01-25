

#ifndef HICLOUDPROVIDER_H
#define HICLOUDPROVIDER_H



#pragma once

#include <string>
#include <nlohmann/json.hpp>

class ICloudProvider
{
public:
    using json = nlohmann::json;

    virtual ~ICloudProvider() = default;

    // Belirli bir koleksiyondaki, verilen zaman damgasından sonraki değişiklikleri getirir.
    virtual json PullChanges(const std::string& collection,
        const std::string& sinceIsoUtc) = 0;

    // Koleksiyona upsert (insert/update) gönderir.
    virtual bool PushUpserts(const std::string& collection,
        const json& rows) = 0;

    // Koleksiyona silme (soft delete) gönderir.
    virtual bool PushDeletes(const std::string& collection,
        const json& sync_ids) = 0;

    // Sunucu için kullanılacak UTC ISO 8601 zaman damgası üretir.
    virtual std::string NowIsoUtc() = 0;
};


#endif // HICLOUDPROVIDER_H