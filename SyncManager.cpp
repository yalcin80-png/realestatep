// DOSYA: src/SyncManager.cpp
#include "stdafx.h"
#include "SyncManager.h"
#include "dataIsMe.h"
#include <vector>
#include <string>

using json = nlohmann::json;

// --- Yardımcı Fonksiyonlar (CString -> UTF8) ---
static std::string ToUtf8(const CString& str)
{
    if (str.IsEmpty()) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)str, str.GetLength(), NULL, 0, NULL, NULL);
    if (size_needed <= 0) return std::string();
    std::string result(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)str, str.GetLength(), &result[0], size_needed, NULL, NULL);
    return result;
}
static std::string FormatToFirestoreDate(const CString& rawDate)
{
    std::string s = ToUtf8(rawDate);
    if (s.empty()) return "1970-01-01T00:00:00Z";

    // Zaten uygun formatta mı? (2024-05-20T14:30:00Z gibi)
    if (s.find('T') != std::string::npos && s.back() == 'Z') return s;

    // Basit düzeltme: "2024-01-01 12:00:00" -> "2024-01-01T12:00:00Z"
    // Boşlukları T ile değiştir
    for (auto& c : s) {
        if (c == ' ') c = 'T';
    }
    // Sona 'Z' (UTC) ekle
    if (s.back() != 'Z') s += 'Z';

    return s;
}
// ----------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------
SyncManager::SyncManager(ISyncLocalAdapter& local, ICloudProvider& cloud)
    : m_local(local), m_cloud(cloud)
{
}

std::string SyncManager::MaxIso(const std::string& a, const std::string& b)
{
    return (a >= b) ? a : b;
}

// ----------------------------------------------------------------------
// SyncMany: Tüm tabloları sırasıyla eşitler
// ----------------------------------------------------------------------
bool SyncManager::SyncMany(const std::vector<std::pair<std::string, std::string>>& pairs, ProgressCallback onProgress)
{
    bool allOk = true;
    size_t total = pairs.size();

    for (size_t i = 0; i < total; ++i)
    {
        std::string tableName = pairs[i].first;
        std::string collectionName = pairs[i].second;

        // İlerleme Bildirimi
        if (onProgress) {
            int percent = (int)((double)i / total * 100.0);
            onProgress("Islem yapiliyor: " + tableName, percent);
        }

        // 1. ADIM: PUSH (Yereldeki verileri Cloud'a gönder)
        if (!PushToCloud(tableName, collectionName)) {
            TRACE(L"[SyncManager] UYARI: %S tablosu gonderilemedi.\n", tableName.c_str());
            // Push hatası olsa bile Pull denenebilir.
        }

        // 2. ADIM: PULL (Cloud'daki verileri yerele çek)
        if (!PullFromCloud(tableName, collectionName)) {
            TRACE(L"[SyncManager] UYARI: %S tablosu alinamadi.\n", tableName.c_str());
            allOk = false;
        }
    }

    if (onProgress) onProgress("Tamamlandi", 100);
    return allOk;
}

// ----------------------------------------------------------------------
// PushToCloud
// ----------------------------------------------------------------------
bool SyncManager::PushToCloud(const std::string& table, const std::string& collection)
{
    try
    {
        CString tableName = CString(table.c_str());
        DatabaseManager& db = DatabaseManager::GetInstance();
        json jsonBatch = json::array();

        // Veritabanından verileri çeken Lambda Fonksiyonu
        auto ProcessTable = [&](auto struct_type_tag) {
            using T = decltype(struct_type_tag);
            auto records = db.GetAllGlobal<T>(); // Tüm kayıtları çek

            for (auto& rec : records) {
                bool modified = false;

                // A) Sync ID yoksa oluştur (Çok Önemli!)
                if (rec.sync_id.IsEmpty()) {
                    rec.sync_id = db.GenerateGuidString();
                    modified = true;
                }
                // B) Tarih yoksa şu anı ver
                if (rec.Updated_At.IsEmpty()) {
                    rec.Updated_At = db.GetCurrentIsoUtc();
                    modified = true;
                }

                // C) Eğer ID veya Tarih yeni eklendiyse Yerel DB'yi güncelle
                if (modified) {
                    db.UpdateGlobal(rec);
                }

                // JSON'a çevir ve listeye ekle
                json jRow = this->RecordToJson(tableName, &rec);
                if (!jRow.empty()) {
                    jsonBatch.push_back(jRow);
                }
            }
            };

        // Tablo Tipine Göre İşlem Seç
        if (table == "Customer")          ProcessTable(Customer_cstr{});
        else if (table == "HomeTbl")      ProcessTable(Home_cstr{});
        else if (table == "Land")         ProcessTable(Land_cstr{});
        else if (table == "Field")        ProcessTable(Field_cstr{});
        else if (table == "Villa")        ProcessTable(Villa_cstr{});
        else if (table == "Vineyard")     ProcessTable(Vineyard_cstr{});
        else if (table == "Commercial")   ProcessTable(Commercial_cstr{});
        else if (table == "Appointments") ProcessTable(Appointment_cstr{});
        else if (table == "Offers")       ProcessTable(Offer_cstr{});
        else {
            return false;
        }

        if (jsonBatch.empty()) return true;

        // CloudProvider'a gönder
        return m_cloud.PushUpserts(collection, jsonBatch);
    }
    catch (...) {
        return false;
    }
}

