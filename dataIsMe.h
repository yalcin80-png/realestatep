#ifndef DATAISME_H
#define DATAISME_H
#include <vector>
#include <io.h>     // _access() fonksiyonu için
#include <comdef.h>   // _com_error ve _bstr_t için gerekli
#include "AllGlobalHeader.h"
#include "SchemaManager.h"
#include "AdoLocalAdapter.h"
//#include <afx.h>
#include "ISyncInterface.h"
#include "SyncBridge.h"
#include "UIBinder.h"



#define ID_TREEDOCKER   23001
#define ID_FILESDOCKER  23002

#define LOG_INFO(msg)  LogToFile(L"[INFO]",  msg)
#define LOG_WARN(msg)  LogToFile(L"[WARN]",  msg)
#define LOG_ERROR(msg) LogToFile(L"[ERROR]", msg)

#define UWM_CUSTOMER_SELECTED (WM_APP + 1001)

#define IMAGE_LAND_ITEM 9812

class DatabaseManager; // İleri bildirim
enum DialogMode
{
    INEWUSER,   // Yeni Mülk Ekleme
	IVIEWER,   // Mülk Görüntüleme
    IUPDATEUSER // Mevcut Mülk Güncelleme
};


//#ifdef UNICODE
//typedef std::wstring tstring;
//#else
//typedef std::string tstring;
//#endif

// Tablo İsimleri Listesi
const CString TABLE_NAME_APPOINTMENT = _T("Appointments"); 
const CString TABLE_NAME_CUSTOMER   = _T("Customer");
const CString TABLE_NAME_HOME       = _T("HomeTbl");
const CString TABLE_NAME_LAND       = _T("Land");
const CString TABLE_NAME_FIELD      = _T("Field");
const CString TABLE_NAME_VINEYARD   = _T("Vineyard");
const CString TABLE_NAME_VILLA      = _T("Villa");
const CString TABLE_NAME_COMMERCIAL = _T("Commercial");
const CString TABLE_NAME_CAR        = _T("Car");
const CString TABLE_NAME_OFFERS     = _T("Offers");
const CString TABLE_NAME_SETTINGS   = _T("CompanySettings");


// dataIsMe.cpp - Yardımcı Fonksiyon

const CString ALL_TABLE_NAMES[] = {
    TABLE_NAME_CUSTOMER,
    TABLE_NAME_HOME,
    TABLE_NAME_LAND,
    TABLE_NAME_FIELD,
    TABLE_NAME_VINEYARD,
    TABLE_NAME_VILLA,
    TABLE_NAME_COMMERCIAL,
    TABLE_NAME_CAR,
    TABLE_NAME_OFFERS,
    TABLE_NAME_APPOINTMENT,
    TABLE_NAME_SETTINGS
};


const int NUM_TABLES = sizeof(ALL_TABLE_NAMES) / sizeof(ALL_TABLE_NAMES[0]);

struct   PropertyTypeInfo
{
    const CString TableName;
    const CString DisplayName;
    const int ImageIndex; // ImageList'teki ikon indeksi
     
};


const CString TABLE_NAMES[] = {
    _T("Customer"),
    _T("HomeTbl"),
    _T("Land"),
    _T("Field"),
    _T("Vineyard"),
    _T("Villa"),
    _T("Commercial"),
    _T("Car"),
    _T("Offers"),
    _T("CompanySettings"),
    _T("Appointments")
};
//const int NUM_TABLES = sizeof(TABLE_NAMES) / sizeof(TABLE_NAMES[0]);
// --- CUSTOMER ---
struct Customer_cstr
{
    CString Cari_Kod;
    CString AdSoyad;
    CString Telefon;
    CString Telefon2;
    CString Email;
    CString Adres;
    CString Sehir;
    CString Ilce;
    CString TCKN;
    CString VergiNo;
    CString MusteriTipi;
    CString KayitTarihi;
    CString Durum;
    CString Notlar;
    CString Calisma_Durumu;

    // YENİ EKLENENLER
    CString BabaAdi;
    CString DogumTarihi;
    CString Uyruk;
    CString KimlikSeriNo;
    CString MedeniHal;
    CString IBAN;
    CString BankaAdi;
    CString VergiDairesi;
    CString YetkiliKisi;

    // SİSTEM
    CString sync_id;
    CString Updated_At;
    CString Deleted;
    CString Status;

    // EKLENMESİ GEREKENLER:
    CString Attributes;
    CString GetAttr(const std::string& key) const;
    void SetAttr(const std::string& key, const CString& value);

};
// src/dataIsMe.h içine eklenecek:

struct Appointment_cstr
{
    CString AppID;          // DB'de sayısal olsa bile C++ tarafında CString tutuyoruz
    CString CustomerID;
    CString PropertyID;
    CString PropertyType;

    CString AppDate;
    CString Title;
    CString Description;
    CString Location;
    CString Status;

    CString GoogleEventID;
    CString IsNotified;     // bool yerine CString ("0"/"1")

    // Sync Sistemi
    CString sync_id;
    CString Updated_At;
    CString Deleted;        // bool yerine CString ("0"/"1")

    // PropertyMap uyumluluğu için gerekli metodlar
    CString GetAttr(const std::string& key) const;
    void SetAttr(const std::string& key, const CString& value);

    CString Attributes; // Attributes alanı eklenmeli (Map için gerekli olabilir)
};// --- OFFER ---
struct Offer_cstr {
    CString OfferID;
    CString PropertyID;
    CString CustomerID;
    CString OfferPrice;
    CString OfferDate;
    CString PaymentMethod;
    CString DepositAmount;
    CString ValidityDate;
    CString Status;
    CString Notes;

    CString sync_id;
    CString Updated_At;
    CString Deleted;

    CString Attributes;
    CString GetAttr(const std::string& key) const;
    void SetAttr(const std::string& key, const CString& value);

    Offer_cstr() {
        OfferID = _T(""); OfferPrice = _T("0"); DepositAmount = _T("0"); Status = _T("Beklemede");
    }
};

// --- HOME ---
struct Home_cstr
{
    CString Cari_Kod;
    CString Home_Code;
    CString ListingNo;
    CString ListingDate;

    // YETKİ SÜRELERİ (YENİ)
    CString YetkiBaslangic;
    CString YetkiBitis;

    CString PropertyType;
    CString Address;
    CString City;
    CString District;
    CString Neighborhood;
    CString Location;

    // TAPU (YENİ)
    CString Ada;
    CString Parsel;
    CString Pafta;

    CString RoomCount;
    CString NetArea;
    CString GrossArea;
    CString Floor;
    CString TotalFloor;
    CString BuildingAge;
    CString HeatingType;
    CString BathroomCount;
    CString KitchenType;
    CString Balcony;
    CString Elevator;
    CString Parking;
    CString Furnished;
    CString UsageStatus;
    CString InSite;
    CString SiteName;
    CString Dues;
    CString CreditEligible;
    CString DeedStatus;
    CString SellerType;
    CString Swap;

    CString Price;
    CString Currency;
    CString PricePerM2;

    CString WebsiteName;
    CString ListingURL;
    CString NoteGeneral;
    CString NoteInternal;
    CString PurchaseDate;
    CString PurchasePrice;
    CString SaleDate;
    CString SalePrice;
    CString EnergyClass;
    CString GeoLatitude;
    CString GeoLongitude;
    CString PhotoCount;
    CString Status;

    CString sync_id;
    CString Updated_At;
    CString Deleted;

    // ============================================================
    // SAHİBİNDEN "ÖZELLİKLER" (Resim 2-3-4) - YENİ ALANLAR
    // Not: Bunlar çoklu seçim olduğu için JSON array string (önerilen)
    // ya da CSV olarak saklanabilir.
    // ============================================================

    // Resim-2: Cephe (çoklu)  -> ["Batı","Doğu","Güney","Kuzey"]
    CString Facades;

    // Resim-2: İç Özellikler (çoklu) -> ["Akıllı Ev","Duşakabin",...]
    CString FeaturesInterior;

    // Resim-3: Dış Özellikler (çoklu) -> ["Kamera Sistemi","Isı Yalıtımı",...]
    CString FeaturesExterior;

