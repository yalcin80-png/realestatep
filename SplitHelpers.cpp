
#include "stdafx.h"
#include "SplitHelpers.h"

static bool IsCustomerKey(const std::wstring& k)
{
    return
        k.find(L"Ad") != std::wstring::npos ||
        k.find(L"Soyad") != std::wstring::npos ||
        k.find(L"Name") != std::wstring::npos ||
        k.find(L"Phone") != std::wstring::npos ||
        k.find(L"TCKN") != std::wstring::npos;
}

static bool IsNoteKey(const std::wstring& k)
{
    return
        k.find(L"Acik") != std::wstring::npos ||
        k.find(L"Note") != std::wstring::npos ||
        k.find(L"Description") != std::wstring::npos;
}

// SplitHelpers.cpp (veya ListingPageLayout.cpp dosyasýnda)
void SplitDataToSections(
    const CString& table,
    const std::vector<std::pair<std::wstring, std::wstring>>& fields,

    std::vector<std::pair<std::wstring, std::wstring>>& customer,
    std::vector<std::pair<std::wstring, std::wstring>>& property,
    std::wstring& notes
)
{
    customer.clear();
    property.clear();
    notes.clear();

    // Notlarýn ve özelliklerin ayrýmý için yardýmcý fonksiyonlarý kullan

    // ?? GÜVENLÝK DÜZELTMESÝ: const auto& kullanýlmalý.
    for (const auto& item : fields)
    {
        const auto& k = item.first;
        const auto& v = item.second;

        // 1. Müþteri Alanlarý (Customer Keys)
        if (IsCustomerKey(k))
        {
            customer.push_back(item);
            continue;
        }

        // 2. Not Alanlarý (Note Keys) - Müþteri alaný deðilse ve not ise
        // Bu notlar, mülk detaylarýndan gelen genel/iç notlardýr (NoteGeneral, NoteInternal)
        if (IsNoteKey(k))
        {
            // Yalnýzca dolu notlarý birleþtir
            if (!v.empty())
            {
                // Not baþlýðýný (k) + deðeri (v) ekle, ardýndan satýr atla
                notes += k + L": " + v + L"\n";
            }
            continue;
        }

        // 3. Mülk Detaylarý (Geriye Kalan Her Þey)
        // Eðer ne müþteri alaný ne de not alaný ise, mülk detayýna aittir.
        property.push_back(item);
    }
}
CString GuessTableFromFields(
    const std::vector<std::pair<std::wstring,std::wstring>>& fields
)
{
    for (auto& p : fields)
    {
        if (p.first.find(L"Home_") != std::wstring::npos)
            return L"HomeTbl";
        if (p.first.find(L"Land_") != std::wstring::npos)
            return L"LandTbl";
        if (p.first.find(L"Field_") != std::wstring::npos)
            return L"FieldTbl";
        if (p.first.find(L"Villa_") != std::wstring::npos)
            return L"VillaTbl";
    }

    return L"Unknown";
}
