#pragma once
#include "PreviewItem.h"
#include "IDocumentLayout.h"
#include "StringHelpers.h" // GetExePath için
#include "GdiLocationFormLayout.h" // <--- YENİ İSİM BURADA
#include "GdiWaiverLayout.h"      // <--- YENİ EKLENEN BELGE TÜRÜ
// --- ESKİ RESSAMLAR (GDI - YEDEK) ---
#include "ContractPageLayout.h"
#include "ListingPageLayout.h"
#include "DepositPageLayout.h"
#include "GdiRentalLayout.h" // <--- EKLE
// --- YENİ MODERN RESSAM (JSON) ---
#include "JsonDocumentLayout.h" 
#include "GdiOfferLetterLayout.h" // <--- EKLE
#include "GdiRentalAuthLayout.h" // <--- EKLE
#include "GdiAppointmentLayout.h"
// Dosya kontrolü için Windows API
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")



/*
 GDI layout sınıfları (legacy / yedek)

ContractPageLayout (satılık yetki vb eski çizim)

ListingPageLayout (mülk bilgi / mülk detay eski çizim)

DepositPageLayout (depozito)

GdiRentalLayout (kira sözleşmesi)

GdiRentalAuthLayout (kiralık tek yetki yedek)

GdiLocationFormLayout (yer gösterme anlaşması yedek)

GdiWaiverLayout (önalım feragatname yedek)

GdiOfferLetterLayout (teklif mektubu yedek)

GdiAppointmentLayout (randevu formu yedek)


*/

















class LayoutFactory
{
public:
    static IDocumentLayout* CreateLayout(PreviewDocType type)
    {
        // Dosya var mı kontrolü yapan yardımcı lambda
        auto JsonExists = [](const CString& relativePath) -> bool {
            CString fullPath = FormEngine::GetExePath() + _T("\\") + relativePath;
            return ::PathFileExists(fullPath);
            };

        switch (type)
        {
            // 1. SENARYO: SATILIK YETKİ BELGESİ
        case DOC_CONTRACT_SALES_AUTH:
        {
            CString jsonFile = _T("templates\\satilik_yetki_sozlesmesi.json");

            if (JsonExists(jsonFile)) {
                // Şablon varsa JSON motorunu kullan
                return new JsonDocumentLayout(jsonFile);
            }
            else {
                // Şablon yoksa eski usül (GDI) devam et
                return new ContractPageLayout();
            }
        }
        case DOC_CONTRACT_RENTAL_OUTH: // Zaten enum'da vardı
        {
            CString jsonFile = _T("templates\\kiralik_yetki.json");

            if (JsonExists(jsonFile)) {
                return new JsonDocumentLayout(jsonFile);
            }
            else {
                return new GdiRentalAuthLayout(); // <--- YEDEK
            }
        }
        // 2. SENARYO: YER GÖSTERME FORMU (Yeni istediğin)
        case DOC_CONTRACT_RENTAL_AUTH: // Kiralık/Yer Gösterme tipi
        {
            CString jsonFile = _T("templates\\yer_gosterme.json");

            if (JsonExists(jsonFile)) {
                return new JsonDocumentLayout(jsonFile);
            }
            else {
                // Yedek olarak eski kiralık sözleşmesi veya boş bir GDI layout dönebilirsin
                return new ContractPageLayout(); // Veya RentalPageLayout
            }
        }

        case DOC_WAIVER_PREEMPTION: // <--- YENİ BELGE TÜRÜ
        {
            CString jsonFile = _T("templates\\onalim_feragatname.json");

            if (JsonExists(jsonFile)) {
                // 1. Öncelik: Modern JSON
                return new JsonDocumentLayout(jsonFile);
            }
            else {
                // 2. Öncelik: GDI Yedek
                return new GdiWaiverLayout();
            }
        }
        case DOC_OFFER_LETTER: // <--- YENİ CASE
        {
            CString jsonFile = _T("templates\\teklif_mektubu.json");

            if (JsonExists(jsonFile)) {
                return new JsonDocumentLayout(jsonFile);
            }
            else {
                return new GdiOfferLetterLayout(); // YEDEK
            }
        }

        case DOC_RENTAL_CONTRACT: // <--- YENİ CASE
        {
            CString jsonFile = _T("templates\\kira_sozlesmesi.json");

            if (JsonExists(jsonFile)) {
                return new JsonDocumentLayout(jsonFile);
            }
            else {
                return new GdiRentalLayout(); // <--- YEDEK
            }
        }
        case DOC_APPOINTMENT_FORM:
        {
            // Önce JSON şablonu var mı diye bak (Modern Yöntem)
            CString jsonFile = _T("templates\\randevu_formu.json");
            if (JsonExists(jsonFile)) {
                return new JsonDocumentLayout(jsonFile);
            }
            // Yoksa GDI kodunu kullan
            else {
                return new GdiAppointmentLayout();
            }
        }
        case DOC_SHOWING_AGREEMENT:
        {
            CString jsonFile = _T("templates\\yer_gosterme.json");

            if (JsonExists(jsonFile)) {
                return new JsonDocumentLayout(jsonFile);
            }
            else {
                // Yedek olarak yeni isimli sınıfı kullanıyoruz
                return new GdiLocationFormLayout();
            }
        }
        case DOC_CONTRACT_DEPOSIT:
            return new DepositPageLayout();

        case DOC_LISTING_DETAILS:
        default:
            // Mülk Detay formu için de JSON yapmıştık, onu da bağlayalım
            if (JsonExists(_T("templates\\emlak_bilgi_formu.json"))) {
                return new JsonDocumentLayout(_T("templates\\emlak_bilgi_formu.json"));
            }
            else {
                return new ListingPageLayout();
            }
        }
    }
};