    // Resim-3: Muhit (çoklu) -> ["Market","Okul",...]
    CString FeaturesNeighborhood;

    // Resim-3: Ulaşım (çoklu) -> ["Metro","Otobüs Durağı",...]
    CString FeaturesTransport;

    // Resim-4: Manzara (çoklu) -> ["Şehir","Doğa",...]
    CString FeaturesView;

    // Resim-4: Konut Tipi (çoğunlukla tek seçim) -> "Ara Kat"
    // Eğer çokluya çevirmek istersen JSON/CSV de tutabilirsin.
    CString HousingType;

    // Resim-4: Engelli ve Yaşlıya Uygun (çoklu) -> ["Giriş / Rampa","Engelliye Uygun Asansör",...]
    CString FeaturesAccessibility;

    // ============================================================
    // Mevcut genel alan (istersen ileride komple Features* buraya da dump edebilirsin)
    // ============================================================
    CString Attributes;

    CString GetAttr(const std::string& key) const;
    void SetAttr(const std::string& key, const CString& value);
};

// --- LAND ---
struct Land_cstr
{
    // Kimlik
    CString Cari_Kod;
    CString Land_Code;
    CString ListingTitle;   // İlan Başlığı
    CString ListingUrl;     // Web Adresi

    // İlan
    CString ListingNo;
    CString ListingDate;
    CString PropertyType;
    CString SellerType;
    CString CreditEligible;
    CString Swap;

    // Konum / Tapu
    CString Adres;
    CString Lokasyon;
    CString Ada;
    CString Parsel;
    CString Pafta;

    // Nitelik / İmar
    CString Metrekare;
    CString PricePerM2;
    CString ImarDurumu;
    CString DeedStatus;
    CString Kaks;
    CString Gabari;

    // Finans
    CString Fiyat;
    CString Currency;

    // Yetki
    CString YetkiBaslangic;
    CString YetkiBitis;

    // Notlar
    CString Teklifler;
    CString Durum;
    CString Status;

    // Ek
    CString RawKML;

    // Sistem
    CString sync_id;
    CString Updated_At;
    CString Deleted;

    // Serbest özellikler (Attributes JSON)
    CString Attributes;

    // JSON helpers (Home ile aynı altyapı)
    CString GetAttr(const std::string& key) const;
    void SetAttr(const std::string& key, const CString& value);

    // Bool helpers (checkbox'lar için)
    bool GetAttrBool(const std::string& key) const;
    void SetAttrBool(const std::string& key, bool value);
};

// --- FIELD ---
struct Field_cstr
{
    CString Cari_Kod;
    CString Field_Code;

    // TAPU
    CString Ada;
    CString Parsel;
    CString Pafta;

    CString Adres;
    CString Lokasyon;
    CString Metrekare;
    CString ToprakTuru;
    CString Sulama;
    CString Ulasim;
    CString ImarDurumu;
    CString DeedStatus;
    CString Kaks;
    CString Gabari;

    CString Fiyat;
    CString Currency; // YENİ
    CString PricePerM2;

    // YETKİ (YENİ)
    CString YetkiBaslangic;
    CString YetkiBitis;

    CString ListingNo;
    CString ListingDate;
    CString PropertyType;
    CString CreditEligible;
    CString SellerType;
    CString Swap;
    CString Teklifler;
    CString Durum;
    CString Status;

    CString RawKML;

    CString sync_id;
    CString Updated_At;
    CString Deleted;
    CString Attributes;
    CString GetAttr(const std::string& key) const;
    void SetAttr(const std::string& key, const CString& value);


};

// --- VINEYARD ---
struct Vineyard_cstr
{
    CString Cari_Kod;
    CString Vineyard_Code;
    CString Adres;
    CString Lokasyon;
    CString Metrekare;
    CString AgacSayisi;
    CString UrunTuru;
    CString Sulama;

    // TAPU (YENİ)
    CString Ada;
    CString Parsel;
    CString Pafta;

    CString Fiyat;
    CString Currency; // YENİ

    // YETKİ (YENİ)
    CString YetkiBaslangic;
    CString YetkiBitis;

    CString Teklifler;
    CString Durum;
    CString Status;

    CString sync_id;
    CString Updated_At;
    CString Deleted;

    CString Attributes;
    CString GetAttr(const std::string& key) const;
    void SetAttr(const std::string& key, const CString& value);

};

// --- VILLA ---
struct Villa_cstr
{
    CString Cari_Kod;
    CString Villa_Code;
    CString Adres;
    CString Lokasyon;



    // ============================================================
    // SAHİBİNDEN / WEB İLAN DETAYLARI (YENİ)
    // ============================================================
    CString ListingNo;        // İlan No
    CString ListingDate;      // İlan Tarihi (ISO / yyyy-mm-dd)
    CString PropertyType;     // Emlak Tipi (Satılık Villa vb.)
    CString ListingURL;       // Web Adresi
    CString WebsiteName;      // Web Sitesi (Sahibinden vb.)

    // Satıcı
    CString SellerName;
    CString SellerPhone;

    // Metin
    CString Description;      // İlan açıklaması

    // Ek alanlar (Resimlerdeki tablo)
    CString AcikAlanM2;       // Açık Alan m² (arsa)
    CString BuildingAge;      // Bina Yaşı
    CString HeatingType;      // Isıtma
    CString BathroomCount;    // Banyo Sayısı
    CString KitchenType;      // Mutfak
    CString Parking;          // Otopark
    CString Furnished;        // Eşyalı
    CString UsageStatus;      // Kullanım Durumu
    CString InSite;           // Site içerisinde
    CString SiteName;         // Site Adı
    CString Dues;             // Aidat (TL)
    CString CreditEligible;   // Krediye Uygun
    CString DeedStatus;       // Tapu Durumu
    CString SellerType;       // Kimden
    CString Swap;             // Takas

    // ============================================================
    // SAHİBİNDEN "ÖZELLİKLER" (çoklu seçim - JSON array string)
    // ============================================================
    CString Facades;
    CString FeaturesInterior;
    CString FeaturesExterior;
    CString FeaturesNeighborhood;
    CString FeaturesTransport;
    CString FeaturesView;
    CString HousingType;
    CString FeaturesAccessibility;

    // TAPU (YENİ)
    CString Ada;
    CString Parsel;
    CString Pafta;

    CString OdaSayisi;
    CString NetMetrekare;
    CString BrutMetrekare;
    CString KatSayisi;
    CString Havuz;
    CString Bahce;
    CString Garaj;

    CString Fiyat;
    CString Currency; // YENİ

    // YETKİ (YENİ)
    CString YetkiBaslangic;
    CString YetkiBitis;

    CString Teklifler;
    CString Durum;
    CString Status;

    CString sync_id;
    CString Updated_At;
    CString Deleted;

    CString Attributes;
    CString GetAttr(const std::string& key) const;
    void SetAttr(const std::string& key, const CString& value);

};

// --- COMMERCIAL ---
struct Commercial_cstr
{
    CString Cari_Kod;
    CString Commercial_Code;
    CString Adres;
    CString Lokasyon;

    // TAPU (YENİ)
    CString Ada;
    CString Parsel;
    CString Pafta;

    CString Alan;
    CString Kat;
    CString Cephe;
    CString Kullanim;

    CString Fiyat;
    CString Currency; // YENİ

    // YETKİ (YENİ)
    CString YetkiBaslangic;
    CString YetkiBitis;

    CString Teklifler;
    CString Durum;
    CString Status;

    CString sync_id;
    CString Updated_At;
    CString Deleted;

    CString Attributes;
    CString GetAttr(const std::string& key) const;
    void SetAttr(const std::string& key, const CString& value);


};




// --- CAR (ARABA) ---
// 21/01/2026 - Duran YALÇIN
// Amaç: Emlak dışı portföy (Araç) kayıtlarını aynı mimariyle yönetmek.
struct Car_cstr
{
    CString Cari_Kod;
    CString Car_Code;

    CString ListingNo;
    CString ListingDate;
    CString WebsiteName;
    CString ListingURL;

    CString Title;
    CString Brand;
    CString Series;
    CString Model;
    CString Year;
    CString Km;
    CString Price;
    CString Currency;

