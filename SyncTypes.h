// SyncTypes.h
#pragma once
#include <string>
#include <map>

struct SyncRecord
{
    std::string table;      // "customers", "properties"
    std::string sync_id;    // GUID
    std::string updated_at; // ISO 8601 UTC
    bool deleted = false;

    std::map<std::string, std::string> fields;
};
