#pragma once
#include <string>
#include <nlohmann/json.hpp>

class ISyncInterface {
public:
    virtual ~ISyncInterface() = default;

    // Yerel deÄŸiÅŸiklik bildirimi (INSERT, UPDATE, DELETE sonrasÄ±)
    virtual void OnLocalChange(const std::string& table,
                               const std::string& action,
                               const nlohmann::json& rowData) = 0;
};