    CString FuelType;
    CString Transmission;
    CString EngineVolume;
    CString EnginePower;
    CString Drive;
    CString BodyType;
    CString Color;

    CString DamageRecord;
    CString Warranty;
    CString Plate;


    // Sahibinden ek alanlar
    CString City;
    CString District;
    CString Neighborhood;

    CString SellerName;
    CString SellerPhone;

    CString VehicleCondition;   // Araç Durumu
    CString FromWho;            // Kimden
    CString Swap;               // Takas

    CString Description;        // Açıklama
    CString SafetyFeatures;     // Güvenlik
    CString InteriorFeatures;   // İç Donanım
    CString ExteriorFeatures;   // Dış Donanım
    CString MultimediaFeatures; // Multimedya

    CString PaintInfo;          // Ekspertiz (JSON)

    CString Notes;
    CString Status;
    CString Durum;

    CString sync_id;
    CString Updated_At;
    CString Deleted;

    CString Attributes;
    CString GetAttr(const std::string& key) const;
    void SetAttr(const std::string& key, const CString& value);
};

// --- COMPANY SETTINGS (FİRMA BİLGİLERİ) ---
struct CompanyInfo_cstr
{
    CString ID;             // Genelde hep "1" olacak
    CString CompanyName;    // Firma Adı
    CString OwnerName;      // Yetkili Kişi
    CString TTBSNo;         // Taşınmaz Ticareti Yetki Belgesi
    CString TaxOffice;      // Vergi Dairesi
    CString TaxNo;          // Vergi No
    CString MersisNo;       // Mersis (Opsiyonel)

    CString PhoneFixed;     // Sabit Tel
    CString PhoneMobile;    // Cep Tel
    CString Email;          // E-Posta
    CString Website;        // Web Sitesi
    CString Address;        // Açık Adres

    CString LogoPath;       // Logonun diskteki yolu (res/logo.png gibi)
    CString AboutUs;        // Kısa tanıtım yazısı (Memo)

    // Sistem Alanları
    CString Deleted;
    CString sync_id;
    CString Updated_At;
    CString Attributes;

    // Helper
    CString GetAttr(const std::string& key) const;
    void SetAttr(const std::string& key, const CString& value);
};









// =============================================================
//  MERKEZİ VERİ FABRİKASI (REFLECTION MAP)
// =============================================================

// =============================================================
//  PropertyMap<T>
//  - Tek merkezden alan metadatası (DB kolon adı, UI id, label, tip, vb.)
//  - Aggregate olacak şekilde TASARLANDI (constructor yok) ki initializer-list ile
//    mevcut { &T::Field, "DB", ... } tanımlarınız bozulmasın.
// =============================================================
template<typename T>
struct PropertyMap {
    CString T::* member;      // Struct içindeki değişkene işaretçi (Sihir burada!)
    const TCHAR* dbName;      // Veritabanı Kolon Adı
    UINT resID;               // Formdaki ID (EditBox/Combo)
    const TCHAR* label;       // Liste Başlığı (Görünen Ad)

    FieldType type;           // Veri Tipi
    int width = 100;          // Liste Genişliği
    bool visible = true;      // Listede Görünür mü?
    bool isAttribute = false; // JSON içinde mi saklanacak?

    // Opsiyonel: aynı alanın farklı isimlerle bulunabilmesi için alias anahtarlar.
    // Örn: RoomCount alanı JSON'da "Oda Sayısı" geçiyorsa burada alias olarak tutulur.
    // Boş bırakılabilir.
    std::vector<CString> aliases{};
};

// Bu fonksiyonu her struct için özelleştireceğiz (Fabrika Deposu)
// GLOBAL FONKSİYON BİLDİRİMİ (Buraya koyun)
template<typename T>
const std::vector<PropertyMap<T>>& GetProperties();




































enum CustomerStatus
{
    CS_CALISMAK_ISTIYOR = 1,    // Yeşil (Green)
    CS_CALISMAK_ISTEMIYOR = 2,  // Kırmızı (Red)
    CS_DUSUNECEK = 3,           // Sarı (Yellow)
    CS_BELIRTILMEDI = 0          // Varsayılan
};








#ifndef RPC_E_ATTEMPTED_MULTIPLE
const long RPC_E_ATTEMPTED_MULTIPLE = 0x8001010A;
#endif

// adCmdNoRecords (ADO Recordset döndürmeme)
#ifndef adCmdNoRecords
const long adCmdNoRecords = 0x2000;
#endif
struct HeaderInfo { CString text; int width; };

//const CString DB_PATH = _T("D:\\RealEstate.mdb");
static const CString DB_PATH = _T("D:\\RealEstate.mdb");
static const LPCTSTR PROVIDER_CREATE = _T("Provider=Microsoft.ACE.OLEDB.12.0;Data Source=");
static const LPCTSTR PROVIDER_CONN = _T("Microsoft.ACE.OLEDB.12.0"); // ya da 16.0 sisteminize göre



#pragma once

inline bool IsPropertyCmd(UINT id)
{
    return (id >= 24000 && id <= 24499); // treelist
}

inline bool IsCustomerCmd(UINT id)
{
    return (id >= 25000 && id <= 25999); // listview
}



extern std::mutex g_propertyCodeMutex;




template<typename T>
struct PropertyTrait
{
    // C++17 'static_assert' ile bu yapının her tip için özelleştirilmesini zorunlu kılarız.
    // Yeni bir mülk tipi eklediğinizde burayı (veya GetPropertyFieldMap'i) güncellemelisiniz.
    static_assert(
        std::is_same_v<T, struct Home_cstr> ||
        std::is_same_v<T, struct Land_cstr> ||
        std::is_same_v<T, struct Field_cstr> || // Düzeltildi: Field_cstr eklendi
        std::is_same_v<T, struct Villa_cstr> ||
        std::is_same_v<T, struct Vineyard_cstr> ||
        std::is_same_v<T, struct Commercial_cstr> ||
        std::is_same_v<T, struct Car_cstr> ||
        std::is_same_v<T, struct Customer_cstr> || 
        std::is_same_v<T, struct Offer_cstr>||
        std::is_same_v < T, struct Appointment_cstr> ||
        std::is_same_v < T, struct CompanyInfo_cstr>,
        "HATA: PropertyTrait tanimi bilinmeyen mülk/cari tipi icin gereklidir.");

    // Gerekli Meta Verileri
    static CString GetTableName();
    static CString GetCodeFieldName();
    static CString GetCodeValue(const T& data);
};


// --- dataIsMe.cpp: Property Trait Uygulamaları (EKLEYİN) ---

// Customer Trait
template<>
struct PropertyTrait<Customer_cstr>
{
    static CString GetTableName() { return TABLE_NAME_CUSTOMER; } // "Customer"
    static CString GetCodeFieldName() { return L"Cari_Kod"; }
    static CString GetCodeValue(const Customer_cstr& data) { return data.Cari_Kod; }
};

// Home Trait
template<>
struct PropertyTrait<Home_cstr>
{
    static CString GetTableName() { return TABLE_NAME_HOME; } // "HomeTbl"
    static CString GetCodeFieldName() { return L"Home_Code"; }
    static CString GetCodeValue(const Home_cstr& data) { return data.Home_Code; }
};

// Land Trait
template<>
struct PropertyTrait<Land_cstr>
{
    static CString GetTableName() { return TABLE_NAME_LAND; } // "Land"
    static CString GetCodeFieldName() { return L"Land_Code"; }
    static CString GetCodeValue(const Land_cstr& data) { return data.Land_Code; }
};


template<>
struct PropertyTrait<Field_cstr>
{
    static CString GetTableName() { return TABLE_NAME_FIELD; } // "Field"
    static CString GetCodeFieldName() { return L"Field_Code"; }
    static CString GetCodeValue(const Field_cstr& data) { return data.Field_Code; }
};
// Villa Trait (Diğerleri de benzer şekilde tanımlanmalıdır.)
template<>
struct PropertyTrait<Villa_cstr>
{
    static CString GetTableName() { return TABLE_NAME_VILLA; } // "Villa"
    static CString GetCodeFieldName() { return L"Villa_Code"; }
    static CString GetCodeValue(const Villa_cstr& data) { return data.Villa_Code; }
};

