#include "stdafx.h"
#include "PreviewPanel.h"
#include "RibbonApp.h"
#include "Mainfrm.h"

// --- YEN MMAR ---
#include "ThemeConfig.h"
#include "LayoutFactory.h"
#include "GdiPlusDrawContext.h"
#include "HaruDrawContext.h"
#include "JsonFormRenderer.h"
#include "RenderConfig.h"

#include <memory>
#include <vector>
#include <gdiplus.h>

#pragma comment (lib,"gdiplus.lib")

using namespace Win32xx;

#define ID_BTN_PREV     3614
#define ID_BTN_NEXT     3615

static ULONG_PTR g_gdiplusToken = 0;



// Kontrat tipi mi? (Sat / Kira / Depozito vs.)
static bool IsContractDoc(PreviewDocType type)
{
    switch (type)
    {
    case DOC_CONTRACT_SALES_AUTH:
    case DOC_CONTRACT_RENTAL_AUTH:
    case DOC_CONTRACT_DEPOSIT:
        return true;
    default:
        return false;
    }
}

// Kontratlar JSON ablonuyla izmeyi dener.
// Baarl olursa true dner, olmazsa false (ve GDI fallback devreye girer).
static bool TryRenderContractWithJson(    IDrawContext& ctx,    const PreviewItem& item,    int pageNo)
{
    // Ayar GDI_ONLY ise hi JSON deneme
    if (g_ContractRenderMode == ContractRenderMode::GDI_ONLY)
        return false;

    // ablon yolu  imdilik docType'a gre sabit isim veriyoruz
    CString jsonPath;

    switch (item.docType)
    {
    case DOC_CONTRACT_SALES_AUTH:
        jsonPath = _T("templates\\satilik_yetki_sozlesmesi.json");
        break;

    case DOC_CONTRACT_RENTAL_AUTH:
        jsonPath = _T("templates\\contract_kira.json");
        break;

    case DOC_CONTRACT_DEPOSIT:
        jsonPath = _T("templates\\contract_depozito.json");
        break;

    default:
        return false; // Kontrat deilse urama
    }

    // Veriyi JsonFormRenderer'n bekledii forma evir (map<CString,CString>)
    std::map<CString, CString> data;
    for (const auto& kv : item.fields)
    {
        // Ayn key birden fazla kez gelirse sonuncusu geerli olur
        data[kv.first] = kv.second;
    }

    // stersen burada ekstra normalize de yapabilirsin:
    // rn: data[_T("OwnerName")] = sahiplere ait isim vs.
    // imdilik ham DB alan adlaryla alyoruz.

    JsonFormRenderer engine;

    // NOT: JsonFormRenderer u an pageNo almyor, tm sayfalar dner.
    // Kontrat iin ok sayfal JSON'a geersen, buraya pageNo destekli
    // yeni bir Render overload' eklemen gerekecek.
    bool ok = engine.Render(ctx, jsonPath, data);

    if (!ok)
    {
        // Burada herhangi bir hata izimi yapmak zorunda deilsin,
        // nk zaten GDI fallback alacak.
        return false;
    }

    // JSON ile render edildi, PreviewPanel GDI'ye gemesin
    return true;
}







CPreviewPanel::~CPreviewPanel()
{
}

void CPreviewPanel::SetPreviewData(const PreviewItem& data)
{
    m_data = data;
    m_currentPage = 1;
    m_zoomPercentage = 100.0; // Başlangıç zoom
    if (IsWindow()) PostMessage(WM_USER + 101);
}

BOOL CPreviewPanel::OnInitDialog()
{
    CDialog::OnInitDialog();

    CreateToolbar();
    CreateStatusBar();
    PostMessage(WM_USER + 101); // İlk çizim

    return TRUE;
}

