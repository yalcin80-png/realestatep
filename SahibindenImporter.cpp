#include "stdafx.h"
#include "SahibindenImporter.h"
#include <regex>
#include <vector>
#include <unordered_map>
#include "json.hpp"

using json = nlohmann::json;

// --- Yardımcılar ---
static CString Utf8ToCString(const std::string& utf8) {
    if (utf8.empty()) return _T("");
    int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), NULL, 0);
    std::wstring wstr(wlen, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), &wstr[0], wlen);
    return CString(wstr.c_str());
}
static CString ToCString(const std::wstring& s) { return CString(s.c_str()); }

// --- Map helpers (customVars + dmpData) ---
//
// KRITIK: Sahibinden'in customVars 'name' alanlari Turkce karakter iceriyor ("İlan Tarihi" gibi).
// Projedeki kaynak dosyasi kod sayfasi/encoding'i degisince dar string literal'lar ("İlan Tarihi")
// bazi makinalarda farkli byte dizisine donusebiliyor ve lookup FAIL oluyor.
// Bu da "sadece ilan no + url" gibi semptom uretiyor.
//
// Bu nedenle anahtar/degeleri wide (UTF-16) map'e cevirip wide literal ile karsilastiriyoruz.

static std::wstring Utf8ToWide(const std::string& s)
{
    if (s.empty()) return L"";
    int wlen = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), NULL, 0);
    if (wlen <= 0) return L"";
    std::wstring w(wlen, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), &w[0], wlen);
    return w;
}

static std::unordered_map<std::wstring, std::wstring> BuildNameValueMapW(const json& arr)
{
    std::unordered_map<std::wstring, std::wstring> m;
    if (!arr.is_array()) return m;
    for (const auto& it : arr)
    {
        if (!it.is_object()) continue;
        if (!it.contains("name") || !it.contains("value")) continue;
        if (!it["name"].is_string() || !it["value"].is_string()) continue;
        std::string n = it["name"].get<std::string>();
        std::string v = it["value"].get<std::string>();
        m[Utf8ToWide(n)] = Utf8ToWide(v);
    }
    return m;
}

static CString PickFirstW(
    const std::unordered_map<std::wstring, std::wstring>& a,
    std::initializer_list<const wchar_t*> keysA,
    const std::unordered_map<std::wstring, std::wstring>& b,
    std::initializer_list<const wchar_t*> keysB)
{
    for (auto k : keysA)
    {
        auto it = a.find(k);
        if (it != a.end() && !it->second.empty()) return CString(it->second.c_str());
    }
    for (auto k : keysB)
    {
        auto it = b.find(k);
        if (it != b.end() && !it->second.empty()) return CString(it->second.c_str());
    }
    return _T("");
}


static CString ExtractListingTitleFromHtmlSimple(const std::wstring& html)
{
    std::wsmatch m;
    std::wregex re(L"<meta[^>]*property\\s*=\\s*\"og:title\"[^>]*content\\s*=\\s*\"([^\"]+)\"[^>]*>",
        std::regex_constants::icase);
    if (std::regex_search(html, m, re) && m.size() > 1) return ToCString(m[1].str());

    std::wregex reH1(L"<h1[^>]*>\\s*([^<]+?)\\s*</h1>", std::regex_constants::icase);
    if (std::regex_search(html, m, reH1) && m.size() > 1) return ToCString(m[1].str());

    std::wregex reTitle(L"<title>\\s*([^<]+?)\\s*</title>", std::regex_constants::icase);
    if (std::regex_search(html, m, reTitle) && m.size() > 1) return ToCString(m[1].str());

    return _T("");
}
















// --- ANA FONKSİYON ---
bool SahibindenImporter::ImportFromHtmlString(const CString& url, const std::string& htmlContent, LogFn log)
{
    if (htmlContent.empty()) {
        if (log) log(_T("Hata: HTML içeriği boş."));
        return false;
    }

    // UTF8 -> Wide String
    int wlen = MultiByteToWideChar(CP_UTF8, 0, htmlContent.c_str(), (int)htmlContent.size(), NULL, 0);
    std::wstring wHtml(wlen, 0);
    MultiByteToWideChar(CP_UTF8, 0, htmlContent.c_str(), (int)htmlContent.size(), &wHtml[0], wlen);

    SahibindenListingPayload payload;
    bool bParsed = false;

    // YÖNTEM 1: JSON Verisini Dene (En Temiz Yöntem)
    std::wstring jsonText;
    if (ExtractTrackingJson(wHtml, jsonText))
    {
        if (ParseTrackingJson(url, jsonText, payload, log)) {
            if (log) log(_T("Veri yöntemi: JSON (Başarılı)"));
            bParsed = true;
        }
    }

    // YÖNTEM 2: JSON Başarısızsa HTML'i Doğrudan Oku (Fallback)
    if (!bParsed)
    {
        if (log) log(_T("Uyarı: JSON verisi bulunamadı, doğrudan HTML okunuyor..."));
        if (ParseHtmlDirectly(url, wHtml, payload, log)) {
            if (log) log(_T("Veri yöntemi: HTML Kazıma (Başarılı)"));
            bParsed = true;
        }
    }

    if (!bParsed) {
        if (log) log(_T("Kritik Hata: İlan verileri hiçbir yöntemle okunamadı. Sayfa tam yüklenmemiş olabilir."));
        return false;
    }

    // Ortak verileri tamamla (Telefon, Özellikler vb.)
    ExtractContactFromHtml(wHtml, payload);
    ExtractFeaturesFromHtml(wHtml, payload);
    // İlan başlığı (arsa dahil tüm tipler için faydalı)
    if (payload.LAND_ListingTitle.IsEmpty())
        payload.LAND_ListingTitle = ExtractListingTitleFromHtmlSimple(wHtml);

    // Kaydet
    if (log) log(_T("Veritabanına kaydediliyor..."));
    return SaveToDatabase(payload, log);
}

// --- Direct JSON Mode ---
bool SahibindenImporter::ImportFromJsonString(const CString& url, const std::string& jsonContentUtf8, LogFn log)
{
    if (jsonContentUtf8.size() < 10) {
        if (log) log(_T("Hata: JSON içeriği boş/çok kısa."));
        return false;
    }

    // Eski davranis: sadece JSON ile kaydet. (HTML yoksa satıcı/telefon eksik kalabilir.)
    return ImportFromJsonAndHtmlString(url, jsonContentUtf8, std::string(), log);
}


// --- YENİ EKLENEN FONKSİYON ---
bool SahibindenImporter::ImportFromJsonAndHtmlString(const CString& url, const std::string& jsonContentUtf8, const std::string& htmlContentUtf8, LogFn log)
{
    // Backward-compatible overload: manuel iletisim bilgisi yok.
    return ImportFromJsonAndHtmlString(url, jsonContentUtf8, htmlContentUtf8, CString(), CString(), log);
}

