#include "stdafx.h"
#include "GooglePeopleProvider.h"

using json = nlohmann::json;

std::string GooglePeopleProvider::CreateContact(const std::string& adSoyad, const std::string& telefon, const std::string& email)
{
    // Ad Soyad'ı ayır (Basitçe)
    std::string ad = adSoyad;
    std::string soyad = "";
    size_t spacePos = adSoyad.find_last_of(' ');
    if (spacePos != std::string::npos) {
        ad = adSoyad.substr(0, spacePos);
        soyad = adSoyad.substr(spacePos + 1);
    }

    json body = {
        {"names",json::array({{ {"givenName", ad}, {"familyName", soyad} }})},
        {"phoneNumbers", json::array({{ {"value", telefon}, {"type", "mobile"} }})},
        {"emailAddresses", json::array({{ {"value", email}, {"type", "work"} }})}
    };

    auto resp = cpr::Post(
        cpr::Url{"https://people.googleapis.com/v1/people:createContact"},
        cpr::Header{
            {"Authorization", "Bearer " + m_token},
            {"Content-Type", "application/json"}
        },
        cpr::Body{body.dump()}
    );

    if (resp.status_code == 200) {
        auto j = json::parse(resp.text);
        return j["resourceName"].get<std::string>(); // Örn: "people/c12345..."
    }
    return "";
}