INT_PTR CPreviewPanel::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    try
    {
        switch (uMsg)
        {
        case WM_USER + 101:
            LayoutChildren();
            UpdatePreview();
            return TRUE;

        case WM_USER + 102: // Zoom değişti mesajı
            UpdateZoomInfo();
            return TRUE;

        case WM_SIZE:
            LayoutChildren();
            return TRUE;

        case WM_KEYDOWN:
            // Klavye kısayolları
            switch (wParam)
            {
            case VK_PRIOR:  // Page Up
                OnBtnPrevPage();
                return TRUE;
            case VK_NEXT:   // Page Down
                OnBtnNextPage();
                return TRUE;
            case VK_HOME:   // İlk sayfa
                if (m_currentPage != 1) {
                    m_currentPage = 1;
                    UpdatePreview();
                    UpdatePageInfo();
                }
                return TRUE;
            case VK_END:    // Son sayfa
                if (m_currentPage != m_totalPages) {
                    m_currentPage = m_totalPages;
                    UpdatePreview();
                    UpdatePageInfo();
                }
                return TRUE;
            case VK_ADD:    // + tuşu (numpad)
            case VK_OEM_PLUS: // + tuşu (klavye)
                if (GetKeyState(VK_CONTROL) & 0x8000) {
                    m_zoomWnd.ZoomIn();
                    UpdateZoomInfo();
                    return TRUE;
                }
                break;
            case VK_SUBTRACT: // - tuşu (numpad)
            case VK_OEM_MINUS: // - tuşu (klavye)
                if (GetKeyState(VK_CONTROL) & 0x8000) {
                    m_zoomWnd.ZoomOut();
                    UpdateZoomInfo();
                    return TRUE;
                }
                break;
            case 'P':       // Ctrl+P = Yazdır
                if (GetKeyState(VK_CONTROL) & 0x8000) {
                    OnPrint();
                    return TRUE;
                }
                break;
            case 'S':       // Ctrl+S = PDF Kaydet
                if (GetKeyState(VK_CONTROL) & 0x8000) {
                    OnSaveAsPDF();
                    return TRUE;
                }
                break;
            }
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
            case IDC_BACKTOLISTVIEW:
                GetContainerApp()->GetMainFrame().SetNormalView();
                return TRUE;

            case ID_BTN_PREV: OnBtnPrevPage(); return TRUE;
            case ID_BTN_NEXT: OnBtnNextPage(); return TRUE;

            case IDC_ZOOMIN:     m_zoomWnd.ZoomIn(); UpdateZoomInfo(); return TRUE;
            case IDC_ZOOMOUT:    m_zoomWnd.ZoomOut(); UpdateZoomInfo(); return TRUE;
            case IDC_BESTFIT:    m_zoomWnd.BestFit(); UpdateZoomInfo(); return TRUE;
            case IDC_ACTUALSIZE: m_zoomWnd.ActualSize(); UpdateZoomInfo(); return TRUE;

            case IDC_NORMAL: OnSaveAsPNG(); return TRUE;

            case IDC_PDF:    OnSaveAsPDF(); return TRUE;

            case IDC_PRINT:  OnPrint(); return TRUE;
            }
            break;
        }
    }
    catch (const std::exception& e)
    {
        CString msg;
        msg.Format(L"Beklenmeyen bir hata olutu:\n%S", e.what());
        MessageBox(msg, L"Hata", MB_ICONERROR);
        return TRUE;
    }

    return DialogProcDefault(uMsg, wParam, lParam);
}

