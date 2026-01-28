# Dinamik Oda Yönetimi - İmplementasyon Kılavuzu

## ✅ Tamamlanan Aşamalar

### 1. Database Layer (dataIsMe.h/cpp)
- ✅ `Home_cstr` ve `Villa_cstr` struct'larına `RoomDetails` CString alanı eklendi
- ✅ `RoomInfo` struct tanımlandı (name, area, hasShower, hasSink)
- ✅ JSON helper fonksiyonları eklendi:
  - `RoomsToJson()` - Oda listesini JSON string'e dönüştürür
  - `JsonToRooms()` - JSON string'i oda listesine dönüştürür
- ✅ `GetProperties<Home_cstr>()` ve `GetProperties<Villa_cstr>()` içine RoomDetails eklendi (FT_MEMO)

### 2. Resource IDs (resource.h)
✅ Aşağıdaki kontrol ID'leri eklendi:

**Home Dialog:**
- `IDC_LISTVIEW_ROOMS_HOME` (43001) - Oda listesi
- `IDC_BTN_ADD_ROOM_HOME` (43002) - Oda ekle butonu
- `IDC_BTN_REMOVE_ROOM_HOME` (43003) - Oda sil butonu
- `IDC_EDIT_ROOM_NAME_HOME` (43004) - Oda adı textbox
- `IDC_EDIT_ROOM_AREA_HOME` (43005) - Alan (m²) textbox
- `IDC_CHECK_ROOM_SHOWER_HOME` (43006) - Duş checkbox
- `IDC_CHECK_ROOM_SINK_HOME` (43007) - Lavabo checkbox

**Villa Dialog:**
- `IDC_LISTVIEW_ROOMS_VILLA` (43101) - Oda listesi
- `IDC_BTN_ADD_ROOM_VILLA` (43102) - Oda ekle butonu
- `IDC_BTN_REMOVE_ROOM_VILLA` (43103) - Oda sil butonu
- `IDC_EDIT_ROOM_NAME_VILLA` (43104) - Oda adı textbox
- `IDC_EDIT_ROOM_AREA_VILLA` (43105) - Alan (m²) textbox
- `IDC_CHECK_ROOM_SHOWER_VILLA` (43106) - Duş checkbox
- `IDC_CHECK_ROOM_SINK_VILLA` (43107) - Lavabo checkbox

### 3. Home Dialog Implementation (vHomeDlg.h/cpp)
✅ Fonksiyonlar eklendi:
- `InitRoomControls()` - ListView kolonlarını hazırla
- `LoadRoomsFromJson()` - JSON'dan oda listesini yükle
- `SaveRoomsToJson()` - Oda listesini JSON'a çevir
- `RefreshRoomListView()` - ListView'i güncelle
- `OnAddRoom()` - Yeni oda ekle
- `OnRemoveRoom()` - Seçili odayı sil

✅ Entegrasyon:
- `OnInitDialog()` içinde `InitRoomControls()` çağrısı eklendi
- Düzenleme modunda `LoadRoomsFromJson()` ile veri yükleme eklendi
- `OnOK()` içinde `SaveRoomsToJson()` ile veri kaydetme eklendi
- `DialogProc()` içinde buton click handler'ları eklendi

### 4. Villa Dialog Implementation (vVillaDlg.h/cpp)
✅ Fonksiyonlar eklendi:
- `InitRoomControls()` - ListView kolonlarını hazırla
- `LoadRoomsFromJson()` - JSON'dan oda listesini yükle (placeholder)
- `SaveRoomsToJson()` - Oda listesini JSON'a çevir
- `RefreshRoomListView()` - ListView'i güncelle
- `OnAddRoom()` - Yeni oda ekle
- `OnRemoveRoom()` - Seçili odayı sil

✅ Entegrasyon:
- `OnInitDialog()` içinde `InitRoomControls()` çağrısı eklendi
- `LoadFromDB()` içinde `LoadRoomsFromJson()` ile veri yükleme eklendi
- `OnOK()` içinde `SaveRoomsToJson()` ile veri kaydetme eklendi
- `DialogProc()` içinde buton click handler'ları eklendi

## ⚠️ Manuel İşlem Gereken Aşamalar

### 5. Resource.rc Dialog Layout Güncellemesi

**NOT:** Bu adım Windows Resource Editor (Visual Studio) ile yapılmalıdır.

#### Home Dialog (IDD_HOME_DIALOG - 2000)
Yeni bir tab sayfası veya ana dialogda uygun bir yere aşağıdaki kontroller eklenmelidir:

```
GROUPBOX "Oda Yönetimi", IDC_STATIC, x, y, w, h
CONTROL "", IDC_LISTVIEW_ROOMS_HOME, WC_LISTVIEW, 
        LVS_REPORT | LVS_SINGLESEL | WS_BORDER | WS_TABSTOP,
        x+5, y+15, w-10, 120

LTEXT "Oda Adı:", IDC_STATIC, x+5, y+140, 50, 10
EDITTEXT IDC_EDIT_ROOM_NAME_HOME, x+60, y+138, 100, 14, ES_AUTOHSCROLL

LTEXT "Alan (m²):", IDC_STATIC, x+165, y+140, 45, 10
EDITTEXT IDC_EDIT_ROOM_AREA_HOME, x+215, y+138, 60, 14, ES_AUTOHSCROLL

CONTROL "Duş", IDC_CHECK_ROOM_SHOWER_HOME, "Button", BS_AUTOCHECKBOX | WS_TABSTOP,
        x+280, y+138, 40, 14

CONTROL "Lavabo", IDC_CHECK_ROOM_SINK_HOME, "Button", BS_AUTOCHECKBOX | WS_TABSTOP,
        x+325, y+138, 50, 14

PUSHBUTTON "Ekle", IDC_BTN_ADD_ROOM_HOME, x+5, y+158, 50, 14
PUSHBUTTON "Sil", IDC_BTN_REMOVE_ROOM_HOME, x+60, y+158, 50, 14
```

