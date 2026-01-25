#include "stdafx.h"
#include "DocumentDataManager.h"
#include <ctime>
#include <iomanip>
#include <sstream>

PreviewItem DocumentDataManager::PrepareDocument(PreviewDocType docType, const CString& tableName, const CString& recordCode)
{
    PreviewItem item;
    item.docType = docType;
    item.tableName = tableName;
    item.recordCode = recordCode;

    // 1. BELGE BAŢLIĐINI BELÝRLE
    switch (docType)
    {
    case DOC_CONTRACT_SALES_AUTH:  item.title = _T("SATILIK TEK YETKÝ SÖZLEŢMESÝ"); break;
    case DOC_CONTRACT_RENTAL_OUTH: item.title = _T("KÝRALIK TEK YETKÝ SÖZLEŢMESÝ"); break; // <-- buraya taţý
    case DOC_CONTRACT_RENTAL_AUTH: item.title = _T("YER GÖSTERME FORMU"); break;
    case DOC_CONTRACT_DEPOSIT:     item.title = _T("ALIM SATIM VE KAPORA SÖZLEŢMESÝ"); break;
    case DOC_LISTING_DETAILS:         item.title = _T("EMLAK GÖSTERME VE BÝLGÝ FORMU"); break;
    default: item.title = _T("GAYRÝMENKUL DETAY FORMU"); break;
    }

    // 2. SÝSTEM TARÝHÝ
    item.fields.push_back({ _T("CurrentDate"), GetDateNow() });

    // 3. EMLAK OFÝSÝ BÝLGÝLERÝ (Sabit)
    LoadAgentInfo(item);

    // 4. MÜLK BÝLGÝLERÝ (DB'den çekilir, Cari Kod döner)
    CString cariKod = LoadPropertyInfo(item, tableName, recordCode);

    // 5. MÜLK SAHÝBÝ BÝLGÝLERÝ
    if (!cariKod.IsEmpty()) {
        LoadOwnerInfo(item, cariKod);
    }
    else {
        item.fields.push_back({ _T("OwnerName"), _T("Kayýtlý Mülk Sahibi Yok") });
    }

    return item;
}

void DocumentDataManager::LoadAgentInfo(PreviewItem& item)
{
    // Buradaki bilgiler firmanýza göre sabitlenmiţtir.
    // Ýleride "Settings" tablosundan çekilebilir.
    item.fields.push_back({ _T("AgentName"),     _T("GAZÝŢEHÝR GAYRÝMENKUL") });
    item.fields.push_back({ _T("AgentTitle"),    _T("Gayrimenkul Danýţmaný") });
    item.fields.push_back({ _T("AgentAuthName"), _T("Ali AY") });
    item.fields.push_back({ _T("AgentPhone"),    _T("0532 000 00 00") });
    item.fields.push_back({ _T("AgentAddress"),  _T("15 Temmuz Mah. Prof. Necmettin Erbakan Cd. No:116") });
}