void CPreviewPanel::CreateToolbar()
{
    const UINT buttons[] = {
        IDC_BACKTOLISTVIEW, 0,
        ID_BTN_PREV, ID_BTN_NEXT, 0,
        IDC_ZOOMIN, IDC_ZOOMOUT, IDC_BESTFIT, IDC_ACTUALSIZE, 0,
        IDC_NORMAL, IDC_PDF, IDC_PRINT
    };

    // NOTE: Turkish characters are encoded with explicit Unicode escapes to avoid
    // source encoding/codepage issues.
    int baseString = m_ToolBar.AddStrings(
        L"Geri\0\u00D6nceki\0Sonraki\0Yak\u0131nla\u015Ft\u0131r\0Uzakla\u015Ft\u0131r\0En iyi s\u0131\u011Fd\u0131r\0Ger\u00E7ek boyut\0PNG\0PDF\0Yazd\u0131r\0"
    );

    if (!m_ToolBar.IsWindow()) m_ToolBar.Create(*this);

    CBitmap bm(IDB_TOOLBAR);
    m_ImgList.Create(32, 32, ILC_COLOR32 | ILC_MASK, 0, 0);
    m_ImgList.Add(bm, RGB(255, 0, 255));
    m_ToolBar.SetImageList(m_ImgList);

    std::vector<TBBUTTON> vec;
    int textIndex = 0;

    for (size_t i = 0; i < _countof(buttons); ++i)
    {
        TBBUTTON tb = {};
        tb.iBitmap = (buttons[i] == 0) ? 0 : (int)i;
        tb.idCommand = buttons[i];
        tb.fsState = TBSTATE_ENABLED;
        // Text should appear UNDER the icon (corporate/modern look).
        // Use mixed buttons + SHOWTEXT instead of TBSTYLE_LIST (which places text beside icon).
        tb.fsStyle = (buttons[i] == 0) ? TBSTYLE_SEP : (TBSTYLE_BUTTON | BTNS_SHOWTEXT);

        if (buttons[i] != 0)
            tb.iString = baseString + textIndex++;
        else
            tb.iString = -1;

        vec.push_back(tb);
    }

    m_ToolBar.AddButtons((UINT)vec.size(), vec.data());

    // Enable text under icons.
    DWORD exStyle = (DWORD)m_ToolBar.SendMessage(TB_GETEXTENDEDSTYLE, 0, 0);
    exStyle |= TBSTYLE_EX_MIXEDBUTTONS;
    m_ToolBar.SendMessage(TB_SETEXTENDEDSTYLE, 0, exStyle);
    m_ToolBar.SendMessage(TB_SETMAXTEXTROWS, 1, 0);
    // Slightly larger buttons so labels don't clip.
    m_ToolBar.SendMessage(TB_SETBUTTONSIZE, 0, MAKELPARAM(78, 56));

    // Force-correct captions (in case of encoding issues)
    m_ToolBar.SetButtonText(ID_BTN_PREV,     L"\u00D6nceki");
    m_ToolBar.SetButtonText(ID_BTN_NEXT,     L"Sonraki");
    m_ToolBar.SetButtonText(IDC_ZOOMIN,      L"Yak\u0131nla\u015Ft\u0131r");
    m_ToolBar.SetButtonText(IDC_ZOOMOUT,     L"Uzakla\u015Ft\u0131r");
    m_ToolBar.SetButtonText(IDC_BESTFIT,    L"En iyi s\u0131\u011Fd\u0131r");
    m_ToolBar.SetButtonText(IDC_ACTUALSIZE, L"Ger\u00E7ek boyut");
    m_ToolBar.SetButtonText(IDC_NORMAL,     L"PNG");
    m_ToolBar.SetButtonText(IDC_PDF,        L"PDF");
    m_ToolBar.SetButtonText(IDC_PRINT,      L"Yazd\u0131r");

    m_ToolBar.Autosize();
    m_ToolBar.ShowWindow(SW_SHOW);
}

// ============================================================================
// STATUS BAR - Sayfa ve Zoom Bilgisi için
// ============================================================================
void CPreviewPanel::CreateStatusBar()
{
    if (!m_StatusBar.IsWindow())
    {
        m_StatusBar.Create(*this);
        
        // 3 bölümlü status bar: [Sayfa Bilgisi] [Zoom] [Hazır]
        int parts[] = { 200, 350, -1 };
        m_StatusBar.SetParts(3, parts);
        
        UpdatePageInfo();
        UpdateZoomInfo();
        m_StatusBar.SetPartText(2, L"Haz\u0131r");
    }
}

void CPreviewPanel::UpdatePageInfo()
{
    if (m_StatusBar.IsWindow())
    {
        CString pageInfo;
        pageInfo.Format(L"Sayfa %d / %d", m_currentPage, m_totalPages);
        m_StatusBar.SetPartText(0, pageInfo);
    }
}

void CPreviewPanel::UpdateZoomInfo()
{
    if (m_StatusBar.IsWindow())
    {
        CString zoomInfo = FormatZoomPercentage();
        m_StatusBar.SetPartText(1, zoomInfo);
    }
}

