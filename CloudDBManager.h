#pragma once
#ifndef CLOUDDBMANAGER_H
#define CLOUDDBMANAGER_H

#include "stdafx.h"
#include <string>
#include <ctime>
#include <fstream>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <filesystem>
#include "LoginManager.h"

class CloudDBManager
{
public:
    CloudDBManager(LoginManager& login, const std::string& localPath);

    bool ConnectToCloudDatabase();   // EÄŸer yerel yoksa indirir
    bool SyncToCloud();              // Yerelden buluta yÃ¼kler
    bool SyncFromCloud();            // Buluttan gÃ¼ncel dosyayÄ± indirir
    bool ExistsInCloud();            // Dosya Drive'da var mÄ± kontrol et
    std::string GetCloudFileId();    // Bulut dosya ID'si

private:
    std::string m_localPath;
    std::string m_fileId;            // Google Drive ID
    LoginManager& m_login;
    std::string m_lastModified;

    std::string GetFileModifiedTime();
    void Log(const std::string& msg);
};

#endif // CLOUDDBMANAGER_H

/*

LoginManager login(CLIENT_ID, CLIENT_SECRET, REDIRECT_URI);
if (login.SignIn()) {
    CloudDBManager cloud(login, "veriler.db");
    cloud.ConnectToCloudDatabase();
}
*/