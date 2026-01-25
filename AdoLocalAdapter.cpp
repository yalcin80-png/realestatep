#include "stdafx.h"
#include "AdoLocalAdapter.h"
#include <sstream>
#include <iomanip>
#include <chrono>
#include <mutex>


using json = nlohmann::json;
static std::mutex g_dbMutex;

// Yardımcı: SQL Escape (Tırnak işaretlerini düzeltir)
static std::string q(const std::string& s) {
    std::string x = s;
    size_t pos = 0;
    while ((pos = x.find("'", pos)) != std::string::npos) { x.insert(pos, "'"); pos += 2; }
    return "'" + x + "'";
}

// ------------------------------------------------------------
// Varsayılan Bağlantı Cümlesi
// ------------------------------------------------------------
// AdoLocalAdapter.cpp içindeki GetDefaultConnStr fonksiyonunu bulun ve DEĞİŞTİRİN:

std::string AdoLocalAdapter::GetDefaultConnStr() {
    // DOĞRU BAĞLANTI CÜMLESİ (DatabaseManager ile aynı olmalı)
    // Eğer DatabaseManager 16.0 kullanıyorsa burası da 16.0 olmalı.
    // Dosya yolu D:\\RealEstate.mdb olarak ayarlandı.
    return "Provider=Microsoft.ACE.OLEDB.16.0;Data Source=D:\\RealEstate.mdb;Persist Security Info=False;";
}


// ------------------------------------------------------------
// Constructor (TEK VE BİRLEŞİK)
// ------------------------------------------------------------
// Constructor'ı (Kurucu fonksiyonu) bununla değiştirin:
AdoLocalAdapter::AdoLocalAdapter(const std::string& connStr)
{
    // Parametre boşsa varsayılanı (yukarıda düzelttiğimiz) al
    m_connStr = connStr.empty() ? GetDefaultConnStr() : connStr;

    try {
        if (m_conn == nullptr) {
            m_conn.CreateInstance(__uuidof(Connection));
        }
        // Bağlantıyı açmayı dene
        m_conn->Open(_bstr_t(m_connStr.c_str()), L"", L"", adConnectUnspecified);
    }
    catch (const _com_error& e) {
        // HATA VARSA LOGLA!
        std::string err = "[AdoLocalAdapter] Baglanti Hatasi: ";
        if (e.ErrorMessage()) err += (const char*)e.ErrorMessage();
        err += "\nKullanilan String: " + m_connStr + "\n";
        OutputDebugStringA(err.c_str());
    }
}
AdoLocalAdapter::~AdoLocalAdapter() {
    try { if (m_conn && m_conn->State == adStateOpen) m_conn->Close(); }
    catch (...) {}
}

// ------------------------------------------------------------
// Temel Veritabanı İşlemleri
// ------------------------------------------------------------
bool AdoLocalAdapter::Exec(const std::string& sql) {
    std::lock_guard<std::mutex> lock(g_dbMutex);
    try {
        m_conn->Execute(_bstr_t(sql.c_str()), nullptr, adCmdText);
        return true;
    }
    catch (const _com_error& e) {
        // Tablo zaten var hatalarını görmezden gelmek isterseniz logu kapatabilirsiniz
        // TRACE(L"[ADO Error] %s\n", e.ErrorMessage());
        return false;
    }
}

json AdoLocalAdapter::QueryJson(const std::string& sql) {
    std::lock_guard<std::mutex> lock(g_dbMutex);
    json arr = json::array();
    try {
        _RecordsetPtr rs;
        rs.CreateInstance(__uuidof(Recordset));
        rs->Open(_bstr_t(sql.c_str()), _variant_t((IDispatch*)m_conn, true), adOpenForwardOnly, adLockReadOnly, adCmdText);

        while (!rs->EndOfFile) {
            json row = json::object();
            long cnt = rs->Fields->Count;
            for (long i = 0; i < cnt; ++i) {
                _bstr_t name = rs->Fields->Item[i]->Name;
                _variant_t v = rs->Fields->Item[i]->Value;

                if (v.vt == VT_NULL) row[(const char*)name] = nullptr;
                else {
                    // Her şeyi string olarak alıp JSON'a koyuyoruz, güvenli yöntem
                    try {
                        v.ChangeType(VT_BSTR);
                        row[(const char*)name] = std::string((const char*)_bstr_t(v));
                    }
                    catch (...) {
                        row[(const char*)name] = "";
                    }
                }
            }
            arr.push_back(row);
            rs->MoveNext();
        }
        rs->Close();
    }
    catch (...) {}
    return arr;
}