// ... Vineyard ve Commercial için de PropertyTrait tanımlarını buraya eklemelisiniz.

// --- dataIsMe.h (EKSİK TRAIT'LERİN EKLENMESİ) ---

// Vineyard Trait
template<>
struct PropertyTrait<Vineyard_cstr>
{
    static CString GetTableName() { return TABLE_NAME_VINEYARD; }
    static CString GetCodeFieldName() { return L"Vineyard_Code"; }
    static CString GetCodeValue(const Vineyard_cstr& data) { return data.Vineyard_Code; }
};

// Commercial Trait
template<>
struct PropertyTrait<Commercial_cstr>
{
    static CString GetTableName() { return TABLE_NAME_COMMERCIAL; }
    static CString GetCodeFieldName() { return L"Commercial_Code"; }
    static CString GetCodeValue(const Commercial_cstr& data) { return data.Commercial_Code; }
};
// Commercial Trait
template<>
struct PropertyTrait<Appointment_cstr>
{
    static CString GetTableName() { return TABLE_NAME_APPOINTMENT; }
    static CString GetCodeFieldName() { return _T("AppID"); }

    // DÜZELTİLMİŞ SATIR:
    static CString GetCodeValue(const Appointment_cstr& data) { return data.AppID; }
};

// Car Trait
template<>
struct PropertyTrait<Car_cstr>
{
    static CString GetTableName() { return TABLE_NAME_CAR; } // "Car"
    static CString GetCodeFieldName() { return L"Car_Code"; }
    static CString GetCodeValue(const Car_cstr& data) { return data.Car_Code; }
};





// Company Trait
template<>
struct PropertyTrait<CompanyInfo_cstr>
{
    static CString GetTableName() { return TABLE_NAME_SETTINGS; }
    static CString GetCodeFieldName() { return _T("ID"); } // Tekil kayıt olduğu için ID
    static CString GetCodeValue(const CompanyInfo_cstr& data) { return data.ID; }
};


// src/dataIsMe.h içinde diğer template<> struct PropertyTrait tanımlarının yanına ekleyin:

// Offer Trait
template<>
struct PropertyTrait<Offer_cstr>
{
    static CString GetTableName() { return TABLE_NAME_OFFERS; } // "Offers"
    static CString GetCodeFieldName() { return _T("OfferID"); }
    static CString GetCodeValue(const Offer_cstr& data) { return data.OfferID; }
};



static CString SqlTypeFromFieldType(FieldType ft);


class DatabaseManager
{
public:
    // 🔹 Singleton Erişimi
    static DatabaseManager& GetInstance();

    // 🔹 Genel Şablon İşlemleri
    template <typename T> inline bool InsertGlobal(const T& data);
    template <typename T> inline bool UpdateGlobal(const T& data);
    template <typename T> inline std::vector<T> GetRecordsByCariKod(const CString& cariKod);
    template <typename T> inline T GetRecordByCode(const CString& codeValue);
    template <typename T> static void SetFieldByStringName(T& data, const CString& name, const CString& value);
    template <typename T> std::vector<T> GetAllGlobal();

    //template<typename T> const std::vector<PropertyMap<T>>& GetProperties();

    std::vector<Offer_cstr> GetOffersByPropertyID(const CString& propID);
    bool SoftDeleteGlobal(const CString& tableName, const CString& code);

    // ... public: bloğunun içine ...

// Tek ve yetkili tanımlar (Varsayılan argüman SADECE BURADA olur)
    static std::string CStringToAnsi(const CString& str, UINT codePage = 1254);
    static CString AnsiToCString(const std::string& str, UINT codePage = 1254);

    //std::string CStringToAnsi(const CString& wideStr, UINT codePage = 1254);
    //CString AnsiToCString(const std::string& ansiStr, UINT codePage = 1254);
    bool SaveCompanyInfo(const CompanyInfo_cstr& info);
    CompanyInfo_cstr GetCompanyInfo();



    // --- TEMPLATE OLANLAR (Struct İçin) ---
    template <typename T> void Bind_Data_To_UI(CWnd* pDlg, const T& data);
    template <typename T> void Bind_UI_To_Data(CWnd* pDlg, T& data);

    // --- 🌟 EKLENENLER (Map İçin - vHomeDlg Uyumluluğu) ---
    void Bind_Data_To_UI(CWnd* pDlg, const CString& tableName, const std::map<CString, CString>& dataMap);
    void Bind_UI_To_Data(CWnd* pDlg, const CString& tableName, std::map<CString, CString>& outDataMap);

    void CheckAndMigrateTables();

    std::map<CString, CString> FetchRecordMap(const CString& tableName, const CString& codeField, const CString& codeValue);

    /* Duran YALÇIN 17/01/2026
    Fonksiyon Adı.:   DatabaseManager::FetchRecordsMapsByCariKod
    Açıklama :        Tek sorgu ile (Cari_Kod filtresiyle) tüm kayıtları alan bazlı map olarak döndürür.
                      TreeListView doldurma gibi senaryolarda N+1 sorgu problemini engeller.
    */
    std::vector<std::map<CString, CString>> FetchRecordsMapsByCariKod(const CString& tableName, const CString& cariKod);
    void    EnsureAttributesColumn();

    std::vector<Customer_cstr> GetCustomers();
    //tüm randevuları al
    std::vector<Appointment_cstr> GetAppointmentsByCustomer(const CString& custID);
    // 🔹 Yardımcı Fonksiyonlar
    CString GenerateGuidString()const;
    CString GenerateSyncId() ;
    CString GetCurrentIsoUtc()const ;
    CString GetCurrentIsoUtc2();
    CString VariantToCString(const _variant_t& v);
    CString EscapeSQLString(const CString& str) const;
    CString AccessDateNowOrLiteral(const CString& maybeDate);

    inline std::string WStringToUtf8(const CString& wstr);

    // 🔹 Bağlantı Yönetimi
    bool InitializeDatabase();
    void CloseConnection();
    bool ExecuteSQL(const CString& strSQL);
    bool EnsureConnection();
    bool CreateDatabaseFile() const;
    bool IsConnected() const { return (IConnectionPtr != NULL && IConnectionPtr->State == adStateOpen); }

    void SetDatabasePath(const CString& path);
    bool EnsureSyncColumnsExists(const CString& tableName, const CString& columnName, const CString& columnType);

    // 🔹 Müşteri İşlemleri
    bool InsertCustomerBatch(const std::vector<Customer_cstr>& customers);
    bool UpdateCustomerField(const CString& cariKod, const CString& fieldName, const CString& newValue);
    bool DeleteCustomers(const std::vector<CString>& cariCodes);
    bool UpdateCustomerStatus(const CString& cariKod, const CString& statusID);
    bool UpdateCustomersStatusBulk(const std::vector<CString>& cariCodes, const CString& statusID);
    bool IsCariCodeExists(const CString& cariKod);

    CString GenerateNextCariCode();
    Customer_cstr GetCustomerByCariKod(const CString& cariKod);
    std::vector<Car_cstr> GetCarsByCariKod(const CString& cariKod);

    // ============================================================================
    //  Sahibinden / Bulk Import - Dedupe & Matching
    //  - Aynı telefonla tekrar müşteri açmayı engeller
    //  - Aynı (WebsiteName, ListingNo) ilanını tekrar eklemeyi engeller
    // ============================================================================
    bool FindCustomerByPhoneLike(const CString& inputPhone, CString& outCariKod);
    bool ExistsPropertyByListing(const CString& websiteName, const CString& listingNo);

    CString GenerateNextCarCode();


    // 🔹 Gayrimenkul (Property) İşlemleri
    CString GenerateNextHomeCode();
    Land_cstr GetLandByCode(const CString& code);
    Field_cstr GetFieldByCode(const CString& code);
    Vineyard_cstr GetVineyardByCode(const CString& code);
    Villa_cstr GetVillaByCode(const CString& code);
    Commercial_cstr GetCommercialByCode(const CString& code);

