# GdiRentalAuthLayout Field Mapping

This document lists all the data fields that the `GdiRentalAuthLayout` expects in the `SetData()` method. These fields are used to populate the 3-page Kiralık Tek Yetki Sözleşmesi (Rental Authorization Agreement) form.

## Page 1 Fields - İş Sahibi, Emlak İşletmesi, Gayrimenkul Bilgileri

### İş Sahibi Bilgileri (Owner Information)
- `OwnerName` - Adı Soyadı (Full Name)
- `OwnerTC` - T.C./Kimlik No. (Turkish ID Number)
- `OwnerPhone` - Telefon (Phone Number)
- `OwnerEmail` - E-Posta (Email Address)
- `OwnerAddress` - Adres (Address)

### Emlak İşletmesi Bilgileri (Real Estate Agency Information)
- `AgentName` - Yetkili Kişi (Authorized Person)
- `AgentTitle` - Unvanı (Title)
- `AgentPhone` - Telefon (Phone Number)
- `AgentEmail` - E-Posta (Email Address)
- `AgentAddress` - Adres (Address)

### Gayrimenkul Tapu Kaydi (Property Deed Registration)
- `City` - İli (City)
- `District` - İlçesi (District)
- `Neighborhood` - Mahallesi (Neighborhood)
- `Pafta` - Pafta (Map Sheet)
- `Ada` - Ada (Island/Block)
- `Parsel` - Parsel (Parcel)
- `Floor` - Kat No. (Floor Number)
- `BuildingNo` - B.B. No. (Building Number)
- `LandShare` - Arsa Payı (Land Share)

### Gayrimenkul Adresi (Property Address)
- `Owners` - Malikler (Owners)
- `FullAddress` - Adres (Full Address)

### İmar Durumu (Zoning Status)
- `ZoningArea` - Bölge/Alan (Zone/Area)
- `ZoningStatus` - İmar Durumu (Zoning Status)

## Page 2 Fields - Yapı Özellikleri

### Yapıya Ait Bilgiler (Building Information)
- `LandArea` - Taşınmazın Alanı (M²) (Land Area in Square Meters)
- `BuildingAge` - Yaşı (Age)
- `ActualUse` - Fiili Kullanım Durumu (Actual Use Status)
- `ConstructionStatus` - İnşaatın Durumu (Construction Status)
- `GrossArea` - Brüt Alan (M²) (Gross Area)
- `NetArea` - Net Alan (M²) (Net Area)
- `NaturalGasArea` - Doğalgaz Alanı (Natural Gas Area)

### İletişim Olanakları (Transportation/Access)
- `PublicTransport` - Toplu Taşıma (Public Transportation)
- `RailSystems` - Raylı Sistemler (Rail Systems)
- `SeaTransport` - Deniz Ulaşımı (Sea Transportation)

### Yapı Bölüm Alanları (Building Areas)
- `LivingRoom` - Salon (Living Room)
- `Kitchen` - Mutfak (Kitchen)
- `Bathroom` - Banyo (Bathroom)
- `WC` - WC (Toilet)
- `BedroomCount` - Yatak Odası Sayısı (Number of Bedrooms)

## Page 3 Fields - Hizmet ve İmza

### Açıklamalar (Descriptions)
- `PropertyType` - Gayrimenkulün Cinsi (Property Type)
- `OccupancyStatus` - Konut İskan Durumu (Occupancy Status)

### Kiralama Bedeli (Rental Amount)
- `RentAmountNumeric` - Rakam İle (In Numbers)
- `RentAmountText` - Yazı İle (In Words)

## Default Values

If any field is not provided in the data, the system will display `....................` as a placeholder.

## Usage Example

```cpp
std::vector<std::pair<CString, CString>> fields;

// İş Sahibi Bilgileri
fields.push_back({_T("OwnerName"), _T("Mehmet Yılmaz")});
fields.push_back({_T("OwnerTC"), _T("12345678901")});
fields.push_back({_T("OwnerPhone"), _T("0532 123 45 67")});
fields.push_back({_T("OwnerEmail"), _T("mehmet@example.com")});
fields.push_back({_T("OwnerAddress"), _T("Atatürk Caddesi No:123 Ankara")});

// Emlak İşletmesi Bilgileri
fields.push_back({_T("AgentName"), _T("Ahmet Demir")});
fields.push_back({_T("AgentTitle"), _T("Emlak Danışmanı")});
fields.push_back({_T("AgentPhone"), _T("0312 456 78 90")});
fields.push_back({_T("AgentEmail"), _T("info@emlak.com")});
fields.push_back({_T("AgentAddress"), _T("Kızılay Mah. İzmir Cad. No:45 Çankaya/Ankara")});

// Gayrimenkul Bilgileri
fields.push_back({_T("City"), _T("Ankara")});
fields.push_back({_T("District"), _T("Çankaya")});
fields.push_back({_T("Neighborhood"), _T("Kızılay")});
fields.push_back({_T("FullAddress"), _T("Kızılay Mahallesi, Atatürk Bulvarı No:100 Daire:5")});
// ... add more fields as needed

GdiRentalAuthLayout layout;
layout.SetData(fields);
```

## Notes

- All text fields support Turkish characters (ç, ğ, ı, ö, ş, ü)
- Field values are displayed in blue (RGB 0, 0, 150) color
- Section headers are displayed in red (RGB 200, 0, 0) color
- The form uses Arial font with sizes ranging from 8pt to 20pt
- The form size is A4 (2100 x 2970 logical units)
