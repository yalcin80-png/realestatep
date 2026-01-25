#pragma once
#ifndef GDI_RENTAL_LAYOUT_H
#define GDI_RENTAL_LAYOUT_H

#include "IDocumentLayout.h"
#include <map>
#include <string>
#include <vector>

class GdiRentalLayout : public IDocumentLayout
{
public:
    GdiRentalLayout();
    virtual ~GdiRentalLayout();

    virtual void SetData(const std::vector<std::pair<CString, CString>>& fields) override;
    virtual int  GetTotalPages() const override { return 1; }
    virtual void Render(IDrawContext& ctx, int pageNo) override;

private:
    std::map<std::wstring, std::wstring> m_data;
    std::wstring GetVal(const std::wstring& key);
    FontDesc MakeFont(const std::wstring& family, float size, bool bold = false);
};

#endif