// ----------------------------------------------------------------------
// PullFromCloud
// ----------------------------------------------------------------------
bool SyncManager::PullFromCloud(const std::string& table, const std::string& collection)
{
    try
    {
        // 1. Tablo kolonlarını kontrol et
        if (!m_local.EnsureSyncColumns(table)) return false;

        // 2. En son ne zaman eşitlemişiz?
        std::string since = m_local.GetLastSyncStampIsoUtc();

        // --- KRİTİK DÜZELTME BAŞLANGICI ---
        // Yerel veritabanından gelen tarih "2026-01-13 10:00:00" gibi bozuk olabilir.
        // Bunu Firestore'un kabul edeceği "2026-01-13T10:00:00Z" formatına zorluyoruz.
        if (!since.empty()) {
            since = FormatToFirestoreDate(CString(since.c_str()));
        }

        // Eğer hala boşsa veya 1970 öncesiyse sıfırla
        if (since.empty() || since == "1970-01-01T00:00:00Z") {
            since = "1970-01-01T00:00:00Z";
        }
        // --- KRİTİK DÜZELTME BİTİŞİ ---

        TRACE(L"[SyncManager] Pull %S -> Tarihinden itibaren: %S\n", table.c_str(), since.c_str());

        // 3. Cloud'dan veriyi iste (Artık 'since' kesinlikle düzgün formatta)
        json cloudData = m_cloud.PullChanges(collection, since);
        if (cloudData.empty()) return true;

        json toUpsert = json::array();
        std::vector<std::string> toDelete;

        for (const auto& row : cloudData) {
            if (!row.is_object()) continue;

            // Silinmiş mi kontrol et
            bool isDeleted = false;
            if (row.contains("deleted") && row["deleted"].is_boolean())
                isDeleted = row["deleted"].get<bool>();
            else if (row.contains("Deleted") && row["Deleted"].is_string())
                isDeleted = (row["Deleted"].get<std::string>() == "TRUE");

            std::string sid = row.value("sync_id", "");

            if (isDeleted && !sid.empty()) toDelete.push_back(sid);
            else toUpsert.push_back(row);
        }

        // 4. Yerel DB'ye Yaz
        bool success = true;
        if (!toUpsert.empty()) {
            if (!m_local.UpsertRows(table, toUpsert)) success = false;
        }
        if (!toDelete.empty()) {
            m_local.MarkDeleted(table, toDelete);
        }

        // 5. Tarih Damgasını Güncelle
        if (success) {
            std::string latest = since;
            for (const auto& r : cloudData) {
                if (r.contains("Updated_At")) latest = MaxIso(latest, r["Updated_At"].get<std::string>());
            }
            // Sunucu saatini de hesaba kat
            latest = MaxIso(latest, m_cloud.NowIsoUtc());
            m_local.SetLastSyncStampIsoUtc(latest);
        }
        return success;
    }
    catch (...) {
        return false;
    }
}

// ----------------------------------------------------------------------
// RecordToJson (Veriyi JSON'a çevir)
// ----------------------------------------------------------------------