namespace {

// "para" alanlarÄ±nÄ± biraz daha gĂĽzel basmak iĂ§in basit heuristik
static bool IsMoneyField(const CString& dbName)
{
    CString s = dbName; s.MakeLower();
    return (s.Find(_T("price")) >= 0) || (s == _T("fiyat")) || (s.Find(_T("ucret")) >= 0) || (s.Find(_T("aidat")) >= 0);
}

static bool IsAreaField(const CString& dbName)
{
    CString s = dbName; s.MakeLower();
    return (s.Find(_T("area")) >= 0) || (s.Find(_T("metrekare")) >= 0) || (s.Find(_T("alan")) >= 0) || (s.Find(_T("m2")) >= 0);
}

// PropertyMap<T> listesinden, record iĂ§indeki deÄźerleri PreviewItem.fields'e ekler.
// AynÄ± deÄźeri birden fazla anahtar ile ekler: dbName, label ve aliases.
// BĂ¶ylece JSON bind "Oda SayÄ±sÄ±" da Ă§alÄ±ĹźÄ±r, Ĺźablonlar "RoomCount"/"RefNo" gibi anahtarlardan da Ă§alÄ±ĹźÄ±r.

template<typename T>
static void AppendMappedFields(PreviewItem& item, const T& rec)
{
    const auto& props = GetProperties<T>();

    for (const auto& p : props)
    {
        if (!p.dbName) continue;

        CString keyDb = p.dbName;
        CString keyLabel = (p.label ? p.label : _T(""));

        CString raw;
        if (p.member) raw = rec.*(p.member);

        CString val = DocumentDataManager::CleanText(raw);

        // Tip bazlÄ± formatlar
        if (p.type == FT_BOOL) {
            val = DocumentDataManager::FormatBool(raw);
        }
        else if (p.type == FT_DOUBLE) {
            if (IsMoneyField(keyDb))
                val = DocumentDataManager::FormatMoney(raw);
            else
                val = DocumentDataManager::CleanText(raw);

            if (!val.IsEmpty() && val != _T("-") && IsAreaField(keyDb)) {
                // bazÄ± alanlarda (m2) birim eklemek isteniyor; boĹź deÄźilse ekle
                // (eÄźer Ĺźablonda zaten birim basÄ±lÄ±yorsa, bu satÄ±rÄ± kaldÄ±rabilirsiniz)
                if (val.Find(_T("m")) < 0) {
                    // Saf sayÄ±ya birim ekle
                    val += _T(" mÂ˛");
                }
            }
        }

        auto pushKey = [&](const CString& k)
        {
            if (k.IsEmpty()) return;
            item.fields.push_back({ k, val });
        };

        pushKey(keyDb);
        pushKey(keyLabel);

        for (const auto& a : p.aliases)
            pushKey(a);
    }
}

// Table'a gĂ¶re doÄźru struct tipi ile DB'den Ă§ekip haritalÄ± alanlarÄ± ekler.
static bool AppendPropertyByTable(PreviewItem& item, const CString& table, const CString& code, CString& outCariKod)
{
    DatabaseManager& db = DatabaseManager::GetInstance();

    if (table == TABLE_NAME_HOME) {
        Home_cstr h = db.GetRecordByCode<Home_cstr>(code);
        outCariKod = h.Cari_Kod;
        AppendMappedFields(item, h);
        // Ĺžablonlar iĂ§in garanti anahtarlar
        item.fields.push_back({ _T("PropertyType"), _T("KONUT") });
        item.fields.push_back({ _T("Emlak Tipi"),  _T("KONUT") });
        return true;
    }
    if (table == TABLE_NAME_LAND) {
        Land_cstr l = db.GetRecordByCode<Land_cstr>(code);
        outCariKod = l.Cari_Kod;
        AppendMappedFields(item, l);
        item.fields.push_back({ _T("PropertyType"), _T("ARSA") });
        item.fields.push_back({ _T("Emlak Tipi"),  _T("ARSA") });
        return true;
    }
    if (table == TABLE_NAME_FIELD) {
        Field_cstr f = db.GetRecordByCode<Field_cstr>(code);
        outCariKod = f.Cari_Kod;
        AppendMappedFields(item, f);
        item.fields.push_back({ _T("PropertyType"), _T("TARLA") });
        item.fields.push_back({ _T("Emlak Tipi"),  _T("TARLA") });
        return true;
    }
    if (table == TABLE_NAME_VINEYARD) {
        Vineyard_cstr v = db.GetRecordByCode<Vineyard_cstr>(code);
        outCariKod = v.Cari_Kod;
        AppendMappedFields(item, v);
        item.fields.push_back({ _T("PropertyType"), _T("BAÄž/BAHĂ‡E") });
        item.fields.push_back({ _T("Emlak Tipi"),  _T("BAÄž/BAHĂ‡E") });
        return true;
    }
    if (table == TABLE_NAME_VILLA) {
        Villa_cstr v = db.GetRecordByCode<Villa_cstr>(code);
        outCariKod = v.Cari_Kod;
        AppendMappedFields(item, v);
        item.fields.push_back({ _T("PropertyType"), _T("VÄ°LLA") });
        item.fields.push_back({ _T("Emlak Tipi"),  _T("VÄ°LLA") });
        return true;
    }
    if (table == TABLE_NAME_COMMERCIAL) {
        Commercial_cstr c = db.GetRecordByCode<Commercial_cstr>(code);
        outCariKod = c.Cari_Kod;
        AppendMappedFields(item, c);
        item.fields.push_back({ _T("PropertyType"), _T("Ä°ĹžYERÄ°") });
        item.fields.push_back({ _T("Emlak Tipi"),  _T("Ä°ĹžYERÄ°") });
        return true;
    }

    // Bilinmeyen tablo
    return false;
}

} // namespace

CString DocumentDataManager::LoadPropertyInfo(PreviewItem& item, const CString& table, const CString& code)
{
    CString cariKod;
    AppendPropertyByTable(item, table, code, cariKod);
    return cariKod;
}

void DocumentDataManager::LoadOwnerInfo(PreviewItem& item, const CString& cariKod)
{
    DatabaseManager& db = DatabaseManager::GetInstance();
    Customer_cstr c = db.GetCustomerByCariKod(cariKod);

    if (!c.Cari_Kod.IsEmpty()) {
        AppendMappedFields(item, c);

        // Ticari mĂĽĹźteri mi? (Bu alan Ĺźablonlarda bazen Ĺźart oluyor)
        bool isCorp = (c.MusteriTipi.CompareNoCase(_T("Ticari")) == 0 || c.MusteriTipi.CompareNoCase(_T("Ĺžirket")) == 0);
        item.fields.push_back({ _T("IsCorporate"), isCorp ? _T("1") : _T("0") });
    }
    else {
        item.fields.push_back({ _T("OwnerName"), _T("...................................") });
        item.fields.push_back({ _T("OwnerPhone"), _T("...................................") });
    }
}
CString DocumentDataManager::CleanText(const CString& text)
{
    // If empty, show placeholder.
    if (text.IsEmpty()) return _T("-");
    return text;
}

CString DocumentDataManager::FormatMoney(const CString& rawPrice)
{
    if (rawPrice.IsEmpty() || rawPrice == _T("0")) return _T("");

    // Remove thousand separators then re-format.
    CString s = rawPrice;
    s.Remove('.');

    // Add simple thousand separators.
    int len = s.GetLength();
    if (len > 3) {
        for (int i = len - 3; i > 0; i -= 3) {
            s.Insert(i, _T("."));
        }
    }
    return s;
}

CString DocumentDataManager::FormatBool(const CString& val)
{
    // Accept 1/True/Evet/Var as true.
    if (val == _T("1") ||
        val.CompareNoCase(_T("True")) == 0 ||
        val.CompareNoCase(_T("Evet")) == 0 ||
        val.CompareNoCase(_T("Var")) == 0)
    {
        return _T("Var");
    }
    return _T("Yok");
}

CString DocumentDataManager::GetDateNow()
{
    time_t now = time(0);
    struct tm tstruct;
    localtime_s(&tstruct, &now);

    CString str;
    str.Format(_T("%02d.%02d.%04d"), tstruct.tm_mday, tstruct.tm_mon + 1, tstruct.tm_year + 1900);
    return str;
}
