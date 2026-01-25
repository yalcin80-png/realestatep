#include "stdafx.h"
#include "CloudDBManager.h"

#include <fstream>
#include <sstream>
#include <filesystem>
#include <chrono>

using json = nlohmann::json;

// ----------------------------------------------------------
// Ctor
// ----------------------------------------------------------
CloudDBManager::CloudDBManager(LoginManager& login, const std::string& localPath)
    : m_login(login)
    , m_localPath(localPath)
{
}

// ----------------------------------------------------------
// Log helper
// ----------------------------------------------------------
void CloudDBManager::Log(const std::string& msg)
{
    CString w(msg.c_str());
    ::OutputDebugString(w + _T("\n"));
}

// ----------------------------------------------------------
// Google Drive’da dosya var mı?
// ----------------------------------------------------------
bool CloudDBManager::ExistsInCloud()
{
    m_fileId.clear();
    m_lastModified.clear();

    const std::string token = m_login.GetAccessToken();
    if (token.empty())
    {
        Log("[Drive] Access token yok.");
        return false;
    }

    cpr::Response r = cpr::Get(
        cpr::Url{ "https://www.googleapis.com/drive/v3/files" },
        cpr::Parameters{
            { "q", "name='veriler.db' and trashed=false" },
            { "fields", "files(id,name,modifiedTime,size)" }
        },
        cpr::Header{
            { "Authorization", "Bearer " + token }
        }
    );

    if (r.status_code != 200)
    {
        Log("[Drive] Exists HTTP hata: " + std::to_string(r.status_code));
        Log(r.text);
        return false;
    }

    try
    {
        json j = json::parse(r.text);

        if (!j.contains("files") || j["files"].empty())
            return false;

        auto& f = j["files"][0];
        m_fileId = f["id"].get<std::string>();
        m_lastModified = f.value("modifiedTime", "");

        Log("[Drive] Dosya bulundu. ID=" + m_fileId);
        return true;
    }
    catch (...)
    {
        Log("[Drive] Exists JSON parse hatası.");
        return false;
    }
}

// ----------------------------------------------------------
// Buluttan indir
// ----------------------------------------------------------
bool CloudDBManager::SyncFromCloud()
{
    if (!ExistsInCloud())
        return false;

    const std::string token = m_login.GetAccessToken();

    std::string url =
        "https://www.googleapis.com/drive/v3/files/" +
        m_fileId + "?alt=media";

    cpr::Response r = cpr::Get(
        cpr::Url{ url },
        cpr::Header{ { "Authorization", "Bearer " + token } }
    );

    if (r.status_code != 200)
    {
        Log("[Drive] Download HTTP hata: " + std::to_string(r.status_code));
        Log(r.text);
        return false;
    }

    std::ofstream ofs(m_localPath, std::ios::binary);
    ofs.write(r.text.data(), (std::streamsize)r.text.size());
    ofs.close();

    Log("[Drive] Buluttan indirildi.");
    return true;
}

// ----------------------------------------------------------
// Multipart body oluştur
// ----------------------------------------------------------
static std::string BuildMultipartBody(const std::string& boundary,
    const std::string& fileData)
{
    json meta;
    meta["name"] = "veriler.db";

    std::ostringstream body;

    body << "--" << boundary << "\r\n";
    body << "Content-Type: application/json; charset=UTF-8\r\n\r\n";
    body << meta.dump() << "\r\n";

    body << "--" << boundary << "\r\n";
    body << "Content-Type: application/octet-stream\r\n\r\n";
    body << fileData << "\r\n";

    body << "--" << boundary << "--";

    return body.str();
}

// ----------------------------------------------------------
// Yerelden Drive’a yükle (CREATE / UPDATE)
// ----------------------------------------------------------
bool CloudDBManager::SyncToCloud()
{
    const std::string token = m_login.GetAccessToken();
    if (token.empty())
    {
        Log("[Drive] Access token yok.");
        return false;
    }

    if (!std::filesystem::exists(m_localPath))
    {
        Log("[Drive] Yerel DB yok.");
        return false;
    }

    std::ifstream ifs(m_localPath, std::ios::binary);
    std::string fileData((std::istreambuf_iterator<char>(ifs)),
        std::istreambuf_iterator<char>());
    ifs.close();

    bool exists = ExistsInCloud();

    std::string boundary = "-------CRMFormBoundary7MA4YWxkTrZu0gW";
    std::string body = BuildMultipartBody(boundary, fileData);

    std::string url;
    if (exists)
    {
        url = "https://www.googleapis.com/upload/drive/v3/files/" +
            m_fileId + "?uploadType=multipart";
    }
    else
    {
        url = "https://www.googleapis.com/upload/drive/v3/files?uploadType=multipart";
    }

    cpr::Response r = cpr::Post(
        cpr::Url{ url },
        cpr::Header{
            { "Authorization", "Bearer " + token },
            { "Content-Type", "multipart/related; boundary=" + boundary }
        },
        cpr::Body{ body }
    );

    if (r.status_code >= 200 && r.status_code < 300)
    {
        Log("[Drive] Yükleme başarılı.");
        return true;
    }

    Log("[Drive] Upload HTTP hata: " + std::to_string(r.status_code));
    Log(r.text);
    return false;
}

// ----------------------------------------------------------
// İlk bağlantı
// ----------------------------------------------------------
bool CloudDBManager::ConnectToCloudDatabase()
{
    if (!std::filesystem::exists(m_localPath))
    {
        Log("[Drive] Yerel yok → bulut kontrol.");

        if (!SyncFromCloud())
        {
            Log("[Drive] Bulutta da yok → boş DB oluşturuldu.");
            std::ofstream ofs(m_localPath, std::ios::binary);
            ofs.close();
        }
    }
    else
    {
        Log("[Drive] Yerel DB mevcut.");
    }

    return true;
}

// ----------------------------------------------------------
// FileId getter
// ----------------------------------------------------------
std::string CloudDBManager::GetCloudFileId()
{
    if (m_fileId.empty())
        ExistsInCloud();
    return m_fileId;
}
