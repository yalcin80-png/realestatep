#include "stdafx.h"
#include "SahibindenParser.h"
#include <sstream>

// ---------------------------------------------------
// Metni normalize et
// ---------------------------------------------------
void SahibindenParser::CleanText(CString& text)
{
    // NBSP -> boşluk
    text.Replace((TCHAR)160, _T(' '));

    // Satır sonları
    text.Replace(_T("\r\n"), _T("\n"));
    text.Replace(_T("\r"), _T("\n"));

    // Bazı ayraçları satıra çevir
    text.Replace(_T("|"), _T("\n"));   // "İlan No | Tarih | ..." için
    text.Replace(_T("•"), _T("\n"));   // madde işaretleri

    text.Trim();

    // Çoklu boşlukları tek boşluğa indir
    for (;;)
    {
        CString old = text;
        text.Replace(_T("  "), _T(" "));
        if (text == old)
            break;
    }
}

// ---------------------------------------------------
// Metni satırlara böl
// ---------------------------------------------------
std::vector<CString> SahibindenParser::SplitToLines(const CString& text)
{
    std::vector<CString> lines;
    std::wistringstream iss((LPCWSTR)text);
    for (std::wstring line; std::getline(iss, line); )
    {
        CString cLine(line.c_str());
        cLine.Trim();
        if (!cLine.IsEmpty())
            lines.push_back(cLine);
    }
    return lines;
}

// ---------------------------------------------------
// Alan tanımları (canonical + label eşanlamlıları)
// Buradaki canonical isimler senin eski GetKnownKeys
// ile birebir uyumlu tutuldu.
// ---------------------------------------------------
const std::vector<SahibindenParser::FieldDef>& SahibindenParser::GetFieldDefs()
{
    static const std::vector<FieldDef> fields = {
        // Temel bilgiler
        { _T("İlan No"),      { _T("İlan No"), _T("İLAN NO"), _T("ilan no") }, false },
        { _T("İlan Tarihi"),  { _T("İlan Tarihi"), _T("İLAN TARİHİ"), _T("ilan tarihi") }, false },
        { _T("Emlak Tipi"),   { _T("Emlak Tipi"), _T("Kategori"), _T("İlan Tipi"), _T("İlan Türü") }, false },

        // Konum
        { _T("Adres"),        { _T("Adres") }, true },
        { _T("Şehir"),        { _T("Şehir"), _T("İl") }, false },
        { _T("İlçe"),         { _T("İlçe") }, false },
        { _T("Mahalle"),      { _T("Mahalle"), _T("Semt"), _T("Bölge") }, false },

        // Finansal
        { _T("Fiyat"),        { _T("Fiyat"), _T("Satış Fiyatı"), _T("Kira"), _T("Toplam Fiyat") }, false },
        { _T("Para Birimi"),  { _T("Para Birimi"), _T("Döviz Türü") }, false },
        { _T("Aidat (TL)"),   { _T("Aidat (TL)"), _T("Aidat"), _T("Site Aidatı") }, false },

        // Özellikler (eski GetKnownKeys'den)
        { _T("m² (Brüt)"),    { _T("m² (Brüt)"), _T("Brüt m²"), _T("Brüt m2") }, false },
        { _T("m² (Net)"),     { _T("m² (Net)"), _T("Net m²"), _T("Net m2"), _T("Kullanım Alanı") }, false },
        { _T("Oda Sayısı"),   { _T("Oda Sayısı"), _T("Oda + Salon Sayısı"), _T("Oda / Salon Sayısı") }, false },
        { _T("Bina Yaşı"),    { _T("Bina Yaşı") }, false },
        { _T("Bulunduğu Kat"),{ _T("Bulunduğu Kat") }, false },
        { _T("Kat Sayısı"),   { _T("Kat Sayısı") }, false },
        { _T("Isıtma"),       { _T("Isıtma"), _T("Isınma Tipi"), _T("Isınma Şekli") }, false },
        { _T("Banyo Sayısı"), { _T("Banyo Sayısı") }, false },
        { _T("Mutfak"),       { _T("Mutfak"), _T("Mutfak Tipi") }, false },
        { _T("Balkon"),       { _T("Balkon") }, false },
        { _T("Asansör"),      { _T("Asansör") }, false },
        { _T("Otopark"),      { _T("Otopark") }, false },
        { _T("Eşyalı"),       { _T("Eşyalı"), _T("Eşya Durumu") }, false },
        { _T("Kullanım Durumu"), { _T("Kullanım Durumu") }, false },
        { _T("Site İçerisinde"), { _T("Site İçerisinde"), _T("Site İçinde") }, false },
        { _T("Site Adı"),     { _T("Site Adı") }, true },
        { _T("Krediye Uygun"),{ _T("Krediye Uygun") }, false },
        { _T("Tapu Durumu"),  { _T("Tapu Durumu") }, false },
        { _T("Kimden"),       { _T("Kimden"), _T("İlan Sahibi") }, false },
        { _T("Takas"),        { _T("Takas") }, false },
        { _T("Ada No"),       { _T("Ada No") }, false },
        { _T("Parsel No"),    { _T("Parsel No") }, false },
        { _T("Pafta No"),     { _T("Pafta No") }, false },
    };
    return fields;
}

