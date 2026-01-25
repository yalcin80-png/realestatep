#include "stdafx.h"
#include "OptionsCatalog.h"
#include "CTreeListVDlg.h"  // TABLE_NAME_* sabitleri

using Win32xx::CString;

const OptionsCatalog::Entry& OptionsCatalog::Empty()
{
    static Entry e{};
    return e;
}

const std::map<std::pair<CString, CString>, OptionsCatalog::Entry>& OptionsCatalog::Catalog()
{
    static std::map<std::pair<CString, CString>, Entry> cat;
    static bool inited = false;
    if (inited) return cat;
    inited = true;

    auto key = [&](const CString& table, const CString& field) {
        return std::make_pair(table, field);
    };

    // ===== HOME: Resim 2-3-4 (Özellikler) =====
    cat[key(TABLE_NAME_HOME, _T("Facades"))] = { Kind::Multi, {
        _T("Kuzey"), _T("Güney"), _T("Doğu"), _T("Batı")
    }};

    cat[key(TABLE_NAME_HOME, _T("FeaturesInterior"))] = { Kind::Multi, {
        _T("ADSL"), _T("Akıllı Ev"), _T("Alarm (Hırsız)"), _T("Alarm (Yangın)"), _T("Alaturka Tuvalet"),
        _T("Ankastre Mutfak"), _T("Balkon"), _T("Barbekü"), _T("Beyaz Eşya"), _T("Boyalı"),
        _T("Bulaşık Makinesi"), _T("Duşakabin"), _T("Fiber İnternet"), _T("Görüntülü Diafon"), _T("Gömme Dolap"),
        _T("Hilton Banyo"), _T("Isıcam"), _T("Jakuzi"), _T("Kartonpiyer"), _T("Klima"),
        _T("Laminat Zemin"), _T("Mobilya"), _T("Mutfak Doğalgazı"), _T("Parke Zemin"), _T("PVC Doğrama"),
        _T("Seramik Zemin"), _T("Set Üstü Ocak"), _T("Şömine"), _T("Teras"), _T("Vestiyer"),
        _T("Wi-Fi"), _T("Çamaşır Makinesi"), _T("Çelik Kapı")
    }};

    cat[key(TABLE_NAME_HOME, _T("FeaturesExterior"))] = { Kind::Multi, {
        _T("Güvenlik"), _T("Isı Yalıtım"), _T("Kapalı Garaj"), _T("Açık Otopark"), _T("Jeneratör"),
        _T("Kapıcı"), _T("Kamera Sistemi"), _T("Oyun Parkı"), _T("Spor Alanı"), _T("Yüzme Havuzu"),
        _T("Sauna"), _T("Hamam"), _T("Basketbol Sahası"), _T("Tenis Kortu"), _T("Yangın Merdiveni")
    }};

    cat[key(TABLE_NAME_HOME, _T("FeaturesNeighborhood"))] = { Kind::Multi, {
        _T("Alışveriş Merkezi"), _T("Belediye"), _T("Cami"), _T("Eczane"), _T("Hastane"),
        _T("İlkokul-Ortaokul"), _T("Lise"), _T("Market"), _T("Park"), _T("Polis Merkezi"),
        _T("Spor Salonu"), _T("Üniversite"), _T("Denize Yakın"), _T("Şehir Merkezi")
    }};

    cat[key(TABLE_NAME_HOME, _T("FeaturesTransport"))] = { Kind::Multi, {
        _T("Anayol"), _T("Metro"), _T("Metrobüs"), _T("Otobüs"), _T("Tramvay"),
        _T("Minibüs / Dolmuş"), _T("Marmaray"), _T("Deniz Otobüsü"), _T("Havaalanı"), _T("E-5")
    }};

    cat[key(TABLE_NAME_HOME, _T("FeaturesView"))] = { Kind::Multi, {
        _T("Deniz"), _T("Göl"), _T("Dağ"), _T("Orman"), _T("Şehir"), _T("Park / Yeşil Alan")
    }};

    cat[key(TABLE_NAME_HOME, _T("HousingType"))] = { Kind::Single, {
        _T("Ara Kat"), _T("Bahçe Katı"), _T("Çatı Katı"), _T("Dubleks"), _T("Çatı Dubleksi"),
        _T("Tripleks"), _T("Müstakil"), _T("Loft"), _T("Rezidans"), _T("Köşe")
    }};

    cat[key(TABLE_NAME_HOME, _T("FeaturesAccessibility"))] = { Kind::Multi, {
        _T("Engelli Rampası"), _T("Asansör Erişimi"), _T("Geniş Kapı Girişi"), _T("Tekerlekli Sandalye Uygun"),
        _T("Yürüyüş Yolu"), _T("Yaşlı Dostu")
    }};

    return cat;
}

const OptionsCatalog::Entry& OptionsCatalog::Get(const CString& tableName, const CString& fieldName)
{
    const auto& cat = Catalog();
    auto it = cat.find(std::make_pair(tableName, fieldName));
    if (it == cat.end()) return Empty();
    return it->second;
}
