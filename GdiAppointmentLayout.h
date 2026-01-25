#pragma once
#include "stdafx.h"
#include "IDocumentLayout.h"
#include "dataIsMe.h"
#include <vector>
#include <string>

class GdiAppointmentLayout : public IDocumentLayout
{
public:
    // Verileri Map/Vector olarak alır
    virtual void SetData(const std::vector<std::pair<CString, CString>>& fields) override
    {
        for (const auto& f : fields) {
            if (f.first == _T("Title")) m_title = f.second;
            else if (f.first == _T("Date")) m_date = f.second;
            else if (f.first == _T("Location")) m_location = f.second;
            else if (f.first == _T("CustName")) m_custName = f.second;
            else if (f.first == _T("CustTel")) m_custTel = f.second;
            else if (f.first == _T("PropInfo")) m_propInfo = f.second;
            else if (f.first == _T("Notes")) m_notes = f.second;
        }
    }

    virtual int GetTotalPages() const override { return 1; }

    virtual void Render(IDrawContext& ctx, int pageNo) override
    {
        // --- 1. Font Tanımları (IDrawContext.h yapısına uygun) ---
        FontDesc titleFont(L"Arial", 24.0f, true, false);
        FontDesc headerFont(L"Arial", 14.0f, true, false);
        FontDesc normalFont(L"Arial", 11.0f, false, false);

        // Sayfa Koordinatları
        double x = 50.0;
        double y = 50.0;
        double dy = 25.0; // Satır yüksekliği
        double w = 500.0; // Metin genişliği

        // Siyah Kalem (Çizgiler için)
        ctx.SetPen(RGB(0, 0, 0), 1.0f);

        // --- 2. Başlık ---
        ctx.SetFont(titleFont);
        // DrawTextW(text, x, y, w, h, flags)
        ctx.DrawTextW(L"RANDEVU BİLGİ FORMU", x, y, w, 40, DT_LEFT | DT_VCENTER);
        y += dy * 2;

        // --- 3. Randevu Detayları ---
        ctx.SetFont(headerFont);
        ctx.DrawTextW(L"1. RANDEVU DETAYLARI", x, y, w, 30, DT_LEFT | DT_VCENTER);

        // Çizgi: DrawLine(x1, y1, x2, y2)
        ctx.DrawLine(x, y + 20, x + w, y + 20);
        y += dy * 1.5;

        ctx.SetFont(normalFont);
        // CString -> std::wstring dönüşümü için .GetString() kullanıyoruz
        ctx.DrawTextW((L"Konu: " + m_title).GetString(), x, y, w, 20, DT_LEFT); y += dy;
        ctx.DrawTextW((L"Tarih: " + m_date).GetString(), x, y, w, 20, DT_LEFT); y += dy;
        ctx.DrawTextW((L"Konum: " + m_location).GetString(), x, y, w, 20, DT_LEFT); y += dy * 2;

        // --- 4. Müşteri Bilgileri ---
        ctx.SetFont(headerFont);
        ctx.DrawTextW(L"2. MÜŞTERİ BİLGİLERİ", x, y, w, 30, DT_LEFT | DT_VCENTER);
        ctx.DrawLine(x, y + 20, x + w, y + 20);
        y += dy * 1.5;

        ctx.SetFont(normalFont);
        ctx.DrawTextW((L"Adı Soyadı: " + m_custName).GetString(), x, y, w, 20, DT_LEFT); y += dy;
        ctx.DrawTextW((L"Telefon: " + m_custTel).GetString(), x, y, w, 20, DT_LEFT); y += dy * 2;

        // --- 5. Mülk Bilgileri ---
        ctx.SetFont(headerFont);
        ctx.DrawTextW(L"3. MÜLK BİLGİLERİ", x, y, w, 30, DT_LEFT | DT_VCENTER);
        ctx.DrawLine(x, y + 20, x + w, y + 20);
        y += dy * 1.5;

        ctx.SetFont(normalFont);
        // Çok satırlı (WordBreak) yazdırma
        ctx.DrawTextW(m_propInfo.GetString(), x, y, w, dy * 4, DT_LEFT | DT_WORDBREAK);
        y += dy * 4; // Mülk bilgisi için yer ayır

        // --- 6. Notlar Kutusu ---
        ctx.SetFont(headerFont);
        ctx.DrawTextW(L"4. GÖRÜŞME NOTLARI", x, y, w, 30, DT_LEFT | DT_VCENTER);
        y += dy;

        // Kutu Çizimi: DrawRect(x, y, w, h, frameOnly) -> frameOnly=true (Sadece çerçeve)
        ctx.DrawRect(x, y, w, 200, true);

        ctx.SetFont(normalFont);
        ctx.DrawTextW(m_notes.GetString(), x + 10, y + 10, w - 20, 180, DT_LEFT | DT_WORDBREAK);
    }

private:
    CString m_title, m_date, m_location, m_custName, m_custTel, m_propInfo, m_notes;
};