#include "stdafx.h"
#include "GoogleServices.h"
#include <cpr/cpr.h>

using json = nlohmann::json;

GoogleServices::GoogleServices(const std::string& accessToken)
    : m_token(accessToken)
{
}

std::string GoogleServices::MakeRequest(const std::string& url, const json& body)
{
    cpr::Response r = cpr::Post(
        cpr::Url{ url },
        cpr::Header{
            { "Authorization", "Bearer " + m_token },
            { "Content-Type", "application/json" }
        },
        cpr::Body{ body.dump() }
    );

    if (r.status_code >= 200 && r.status_code < 300)
        return r.text;
    
    // Hata durumunda debug için loglanabilir
    OutputDebugStringA(("[GoogleServices] Hata: " + r.text + "\n").c_str());
    return "";
}

std::string GoogleServices::CreateContact(const std::string& firstName, const std::string& lastName, const std::string& phone, const std::string& email)
{
    // People API createContact formatı
    json person;
    
    // İsimler
    person["names"] = json::array({
        { {"givenName", firstName}, {"familyName", lastName} }
    });

    // Telefon
    if (!phone.empty()) {
        person["phoneNumbers"] = json::array({
            { {"value", phone}, {"type", "mobile"} }
        });
    }

    // Email
    if (!email.empty()) {
        person["emailAddresses"] = json::array({
      { {"value", email}, {"type", "home"} }
   });
    }

    std::string response = MakeRequest("https://people.googleapis.com/v1/people:createContact", person);
    
  if (response.empty()) return "";

    try {
        auto j = json::parse(response);
        if (j.contains("resourceName"))
            return j["resourceName"].get<std::string>();
    } catch(...) {}

return "";
}

// ============================================================================
// ✅ YENİ: Gmail Contacts'tan tüm kişileri çek
// ============================================================================
std::vector<std::pair<std::string, std::string>> GoogleServices::GetAllContacts()
{
    std::vector<std::pair<std::string, std::string>> contacts;  // {name, phone}

    // ✅ STEP 1: API URL oluştur
    std::string url = "https://people.googleapis.com/v1/people/me/connections"
        "?personFields=names,phoneNumbers,emailAddresses"
     "&pageSize=1000";  // Maksimum 1000 kişi

    try
    {
        // ✅ STEP 2: GET isteği gönder
      cpr::Response r = cpr::Get(
            cpr::Url{ url },
         cpr::Header{
            { "Authorization", "Bearer " + m_token },
 { "Content-Type", "application/json" }
         },
            cpr::Timeout{ 30000 }  // 30 saniye timeout
        );

        // ✅ STEP 3: HTTP status kontrol
        if (r.status_code < 200 || r.status_code >= 300)
{
   OutputDebugStringA(("[GoogleServices] GetAllContacts HTTP hatası: " + 
   std::to_string(r.status_code) + "\n").c_str());
            return contacts;
        }

        // ✅ STEP 4: JSON parse
      auto responseJson = json::parse(r.text);

    if (!responseJson.contains("connections") || !responseJson["connections"].is_array())
    {
            OutputDebugStringA("[GoogleServices] Contacts bulunamadı\n");
     return contacts;
        }

        // ✅ STEP 5: Her kişiyi işle
        for (const auto& connection : responseJson["connections"])
        {
    std::string name;
       std::string phone;
     std::string email;

// Ad çıkart
          if (connection.contains("names") && connection["names"].is_array() && 
!connection["names"].empty())
{
            const auto& nameObj = connection["names"][0];
    if (nameObj.contains("displayName") && nameObj["displayName"].is_string())
              {
            name = nameObj["displayName"].get<std::string>();
        }
            }

    // Telefon numarası çıkart (ilk telefonu al)
  if (connection.contains("phoneNumbers") && connection["phoneNumbers"].is_array() &&
              !connection["phoneNumbers"].empty())
       {
const auto& phoneObj = connection["phoneNumbers"][0];
         if (phoneObj.contains("value") && phoneObj["value"].is_string())
        {
      phone = phoneObj["value"].get<std::string>();
    }
        }

   // Email çıkart (ilk emaili al)
            if (connection.contains("emailAddresses") && connection["emailAddresses"].is_array() &&
     !connection["emailAddresses"].empty())
     {
        const auto& emailObj = connection["emailAddresses"][0];
      if (emailObj.contains("value") && emailObj["value"].is_string())
         {
          email = emailObj["value"].get<std::string>();
   }
            }

 // ✅ STEP 6: En az ad ve telefon varsa ekle
            if (!name.empty() && !phone.empty())
     {
                contacts.push_back({ name, phone });
              
     std::string debugMsg = "[GoogleServices] Kişi eklendi: " + name + " - " + phone + "\n";
      OutputDebugStringA(debugMsg.c_str());
        }
  }

     std::string debugMsg = "[GoogleServices] Toplam " + std::to_string(contacts.size()) + 
            " kişi Gmail'den çekildi\n";
    OutputDebugStringA(debugMsg.c_str());
    }
    catch (const json::exception& e)
    {
        OutputDebugStringA(("[GoogleServices] JSON parse hatası: " + std::string(e.what()) + "\n").c_str());
    }
    catch (const std::exception& e)
    {
        OutputDebugStringA(("[GoogleServices] Exception: " + std::string(e.what()) + "\n").c_str());
  }

    return contacts;
}

bool GoogleServices::CreateEvent(const std::string& title, const std::string& description, const std::string& startIso, const std::string& endIso)
{
    // Calendar API insert formatı
    json evt;
    evt["summary"] = title;
    evt["description"] = description;
    
    evt["start"] = { {"dateTime", startIso}, {"timeZone", "UTC"} };
    evt["end"]   = { {"dateTime", endIso},   {"timeZone", "UTC"} };

    std::string response = MakeRequest("https://www.googleapis.com/calendar/v3/calendars/primary/events", evt);
    
    return !response.empty();
}