// --- Overload: kullanici onayli manuel iletisim bilgileriyle ---
bool SahibindenImporter::ImportFromJsonAndHtmlString(const CString& url,
    const std::string& jsonContentUtf8,
    const std::string& htmlContentUtf8,
    const CString& manualContactName,
    const CString& manualContactPhone,
    LogFn log)
{
    SahibindenListingPayload payload;
    payload.listingUrl = url;
    bool jsonSuccess = false;

    // 1. ADIM: JSON Parse (Teknik Detaylar İçin)
    if (!jsonContentUtf8.empty() && jsonContentUtf8.size() > 10) {
        try {
            json j = json::parse(jsonContentUtf8);
            auto customVars = j.contains("customVars") ? BuildNameValueMapW(j["customVars"]) : std::unordered_map<std::wstring, std::wstring>{};
            auto dmpData = j.contains("dmpData") ? BuildNameValueMapW(j["dmpData"]) : std::unordered_map<std::wstring, std::wstring>{};

            payload.listingNo = PickFirstW(customVars, { L"İlan No", L"ilan_no" }, dmpData, { L"classifiedId", L"id" });
            payload.listingDate = PickFirstW(customVars, { L"İlan Tarihi" }, dmpData, { L"ilan_tarihi" });
            payload.emlakTipi = PickFirstW(customVars, { L"Emlak Tipi" }, dmpData, { L"cat0" });

            payload.priceText = PickFirstW(customVars, { L"ilan_fiyat", L"Fiyat" }, dmpData, {});
            if (payload.priceText.IsEmpty()) {
                CString n = PickFirstW(customVars, {}, dmpData, { L"fiyat" });
                if (!n.IsEmpty()) payload.priceText = n + _T(" TL");
            }

            payload.city = PickFirstW(customVars, { L"loc2" }, dmpData, { L"loc2" });
            payload.district = PickFirstW(customVars, { L"loc3" }, dmpData, { L"loc3" });
            payload.neighborhood = PickFirstW(customVars, { L"loc5" }, dmpData, { L"loc5" });

            payload.m2Brut = PickFirstW(customVars, { L"m² (Brüt)", L"m2 (Brüt)" }, dmpData, { L"m2_brut" });
            payload.m2Net = PickFirstW(customVars, { L"m² (Net)", L"m2 (Net)" }, dmpData, { L"m2_net" });
            payload.roomCount = PickFirstW(customVars, { L"Oda Sayısı" }, dmpData, { L"oda_sayisi" });
            payload.buildingAge = PickFirstW(customVars, { L"Bina Yaşı" }, dmpData, { L"bina_yasi" });
            payload.floor = PickFirstW(customVars, { L"Bulunduğu Kat" }, dmpData, { L"bulundugu_kat" });
            payload.totalFloor = PickFirstW(customVars, { L"Kat Sayısı" }, dmpData, { L"kat_sayisi" });
            payload.heating = PickFirstW(customVars, { L"Isıtma" }, dmpData, { L"isitma" });
            payload.bathroomCount = PickFirstW(customVars, { L"Banyo Sayısı" }, dmpData, { L"banyo_sayisi" });
            payload.kitchen = PickFirstW(customVars, { L"Mutfak" }, dmpData, { L"mutfak" });
            payload.balcony = PickFirstW(customVars, { L"Balkon" }, dmpData, { L"balkon" });
            payload.elevator = PickFirstW(customVars, { L"Asansör" }, dmpData, { L"asansor" });
            payload.parking = PickFirstW(customVars, { L"Otopark" }, dmpData, { L"otopark" });
            payload.furnished = PickFirstW(customVars, { L"Eşyalı" }, dmpData, { L"esyali" });
            payload.usageStatus = PickFirstW(customVars, { L"Kullanım Durumu" }, dmpData, { L"kullanim_durumu" });
            payload.inSite = PickFirstW(customVars, { L"Site İçerisinde" }, dmpData, { L"site_icerisinde" });
            payload.siteName = PickFirstW(customVars, { L"Site Adı" }, dmpData, { L"site_adi" });
            payload.dues = PickFirstW(customVars, { L"Aidat (TL)", L"Aidat" }, dmpData, { L"aidat_tl" });
            payload.creditEligible = PickFirstW(customVars, { L"Krediye Uygun" }, dmpData, { L"krediye_uygun" });
            payload.deedStatus = PickFirstW(customVars, { L"Tapu Durumu" }, dmpData, { L"tapu_durumu" });
            payload.sellerType = PickFirstW(customVars, { L"Kimden" }, dmpData, { L"kimden" });
            payload.swap = PickFirstW(customVars, { L"Takas" }, dmpData, {});


            // ---- LAND (Arsa) teknik alanlar ----
            payload.LAND_Cat2 = PickFirstW(customVars, { L"cat2" }, dmpData, { L"cat2" });

            payload.LAND_ImarDurumu = PickFirstW(customVars, { L"İmar Durumu" }, dmpData, { L"imar_durumu" });
            payload.LAND_M2 = PickFirstW(customVars, { L"m²", L"Metrekare" }, dmpData, { L"m2" });
            payload.LAND_M2Fiyati = PickFirstW(customVars, { L"m² Fiyatı" }, dmpData, { L"m2_fiyati" });

            payload.LAND_AdaNo = PickFirstW(customVars, { L"Ada No", L"Ada" }, dmpData, { L"ada_no" });
            payload.LAND_ParselNo = PickFirstW(customVars, { L"Parsel No", L"Parsel" }, dmpData, { L"parsel_no" });
            payload.LAND_PaftaNo = PickFirstW(customVars, { L"Pafta No", L"Pafta" }, dmpData, { L"pafta_no" });

            payload.LAND_KaksEmsal = PickFirstW(customVars, { L"Kaks (Emsal)", L"KAKS", L"Emsal" }, dmpData, { L"kaks_emsal" });
            payload.LAND_Gabari = PickFirstW(customVars, { L"Gabari" }, dmpData, { L"gabari" });

            payload.LAND_KrediUygunluk = PickFirstW(customVars, { L"Krediye Uygunluk", L"Krediye Uygun" }, dmpData, { L"krediye_uygunluk" });
            payload.LAND_TapuDurumu = PickFirstW(customVars, { L"Tapu Durumu" }, dmpData, { L"tapu_durumu" });
            payload.LAND_Kimden = PickFirstW(customVars, { L"Kimden" }, dmpData, { L"kimden" });
            payload.LAND_Takas = PickFirstW(customVars, { L"Takas" }, dmpData, {});



            // ---- LAND (Arsa) teknik alanlar - GENIS KEY LIST ----
            payload.LAND_Cat2 = PickFirstW(customVars, { L"cat2" }, dmpData, { L"cat2" });

            // İmar
            payload.LAND_ImarDurumu = PickFirstW(
                customVars, { L"İmar Durumu", L"Imar Durumu", L"ZoningStatus" },
                dmpData, { L"imar_durumu", L"zoning_status" }
            );

            // m²
            payload.LAND_M2 = PickFirstW(
                customVars, { L"m²", L"m2", L"Metrekare", L"Tapu Alanı (m²)" },
                dmpData, { L"m2", L"m2_brut", L"m2_net", L"area" }
            );

            // m² fiyatı
            payload.LAND_M2Fiyati = PickFirstW(
                customVars, { L"m² Fiyatı", L"m2 Fiyatı", L"PricePerM2" },
                dmpData, { L"m2_fiyati", L"price_per_m2" }
            );

            // Ada/Parsel/Pafta
            payload.LAND_AdaNo = PickFirstW(
                customVars, { L"Ada No", L"Ada", L"Ada Numarası" },
                dmpData, { L"ada_no" }
            );
            payload.LAND_ParselNo = PickFirstW(
                customVars, { L"Parsel No", L"Parsel", L"Parsel Numarası" },
                dmpData, { L"parsel_no" }
            );
            payload.LAND_PaftaNo = PickFirstW(
                customVars, { L"Pafta No", L"Pafta" },
                dmpData, { L"pafta_no" }
            );

            // Kaks/Gabari
            payload.LAND_KaksEmsal = PickFirstW(
                customVars, { L"Kaks (Emsal)", L"KAKS", L"Emsal", L"Kaks" },
                dmpData, { L"kaks_emsal", L"kaks", L"emsal" }
            );
            payload.LAND_Gabari = PickFirstW(
                customVars, { L"Gabari" },
                dmpData, { L"gabari" }
            );

            // Kredi / Tapu / Kimden / Takas
            payload.LAND_KrediUygunluk = PickFirstW(
                customVars, { L"Krediye Uygunluk", L"Krediye Uygun", L"Krediye Uygunluk" },
                dmpData, { L"krediye_uygunluk", L"krediye_uygun" }
            );

            payload.LAND_TapuDurumu = PickFirstW(
                customVars, { L"Tapu Durumu" },
                dmpData, { L"tapu_durumu" }
            );

            payload.LAND_Kimden = PickFirstW(
                customVars, { L"Kimden" },
                dmpData, { L"kimden" }
            );

            payload.LAND_Takas = PickFirstW(customVars, { L"Takas" }, dmpData, { L"takas" });

            // ---- VILLA-specific fields ----
            // Açık Alan m² is specific to villas (outdoor area)
            payload.VILLA_AcikAlanM2 = PickFirstW(
                customVars, { L"Açık Alan m²", L"Açık Alan", L"Acik Alan m2" },
                dmpData, { L"acik_alan_m2", L"outdoor_area" }
            );

            // ---- CAR/VEHICLE (Araç) teknik alanlar ----
            // Brand and model information
            payload.CAR_Brand = PickFirstW(
                customVars, { L"Marka" },
                dmpData, { L"marka", L"brand" }
            );
            payload.CAR_Series = PickFirstW(
                customVars, { L"Seri" },
                dmpData, { L"seri", L"series" }
            );
            payload.CAR_Model = PickFirstW(
                customVars, { L"Model" },
                dmpData, { L"model" }
            );
            payload.CAR_Year = PickFirstW(
                customVars, { L"Yıl", L"Model Yılı" },
                dmpData, { L"yil", L"year", L"model_yili" }
            );
            
            // Mileage
            payload.CAR_Km = PickFirstW(
                customVars, { L"KM", L"Kilometre" },
                dmpData, { L"km", L"kilometre" }
            );
            
            // Engine and fuel
            payload.CAR_FuelType = PickFirstW(
                customVars, { L"Yakıt Tipi", L"Yakıt" },
                dmpData, { L"yakit_tipi", L"yakit", L"fuel_type" }
            );
            payload.CAR_Transmission = PickFirstW(
                customVars, { L"Vites Tipi", L"Vites" },
                dmpData, { L"vites_tipi", L"vites", L"transmission" }
            );
            payload.CAR_EngineVolume = PickFirstW(
                customVars, { L"Motor Hacmi" },
                dmpData, { L"motor_hacmi", L"engine_volume" }
            );
            payload.CAR_EnginePower = PickFirstW(
                customVars, { L"Motor Gücü" },
                dmpData, { L"motor_gucu", L"engine_power" }
            );
            
            // Drive and body
            payload.CAR_Drive = PickFirstW(
                customVars, { L"Çekiş" },
                dmpData, { L"cekis", L"drive" }
            );
            payload.CAR_BodyType = PickFirstW(
                customVars, { L"Kasa Tipi" },
                dmpData, { L"kasa_tipi", L"body_type" }
            );
            payload.CAR_Color = PickFirstW(
                customVars, { L"Renk" },
                dmpData, { L"renk", L"color" }
            );
            
            // Additional vehicle info
            payload.CAR_DamageRecord = PickFirstW(
                customVars, { L"Hasar Kaydı", L"Hasarlı" },
                dmpData, { L"hasar_kaydi", L"damage_record" }
            );
            payload.CAR_Warranty = PickFirstW(
                customVars, { L"Garanti", L"Garanti Durumu" },
                dmpData, { L"garanti", L"warranty" }
            );
            payload.CAR_Plate = PickFirstW(
                customVars, { L"Plaka" },
                dmpData, { L"plaka", L"plate" }
            );
            payload.CAR_VehicleCondition = PickFirstW(
                customVars, { L"Araç Durumu", L"Vasıta Durumu" },
                dmpData, { L"arac_durumu", L"vehicle_condition", L"vasita_durumu" }
            );



            if (payload.listingNo.IsEmpty()) payload.listingNo = ExtractIdFromUrl(url);
            jsonSuccess = true;
            if (log) log(_T("JSON Analizi Başarılı."));
        }
        catch (...) {
            if (log) log(_T("JSON Ayrıştırılamadı."));
        }
    }

    // 2. ADIM: HTML Parse (Ilan ozellikleri vb. icin)
    if (!htmlContentUtf8.empty()) {
        int wlen = MultiByteToWideChar(CP_UTF8, 0, htmlContentUtf8.c_str(), (int)htmlContentUtf8.size(), NULL, 0);
        std::wstring wHtml(wlen, 0);
        MultiByteToWideChar(CP_UTF8, 0, htmlContentUtf8.c_str(), (int)htmlContentUtf8.size(), &wHtml[0], wlen);

        // HTML Parse:
        // - Özellikler gibi PII-disi alanlari HTML'den aliyoruz.
        // - Bulk otomasyonda manualContactPhone/name bazi zamanlar bos gelebiliyor.
        //   Bu durumda (sadece bosken) HTML icinden telefon/isim kurtarmaya izin veriyoruz.
        //   (Regex tabanli; sayfa gorunur oldugunda genelde stabil.)
        ExtractFeaturesFromHtml(wHtml, payload);
        // İlan başlığı (arsa dahil tüm tipler için faydalı)
        if (payload.LAND_ListingTitle.IsEmpty())
            payload.LAND_ListingTitle = ExtractListingTitleFromHtmlSimple(wHtml);

        if (manualContactName.IsEmpty() && manualContactPhone.IsEmpty())
            ExtractContactFromHtml(wHtml, payload);

        // Eğer JSON başarısız olduysa, temel verileri de HTML'den kurtarmayı dene
        if (!jsonSuccess) {
            ParseHtmlDirectly(url, wHtml, payload, nullptr); // Log spam yapmaması için null
            if (log) log(_T("Veriler HTML üzerinden kurtarıldı."));
        }
    }

    // 3. ADIM: Manuel iletisim bilgileri (kullanici girisi) varsa uygula.
    if (!manualContactName.IsEmpty()) payload.contactName = manualContactName;
    if (!manualContactPhone.IsEmpty()) payload.contactPhone = manualContactPhone;

    if (log) log(_T("Veritabanına kaydediliyor..."));
    return SaveToDatabase(payload, log);
}