    bool DeleteRecord(const CString& tableName, const CString& code);

    // 🔹 Kod Üretim Fonksiyonları
    CString GenerateNextPropertyCode(const CString& tableName, const CString& prefix);
    CString GenerateNextLandCode();
    CString GenerateNextFieldCode();
    CString GenerateNextVineyardCode();
    CString GenerateNextVillaCode();
    CString GenerateNextCommercialCode();
    //CString GenerateNextCarCode();
    bool UpdateFieldGlobal(const CString& tableName,
        const CString& codeField,
        const CString& codeValue,
        const CString& fieldName,
        const CString& newValue);
    // 🔹 Veri Çekme Fonksiyonları
    std::vector<HeaderInfo> GetHeaderConfig(const CString& tableName) const;
    bool FetchRecordFields(const CString& tableName, const CString& codeField,
        const CString& codeValue, std::vector<std::pair<CString, CString>>& outFields);
    bool FetchRecordFieldsWithCustomer(const CString& tableName, const CString& codeField,
        const CString& codeValue, std::vector<std::pair<CString, CString>>& outFields);

    std::vector<Land_cstr> GetLandsByCariKod(const CString& cariKod);
    std::vector<Home_cstr> GetHomesByCariKod(const CString& cariKod);
    std::vector<Field_cstr> GetFieldsByCariKod(const CString& cariKod);
    std::vector<Vineyard_cstr> GetVineyardsByCariKod(const CString& cariKod);
    std::vector<Villa_cstr> GetVillasByCariKod(const CString& cariKod);
    std::vector<Commercial_cstr> GetCommercialsByCariKod(const CString& cariKod);
    //std::vector<Car_cstr> GetCarsByCariKod(const CString& cariKod);

    // 🔹 Sync & Log
    void SetSyncInterface(SyncBridge* bridge);
    SyncBridge* GetSyncInterface() const { return m_syncBridge; }

    void InitTableDefinitions();
    void RepairHomeTableEncoding();
    inline void LogToFile(const CString& level, const CString& message);
    CString GetFieldString(_RecordsetPtr& rs, const CString& fieldName) const;
    template<typename T>
    std::map<CString, CString> GetPropertyFieldMap(const T& data) const;

public:
  // 🔹 Cloud Sync Operasyonları
    
    // ✅ Firestore'dan müşterileri indir ve DB'ye ekle/güncelle
    bool ImportCustomersFromCloud(const nlohmann::json& cloudCustomers);
    
    // ✅ Firestore'dan mülkleri indir ve DB'ye ekle/güncelle
    bool ImportPropertiesFromCloud(const nlohmann::json& cloudProperties, const CString& tableName);

    // ✅ Firestore'dan randevuları indir ve DB'ye ekle/güncelle
    bool ImportAppointmentsFromCloud(const nlohmann::json& cloudAppointments);

    // ✅ YENİ: Gmail Contacts'tan müşterileri içe aktar
    bool ImportContactsFromGmail(const std::vector<std::pair<std::string, std::string>>& gmailContacts);
    CString m_connectionString;
    CString m_databasePath;
    CString m_lastDbPath;

    SyncBridge* m_syncBridge = nullptr;
    ISyncInterface* m_pSync = nullptr;

    _ConnectionPtr IConnectionPtr = NULL;
    _RecordsetPtr  IRecordsetPtr;
    _CommandPtr    ICommandPtr;
    ADOX::_CatalogPtr pCatalog;
    CString GetConnectionString() const;
    std::map<CString, CString> m_tableDefinitions;
    std::mutex g_logMutex;
private:
    // 🔸 Constructor & Destructor
    DatabaseManager();
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    // 🔸 Yardımcı Şablonlar

    // 🔸 Dahili Yardımcılar
    CString GetCreateTableSQL(const CString& tableName) const;
    bool CheckAndCreateTables();
    bool TableExists(const CString& tableName);
    bool IsTableEmpty(const CString& tableName);

private:
    // 🔸 Üyeler

};



// --- dataIsMe.h içine eklenecek ---

// T tipine ait (Home_cstr, Land_cstr vb.) tüm silinmemiş kayıtları getir.
template <typename T>
std::vector<T> DatabaseManager::GetAllGlobal()
{
    std::vector<T> records;
    const CString tableName = PropertyTrait<T>::GetTableName(); // Tablo adını otomatik alır

    if (!EnsureConnection()) return records;

    // Sadece silinmemiş kayıtları çek
    CString strSQL;
    strSQL.Format(_T("SELECT * FROM [%s] WHERE Deleted = FALSE"), tableName.c_str());

    _RecordsetPtr rs;
    try
    {
        rs.CreateInstance(__uuidof(Recordset));
        rs->Open((_variant_t)strSQL,
            _variant_t((IDispatch*)IConnectionPtr, true),
            adOpenStatic, adLockReadOnly, adCmdText);

        if (rs->EndOfFile == VARIANT_FALSE)
        {
            long fieldCount = rs->Fields->Count;
            while (!rs->EndOfFile)
            {
                T record = {};
                for (long i = 0; i < fieldCount; ++i)
                {
                    _bstr_t name = rs->Fields->GetItem(i)->Name;
                    _variant_t val = rs->Fields->GetItem(i)->Value;

                    // Struct içine veriyi doldur
                    SetFieldByStringName(record, (LPCWSTR)name, VariantToCString(val));
                }
                records.push_back(record);
                rs->MoveNext();
            }
        }
        if (rs) rs->Close();
    }
    catch (const _com_error& e)
    {
        LogToFile(L"[ERROR]", L"GetAllGlobal Hatası: " + tableName + L" - " + (LPCWSTR)e.Description());
    }

    return records;
}




// Bu fonksiyon DatabaseManager sınıfının üyesidir