CString CPreviewPanel::FormatZoomPercentage() const
{
    CString result;
    
    // ZoomWnd'den gerçek zoom yüzdesini al
    if (m_zoomWnd.IsWindow() && m_zoomWnd.m_cxImage > 0)
    {
        // Mevcut görüntüleme boyutunu orijinal boyuta bölerek zoom hesapla
        double zoomX = (m_zoomWnd.m_ptszDest.cx * 100.0) / m_zoomWnd.m_cxImage;
        result.Format(L"Yakınlaştırma: %%%d", (int)(zoomX + 0.5));
    }
    else
    {
        result = L"Yakınlaştırma: %100";
    }
    
    return result;
}


void CPreviewPanel::LayoutChildren()
{
    if (!IsWindow()) return;
    CRect rcClient = GetClientRect();

    int tbH = 0;
    if (m_ToolBar.IsWindow()) {
        SIZE sz = m_ToolBar.GetMaxSize();
        tbH = sz.cy;
        m_ToolBar.SetWindowPos(NULL, 0, 0, rcClient.Width(), tbH, SWP_SHOWWINDOW);
    }

    int sbH = 0;
    if (m_StatusBar.IsWindow()) {
        CRect rcStatus = m_StatusBar.GetClientRect();
        sbH = rcStatus.Height();
        m_StatusBar.SetWindowPos(NULL, 0, rcClient.Height() - sbH, 
                                 rcClient.Width(), sbH, SWP_SHOWWINDOW);
    }

    CRect rcZoom(0, tbH, rcClient.Width(), rcClient.Height() - sbH);
    if (!m_zoomWnd.IsWindow()) m_zoomWnd.Create(*this);
    m_zoomWnd.SetWindowPos(NULL, rcZoom, SWP_SHOWWINDOW);
}

// ============================================================================
// SAYFA GEZNTS
// ============================================================================
void CPreviewPanel::OnBtnPrevPage() {
    if (m_currentPage > 1) {
        m_currentPage--;
        UpdatePreview();
        UpdatePageInfo();
    }
}

void CPreviewPanel::OnBtnNextPage() {
    if (m_currentPage < m_totalPages) {
        m_currentPage++;
        UpdatePreview();
        UpdatePageInfo();
    }
}

// ============================================================================
// ZM MOTORU (GDI+)
// ============================================================================
// src/PreviewPanel.cpp iindeki GeneratePreviewBitmap fonksiyonu

HBITMAP CPreviewPanel::GeneratePreviewBitmap()
{
    const int W = 2480; // A4 300 DPI
    const int H = 3508;

    CClientDC dc(*this);
    CBitmap bmp;
    bmp.CreateCompatibleBitmap(dc, W, H);

    CDC memDC;
    memDC.CreateCompatibleDC(dc);
    CBitmap oldBmp = memDC.SelectObject(bmp);

    GdiPlusDrawContext ctx;
    if (ctx.Begin(memDC.GetHDC()))
    {
        try
        {
            // 1. FABRKADAN DORU RESSAMI STE
            // LayoutFactory, JSON dosyas varsa JsonDocumentLayout, yoksa eski GDI snfn verecek.
            std::unique_ptr<IDocumentLayout> layout(LayoutFactory::CreateLayout(m_data.docType));

            if (layout)
            {
                // 2. VERY VER
                layout->SetData(m_data.fields);

                // 3. SAYFA SAYISINI AL
                m_totalPages = layout->GetTotalPages();
                if (m_currentPage > m_totalPages) m_currentPage = m_totalPages;

                // 4. Z (ster JSON olsun ister GDI, komut ayn)
                layout->Render(ctx, m_currentPage);
            }
        }
        catch (...) {}
        ctx.End();
    }

    memDC.SelectObject(oldBmp);
    return (HBITMAP)bmp.Detach();
}
void CPreviewPanel::UpdatePreview()
{
    HBITMAP hBmp = GeneratePreviewBitmap();
    if (hBmp) {
        m_zoomWnd.SetBitmap(hBmp);
        m_zoomWnd.Invalidate();
        UpdatePageInfo();
        UpdateZoomInfo();
    }
}

