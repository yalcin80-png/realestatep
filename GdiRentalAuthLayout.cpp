#include "stdafx.h"
#include "GdiRentalAuthLayout.h"

// (Yardımcı dönüşüm fonksiyonu ToStdW, önceki dosyalarda vardı, aynısı)
static std::wstring ToStdW(const CString& str) {
#ifdef UNICODE
    return std::wstring(str.GetString());
#else
    // Basit ANSI dönüşümü
    int len = MultiByteToWideChar(CP_ACP, 0, str.GetString(), -1, NULL, 0);
    if(len<=0) return L"";
    std::vector<wchar_t> buf(len);
    MultiByteToWideChar(CP_ACP, 0, str.GetString(), -1, buf.data(), len);
    return std::wstring(buf.data());
#endif
}

GdiRentalAuthLayout::GdiRentalAuthLayout() {}
GdiRentalAuthLayout::~GdiRentalAuthLayout() {}

void GdiRentalAuthLayout::SetData(const std::vector<std::pair<CString, CString>>& fields) {
    m_data.clear();
    for(const auto& kv : fields) m_data[ToStdW(kv.first)] = ToStdW(kv.second);
}

std::wstring GdiRentalAuthLayout::GetVal(const std::wstring& key) {
    auto it = m_data.find(key);
    if (it != m_data.end() && !it->second.empty()) {
        return it->second;
    }
    return L""; // Boş string döndür, placeholder değil
}

FontDesc GdiRentalAuthLayout::MakeFont(const std::wstring& family, float size, bool bold) {
    return FontDesc(family, size, bold, false);
}

