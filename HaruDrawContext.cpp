#include "stdafx.h"
#include "HaruDrawContext.h"
#include "ThemeConfig.h" // Renk sabitleri için

#include <vector>
#include <shlobj.h>
#pragma comment(lib, "shell32.lib")

// Hata iþleyici (Loglamak isterseniz burayý doldurabilirsiniz)
static void __stdcall ErrorHandler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void* user_data)
{
}

// =========================================================
// YARDIMCI: RENK DÖNÜÞÜMÜ (COLORREF -> LibHaru RGB)
// =========================================================
void SetHaruStrokeColor(HPDF_Page page, COLORREF c)
{
    HPDF_REAL r = (double)GetRValue(c) / 255.0;
    HPDF_REAL g = (double)GetGValue(c) / 255.0;
    HPDF_REAL b = (double)GetBValue(c) / 255.0;
    HPDF_Page_SetRGBStroke(page, r, g, b);
}

void SetHaruFillColor(HPDF_Page page, COLORREF c)
{
    HPDF_REAL r = (double)GetRValue(c) / 255.0;
    HPDF_REAL g = (double)GetGValue(c) / 255.0;
    HPDF_REAL b = (double)GetBValue(c) / 255.0;
    HPDF_Page_SetRGBFill(page, r, g, b);
}

// Sistem Font Yolu
CString GetSystemFontFile(const CString& faceName)
{
    TCHAR szPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_FONTS, NULL, 0, szPath)))
    {
        CString strPath = szPath;
        strPath += _T("\\arial.ttf"); // Varsayýlan olarak Arial

        // Basit eþleþtirme
        if (faceName.Find(_T("Times")) >= 0) strPath = CString(szPath) + _T("\\times.ttf");

        return strPath;
    }
    return _T("C:\\Windows\\Fonts\\arial.ttf");
}

// =========================================================
// CONSTRUCTOR / DESTRUCTOR
// =========================================================

HaruDrawContext::HaruDrawContext()
    : m_pdf(nullptr), m_page(nullptr), m_currentFont(nullptr),
    m_currentFontSize(10.0f), m_logicalW(0), m_logicalH(0)
{
}

HaruDrawContext::~HaruDrawContext()
{
    if (m_pdf) {
        HPDF_Free(m_pdf);
        m_pdf = nullptr;
    }
}

// =========================================================
// STRING DÖNÜÞÜMLERÝ
// =========================================================
std::string HaruDrawContext::WStringToUTF8(const std::wstring& w) const
{
    if (w.empty()) return {};
    int len = ::WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), NULL, 0, NULL, NULL);
    std::string out(len, 0);
    ::WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), &out[0], len, NULL, NULL);
    return out;
}

std::string HaruDrawContext::ConvertToAnsiPath(const CString& path) const
{
    int len = ::WideCharToMultiByte(CP_ACP, 0, path, -1, nullptr, 0, nullptr, nullptr);
    std::string out(len, 0);
    ::WideCharToMultiByte(CP_ACP, 0, path, -1, &out[0], len, nullptr, nullptr);
    if (!out.empty() && out.back() == '\0') out.pop_back();
    return out;
}

double HaruDrawContext::MapY(double y) const
{
    return m_logicalH - y;
}

// =========================================================
// BELGE YÖNETÝMÝ
// =========================================================

bool HaruDrawContext::BeginDocument(const CString& filePath)
{
    m_filePath = filePath;
    m_pdf = HPDF_New(ErrorHandler, nullptr);
    if (!m_pdf) return false;

    // Türkçe karakter desteði için UTF-8
    HPDF_UseUTFEncodings(m_pdf);
    HPDF_SetCurrentEncoder(m_pdf, "UTF-8");
    HPDF_SetCompressionMode(m_pdf, HPDF_COMP_ALL);

    return true;
}

bool HaruDrawContext::EndDocument()
{
    if (!m_pdf) return false;

    // Diske kaydet
    HPDF_SaveToStream(m_pdf);

    // Stream'den dosyaya yaz
    HPDF_UINT size = HPDF_GetStreamSize(m_pdf);
    if (size > 0) {
        std::vector<HPDF_BYTE> buffer(size);
        HPDF_ResetStream(m_pdf);
        HPDF_ReadFromStream(m_pdf, buffer.data(), &size);

        FILE* fp = nullptr;
        if (_wfopen_s(&fp, m_filePath, L"wb") == 0 && fp) {
            fwrite(buffer.data(), 1, size, fp);
            fclose(fp);
        }
    }

    HPDF_Free(m_pdf);
    m_pdf = nullptr;
    return true;
}

// =========================================================
// SAYFA YÖNETÝMÝ
// =========================================================

bool HaruDrawContext::BeginPage(double w, double h)
{
    if (!m_pdf) return false;
    m_page = HPDF_AddPage(m_pdf);
    HPDF_Page_SetWidth(m_page, (HPDF_REAL)w);
    HPDF_Page_SetHeight(m_page, (HPDF_REAL)h);
    return true;
}

void HaruDrawContext::EndPage()
{
    // Haru'da explicit EndPage yok, yeni sayfa eklenince eskisi biter
}