// ---------------------------------------------------
// Fiyat yakalayıcı (label olmasa da)
// ---------------------------------------------------
bool SahibindenParser::TryParsePrice(const CString& line, std::map<CString, CString>& out)
{
    // Zaten fiyat bulunduysa veya satır çok uzunsa uğraşma
    if (out.find(_T("Fiyat")) != out.end() || line.GetLength() > 80)
        return false;

    if (line.Find(_T("TL")) != -1 || line.Find(_T("USD")) != -1 || line.Find(_T("EUR")) != -1)
    {
        CString digits;
        for (int k = 0; k < line.GetLength(); ++k)
        {
            if (_istdigit(line[k]))
                digits += line[k];
        }

        if (!digits.IsEmpty())
        {
            if (out[_T("Fiyat")].IsEmpty())
                out[_T("Fiyat")] = digits;

            if (out[_T("Para Birimi")].IsEmpty())
            {
                if (line.Find(_T("USD")) != -1)      out[_T("Para Birimi")] = _T("USD");
                else if (line.Find(_T("EUR")) != -1) out[_T("Para Birimi")] = _T("EUR");
                else                                 out[_T("Para Birimi")] = _T("TL");
            }
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------
// Şehir / İlçe / Mahalle formatlı adres yakalayıcı
// ---------------------------------------------------
bool SahibindenParser::TryParseAddress(const CString& line, std::map<CString, CString>& out)
{
    if (line.Find(_T('/')) == -1 ||
        line.Find(_T("İlan No")) != -1 ||
        line.Find(_T("TL")) != -1)
        return false;

    CString temp = line;
    temp.Replace(_T(" / "), _T("/"));

    int pos1 = temp.Find(_T('/'));
    if (pos1 != -1)
    {
        CString city = temp.Left(pos1); city.Trim();
        out[_T("Şehir")] = city;

        int pos2 = temp.Find(_T('/'), pos1 + 1);
        if (pos2 != -1)
        {
            CString dist = temp.Mid(pos1 + 1, pos2 - pos1 - 1); dist.Trim();
            CString nb = temp.Mid(pos2 + 1); nb.Trim();
            out[_T("İlçe")] = dist;
            out[_T("Mahalle")] = nb;
        }
        else
        {
            CString dist = temp.Mid(pos1 + 1); dist.Trim();
            out[_T("İlçe")] = dist;
        }

        if (out[_T("Adres")].IsEmpty())
            out[_T("Adres")] = line;

        return true;
    }

    return false;
}

// ---------------------------------------------------
// Label ile başlıyor mu? Değer kısmını çıkar
// ---------------------------------------------------
bool SahibindenParser::StartsWithLabel(const CString& line, const CString& label, CString& valuePart)
{
    CString lowLine = line;
    CString lowLbl = label;
    lowLine.MakeLower();
    lowLbl.MakeLower();

    int pos = lowLine.Find(lowLbl);
    if (pos == -1)
        return false;

    // Label başta değilse, öncesi boşluk / tire / bullet olmalı
    if (pos > 0)
    {
        TCHAR prev = lowLine[pos - 1];
        if (prev != _T(' ') && prev != _T('\t') &&
            prev != _T('-') && prev != _T('•'))
            return false;
    }

    int valuePos = pos + label.GetLength();
    if (valuePos >= line.GetLength())
    {
        valuePart.Empty();
        return true;    // değer alt satırda olabilir
    }

    CString rest = line.Mid(valuePos);
    rest.TrimLeft();

    // ':' , '-' , '=' vs. ayraçları kırp
    while (!rest.IsEmpty())
    {
        TCHAR ch = rest[0];
        if (ch == _T(':') || ch == _T('-') || ch == _T('=') || ch == _T('>'))
            rest = rest.Mid(1);
        else if (ch == _T(' ') || ch == _T('\t'))
            rest = rest.Mid(1);
        else
            break;
    }

    rest.Trim();
    valuePart = rest;
    return true;
}

// ---------------------------------------------------
// Ana parse fonksiyonu
// ---------------------------------------------------
std::map<CString, CString> SahibindenParser::Parse(const CString& rawText)
{
    std::map<CString, CString> out;

    // 0) Normalize
    CString text = rawText;
    CleanText(text);

    // 1) Satırlara böl
    auto lines = SplitToLines(text);
    const auto& fields = GetFieldDefs();

    // 2) Tüm label'ları topla (başka key mi kontrolü için)
    std::vector<CString> allLabels;
    allLabels.reserve(fields.size() * 3);
    for (const auto& f : fields)
        for (const auto& lbl : f.labels)
            allLabels.push_back(lbl);

    auto toLower = [](CString s) {
        s.Trim();
        s.MakeLower();
        return s;
        };

    auto lineLooksLikeKey = [&](const CString& line) -> bool
        {
            CString trimmed = line;
            trimmed.Trim();
            if (trimmed.IsEmpty())
                return false;

            CString lowLine = toLower(trimmed);
            for (const auto& lbl : allLabels)
            {
                CString lowLbl = toLower(lbl);
                if (lowLine.Find(lowLbl) == 0)   // satır label ile başlıyorsa
                    return true;
            }
            return false;
        };

    // 3) Satırlar üzerinde dön
    for (size_t i = 0; i < lines.size(); ++i)
    {
        CString line = lines[i];

        // Özel analizler
        if (TryParsePrice(line, out))
            continue;
        if (TryParseAddress(line, out))
            continue;

        // Genel label bazlı parse
        bool consumedThisLine = false;

        for (const auto& def : fields)
        {
            for (const auto& lbl : def.labels)
            {
                CString valuePart;
                if (!StartsWithLabel(line, lbl, valuePart))
                    continue;

                CString value = valuePart;

                // Çok satırlı alan ise ve aynı satırda değer yoksa alt satırlara bak
                if (value.IsEmpty() && def.multiLine)
                {
                    size_t j = i + 1;
                    while (j < lines.size())
                    {
                        CString next = lines[j];
                        if (lineLooksLikeKey(next))
                            break;  // sıradaki satır yeni bir key ise dur

                        CString trimmedNext = next;
                        trimmedNext.Trim();
                        if (!trimmedNext.IsEmpty())
                        {
                            if (!value.IsEmpty())
                                value += _T(" ");
                            value += trimmedNext;
                        }
                        ++j;
                    }
                }

                value.Trim();
                if (!value.IsEmpty())
                {
                    auto it = out.find(def.canonical);
                    if (it == out.end() || it->second.IsEmpty())
                        out[def.canonical] = value;
                }

                consumedThisLine = true;
                break;
            }

            if (consumedThisLine)
                break;
        }

        // İstersen burada eski ReverseFind fallback'ini ekleyebilirsin;
        // ben gürültü veri girmesin diye pas geçiyorum.
    }

    return out;
}





bool SahibindenParser::ExtractGaJsonFromHtml(const CString& html, CString& gaJson)
{
    gaJson.Empty();

    // 1) div id'sini bul
    int divPos = html.Find(_T("id=\"gaPageViewTrackingJson\""));
    if (divPos == -1)
        return false;

    // 2) Bu div içinde data-json="..." attribute'unu bul
    int attrPos = html.Find(_T("data-json=\""), divPos);
    if (attrPos == -1)
        return false;

    attrPos += (int)_tcslen(_T("data-json=\""));

    // 3) Attribute'un kapanış tırnağını bul
    int endPos = html.Find(_T("\""), attrPos);
    if (endPos == -1 || endPos <= attrPos)
        return false;

    gaJson = html.Mid(attrPos, endPos - attrPos);
    gaJson.Trim();

    return !gaJson.IsEmpty();
}




bool SahibindenParser::ParseGaJson(const CString& gaJson, std::map<CString, CString>& out)
{
    out.clear();

    CString s = gaJson;

    // 1) customVars bölümü
    {
        int pos = s.Find(_T("customVars:["));
        if (pos != -1)
        {
            int start = pos + (int)_tcslen(_T("customVars:["));
            int end = s.Find(_T("]"), start);
            if (end != -1 && end > start)
            {
                CString section = s.Mid(start, end - start);
                ParseKeyValueArray(section, /*lastTokenIsIndex=*/true, out);
            }
        }
    }

    // 2) dmpData bölümü (opsiyonel, istersen buradan da veri çekebilirsin)
    {
        int pos = s.Find(_T("dmpData:["));
        if (pos != -1)
        {
            int start = pos + (int)_tcslen(_T("dmpData:["));
            int end = s.Find(_T("]"), start);
            if (end != -1 && end > start)
            {
                CString section = s.Mid(start, end - start);
                // Burada index yok, {key,value} formatında
                ParseKeyValueArray(section, /*lastTokenIsIndex=*/false, out);
            }
        }
    }

    return !out.empty();
}



void SahibindenParser::ParseKeyValueArray(const CString& section,
    bool lastTokenIsIndex,
    std::map<CString, CString>& out)
{
    // section: "{,İlan No,1273242113,:2},{,İlan Tarihi,04 Aralık 2025,:2},..."
    CString tmp = section;

    // Kolay parçalansın diye "},{" yerine "|" koy
    tmp.Replace(_T("},{"), _T("|"));

    // Baştaki/sondaki süslü parantezleri biraz normalize edelim
    if (tmp.Left(1) == _T("{"))
        tmp = tmp.Mid(1);
    if (tmp.Right(1) == _T("}"))
        tmp = tmp.Left(tmp.GetLength() - 1);

    int cur = 0;
    while (cur < tmp.GetLength())
    {
        int sep = tmp.Find(_T("|"), cur);
        CString item = (sep == -1) ? tmp.Mid(cur) : tmp.Mid(cur, sep - cur);
        cur = (sep == -1) ? tmp.GetLength() : sep + 1;

        item.Trim();
        if (item.IsEmpty())
            continue;

        // Kalan { ve } karakterlerini temizle
        if (!item.IsEmpty() && item[0] == _T('{'))
            item = item.Mid(1);
        if (!item.IsEmpty() && item[item.GetLength() - 1] == _T('}'))
            item = item.Left(item.GetLength() - 1);

        // Şimdi virgül ile token'lara ayıralım
        std::vector<CString> tokens;
        int pos = 0;
        while (pos < item.GetLength())
        {
            int cpos = item.Find(_T(','), pos);
            CString tok;
            if (cpos == -1)
            {
                tok = item.Mid(pos);
                pos = item.GetLength();
            }
            else
            {
                tok = item.Mid(pos, cpos - pos);
                pos = cpos + 1;
            }
            tok.Trim();
            if (!tok.IsEmpty())
                tokens.push_back(tok);
        }

        if (tokens.empty())
            continue;

        CString key, value;

        if (lastTokenIsIndex)
        {
            // Ör: {,İlan No,1273242113,:2}
            // tokens: ["İlan No", "1273242113", ":2"]
            if (tokens.size() >= 3)
            {
                key = tokens[0];
                value = tokens[1];
            }
            else if (tokens.size() == 2 && tokens[1][0] != _T(':'))
            {
                // Ör: "Emlak,:2" gibi tek başına olanları çoğunlukla boş geçiyoruz
                key = tokens[0];
                value = tokens[1];
            }
        }
        else
        {
            // dmpData: {fiyat,8250000} -> ["fiyat", "8250000"]
            if (tokens.size() >= 2)
            {
                key = tokens[0];
                value = tokens[1];
            }
        }

        key.Trim();
        value.Trim();
        if (!key.IsEmpty() && !value.IsEmpty())
        {
            // Türkçe key'ler doğrudan HomeDlg şemasıyla uyumlu:
            // "İlan No", "İlan Tarihi", "Emlak Tipi", "m² (Net)" vs.
            out[key] = value;
        }
    }
}



bool SahibindenParser::ParseFromHtml(const CString& html, std::map<CString, CString>& out)
{
    CString gaJson;
    if (!ExtractGaJsonFromHtml(html, gaJson))
        return false;

    return ParseGaJson(gaJson, out);
}
