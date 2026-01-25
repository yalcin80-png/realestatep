#pragma once
#ifndef IDOCUMENTLAYOUT_H
#define IDOCUMENTLAYOUT_H

#include "stdafx.h"
#include "IDrawContext.h"
#include <vector>
#include <utility>

// Bu sınıf, tüm belgelerin (Sözleşme, İlan, Makbuz) ortak atasıdır.
class IDocumentLayout
{
public:
    virtual ~IDocumentLayout() {}

    // KURAL 1: Her belge veri alabilmeli
    virtual void SetData(const std::vector<std::pair<CString, CString>>& fields) = 0;
    //virtual void SetData(const std::vector<std::pair<CString, CString>>& fields) = 0;
    // KURAL 2: Her belge kendini çizebilmeli
    virtual void Render(IDrawContext& ctx, int pageNo) = 0;

    // KURAL 3: Her belge kaç sayfa olduğunu söylemeli
    virtual int GetTotalPages() const = 0;
};

#endif