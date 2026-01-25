#pragma once
#ifndef DEPOSITPAGELAYOUT_H
#define DEPOSITPAGELAYOUT_H

#include "stdafx.h"
#include "IDrawContext.h"
#include "IDocumentLayout.h" 
#include <map>
#include <string>
#include <vector>
#include <utility>

class DepositPageLayout : public IDocumentLayout
{
public:
    DepositPageLayout();
    virtual ~DepositPageLayout() {}

    // IDocumentLayout Arayüzü
    virtual void SetData(const std::vector<std::pair<CString, CString>>& fields) override;
    virtual void Render(IDrawContext& ctx, int pageNo) override;
    virtual int  GetTotalPages() const override { return 2; }

private:
    // --- SAYFA 1: TARAFLAR VE ÖDEME ---
    void RenderPage1(IDrawContext& ctx);
    void RenderHeader(IDrawContext& ctx, int& y);
    void RenderSellerInfo(IDrawContext& ctx, int& y);
    void RenderBuyerInfo(IDrawContext& ctx, int& y);
    void RenderPropertyShort(IDrawContext& ctx, int& y);
    void RenderPaymentPlan(IDrawContext& ctx, int& y);
    void RenderFooter(IDrawContext& ctx);

    // --- SAYFA 2: HÜKÜMLER ---
    void RenderPage2(IDrawContext& ctx);

    // --- YARDIMCILAR (ThemeConfig Uyumlu) ---
    void DrawSectionHeader(IDrawContext& ctx, int x, int y, int w, int h, const std::wstring& title);

    void DrawFieldBox(IDrawContext& ctx, int x, int y, int w, int h,
        const std::wstring& label, const std::wstring& value);

    void DrawClause(IDrawContext& ctx, int x, int& y, int w,
        const std::wstring& title, const std::wstring& content);

    void RenderFrame(IDrawContext& ctx);

    // Veri Dönüþüm
    std::wstring GetW(const wchar_t* key, const wchar_t* def = L"") const;
    static std::wstring ToW(const CString& s);

private:
    std::map<std::wstring, std::wstring> m_dataMap;
};

#endif // DEPOSITPAGELAYOUT_H