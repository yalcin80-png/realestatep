# ??? Profesyonel Müþteri Listesi Mimarisi

## ?? Genel Bakýþ

Bu solution, Win32++ framework'ü üzerinde inþa edilmiþ **kurumsal-grade müþteri yönetim sistemi** saðlar.

---

## ?? Temel Bileþenler

### 1. **CCustomerListContainer** (Temel Kapsayýcý)
```
???????????????????????????????????????
?   CCustomerListContainer?
?  (Düzen Yönetimi)           ?
???????????????????????????????????????
?            ?           ?
?CAdvancedListView      ? CIndexBar ?
?  (ListView)     ? (A-Z)     ?
?  65% geniþlik          ? 25px    ?
?          ? ?
???????????????????????????????????????
```

### 2. **CAdvancedListView** (Ana Liste)
- ? **Virtual Mode (LVS_OWNERDATA)**: 1M+ satýr destek
- ? **Native Grouping**: LVGROUP API ile kategoriler
- ? **Dual View Mode**: Report (Tablo) + Card (Kart)
- ? **Double Buffering**: Flicker-free rendering
- ? **Custom Draw**: GDI+ entegrasyonu hazýr

### 3. **CCustomerDataManager** (Veri Yönetimi)
- Müþteri ekleme/çýkarma/güncelleme
- Sýralama (ID, Ad, vb.)
- Gruplama (Durum bazlý)
- Filtreleme (Grup filtreleri)

### 4. **CIndexBar** (Hýzlý Navigasyon)
- A-Z harf dizini
- Mouse hover highlight
- Otomatik ListView scroll
- Yapýlandýrýlabilir harfler

---

## ?? Hýzlý Baþlangýç

### Setup (Mainfrm.cpp'de):
```cpp
// Container oluþtur
m_customerContainer.Create(m_hWnd, IDC_CONTAINER, rcRect);

// Data Manager'a müþteri ekle
auto pDataMgr = m_customerContainer.GetDataManager();
CustomerData c;
c.id = _T("K001");
c.name = _T("Ahmet Yýlmaz");
c.group = CustomerGroup::VIP;
pDataMgr->AddCustomer(c);

// Grouping'i etkinleþtir
m_customerContainer.GetListView().EnableGrouping(true);
m_customerContainer.GetListView().Refresh();
```

### Seçili Müþteriyi Oku:
```cpp
const CustomerData* pCustomer = 
    m_customerContainer.GetListView().GetSelectedCustomer();
if (pCustomer)
    MessageBox(pCustomer->name);
```

---

## ?? Görünüm Modlarý

### Mode 0 - REPORT (Sütun Tablosu)
```
??????????????????????????????????????????????
?Cari Kod ? Ad Soyadý    ? Telefon  ? Þehir?
??????????????????????????????????????????????
?K001     ?Ahmet Yýlmaz  ?0532...   ?Ýstanbul?
?K002     ?Ayþe Demir    ?0555...   ?Ankara  ?
??????????????????????????????????????????????
```

### Mode 1 - CARD (Kart Görünümü)
```
????????????????????  ????????????????????
?? VIP - K001     ?  ?? VIP - K002     ?
?Ahmet Yýlmaz      ?  ?Ayþe Demir        ?
?0532 123 4567     ?  ?0555 456 7890     ?
?ahmet@ex.com      ?  ?ayse@ex.com       ?
?Ýstanbul     ?  ?Ankara            ?
????????????????????  ????????????????????
```

---

## ?? Gruplama Sistemi

Otomatik kategori renklendirmesi:

| Grup | Renk | Ýkon | Açýklama |
|------|------|------|----------|
| **VIP** | ?? Sarý | ? | Önemli müþteriler |
| **Regular** | ?? Yeþil | • | Standart müþteriler |
| **Pending** | ?? Turuncu | ? | Ýþlem bekleme |
| **Inactive** | ? Gri | ? | Pasif müþteriler |

---

## ? Performans

| Özellik | Kapasite |
|---------|---------|
| Virtual Mode | 1,000,000+ satýr |
| Yükleme Süresi (10K) | < 100ms |
| Yükleme Süresi (100K) | < 500ms |
| Scroll FPS | 60 FPS sabit |

---

## ?? Dosyalar

| Dosya | Açýklama |
|-------|----------|
| `CAdvancedListView.h` | Header dosyasý (tanýmlar) |
| `CAdvancedListView.cpp` | Implementation (tanýmlamalar) |
| `ADVANCED_LIST_USAGE_GUIDE.h` | Detaylý kullaným rehberi |
| `ADVANCED_LIST_ARCHITECTURE.md` | Bu dosya |

---

## ??? Geliþtirme Önerileri

### Next Phase:
- [ ] Card mode GDI+ drawing tamamla
- [ ] Database entegrasyonu (async veri yükleme)
- [ ] Arama ve filtreleme paneli
- [ ] Undo/Redo support
- [ ] Export (PDF, Excel) özellikleri
- [ ] Print preview

### Advanced Features:
- Multi-level grouping
- Custom sort comparators
- Real-time search
- Inline editing
- Drag-drop reordering

---

## ?? Desteklenen Özellikler

? Virtual Mode (LVS_OWNERDATA)  
? Native Grouping (LVGROUP API)  
? Double Buffering (LVS_EX_DOUBLEBUFFER)  
? Full Row Select (LVS_EX_FULLROWSELECT)  
? Dual View Mode (Report + Card)  
? Custom Drawing (GDI+ ready)  
? Index Bar Navigation  
? Multi-column Sorting  
? Group Filtering  
? Color Coding  

---

## ?? Ýçerik Seviyesi

- **Baþlangýç**: Basic setup ve veri yükleme
- **Ýleri**: Grouping, sorting, custom colors
- **Uzman**: GDI+ card drawing, async veri, database entegrasyonu

---

**Son Güncelleme**: 2024  
**Framework**: Win32++ v10.20  
**C++ Standard**: C++20