// ============================================================================
// PDF KAYDETME (Haru - UTF-8 / Türkçe karakter güvenli)
// ============================================================================
void CPreviewPanel::OnSaveAsPDF()
{
    // HATA DZELTME: CString(L"Belge") cast ilemi belirsizlii zer
    CString fileName = m_data.recordCode.IsEmpty() ? CString(L"Belge") : m_data.recordCode;

    CFileDialog dlg(FALSE, L"pdf", fileName, OFN_OVERWRITEPROMPT, L"PDF Dosyas (*.pdf)|*.pdf||");
    if (dlg.DoModal(*this) != IDOK) return;

    HaruDrawContext pdf;

    if (pdf.BeginDocument(dlg.GetPathName()))
    {
        try
        {
            std::unique_ptr<IDocumentLayout> layout(LayoutFactory::CreateLayout(m_data.docType));
            if (layout)
            {
                layout->SetData(m_data.fields);
                int pages = layout->GetTotalPages();

                for (int i = 1; i <= pages; ++i)
                {
                    layout->Render(pdf, i);
                }
            }
            pdf.EndDocument();
            MessageBox(L"PDF Kaydedildi.", L"Bilgi", MB_OK);
        }
        catch (const std::exception& e)
        {
            CString err; err.Format(L"PDF Hatas: %S", e.what());
            MessageBox(err, L"Hata", MB_ICONERROR);
        }
    }
    else
    {
        MessageBox(L"PDF Dosyas oluturulamad (Dosya ak olabilir).", L"Hata", MB_ICONERROR);
    }
}

// ============================================================================
// PNG KAYDETME
// ============================================================================
void CPreviewPanel::OnSaveAsPNG()
{
    // HATA DZELTME: Ambiguous hatas iin CString cast
    CString fileName = m_data.recordCode.IsEmpty() ? CString(L"Resim") : m_data.recordCode;

    CFileDialog dlg(FALSE, L"png", fileName, OFN_OVERWRITEPROMPT, L"PNG Resim (*.png)|*.png||");
    if (dlg.DoModal(*this) != IDOK) return;

    HBITMAP hBmp = GeneratePreviewBitmap();
    if (hBmp) {
        if (SaveBitmapToPNG(hBmp, dlg.GetPathName()))
            MessageBox(L"Resim Kaydedildi.", L"Bilgi", MB_OK);
        else
            MessageBox(L"Resim kaydedilemedi.", L"Hata", MB_ICONERROR);
        DeleteObject(hBmp);
    }
}

bool CPreviewPanel::SaveBitmapToPNG(HBITMAP hBmp, const CString& filePath)
{
    Gdiplus::Bitmap bmp(hBmp, NULL);
    CLSID pngClsid;

    UINT num = 0, size = 0;
    Gdiplus::GetImageEncodersSize(&num, &size);
    if (size == 0) return false;

    std::vector<BYTE> buffer(size);
    Gdiplus::ImageCodecInfo* pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(buffer.data());
    Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

    for (UINT j = 0; j < num; ++j) {
        if (wcscmp(pImageCodecInfo[j].MimeType, L"image/png") == 0) {
            pngClsid = pImageCodecInfo[j].Clsid;
            break;
        }
    }
    return (bmp.Save(filePath, &pngClsid, NULL) == Gdiplus::Ok);
}

