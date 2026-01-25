#pragma once
#ifndef GDI_WAIVER_LAYOUT_H
#define GDI_WAIVER_LAYOUT_H

#include "IDocumentLayout.h"
#include <map>
#include <string>
#include <vector>

class GdiWaiverLayout : public IDocumentLayout
{
public:
    GdiWaiverLayout();
    virtual ~GdiWaiverLayout();

    // Veri seti (CString gelse bile içeride wstring'e çevireceğiz)
    virtual void SetData(const std::vector<std::pair<CString, CString>>& fields) override;
    virtual int  GetTotalPages() const override { return 1; }
    virtual void Render(IDrawContext& ctx, int pageNo) override;

private:
    // Verileri standart string olarak tutuyoruz
    std::map<std::wstring, std::wstring> m_data;

    // Yardımcılar
    std::wstring GetVal(const std::wstring& key);
    FontDesc MakeFont(const std::wstring& family, float size, bool bold = false, bool italic = false);
};

#endif