void HaruDrawContext::SetLogicalPageSize(double width, double height)
{
    m_logicalW = width;
    m_logicalH = height;
    if (!m_page) BeginPage(width, height);
}

// =========================================================
// AYARLAR (FONT, PEN, BRUSH)
// =========================================================

void HaruDrawContext::SetFont(const FontDesc& f)
{
    if (!m_pdf) return;

    // Sistem fontunu yükle
    CString fontPath = GetSystemFontFile(f.family.c_str());
    std::string pathA = ConvertToAnsiPath(fontPath);

    const char* fontName = HPDF_LoadTTFontFromFile(m_pdf, pathA.c_str(), HPDF_TRUE);

    if (fontName) {
        m_currentFont = HPDF_GetFont(m_pdf, fontName, "UTF-8");
    }
    else {
        // Bulunamazsa standart font
        m_currentFont = HPDF_GetFont(m_pdf, "Helvetica", "WinAnsiEncoding");
    }

    m_currentFontSize = f.sizePt;
    if (m_page && m_currentFont) {
        HPDF_Page_SetFontAndSize(m_page, m_currentFont, m_currentFontSize);
    }
}

void HaruDrawContext::SetPen(COLORREF c, float width)
{
    if (m_page) {
        SetHaruStrokeColor(m_page, c);
        HPDF_Page_SetLineWidth(m_page, width);
    }
}

void HaruDrawContext::SetBrush(COLORREF c)
{
    if (m_page) {
        SetHaruFillColor(m_page, c);
    }
}

// =========================================================
// ÇÝZÝM FONKSÝYONLARI
// =========================================================

void HaruDrawContext::DrawLine(double x1, double y1, double x2, double y2)
{
    if (!m_page) return;
    HPDF_Page_MoveTo(m_page, (HPDF_REAL)x1, (HPDF_REAL)MapY(y1));
    HPDF_Page_LineTo(m_page, (HPDF_REAL)x2, (HPDF_REAL)MapY(y2));
    HPDF_Page_Stroke(m_page);
}

void HaruDrawContext::DrawRect(double x, double y, double w, double h, bool frameOnly)
{
    if (!m_page) return;

    double py = MapY(y) - h;
    HPDF_Page_Rectangle(m_page, (HPDF_REAL)x, (HPDF_REAL)py, (HPDF_REAL)w, (HPDF_REAL)h);

    if (frameOnly)
        HPDF_Page_Stroke(m_page);
    else {
        // Hem dolgu hem çizgi yapmak için FillStroke kullanýlýr
        // Ama basitçe Fill yeterliyse Fill.
        // Genelde DrawRect dolgusuzdur, ama parametre false ise dolgulu istenir.
        HPDF_Page_Fill(m_page);
    }
}

void HaruDrawContext::FillRect(double x, double y, double w, double h, COLORREF color)
{
    if (!m_page) return;

    // Rengi ayarla
    SetHaruFillColor(m_page, color);

    double py = MapY(y) - h;
    HPDF_Page_Rectangle(m_page, (HPDF_REAL)x, (HPDF_REAL)py, (HPDF_REAL)w, (HPDF_REAL)h);
    HPDF_Page_Fill(m_page);
}

void HaruDrawContext::DrawTextW(const std::wstring& text, double x, double y, double w, double h, UINT flags)
{
    if (!m_page || !m_currentFont || text.empty()) return;

    std::string utf8Text = WStringToUTF8(text);

    // Geniþlik hesabý
    HPDF_REAL tw = HPDF_Page_TextWidth(m_page, utf8Text.c_str());
    HPDF_REAL tx = (HPDF_REAL)x;

    // Yatay Hizalama
    if (flags & DT_CENTER)
        tx = (HPDF_REAL)(x + (w - tw) / 2.0);
    else if (flags & DT_RIGHT)
        tx = (HPDF_REAL)(x + w - tw);

    // Dikey Hizalama
    double ty = MapY(y) - (m_currentFontSize * 0.85);
    if (flags & DT_VCENTER)
        ty = (MapY(y) - h / 2.0) - (m_currentFontSize * 0.35);

    HPDF_Page_BeginText(m_page);
    HPDF_Page_MoveTextPos(m_page, tx, (HPDF_REAL)ty);
    HPDF_Page_ShowText(m_page, utf8Text.c_str());
    HPDF_Page_EndText(m_page);
}

void HaruDrawContext::DrawImage(double x, double y, double w, double h, const std::wstring& filePath)
{
    if (!m_pdf || !m_page) return;

    std::string path = ConvertToAnsiPath(filePath.c_str());

    // Format kontrolü basitçe uzantýya göre yapýlabilir veya ikisi de denenebilir
    HPDF_Image img = HPDF_LoadPngImageFromFile(m_pdf, path.c_str());
    if (!img) img = HPDF_LoadJpegImageFromFile(m_pdf, path.c_str());

    if (img) {
        double py = MapY(y) - h;
        HPDF_Page_DrawImage(m_page, img, (HPDF_REAL)x, (HPDF_REAL)py, (HPDF_REAL)w, (HPDF_REAL)h);
    }
}