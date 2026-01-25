#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

class GoogleServices
{
public:
    explicit GoogleServices(const std::string& accessToken);

    // Google Contacts'a Kişi Ekleme
    // Dönüş: Oluşturulan kişinin Google Resource ID'si (örn: "people/12345")
    std::string CreateContact(const std::string& firstName, const std::string& lastName, const std::string& phone, const std::string& email);

    // ✅ YENİ: Gmail Contacts'tan tüm kişileri çek
    std::vector<std::pair<std::string, std::string>> GetAllContacts();

    // Google Calendar'a Etkinlik Ekleme
    // startIso/endIso formatı: "2023-12-31T14:00:00Z"
    bool CreateEvent(const std::string& title, const std::string& description, const std::string& startIso, const std::string& endIso);

private:
    std::string m_token;
    std::string MakeRequest(const std::string& url, const nlohmann::json& body);
};