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

    // ---- CAR (Vasıta/Otomobil) detay alanları ----
    CString CAR_Brand;             // "Marka" / dmpData: marka
    CString CAR_Series;            // "Seri" / dmpData: seri
    CString CAR_Model;             // "Model" / dmpData: model
    CString CAR_Year;              // "Yıl" / dmpData: yil
    CString CAR_Km;                // "KM" / dmpData: km
    CString CAR_FuelType;          // "Yakıt Tipi" / dmpData: yakit_tipi
    CString CAR_Transmission;      // "Vites" / dmpData: vites
    CString CAR_BodyType;          // "Kasa Tipi" / dmpData: kasa_tipi
    CString CAR_EnginePower;       // "Motor Gücü" / dmpData: motor_gucu
    CString CAR_EngineVolume;      // "Motor Hacmi" / dmpData: motor_hacmi
    CString CAR_Drive;             // "Çekiş" / dmpData: cekis
    CString CAR_Color;             // "Renk" / dmpData: renk
    CString CAR_Warranty;          // "Garanti" / dmpData: garanti
    CString CAR_DamageRecord;      // "Ağır Hasar Kayıtlı" / dmpData: agir_hasar_kayitli
    CString CAR_Plate;             // "Plaka/Uyruk" / dmpData: plaka
    CString CAR_FromWho;           // "Kimden" / dmpData: kimden
    CString CAR_Cat2;              // customVars cat2 ("Otomobil"), dmpData cat2

    CString CAR_Title;             // İlan başlığı (og:title / title)
    CString CAR_VehicleCondition;  // "Araç Durumu" / dmpData: arac_durumu

    // ---- VILLA detay alanları (eksik olanlar) ----
    CString VILLA_OpenArea;        // "Açık Alan m²" / dmpData: acik_alan
    CString VILLA_AcikAlanM2;      // Legacy open area mapping
    CString VILLA_TotalFloors;     // "Kat Sayısı" / dmpData: kat_sayisi
    CString VILLA_Balcony;         // "Balkon" / dmpData: balkon
    CString VILLA_Elevator;        // "Asansör" / dmpData: asansor

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