// 🧙‍♂️ UI BINDING: DİYALOG <-> VERİ OTOMASYONU (GÜÇLENDİRİLMİŞ)
template <typename T>
void DatabaseManager::Bind_Data_To_UI(CWnd* pDlg, const T& data)
{
    if (!pDlg || !pDlg->GetHwnd()) return;

    const auto& props = GetProperties<T>();

    for (const auto& p : props)
    {
        // Formda karşılığı olmayan alanları atla
        if (p.resID == 0) continue;

        // 1. Veriyi Struct'tan Oku ve Temizle
        CString val = data.*(p.member);
        val.Trim(); // 🔥 En önemli düzeltme: Boşlukları temizle!

        // 2. Kontrolü Bul
        HWND hCtrl = pDlg->GetDlgItem(p.resID);
        if (!hCtrl) {
            TRACE(L"[UI-BIND UYARI] Kontrol bulunamadı! ID: %d, Field: %s\n", p.resID, p.dbName);
            continue;
        }

        // 3. Kontrol Tipini Belirle (Genişletilmiş Algılama)
        TCHAR szClass[64];
        ::GetClassName(hCtrl, szClass, 64);
        CString className = szClass;


        // --- DATE TIME PICKER (SysDateTimePick32) ---
        if (className.CompareNoCase(_T("SysDateTimePick32")) == 0 || p.type == FT_DATE)
        {
            // Beklenen format: yyyy-mm-dd veya yyyy-mm-ddThh:mm:ssZ vb.
            SYSTEMTIME st{}; 
            ::GetLocalTime(&st); // default
            int y=0,mn=0,d=0,hh=0,mm=0,ss=0;
            // Basit parse: yyyy-mm-dd
            if (_stscanf_s(val, _T("%d-%d-%d"), &y, &mn, &d) >= 3 && y>1900 && mn>=1 && mn<=12 && d>=1 && d<=31) {
                st.wYear=(WORD)y; st.wMonth=(WORD)mn; st.wDay=(WORD)d;
                // Saat bilgisi varsa al
                _stscanf_s(val, _T("%d-%d-%dT%d:%d:%d"), &y, &mn, &d, &hh, &mm, &ss);
                if (hh>=0 && hh<=23) st.wHour=(WORD)hh;
                if (mm>=0 && mm<=59) st.wMinute=(WORD)mm;
                if (ss>=0 && ss<=59) st.wSecond=(WORD)ss;
            }
            DateTime_SetSystemtime(hCtrl, GDT_VALID, &st);
            continue;
        }

        // Debug için log (Gerekirse açabilirsiniz)
        // TRACE(L"Field: %s, Val: %s, Class: %s\n", p.dbName, val.c_str(), className.c_str());

        // --- A) COMBOBOX İŞLEMLERİ ---
        // "Combo" kelimesi geçiyorsa (ComboBox, ComboBoxEx32 vb.)
        if (className.Find(_T("Combo")) != -1)
        {
            // Eğer veri boşsa seçimi kaldır
            if (val.IsEmpty()) {
                ::SendMessage(hCtrl, CB_SETCURSEL, -1, 0);
                continue;
            }

            // 1. Deneme: Tam Eşleşme Ara (Exact Match)
            LRESULT idx = ::SendMessage(hCtrl, CB_FINDSTRINGEXACT, -1, (LPARAM)(LPCTSTR)val);

            // 2. Deneme: Bulamazsa "İle Başlayan" Ara (Prefix Match)
            if (idx == CB_ERR) {
                idx = ::SendMessage(hCtrl, CB_SELECTSTRING, -1, (LPARAM)(LPCTSTR)val);
            }
            else {
                // Tam eşleşme bulduysa seç
                ::SendMessage(hCtrl, CB_SETCURSEL, idx, 0);
            }

            // Eğer hala seçemediyse ve veri varsa...
            if (idx == CB_ERR)
            {
                // DropDown (Yazılabilir) ise metni direkt yaz
                // Style kontrolü: CBS_DROPDOWNLIST (Sadece Seçim) mi?
                LONG_PTR style = ::GetWindowLongPtr(hCtrl, GWL_STYLE);
                if ((style & CBS_DROPDOWNLIST) != CBS_DROPDOWNLIST) {
                    ::SetWindowText(hCtrl, val);
                    // TRACE(L" -> Combo listede yoktu, metin olarak yazıldı: %s\n", val.c_str());
                }
                else {
                    // TRACE(L" -> HATA: Combo'da '%s' değeri listede yok ve kutu kilitli (DropDownList)!\n", val.c_str());
                    // İsterseniz burada listeye ekleme yapabilirsiniz:
                    // ::SendMessage(hCtrl, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)val);
                    // ::SendMessage(hCtrl, CB_SELECTSTRING, -1, (LPARAM)(LPCTSTR)val);
                }
            }
        }
        // --- B) CHECKBOX / RADIO BUTTON İŞLEMLERİ ---
        else if (p.type == FT_BOOL || className.Find(_T("Button")) != -1)
        {
            // "1", "True", "Evet", "Var" -> İşaretle
            bool bCheck = (val == _T("1") ||
                val.CompareNoCase(_T("True")) == 0 ||
                val.CompareNoCase(_T("Evet")) == 0 ||
                val.CompareNoCase(_T("Var")) == 0);

            ::SendMessage(hCtrl, BM_SETCHECK, bCheck ? BST_CHECKED : BST_UNCHECKED, 0);
        }
        // --- C) EDITBOX / DİĞERLERİ ---
        else
        {
            ::SetWindowText(hCtrl, val);
        }
    }
}
template <typename T>
void DatabaseManager::Bind_UI_To_Data(CWnd* pDlg, T& data) {
    const auto& props = GetProperties<T>();
    for (const auto& p : props) {
        if (p.resID == 0) continue;
        HWND hCtrl = pDlg->GetDlgItem(p.resID);
        if (!hCtrl) continue;
        TCHAR szClass[64]; ::GetClassName(hCtrl, szClass, 64);
        CString className = szClass; CString val;


        // --- DATE TIME PICKER (SysDateTimePick32) ---
        if (className.CompareNoCase(_T("SysDateTimePick32")) == 0 || p.type == FT_DATE) {
            SYSTEMTIME st{}; 
            if (DateTime_GetSystemtime(hCtrl, &st) == GDT_VALID) {
                val.Format(_T("%04u-%02u-%02u"), st.wYear, st.wMonth, st.wDay);
            } else {
                val = _T("");
            }
            data.*(p.member) = val;
            continue;
        }
        if (className.CompareNoCase(_T("ComboBox")) == 0) {
            int sel = (int)::SendMessage(hCtrl, CB_GETCURSEL, 0, 0);
            if (sel != CB_ERR) {
                int len = (int)::SendMessage(hCtrl, CB_GETLBTEXTLEN, sel, 0);
                if (len > 0) { std::vector<TCHAR> buf(len + 1); ::SendMessage(hCtrl, CB_GETLBTEXT, sel, (LPARAM)buf.data()); val = buf.data(); }
            }
            else {
                int len = ::GetWindowTextLength(hCtrl); if (len > 0) { std::vector<TCHAR> buf(len + 1); ::GetWindowText(hCtrl, buf.data(), len + 1); val = buf.data(); }
            }
        }
        else if (p.type == FT_BOOL || className.CompareNoCase(_T("Button")) == 0) {
            val = (::SendMessage(hCtrl, BM_GETCHECK, 0, 0) == BST_CHECKED) ? _T("1") : _T("0");
        }
        else {
            int len = ::GetWindowTextLength(hCtrl); if (len > 0) { std::vector<TCHAR> buf(len + 1); ::GetWindowText(hCtrl, buf.data(), len + 1); val = buf.data(); }
        }
        data.*(p.member) = val;
    }
}




// =============================================================
//  TEMPLATE IMPLEMENTASYONLARI (Header'da olmak zorunda)
// =============================================================

template<typename T>
void DatabaseManager::SetFieldByStringName(T& data, const CString& name, const CString& value)
{
    const auto& props = GetProperties<T>();
    for (const auto& p : props) {
        if (p.dbName && name.CompareNoCase(p.dbName) == 0) {
            data.*(p.member) = value;
            return;
        }
    }
}

template <typename T>
std::map<CString, CString> DatabaseManager::GetPropertyFieldMap(const T& data) const
{
    std::map<CString, CString> fields;
    const auto& props = GetProperties<T>();
    for (const auto& p : props) {
        if (p.dbName) {
            CString val = data.*(p.member);
            val.Trim();
            fields[p.dbName] = val;
        }
    }
    return fields;
}


