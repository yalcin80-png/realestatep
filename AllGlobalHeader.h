#pragma once

// ⚙️ ADO / ADOX importları (Win32++ uyumlu, ATL'siz)
#pragma warning(push)
#pragma warning(disable: 4146 4278 4192)

// ============================================================
// 1️⃣ ADODB (msado15.dll)
//     → Veriye erişim (Connection, Recordset, Command)
// ============================================================
#import "C:\\Progra~1\\Common~1\\System\\ado\\msado15.dll" \
    no_namespace rename("EOF","EndOfFile") rename("BOF","ADOBEGIN")
// ============================================================
// 2️⃣ ADOX (msadox.dll)
//     → Veritabanı oluşturma, tablo ve kolon yönetimi
//     (ADODB'e bağımlıdır — bu yüzden her zaman sonra import edilir)
// ============================================================
#import "C:\\Progra~1\\Common~1\\System\\ado\\msadox.dll" \
    rename_namespace("ADOX") rename("EndOfFile","ADOEOF") rename("BOF","ADOBEGIN")



#pragma warning(pop)

#include <comdef.h>   // _bstr_t, _variant_t

// -------------------------------------------------------------
// 🔹 Kullanım kolaylığı için kısa typedef’ler
// -------------------------------------------------------------
typedef _ConnectionPtr     ConnectionPtr;
typedef _RecordsetPtr      RecordsetPtr;
typedef _CommandPtr        CommandPtr;
typedef _ParameterPtr      ParameterPtr;

// ADOX için
// ADOX için
typedef ADOX::_CatalogPtr   CatalogPtr;
typedef ADOX::_TablePtr     TablePtr;
typedef ADOX::_ColumnPtr    ColumnPtr;
typedef ADOX::_IndexPtr     IndexPtr;
typedef ADOX::_KeyPtr       KeyPtr;


// -------------------------------------------------------------
// 🔹 COM Başlatma / Kapatma yardımcıları
// -------------------------------------------------------------
inline bool InitializeCOM()
{
    HRESULT hr = CoInitialize(NULL);
    return (SUCCEEDED(hr) || hr == RPC_E_CHANGED_MODE);
}

inline void UninitializeCOM()
{
    CoUninitialize();
}

// -------------------------------------------------------------
// 🔹 ADO hata mesajı dönüştürücü
// -------------------------------------------------------------
inline CStringA GetAdoErrorString(const _com_error& e)
{
    CStringA msg;
    msg.Format("ADO Hatası:\nCode: 0x%08X\nDesc: %s\nSource: %s",
        e.Error(),
        (LPCSTR)_bstr_t(e.Description()),
        (LPCSTR)_bstr_t(e.Source()));
    return msg;
}

// -------------------------------------------------------------
// 🔹 Örnek kullanım
// -------------------------------------------------------------
// ConnectionPtr conn;
// conn.CreateInstance(__uuidof(Connection));
// conn->Open("Provider=Microsoft.ACE.OLEDB.12.0;Data Source=C:\\EmlakCRM\\veriler.accdb;", "", "", adConnectUnspecified);
//
// RecordsetPtr rs;
// rs.CreateInstance(__uuidof(Recordset));
// rs->Open("SELECT * FROM Customer", conn.GetInterfacePtr(), adOpenDynamic, adLockOptimistic, adCmdText);





/*
┌────────────────────────────────────────────────────────────┐
│                       🖥  UI Katmanı                       │
│────────────────────────────────────────────────────────────│
│ RibbonDockFrame / CMainFrame / vHomeDlg / vLandDlg / ...   │
│   • Kullanıcı işlemleri (ekle, sil, güncelle)              │
│   • Giriş, çıkış, senkronizasyon durumu gösterimi          │
│   • Menüden “Bulut Yedekle / Geri Yükle” erişimi           │
└────────────────────────────────────────────────────────────┘
                │
                ▼
┌────────────────────────────────────────────────────────────┐
│               ⚙️  Uygulama Yönetim Katmanı                 │
│────────────────────────────────────────────────────────────│
│ DatabaseManager (Singleton)                                │
│   • Yerel Access bağlantı yönetimi                         │
│   • SQL sorgu yürütme (ADO / ADOX)                         │
│   • Hata yönetimi ve loglama                               │
│                                                            │
│ LoginManager                                               │
│   • Google OAuth 2.0 giriş işlemi                          │
│   • Access / Refresh token yönetimi                        │
│                                                            │
│ CloudSyncManager                                           │
│   • Dosya yedekleme (Google Drive / API)                   │
│   • Buluttan geri yükleme                                  │
│                                                            │
│ SyncManager + SyncBridge                                   │
│   • Yerel <-> Bulut verisini eşitleme                      │
│   • Çevrimdışı kuyruğa alma ve “retry-on-connect”          │
└────────────────────────────────────────────────────────────┘
                │
                ▼
┌────────────────────────────────────────────────────────────┐
│               🗄  Veri Katmanı (Çift Yapı)                 │
│────────────────────────────────────────────────────────────│
│ 1️⃣ Yerel Veritabanı (Access .accdb)                       │
│     • ADO / ADOX kullanımı                                 │
│     • AdoLocalAdapter sınıfı                               │
│     • Çevrimdışı cache olarak görev yapar                  │
│                                                            │
│ 2️⃣ Bulut Veritabanı (Web API / Firestore / REST)          │
│     • FirestoreProvider veya REST tabanlı API              │
│     • HTTPS ile erişim (httplib / cpr)                     │
│     • JSON tabanlı veri transferi                          │
└────────────────────────────────────────────────────────────┘
                │
                ▼
┌────────────────────────────────────────────────────────────┐
│                   🌐 Bulut Katmanı (Backend)               │
│────────────────────────────────────────────────────────────│
│  🔹 api.emlakcrm.com                                      │
│     • Web tabanlı veritabanı (MySQL / Firestore / SQL API) │
│     • CRUD endpoint’leri: /customers, /homes, /lands ...   │
│     • JSON REST API formatı                               │
│                                                            │
│  🔹 Google Drive (opsiyonel)                              │
│     • Access yedek dosyalarını (.accdb) depolar            │
│     • OAuth 2.0 doğrulama ile erişim                       │
└────────────────────────────────────────────────────────────┘




*/