#pragma once
#include <vector>
#include <functional>
#include <string>
#include <optional>
#include "wxx_wincore.h"
#include "dataIsMe.h" 

struct SahibindenListingPayload
{
    CString listingUrl;
    CString listingNo;
    CString listingDate;
    CString emlakTipi;
    CString priceText;
    CString city;
    CString district;
    CString neighborhood;
    CString m2Brut;
    CString m2Net;
    CString roomCount;
    CString buildingAge;
    CString floor;
    CString totalFloor;
    CString heating;
    CString bathroomCount;
    CString kitchen;
    CString balcony;
    CString elevator;
    CString parking;
    CString furnished;
    CString usageStatus;
    CString inSite;
    CString siteName;
    CString dues;
    CString creditEligible;
    CString deedStatus;
    CString sellerType;
    CString swap;
    CString contactName;
    CString contactPhone;
    CString featuresText;
    
    // ---- NEW: Categorized Features (Images 2-3-4) ----
    CString facades;              // Cephe (multiple)
    CString featuresInterior;     // İç Özellikler (multiple)
    CString featuresExterior;     // Dış Özellikler (multiple)
    CString featuresNeighborhood; // Muhit (multiple)
    CString featuresTransport;    // Ulaşım (multiple)
    CString featuresView;         // Manzara (multiple)
    CString housingType;          // Konut Tipi (usually single)
    CString featuresAccessibility;// Engelli ve Yaşlıya Uygun (multiple)
    
    // ---- LAND (Arsa/Tarla/Bag/Bahçe) ----
// ---- LAND (Arsa/Tarla/Bag/Bahçe) detay alanları ----
    CString LAND_ListingTitle;     // HTML'den (og:title / h1 / title)
    CString LAND_ImarDurumu;       // "İmar Durumu" / dmpData: imar_durumu
    CString LAND_M2;               // "m²" / dmpData: m2
    CString LAND_M2Fiyati;         // "m² Fiyatı" / dmpData: m2_fiyati
    CString LAND_AdaNo;            // "Ada No" / dmpData: ada_no
    CString LAND_ParselNo;         // "Parsel No" / dmpData: parsel_no
    CString LAND_PaftaNo;          // "Pafta No" / dmpData: pafta_no
    CString LAND_KaksEmsal;        // "Kaks (Emsal)" / dmpData: kaks_emsal
    CString LAND_Gabari;           // "Gabari" / dmpData: gabari
    CString LAND_KrediUygunluk;    // "Krediye Uygunluk" / dmpData: krediye_uygunluk
    CString LAND_TapuDurumu;       // "Tapu Durumu" / dmpData: tapu_durumu
    CString LAND_Kimden;           // "Kimden" / dmpData: kimden
    CString LAND_Takas;            // "Takas"
    CString LAND_Cat2;             // customVars cat2 ("Arsa"), dmpData cat2 ("arsa") -> tip tespiti güçlendirir

    // ---- VILLA-specific fields ----
    CString VILLA_AcikAlanM2;      // "Açık Alan m²" / open area square meters
    
    // ---- CAR/VEHICLE (Araç) detay alanları ----
    CString CAR_Brand;             // "Marka" / brand
    CString CAR_Series;            // "Seri" / series
    CString CAR_Model;             // "Model" / model
    CString CAR_Year;              // "Yıl" / year
    CString CAR_Km;                // "KM" / kilometers
    CString CAR_FuelType;          // "Yakıt Tipi" / fuel type
    CString CAR_Transmission;      // "Vites Tipi" / transmission
    CString CAR_EngineVolume;      // "Motor Hacmi" / engine volume
    CString CAR_EnginePower;       // "Motor Gücü" / engine power
    CString CAR_Drive;             // "Çekiş" / drive type
    CString CAR_BodyType;          // "Kasa Tipi" / body type
    CString CAR_Color;             // "Renk" / color
    CString CAR_DamageRecord;      // "Hasar Kaydı" / damage record
    CString CAR_Warranty;          // "Garanti" / warranty
    CString CAR_Plate;             // "Plaka" / license plate
    CString CAR_VehicleCondition;  // "Araç Durumu" / vehicle condition
    CString CAR_Title;             // Listing title

};

class SahibindenImporter
{
public:
    using LogFn = std::function<void(const CString&)>;

    // Direct JSON Mode: WebView2 üzerinden alınan gaPageViewTrackingJson@data-json içeriğini (decode edilmiş saf JSON)
    // doğrudan işler. HTML wrapper gerektirmez.
    bool ImportFromJsonString(const CString& url, const std::string& jsonContentUtf8, LogFn log);
    // JSON'dan teknik detayları, HTML'den satıcı/iletişim bilgilerini alır.
    //bool ImportFromJsonAndHtmlString(const CString& url, const std::string& jsonContentUtf8, const std::string& htmlContentUtf8, LogFn log);

    // En güvenli yol: JSON + HTML birlikte işlenir.
    // - JSON: ilan no, fiyat, metrekare, oda, konum vb.
    // - HTML: satıcı adı / telefon (gösteriliyorsa) + ek özellikler.
    bool ImportFromJsonAndHtmlString(const CString& url,
        const std::string& jsonContentUtf8,
        const std::string& htmlContentUtf8,
        LogFn log);

    // Manuel girilen iletisim bilgilerini (kullanici onayli) zorla kullanmak icin overload.
    // Not: Bu overload, otomatik PII kazimayi degil; kullanicinin girdigi degerleri baz alir.
    bool ImportFromJsonAndHtmlString(const CString& url,
        const std::string& jsonContentUtf8,
        const std::string& htmlContentUtf8,
        const CString& manualContactName,
        const CString& manualContactPhone,
        LogFn log);
    bool ImportFromHtmlString(const CString& url, const std::string& htmlContent, LogFn log);

    // Ana Fonksiyon
    //bool ImportFromHtmlString(const CString& url, const std::string& htmlContent, LogFn log);
    CString ExtractIdFromUrl(const CString& url);

    // Fetch property data by property ID
    std::optional<IlanBilgisi> FetchByIlanNumarasi(const CString& ilanNumarasi);

private:
    // JSON Yöntemi (Hızlı)
    bool ExtractTrackingJson(const std::wstring& html, std::wstring& outJson);
    bool ParseTrackingJson(const CString& url, const std::wstring& jsonText, SahibindenListingPayload& outPayload, LogFn log);

    // HTML Kazıma Yöntemi (Yedek/Fallback)
    bool ParseHtmlDirectly(const CString& url, const std::wstring& html, SahibindenListingPayload& outPayload, LogFn log);

    // Ortak İşlemler
    std::wstring HtmlEntityDecode(const std::wstring& s);
    bool ExtractContactFromHtml(const std::wstring& html, SahibindenListingPayload& ioPayload);
    bool ExtractFeaturesFromHtml(const std::wstring& html, SahibindenListingPayload& ioPayload);
    bool SaveToDatabase(const SahibindenListingPayload& p, LogFn log);
    CString NormalizePhone(const CString& phone);
};