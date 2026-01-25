#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include <cpr/cpr.h>

class GooglePeopleProvider
{
public:
    GooglePeopleProvider(const std::string& accessToken) : m_token(accessToken) {}

    // Google Kişiler'e yeni kişi ekler ve resourceName (Google ID) döner
    std::string CreateContact(const std::string& adSoyad, const std::string& telefon, const std::string& email);

    // Mevcut kişiyi günceller
    bool UpdateContact(const std::string& resourceName, const std::string& adSoyad, const std::string& telefon);

private:
    std::string m_token;
};