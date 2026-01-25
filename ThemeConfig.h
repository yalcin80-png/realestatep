/////////////////////////////
// Doc.h
//
#pragma once

#ifndef THEMECONFIG_H
#define THEMECONFIG_H


#include "stdafx.h"
// =============================================================
// PROJE TEMA VE AYAR DOSYASI
// =============================================================


namespace Theme
{
    // =============================================================
    // 1. RENK PALETİ
    // =============================================================

    // Zeminler
    static const COLORREF Color_PageBg = RGB(255, 255, 255);
    static const COLORREF Color_BoxBg = RGB(255, 255, 255);
    static const COLORREF Color_BoxBgAlt = RGB(242, 242, 242); // Biraz daha belirgin gri

    // Kurumsal
    static const COLORREF Color_BrandRed = RGB(204, 0, 0);     // Daha canlı kırmızı
    static const COLORREF Color_HeaderBg = RGB(20, 30, 50);    // Daha koyu, asil lacivert

    // Çizgiler & Metinler
    static const COLORREF Color_BorderLight = RGB(160, 160, 160); // Çerçeveler daha net
    static const COLORREF Color_BorderDark = RGB(0, 0, 0);
    static const COLORREF Color_TextMain = RGB(0, 0, 0);       // Simsiyah metin (En net okuma)
    static const COLORREF Color_TextLabel = RGB(80, 80, 80);    // Etiketler koyu gri
    static const COLORREF Color_TextWhite = RGB(255, 255, 255);
    static const COLORREF Color_TextAccent = RGB(0, 40, 160);

    // =============================================================
    // 2. ÖLÇÜLER (BÜYÜTÜLDÜ - A4: 2100x2970)
    // =============================================================

    static const int Page_Width = 2100;
    static const int Page_Height = 2970;
    static const int Margin = 50;   // Kenar boşluğunu biraz azalttık (Daha geniş alan)
    static const int Content_Width = Page_Width - (3 * Margin);

    // --- SATIR YÜKSEKLİKLERİ (ARTIRILDI) ---
    static const int H_Header = 180; // Logo alanı büyüdü
    static const int H_Section = 40;  // Siyah başlık şeritleri kalınlaştı
    static const int H_RowStd = 150;  // Veri satırları açıldı (Daha ferah)
    static const int H_RowTall = 80; // Not alanları genişledi

    // --- FONT BOYUTLARI (PUNTO - CİDDİ ORANDA BÜYÜTÜLDÜ) ---

    // Başlık "SATILIK..." (Word'de 48 punto gibi düşün)
    static const float Font_H1 = 40.0f;

    // Bölüm Başlığı "İŞ SAHİBİ BİLGİLERİ"
    static const float Font_H2 =16.0f;

    // Etiket "Adı Soyadı:" (Okunaklı olsun)
    static const float Font_Label = 14.0f;

    // Değer "Ahmet Yılmaz" (En önemli kısım - Büyük ve Net)
    static const float Font_Val = 16.0f;

    // Fiyat vb. (Devasa)
    static const float Font_ValBig = 28.0f;
}
#endif  // WIN32XX_DOC_H