// ------------------------------------------------------------
// Sync Interface Uygulamaları
// ------------------------------------------------------------
bool AdoLocalAdapter::EnsureSyncColumns(const std::string& table) {
    // Sütun eklemeleri (Hata verirse zaten var demektir, devam eder)
    Exec("ALTER TABLE " + table + " ADD COLUMN sync_id TEXT(50)");
    Exec("ALTER TABLE " + table + " ADD COLUMN Updated_At TEXT(30)");
    Exec("ALTER TABLE " + table + " ADD COLUMN deleted YESNO");
    return true;
}

json AdoLocalAdapter::GetChangedRows(const std::string& table, const std::string& sinceIsoUtc) {
    return QueryJson("SELECT * FROM " + table + " WHERE Updated_At > " + q(sinceIsoUtc));
}

// AdoLocalAdapter.cpp içine, mevcut UpsertRows fonksiyonunun yerine yapıştırın.

// AdoLocalAdapter.cpp -> UpsertRows fonksiyonunu bununla değiştirin

// src/AdoLocalAdapter.cpp -> UpsertRows fonksiyonunu bununla tamamen değiştirin:
// src/AdoLocalAdapter.cpp -> UpsertRows fonksiyonunu bununla değiştirin:

// BU FONKSİYONU AdoLocalAdapter.cpp EN BAŞINA (Headerların altına) EKLEYİN
// UTF-8 (Firestore) -> ANSI (Windows Türkçe) Çevirici
static std::string Utf8ToAnsi(const std::string& utf8Str)
{
    if (utf8Str.empty()) return "";

    // 1. UTF-8'den Wide Char'a (Unicode) çevir
    int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, NULL, 0);
    if (wlen == 0) return "";
    std::vector<wchar_t> wstr(wlen);
    MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, &wstr[0], wlen);

    // 2. Wide Char'dan ANSI'ye (Sistem Dili - Türkçe) çevir
    // CP_ACP: Aktif Kod Sayfası (Türkçe Windows'ta 1254'tür)
    int len = WideCharToMultiByte(CP_ACP, 0, &wstr[0], -1, NULL, 0, NULL, NULL);
    if (len == 0) return "";
    std::vector<char> str(len);
    WideCharToMultiByte(CP_ACP, 0, &wstr[0], -1, &str[0], len, NULL, NULL);

    return std::string(&str[0]);
}