template <typename T>
bool DatabaseManager::InsertGlobal(const T& data)
{
    // 1. Meta Verileri Al
    const CString tableName = PropertyTrait<T>::GetTableName();
    if (!EnsureConnection()) return false;

    // 2. Veri Kopyası ve Otomatik Alanlar
    T copy = data;
    if constexpr (!std::is_same_v<T, Offer_cstr>) {
        if (copy.sync_id.IsEmpty()) copy.sync_id = GenerateGuidString();
        copy.Updated_At = GetCurrentIsoUtc();
        copy.Deleted = _T("FALSE");
    }

    // 3. Fabrikadan Alanları ve Tipleri Çek
    const auto& props = GetProperties<T>();

    // SQL için geçici liste: {DB Adı, Değer, Tip}
    struct ValInfo { CString name; CString val; FieldType type; };
    std::vector<ValInfo> insertFields;

    for (const auto& p : props) {
        if (!p.dbName) continue; // DB alanı değilse geç

        // Struct'tan değeri oku
        CString val = copy.*(p.member);
        val.Trim(); // Boşlukları temizle

        insertFields.push_back({ p.dbName, val, p.type });
    }

    if (insertFields.empty()) return false;

    // 4. SQL Hazırlığı
    CString colList, qList;
    for (size_t i = 0; i < insertFields.size(); ++i) {
        if (i > 0) { colList += L","; qList += L","; }
        colList += L"[" + insertFields[i].name + L"]";
        qList += L"?";
    }

    // 5. Parametre Bağlama ve Çalıştırma
    try {
        _CommandPtr cmd; cmd.CreateInstance(__uuidof(Command));
        cmd->ActiveConnection = IConnectionPtr;
        cmd->CommandText = (_bstr_t)CString(_T("INSERT INTO [") + tableName + _T("] (") + colList + _T(") VALUES (") + qList + _T(")"));

        for (const auto& f : insertFields) {
            _variant_t v;
            DataTypeEnum adType = adVarWChar;
            long size = f.val.GetLength() + 1;

            // --- AKILLI TİP YÖNETİMİ ---
            if (f.name.CompareNoCase(_T("Deleted")) == 0) {
                // Boolean Özel Durumu
                bool b = (f.val.CompareNoCase(_T("TRUE")) == 0 || f.val == _T("1"));
                v = b; adType = adBoolean; size = 0;
            }
            else if (f.type == FT_DATE) {
                adType = adDate;

                if (f.val.IsEmpty()) {
                    v.vt = VT_NULL;
                }
                else {
                    // Metni (YYYY-MM-DD HH:MM) parçala ve gerçek Tarih (VT_DATE) yap
                    int year = 0, month = 0, day = 0, hour = 0, minute = 0;

                    // Formatı kontrol ederek oku
                    if (_stscanf_s(f.val, _T("%d-%d-%d %d:%d"), &year, &month, &day, &hour, &minute) >= 3)
                    {
                        SYSTEMTIME st = { 0 };
                        st.wYear = (WORD)year;
                        st.wMonth = (WORD)month;
                        st.wDay = (WORD)day;
                        st.wHour = (WORD)hour;
                        st.wMinute = (WORD)minute;

                        double dDate = 0.0;
                        // Windows API ile SystemTime -> VariantTime dönüşümü
                        if (SystemTimeToVariantTime(&st, &dDate)) {
                            v.vt = VT_DATE;
                            v.date = dDate;
                        }
                        else {
                            v.vt = VT_NULL; // Dönüşüm başarısızsa NULL gönder
                        }
                    }
                    else {
                        // Format bozuksa NULL gönder (Hata vermesin)
                        v.vt = VT_NULL;
                    }
                }
            }            // ...
            else if (f.type == FT_DOUBLE || f.type == FT_INT) {
                // Sayısal alanlar için türü belirle
                adType = (f.type == FT_DOUBLE) ? adDouble : adInteger;

                if (f.val.IsEmpty()) {
                    v.vt = VT_NULL;
                }
                else {
                    // DÜZELTME: Metni gerçek sayıya (Double/Long) çeviriyoruz.
                    if (f.type == FT_DOUBLE)
                        v = _tstof(f.val); // Double (Ondalıklı Sayı)
                    else
                        v = (long)_ttol(f.val);  // Long (Tam Sayı)
                }
            }
            // ...
            else {
                // Metin: Olduğu gibi gönder (Boş string NULL değildir)
                adType = adVarWChar;
                if (f.type == FT_MEMO) adType = adLongVarWChar;
                v = (f.val.IsEmpty()) ? _variant_t(L"") : _variant_t((LPCTSTR)f.val);
            }

            cmd->Parameters->Append(cmd->CreateParameter(_bstr_t(""), adType, adParamInput, size, v));
        }

        cmd->Execute(NULL, NULL, adCmdText);

        // Sync Bildirimi
        if (m_pSync) {
            nlohmann::json jRow;
            for (const auto& f : insertFields) jRow[CStringToAnsi(f.name)] = CStringToAnsi(f.val);
            m_pSync->OnLocalChange(CStringToAnsi(tableName), "UPSERT", jRow);
        }
        return true;
    }
    catch (const _com_error& e) {
        CString err; err.Format(L"InsertGlobal Hatası (%s): %s", tableName.GetString(), (LPCTSTR)e.Description());
        LogToFile(L"[ERROR]", err);
        ::MessageBox(NULL, err, L"Kayıt Hatası", MB_ICONERROR); // Hatayı ekranda göster
        return false;
    }
    catch (...) { return false; }
}

template <typename T>
bool DatabaseManager::UpdateGlobal(const T& data)
{
    const CString tableName = PropertyTrait<T>::GetTableName();
    const CString codeField = PropertyTrait<T>::GetCodeFieldName();
    const CString codeValue = PropertyTrait<T>::GetCodeValue(data);

    if (!EnsureConnection()) return false;

    T copy = data;
    if constexpr (!std::is_same_v<T, Offer_cstr>) {
        copy.Updated_At = GetCurrentIsoUtc();
    }

    // Fabrikadan Alanları Çek
    const auto& props = GetProperties<T>();

    struct ValInfo { CString name; CString val; FieldType type; };
    std::vector<ValInfo> updateFields;

    for (const auto& p : props) {
        if (!p.dbName) continue;
        if (CString(p.dbName).CompareNoCase(codeField) == 0) continue; // PK güncellenmez

        CString val = copy.*(p.member);
        val.Trim();
        updateFields.push_back({ p.dbName, val, p.type });
    }

    if (updateFields.empty()) return false;

    // SQL Hazırlığı
    CString setList;
    for (size_t i = 0; i < updateFields.size(); ++i) {
        if (i > 0) setList += L",";
        setList += L"[" + updateFields[i].name + L"]=?";
    }

    try {
        _CommandPtr cmd; cmd.CreateInstance(__uuidof(Command));
        cmd->ActiveConnection = IConnectionPtr;
        cmd->CommandText = (_bstr_t)CString(_T("UPDATE [") + tableName + _T("] SET ") + setList + _T(" WHERE [") + codeField + _T("]=?"));

        // A) SET Parametreleri
        for (const auto& f : updateFields) {
            _variant_t v;
            DataTypeEnum adType = adVarWChar;
            long size = f.val.GetLength() + 1;

            // --- AKILLI TİP YÖNETİMİ ---
            if (f.name.CompareNoCase(_T("Deleted")) == 0) {
                bool b = (f.val.CompareNoCase(_T("TRUE")) == 0 || f.val == _T("1"));
                v = b; adType = adBoolean; size = 0;
            }
            else if (f.type == FT_DATE) {
                adType = adDate;

                if (f.val.IsEmpty()) {
                    v.vt = VT_NULL;
                }
                else {
                    // Metni (YYYY-MM-DD HH:MM) parçala ve gerçek Tarih (VT_DATE) yap
                    int year = 0, month = 0, day = 0, hour = 0, minute = 0;

                    // Formatı kontrol ederek oku
                    if (_stscanf_s(f.val, _T("%d-%d-%d %d:%d"), &year, &month, &day, &hour, &minute) >= 3)
                    {
                        SYSTEMTIME st = { 0 };
                        st.wYear = (WORD)year;
                        st.wMonth = (WORD)month;
                        st.wDay = (WORD)day;
                        st.wHour = (WORD)hour;
                        st.wMinute = (WORD)minute;

                        double dDate = 0.0;
                        // Windows API ile SystemTime -> VariantTime dönüşümü
                        if (SystemTimeToVariantTime(&st, &dDate)) {
                            v.vt = VT_DATE;
                            v.date = dDate;
                        }
                        else {
                            v.vt = VT_NULL; // Dönüşüm başarısızsa NULL gönder
                        }
                    }
                    else {
                        // Format bozuksa NULL gönder (Hata vermesin)
                        v.vt = VT_NULL;
                    }
                }
            }            // ...
            else if (f.type == FT_DOUBLE || f.type == FT_INT) {
                // Sayısal alanlar için türü belirle
                adType = (f.type == FT_DOUBLE) ? adDouble : adInteger;

                if (f.val.IsEmpty()) {
                    v.vt = VT_NULL;
                }
                else {
                    // DÜZELTME: Metni gerçek sayıya (Double/Long) çeviriyoruz.
                    if (f.type == FT_DOUBLE)
                        v = _tstof(f.val); // Double (Ondalıklı Sayı)
                    else
                        v = (long)_ttol(f.val);  // Long (Tam Sayı)
                }
            }
            // ...
            else {
                adType = adVarWChar;
                if (f.type == FT_MEMO) adType = adLongVarWChar;
                v = (f.val.IsEmpty()) ? _variant_t(L"") : _variant_t((LPCTSTR)f.val);
            }
            cmd->Parameters->Append(cmd->CreateParameter(_bstr_t(""), adType, adParamInput, size, v));
        }

        // B) WHERE Parametresi (PK)
        cmd->Parameters->Append(cmd->CreateParameter(_bstr_t(""), adVarWChar, adParamInput, codeValue.GetLength() + 1, _variant_t((LPCTSTR)codeValue)));

        cmd->Execute(NULL, NULL, adCmdText);

        // Sync Bildirimi
        if (m_pSync) {
            nlohmann::json jRow;
            for (const auto& f : updateFields) jRow[CStringToAnsi(f.name)] = CStringToAnsi(f.val);
            // PK'yı da ekle ki neyin güncellendiği belli olsun
            jRow[CStringToAnsi(codeField)] = CStringToAnsi(codeValue);
            m_pSync->OnLocalChange(CStringToAnsi(tableName), "UPSERT", jRow);
        }
        return true;
    }
    catch (const _com_error& e) {
        CString err; err.Format(L"UpdateGlobal Hatası (%s): %s", tableName.GetString(), (LPCTSTR)e.Description());
        LogToFile(L"[ERROR]", err);
        ::MessageBox(NULL, err, L"Güncelleme Hatası", MB_ICONERROR);
        return false;
    }
    catch (...) { return false; }
}



