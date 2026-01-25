
#ifndef PREVIEWITEM_H
#define PREVIEWITEM_H






#pragma once
#include "stdafx.h"
#include <map>
#include <vector>

 //PreviewPanel ? ListingPageLayout arasý veri taþýyýcý
 //Müþteri + Mülk + Alanlar (DB’den gelen tüm field'lar)
 //1. EKLEDÝÐÝMÝZ KISIM: Belge Türleri Listesi
enum PreviewDocType {
    DOC_UNKNOWN= 0,
    DOC_LISTING_DETAILS ,    // Mevcut Ýlan Detayý
    DOC_CONTRACT_SALES_AUTH,    // Satýlýk Yetkilendirme Sözleþmesi
    DOC_CONTRACT_RENTAL_AUTH,   // Kiralýk Yetkilendirme (Ýleride eklersin)
    DOC_CONTRACT_RENTAL_OUTH,
    DOC_PENALTY_AGREEMENT,       // Cayma Akçesi (Ýleride eklersin)
    DOC_CONTRACT_DEPOSIT,        // <--- YENÝ: Cayma Akçesi / Alým-Satým
    DOC_SHOWING_AGREEMENT,  // <--- YENÝ: Yer Gösterme Belgesi
    DOC_WAIVER_PREEMPTION,
    DOC_RENTAL_CONTRACT, // <--- YENÝ: Kira Kontratý
    DOC_OFFER_LETTER, // <--- YENÝ: Teklif Mektubu
    DOC_APPOINTMENT_FORM  // <--- Randevu Formu Tipi

};






struct PreviewItem
{
    // 2. EKLEDÝÐÝMÝZ KISIM: Belgenin Türü (Varsayýlan = Ýlan)
    PreviewDocType docType = DOC_LISTING_DETAILS;

    // 3. EKLEDÝÐÝMÝZ KISIM: Pencere Baþlýðý
    CString title;

    // Mevcut Alanlarýnýz
    CString tableName;   // "HomeTbl" vb.
    CString recordCode;  // "HM001" vb.

    // 4. DEÐÝÞÝKLÝK ÖNERÝSÝ: std::wstring yerine CString kullanýn.
    // Neden? Çünkü DatabaseManager CString döndürüyor, ContractPageLayout CString bekliyor.
    // std::wstring kullanýrsanýz sürekli çeviri yapmanýz gerekir.
    std::vector<std::pair<CString, CString>> fields;

    // Notlar
    std::wstring notes;

    PreviewItem() {}
};


static const int pageV = 2100;
static const int pageH = 2970;

const double A4_W = 595;
const double A4_H = 842;




#endif // PREVIEWITEM_H