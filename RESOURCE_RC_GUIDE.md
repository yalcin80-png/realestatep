# Resource.rc Dialog Layout Guide

Bu dosya, Visual Studio Resource Editor kullanılarak Resource.rc dosyasına eklenecek kontrolleri detaylı olarak açıklar.

## 📋 Home Dialog (IDD_HOME_DIALOG - 2000)

### Önerilen Yerleşim

Yeni bir Tab sayfası olarak veya mevcut "Genel" tab'ının altına eklenebilir.

#### Örnek Koordinatlar (Dialog'un boyutuna göre ayarlanmalı)

```rc
// Oda Yönetimi Grubu
GROUPBOX        "Oda Yönetimi",IDC_STATIC,7,300,400,180

// ListView (Oda Listesi)
CONTROL         "",IDC_LISTVIEW_ROOMS_HOME,WC_LISTVIEW,LVS_REPORT | LVS_SINGLESEL | WS_BORDER | WS_TABSTOP,
                12,315,390,110

// Oda Bilgileri Girişi
LTEXT           "Oda Adı:",IDC_STATIC,12,432,40,8
EDITTEXT        IDC_EDIT_ROOM_NAME_HOME,55,430,120,14,ES_AUTOHSCROLL

LTEXT           "Alan (m²):",IDC_STATIC,180,432,40,8
EDITTEXT        IDC_EDIT_ROOM_AREA_HOME,225,430,60,14,ES_AUTOHSCROLL

CONTROL         "Duş",IDC_CHECK_ROOM_SHOWER_HOME,"Button",BS_AUTOCHECKBOX | WS_TABSTOP,
                290,430,40,14

CONTROL         "Lavabo",IDC_CHECK_ROOM_SINK_HOME,"Button",BS_AUTOCHECKBOX | WS_TABSTOP,
                335,430,60,14

// Butonlar
PUSHBUTTON      "Ekle",IDC_BTN_ADD_ROOM_HOME,12,452,50,14
PUSHBUTTON      "Sil",IDC_BTN_REMOVE_ROOM_HOME,67,452,50,14
```

### Visual Studio'da Adım Adım

1. **Resource.rc'yi aç**
   - Solution Explorer → Resource Files → Resource.rc
   - Dialog klasörü → IDD_HOME_DIALOG

2. **GroupBox ekle**
   - Toolbox → GroupBox
   - Dialog üzerine sürükle
   - Properties: Text = "Oda Yönetimi"

3. **ListView ekle**
   - Toolbox → List Control
   - GroupBox içine yerleştir
   - Properties:
     - ID: IDC_LISTVIEW_ROOMS_HOME
     - View: Report
     - Single Selection: True
     - Border: True

4. **Label ve TextBox'lar**
   - Static Text: "Oda Adı:" ve "Alan (m²):"
   - Edit Control: IDC_EDIT_ROOM_NAME_HOME
   - Edit Control: IDC_EDIT_ROOM_AREA_HOME

5. **Checkbox'lar**
   - Check Box: IDC_CHECK_ROOM_SHOWER_HOME (Text: "Duş")
   - Check Box: IDC_CHECK_ROOM_SINK_HOME (Text: "Lavabo")

6. **Butonlar**
   - Button: IDC_BTN_ADD_ROOM_HOME (Text: "Ekle")
   - Button: IDC_BTN_REMOVE_ROOM_HOME (Text: "Sil")

## 📋 Villa Dialog - Genel Page (IDD_VILLA_PAGE_GENEL - 8301)

### Örnek Koordinatlar

