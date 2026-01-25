#include "stdafx.h"
#include "LoginManager.h"

// Winsock başlıkları (Windows.h'dan önce veya sonra olabilir, stdafx genelde yönetir)
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <shellapi.h>

#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <iomanip> 

// Linker'a ws2_32.lib kütüphanesini otomatik ekler
#pragma comment(lib, "ws2_32.lib")

#include <nlohmann/json.hpp>

// --- ÖNEMLİ DÜZELTME: API.H HATASINI GİDEREN KOD ---
// Windows'un DELETE makrosu ile CPR'ın çakışmasını engeller
#ifdef DELETE
#undef DELETE
#endif
// ----------------------------------------------------

#include <cpr/cpr.h>

using json = nlohmann::json;
static std::mutex g_loginMutex;
// =============================================================
// Kurucu ve Temel Fonksiyonlar
// =============================================================

LoginManager::LoginManager()
{
    InitTokenFilePath();
}

LoginManager::LoginManager(const std::string& clientId,
    const std::string& clientSecret,
    const std::string& redirectUri)
    : m_clientId(clientId)
    , m_clientSecret(clientSecret)
    , m_redirectUri(redirectUri)
{
    InitTokenFilePath();
}

//bool LoginManager::IsConfigured() const
//{
//    // client_secret can be empty for some OAuth client types, but we need at least
//    // client_id and redirect_uri to start an interactive login.
//    return !m_clientId.empty() && !m_redirectUri.empty();
//}

std::string LoginManager::GetAccessToken() const
{
    return m_accessToken;
}

bool LoginManager::IsConfigured() const
{
    return !m_clientId.empty() && !m_clientSecret.empty() && !m_redirectUri.empty();
}

void LoginManager::InitTokenFilePath()
{
    // Profesyonel yaklaşım: Tokenları Program Files veya Exe yanında değil, 
    // Kullanıcının AppData/Local klasöründe saklamak.
    PWSTR path = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &path)))
    {
        std::wstring baseDir(path);
        m_tokenFilePath = baseDir + L"\\EmlakProgrami";

        // Klasör yoksa oluştur
        CreateDirectoryW(m_tokenFilePath.c_str(), nullptr);

        m_tokenFilePath += L"\\google_oauth_tokens.json";
        CoTaskMemFree(path);
    }
    else
    {
        // Fallback: Exe yanına kaydet
        m_tokenFilePath = L"google_oauth_tokens.json";
    }
}
// =============================================================
// Çıkış Yapma (Logout) İşlemleri
// =============================================================

void LoginManager::SignOut()
{
    std::lock_guard<std::mutex> lock(g_loginMutex);

    try {
        RevokeTokenOnServer();
    }
    catch (const std::exception& e) {
        // LogError(e.what());
    }

    m_accessToken.clear();
    m_refreshToken.clear();
    m_expiryUtc = std::chrono::system_clock::time_point{};

    if (GetFileAttributesW(m_tokenFilePath.c_str()) != INVALID_FILE_ATTRIBUTES)
    {
        if (!DeleteFileW(m_tokenFilePath.c_str()))
        {
            // Dosya kilitliyse içeriğini temizle
            std::ofstream ofs(m_tokenFilePath, std::ios::trunc);
        }
    }
}



void LoginManager::RevokeTokenOnServer()
{
    if (m_accessToken.empty() && m_refreshToken.empty())
        return;

    std::string tokenToRevoke = m_accessToken;
    if (tokenToRevoke.empty())
        tokenToRevoke = m_refreshToken;

    // DLL çökmesini önlemek için manuel string body kullanıyoruz
    std::string bodyStr = "token=" + tokenToRevoke;

    cpr::Post(
        cpr::Url{ "https://oauth2.googleapis.com/revoke" },
        cpr::Body{ bodyStr.c_str() },
        cpr::Header{ { "Content-Type", "application/x-www-form-urlencoded" } }
    );
}

// =============================================================
// Token Yönetimi
// =============================================================

bool LoginManager::LoadTokensFromDisk()
{
    std::ifstream ifs(m_tokenFilePath, std::ios::in | std::ios::binary);
    if (!ifs.is_open())
        return false;

    try
    {
        json j;
        ifs >> j;

        if (j.contains("access_token") && j["access_token"].is_string())
            m_accessToken = j["access_token"].get<std::string>();

        if (j.contains("refresh_token") && j["refresh_token"].is_string())
            m_refreshToken = j["refresh_token"].get<std::string>();

        if (j.contains("expires_at") && j["expires_at"].is_number_integer())
        {
            long long expiresAt = j["expires_at"].get<long long>();
            using namespace std::chrono;
            m_expiryUtc = system_clock::time_point(seconds(expiresAt));
        }
        else
        {
            m_expiryUtc = std::chrono::system_clock::time_point{};
        }
    }
    catch (...)
    {
        return false;
    }

    return !m_accessToken.empty();
}

