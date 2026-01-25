#pragma once
#ifndef CONTRACTPAGELAYOUT_H
#define CONTRACTPAGELAYOUT_H

#include "stdafx.h"
#include "IDrawContext.h"
#include "IDocumentLayout.h" // Base class
#include <map>
#include <string>
#include <vector>
#include <utility>

class ContractPageLayout : public IDocumentLayout
{
public:
    ContractPageLayout();
    virtual ~ContractPageLayout() {}

    // IDocumentLayout Arayüzü
    virtual void SetData(const std::vector<std::pair<CString, CString>>& fields) override;
    virtual void Render(IDrawContext& ctx, int pageNo) override;
    virtual int  GetTotalPages() const override { return 3; }

private:
    // Sayfa Çizim Fonksiyonlarý
    void RenderPage1(IDrawContext& ctx);
    void RenderPage2(IDrawContext& ctx);
    void RenderPage3(IDrawContext& ctx);

    // Ortak Bileþenler
    void RenderFrame(IDrawContext& ctx);
    void RenderHeader(IDrawContext& ctx, int& y);
    void RenderFooter(IDrawContext& ctx, int& y);

    // Sayfa 1 Bölümleri
    void RenderOwnerSection(IDrawContext& ctx, int& y);
    void RenderOfficeSection(IDrawContext& ctx, int& y);
    void RenderDeedSection(IDrawContext& ctx, int& y);
    void RenderAddressSection(IDrawContext& ctx, int& y);
    void RenderPropertyTypeSection(IDrawContext& ctx, int& y);
    void RenderFooterPage1(IDrawContext& ctx, int&);

    // Sayfa 2 Bölümleri
    void RenderBuildingInfoPage2(IDrawContext& ctx, int& y);
    void RenderTransportInfoPage2(IDrawContext& ctx, int& y);
    void RenderInteriorFeaturesPage2(IDrawContext& ctx, int& y);
    void RenderMainPropertyFeaturesPage2(IDrawContext& ctx, int& y);

    // Sayfa 3 Bölümleri
    void RenderTextClausesPage3(IDrawContext& ctx, int& y);

    // --- YENÝ ÇÝZÝM YARDIMCILARI (ThemeConfig Uyumlu) ---
    void DrawSectionHeader(IDrawContext& ctx, int x, int y, int w, int h, const std::wstring& title);

    void DrawFieldBox(IDrawContext& ctx, int x, int y, int w, int h,
        const std::wstring& label, const std::wstring& value);

    void DrawCheckbox(IDrawContext& ctx, int x, int y,
        const std::wstring& label, bool isChecked);

    void DrawLabel(IDrawContext& ctx, int x, int y, int w, int h,
        const std::wstring& text, bool bold = false);

    // Veri Yardýmcýlarý
    std::wstring GetW(const wchar_t* key, const wchar_t* def = L"") const;
    bool         IsTrue(const wchar_t* key, const wchar_t* expected = L"1") const;
    static std::wstring ToW(const CString& s);

private:
    std::map<std::wstring, std::wstring> m_dataMap;
};

#endif // CONTRACTPAGELAYOUT_H