bool AdoLocalAdapter::UpsertRows(const std::string& table, const json& rows)
{
    std::lock_guard<std::mutex> lock(g_dbMutex);
    bool globalSuccess = true;

    if (m_conn == nullptr || m_conn->State != adStateOpen) {
        OutputDebugStringA("[Upsert Error] Veritabanı bağlantısı kapalı!\n");
        return false;
    }

    // Tarih temizleyici
    auto CleanDate = [](std::string val) -> std::string {
        size_t tPos = val.find('T');
        if (tPos != std::string::npos) val[tPos] = ' ';
        if (!val.empty() && val.back() == 'Z') val.pop_back();
        return val;
        };

    _RecordsetPtr rs = nullptr;
    try {
        rs.CreateInstance(__uuidof(Recordset));
    }
    catch (...) { return false; }

    for (const auto& r : rows)
    {
        m_conn->BeginTrans();

        try
        {
            if (!r.contains("sync_id") || r["sync_id"].is_null()) {
                m_conn->RollbackTrans();
                continue;
            }
            std::string sid = r["sync_id"].get<std::string>();
            if (sid.empty()) {
                m_conn->RollbackTrans();
                continue;
            }

            // Silme İşlemi
            std::string sqlDelete = "DELETE FROM [" + table + "] WHERE sync_id='" + sid + "'";
            m_conn->Execute(_bstr_t(sqlDelete.c_str()), NULL, adCmdText);

            // Ekleme Hazırlığı (AddNew)
            std::string selectCmd = "SELECT * FROM [" + table + "] WHERE 1=0";
            rs->Open(_bstr_t(selectCmd.c_str()),
                _variant_t((IDispatch*)m_conn, true),
                adOpenKeyset, adLockOptimistic, adCmdText);

            rs->AddNew();

            for (auto it = r.begin(); it != r.end(); ++it)
            {
                std::string key = it.key();
                if (it->is_null()) continue;

                try
                {
                    _variant_t val;

                    if (it->is_boolean()) {
                        val = it->get<bool>();
                    }
                    else if (it->is_number()) {
                        val = it->dump().c_str();
                    }
                    else {
                        // String İşlemleri
                        std::string sVal = it->get<std::string>();

                        // ÖNCE KARAKTER KODLAMASINI DÜZELT (UTF8 -> ANSI)
                        sVal = Utf8ToAnsi(sVal);

                        // Sonra Tarih Formatı Temizliği
                        if (sVal.length() > 10 && sVal[10] == 'T') {
                            sVal = CleanDate(sVal); // CleanDate artık ANSI string üzerinde çalışır
                        }

                        val = sVal.c_str();
                    }

                    rs->Fields->GetItem(key.c_str())->Value = val;
                }
                catch (...) {}
            }

            rs->Update();
            rs->Close();
            m_conn->CommitTrans();
        }
        catch (const _com_error& e)
        {
            m_conn->RollbackTrans();
            if (rs->State == adStateOpen) rs->Close();
            // Loglama...
            globalSuccess = false;
        }
        catch (...)
        {
            m_conn->RollbackTrans();
            if (rs->State == adStateOpen) rs->Close();
            globalSuccess = false;
        }
    }

    return globalSuccess;
}
// src/AdoLocalAdapter.cpp -> UpsertRows fonksiyonunu bununla tamamen değiştirin:
//bool AdoLocalAdapter::UpsertRows(const std::string& table, const json& rows)
//{
//    std::lock_guard<std::mutex> lock(g_dbMutex);
//    bool globalSuccess = true;
//
//    if (m_conn == nullptr || m_conn->State != adStateOpen) {
//        OutputDebugStringA("[Upsert Error] Veritabanı bağlantısı kapalı!\n");
//        return false;
//    }
//
//    // Yardımcı: Tarih formatını temizle (ISO8601 -> Access string)
//    // Firestore "2024-05-21T15:30:00Z" gönderir, Access "2024-05-21 15:30:00" ister.
//    auto CleanDate = [](std::string val) -> std::string {
//        size_t tPos = val.find('T');
//        if (tPos != std::string::npos) val[tPos] = ' ';
//        if (!val.empty() && val.back() == 'Z') val.pop_back();
//        return val;
//        };
//
//    // Kayıt kümesi (Recordset) nesnesi oluştur
//    _RecordsetPtr rs = nullptr;
//    try {
//        rs.CreateInstance(__uuidof(Recordset));
//    }
//    catch (...) { return false; }
//
//    for (const auto& r : rows)
//    {
//        // 1. Transaction Başlat (Ya Hep Ya Hiç)
//        m_conn->BeginTrans();
//
//        try
//        {
//            // Kimlik Kontrolü
//            if (!r.contains("sync_id") || r["sync_id"].is_null()) {
//                m_conn->RollbackTrans();
//                continue;
//            }
//            std::string sid = r["sync_id"].get<std::string>();
//            if (sid.empty()) {
//                m_conn->RollbackTrans();
//                continue;
//            }
//
//            // 2. Önce Eski Kaydı Sil (SQL ile silmek güvenlidir)
//            // Köşeli parantez kullanarak tablo ismini koruyoruz
//            std::string sqlDelete = "DELETE FROM [" + table + "] WHERE sync_id='" + sid + "'";
//            m_conn->Execute(_bstr_t(sqlDelete.c_str()), NULL, adCmdText);
//
//            // 3. YENİ YÖNTEM: SQL YAZMADAN EKLEME (AddNew)
//            // Boş bir kayıt kümesi açıyoruz (Veri çekmeden sadece yapı gelir)
//            std::string selectCmd = "SELECT * FROM [" + table + "] WHERE 1=0";
//
//            rs->Open(_bstr_t(selectCmd.c_str()),
//                _variant_t((IDispatch*)m_conn, true),
//                adOpenKeyset,
//                adLockOptimistic,
//                adCmdText);
//
//            rs->AddNew(); // Yeni boş satır oluştur
//
//            // JSON içindeki her alanı döngüyle gez ve tabloya yaz
//            for (auto it = r.begin(); it != r.end(); ++it)
//            {
//                std::string key = it.key();
//                if (it->is_null()) continue; // Boş veriyi atla
//
//                try
//                {
//                    _variant_t val;
//
//                    if (it->is_boolean()) {
//                        val = it->get<bool>();
//                    }
//                    else if (it->is_number()) {
//                        // Sayıları string olarak versek bile Access doğru tipe çevirir
//                        val = it->dump().c_str();
//                    }
//                    else {
//                        // String İşlemleri
//                        std::string sVal = it->get<std::string>();
//
//                        // Tarih formatı kontrolü (T ve Z varsa temizle)
//                        if (sVal.length() > 10 && sVal[10] == 'T') {
//                            sVal = CleanDate(sVal);
//                        }
//                        val = sVal.c_str();
//                    }
//
//                    // KRİTİK NOKTA: Alan veritabanında var mı?
//                    // Varsa yazar, yoksa (örn: metadata) hata verir, catch yakalar ve devam eder.
//                    // Böylece "Bilinmeyen Sütun" veya #3079 hatası almayız.
//                    rs->Fields->GetItem(key.c_str())->Value = val;
//                }
//                catch (...) {
//                    // Bu alan Access tablosunda yokmuş, sorun değil, sessizce geç.
//                }
//            }
//
//            rs->Update(); // Veriyi fiziksel olarak kaydet
//            rs->Close();  // Kayıt kümesini kapat
//
//            // Her şey başarılı, işlemi onayla (Commit)
//            m_conn->CommitTrans();
//        }
//        catch (const _com_error& e)
//        {
//            // Hata olursa her şeyi geri al (Rollback)
//            m_conn->RollbackTrans();
//            if (rs->State == adStateOpen) rs->Close();
//
//            std::string errMsg = (e.ErrorMessage()) ? (const char*)e.ErrorMessage() : "Bilinmeyen Hata";
//            std::string fullErr = "[Upsert HATA - ROLLBACK] Tablo: " + table + " Mesaj: " + errMsg + "\n";
//            OutputDebugStringA(fullErr.c_str());
//
//            globalSuccess = false;
//        }
//        catch (...)
//        {
//            m_conn->RollbackTrans();
//            if (rs->State == adStateOpen) rs->Close();
//            OutputDebugStringA("[Upsert Bilinmeyen Hata]\n");
//            globalSuccess = false;
//        }
//    }
//
//    return globalSuccess;
//}
//bool AdoLocalAdapter::UpsertRows(const std::string& table, const json& rows)
//{
//    std::lock_guard<std::mutex> lock(g_dbMutex);
//    bool globalSuccess = true;
//
//    if (m_conn == nullptr || m_conn->State != adStateOpen) {
//        OutputDebugStringA("[Upsert Error] Veritabanı bağlantısı kapalı!\n");
//        return false;
//    }
//
//    // Yardımcı: Tarih formatını temizle (ISO8601 -> Access string)
//    auto CleanDate = [](std::string val) -> std::string {
//        // "2024-05-21T15:30:00Z" -> "2024-05-21 15:30:00"
//        size_t tPos = val.find('T');
//        if (tPos != std::string::npos) val[tPos] = ' ';
//        if (!val.empty() && val.back() == 'Z') val.pop_back();
//        return val;
//        };
//
//    // Kayıt kümesi nesnesi oluştur
//    _RecordsetPtr rs = nullptr;
//    try {
//        rs.CreateInstance(__uuidof(Recordset));
//    }
//    catch (...) { return false; }
//
//    for (const auto& r : rows)
//    {
//        // Transaction: "Ya hep ya hiç"
//        m_conn->BeginTrans();
//
//        try
//        {
//            // 1. sync_id Kontrolü
//            if (!r.contains("sync_id") || r["sync_id"].is_null()) {
//                m_conn->RollbackTrans();
//                continue;
//            }
//            std::string sid = r["sync_id"].get<std::string>();
//            if (sid.empty()) {
//                m_conn->RollbackTrans();
//                continue;
//            }
//
//            // 2. Önce Eski Kaydı Sil (SQL ile silmek güvenlidir)
//            // Tırnak hatasını önlemek için basit ve temiz bir sorgu
//            std::string sqlDelete = "DELETE FROM [" + table + "] WHERE sync_id='" + sid + "'";
//            m_conn->Execute(_bstr_t(sqlDelete.c_str()), NULL, adCmdText);
//
//            // 3. Yeni Kaydı Ekle (SQL YERİNE RECORDSET KULLANIYORUZ - #3079 ÇÖZÜMÜ)
//            // Boş bir kayıt kümesi açıyoruz (WHERE 1=0 ile veri çekmiyoruz, sadece yapı geliyor)
//            std::string selectCmd = "SELECT * FROM [" + table + "] WHERE 1=0";
//
//            rs->Open(_bstr_t(selectCmd.c_str()),
//                _variant_t((IDispatch*)m_conn, true),
//                adOpenKeyset,
//                adLockOptimistic,
//                adCmdText);
//
//            rs->AddNew(); // Yeni satır oluştur
//
//            for (auto it = r.begin(); it != r.end(); ++it)
//            {
//                std::string key = it.key();
//                if (it->is_null()) continue; // Boşsa geç
//
//                try
//                {
//                    // Değeri hazırla
//                    _variant_t val;
//
//                    if (it->is_boolean()) {
//                        val = it->get<bool>();
//                    }
//                    else if (it->is_number()) {
//                        // Sayıları da string olarak atamak Access'te daha güvenlidir, o çevirir
//                        val = it->dump().c_str();
//                    }
//                    else {
//                        // String İşlemleri
//                        std::string sVal = it->get<std::string>();
//
//                        // Tarih formatı kontrolü
//                        if (sVal.length() > 10 && sVal[10] == 'T') {
//                            sVal = CleanDate(sVal);
//                        }
//                        val = sVal.c_str();
//                    }
//
//                    // ALAN VAR MI DİYE DENE VE YAZ
//                    // Eğer tablo sütunu yoksa burada hata verir, catch yakalar ve devam eder.
//                    // Bu sayede "Bilinmeyen Sütun" hatası almayız.
//                    rs->Fields->GetItem(key.c_str())->Value = val;
//                }
//                catch (...) {
//                    // Bu alan Access tablosunda yokmuş, sorun değil, atla.
//                }
//            }
//
//            rs->Update(); // Kaydı fiziksel olarak yaz
//            rs->Close();  // Kayıt kümesini kapat
//
//            // Her şey yolunda, onayla
//            m_conn->CommitTrans();
//        }
//        catch (const _com_error& e)
//        {
//            // Hata olursa işlemi geri al (Silinen veri geri gelir)
//            m_conn->RollbackTrans();
//            if (rs->State == adStateOpen) rs->Close();
//
//            std::string errMsg = (e.ErrorMessage()) ? (const char*)e.ErrorMessage() : "Unknown COM Error";
//            std::string fullErr = "[Upsert HATA - ROLLBACK] Tablo: " + table + " Mesaj: " + errMsg + "\n";
//            OutputDebugStringA(fullErr.c_str());
//
//            globalSuccess = false;
//        }
//        catch (...)
//        {
//            m_conn->RollbackTrans();
//            if (rs->State == adStateOpen) rs->Close();
//            OutputDebugStringA("[Upsert Bilinmeyen Hata]\n");
//            globalSuccess = false;
//        }
//    }
//
//    return globalSuccess;
//}
bool AdoLocalAdapter::MarkDeleted(const std::string& table, const std::vector<std::string>& sync_ids) {
    for (const auto& id : sync_ids)
        Exec("UPDATE " + table + " SET deleted=TRUE WHERE sync_id=" + q(id));
    return true;
}