//bool SahibindenImporter::ImportFromJsonAndHtmlString(const CString& url,
//    const std::string& jsonContentUtf8,
//    const std::string& htmlContentUtf8,
//    LogFn log)
//{
//    SahibindenListingPayload payload;
//    payload.listingUrl = url;
//
//    try {
//        json j = json::parse(jsonContentUtf8);
//
//        if (j.contains("sellerName") && j["sellerName"].is_string())
//            payload.contactName = Utf8ToCString(j["sellerName"].get<std::string>());
//        if (j.contains("phoneText") && j["phoneText"].is_string())
//            payload.contactPhone = Utf8ToCString(j["phoneText"].get<std::string>());
//
//        auto customVars = j.contains("customVars") ? BuildNameValueMapW(j["customVars"]) : std::unordered_map<std::wstring, std::wstring>{};
//        auto dmpData    = j.contains("dmpData")    ? BuildNameValueMapW(j["dmpData"])    : std::unordered_map<std::wstring, std::wstring>{};
//        if (log) {
//            CString msg;
//            msg.Format(_T("customVars keys=%zu, dmpData keys=%zu"), customVars.size(), dmpData.size());
//            log(msg);
//        }
//
//        payload.listingNo   = PickFirstW(customVars, {L"İlan No", L"ilan_no", L"ilanNo"}, dmpData, {L"classifiedId", L"id"});
//        payload.listingDate = PickFirstW(customVars, {L"İlan Tarihi"}, dmpData, {L"ilan_tarihi"});
//        payload.emlakTipi   = PickFirstW(customVars, {L"Emlak Tipi"}, dmpData, {L"cat0"});
//
//        payload.priceText   = PickFirstW(customVars, {L"ilan_fiyat", L"Fiyat"}, dmpData, {});
//        if (payload.priceText.IsEmpty()) {
//            CString n = PickFirstW(customVars, {}, dmpData, {L"fiyat"});
//            if (!n.IsEmpty()) payload.priceText = n + _T(" TL");
//        }
//
//        payload.city         = PickFirstW(customVars, {L"loc2"}, dmpData, {L"loc2"});
//        payload.district     = PickFirstW(customVars, {L"loc3"}, dmpData, {L"loc3"});
//        payload.neighborhood = PickFirstW(customVars, {L"loc5"}, dmpData, {L"loc5"});
//
//        payload.m2Brut      = PickFirstW(customVars, {L"m² (Brüt)", L"m2 (Brüt)"}, dmpData, {L"m2_brut"});
//        payload.m2Net       = PickFirstW(customVars, {L"m² (Net)", L"m2 (Net)"},  dmpData, {L"m2_net"});
//        payload.roomCount   = PickFirstW(customVars, {L"Oda Sayısı"}, dmpData, {L"oda_sayisi"});
//        payload.buildingAge = PickFirstW(customVars, {L"Bina Yaşı"}, dmpData, {L"bina_yasi"});
//        payload.floor       = PickFirstW(customVars, {L"Bulunduğu Kat"}, dmpData, {L"bulundugu_kat"});
//        payload.totalFloor  = PickFirstW(customVars, {L"Kat Sayısı"}, dmpData, {L"kat_sayisi"});
//        payload.heating     = PickFirstW(customVars, {L"Isıtma"}, dmpData, {L"isitma"});
//        payload.bathroomCount = PickFirstW(customVars, {L"Banyo Sayısı"}, dmpData, {L"banyo_sayisi"});
//        payload.kitchen     = PickFirstW(customVars, {L"Mutfak"}, dmpData, {L"mutfak"});
//        payload.balcony     = PickFirstW(customVars, {L"Balkon"}, dmpData, {L"balkon"});
//        payload.elevator    = PickFirstW(customVars, {L"Asansör"}, dmpData, {L"asansor"});
//        payload.parking     = PickFirstW(customVars, {L"Otopark"}, dmpData, {L"otopark"});
//        payload.furnished   = PickFirstW(customVars, {L"Eşyalı"}, dmpData, {L"esyali"});
//        payload.usageStatus = PickFirstW(customVars, {L"Kullanım Durumu"}, dmpData, {L"kullanim_durumu"});
//        payload.inSite      = PickFirstW(customVars, {L"Site İçerisinde"}, dmpData, {L"site_icerisinde"});
//        payload.siteName    = PickFirstW(customVars, {L"Site Adı"}, dmpData, {L"site_adi"});
//        payload.dues        = PickFirstW(customVars, {L"Aidat (TL)", L"Aidat"}, dmpData, {L"aidat_tl"});
//        payload.creditEligible = PickFirstW(customVars, {L"Krediye Uygun"}, dmpData, {L"krediye_uygun"});
//        payload.deedStatus  = PickFirstW(customVars, {L"Tapu Durumu"}, dmpData, {L"tapu_durumu"});
//        payload.sellerType  = PickFirstW(customVars, {L"Kimden"}, dmpData, {L"kimden"});
//        payload.swap        = PickFirstW(customVars, {L"Takas"}, dmpData, {});
//
//        if (payload.listingNo.IsEmpty()) payload.listingNo = ExtractIdFromUrl(url);
//        if (log) log(_T("Veri yöntemi: Direct JSON (customVars+dmpData)"));
//    }
//    catch (...) {
//        if (log) log(_T("Hata: JSON parse edilemedi."));
//        return false;
//    }
//
//    // HTML varsa satıcı/telefon/ozellikler... tamamla
//    if (!htmlContentUtf8.empty()) {
//        int wlen = MultiByteToWideChar(CP_UTF8, 0, htmlContentUtf8.c_str(), (int)htmlContentUtf8.size(), NULL, 0);
//        std::wstring wHtml(wlen, 0);
//        MultiByteToWideChar(CP_UTF8, 0, htmlContentUtf8.c_str(), (int)htmlContentUtf8.size(), &wHtml[0], wlen);
//        ExtractContactFromHtml(wHtml, payload);
//        ExtractFeaturesFromHtml(wHtml, payload);
//    }
//
//    if (log) log(_T("Veritabanına kaydediliyor..."));
//    return SaveToDatabase(payload, log);
//}