// ============================================================================
// YAZDIRMA (PRINT) - PROFESYONEL GELŞTRME
// ============================================================================
void CPreviewPanel::OnPrint()
{
    // Gelişmiş yazdırma diyaloğu - Kopya sayısı, sayfa aralığı vb.
    PRINTDLG pd = { 0 };
    pd.lStructSize = sizeof(pd);
    pd.hwndOwner = *this;
    pd.Flags = PD_RETURNDC | PD_USEDEVMODECOPIESANDCOLLATE | PD_ALLPAGES | PD_NOSELECTION;
    
    // Sayfa aralığı seçeneği ekle
    if (m_totalPages > 1)
    {
        pd.Flags |= PD_PAGENUMS;
        pd.nMinPage = 1;
        pd.nMaxPage = m_totalPages;
        pd.nFromPage = 1;
        pd.nToPage = m_totalPages;
    }
    
    if (!::PrintDlg(&pd))
        return; // Kullanıcı iptal etti

    HDC hPrinterDC = pd.hDC;
    if (!hPrinterDC)
    {
        MessageBox(L"Yazıcı cihazı açılamadı.", L"Hata", MB_ICONERROR);
        return;
    }

    CDC dcPrinter;
    dcPrinter.Attach(hPrinterDC);

    // Yazdırma işi başlat
    CString docTitle = m_data.recordCode.IsEmpty() ? L"Emlak Belgesi" : m_data.recordCode;
    DOCINFO di = { sizeof(DOCINFO) };
    di.lpszDocName = docTitle;
    
    if (dcPrinter.StartDoc(&di) <= 0)
    {
        MessageBox(L"Yazdırma işi başlatılamadı.", L"Hata", MB_ICONERROR);
        return;
    }

    try
    {
        std::unique_ptr<IDocumentLayout> layout(LayoutFactory::CreateLayout(m_data.docType));
        if (!layout)
        {
            throw std::runtime_error("Belge düzeni oluşturulamadı");
        }

        layout->SetData(m_data.fields);
        int totalPages = layout->GetTotalPages();

        // Hangi sayfaları yazdıracağımızı belirle
        int startPage = 1;
        int endPage = totalPages;
        
        if (pd.Flags & PD_PAGENUMS)
        {
            startPage = pd.nFromPage;
            endPage = pd.nToPage;
            // Geçerli aralık kontrolü
            if (startPage < 1) startPage = 1;
            if (endPage > totalPages) endPage = totalPages;
            if (startPage > endPage) startPage = endPage;
        }

        // İlerleme için status bar güncelle
        CString statusMsg;
        statusMsg.Format(L"Yazdırılıyor: Sayfa %d-%d / %d", startPage, endPage, totalPages);
        if (m_StatusBar.IsWindow())
            m_StatusBar.SetPartText(2, statusMsg);

        // Sayfaları yazdır
        for (int i = startPage; i <= endPage; ++i)
        {
            dcPrinter.StartPage();

            // Yazıcı DC'yi kullanarak yüksek kaliteli yazdırma
            // SetLogicalPageSize fonksiyonu otomatik ölçeklendirme yapacak
            GdiPlusDrawContext ctx;
            if (ctx.Begin(hPrinterDC))
            {
                // A4 boyutunda mantıksal sayfa (595x842 point = 210x297 mm)
                ctx.SetLogicalPageSize(2480, 3508); // A4 @ 300 DPI
                
                layout->Render(ctx, i);
                ctx.End();
            }

            dcPrinter.EndPage();

            // İlerleme güncelle
            if (m_StatusBar.IsWindow())
            {
                statusMsg.Format(L"Yazdırılıyor: Sayfa %d / %d", i, endPage);
                m_StatusBar.SetPartText(2, statusMsg);
            }
        }

        dcPrinter.EndDoc();
        
        // Başarı mesajı
        CString successMsg;
        successMsg.Format(L"Yazdırma tamamlandı.\n%d sayfa başarıyla yazdırıldı.", 
                         endPage - startPage + 1);
        MessageBox(successMsg, L"Bilgi", MB_ICONINFORMATION);
        
        if (m_StatusBar.IsWindow())
            m_StatusBar.SetPartText(2, L"Hazır");
    }
    catch (const std::exception& e)
    {
        dcPrinter.AbortDoc();
        
        CString errMsg;
        errMsg.Format(L"Yazdırma sırasında hata oluştu:\n%S", e.what());
        MessageBox(errMsg, L"Hata", MB_ICONERROR);
        
        if (m_StatusBar.IsWindow())
            m_StatusBar.SetPartText(2, L"Hata - Yazdırma iptal edildi");
    }
    catch (...)
    {
        dcPrinter.AbortDoc();
        MessageBox(L"Yazdırma sırasında beklenmeyen bir hata oluştu.", L"Hata", MB_ICONERROR);
        
        if (m_StatusBar.IsWindow())
            m_StatusBar.SetPartText(2, L"Hata - Yazdırma iptal edildi");
    }
}