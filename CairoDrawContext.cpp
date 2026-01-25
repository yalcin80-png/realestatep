#include "stdafx.h"
#include "CairoDrawContext.h"
#include "ThemeConfig.h" // Renkler için

// =========================================================
// YARDIMCI DÖNÜŞÜMLER
// =========================================================
std::string CairoDrawContext::WStringToUTF8(const std::wstring& w) const
{
    if (w.empty()) return "";
    int len = ::WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), NULL, 0, NULL, NULL);
    std::string out(len, 0);
    ::WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), &out[0], len, NULL, NULL);
    return out;
}

// 0-255 Arası rengi 0.0-1.0 arasına çevirip Cairo'ya verir
void CairoDrawContext::SetSourceColor(COLORREF c)
{
    if (!m_cr) return;
    double r = (double)GetRValue(c) / 255.0;
    double g = (double)GetGValue(c) / 255.0;
    double b = (double)GetBValue(c) / 255.0;
    cairo_set_source_rgb(m_cr, r, g, b);
}

// =========================================================
// CONSTRUCTOR / DESTRUCTOR
// =========================================================
CairoDrawContext::CairoDrawContext()
    : m_surface(nullptr), m_cr(nullptr),
    m_width(0), m_height(0),
    m_penColor(RGB(0, 0, 0)), m_penWidth(1.0f),
    m_brushColor(RGB(255, 255, 255)), m_fontSize(10.0f)
{
}

CairoDrawContext::~CairoDrawContext()
{
    if (m_cr) cairo_destroy(m_cr);
    if (m_surface) cairo_surface_destroy(m_surface);
}

// =========================================================
// BELGE YÖNETİMİ
// =========================================================
bool CairoDrawContext::BeginDocument(const CString& filePath)
{
    m_filePath = filePath;
    // Cairo PDF Surface oluşturulacak. Ancak boyut henüz bilinmiyor.
    // İlk SetLogicalPageSize çağrısında oluşturacağız veya burada varsayılan A4 açabiliriz.
    return true;
}

bool CairoDrawContext::EndDocument()
{
    if (m_surface) {
        cairo_surface_finish(m_surface);
        cairo_destroy(m_cr);
        cairo_surface_destroy(m_surface);
        m_cr = nullptr;
        m_surface = nullptr;
    }
    return true;
}

void CairoDrawContext::SetLogicalPageSize(double width, double height)
{
    m_width = width;
    m_height = height;

    // Eğer belge zaten açıksa yeni sayfa, değilse belgeyi oluştur
    if (!m_surface) {
        // Dosya yolunu ANSI'ye çevir (Cairo dosya yolu için char* ister)
        int len = WideCharToMultiByte(CP_ACP, 0, m_filePath, -1, NULL, 0, NULL, NULL);
        std::vector<char> pathA(len);
        WideCharToMultiByte(CP_ACP, 0, m_filePath, -1, pathA.data(), len, NULL, NULL);

        m_surface = cairo_pdf_surface_create(pathA.data(), width, height);
        m_cr = cairo_create(m_surface);
    }
    else {
        // Yeni sayfa
        cairo_show_page(m_cr);
        cairo_pdf_surface_set_size(m_surface, width, height);
        // Context resetlenebilir, font vb. tekrar set etmek gerekebilir.
    }
}

// =========================================================
// AYARLAR
// =========================================================
void CairoDrawContext::SetPen(COLORREF color, float width)
{
    m_penColor = color;
    m_penWidth = width;
    if (m_cr) {
        cairo_set_line_width(m_cr, width);
        // Not: Rengi çizim anında set edeceğiz (Stroke vs Fill farkı için)
    }
}

void CairoDrawContext::SetBrush(COLORREF color)
{
    m_brushColor = color;
}