// Eski (Kullanılmayan) Fonksiyonlar
void SyncManager::NotifyUpsert(const std::string& collection, const json& row) {}
void SyncManager::NotifyDelete(const std::string& collection, const std::string& sync_id) {}
json SyncManager::ResolveConflicts(const json& localRows, const json& cloudRows) { return json::array(); }
bool SyncManager::Sync(const std::string& table, const std::string& collection) { return false; }


json SyncManager::RecordToJson(const CString& tableName, void* recordData)
{
    json j;

    // ========================================================================
    // 1. MÜŞTERİ (Customer)
    // ========================================================================
    if (tableName == L"Customer")
    {
        Customer_cstr* c = (Customer_cstr*)recordData;
        
        j["Cari_Kod"]       = ToUtf8(c->Cari_Kod);
        j["AdSoyad"]        = ToUtf8(c->AdSoyad);
        j["Telefon"]        = ToUtf8(c->Telefon);
        j["Telefon2"]       = ToUtf8(c->Telefon2);
        j["Email"]          = ToUtf8(c->Email);
        j["Adres"]          = ToUtf8(c->Adres);
        j["Sehir"]          = ToUtf8(c->Sehir);
        j["Ilce"]           = ToUtf8(c->Ilce);
        j["TCKN"]           = ToUtf8(c->TCKN);
        j["VergiNo"]        = ToUtf8(c->VergiNo);
        j["MusteriTipi"]    = ToUtf8(c->MusteriTipi);
        j["KayitTarihi"]    = ToUtf8(c->KayitTarihi);
        j["Durum"]          = ToUtf8(c->Durum);
        j["Notlar"]         = ToUtf8(c->Notlar);
        j["BabaAdi"]        = ToUtf8(c->BabaAdi);
        j["DogumTarihi"]    = ToUtf8(c->DogumTarihi);
        j["Nationality"]    = ToUtf8(c->Uyruk);
        j["KimlikSeriNo"]   = ToUtf8(c->KimlikSeriNo);
        j["MedeniHal"]      = ToUtf8(c->MedeniHal);
        j["IBAN"]           = ToUtf8(c->IBAN);
        j["BankaAdi"]       = ToUtf8(c->BankaAdi);
        j["VergiDairesi"]   = ToUtf8(c->VergiDairesi);
        j["YetkiliKisi"]    = ToUtf8(c->YetkiliKisi);
        j["Calisma_Durumu"] = ToUtf8(c->Calisma_Durumu);
        j["Status"]         = ToUtf8(c->Status);
        
        // Sistem Alanları
        j["sync_id"]    = ToUtf8(c->sync_id);
        j["Updated_At"] = FormatToFirestoreDate(c->Updated_At); // <-- YENİSİ
        j["Deleted"]    = ToUtf8(c->Deleted);
        j["Attributes"] = ToUtf8(c->Attributes);
    }

    // ========================================================================
    // 2. KONUT (HomeTbl)
    // ========================================================================
    else if (tableName == L"HomeTbl")
    {
        Home_cstr* h = (Home_cstr*)recordData;
        
        j["Cari_Kod"]       = ToUtf8(h->Cari_Kod);
        j["Home_Code"]      = ToUtf8(h->Home_Code);
        j["ListingNo"]      = ToUtf8(h->ListingNo);
        j["ListingDate"]    = ToUtf8(h->ListingDate);
        j["PropertyType"]   = ToUtf8(h->PropertyType);
        j["Address"]        = ToUtf8(h->Address);
        j["City"]           = ToUtf8(h->City);
        j["District"]       = ToUtf8(h->District);
        j["Neighborhood"]   = ToUtf8(h->Neighborhood);
        j["Location"]       = ToUtf8(h->Location);
        j["Price"]          = ToUtf8(h->Price);
        j["Currency"]       = ToUtf8(h->Currency);
        j["PricePerM2"]     = ToUtf8(h->PricePerM2);
        j["Dues"]           = ToUtf8(h->Dues);
        j["CreditEligible"] = ToUtf8(h->CreditEligible);
        j["Swap"]           = ToUtf8(h->Swap);
        j["RoomCount"]      = ToUtf8(h->RoomCount);
        j["NetArea"]        = ToUtf8(h->NetArea);
        j["GrossArea"]      = ToUtf8(h->GrossArea);
        j["Floor"]          = ToUtf8(h->Floor);
        j["TotalFloor"]     = ToUtf8(h->TotalFloor);
        j["BuildingAge"]    = ToUtf8(h->BuildingAge);
        j["HeatingType"]    = ToUtf8(h->HeatingType);
        j["BathroomCount"]  = ToUtf8(h->BathroomCount);
        j["KitchenType"]    = ToUtf8(h->KitchenType);
        j["Balcony"]        = ToUtf8(h->Balcony);
        j["Elevator"]       = ToUtf8(h->Elevator);
        j["Parking"]        = ToUtf8(h->Parking);
        j["Furnished"]      = ToUtf8(h->Furnished);
        j["InSite"]         = ToUtf8(h->InSite);
        j["SiteName"]       = ToUtf8(h->SiteName);
        j["Ada"]            = ToUtf8(h->Ada);
        j["Parsel"]         = ToUtf8(h->Parsel);
        j["Pafta"]          = ToUtf8(h->Pafta);
        j["DeedStatus"]     = ToUtf8(h->DeedStatus);
        j["YetkiBaslangic"] = ToUtf8(h->YetkiBaslangic);
        j["YetkiBitis"]     = ToUtf8(h->YetkiBitis);
        j["SellerType"]     = ToUtf8(h->SellerType);
        j["UsageStatus"]    = ToUtf8(h->UsageStatus);
        j["Status"]         = ToUtf8(h->Status);
        j["WebsiteName"]    = ToUtf8(h->WebsiteName);
        j["ListingURL"]     = ToUtf8(h->ListingURL);
        j["NoteGeneral"]    = ToUtf8(h->NoteGeneral);
        j["NoteInternal"]   = ToUtf8(h->NoteInternal);
        j["PurchaseDate"]   = ToUtf8(h->PurchaseDate);
        j["PurchasePrice"]  = ToUtf8(h->PurchasePrice);
        j["SaleDate"]       = ToUtf8(h->SaleDate);
        j["SalePrice"]      = ToUtf8(h->SalePrice);
        j["EnergyClass"]    = ToUtf8(h->EnergyClass);
        j["GeoLatitude"]    = ToUtf8(h->GeoLatitude);
        j["GeoLongitude"]   = ToUtf8(h->GeoLongitude);
        j["PhotoCount"]     = ToUtf8(h->PhotoCount);

        j["sync_id"]    = ToUtf8(h->sync_id);
        j["Updated_At"] = FormatToFirestoreDate(h->Updated_At); // <-- YENİSİ
        j["Deleted"]    = ToUtf8(h->Deleted);
        j["Attributes"] = ToUtf8(h->Attributes);
    }

    // ========================================================================
    // 3. ARSA (Land)
    // ========================================================================
    else if (tableName == L"Land")
    {
        Land_cstr* l = (Land_cstr*)recordData;
        
        j["Cari_Kod"]       = ToUtf8(l->Cari_Kod);
        j["Land_Code"]      = ToUtf8(l->Land_Code);
        j["Adres"]          = ToUtf8(l->Adres);
        j["Lokasyon"]       = ToUtf8(l->Lokasyon);
        j["Ada"]            = ToUtf8(l->Ada);
        j["Parsel"]         = ToUtf8(l->Parsel);
        j["Pafta"]          = ToUtf8(l->Pafta);
        j["Metrekare"]      = ToUtf8(l->Metrekare);
        j["ImarDurumu"]     = ToUtf8(l->ImarDurumu);
        j["Fiyat"]          = ToUtf8(l->Fiyat);
        j["Currency"]       = ToUtf8(l->Currency);
        j["Teklifler"]      = ToUtf8(l->Teklifler);
        j["Durum"]          = ToUtf8(l->Durum);
        j["Status"]         = ToUtf8(l->Status);
        j["YetkiBaslangic"] = ToUtf8(l->YetkiBaslangic);
        j["YetkiBitis"]     = ToUtf8(l->YetkiBitis);

        j["sync_id"]    = ToUtf8(l->sync_id);
        j["Updated_At"] = FormatToFirestoreDate(l->Updated_At); // <-- YENİSİ
        j["Deleted"]    = ToUtf8(l->Deleted);
        j["Attributes"] = ToUtf8(l->Attributes);
    }

    // ========================================================================
    // 4. TARLA (Field)
    // ========================================================================
    else if (tableName == L"Field")
    {
        Field_cstr* f = (Field_cstr*)recordData;
        
        j["Cari_Kod"]       = ToUtf8(f->Cari_Kod);
        j["Field_Code"]     = ToUtf8(f->Field_Code);
        j["ListingNo"]      = ToUtf8(f->ListingNo);
        j["ListingDate"]    = ToUtf8(f->ListingDate);
        j["Adres"]          = ToUtf8(f->Adres);
        j["Lokasyon"]       = ToUtf8(f->Lokasyon);
        j["Metrekare"]      = ToUtf8(f->Metrekare);
        j["Fiyat"]          = ToUtf8(f->Fiyat);
        j["Currency"]       = ToUtf8(f->Currency);
        j["PricePerM2"]     = ToUtf8(f->PricePerM2);
        j["ToprakTuru"]     = ToUtf8(f->ToprakTuru);
        j["Sulama"]         = ToUtf8(f->Sulama);
        j["Ulasim"]         = ToUtf8(f->Ulasim);
        j["Ada"]            = ToUtf8(f->Ada);
        j["Parsel"]         = ToUtf8(f->Parsel);
        j["Pafta"]          = ToUtf8(f->Pafta);
        j["ImarDurumu"]     = ToUtf8(f->ImarDurumu);
        j["DeedStatus"]     = ToUtf8(f->DeedStatus);
        j["Kaks"]           = ToUtf8(f->Kaks);
        j["Gabari"]         = ToUtf8(f->Gabari);
        j["CreditEligible"] = ToUtf8(f->CreditEligible);
        j["SellerType"]     = ToUtf8(f->SellerType);
        j["Swap"]           = ToUtf8(f->Swap);
        j["Teklifler"]      = ToUtf8(f->Teklifler);
        j["Durum"]          = ToUtf8(f->Durum);
        j["Status"]         = ToUtf8(f->Status);
        j["RawKML"]         = ToUtf8(f->RawKML);
        j["PropertyType"]   = ToUtf8(f->PropertyType);
        j["YetkiBaslangic"] = ToUtf8(f->YetkiBaslangic);
        j["YetkiBitis"]     = ToUtf8(f->YetkiBitis);

        j["sync_id"]    = ToUtf8(f->sync_id);
        j["Updated_At"] = FormatToFirestoreDate(f->Updated_At); // <-- YENİSİ
        j["Deleted"]    = ToUtf8(f->Deleted);
        j["Attributes"] = ToUtf8(f->Attributes);
    }

    // ========================================================================
    // 5. BAĞ/BAHÇE (Vineyard)
    // ========================================================================
    else if (tableName == L"Vineyard")
    {
        Vineyard_cstr* v = (Vineyard_cstr*)recordData;
        
        j["Cari_Kod"]       = ToUtf8(v->Cari_Kod);
        j["Vineyard_Code"]  = ToUtf8(v->Vineyard_Code);
        j["Adres"]          = ToUtf8(v->Adres);
        j["Lokasyon"]       = ToUtf8(v->Lokasyon);
        j["Metrekare"]      = ToUtf8(v->Metrekare);
        j["AgacSayisi"]     = ToUtf8(v->AgacSayisi);
        j["UrunTuru"]       = ToUtf8(v->UrunTuru);
        j["Sulama"]         = ToUtf8(v->Sulama);
        j["Fiyat"]          = ToUtf8(v->Fiyat);
        j["Currency"]       = ToUtf8(v->Currency);
        j["Ada"]            = ToUtf8(v->Ada);
        j["Parsel"]         = ToUtf8(v->Parsel);
        j["Pafta"]          = ToUtf8(v->Pafta);
        j["YetkiBaslangic"] = ToUtf8(v->YetkiBaslangic);
        j["YetkiBitis"]     = ToUtf8(v->YetkiBitis);
        j["Teklifler"]      = ToUtf8(v->Teklifler);
        j["Durum"]          = ToUtf8(v->Durum);
        j["Status"]         = ToUtf8(v->Status);

        j["sync_id"]    = ToUtf8(v->sync_id);
        j["Updated_At"] = FormatToFirestoreDate(v->Updated_At); // <-- YENİSİ
        j["Deleted"]    = ToUtf8(v->Deleted);
        j["Attributes"] = ToUtf8(v->Attributes);
    }

    // ========================================================================
    // 6. VILLA (Villa)
    // ========================================================================
    else if (tableName == L"Villa")
    {
        Villa_cstr* v = (Villa_cstr*)recordData;
        
        j["Cari_Kod"]       = ToUtf8(v->Cari_Kod);
        j["Villa_Code"]     = ToUtf8(v->Villa_Code);
        j["Adres"]          = ToUtf8(v->Adres);
        j["Lokasyon"]       = ToUtf8(v->Lokasyon);
        j["Ada"]            = ToUtf8(v->Ada);
        j["Parsel"]         = ToUtf8(v->Parsel);
        j["Pafta"]          = ToUtf8(v->Pafta);
        j["OdaSayisi"]      = ToUtf8(v->OdaSayisi);
        j["NetMetrekare"]   = ToUtf8(v->NetMetrekare);
        j["BrutMetrekare"]  = ToUtf8(v->BrutMetrekare);
        j["KatSayisi"]      = ToUtf8(v->KatSayisi);
        j["Havuz"]          = ToUtf8(v->Havuz);
        j["Bahce"]          = ToUtf8(v->Bahce);
        j["Garaj"]          = ToUtf8(v->Garaj);
        j["Fiyat"]          = ToUtf8(v->Fiyat);
        j["Currency"]       = ToUtf8(v->Currency);
        j["YetkiBaslangic"] = ToUtf8(v->YetkiBaslangic);
        j["YetkiBitis"]     = ToUtf8(v->YetkiBitis);
        j["Teklifler"]      = ToUtf8(v->Teklifler);
        j["Durum"]          = ToUtf8(v->Durum);
        j["Status"]         = ToUtf8(v->Status);

        j["sync_id"]    = ToUtf8(v->sync_id);
        j["Updated_At"] = FormatToFirestoreDate(v->Updated_At); // <-- YENİSİ
        j["Deleted"]    = ToUtf8(v->Deleted);
        j["Attributes"] = ToUtf8(v->Attributes);
    }

    // ========================================================================
    // 7. TİCARİ (Commercial)
    // ========================================================================
    else if (tableName == L"Commercial")
    {
        Commercial_cstr* c = (Commercial_cstr*)recordData;
        
        j["Cari_Kod"]        = ToUtf8(c->Cari_Kod);
        j["Commercial_Code"] = ToUtf8(c->Commercial_Code);
        j["Adres"]           = ToUtf8(c->Adres);
        j["Lokasyon"]        = ToUtf8(c->Lokasyon);
        j["Alan"]            = ToUtf8(c->Alan);
        j["Kat"]             = ToUtf8(c->Kat);
        j["Cephe"]           = ToUtf8(c->Cephe);
        j["Kullanim"]        = ToUtf8(c->Kullanim);
        j["Fiyat"]           = ToUtf8(c->Fiyat);
        j["Currency"]        = ToUtf8(c->Currency);
        j["Ada"]             = ToUtf8(c->Ada);
        j["Parsel"]          = ToUtf8(c->Parsel);
        j["Pafta"]           = ToUtf8(c->Pafta);
        j["YetkiBaslangic"]  = ToUtf8(c->YetkiBaslangic);
        j["YetkiBitis"]      = ToUtf8(c->YetkiBitis);
        j["Teklifler"]       = ToUtf8(c->Teklifler);
        j["Durum"]           = ToUtf8(c->Durum);
        j["Status"]          = ToUtf8(c->Status);

        j["sync_id"]    = ToUtf8(c->sync_id);
        j["Updated_At"] = FormatToFirestoreDate(c->Updated_At); // <-- YENİSİ
        j["Deleted"]    = ToUtf8(c->Deleted);
        j["Attributes"] = ToUtf8(c->Attributes);
    }

    // ========================================================================
    // 8. RANDEVULAR (Appointments) - EKLENDİ
    // ========================================================================
    else if (tableName == L"Appointments")
    {
        Appointment_cstr* a = (Appointment_cstr*)recordData;

        j["AppID"]          = ToUtf8(a->AppID);
        j["CustomerID"]     = ToUtf8(a->CustomerID);
        j["PropertyID"]     = ToUtf8(a->PropertyID);
        j["PropertyType"]   = ToUtf8(a->PropertyType);
        j["AppDate"]        = ToUtf8(a->AppDate);
        j["Title"]          = ToUtf8(a->Title);
        j["Description"]    = ToUtf8(a->Description);
        j["Location"]       = ToUtf8(a->Location);
        j["Status"]         = ToUtf8(a->Status);
        j["GoogleEventID"]  = ToUtf8(a->GoogleEventID);
        j["IsNotified"]     = ToUtf8(a->IsNotified);

        j["sync_id"]    = ToUtf8(a->sync_id);
        j["Updated_At"] = FormatToFirestoreDate(a->Updated_At); // <-- YENİSİ
        j["Deleted"]    = ToUtf8(a->Deleted);
        j["Attributes"] = ToUtf8(a->Attributes);
    }

    // ========================================================================
    // 9. TEKLİFLER (Offers) - EKLENDİ
    // ========================================================================
    else if (tableName == L"Offers")
    {
        Offer_cstr* o = (Offer_cstr*)recordData;

        j["OfferID"]        = ToUtf8(o->OfferID);
        j["PropertyID"]     = ToUtf8(o->PropertyID);
        j["CustomerID"]     = ToUtf8(o->CustomerID);
        j["OfferPrice"]     = ToUtf8(o->OfferPrice);
        j["OfferDate"]      = ToUtf8(o->OfferDate);
        j["PaymentMethod"]  = ToUtf8(o->PaymentMethod);
        j["DepositAmount"]  = ToUtf8(o->DepositAmount);
        j["ValidityDate"]   = ToUtf8(o->ValidityDate);
        j["Status"]         = ToUtf8(o->Status);
        j["Notes"]          = ToUtf8(o->Notes);

        j["sync_id"]    = ToUtf8(o->sync_id);
        j["Updated_At"] = FormatToFirestoreDate(o->Updated_At); // <-- YENİSİ
        j["Deleted"]    = ToUtf8(o->Deleted);
        j["Attributes"] = ToUtf8(o->Attributes);
    }

    return j;
}