// --- HTML KAZIMA (YEDEK SİSTEM) ---
bool SahibindenImporter::ParseHtmlDirectly(const CString& url, const std::wstring& html, SahibindenListingPayload& p, LogFn log)
{
    p.listingUrl = url;
    p.listingNo = ExtractIdFromUrl(url);

    // 1. Fiyatı Bul (class="classifiedInfo" içindeki h3)
    std::wregex rePrice(L"class=\"classifiedInfo\"[^>]*>\\s*<h3>\\s*([\\d\\.,]+)\\s*TL");
    std::wsmatch m;
    if (std::regex_search(html, m, rePrice) && m.size() > 1) {
        p.priceText = ToCString(m[1].str()) + _T(" TL");
    }

    // 2. Özellik Listesini Tara (ul class="classifiedInfoList")
    // Yapı: <li><strong>Etiket</strong>&nbsp;<span>Değer</span></li>
    std::wregex reItem(L"<li>\\s*<strong>([^<]+)</strong>\\s*&nbsp;\\s*<span[^>]*>\\s*([^<]+)\\s*</span>");

    auto begin = std::wsregex_iterator(html.begin(), html.end(), reItem);
    auto end = std::wsregex_iterator();

    for (auto it = begin; it != end; ++it) {
        std::wstring label = (*it)[1].str();
        std::wstring value = (*it)[2].str();

        // Türkçe karakter temizliği ve boşluk silme
        label = HtmlEntityDecode(label);
        value = HtmlEntityDecode(value);

        // Değerleri Eşle
        if (label.find(L"İlan Tarihi") != std::wstring::npos) p.listingDate = ToCString(value);
        else if (label.find(L"Emlak Tipi") != std::wstring::npos) p.emlakTipi = ToCString(value);
        else if (label.find(L"m² (Brüt)") != std::wstring::npos) p.m2Brut = ToCString(value);
        else if (label.find(L"m² (Net)") != std::wstring::npos) p.m2Net = ToCString(value);
        else if (label.find(L"Oda Sayısı") != std::wstring::npos) p.roomCount = ToCString(value);
        else if (label.find(L"Bina Yaşı") != std::wstring::npos) p.buildingAge = ToCString(value);
        else if (label.find(L"Bulunduğu Kat") != std::wstring::npos) p.floor = ToCString(value);
        else if (label.find(L"Kat Sayısı") != std::wstring::npos) p.totalFloor = ToCString(value);
        else if (label.find(L"Isıtma") != std::wstring::npos) p.heating = ToCString(value);
        else if (label.find(L"Banyo Sayısı") != std::wstring::npos) p.bathroomCount = ToCString(value);
        else if (label.find(L"Balkon") != std::wstring::npos) p.balcony = ToCString(value);
        else if (label.find(L"Eşyalı") != std::wstring::npos) p.furnished = ToCString(value);
        else if (label.find(L"Site İçerisinde") != std::wstring::npos) p.inSite = ToCString(value);
        else if (label.find(L"Aidat") != std::wstring::npos) p.dues = ToCString(value);
        else if (label.find(L"Krediye Uygun") != std::wstring::npos) p.creditEligible = ToCString(value);
        else if (label.find(L"Tapu Durumu") != std::wstring::npos) p.deedStatus = ToCString(value);
        else if (label.find(L"Kimden") != std::wstring::npos) p.sellerType = ToCString(value);
        else if (label.find(L"İmar Durumu") != std::wstring::npos) p.LAND_ImarDurumu = ToCString(value);
        else if (label == L"m²" || label.find(L"m²") != std::wstring::npos) p.LAND_M2 = ToCString(value);
        else if (label.find(L"m² Fiyatı") != std::wstring::npos) p.LAND_M2Fiyati = ToCString(value);
        else if (label.find(L"Ada No") != std::wstring::npos) p.LAND_AdaNo = ToCString(value);
        else if (label.find(L"Parsel No") != std::wstring::npos) p.LAND_ParselNo = ToCString(value);
        else if (label.find(L"Pafta No") != std::wstring::npos) p.LAND_PaftaNo = ToCString(value);
        else if (label.find(L"Kaks") != std::wstring::npos) p.LAND_KaksEmsal = ToCString(value);
        else if (label.find(L"Gabari") != std::wstring::npos) p.LAND_Gabari = ToCString(value);
        else if (label.find(L"Krediye Uygunluk") != std::wstring::npos) p.LAND_KrediUygunluk = ToCString(value);
        // Villa-specific
        else if (label.find(L"Açık Alan") != std::wstring::npos) p.VILLA_AcikAlanM2 = ToCString(value);
        // Car/Vehicle fields
        else if (label.find(L"Marka") != std::wstring::npos) p.CAR_Brand = ToCString(value);
        else if (label.find(L"Seri") != std::wstring::npos) p.CAR_Series = ToCString(value);
        else if (label.find(L"Model") != std::wstring::npos) p.CAR_Model = ToCString(value);
        else if (label.find(L"Yıl") != std::wstring::npos) p.CAR_Year = ToCString(value);
        else if (label.find(L"KM") != std::wstring::npos || label.find(L"Kilometre") != std::wstring::npos) p.CAR_Km = ToCString(value);
        else if (label.find(L"Yakıt") != std::wstring::npos) p.CAR_FuelType = ToCString(value);
        else if (label.find(L"Vites") != std::wstring::npos) p.CAR_Transmission = ToCString(value);
        else if (label.find(L"Motor Hacmi") != std::wstring::npos) p.CAR_EngineVolume = ToCString(value);
        else if (label.find(L"Motor Gücü") != std::wstring::npos) p.CAR_EnginePower = ToCString(value);
        else if (label.find(L"Çekiş") != std::wstring::npos) p.CAR_Drive = ToCString(value);
        else if (label.find(L"Kasa Tipi") != std::wstring::npos) p.CAR_BodyType = ToCString(value);
        else if (label.find(L"Renk") != std::wstring::npos) p.CAR_Color = ToCString(value);
        else if (label.find(L"Hasar") != std::wstring::npos) p.CAR_DamageRecord = ToCString(value);
        else if (label.find(L"Garanti") != std::wstring::npos) p.CAR_Warranty = ToCString(value);
        else if (label.find(L"Plaka") != std::wstring::npos) p.CAR_Plate = ToCString(value);
        else if (label.find(L"Araç Durumu") != std::wstring::npos || label.find(L"Vasıta Durumu") != std::wstring::npos) p.CAR_VehicleCondition = ToCString(value);


    }

    // 3. Konum Bilgisi (h2 içindeki linkler)
    // <h2> ... <a ...>İl</a> ... <a ...>İlçe</a> ... <a ...>Mahalle</a> ... </h2>
    std::wregex reLoc(L"<h2>([\\s\\S]*?)</h2>");
    if (std::regex_search(html, m, reLoc) && m.size() > 1) {
        std::wstring h2Content = m[1].str();
        std::wregex reLink(L"<a[^>]*>\\s*([^<]+)\\s*</a>");
        auto lBegin = std::wsregex_iterator(h2Content.begin(), h2Content.end(), reLink);
        auto lEnd = std::wsregex_iterator();

        std::vector<CString> locs;
        for (auto it = lBegin; it != lEnd; ++it) locs.push_back(ToCString(HtmlEntityDecode((*it)[1].str())));

        if (locs.size() >= 1) p.city = locs[0];
        if (locs.size() >= 2) p.district = locs[1];
        if (locs.size() >= 3) p.neighborhood = locs[2];
    }

    // En azından fiyat veya oda sayısı bulduysak başarılı sayalım
    if (!p.priceText.IsEmpty() || !p.roomCount.IsEmpty()) return true;

    return false;
}