void CairoDrawContext::SetFont(const FontDesc& f)
{
    if (!m_cr) return;

    // Font ailesini seç (Basit eşleştirme)
    std::string family = WStringToUTF8(f.family);

    cairo_font_slant_t slant = f.italic ? CAIRO_FONT_SLANT_ITALIC : CAIRO_FONT_SLANT_NORMAL;
    cairo_font_weight_t weight = f.bold ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL;

    cairo_select_font_face(m_cr, family.c_str(), slant, weight);
    cairo_set_font_size(m_cr, f.sizePt);

    m_fontSize = f.sizePt;
}

// =========================================================
// ÇİZİM FONKSİYONLARI
// =========================================================
void CairoDrawContext::DrawLine(double x1, double y1, double x2, double y2)
{
    if (!m_cr) return;

    SetSourceColor(m_penColor); // Kalem rengini yükle
    cairo_move_to(m_cr, x1, y1);
    cairo_line_to(m_cr, x2, y2);
    cairo_stroke(m_cr);
}

void CairoDrawContext::DrawRect(double x, double y, double w, double h, bool frameOnly)
{
    if (!m_cr) return;

    if (frameOnly) {
        // Sadece çerçeve
        SetSourceColor(m_penColor);
        cairo_rectangle(m_cr, x, y, w, h);
        cairo_stroke(m_cr);
    }
    else {
        // Dolgu ve Çerçeve
        // 1. Dolgu
        SetSourceColor(m_brushColor);
        cairo_rectangle(m_cr, x, y, w, h);
        cairo_fill_preserve(m_cr); // Yolu koru

        // 2. Çerçeve
        SetSourceColor(m_penColor);
        cairo_stroke(m_cr);
    }
}

void CairoDrawContext::FillRect(double x, double y, double w, double h, COLORREF color)
{
    if (!m_cr) return;

    SetSourceColor(color);
    cairo_rectangle(m_cr, x, y, w, h);
    cairo_fill(m_cr);
}

void CairoDrawContext::DrawTextW(const std::wstring& text, double x, double y, double w, double h, UINT flags)
{
    if (!m_cr || text.empty()) return;

    // Metin Rengi (Genelde Pen rengi ile aynıdır veya Brush, burada Pen kullanıyoruz)
    SetSourceColor(m_penColor); // Veya m_brushColor? GDI'da TextColor ayrıdır ama genelde siyahtır.
    // ThemeConfig'e göre Main Text rengi set edilmeli.

    std::string utf8 = WStringToUTF8(text);

    cairo_text_extents_t extents;
    cairo_text_extents(m_cr, utf8.c_str(), &extents);

    double tx = x;
    double ty = y + m_fontSize; // Cairo baseline kullanır, GDI top-left. Kabaca font boyu kadar aşağı.

    // Yatay Hizalama
    if (flags & DT_CENTER)
        tx = x + (w - extents.width) / 2.0 - extents.x_bearing;
    else if (flags & DT_RIGHT)
        tx = x + w - extents.width - extents.x_bearing;

    // Dikey Hizalama
    if (flags & DT_VCENTER)
        ty = y + (h - extents.height) / 2.0 - extents.y_bearing; // Yaklaşık orta

    cairo_move_to(m_cr, tx, ty);
    cairo_show_text(m_cr, utf8.c_str());
}

void CairoDrawContext::DrawImage(double x, double y, double w, double h, const std::wstring& filePath)
{
    if (!m_cr) return;

    // Dosya yolunu çevir
    std::string path = WStringToUTF8(filePath);

    // PNG yükle (Sadece PNG destekler bu basit örnek)
    cairo_surface_t* image = cairo_image_surface_create_from_png(path.c_str());

    if (cairo_surface_status(image) == CAIRO_STATUS_SUCCESS) {
        int imgW = cairo_image_surface_get_width(image);
        int imgH = cairo_image_surface_get_height(image);

        cairo_save(m_cr);

        // Konuma git ve ölçekle
        cairo_translate(m_cr, x, y);
        cairo_scale(m_cr, w / (double)imgW, h / (double)imgH);

        cairo_set_source_surface(m_cr, image, 0, 0);
        cairo_paint(m_cr);

        cairo_restore(m_cr);
    }

    cairo_surface_destroy(image);
}