**Önerilen Yerleşim:**
- Oda yönetimi kontrollerini yeni bir Tab sayfası (örn: "Oda Detayları") olarak ekleyin
- Veya mevcut "Genel" tab'ının altına yerleştirin

#### Villa Dialog - Genel Page (IDD_VILLA_PAGE_GENEL - 8301)
Villa için oda yönetimi kontrollerini "Genel" tab sayfasına eklenmeli:

```
GROUPBOX "Oda Yönetimi", IDC_STATIC, x, y, w, h
CONTROL "", IDC_LISTVIEW_ROOMS_VILLA, WC_LISTVIEW,
        LVS_REPORT | LVS_SINGLESEL | WS_BORDER | WS_TABSTOP,
        x+5, y+15, w-10, 120

LTEXT "Oda Adı:", IDC_STATIC, x+5, y+140, 50, 10
EDITTEXT IDC_EDIT_ROOM_NAME_VILLA, x+60, y+138, 100, 14, ES_AUTOHSCROLL

LTEXT "Alan (m²):", IDC_STATIC, x+165, y+140, 45, 10
EDITTEXT IDC_EDIT_ROOM_AREA_VILLA, x+215, y+138, 60, 14, ES_AUTOHSCROLL

CONTROL "Duş", IDC_CHECK_ROOM_SHOWER_VILLA, "Button", BS_AUTOCHECKBOX | WS_TABSTOP,
        x+280, y+138, 40, 14

CONTROL "Lavabo", IDC_CHECK_ROOM_SINK_VILLA, "Button", BS_AUTOCHECKBOX | WS_TABSTOP,
        x+325, y+138, 50, 14

PUSHBUTTON "Ekle", IDC_BTN_ADD_ROOM_VILLA, x+5, y+158, 50, 14
PUSHBUTTON "Sil", IDC_BTN_REMOVE_ROOM_VILLA, x+60, y+158, 50, 14
```

### 6. GDI Layout Entegrasyonu (Opsiyonel)

Mülk bilgileri formlarında (PDF/baskı çıktılarında) oda detaylarını göstermek için:

**İlgili Dosyalar:**
- `GdiLocationFormLayout.cpp`
- `JsonDocumentLayout.cpp`
- İlgili layout sınıfları

**Eklenecek Kod Örneği:**
```cpp
// Oda detaylarını göster
if (!propertyData.RoomDetails.IsEmpty()) {
    std::vector<RoomInfo> rooms = JsonToRooms(propertyData.RoomDetails);
    
    for (const auto& room : rooms) {
        // Her oda için bilgileri yazdır
        CString roomInfo;
        roomInfo.Format(_T("%s - %.2f m² %s%s"),
            room.name,
            room.area,
            room.hasShower ? _T("[Duş] ") : _T(""),
            room.hasSink ? _T("[Lavabo]") : _T(""));
        
        // GDI ile yazdır
        // DrawText(...);
    }
}
```

## 🔄 JSON Format

Oda detayları aşağıdaki JSON formatında saklanır:

```json
[
  {
    "name": "Oda 1",
    "area": 15.5,
    "hasShower": true,
    "hasSink": true
  },
  {
    "name": "Salon",
    "area": 30.0,
    "hasShower": false,
    "hasSink": false
  },
  {
    "name": "Mutfak",
    "area": 12.5,
    "hasShower": false,
    "hasSink": true
  }
]
```

## 🧪 Test Senaryoları

### Manuel Test Adımları:

1. **Yeni Ev/Daire Ekleme:**
   - Yeni Home kaydı oluştur
   - Oda Yönetimi bölümünde "Oda 1", 15 m², Duş ✓, Lavabo ✓ ekle
   - "Salon", 30 m², Duş ✗, Lavabo ✗ ekle
   - Kaydet
   - Kaydı tekrar aç ve odaların yüklendiğini doğrula

2. **Mevcut Ev/Daire Düzenleme:**
   - Var olan Home kaydını aç
   - Yeni oda ekle
   - Var olan bir odayı sil
   - Kaydet ve tekrar açarak değişiklikleri doğrula

3. **Villa İçin Aynı Testler:**
   - Yeni Villa kaydı oluştur ve oda ekle
   - Mevcut Villa'yı düzenle

4. **JSON Doğrulama:**
   - Database'de RoomDetails sütununu kontrol et
   - JSON formatının doğru olduğunu doğrula

## 📝 Notlar

- Database şeması otomatik olarak `GetProperties` template'inden üretilir
- `RoomDetails` MEMO tipinde bir kolon olarak database'e eklenir
- JSON parsing için `nlohmann::json` kütüphanesi kullanılır (zaten projede mevcut)
- Villa için `LoadRoomsFromJson()` basitleştirilmiş implementasyon içerir; gerekirse Home versiyonu ile aynı hale getirilebilir

## 🔧 Derleme ve Çalıştırma

Kod değişiklikleri tamamlandı. Sadece Resource.rc güncellemesi manual olarak yapılmalıdır.

1. Visual Studio'da Resource.rc dosyasını aç
2. Yukarıdaki kontrolleri ilgili dialog'lara ekle
3. Projeyi derle
4. Test senaryolarını çalıştır