// --- MEVCUT JSON FONKSİYONLARI (Aynen korundu) ---
bool SahibindenImporter::ExtractTrackingJson(const std::wstring& html, std::wstring& outJson) {
    std::wregex re(L"id=\"gaPageViewTrackingJson\"[^>]*data-json=\"([^\"]*)\"");
    std::wsmatch m;
    if (std::regex_search(html, m, re) && m.size() > 1) { outJson = HtmlEntityDecode(m[1].str()); return true; }

    std::wregex re2(L"id=\"gaPageViewTrackingJson\"[^>]*data-json='([^']*)'");
    if (std::regex_search(html, m, re2) && m.size() > 1) { outJson = HtmlEntityDecode(m[1].str()); return true; }
    return false;
}

bool SahibindenImporter::ParseTrackingJson(const CString& url, const std::wstring& jsonText, SahibindenListingPayload& p, LogFn log) {
    p = SahibindenListingPayload{}; p.listingUrl = url;
    try {
        // Wide -> UTF-8 (null terminator eklemeden)
        std::string utf8;
        int len = WideCharToMultiByte(CP_UTF8, 0, jsonText.data(), (int)jsonText.size(), NULL, 0, NULL, NULL);
        if (len <= 0) return false;
        utf8.resize(len);
        WideCharToMultiByte(CP_UTF8, 0, jsonText.data(), (int)jsonText.size(), &utf8[0], len, NULL, NULL);

        json j = json::parse(utf8);

        auto customVars = j.contains("customVars") ? BuildNameValueMapW(j["customVars"]) : std::unordered_map<std::wstring, std::wstring>{};
        auto dmpData    = j.contains("dmpData")    ? BuildNameValueMapW(j["dmpData"])    : std::unordered_map<std::wstring, std::wstring>{};

        p.listingNo      = PickFirstW(customVars, {L"İlan No"}, dmpData, {L"classifiedId", L"id"});
        p.listingDate    = PickFirstW(customVars, {L"İlan Tarihi"}, dmpData, {L"ilan_tarihi"});
        p.emlakTipi      = PickFirstW(customVars, {L"Emlak Tipi"}, dmpData, {L"cat0"});

        p.priceText      = PickFirstW(customVars, {L"ilan_fiyat", L"Fiyat"}, dmpData, {});
        if (p.priceText.IsEmpty()) {
            CString n = PickFirstW(customVars, {}, dmpData, {L"fiyat"});
            if (!n.IsEmpty()) p.priceText = n + _T(" TL");
        }

        p.m2Brut         = PickFirstW(customVars, {L"m² (Brüt)", L"m2 (Brüt)"}, dmpData, {L"m2_brut"});
        p.m2Net          = PickFirstW(customVars, {L"m² (Net)", L"m2 (Net)"}, dmpData, {L"m2_net"});
        p.roomCount      = PickFirstW(customVars, {L"Oda Sayısı"}, dmpData, {L"oda_sayisi"});
        p.buildingAge    = PickFirstW(customVars, {L"Bina Yaşı"}, dmpData, {L"bina_yasi"});
        p.floor          = PickFirstW(customVars, {L"Bulunduğu Kat"}, dmpData, {L"bulundugu_kat"});
        p.totalFloor     = PickFirstW(customVars, {L"Kat Sayısı"}, dmpData, {L"kat_sayisi"});
        p.heating        = PickFirstW(customVars, {L"Isıtma"}, dmpData, {L"isitma"});
        p.bathroomCount  = PickFirstW(customVars, {L"Banyo Sayısı"}, dmpData, {L"banyo_sayisi"});
        p.kitchen        = PickFirstW(customVars, {L"Mutfak"}, dmpData, {L"mutfak"});
        p.balcony        = PickFirstW(customVars, {L"Balkon"}, dmpData, {L"balkon"});
        p.elevator       = PickFirstW(customVars, {L"Asansör"}, dmpData, {L"asansor"});
        p.parking        = PickFirstW(customVars, {L"Otopark"}, dmpData, {L"otopark"});
        p.furnished      = PickFirstW(customVars, {L"Eşyalı"}, dmpData, {L"esyali"});
        p.usageStatus    = PickFirstW(customVars, {L"Kullanım Durumu"}, dmpData, {L"kullanim_durumu"});
        p.inSite         = PickFirstW(customVars, {L"Site İçerisinde"}, dmpData, {L"site_icerisinde"});
        p.siteName       = PickFirstW(customVars, {L"Site Adı"}, dmpData, {L"site_adi"});
        p.dues           = PickFirstW(customVars, {L"Aidat (TL)", L"Aidat"}, dmpData, {L"aidat_tl"});
        p.creditEligible = PickFirstW(customVars, {L"Krediye Uygun"}, dmpData, {L"krediye_uygun"});
        p.deedStatus     = PickFirstW(customVars, {L"Tapu Durumu"}, dmpData, {L"tapu_durumu"});
        p.sellerType     = PickFirstW(customVars, {L"Kimden"}, dmpData, {L"kimden"});
        p.swap           = PickFirstW(customVars, {L"Takas"}, dmpData, {});

        // ---- LAND (Arsa) teknik alanlar ----
        p.LAND_Cat2 = PickFirstW(customVars, { L"cat2" }, dmpData, { L"cat2" });

        p.LAND_ImarDurumu = PickFirstW(customVars, { L"İmar Durumu" }, dmpData, { L"imar_durumu" });
        p.LAND_M2 = PickFirstW(customVars, { L"m²", L"Metrekare" }, dmpData, { L"m2" });
        p.LAND_M2Fiyati = PickFirstW(customVars, { L"m² Fiyatı" }, dmpData, { L"m2_fiyati" });

        p.LAND_AdaNo = PickFirstW(customVars, { L"Ada No", L"Ada" }, dmpData, { L"ada_no" });
        p.LAND_ParselNo = PickFirstW(customVars, { L"Parsel No", L"Parsel" }, dmpData, { L"parsel_no" });
        p.LAND_PaftaNo = PickFirstW(customVars, { L"Pafta No", L"Pafta" }, dmpData, { L"pafta_no" });

        p.LAND_KaksEmsal = PickFirstW(customVars, { L"Kaks (Emsal)", L"KAKS", L"Emsal" }, dmpData, { L"kaks_emsal" });
        p.LAND_Gabari = PickFirstW(customVars, { L"Gabari" }, dmpData, { L"gabari" });

        p.LAND_KrediUygunluk = PickFirstW(customVars, { L"Krediye Uygunluk", L"Krediye Uygun" }, dmpData, { L"krediye_uygunluk" });
        p.LAND_TapuDurumu = PickFirstW(customVars, { L"Tapu Durumu" }, dmpData, { L"tapu_durumu" });
        p.LAND_Kimden = PickFirstW(customVars, { L"Kimden" }, dmpData, { L"kimden" });
        p.LAND_Takas = PickFirstW(customVars, { L"Takas" }, dmpData, {});

        // ---- VILLA-specific fields ----
        p.VILLA_AcikAlanM2 = PickFirstW(
            customVars, { L"Açık Alan m²", L"Açık Alan", L"Acik Alan m2" },
            dmpData, { L"acik_alan_m2", L"outdoor_area" }
        );

        // ---- CAR/VEHICLE (Araç) teknik alanlar ----
        p.CAR_Brand = PickFirstW(customVars, { L"Marka" }, dmpData, { L"marka", L"brand" });
        p.CAR_Series = PickFirstW(customVars, { L"Seri" }, dmpData, { L"seri", L"series" });
        p.CAR_Model = PickFirstW(customVars, { L"Model" }, dmpData, { L"model" });
        p.CAR_Year = PickFirstW(customVars, { L"Yıl", L"Model Yılı" }, dmpData, { L"yil", L"year", L"model_yili" });
        p.CAR_Km = PickFirstW(customVars, { L"KM", L"Kilometre" }, dmpData, { L"km", L"kilometre" });
        p.CAR_FuelType = PickFirstW(customVars, { L"Yakıt Tipi", L"Yakıt" }, dmpData, { L"yakit_tipi", L"yakit", L"fuel_type" });
        p.CAR_Transmission = PickFirstW(customVars, { L"Vites Tipi", L"Vites" }, dmpData, { L"vites_tipi", L"vites", L"transmission" });
        p.CAR_EngineVolume = PickFirstW(customVars, { L"Motor Hacmi" }, dmpData, { L"motor_hacmi", L"engine_volume" });
        p.CAR_EnginePower = PickFirstW(customVars, { L"Motor Gücü" }, dmpData, { L"motor_gucu", L"engine_power" });
        p.CAR_Drive = PickFirstW(customVars, { L"Çekiş" }, dmpData, { L"cekis", L"drive" });
        p.CAR_BodyType = PickFirstW(customVars, { L"Kasa Tipi" }, dmpData, { L"kasa_tipi", L"body_type" });
        p.CAR_Color = PickFirstW(customVars, { L"Renk" }, dmpData, { L"renk", L"color" });
        p.CAR_DamageRecord = PickFirstW(customVars, { L"Hasar Kaydı", L"Hasarlı" }, dmpData, { L"hasar_kaydi", L"damage_record" });
        p.CAR_Warranty = PickFirstW(customVars, { L"Garanti", L"Garanti Durumu" }, dmpData, { L"garanti", L"warranty" });
        p.CAR_Plate = PickFirstW(customVars, { L"Plaka" }, dmpData, { L"plaka", L"plate" });
        p.CAR_VehicleCondition = PickFirstW(customVars, { L"Araç Durumu", L"Vasıta Durumu" }, dmpData, { L"arac_durumu", L"vehicle_condition", L"vasita_durumu" });






        p.city           = PickFirstW(customVars, {L"loc2"}, dmpData, {L"loc2"});
        p.district       = PickFirstW(customVars, {L"loc3"}, dmpData, {L"loc3"});
        p.neighborhood   = PickFirstW(customVars, {L"loc5"}, dmpData, {L"loc5"});
        if (p.listingNo.IsEmpty()) p.listingNo = ExtractIdFromUrl(url);
        return true;
    }
    catch (...) { return false; }
}