```rc
// Oda Yönetimi Grubu (Villa)
GROUPBOX        "Oda Yönetimi",IDC_STATIC,7,180,400,180

// ListView (Oda Listesi)
CONTROL         "",IDC_LISTVIEW_ROOMS_VILLA,WC_LISTVIEW,LVS_REPORT | LVS_SINGLESEL | WS_BORDER | WS_TABSTOP,
                12,195,390,110

// Oda Bilgileri Girişi
LTEXT           "Oda Adı:",IDC_STATIC,12,312,40,8
EDITTEXT        IDC_EDIT_ROOM_NAME_VILLA,55,310,120,14,ES_AUTOHSCROLL

LTEXT           "Alan (m²):",IDC_STATIC,180,312,40,8
EDITTEXT        IDC_EDIT_ROOM_AREA_VILLA,225,310,60,14,ES_AUTOHSCROLL

CONTROL         "Duş",IDC_CHECK_ROOM_SHOWER_VILLA,"Button",BS_AUTOCHECKBOX | WS_TABSTOP,
                290,310,40,14

CONTROL         "Lavabo",IDC_CHECK_ROOM_SINK_VILLA,"Button",BS_AUTOCHECKBOX | WS_TABSTOP,
                335,310,60,14

// Butonlar
PUSHBUTTON      "Ekle",IDC_BTN_ADD_ROOM_VILLA,12,332,50,14
PUSHBUTTON      "Sil",IDC_BTN_REMOVE_ROOM_VILLA,67,332,50,14
```

### Visual Studio'da Adım Adım

1. **IDD_VILLA_PAGE_GENEL'i aç**
   - Resource.rc → Dialog → IDD_VILLA_PAGE_GENEL

2. **Yukarıdaki adımları tekrarla**
   - Villa için ID'ler farklı (43101-43107)
   - Yerleşim Genel page boyutuna göre ayarlanmalı

## 🎨 Görünüm Önerileri

### ListView Kolonları (Kod tarafından eklenir, RC'de ayar gerekmez)
```
Kolon 1: "Oda Adı"     - 120 pixel
Kolon 2: "Alan (m²)"   - 80 pixel
Kolon 3: "Duş"         - 50 pixel
Kolon 4: "Lavabo"      - 60 pixel
```

### Önerilen Boyutlar
```
GroupBox: 400x180 pixel
ListView: 390x110 pixel
TextBox: 120x14 / 60x14 pixel
CheckBox: 40x14 / 60x14 pixel
Button: 50x14 pixel
```

## 🔧 Alternatif Yaklaşımlar

### Yaklaşım 1: Yeni Tab Sayfası
Home ve Villa için "Oda Detayları" adında yeni bir tab sayfası:
-장점: Daha temiz görünüm
- Dezavantaj: Ekstra tab sayfası

### Yaklaşım 2: Mevcut Tab'a Ekleme
"Genel" tab'ının altına yerleştirme:
-장점: Tüm temel bilgiler bir yerde
- Dezavantaj: Dialog daha uzun olabilir

### Yaklaşım 3: Açılır Panel
Expand/Collapse özellikli panel:
-장점: Alan tasarrufu
- Dezavantaj: Daha karmaşık implementasyon

## ✅ Kontrol Listesi

RC güncellemesi yapılırken kontrol edilecekler:

- [ ] Tüm ID'ler doğru (resource.h'deki ile eşleşiyor)
- [ ] ListView LVS_REPORT stilinde
- [ ] ListView LVS_SINGLESEL seçili
- [ ] TextBox'lar ES_AUTOHSCROLL stilinde
- [ ] CheckBox'lar BS_AUTOCHECKBOX stilinde
- [ ] Tab order doğru (Tab tuşu ile gezinme)
- [ ] Butonlar erişilebilir konumda
- [ ] Yazı tipleri tutarlı
- [ ] Yerleşim responsive (dialog büyütülünce)

## 📱 Test Senaryosu

RC güncellemesi sonrası test:

1. **Görsel Test:**
   - Dialog açılıyor mu?
   - Kontroller görünür mü?
   - Yerleşim düzgün mü?

2. **Fonksiyon Test:**
   - ListView gösteriliyor mu?
   - TextBox'lara yazılıyor mu?
   - CheckBox'lar çalışıyor mu?
   - Butonlar tıklanabiliyor mu?

3. **Data Flow Test:**
   - Oda eklenince ListView'de görünüyor mu?
   - Kaydet/Yükle çalışıyor mu?

## 🎯 Sonuç

Resource.rc güncellendikten sonra:
1. Projeyi derle
2. Dialog'u aç ve kontrolleri test et
3. Oda ekle/sil fonksiyonlarını test et
4. Database'e kaydet ve tekrar aç
5. JSON formatını kontrol et

**Güncelleme tamamlandığında sistem tamamen çalışır halde olacak! 🎉**