void GdiRentalAuthLayout::Render(IDrawContext& ctx, int pageNo) {
    ctx.SetLogicalPageSize(2100, 2970);
    ctx.FillRect(0, 0, 2100, 2970, RGB(255, 255, 255));
    
    // Ana çerçeve
    ctx.SetPen(RGB(0,0,0), 2.0f);
    ctx.DrawRect(50, 50, 2000, 2870, true);

    int x = 100, y = 100, w = 1900;
    const int lineH = 45;

    // Yardımcı lambda fonksiyonlar
    auto DrawSectionHeader = [&](const std::wstring& title, COLORREF bgColor = RGB(220, 220, 220)) {
        ctx.FillRect(x, y, w, 35, bgColor);
        ctx.SetPen(RGB(0, 0, 0), 1.0f);
        ctx.DrawRect(x, y, w, 35, true);
        ctx.SetFont(MakeFont(L"Arial", 12, true));
        ctx.SetBrush(RGB(200, 0, 0)); // Kızıl renk başlık
        ctx.DrawTextW(title, x + 10, y + 5, w - 20, 25, DT_LEFT | DT_VCENTER);
        y += 40;
    };

    auto DrawField = [&](const std::wstring& label, const std::wstring& value, int labelWidth = 300) {
        ctx.SetFont(MakeFont(L"Arial", 10, true));
        ctx.SetBrush(RGB(0, 0, 0));
        ctx.DrawTextW(label, x + 10, y, labelWidth, 30, DT_LEFT | DT_VCENTER);
        
        ctx.SetFont(MakeFont(L"Arial", 10, false));
        ctx.SetBrush(RGB(0, 0, 150)); // Mavi veri metni
        ctx.DrawTextW(value, x + labelWidth + 10, y, w - labelWidth - 20, 30, DT_LEFT | DT_VCENTER);
        
        ctx.SetPen(RGB(200, 200, 200), 0.5f);
        ctx.DrawLine(x, y + 35, x + w, y + 35);
        y += lineH;
    };

    auto DrawTwoFields = [&](const std::wstring& label1, const std::wstring& value1,
                              const std::wstring& label2, const std::wstring& value2) {
        int halfW = w / 2;
        ctx.SetFont(MakeFont(L"Arial", 10, true));
        ctx.SetBrush(RGB(0, 0, 0));
        ctx.DrawTextW(label1, x + 10, y, 180, 30, DT_LEFT | DT_VCENTER);
        
        ctx.SetFont(MakeFont(L"Arial", 10, false));
        ctx.SetBrush(RGB(0, 0, 150));
        ctx.DrawTextW(value1, x + 200, y, halfW - 210, 30, DT_LEFT | DT_VCENTER);
        
        ctx.SetFont(MakeFont(L"Arial", 10, true));
        ctx.SetBrush(RGB(0, 0, 0));
        ctx.DrawTextW(label2, x + halfW + 10, y, 180, 30, DT_LEFT | DT_VCENTER);
        
        ctx.SetFont(MakeFont(L"Arial", 10, false));
        ctx.SetBrush(RGB(0, 0, 150));
        ctx.DrawTextW(value2, x + halfW + 200, y, halfW - 210, 30, DT_LEFT | DT_VCENTER);
        
        ctx.SetPen(RGB(200, 200, 200), 0.5f);
        ctx.DrawLine(x, y + 35, x + w, y + 35);
        y += lineH;
    };

    auto DrawCheckbox = [&](const std::wstring& label, bool checked = false) {
        // Checkbox kutucuğu
        int boxSize = 18;
        ctx.SetPen(RGB(0, 0, 0), 1.0f);
        ctx.DrawRect(x + 10, y + 6, boxSize, boxSize, true);
        if (checked) {
            ctx.DrawLine(x + 12, y + 14, x + 18, y + 20);
            ctx.DrawLine(x + 18, y + 20, x + 26, y + 10);
        }
        ctx.SetFont(MakeFont(L"Arial", 10, false));
        ctx.SetBrush(RGB(0, 0, 0));
        ctx.DrawTextW(label, x + 35, y, 300, 30, DT_LEFT | DT_VCENTER);
        y += 30;
    };

    if (pageNo == 1) {
        // ===== SAYFA 1: BİLGİLER =====
        // Ana başlık
        ctx.SetFont(MakeFont(L"Arial", 20, true));
        ctx.SetBrush(RGB(200, 0, 0));
        ctx.DrawTextW(L"KİRALIK TEK YETKİ SÖZLEŞMESİ", x, y, w, 50, DT_CENTER);
        y += 70;

        // 1. İŞ SAHİBİ BİLGİLERİ
        DrawSectionHeader(L"1. İŞ SAHİBİ BİLGİLERİ");
        DrawField(L"ADI SOYADI:", GetVal(L"OwnerName"));
        DrawTwoFields(L"T.C./KİMLİK NO.:", GetVal(L"OwnerTC"), 
                      L"TELEFON:", GetVal(L"OwnerPhone"));
        DrawField(L"E-POSTA:", GetVal(L"OwnerEmail"));
        DrawField(L"ADRES:", GetVal(L"OwnerAddress"));
        y += 20;

        // 2. EMLAK İŞLETMESİ BİLGİLERİ
        DrawSectionHeader(L"2. EMLAK İŞLETMESİ BİLGİLERİ");
        DrawField(L"YETKİLİ KİŞİ:", GetVal(L"AgentName"));
        DrawTwoFields(L"UNVANI:", GetVal(L"AgentTitle"), 
                      L"TELEFON:", GetVal(L"AgentPhone"));
        DrawField(L"E-POSTA:", GetVal(L"AgentEmail"));
        DrawField(L"ADRES:", GetVal(L"AgentAddress"));
        y += 20;

        // 3. GAYRIMENKUL TAPU KAYDI
        DrawSectionHeader(L"3. GAYRİMENKUL TAPU KAYDI");
        DrawTwoFields(L"İLİ:", GetVal(L"City"), L"İLÇESİ:", GetVal(L"District"));
        DrawTwoFields(L"MAHALLESİ:", GetVal(L"Neighborhood"), L"PAFTA:", GetVal(L"Pafta"));
        DrawTwoFields(L"ADA:", GetVal(L"Ada"), L"PARSEL:", GetVal(L"Parsel"));
        DrawTwoFields(L"KAT NO.:", GetVal(L"Floor"), L"B.B. NO.:", GetVal(L"BuildingNo"));
        DrawField(L"ARSA PAYI:", GetVal(L"LandShare"));
        y += 20;

        // 4. GAYRIMENKUL ADRESİ
        DrawSectionHeader(L"4. GAYRİMENKUL ADRESİ");
        DrawField(L"MALİKLER:", GetVal(L"Owners"));
        DrawField(L"ADRES:", GetVal(L"FullAddress"));
        
        // Mülkiyet türü checkboxlar
        y += 5;
        int cbStartY = y;
        DrawCheckbox(L"KAT MÜLKİYETİ");
        int cbY1 = y;
        y = cbStartY;
        x += 300;
        DrawCheckbox(L"KAT İRTİFAKI");
        int cbY2 = y;
        y = cbStartY;
        x += 300;
        DrawCheckbox(L"ARSA/ARAZİ");
        x -= 600; // Reset x
        y = (cbY1 > cbY2 ? cbY1 : cbY2) + 10;

        // 5. İMAR DURUMU
        DrawSectionHeader(L"5. İMAR DURUMU");
        DrawTwoFields(L"BÖLGE/ALAN:", GetVal(L"ZoningArea"), L"İMAR DURUMU:", GetVal(L"ZoningStatus"));
        
    } else if (pageNo == 2) {
        // ===== SAYFA 2: YAPI BİLGİLERİ VE ÖZELLİKLER =====
        ctx.SetFont(MakeFont(L"Arial", 16, true));
        ctx.SetBrush(RGB(200, 0, 0));
        ctx.DrawTextW(L"GAYRİMENKUL ÖZELLİKLERİ", x, y, w, 40, DT_CENTER);
        y += 60;

        // 1. YAPIYA AİT BİLGİLER
        DrawSectionHeader(L"1. YAPIYA AİT BİLGİLER");
        DrawTwoFields(L"TAŞINMAZIN ALANI (M²):", GetVal(L"LandArea"), 
                      L"YAŞI:", GetVal(L"BuildingAge"));
        DrawTwoFields(L"FİİLİ KULLANIM DURUMU:", GetVal(L"ActualUse"), 
                      L"İNŞAATIN DURUMU:", GetVal(L"ConstructionStatus"));
        DrawTwoFields(L"BRÜT ALAN (M²):", GetVal(L"GrossArea"), 
                      L"NET ALAN (M²):", GetVal(L"NetArea"));
        DrawField(L"DOĞALGAZ ALANI:", GetVal(L"NaturalGasArea"));
        y += 20;

        // 2. İLETİŞİM OLANAKLARI
        DrawSectionHeader(L"2. İLETİŞİM OLANAKLARI");
        DrawField(L"TOPLU TAŞIMA:", GetVal(L"PublicTransport"));
        DrawField(L"RAYLI SİSTEMLER:", GetVal(L"RailSystems"));
        DrawField(L"DENİZ ULAŞIMI:", GetVal(L"SeaTransport"));
        
        // Otopark bilgisi - horizontal checkboxes
        ctx.SetFont(MakeFont(L"Arial", 10, true));
        ctx.SetBrush(RGB(0, 0, 0));
        ctx.DrawTextW(L"OTOPARK:", x + 10, y, 200, 30, DT_LEFT);
        
        int parkingX = x + 220;
        int parkingStartY = y;
        const int parkingSpacing = 150;
        std::vector<std::wstring> parkingOptions = {L"AÇIK", L"KAPALI", L"YOK"};
        
        for (size_t i = 0; i < parkingOptions.size(); i++) {
            int tempX = x;
            x = parkingX + (i * parkingSpacing);
            y = parkingStartY;
            DrawCheckbox(parkingOptions[i]);
            x = tempX;
        }
        y = parkingStartY + 30;
        y += 20;

        // 3. YAPI BÖLÜM ALANLARI
        DrawSectionHeader(L"3. YAPI BÖLÜM ALANLARI");
        DrawTwoFields(L"SALON:", GetVal(L"LivingRoom"), L"MUTFAK:", GetVal(L"Kitchen"));
        DrawTwoFields(L"BANYO:", GetVal(L"Bathroom"), L"WC:", GetVal(L"WC"));
        DrawField(L"YATAK ODASI SAYISI:", GetVal(L"BedroomCount"));
        y += 20;

        // 4. İÇ ÖZELLİKLER
        DrawSectionHeader(L"4. İÇ ÖZELLİKLER");
        
        // Grid layout for checkboxes (3 columns)
        const int checkboxHeight = 30; // Height added by DrawCheckbox
        int colCount = 3;
        int colWidth = w / colCount;
        int startX = x;
        int cbCol = 0;
        
        std::vector<std::wstring> features = {
            L"MERDİVEN", L"ASANSÖR", L"KLİMA", L"ISITMA",
            L"PANJUR", L"ALARM", L"GÜVENLIK", L"OTOPARK",
            L"BALKON", L"TERAS", L"BAHÇE", L"HAVUZ"
        };
        
        int maxY = y;
        for (const auto& feature : features) {
            x = startX + (cbCol * colWidth);
            DrawCheckbox(feature);
            if (y > maxY) maxY = y;
            
            cbCol++;
            if (cbCol >= colCount) {
                cbCol = 0;
                y = maxY;
            } else {
                y -= checkboxHeight; // Reset to row start
            }
        }
        x = startX; // Reset x
        y = maxY + 20;

    } else { // pageNo == 3
        // ===== SAYFA 3: HİZMET VE İMZA =====
        ctx.SetFont(MakeFont(L"Arial", 16, true));
        ctx.SetBrush(RGB(200, 0, 0));
        ctx.DrawTextW(L"HİZMETİN NİTELİĞİ VE KAPSAMI", x, y, w, 40, DT_CENTER);
        y += 60;

        // 1. AÇIKLAMALAR
        DrawSectionHeader(L"1. AÇIKLAMALAR");
        DrawField(L"GAYRİMENKÜLÜN CİNSİ:", GetVal(L"PropertyType"));
        DrawField(L"KONUT İSKAN DURUMU:", GetVal(L"OccupancyStatus"));
        y += 20;

        // 2. KİRALAMA BEDELİ
        DrawSectionHeader(L"2. KİRALAMA BEDELİ");
        DrawField(L"RAKAM İLE:", GetVal(L"RentAmountNumeric"));
        DrawField(L"YAZI İLE:", GetVal(L"RentAmountText"));
        y += 20;

        // 3. KİRALAMA HİZMET BEDELİ
        DrawSectionHeader(L"3. KİRALAMA HİZMET BEDELİ");
        ctx.SetFont(MakeFont(L"Arial", 9, false));
        ctx.SetBrush(RGB(0, 0, 0));
        std::wstring serviceText = 
            L"Emlak İşletmesi, kira bedelinin bir aylık tutarına eşit hizmet bedeli alır. "
            L"Bu bedel, gayrimenkulün kiracıya teslim edilmesi ile tahsil edilir. "
            L"Sözleşme süresi içinde iş sahibi tarafından feshedilmesi halinde, "
            L"hizmet bedeli iade edilmez.";
        ctx.DrawTextW(serviceText, x + 10, y, w - 20, 120, DT_LEFT | DT_WORDBREAK);
        y += 140;

        // 4. YÜKÜMLÜLÜKLER
        DrawSectionHeader(L"4. TARAFLARIN YÜKÜMLÜLÜKLERİ");
        ctx.SetFont(MakeFont(L"Arial", 9, false));
        ctx.SetBrush(RGB(0, 0, 0));
        std::wstring obligations = 
            L"• İş sahibi, gayrimenkul ile ilgili tüm bilgi ve belgeleri eksiksiz sunmakla yükümlüdür.\n"
            L"• Emlak İşletmesi, profesyonel özen ve dikkatle kiracı bulmaya çalışacaktır.\n"
            L"• Sözleşme süresi 6 (altı) aydır. İş sahibi isterse süreyi uzatabilir.\n"
            L"• Taraflar karşılıklı mutabakat ile sözleşmeyi sonlandırabilir.";
        ctx.DrawTextW(obligations, x + 10, y, w - 20, 200, DT_LEFT | DT_WORDBREAK);
        y += 220;

        // 5. İMZA ALANLARI
        y += 50;
        int sigY = y;
        int sigBoxW = 550;
        int sigBoxH = 200;
        int sigGap = 100;

        // Sol imza: Emlak İşletmesi
        ctx.SetFont(MakeFont(L"Arial", 11, true));
        ctx.SetBrush(RGB(0, 0, 0));
        ctx.DrawTextW(L"EMLAK İŞLETMESİ", x, sigY, sigBoxW, 30, DT_CENTER);
        ctx.SetFont(MakeFont(L"Arial", 10, false));
        ctx.DrawTextW(GetVal(L"AgentName"), x, sigY + 35, sigBoxW, 25, DT_CENTER);
        ctx.SetPen(RGB(0, 0, 0), 1.0f);
        ctx.DrawRect(x, sigY + 70, sigBoxW, sigBoxH, true);
        ctx.SetFont(MakeFont(L"Arial", 9, false));
        ctx.DrawTextW(L"Tarih ve İmza", x, sigY + sigBoxH + 75, sigBoxW, 20, DT_CENTER);

        // Orta imza: Sözleşmeli İşletme (opsiyonel)
        int midX = x + sigBoxW + sigGap;
        ctx.SetFont(MakeFont(L"Arial", 11, true));
        ctx.DrawTextW(L"SÖZLEŞMELİ İŞLETME", midX, sigY, sigBoxW, 30, DT_CENTER);
        ctx.DrawRect(midX, sigY + 70, sigBoxW, sigBoxH, true);
        ctx.SetFont(MakeFont(L"Arial", 9, false));
        ctx.DrawTextW(L"Tarih ve İmza", midX, sigY + sigBoxH + 75, sigBoxW, 20, DT_CENTER);

        // Sağ imza: İş Sahibi
        int rightX = midX + sigBoxW + sigGap;
        ctx.SetFont(MakeFont(L"Arial", 11, true));
        ctx.DrawTextW(L"İŞ SAHİBİ", rightX, sigY, sigBoxW, 30, DT_CENTER);
        ctx.SetFont(MakeFont(L"Arial", 10, false));
        ctx.DrawTextW(GetVal(L"OwnerName"), rightX, sigY + 35, sigBoxW, 25, DT_CENTER);
        ctx.DrawRect(rightX, sigY + 70, sigBoxW, sigBoxH, true);
        ctx.SetFont(MakeFont(L"Arial", 9, false));
        ctx.DrawTextW(L"Tarih ve İmza", rightX, sigY + sigBoxH + 75, sigBoxW, 20, DT_CENTER);

        // Footer note
        y = sigY + sigBoxH + 120;
        ctx.SetFont(MakeFont(L"Arial", 8, false));
        ctx.SetBrush(RGB(100, 100, 100));
        ctx.DrawTextW(L"Bu sözleşme 3 nüsha olarak düzenlenmiş olup, taraflarca imzalanmıştır.", 
                     x, y, w, 25, DT_CENTER);
    }
}