// --- DİĞER YARDIMCILAR ---
CString SahibindenImporter::ExtractIdFromUrl(const CString& url) {
    int nDetay = url.Find(_T("/detay")); if (nDetay == -1) return _T("");
    CString pre = url.Left(nDetay); int nHyphen = pre.ReverseFind(_T('-'));
    if (nHyphen == -1) return _T(""); return pre.Mid(nHyphen + 1);
}

std::wstring SahibindenImporter::HtmlEntityDecode(const std::wstring& s) {
    std::wstring out = s;
    auto rep = [&](const wchar_t* a, const wchar_t* b) {
        size_t pos = 0; while ((pos = out.find(a, pos)) != std::wstring::npos) { out.replace(pos, wcslen(a), b); pos += wcslen(b); }
        };
    rep(L"&quot;", L"\""); rep(L"&amp;", L"&"); rep(L"&#39;", L"'"); rep(L"&lt;", L"<"); rep(L"&gt;", L">"); rep(L"&nbsp;", L" ");
    rep(L"&#214;", L"Ö"); rep(L"&#246;", L"ö"); rep(L"&#220;", L"Ü"); rep(L"&#252;", L"ü");
    rep(L"&#199;", L"Ç"); rep(L"&#231;", L"ç"); rep(L"&#304;", L"İ"); rep(L"&#305;", L"ı");
    rep(L"&#286;", L"Ğ"); rep(L"&#287;", L"ğ"); rep(L"&#350;", L"Ş"); rep(L"&#351;", L"ş");
    return out;
}

bool SahibindenImporter::ExtractContactFromHtml(const std::wstring& html, SahibindenListingPayload& ioPayload) {
    std::wregex rePhone(L"0\\s*\\(\\s*\\d{3}\\s*\\)\\s*\\d{3}\\s*\\d{2}\\s*\\d{2}");
    std::wsmatch m;
    if (std::regex_search(html, m, rePhone)) ioPayload.contactPhone = ToCString(m[0].str());

    std::vector<std::wstring> namePatterns = {
        L"class=\"username-info-area\"[^>]*>\\s*<h5>\\s*([^<]+)\\s*</h5>",
        L"class=\"storeProfileUrl\"[^>]*>\\s*([^<]+)\\s*<",
        L"class=\"store-profile-title\"[^>]*>\\s*([^<]+)\\s*<"
    };
    for (const auto& pat : namePatterns) {
        std::wregex re(pat);
        if (std::regex_search(html, m, re) && m.size() > 1) {
            ioPayload.contactName = ToCString(HtmlEntityDecode(m[1].str()));
            break;
        }
    }
    return true;
}

bool SahibindenImporter::ExtractFeaturesFromHtml(const std::wstring& html, SahibindenListingPayload& ioPayload) {
    std::wregex reLi(L"<li[^>]*class=\"selected\"[^>]*>\\s*([^<]+)\\s*</li>");
    auto begin = std::wsregex_iterator(html.begin(), html.end(), reLi);
    auto end = std::wsregex_iterator();
    CString features;
    for (auto it = begin; it != end; ++it) {
        std::wstring item = (*it)[1].str();
        if (item.length() > 2) {
            if (!features.IsEmpty()) features += _T(", ");
            features += ToCString(HtmlEntityDecode(item));
        }
    }
    ioPayload.featuresText = features;
    return true;
}

CString SahibindenImporter::NormalizePhone(const CString& phone) {
    CString out; for (int i = 0; i < phone.GetLength(); ++i) if (phone[i] >= '0' && phone[i] <= '9') out += phone[i]; return out;
}

