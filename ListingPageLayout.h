#pragma once
#ifndef LISTINGPAGELAYOUT_H
#define LISTINGPAGELAYOUT_H

#include "stdafx.h"
#include <vector>
#include <utility>
#include <string>
#include <map>
#include "IDrawContext.h"
#include "IDocumentLayout.h" 
#include "PreviewItem.h" 

class ListingPageLayout : public IDocumentLayout
{
public:
    ListingPageLayout();
    virtual ~ListingPageLayout() {}

    // IDocumentLayout Arayüzü
    virtual void SetData(const std::vector<std::pair<CString, CString>>& fields) override;
    virtual void Render(IDrawContext& ctx, int pageNo) override;
    virtual int  GetTotalPages() const override { return 1; } // Þimdilik tek sayfa

private:
    // --- ÇÝZÝM PARÇALARI ---
    void RenderHeader(IDrawContext& ctx, int& y);
    void RenderCustomer(IDrawContext& ctx, int& y);
    void RenderProperty(IDrawContext& ctx, int& y);
    void RenderFooter(IDrawContext& ctx);

    // --- YARDIMCILAR (ThemeConfig Uyumlu) ---
    void DrawSectionTitle(IDrawContext& ctx, const std::wstring& title, int& y);

    void DrawRowNCols(IDrawContext& ctx,
        const std::vector<std::pair<std::wstring, std::wstring>>& fields,
        size_t startIndex, int numCols, int& y);

    void DrawLabelValueCell(IDrawContext& ctx,
        const std::wstring& label, const std::wstring& value,
        int x, int y, int w, int h);

    void DrawLineH(IDrawContext& ctx, int x1, int y, int x2);

    // Veri Dönüþüm
    static std::wstring ToW(const CString& s);

private:
    // Veriler
    CString m_tableName;
    CString m_title;

    // Ayrýþtýrýlmýþ Veriler
    std::vector<std::pair<std::wstring, std::wstring>> m_customerFields;
    std::vector<std::pair<std::wstring, std::wstring>> m_propertyFields;
    std::wstring m_notes;
};

#endif // LISTINGPAGELAYOUT_H