void SyncManager::DownloadEverything()
{
    // 1. Tablo Listesi (Yerel Tablo Adı -> Firestore Koleksiyon Adı)
    std::vector<std::pair<std::string, std::string>> tables = {
        {"Customer", "Customer"},
        {"HomeTbl", "Home"},
        {"Land", "Land"},
        {"Field", "Field"},
        {"Villa", "Villa"},
        {"Vineyard", "Vineyard"},
        {"Commercial", "Commercial"}
    };

    TRACE(L"--- Tam Indirme Baslatildi ---\n");

    for (const auto& pair : tables)
    {
        std::string localTable = pair.first;
        std::string cloudColl = pair.second;

        try {
            // A) Buluttan Tarih Filtresi Olmadan Her Şeyi Çek (since = 1970)
            // Not: PullChanges fonksiyonun zaten bu işi yapıyor.
            json cloudData = m_cloud.PullChanges(cloudColl, "1970-01-01T00:00:00Z");

            if (cloudData.empty() || !cloudData.is_array()) {
                TRACE(L"[%S] Bulutta veri bulunamadi veya baglanti hatasi.\n", localTable.c_str());
                continue;
            }

            TRACE(L"[%S] %llu adet kayit indiriliyor...\n", localTable.c_str(), cloudData.size());

            // B) Yerel Veritabanına Yaz (Mevcut UpsertRows fonksiyonunu kullanıyoruz)
            // Bu fonksiyon zaten sync_id'ye bakıp varsa güncelliyor, yoksa ekliyor.
            if (m_local.UpsertRows(localTable, cloudData)) {
                TRACE(L"[%S] Basariyla yerel veritabanina islendi.\n", localTable.c_str());
            }
            else {
                TRACE(L"[%S] Veritabanina yazilirken HATA olustu!\n", localTable.c_str());
            }

        }
        catch (...) {
            TRACE(L"[%S] Beklenmedik bir hata olustu!\n", localTable.c_str());
        }
    }

    // C) Senkronizasyon Zamanını Güncelle (Bir dahaki sefere her şeyi çekmesin diye)
    m_local.SetLastSyncStampIsoUtc(m_cloud.NowIsoUtc());

    TRACE(L"--- Tum Islemler Tamamlandi ---\n");
}