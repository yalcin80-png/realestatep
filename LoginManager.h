#ifndef LOGINMANAGER_H
#define LOGINMANAGER_H

#include <string>
#include <chrono>
#include <windows.h> // HWND ve temel Windows tipleri için

class LoginManager
{
public:
    // Default ctor is used by MainFrame before OAuth config is known.
    // We still initialize token file path so file operations are safe.
    LoginManager();
    ~LoginManager() {};
    // Kurucu metod: Client ID, Secret ve Redirect URI alır
    LoginManager(const std::string& clientId,
        const std::string& clientSecret,
        const std::string& redirectUri);
    bool IsLoggedIn();
    // Ana giriş metodu
    bool SignIn(HWND ownerHwnd);

    // Çıkış Yapma (Token iptali ve dosya silme)
    void SignOut();

    // Access Token'ı okumak için
    std::string GetAccessToken() const;

    // OAuth config is required for the interactive login flow.
    // Some code paths create LoginManager first and set config later.
    bool IsConfigured() const;

private:
    std::string m_clientId;
    std::string m_clientSecret;
    std::string m_redirectUri;
    std::wstring m_tokenFilePath;

    std::string m_accessToken;
    std::string m_refreshToken;
    std::chrono::system_clock::time_point m_expiryUtc;

    // Yardımcı dosya ve token fonksiyonları
    void InitTokenFilePath();
    bool LoadTokensFromDisk();
    bool SaveTokensToDisk() const;
    bool HasValidAccessToken() const;
    bool RefreshAccessToken();

    // Token'ı Google sunucusundan iptal etme
    void RevokeTokenOnServer();

    // OAuth akış fonksiyonları
    bool StartInteractiveLogin(HWND ownerHwnd);
    bool ExchangeCodeForTokens(const std::string& authCode);
    bool OpenUrlInBrowser(const std::wstring& url);

    // Otomatik kod yakalama fonksiyonu (Socket Listener)
    std::string WaitForAuthCode(int port);
    std::string WaitForAuthCode(int port, const std::wstring& authUrl);
    std::string UrlEncode(const std::string& value);
};

#endif // LOGINMANAGER_H