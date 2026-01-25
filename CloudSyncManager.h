// CloudSyncManager.h
#pragma once
#ifndef CLOUDSYNCMANAGER_H
#define CLOUDSYNCMANAGER_H
#include "stdafx.h"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>
#include <cpr/cpr.h>

class CloudSyncManager
{
public:
    using json = nlohmann::json;

    explicit CloudSyncManager(const std::string& accessToken);
    ~CloudSyncManager() = default;

    // Google Drive'a dosya yükler (ör: veriler.accdb)
    bool UploadFile(const std::string& filePath,
        const std::string& mimeType,
        const std::string& folderId = std::string());
    void LogInfo(const std::string& msg);

    // Google Drive'dan dosya indirir.
    bool DownloadFile(const std::string& fileId,
        const std::string& savePath);

    // Drive içeriğini listeler (isteğe bağlı arama kriteri ile).
    std::vector<std::string> ListFiles(const std::string& query = std::string());

private:
    std::string m_accessToken;

    // Yardımcı fonksiyonlar
    std::string      ReadFileContent(const std::string& path);
    static std::string URLEncode(const std::string& value);
    void             LogError(const std::string& msg);
};


#endif // CLOUDSYNCMANAGER_H