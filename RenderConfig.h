
#pragma once

#ifndef RENDERCONFIG
#define RENDERCONFIG
#include "stdafx.h"


// RenderConfig.h
#pragma once

// Kontrat sayfaları için hangi motorun tercih edileceği
enum class ContractRenderMode
{
    GDI_ONLY,        // Sadece eski GDI kontrat layout'u (ContractPageLayout)
    JSON_PREFERRED   // Önce JSON dene, JSON çalışmazsa GDI'ye düş
};

// Global değişken deklarasyonu
extern ContractRenderMode g_ContractRenderMode;

#endif  // WIN32XX_TEMPLOAD_H