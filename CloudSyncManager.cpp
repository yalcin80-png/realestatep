// CloudSyncManager.cpp
#include "stdafx.h"
#include "CloudSyncManager.h"
#include "CloudHttp.h"

#include <fstream>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <random>

using json = nlohmann::json;

// ============================================================
//  Utility
// ============================================================

std::string CloudSyncManager::URLEncode(const std::string& s)
{
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (unsigned char c : s)
    {
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
            escaped << c;
        else
            escaped << '%' << std::uppercase << std::setw(2)
            << static_cast<int>(c) << std::nouppercase;
    }
    return escaped.str();
}

std::string CloudSyncManager::ReadFileContent(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file) return {};

    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

void CloudSyncManager::LogError(const std::string& msg)
{
    OutputDebugStringA(("[CloudSyncManager] ERROR: " + msg + "\n").c_str());
    ::MessageBoxA(nullptr, msg.c_str(), "CloudSyncManager", MB_ICONERROR);
}

void CloudSyncManager::LogInfo(const std::string& msg)
{
    OutputDebugStringA(("[CloudSyncManager] " + msg + "\n").c_str());
}

// ============================================================
//  Multipart boundary
// ============================================================

static std::string GenerateBoundary()
{
    static const char chars[] =
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    std::mt19937 rng{ std::random_device{}() };
    std::uniform_int_distribution<> dist(0, sizeof(chars) - 2);

    std::string b = "----EmlakCRM_Boundary_";
    for (int i = 0; i < 16; ++i)
        b += chars[dist(rng)];

    return b;
}

// ============================================================
//  Ctor
// ============================================================

CloudSyncManager::CloudSyncManager(const std::string& accessToken)
    : m_accessToken(accessToken)
{
}

// ============================================================
//  Upload (Create / Overwrite ready)
// ============================================================

bool CloudSyncManager::UploadFile(const std::string& filePath,
    const std::string& mimeType,
    const std::string& folderId)
{
    try
    {
        LogInfo("UploadFile başladı: " + filePath);

        if (m_accessToken.empty())
        {
            LogError("Access token yok.");
            return false;
        }

        if (!std::filesystem::exists(filePath))
        {
            LogError("Dosya yok: " + filePath);
            return false;
        }

        std::string fileName =
            std::filesystem::path(filePath).filename().string();

        std::string fileData = ReadFileContent(filePath);
        if (fileData.empty())
        {
            LogError("Dosya okunamadı.");
            return false;
        }

        // ---- Metadata
        json metadata;
        metadata["name"] = fileName;
        if (!folderId.empty())
            metadata["parents"] = json::array({ folderId });

        // ---- Multipart body
        std::string boundary = GenerateBoundary();
        std::ostringstream body;

        body << "--" << boundary << "\r\n";
        body << "Content-Type: application/json; charset=UTF-8\r\n\r\n";
        body << metadata.dump() << "\r\n";

        body << "--" << boundary << "\r\n";
        body << "Content-Type: " << (mimeType.empty() ? "application/octet-stream" : mimeType) << "\r\n\r\n";
        body << fileData << "\r\n";

        body << "--" << boundary << "--";

        // ---- Header
        cpr::Header header = MakeAuthHeader(m_accessToken);
        header["Content-Type"] = "multipart/related; boundary=" + boundary;

        // ---- POST
        auto resp = cpr::Post(
            cpr::Url{ "https://www.googleapis.com/upload/drive/v3/files?uploadType=multipart" },
            header,
            cpr::Body{ body.str() },
            cpr::Timeout{ 60000 }
        );

        LogInfo("HTTP: " + std::to_string(resp.status_code));

        if (resp.status_code >= 200 && resp.status_code < 300)
        {
            LogInfo("Upload başarılı.");
            return true;
        }

        LogError("Upload başarısız.\n" + resp.text);
        return false;
    }
    catch (const std::exception& ex)
    {
        LogError(std::string("Exception: ") + ex.what());
        return false;
    }
}

// ============================================================
//  Download
// ============================================================

bool CloudSyncManager::DownloadFile(const std::string& fileId,
    const std::string& savePath)
{
    if (m_accessToken.empty())
        return false;

    auto resp = cpr::Get(
        cpr::Url{ "https://www.googleapis.com/drive/v3/files/" + fileId + "?alt=media" },
        MakeAuthHeader(m_accessToken),
        cpr::Timeout{ 60000 }
    );

    if (resp.status_code >= 200 && resp.status_code < 300)
    {
        std::ofstream out(savePath, std::ios::binary);
        out.write(resp.text.data(), resp.text.size());
        return true;
    }

    LogError("Download hatası: " + std::to_string(resp.status_code));
    return false;
}

// ============================================================
//  List
// ============================================================

std::vector<std::string> CloudSyncManager::ListFiles(const std::string& query)
{
    std::vector<std::string> files;

    std::string url =
        "https://www.googleapis.com/drive/v3/files?fields=files(id,name)";

    if (!query.empty())
        url += "&q=" + URLEncode(query);

    auto resp = cpr::Get(
        cpr::Url{ url },
        MakeAuthHeader(m_accessToken),
        cpr::Timeout{ 30000 }
    );

    if (resp.status_code != 200)
    {
        LogError("Listeleme hatası.");
        return files;
    }

    auto j = json::parse(resp.text, nullptr, false);
    if (!j.is_object() || !j.contains("files")) return files;

    for (auto& f : j["files"])
        if (f.contains("name"))
            files.push_back(f["name"]);

    return files;
}