bool SahibindenImporter::SaveToDatabase(const SahibindenListingPayload& p, LogFn log) {
    DatabaseManager& db = DatabaseManager::GetInstance();
    if (!db.IsConnected()) db.EnsureConnection();

    // ------------------------------------------------------------
    // Idempotent import: ayni ilan tekrar tekrar eklenmesin
    // ------------------------------------------------------------
    const CString kWebsite = _T("sahibinden");
    if (!p.listingNo.IsEmpty()) {
        if (db.ExistsPropertyByListing(kWebsite, p.listingNo)) {
            if (log) { CString m; m.Format(_T("[SKIP] Ayni ilan zaten var (ListingNo=%s)"), p.listingNo.GetString()); log(m); }
            return true;
        }
    }
    CString custCari;
    CString normPhone = NormalizePhone(p.contactPhone);
    bool bFound = false;

    if (!normPhone.IsEmpty())
    {
        // HIZLI ve guvenli: önce LIKE ile olasi adaylari bul, sonra normalize edip dogrula
        CString foundCari;
        if (db.FindCustomerByPhoneLike(normPhone, foundCari) && !foundCari.IsEmpty())
        {
            custCari = foundCari;
            bFound = true;
        }
    }

    if (!bFound)
    {
        Customer_cstr c{};
        c.Cari_Kod = db.GenerateNextCariCode();
        c.AdSoyad = p.contactName.IsEmpty() ? CString("Sahibinden Satıcı") : p.contactName;
        c.Telefon = p.contactPhone;
        c.Sehir = p.city; c.Ilce = p.district; c.Adres = p.neighborhood;
        c.sync_id = db.GenerateSyncId(); c.Updated_At = db.GetCurrentIsoUtc(); c.Deleted = _T("0"); c.Status = _T("1");
        if (db.InsertGlobal(c)) custCari = c.Cari_Kod; else return false;
    }

    // ------------------------------------------------------------
    // Emlak tipine gore dogru tabloya yaz
    // ------------------------------------------------------------
    CString type = p.emlakTipi;
    type.MakeLower();

    CString urlLower = p.listingUrl;
    urlLower.MakeLower();

    // Adres birlestirici (tabloya gore alan adi degisebilir)
    CString addr;
    if (!p.city.IsEmpty()) addr += p.city;
    if (!p.district.IsEmpty()) { if (!addr.IsEmpty()) addr += _T(", "); addr += p.district; }
    if (!p.neighborhood.IsEmpty()) { if (!addr.IsEmpty()) addr += _T(", "); addr += p.neighborhood; }

    // Fiyat normalize (tum tipler icin ortak)
    CString priceRaw = p.priceText;
    CString priceDigits;
    for (int i = 0; i < priceRaw.GetLength(); ++i) {
        const TCHAR ch = priceRaw[i];
        if (ch >= _T('0') && ch <= _T('9')) priceDigits += ch;
    }
    CString priceNorm = priceDigits.IsEmpty() ? priceRaw : priceDigits;
    CString currency = _T("TL");
    if (priceRaw.Find(_T("USD")) != -1) currency = _T("USD");
    else if (priceRaw.Find(_T("EUR")) != -1) currency = _T("EUR");

    auto isType = [&](const TCHAR* token) -> bool {
        CString t(token); t.MakeLower();
        return (type.Find(t) != -1) || (urlLower.Find(t) != -1);
    };

    auto isLand = [&]() -> bool {
        CString cat2 = p.LAND_Cat2; cat2.MakeLower();
        return isType(_T("arsa")) || (cat2 == _T("arsa"));
        };

    auto isCar = [&]() -> bool {
        // Check if it's a vehicle/car listing
        // Common patterns: /vasita/, /otomobil/, /araba/, or if we have brand/model/km data
        return (urlLower.Find(_T("/vasita/")) != -1) ||
               (urlLower.Find(_T("/otomobil/")) != -1) ||
               (urlLower.Find(_T("/araba/")) != -1) ||
               (!p.CAR_Brand.IsEmpty() && !p.CAR_Model.IsEmpty()) ||
               (!p.CAR_Km.IsEmpty());
        };



    // --- ARSA ---
    if (isLand())
    {
        Land_cstr r{};
        r.Cari_Kod = custCari;
        r.Land_Code = db.GenerateNextLandCode();

        r.Adres = addr;

        // Metrekare: Arsa için JSON'taki "m²" daha doğru -> yoksa brut/net fallback
        CString m2 = p.LAND_M2;
        if (m2.IsEmpty()) m2 = !p.m2Brut.IsEmpty() ? p.m2Brut : p.m2Net;
        r.Metrekare = m2;

        // İmar / Tapu / Kredi / Kimden / Takas
        r.ImarDurumu = p.LAND_ImarDurumu;          // doğru
        r.DeedStatus = !p.LAND_TapuDurumu.IsEmpty() ? p.LAND_TapuDurumu : p.deedStatus;
        r.CreditEligible = !p.LAND_KrediUygunluk.IsEmpty() ? p.LAND_KrediUygunluk : p.creditEligible;
        r.SellerType = !p.LAND_Kimden.IsEmpty() ? p.LAND_Kimden : p.sellerType;
        r.Swap = !p.LAND_Takas.IsEmpty() ? p.LAND_Takas : p.swap;

        // Ada/Parsel/Pafta/Kaks/Gabari/m2 fiyatı
        r.Ada = p.LAND_AdaNo;
        r.Parsel = p.LAND_ParselNo;
        r.Pafta = p.LAND_PaftaNo;
        r.Kaks = p.LAND_KaksEmsal;
        r.Gabari = p.LAND_Gabari;
        r.PricePerM2 = p.LAND_M2Fiyati;

        // Fiyat
        r.Fiyat = priceNorm;
        r.Currency = currency;

        // Attr (debug + manuel giriş için lazım)
        r.SetAttr("website", _T("sahibinden"));
        r.SetAttr("listing_no", p.listingNo);
        r.SetAttr("listing_date", p.listingDate);
        r.SetAttr("property_type", p.emlakTipi);
        r.SetAttr("listing_url", p.listingUrl);
        r.SetAttr("listing_title", p.LAND_ListingTitle); // <- başlık burada
        r.SetAttr("features", p.featuresText);
        r.SetAttr("price_raw", priceRaw);

        r.sync_id = db.GenerateSyncId();
        r.Updated_At = db.GetCurrentIsoUtc();
        r.Deleted = _T("0");
        r.Status = _T("1");
        return db.InsertGlobal(r);
    }

    // --- CAR/VEHICLE (Araç) ---
    if (isCar())
    {
        Car_cstr r{};
        r.Cari_Kod = custCari;
        r.Car_Code = db.GenerateNextCarCode();
        
        // Listing information
        r.ListingNo = p.listingNo;
        r.ListingDate = p.listingDate;
        r.WebsiteName = _T("sahibinden");
        r.ListingURL = p.listingUrl;
        
        // Title from CAR_Title or listing title
        if (!p.CAR_Title.IsEmpty()) {
            r.Title = p.CAR_Title;
        } else if (!p.LAND_ListingTitle.IsEmpty()) {
            r.Title = p.LAND_ListingTitle;
        }
        
        // Vehicle specifications
        r.Brand = p.CAR_Brand;             // Marka
        r.Series = p.CAR_Series;           // Seri
        r.Model = p.CAR_Model;             // Model
        r.Year = p.CAR_Year;               // Yıl
        r.Km = p.CAR_Km;                   // Kilometre
        r.Price = priceNorm;
        r.Currency = currency;
        
        // Engine and transmission
        r.FuelType = p.CAR_FuelType;       // Yakıt Tipi
        r.Transmission = p.CAR_Transmission; // Vites Tipi
        r.EngineVolume = p.CAR_EngineVolume; // Motor Hacmi
        r.EnginePower = p.CAR_EnginePower;   // Motor Gücü
        r.Drive = p.CAR_Drive;             // Çekiş
        r.BodyType = p.CAR_BodyType;       // Kasa Tipi
        r.Color = p.CAR_Color;             // Renk
        
        // Condition and warranty
        r.DamageRecord = p.CAR_DamageRecord;   // Hasar Kaydı
        r.Warranty = p.CAR_Warranty;           // Garanti
        r.Plate = p.CAR_Plate;                 // Plaka
        r.VehicleCondition = p.CAR_VehicleCondition; // Araç Durumu
        
        // Location
        r.City = p.city;
        r.District = p.district;
        r.Neighborhood = p.neighborhood;
        
        // Seller information
        r.SellerName = p.contactName;
        r.SellerPhone = p.contactPhone;
        
        // Additional attributes
        r.SetAttr("website", _T("sahibinden"));
        r.SetAttr("listing_url", p.listingUrl);
        r.SetAttr("features", p.featuresText);
        r.SetAttr("price_raw", priceRaw);
        
        r.sync_id = db.GenerateSyncId();
        r.Updated_At = db.GetCurrentIsoUtc();
        r.Deleted = _T("0");
        r.Status = _T("1");
        
        return db.InsertGlobal(r);
    }

    // --- TARLA ---
    if (isType(_T("tarla")))
    {
        Field_cstr r{};
        r.Cari_Kod = custCari;
        r.Field_Code = db.GenerateNextFieldCode();
        r.Adres = addr;
        r.Metrekare = !p.m2Brut.IsEmpty() ? p.m2Brut : p.m2Net;
        r.ImarDurumu = p.deedStatus;
        r.Fiyat = priceNorm;
        r.Currency = currency;
        r.ListingNo = p.listingNo;
        r.ListingDate = p.listingDate;
        r.PropertyType = p.emlakTipi;
        r.CreditEligible = p.creditEligible;
        r.SellerType = p.sellerType;
        r.Swap = p.swap;
        r.SetAttr("website", _T("sahibinden"));
        r.SetAttr("listing_url", p.listingUrl);
        r.SetAttr("features", p.featuresText);
        r.SetAttr("price_raw", priceRaw);
        r.sync_id = db.GenerateSyncId();
        r.Updated_At = db.GetCurrentIsoUtc();
        r.Deleted = _T("0");
        r.Status = _T("1");
        return db.InsertGlobal(r);
    }

    // --- BAG / BAHCE ---
    if (isType(_T("bağ")) || isType(_T("bag")) || isType(_T("bahçe")) || isType(_T("bahce")))
    {
        Vineyard_cstr r{};
        r.Cari_Kod = custCari;
        r.Vineyard_Code = db.GenerateNextVineyardCode();
        r.Adres = addr;
        r.Metrekare = !p.m2Brut.IsEmpty() ? p.m2Brut : p.m2Net;
        r.Fiyat = priceNorm;
        r.Currency = currency;
        r.SetAttr("listing_no", p.listingNo);
        r.SetAttr("listing_date", p.listingDate);
        r.SetAttr("property_type", p.emlakTipi);
        r.SetAttr("website", _T("sahibinden"));
        r.SetAttr("listing_url", p.listingUrl);
        r.SetAttr("features", p.featuresText);
        r.SetAttr("price_raw", priceRaw);
        r.sync_id = db.GenerateSyncId();
        r.Updated_At = db.GetCurrentIsoUtc();
        r.Deleted = _T("0");
        r.Status = _T("1");
        return db.InsertGlobal(r);
    }

    // --- VILLA ---
    if (isType(_T("villa")))
    {
        Villa_cstr r{};
        r.Cari_Kod = custCari;
        r.Villa_Code = db.GenerateNextVillaCode();
        
        // Basic information
        r.Adres = addr;
        r.OdaSayisi = p.roomCount;
        r.NetMetrekare = p.m2Net;
        r.BrutMetrekare = p.m2Brut;
        r.Fiyat = priceNorm;
        r.Currency = currency;
        
        // Villa-specific fields (from Villa_cstr struct in dataIsMe.h)
        r.ListingNo = p.listingNo;
        r.ListingDate = p.listingDate;
        r.PropertyType = p.emlakTipi;
        r.ListingURL = p.listingUrl;
        r.WebsiteName = _T("sahibinden");
        
        // Seller info
        r.SellerName = p.contactName;
        r.SellerPhone = p.contactPhone;
        
        // Additional villa fields
        r.AcikAlanM2 = p.VILLA_AcikAlanM2;     // CRITICAL: Açık Alan m² (outdoor area)
        r.BuildingAge = p.buildingAge;
        r.HeatingType = p.heating;
        r.BathroomCount = p.bathroomCount;
        r.KitchenType = p.kitchen;
        r.Parking = p.parking;
        r.Furnished = p.furnished;
        r.UsageStatus = p.usageStatus;
        r.InSite = p.inSite;
        r.SiteName = p.siteName;
        r.Dues = p.dues;
        r.CreditEligible = p.creditEligible;
        r.DeedStatus = p.deedStatus;
        r.SellerType = p.sellerType;
        r.Swap = p.swap;
        r.Description = p.featuresText;
        
        r.SetAttr("listing_no", p.listingNo);
        r.SetAttr("listing_date", p.listingDate);
        r.SetAttr("property_type", p.emlakTipi);
        r.SetAttr("website", _T("sahibinden"));
        r.SetAttr("listing_url", p.listingUrl);
        r.SetAttr("features", p.featuresText);
        r.SetAttr("price_raw", priceRaw);
        r.SetAttr("balcony", p.balcony);
        r.SetAttr("elevator", p.elevator);
        r.sync_id = db.GenerateSyncId();
        r.Updated_At = db.GetCurrentIsoUtc();
        r.Deleted = _T("0");
        r.Status = _T("1");
        return db.InsertGlobal(r);
    }

    // --- TICARI ---
    if (isType(_T("ticari")) || isType(_T("dükkan")) || isType(_T("dukkan")) || isType(_T("ofis")))
    {
        Commercial_cstr r{};
        r.Cari_Kod = custCari;
        r.Commercial_Code = db.GenerateNextCommercialCode();
        r.Adres = addr;
        r.Alan = !p.m2Brut.IsEmpty() ? p.m2Brut : p.m2Net;
        r.Kat = p.floor;
        r.Fiyat = priceNorm;
        r.Currency = currency;
        r.SetAttr("listing_no", p.listingNo);
        r.SetAttr("listing_date", p.listingDate);
        r.SetAttr("property_type", p.emlakTipi);
        r.SetAttr("website", _T("sahibinden"));
        r.SetAttr("listing_url", p.listingUrl);
        r.SetAttr("features", p.featuresText);
        r.SetAttr("price_raw", priceRaw);
        r.sync_id = db.GenerateSyncId();
        r.Updated_At = db.GetCurrentIsoUtc();
        r.Deleted = _T("0");
        r.Status = _T("1");
        return db.InsertGlobal(r);
    }

    // --- DEFAULT: HOME (Daire/Mustakil vb.) ---
    Home_cstr h{};
    h.Cari_Kod = custCari;
    h.Home_Code = db.GenerateNextHomeCode();
    h.ListingNo = p.listingNo;
    h.ListingDate = p.listingDate;
    h.PropertyType = p.emlakTipi;
    h.Address = addr;
    h.City = p.city;
    h.District = p.district;
    h.Neighborhood = p.neighborhood;
    h.RoomCount = p.roomCount;
    h.NetArea = p.m2Net;
    h.GrossArea = p.m2Brut;
    h.BuildingAge = p.buildingAge;
    h.Floor = p.floor;
    h.TotalFloor = p.totalFloor;
    h.HeatingType = p.heating;
    h.BathroomCount = p.bathroomCount;
    h.Balcony = p.balcony;
    h.KitchenType = p.kitchen;
    h.Elevator = p.elevator;
    h.Parking = p.parking;
    h.SiteName = p.siteName;
    h.Swap = p.swap;
    h.Furnished = p.furnished;
    h.UsageStatus = p.usageStatus;
    h.InSite = p.inSite;
    h.Dues = p.dues;
    h.CreditEligible = p.creditEligible;
    h.DeedStatus = p.deedStatus;
    h.SellerType = p.sellerType;
    h.Price = priceNorm;
    h.Currency = currency;
    h.WebsiteName = _T("sahibinden");
    h.ListingURL = p.listingUrl;
    h.NoteGeneral = p.featuresText;
    h.SetAttr("price_raw", priceRaw);
    h.sync_id = db.GenerateSyncId();
    h.Updated_At = db.GetCurrentIsoUtc();
    h.Deleted = _T("0");
    h.Status = _T("1");
    return db.InsertGlobal(h);
}