std::string AdoLocalAdapter::GetLastSyncStampIsoUtc() {
    auto arr = QueryJson("SELECT last_sync_utc FROM app_settings WHERE id=1");
    if (!arr.empty() && arr[0].contains("last_sync_utc") && !arr[0]["last_sync_utc"].is_null())
        return arr[0]["last_sync_utc"].get<std::string>();
    return "";
}

void AdoLocalAdapter::SetLastSyncStampIsoUtc(const std::string& isoUtc) {
    // 1970'e çekme işleminin çalışması için burası kritik
    std::string sqlUpd = "UPDATE app_settings SET last_sync_utc=" + q(isoUtc) + " WHERE id=1";

    // Önce güncellemeyi dene
    if (!Exec(sqlUpd)) {
        // Eğer güncelleme başarısızsa (tablo yoksa veya kayıt yoksa) Ekleme dene
        // Not: Exec fonksiyonumuz hata durumunda false döner.
        // Ancak ADO '0 records affected' durumunda hata vermeyebilir.
        // O yüzden en garantisi: Doğrudan INSERT denemektir (ID=1 primary key ise hata verir, biz de yakalarız)

        // Basit yöntem: Sil ve Ekle (Replace mantığı)
        Exec("DELETE FROM app_settings WHERE id=1");
        Exec("INSERT INTO app_settings (id, last_sync_utc) VALUES (1, " + q(isoUtc) + ")");
    }
    else {
        // Update çalıştı ama belki 0 kayıt etkilendi.
        // Bunu kontrol etmek zor olduğu için, garanti olsun diye Insert de denenebilir.
        // Ama en temiz yöntem şudur:
        auto arr = QueryJson("SELECT count(*) as C FROM app_settings WHERE id=1");
        if (arr.empty() || arr[0].value("C", "0") == "0") {
            Exec("INSERT INTO app_settings (id, last_sync_utc) VALUES (1, " + q(isoUtc) + ")");
        }
    }
}

std::string AdoLocalAdapter::GetLastSyncStampIsoUtc(const std::string& table) { return GetLastSyncStampIsoUtc(); }
void AdoLocalAdapter::SetLastSyncStampIsoUtc(const std::string& table, const std::string& stamp) { SetLastSyncStampIsoUtc(stamp); }