bool LoginManager::SaveTokensToDisk() const
{
    try
    {
        json j;
        j["access_token"] = m_accessToken;
        j["refresh_token"] = m_refreshToken;

        using namespace std::chrono;
        long long expiresAt = duration_cast<seconds>(m_expiryUtc.time_since_epoch()).count();
        j["expires_at"] = expiresAt;

        std::ofstream ofs(m_tokenFilePath, std::ios::out | std::ios::binary | std::ios::trunc);
        if (!ofs.is_open())
            return false;

        ofs << j.dump(2);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool LoginManager::HasValidAccessToken() const
{
    if (m_accessToken.empty())
        return false;

    using namespace std::chrono;
    auto now = system_clock::now();
    return (now + std::chrono::seconds(30)) < m_expiryUtc;
}

// =============================================================
// Giriş Akışı
// =============================================================

bool LoginManager::SignIn(HWND ownerHwnd)
{
    if (LoadTokensFromDisk())
    {
        if (HasValidAccessToken())
            return true;

        if (!m_refreshToken.empty())
        {
            if (RefreshAccessToken())
            {
                SaveTokensToDisk();
                return true;
            }
        }
    }

    return StartInteractiveLogin(ownerHwnd);
}

// =============================================================
// API İstekleri (GÜVENLİ METHODLAR)
// =============================================================

bool LoginManager::RefreshAccessToken()
{
    if (m_refreshToken.empty()) return false;

    // Token yenileme isteği
    cpr::Payload payload{
        {"client_id", m_clientId},
        {"client_secret", m_clientSecret},
        {"refresh_token", m_refreshToken},
        {"grant_type", "refresh_token"}
    };

    auto r = cpr::Post(
        cpr::Url{ "https://oauth2.googleapis.com/token" },
        payload
    );

    if (r.status_code != 200) return false;

    try {
        auto j = json::parse(r.text);
        m_accessToken = j.at("access_token").get<std::string>();

        int expiresIn = j.value("expires_in", 3600);
        m_expiryUtc = std::chrono::system_clock::now() + std::chrono::seconds(expiresIn);
        return true;
    }
    catch (const json::exception&) {
        return false;
    }
}
bool LoginManager::ExchangeCodeForTokens(const std::string& authCode)
{
    std::ostringstream os;
    os << "client_id=" << UrlEncode(m_clientId)
        << "&client_secret=" << UrlEncode(m_clientSecret)
        << "&code=" << UrlEncode(authCode)
        << "&grant_type=authorization_code"
        << "&redirect_uri=" << UrlEncode(m_redirectUri);

    std::string bodyStr = os.str();

    auto r = cpr::Post(
        cpr::Url{ "https://oauth2.googleapis.com/token" },
        cpr::Body{ bodyStr.c_str() }, // Güvenli ham veri
        cpr::Header{ {"Content-Type", "application/x-www-form-urlencoded"} }
    );

    if (r.status_code != 200)
        return false;

    try
    {
        json j = json::parse(r.text);

        if (!j.contains("access_token"))
            return false;

        m_accessToken = j["access_token"].get<std::string>();

        if (j.contains("refresh_token") && j["refresh_token"].is_string())
            m_refreshToken = j["refresh_token"].get<std::string>();

        int expiresIn = 3600;
        if (j.contains("expires_in") && j["expires_in"].is_number_integer())
            expiresIn = j["expires_in"].get<int>();

        m_expiryUtc = std::chrono::system_clock::now() + std::chrono::seconds(expiresIn);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

// =============================================================
// Interactive Login & Socket Listener
// =============================================================
bool LoginManager::StartInteractiveLogin(HWND ownerHwnd)
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        MessageBox(ownerHwnd, L"Winsock başlatılamadı.", L"Hata", MB_ICONERROR);
        return false;
    }

    if (m_redirectUri.empty())
    {
        MessageBox(ownerHwnd, L"Redirect URI eksik!", L"Hata", MB_ICONERROR);
        WSACleanup();
        return false;
    }

    const std::string authEndpoint = "https://accounts.google.com/o/oauth2/v2/auth";

    // YENİ SCOPE LİSTESİ (Rehber, Takvim ve Firestore dahil)
    const std::string scope =
        "email profile "
        "https://www.googleapis.com/auth/contacts "   // Google Contacts
        "https://www.googleapis.com/auth/calendar "   // Google Calendar
        "https://www.googleapis.com/auth/datastore "  // Firestore
        "https://www.googleapis.com/auth/cloud-platform ";

    std::ostringstream oss;
    oss << authEndpoint
        << "?client_id=" << UrlEncode(m_clientId)
        << "&redirect_uri=" << UrlEncode(m_redirectUri)
        << "&response_type=code"
        << "&scope=" << UrlEncode(scope)
        << "&access_type=offline"
        << "&prompt=consent";

    std::string url = oss.str();
    std::wstring wurl(url.begin(), url.end());

    // Tarayıcıyı açacak ve kodu bekleyecek fonksiyon
    std::string authCode = WaitForAuthCode(8888, wurl);

    WSACleanup();

    if (authCode.empty()) return false;

    if (!ExchangeCodeForTokens(authCode))
    {
        MessageBox(ownerHwnd, L"Token alınamadı.", L"Hata", MB_ICONERROR);
        return false;
    }

    SaveTokensToDisk();
    return true;
}

//bool LoginManager::StartInteractiveLogin(HWND ownerHwnd)
//{
//    WSADATA wsaData;
//    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
//    {
//        MessageBox(ownerHwnd, L"Winsock başlatılamadı.", L"Hata", MB_ICONERROR);
//        return false;
//    }
//
//    const std::string authEndpoint = "https://accounts.google.com/o/oauth2/v2/auth";
//    const std::string scope = "https://www.googleapis.com/auth/drive.file";
//
//    std::ostringstream oss;
//    oss << authEndpoint
//        << "?client_id=" << UrlEncode(m_clientId)
//        << "&redirect_uri=" << UrlEncode(m_redirectUri)
//        << "&response_type=code"
//        << "&scope=" << UrlEncode(scope)
//        << "&access_type=offline"
//        << "&prompt=consent";
//
//    std::string url = oss.str();
//    std::wstring wurl(url.begin(), url.end());
//
//    if (!OpenUrlInBrowser(wurl))
//    {
//        MessageBox(ownerHwnd, L"Tarayıcı açılamadı.", L"Hata", MB_ICONERROR);
//        WSACleanup();
//        return false;
//    }
//
//    std::string authCode = WaitForAuthCode(8888);
//
//    WSACleanup();
//
//    if (authCode.empty())
//    {
//        MessageBox(ownerHwnd, L"Kod alınamadı.", L"Hata", MB_ICONERROR);
//        return false;
//    }
//
//    if (!ExchangeCodeForTokens(authCode))
//    {
//        MessageBox(ownerHwnd, L"Token alınamadı.", L"Hata", MB_ICONERROR);
//        return false;
//    }
//
//    SaveTokensToDisk();
//    return true;
//}

// =============================================================
// Interactive Login (Profesyonel Socket Yönetimi)
// =============================================================

std::string LoginManager::WaitForAuthCode(int port, const std::wstring& authUrl)
{
    // Socket için RAII wrapper (Otomatik kapatma sağlar)
    struct ScopedSocket {
        SOCKET s;
        ScopedSocket(SOCKET sock) : s(sock) {}
        ~ScopedSocket() { if (s != INVALID_SOCKET) closesocket(s); }
        operator SOCKET() { return s; }
    };

    SOCKET rawSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (rawSock == INVALID_SOCKET) return "";
    ScopedSocket serverSock(rawSock);

    int reuse = 1;
    setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1 daha güvenli
    addr.sin_port = htons(port);

    if (bind(serverSock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) return "";
    if (listen(serverSock, 1) == SOCKET_ERROR) return "";

    // Socket hazır, şimdi tarayıcıyı aç
    if (!authUrl.empty()) OpenUrlInBrowser(authUrl);

    // Select ile Timeout Yönetimi
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(serverSock, &fds);
    timeval tv{ 60, 0 }; // 60 saniye timeout

    if (select(0, &fds, nullptr, nullptr, &tv) <= 0) return "";

    SOCKET rawClient = accept(serverSock, nullptr, nullptr);
    if (rawClient == INVALID_SOCKET) return "";
    ScopedSocket clientSock(rawClient);

    std::vector<char> buffer(8192);
    int received = recv(clientSock, buffer.data(), (int)buffer.size(), 0);
    if (received <= 0) return "";

    std::string request(buffer.data(), received);

    // Auth Code Ayıklama (Regex veya daha sağlam string manipülasyonu)
    std::string code;
    size_t pos = request.find("code=");
    if (pos != std::string::npos) {
        size_t start = pos + 5;
        size_t end = request.find_first_of(" \r\n&", start);
        code = request.substr(start, end - start);
    }

    // Profesyonel HTTP Yanıtı
    std::string html = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\n\r\n"
        "<html><body style='font-family:sans-serif; text-align:center; padding-top:50px;'>"
        "<h1 style='color:#2ecc71;'>Giriş Başarılı!</h1>"
        "<p>Lütfen programa geri dönün. Bu sekme otomatik kapanabilir.</p>"
        "<script>window.setTimeout(function(){window.close();}, 3000);</script>"
        "</body></html>";

    send(clientSock, html.c_str(), (int)html.size(), 0);

    return code;
}
bool LoginManager::OpenUrlInBrowser(const std::wstring& url)
{
    HINSTANCE h = ShellExecuteW(nullptr, L"open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    return reinterpret_cast<int>(h) > 32;
}

// Yardımcı: URL Encoding
std::string LoginManager::UrlEncode(const std::string& value)
{
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (unsigned char c : value) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
        }
        else {
            escaped << '%' << std::uppercase << std::setw(2) << int(c);
        }
    }
    return escaped.str();
}

// LoginManager.cpp

bool LoginManager::IsLoggedIn()
{
    std::lock_guard<std::mutex> lock(g_loginMutex);

    if (HasValidAccessToken()) return true;

    if (LoadTokensFromDisk()) {
        if (HasValidAccessToken()) return true;
        if (!m_refreshToken.empty() && RefreshAccessToken()) {
            SaveTokensToDisk();
            return true;
        }
    }
    return false;
}







//std::string LoginManager::WaitForAuthCode(int port, const std::wstring& authUrl)
//{
//    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
//    if (serverSocket == INVALID_SOCKET) return "";
//
//    sockaddr_in serverAddr;
//    serverAddr.sin_family = AF_INET;
//    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
//    serverAddr.sin_port = htons(port);
//
//    // Portun kilitli kalmaması için Reuse Address
//    char yes = 1;
//    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
//
//    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
//    {
//        closesocket(serverSocket);
//        return "";
//    }
//
//    if (listen(serverSocket, 1) == SOCKET_ERROR)
//    {
//        closesocket(serverSocket);
//        return "";
//    }
//
//    // -----------------------------------------------------------------
//    // KRİTİK NOKTA: Socket artık dinliyor (listen durumunda).
//    // ŞİMDİ tarayıcıyı açıyoruz. Tarayıcı adrese gittiğinde port açık olacak.
//    // -----------------------------------------------------------------
//    if (!authUrl.empty())
//    {
//        OpenUrlInBrowser(authUrl);
//    }
//
//    // --- TIMEOUT MANTIĞI ---
//    fd_set readfds;
//    FD_ZERO(&readfds);
//    FD_SET(serverSocket, &readfds);
//
//    struct timeval tv;
//    tv.tv_sec = 60;  // 60 Saniye bekle
//    tv.tv_usec = 0;
//
//    // Burada bekliyoruz. Tarayıcı zaten açıldı ve port açık olduğu için 
//    // Google yönlendirmesi anında buraya düşecek.
//    int result = select(0, &readfds, NULL, NULL, &tv);
//
//    if (result <= 0) // Timeout veya Hata
//    {
//        closesocket(serverSocket);
//        return ""; // Kilitlenmeden çıkış
//    }
//
//    // Bağlantı geldi, kabul et
//    SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
//    if (clientSocket == INVALID_SOCKET)
//    {
//        closesocket(serverSocket);
//        return "";
//    }
//
//    // Veriyi oku
//    char buffer[4096];
//    int bytesReceived = recv(clientSocket, buffer, 4096, 0);
//    if (bytesReceived <= 0)
//    {
//        closesocket(clientSocket);
//        closesocket(serverSocket);
//        return "";
//    }
//
//    std::string request(buffer, bytesReceived);
//    std::string code;
//    size_t codePos = request.find("code=");
//    if (codePos != std::string::npos)
//    {
//        size_t start = codePos + 5;
//        size_t endPos = request.find_first_of(" &", start);
//        if (endPos != std::string::npos)
//            code = request.substr(start, endPos - start);
//        else
//            code = request.substr(start);
//    }
//
//    // Cevap gönder
//    std::string response =
//        "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n"
//        "<html><head><title>Giris Basarili</title></head>"
//        "<body><h1 style='color:green;text-align:center;margin-top:50px;'>Giris Basarili!</h1>"
//        "<script>window.close();</script></body></html>";
//
//    send(clientSocket, response.c_str(), (int)response.size(), 0);
//
//    closesocket(clientSocket);
//    closesocket(serverSocket);
//
//    return code;
//}