template <typename T>
std::vector<T> DatabaseManager::GetRecordsByCariKod(const CString& cariKod)
{
    std::vector<T> records;
    const CString tableName = PropertyTrait<T>::GetTableName(); // Tablo adını trait'ten al

    // Bağlantı kontrolü
    if (!IsConnected())

    {

        InitializeDatabase();

        TRACE(_T("HATA: GetHomesByCariKod çağrısı başarısız. Bağlantı açık değil.\n"));

        return records;

    }

    CString escapedCariKod = EscapeSQLString(cariKod);

    // SQL Sorgusu: Tüm alanları (*) ve Cari Kod filtresi + Deleted=FALSE filtresi
    CString strSQL;
    strSQL.Format(
        _T("SELECT * FROM [%s] WHERE [Cari_Kod] = '%s' AND Deleted = FALSE"),
        tableName.c_str(), (LPCTSTR)escapedCariKod
    );

    _RecordsetPtr rs;
    rs.CreateInstance(__uuidof(Recordset));

    try
    {
        rs->Open((_variant_t)strSQL,
            _variant_t((IDispatch*)IConnectionPtr, true),
            adOpenStatic, adLockReadOnly, adCmdText);

        long fieldCount = rs->Fields->Count;

        while (!rs->EndOfFile)
        {
            T record = {}; // Yeni struct objesi, varsayılan olarak sıfırlanır

            for (long i = 0; i < fieldCount; ++i)
            {
                _bstr_t name = rs->Fields->GetItem(i)->Name;
                _variant_t val = rs->Fields->GetItem(i)->Value;

                CString fieldName = (LPCWSTR)name;

                // VariantToCString, NULL/EMPTY değerlerini CString(L"") döndürerek halleder
                CString value = VariantToCString(val);

                // 🔥 EN KRİTİK EŞLEŞTİRME 🔥
                SetFieldByStringName(record, fieldName, value);
            }

            records.push_back(record);
            rs->MoveNext();
        }

        if (rs) rs->Close();
    }
    catch (const _com_error& e)
    {
        TRACE(L"HATA: GetRecordsByCariKod (%s) - ADO Hatası: %s\n", tableName.c_str(), (LPCWSTR)e.Description());
    }

    return records;
}



// dataIsMe.cpp

// dataIsMe.cpp

template <typename T>
T DatabaseManager::GetRecordByCode(const CString& codeValue)
{
    T record = {};
    const CString tableName = PropertyTrait<T>::GetTableName();
    const CString codeField = PropertyTrait<T>::GetCodeFieldName();

    if (!EnsureConnection()) return record;

    CString escapedCodeValue = EscapeSQLString(codeValue);

    // SQL Sorgusu: Deleted=FALSE filtresi
    CString strSQL;
    strSQL.Format(
        _T("SELECT * FROM [%s] WHERE [%s] = '%s' AND Deleted = FALSE"),
        tableName.c_str(), codeField.c_str(), (LPCTSTR)escapedCodeValue
    );

    _RecordsetPtr rs;
    rs.CreateInstance(__uuidof(Recordset));

    try
    {
        rs->Open((_variant_t)strSQL,
            _variant_t((IDispatch*)IConnectionPtr, true),
            adOpenStatic, adLockReadOnly, adCmdText);

        // 🔥 GÜVENLİK KONTROLÜ: Kayıt var mı? (İmleç ilk kayıttayken kontrol edilir)
        if (rs->EndOfFile == VARIANT_FALSE)
        {
            long fieldCount = rs->Fields->Count;

            // Kaydın sadece ilk satırını okur (Primary Key sorgusu olduğu için)
            // rs->MoveFirst() veya while döngüsüne gerek yoktur.

            for (long i = 0; i < fieldCount; ++i)
            {
                _bstr_t name = rs->Fields->GetItem(i)->Name;
                _variant_t val = rs->Fields->GetItem(i)->Value;

                CString fieldName = (LPCWSTR)name;

                // VariantToCString: VT_NULL/VT_EMPTY durumlarını CString("") döndürerek yönetir.
                CString value = VariantToCString(val);

                // SetFieldByStringName: Struct alanına eşleştirir.
                SetFieldByStringName(record, fieldName, value);
            }
        }

        if (rs) rs->Close();
    }
    catch (const _com_error& e)
    {
        // ADO Hatalarını Yakalama (Örn: Tablo adı yanlış veya sütun adı yanlış)
        TRACE(L"HATA: GetRecordByCode (%s - %s) - ADO Hatası: %s\n",
            tableName.c_str(),
            codeValue.c_str(),
            (LPCWSTR)e.Description());
    }

    return record;
}

// ============================================================================
// ✅ MÜŞTERİ KATEGORİ SİSTEMİ
// ============================================================================

//enum class CustomerCategory : int
//{
//    ALL = 0,                    // Tümü
//    ACTIVE_BUYERS = 1,          // 🟢 Aktif Alıcılar (Çalışma Yapabilir)
//    ACTIVE_SELLERS = 2,       // 🟢 Aktif Satıcılar (Çalışma Yapabilir)
//    NEGOTIATING = 3,            // 🟡 Müzakere Aşamasında
//    WAITING_FEEDBACK = 4,       // ⏳ Geri Dönüş Bekleniyor
//    NOT_INTERESTED = 5,         // 🔴 İlgisiz / Çalışmak İstemeyenler
//    COLD_PROSPECT = 6,     // ❄️ Soğuk İletişim / İlk Kez İletişim
//    VIP_CLIENT = 7,    // ⭐ VIP Müşteriler
//    INACTIVE = 8     // ⚫ Pasif / Arşivlenmiş
//};

// Files.h veya ilgili header dosyası

inline CString GetCategoryName(CustomerCategory cat)
{
    switch (cat)
    {
    case CAT_HEPSI:               return _T("Tümü");

        // İşlem Önceliği Yüksek Olanlar (Sıcak)
    case CAT_SICAK_ALICI:         return _T("🔥 Sıcak Alıcı");
    case CAT_SICAK_SATICI:        return _T("🔥 Sıcak Satıcı");

        // Potansiyeller (Orta Vadeli)
    case CAT_POTANSIYEL_ALICI:    return _T("🟡 Potansiyel Alıcı");
    case CAT_POTANSIYEL_SATICI:   return _T("🟡 Potansiyel Satıcı");

        // Özel Gruplar
    case CAT_YATIRIMCI:           return _T("💼 Yatırımcı");
    case CAT_KIRACI_ADAYI:        return _T("🏠 Kiracı Adayı");
    case CAT_EV_SAHIBI_KIRALIK:   return _T("🔑 Ev Sahibi (Kiralık)");

        // Süreçtekiler
    case CAT_YENI_MUSTERI:        return _T("✨ Yeni Müşteri");
    case CAT_TAKIPTE:             return _T("📅 Takipte (Aranacak)");
    case CAT_RANDEVULU:           return _T("🤝 Randevulu");
    case CAT_TEKLIF_VERDI:        return _T("📄 Teklif Verdi");

        // Sonuçlananlar
    case CAT_ISLEM_TAMAM:         return _T("✅ İşlem Tamamlandı");

    default:                      return _T("❓ Tanımsız Durum");
    }
}
#endif // !DATAISME_H