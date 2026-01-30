/////////////////////////////
// Files.cpp
//

#include "stdafx.h"
#include "Files.h"
#include "RibbonApp.h"
#include "Mainfrm.h"
#include "Resource.h"
#include "CTreeListVDlg.h"



// NOTE (2026-01-21):
// TreeList (portfolio) navigation is now owned by CContainFiles.
// CListCustomerView never reaches into MainFrame/dockers directly.
// Use UWM_FILES_SHOW_PORTFOLIO / UWM_FILES_SHOW_CUSTOMERS messages instead.

//////////////////////////////////
// CListCustomerView function definitions
//



















LRESULT CInlineEdit::WndProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
    if (msg == WM_KEYDOWN)
    {
        if (wparam == VK_RETURN)
        {
            CString text; 
            text = GetWindowText();
            if (m_pParent)
                m_pParent->OnInlineEditCommit(text);
            Destroy();
            return 0;
        }
        if (wparam == VK_ESCAPE)
        {
            Destroy();
            return 0;
        }
    }
    return CEdit::WndProc(msg, wparam, lparam);
}

// ---- CInlineCombo::WndProc --------------------------------------
LRESULT CInlineCombo::WndProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
    if (msg == WM_KEYDOWN)
    {
        if (wparam == VK_RETURN)
        {
            int sel = GetCurSel();
            CString val; val.Format(L"%d", sel);
            if (m_pParent)
                m_pParent->OnInlineComboCommit(val);
            Destroy();
            return 0;
        }
        if (wparam == VK_ESCAPE)
        {
            Destroy();
            return 0;
        }
    }
    return CComboBox::WndProc(msg, wparam, lparam);
}

void CCustomerInfoPopup::ShowInfo(HWND parent, const std::vector<CString>& headers,
    const std::vector<CString>& values, const POINT& pt,COLORREF rowColor)
{
    if (!::IsWindow(parent))
        return;

    m_headers = headers;
    m_values = values;
    m_rowColor = rowColor;
    // Sadece ilk kez oluşturuluyorsa:
    if (!GetHwnd())
    {
        // Basit sınıf: STATIC işimizi görür. İstersen özel pencere sınıfı da yazabiliriz.
        CWnd::CreateEx(
            WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE, // Z-order ve aktivasyon yok
            _T("STATIC"),
            _T("InfoPopup"),
            WS_POPUP | WS_BORDER,
            0, 0, 0, 0,
            parent,
            0U
        );
    }

    // İçerik boyuna göre yükseklik
    const int height = 25 + (int)m_headers.size() * 22;
    const int width = 280;

    // Konumlandır ve göster (aktive etmeden)
    SetWindowPos(HWND_TOPMOST, pt.x + 15, pt.y + 15, width, height,
        SWP_NOACTIVATE | SWP_SHOWWINDOW);

    m_visible = true;
    RedrawWindow();
}
void CCustomerInfoPopup::Hide()
{
    if (m_visible)
    {
        ShowWindow(SW_HIDE);
        m_visible = false;
    }
}

LRESULT CCustomerInfoPopup::WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps{};
        HDC hdc = BeginPaint(ps);
        Graphics g(hdc);
        g.SetSmoothingMode(SmoothingModeAntiAlias);

        Rect r(0, 0, ps.rcPaint.right - ps.rcPaint.left, ps.rcPaint.bottom - ps.rcPaint.top);

        // 🔹 Arka plan: ListView satır renginden türetilmiş degrade
        Color baseColor(GetRValue(m_rowColor), GetGValue(m_rowColor), GetBValue(m_rowColor));
        Color topColor(
            (BYTE)std::min(GetRValue(m_rowColor) + 15, 255),
            (BYTE)std::min(GetGValue(m_rowColor) + 15, 255),
            (BYTE)std::min(GetBValue(m_rowColor) + 15, 255)
        );

        LinearGradientBrush bg(r, topColor, baseColor, LinearGradientModeVertical);
        g.FillRectangle(&bg, r);

        // 🔹 Dış kenarlık
        Pen borderPen(Color(160, 160, 160), 1.0f);
        g.DrawRectangle(&borderPen, r);

        // 🔹 Yazı fontu ve renkleri
        Gdiplus::Font font(L"Segoe UI", 9);
        SolidBrush textBrush(Color(25, 25, 25));

        // 🔹 Satır ayırıcı çizgi
        Pen linePen(Color(200, 200, 200), 1);

        int y = 10;
        const int rowHeight = 22;
        const int headerWidth = 100;

        for (size_t i = 0; i < m_headers.size() && i < m_values.size(); ++i)
        {
            // Alternatif zemin (hafif çizgili efekt için)
            if (i % 2 == 1)
            {
                SolidBrush altBrush(Color(20, 255, 255, 255)); // yarı saydam beyaz overlay
                g.FillRectangle(&altBrush, 5, y, r.Width - 10, rowHeight);
            }

            CString header = m_headers[i] + _T(":");
            CString value = m_values[i];

            RectF rectHeader(10, (REAL)y, (REAL)headerWidth, (REAL)rowHeight);
            RectF rectValue(15 + headerWidth, (REAL)y, (REAL)(r.Width - headerWidth - 25), (REAL)rowHeight);

            // Yazılar
            g.DrawString(header, -1, &font, rectHeader, NULL, &textBrush);
            g.DrawString(value, -1, &font, rectValue, NULL, &textBrush);

            // Alt çizgi (satır ayırıcı)
            if (i < m_headers.size() - 1)
                g.DrawLine(&linePen, 8, y + rowHeight - 1, r.Width - 8, y + rowHeight - 1);

            y += rowHeight;
        }

        EndPaint(ps);
        return 0;
    }
    }
    return CWnd::WndProcDefault(msg, wparam, lparam);
}






CListCustomerView::~CListCustomerView()
{
    SaveSettings();
    if (IsWindow()) DeleteAllItems();
}

// Insert 4 list view items.
// MEVCUT InsertItems FONKSİYONUNU BUL VE ŞU ŞEKİLDE GÜNCELLE:
void CListCustomerView::InsertItems()
{
    DeleteAllItems();

    if (!db_Manager.IsConnected())
        db_Manager.EnsureConnection();

    std::vector<Customer_cstr> customerList = db_Manager.GetCustomers();
    SetRedraw(FALSE);

    int itemCount = 0;

    for (size_t i = 0; i < customerList.size(); ++i)
    {
        const Customer_cstr& c = customerList[i];

        // ✅ YENİ FİLTRELEME MANTIĞI BURADA BAŞLIYOR
        // Eğer filtre "HEPSI" değilse ve İsim filtrelenen harfle başlamıyorsa bu kaydı atla.
        if (m_currentFilter != _T("HEPSI"))
        {
            CString firstLetter = c.AdSoyad.Left(1);
            // Türkçeye özel karakterleri (İ/I, Ş/S) düşünerek karşılaştırma (Locale duyarlı)
            if (firstLetter.CompareNoCase(m_currentFilter) != 0)
                continue;
        }
        // ✅ FİLTRELEME MANTIĞI SONU


        // ✅ STATUS/KATEGORİ FİLTRESİ (StatusFilterPanel ile senkron)
        // DB'de Calisma_Durumu alanı sayısal status ID olarak tutuluyor (ör: 5001=Randevu).
        // 0 => HEPSİ
        if (m_filterCategory != 0)
        {
            const int st = _ttoi(c.Calisma_Durumu);
            if (st != m_filterCategory)
                continue;
        }

        // ... (Aşağıdaki InsertItem kodların AYNEN kalsın) ...
        // AllocSysString OUTPUT buffer gibi kullanılmamalı (heap corruption/leak).
        int item = InsertItem(itemCount, c.Cari_Kod);
        SetItemText(item, 1, c.AdSoyad);
        // ... (Diğer kolon setleri aynen kalsın) ...
        SetItemText(item, 14, c.Calisma_Durumu);
        SetItemData(item, i);

        itemCount++;
    }

    SetRedraw(TRUE);
    Invalidate();
}




// Files.cpp (CListCustomerView sınıfı içinde)

// Tek bir müşteri yapısını alıp ListView'ın sonuna, tüm 14 sütun ile ekler.
void CListCustomerView::AddItemFromStruct(const Customer_cstr& c)
{
    // ListView'daki mevcut öğe sayısını bul (Listenin sonuna eklemek için)
    int nIndex = GetItemCount();

    // 0. Kolon: Cari Kod (Ana Sütun)
    int item = InsertItem(nIndex, c.Cari_Kod);

    // 1. Kolon: Ad Soyad
    SetItemText(item, 1, c.AdSoyad);

    // 2. Kolon: Telefon
    SetItemText(item, 2, c.Telefon);

    // 3. Kolon: Telefon 2
    SetItemText(item, 3, c.Telefon2);

    // 4. Kolon: E-Posta
    SetItemText(item, 4, c.Email);

    // 5. Kolon: Adres
    SetItemText(item, 5, c.Adres);

    // 6. Kolon: Şehir
    SetItemText(item, 6, c.Sehir);

    // 7. Kolon: İlçe
    SetItemText(item, 7, c.Ilce);

    // 8. Kolon: TCKN
    SetItemText(item, 8, c.TCKN);

    // 9. Kolon: Vergi No
    SetItemText(item, 9, c.VergiNo);

    // 10. Kolon: Müşteri Tipi
    SetItemText(item, 10, c.MusteriTipi);

    // 11. Kolon: Kayıt Tarihi
    SetItemText(item, 11, c.KayitTarihi);

    // 12. Kolon: Durum
    SetItemText(item, 12, c.Durum);

    // 13. Kolon: Notlar (Toplam 14 sütun, 0'dan 13'e kadar)
    SetItemText(item, 13, c.Notlar);

	SetItemText(item, 14, c.Calisma_Durumu);

    // Seçili hale getir ve odağı ver
    SetItemState(item, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    EnsureVisible(item, FALSE);
}


std::vector<CString> CListCustomerView::GetSelectedCariCodes()
{
    std::vector<CString> selectedCodes;
    int nItem = -1; // Aramaya -1'den başlar

    // LVNI_SELECTED bayrağı ile bir sonraki seçili öğeyi bul
    while ((nItem = GetNextItem(nItem, LVNI_SELECTED)) != -1)
    {
        // 0. SubItem (Cari Kod) metnini al
        CString cariKod = GetItemText(nItem, 0);
        if (!cariKod.IsEmpty())
        {
            selectedCodes.push_back(cariKod);
        }
    }

    TRACE(_T("ListView'dan %d adet Cari Kod seçildi.\n"), selectedCodes.size());
    return selectedCodes;
}





void CListCustomerView::OnInitialUpdate()
{
    LoadSettings();
    // Call the base class function.
    m_currentFilter = _T("HEPSI");
    // ✅ SON AYARLARI REGISTRY'DEN ÇEK
    SetRowHeight(65);
    // Varsayılan olarak kart modunu açalım (veya tuttuğunuz değere göre)
    m_viewMode = VIEW_MODE_CARD;

    // Apply initial view mode styling
    ApplyViewMode(m_viewMode);

    // Extended styles are handled in ApplyViewMode.
    // Çift tamponlama (gerekli)
    //SetExtendedStyle(LVS_EX_DOUBLEBUFFER| LVS_EX_FULLROWSELECT);

    //SetDPIImages();
    SetColumns();
    InsertItems();

}

void CListCustomerView::ApplyViewMode(int mode)
{
    if (mode != VIEW_MODE_REPORT && mode != VIEW_MODE_CARD)
        mode = VIEW_MODE_CARD;

    m_viewMode = mode;

    DWORD dwStyle = (DWORD)GetWindowLongPtr(GWL_STYLE);
    dwStyle &= ~(LVS_TYPEMASK);

    if (m_viewMode == VIEW_MODE_CARD)
    {
        dwStyle |= LVS_ICON;
        dwStyle &= ~LVS_NOCOLUMNHEADER;
        SetWindowLongPtr(GWL_STYLE, dwStyle);

        // card spacing
        ListView_SetIconSpacing(GetHwnd(),
            DpiScaleInt(CARD_TOTAL_WIDTH_BASE),
            DpiScaleInt(CARD_TOTAL_HEIGHT_BASE - 3));

        SetExtendedStyle(LVS_EX_DOUBLEBUFFER);
    }
    else
    {
        // Pro report: corporate list (header visible + comfortable row height)
        dwStyle |= LVS_REPORT;
        dwStyle &= ~LVS_NOCOLUMNHEADER;
        SetWindowLongPtr(GWL_STYLE, dwStyle);

        SetExtendedStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT |
                         LVS_EX_GRIDLINES   | LVS_EX_INFOTIP      |
                         LVS_EX_LABELTIP);

        SetRowHeight(DpiScaleInt(64));

        // Show header
        if (HWND hHdr = ListView_GetHeader(GetHwnd()))
            ::ShowWindow(hHdr, SW_SHOW);
}

    Invalidate();
    UpdateWindow();
}

void CListCustomerView::SetFilter(const CString& filterLetter)
{
    m_currentFilter = filterLetter;

    // En stabil yaklaşım: listeyi yeniden doldur
    RefreshCustomerList();
}









LRESULT CListCustomerView::OnMouseActivate(UINT msg, WPARAM wparam, LPARAM lparam)
// Respond to a mouse click on the window
{
    // Set window focus. The docker will now report this as active.
    SetFocus();
    return FinalWindowProc(msg, wparam, lparam);
}

// Called in response to a WM_DPICHANGED_BEFOREPARENT message that is sent to child
// windows after a DPI change. A WM_DPICHANGED_BEFOREPARENT is only received when the
// application is DPI_AWARENESS_PER_MONITOR_AWARE.
LRESULT CListCustomerView::OnDpiChangedBeforeParent(UINT msg, WPARAM wparam, LPARAM lparam)
{
    SetDPIImages();
    SetDPIColumnWidths();
    return FinalWindowProc(msg, wparam, lparam);
}

// Files.cpp -> CListCustomerView::SetColumns

void CListCustomerView::SetColumns()
{
    // Mevcut kolonları temizlemiyoruz, çünkü InsertColumn var olanın üzerine eklemez, sırayla ekler.
    // Ancak temiz bir başlangıç için güvenli yöntem:
    while (DeleteColumn(0));

    LV_COLUMN column{};
    column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    column.fmt = LVCFMT_LEFT;

    const WCHAR* headers[] =
    {
        L"Cari Kod",        // 0
        L"Ad Soyadı",       // 1
        L"Telefon",         // 2
        L"Telefon 2",       // 3
        L"E-Posta",         // 4
        L"Adres",           // 5
        L"Şehir",           // 6
        L"İlçe",            // 7
        L"TCKN",            // 8
        L"Vergi No",        // 9
        L"Müşteri Tipi",    // 10
        L"Kayıt Tarihi",    // 11
        L"Durum",           // 12
        L"Notlar",          // 13 
        L"Çalışma Durumu"   // 14
    };

    // Rapor Modu İçin İdeal Genişlikler
    int reportWidths[] = {
        100, // Cari Kod
        200, // Ad Soyad (Geniş)
        120, // Tel
        120, // Tel2
        180, // E-Posta
        250, // Adres
        100, // Şehir
        100, // İlçe
        100, // TCKN
        100, // Vergi
        100, // Tip
        100, // Tarih
        80,  // Durum
        200, // Notlar
        20   // Çalışma Durumu (Gizli/Küçük)
    };

    for (int i = 0; i < _countof(headers); ++i)
    {
        column.pszText = const_cast<LPWSTR>(static_cast<LPCWSTR>(headers[i]));

        // ✅ DÜZELTME BURADA:
        if (m_viewMode == VIEW_MODE_CARD)
        {
            // Kart modunda sadece ilk kolonun genişliği önemlidir (Kart genişliği kadar)
            // Diğerleri 0 olabilir çünkü görünmezler.
            if (i == 0)
                column.cx = DpiScaleInt(270); // Kart genişliği
            else
                column.cx = 0;
        }
        else // VIEW_MODE_REPORT
        {
            // Rapor modunda gerçek genişlikleri kullan
            if (i < _countof(reportWidths))
                column.cx = DpiScaleInt(reportWidths[i]);
            else
                column.cx = DpiScaleInt(100);
        }

        InsertColumn(i, column);
    }
}

void CListCustomerView::AdjustCustomerColumnWidths()
{
    // Bu metot, SetColumns'taki genişlikleri tekrar ayarlamalıdır.
    // Mevcut kodunuzdaki SetDPIColumnWidths metodu sadece 3 kolonu ayarlıyor, 
    // bunu tüm kolonları ayarlayacak şekilde genişletmeliyiz.
    SetColumnWidth(0, DpiScaleInt(100));
    SetColumnWidth(1, DpiScaleInt(200));
    SetColumnWidth(2, DpiScaleInt(100));
    // ... Diğer kolonlar için de devam etmeli.
}
//void CListCustomerView::OnAttach()
//{
//    CListView::OnAttach();
//    SetDPIImages();
//
//    // Rapor stili zaten ayarlanmış
//    DWORD dwStyle = GetStyle();
//    SetStyle((dwStyle & ~LVS_TYPEMASK) | LVS_REPORT);
//
//    // Yeni, güncellenmiş kolonlarımızı ayarlıyoruz
//    SetColumns();
//    // Veri yüklemesini, MainFrame'den gelecek olan veriyi alan AddCustomerData ile yapacağız.
//}
// Adjusts the listview column widths in response to window DPI changes.
void CListCustomerView::SetDPIColumnWidths()
{
    //SetColumnWidth(0, DpiScaleInt(120));
    //SetColumnWidth(1, DpiScaleInt(50));
    //SetColumnWidth(2, DpiScaleInt(100));
}

// Adjusts the listview image sizes widths in response to window DPI changes.
void CListCustomerView::SetDPIImages()
{
    // Set the image lists
    //CBitmap bmImage(IDC_CMD_ABOUT_LargeImages_RESID);
    //bmImage = DpiScaleUpBitmap(bmImage);
    //int scale = bmImage.GetSize().cy / 15;
    //m_smallImages.Create(scale * 16, scale * 15, ILC_COLOR32 | ILC_MASK, 1, 0);
    //m_smallImages.Add(bmImage, RGB(255, 0, 255));
    //SetImageList(m_smallImages, LVSIL_SMALL);
}
CString CListCustomerView::GetSelectedItemText(int nSubItem)
{
    CString mStr;
    int iSelectedItem = 0;
    iSelectedItem = GetNextItem(-1, LVNI_SELECTED);
    mStr = GetItemText(iSelectedItem, nSubItem, 50);
    return mStr;
};

void CListCustomerView::OnNewCustomer()
{
    // EKLEME MODU: Cari Kod boş (varsayılan) bırakılır.
    CCustomerDialog customerDlg(db_Manager, INEWUSER, _T(""));

    if (customerDlg.DoModal(GetHwnd()) == IDOK)
    {
        // Kayıt başarılıysa, ListView yenileme işlemi dialog içinde yapılmıştır.
        TRACE(_T("Yeni müşteri kaydı başarılı ve ListView güncellendi.\n"));
    }
}

// Mevcut Müşteriyi Güncelleme Başlatma (Sağ tık menüsünden çağrılır)
void CListCustomerView::OnEditCustomer(const CString& cariKod)
{
    if (cariKod.IsEmpty()) return;

    // GÜNCELLEME MODU: Cari Kod, düzenlenecek müşterinin koduyla gönderilir.
    CCustomerDialog customerDlg(db_Manager, IUPDATEUSER, cariKod);

    if (customerDlg.DoModal(GetHwnd()) == IDOK)
    {
        // Güncelleme başarılıysa, ListView yenileme işlemi dialog içinde yapılmıştır.
        TRACE(_T("Müşteri güncelleme başarılı ve ListView güncellendi.\n"));
    }
}
// 👇 BU SATIRI EKLEYİN
#include "SahibindenImportDlg.h"
#include "SahibindenBulkWindow.h"
#include "AppointmentDlg.h"
#include "AppointmentListDlg.h"
#include "vCompanyDlg.h"
BOOL CListCustomerView::OnCommand(WPARAM wparam, LPARAM lParam)
{
	CCompanyDialog mcompanyDlg(db_Manager);
    UINT id = LOWORD(wparam);
    UINT code = HIWORD(wparam);
    HWND hwndCtrl = (HWND)lParam;
    DWORD dwStyle;
    CString cariKod;
    std::vector<CString> codesToDelete;
    CString message;
    TCHAR text[16]{};

    // ======================================================
    // 🟩 INLINE EDIT / COMBO ORTAK YÖNETİM BLOĞU
    // ======================================================

    // 1️⃣ EditBox olayı
    if (hwndCtrl && hwndCtrl == m_editBox.GetHwnd())
    {
        if (code == EN_KILLFOCUS)
        {
            // Fokus kaybında kontrol başka yere geçtiyse
            HWND hFocus = ::GetFocus();
            if (hFocus != GetHwnd() && hFocus != m_comboBox.GetHwnd())
            {
                CString newText = m_editBox.GetWindowText();
                if (m_editItem >= 0 && !newText.IsEmpty())
                {
                    CString cariKod = GetItemText(m_editItem, 0);
                    CString field = GetColumnFieldName(m_editSubItem);
                    SetItemText(m_editItem, m_editSubItem, newText);
                    db_Manager.UpdateCustomerField(cariKod, field, newText);
                }
                if (m_editBox.IsWindow())
                    m_editBox.Destroy();
            }
            return TRUE;
        }
    }

    // 2️⃣ ComboBox olayı
    else if (hwndCtrl && hwndCtrl == m_comboBox.GetHwnd())
    {
        switch (code)
        {
        case CBN_SELCHANGE:
        {
            int sel = m_comboBox.GetCurSel();
            CString newValue;
            newValue.Format(L"%d", sel);

            if (m_editItem >= 0)
            {
                CString cariKod = GetItemText(m_editItem, 0);
                SetItemText(m_editItem, m_editSubItem, newValue);
                db_Manager.UpdateCustomerField(cariKod, L"Calisma_Durumu", newValue);
            }
            break;
        }

        case CBN_KILLFOCUS:
        {
            HWND hFocus = ::GetFocus();
            // Eğer focus editbox veya listview’e döndüyse kapat
            if (hFocus != GetHwnd() && hFocus != m_editBox.GetHwnd())
            {
                if (m_comboBox.IsWindow())
                    m_comboBox.Destroy();
            }
            break;
        }
        }
        return TRUE;
    }

    // ======================================================
    // 🟦 NORMAL MENÜ / SAĞ TIK KOMUTLARI BLOĞU
    // ======================================================
    switch (id)
    {

        // 👇 BURAYI EKLEYİN: Sahibinden İlan İndirme Dialogunu Çağır
    case IDM_IMPORT_SAHIBINDEN:
    {
        static CSahibindenBulkWindow* s_bulk = nullptr;

        if (!s_bulk || !s_bulk->IsWindow())
        {
            s_bulk = new CSahibindenBulkWindow();
            s_bulk->CreateBulkWindow(GetHwnd());   // Create değil!
        }

        s_bulk->ShowWindow(SW_SHOW);
        ::SetForegroundWindow(s_bulk->GetHwnd());
        return TRUE;
    }



        break;

    case IDM_COMPANY_INFO:
		mcompanyDlg.DoModal(GetHwnd());

        break;

    case IDM_GOTOADRESS:
    {
        int iSelected = GetNextItem(-1, LVNI_SELECTED);
        if (iSelected >= 0)
        {
            // Web sütunu numarasını belirt
            const int WEB_COLUMN_INDEX = 5; // kendi sütun numarana göre ayarla
            CString url = GetItemText(iSelected, WEB_COLUMN_INDEX);
            url.Trim();

            if (!url.IsEmpty())
            {
                if (url.Left(4).CompareNoCase(L"http") != 0)
                    url = L"http://" + url;

                HINSTANCE result = ::ShellExecute(NULL, L"open", url, NULL, NULL, SW_SHOWNORMAL);
                if ((INT_PTR)result <= 32)
                {
                    CString msg;
                    msg.Format(L"Bağlantı açılamadı:\n%s", url.c_str());
                    ::MessageBox(GetHwnd(), msg, L"Hata", MB_ICONERROR);
                }
            }
            else
            {
                ::MessageBox(GetHwnd(), L"Bu kayıt için web adresi bulunamadı.", L"Bilgi", MB_ICONINFORMATION);
            }
        }
        return TRUE;
    }
        break;

        // Files.cpp -> OnCommand

    case IDM_VIEW_REPORT:
        m_viewMode = VIEW_MODE_REPORT;

        // 1. Stili değiştir
        dwStyle = (DWORD)GetWindowLongPtr(GWL_STYLE);
        SetWindowLongPtr(GWL_STYLE, (dwStyle & ~LVS_TYPEMASK) | LVS_REPORT);

        // 2. ✅ KOLONLARI GÜNCELLE (Bu satır kritik)
        UpdateColumnWidths();

        // 3. Satır yüksekliğini normale döndür (Image list'i kaldırarak)
        // Eğer 3 satırlı mod için image list eklediyseniz, burada NULL yapmalısınız.
        // ListView_SetImageList(GetHwnd(), NULL, LVSIL_SMALL);

        Invalidate();
        return TRUE;

    case IDM_VIEW_CARD:
        m_viewMode = VIEW_MODE_CARD;

        // 1. Stili değiştir
        dwStyle = (DWORD)GetWindowLongPtr(GWL_STYLE);
        SetWindowLongPtr(GWL_STYLE, (dwStyle & ~LVS_TYPEMASK) | LVS_ICON); // veya LVS_ALIGNTOP

        // 2. Kart moduna uygun genişlik (Sadece ilk kolon)
        UpdateColumnWidths();

        // 3. Kart aralıklarını ayarla
        ListView_SetIconSpacing(GetHwnd(), DpiScaleInt(270), DpiScaleInt(130));

        Invalidate();
        return TRUE;


    case IDM_SAVE_TO_DATABASE:
        OnSaveListToDatabase();
        return TRUE;

    case IDM_SET_CALISIR:
        db_Manager.EnsureConnection();
        cariKod = GetSelectedItemText(0);
        UpdateCustomerStatus(cariKod, IDM_SET_CALISIR);
        SaveSettings();
        break;

    case IDM_SET_CALISMAZ:
        db_Manager.EnsureConnection();
        cariKod = GetSelectedItemText(0);
        UpdateCustomerStatus(cariKod, IDM_SET_CALISMAZ);
        SaveSettings();
        break;

    case IDM_SET_DUSUNECEK:
        db_Manager.EnsureConnection();
        cariKod = GetSelectedItemText(0);
        UpdateCustomerStatus(cariKod, IDM_SET_DUSUNECEK);
        SaveSettings();
        break;
    case IDM_FLT_HEPSI          :         SetCategoryFilter(CAT_HEPSI); return TRUE;
    case IDM_FLT_SICAK_ALICI    :   SetCategoryFilter(CAT_SICAK_ALICI); return TRUE;
    case IDM_FLT_SICAK_SATICI   :  SetCategoryFilter(CAT_SICAK_SATICI); return TRUE;
    case IDM_FLT_POT_ALICI      :     SetCategoryFilter(CAT_POTANSIYEL_ALICI); return TRUE;
    case IDM_FLT_POT_SATICI     :    SetCategoryFilter(CAT_POTANSIYEL_SATICI); return TRUE;
    case IDM_FLT_YATIRIMCI      :     SetCategoryFilter(CAT_YATIRIMCI); return TRUE;
    case IDM_FLT_KIRACI         :        SetCategoryFilter(CAT_KIRACI_ADAYI); return TRUE;
    case IDM_FLT_EV_SAHIBI      :     SetCategoryFilter(CAT_EV_SAHIBI_KIRALIK); return TRUE;
    case IDM_FLT_YENI           :          SetCategoryFilter(CAT_YENI_MUSTERI); return TRUE;
    case IDM_FLT_TAKIP          :         SetCategoryFilter(CAT_TAKIPTE); return TRUE;
    case IDM_FLT_RANDEVU        :       SetCategoryFilter(CAT_RANDEVULU); return TRUE;
    case IDM_FLT_TEKLIF         :        SetCategoryFilter(CAT_TEKLIF_VERDI); return TRUE;
    case IDM_FLT_TAMAMLANDI     :    SetCategoryFilter(CAT_ISLEM_TAMAM); return TRUE;
    case IDM_ADD_HOME:
        cariKod = GetSelectedItemText(0);
        OnAddHome(cariKod);
        break;
    case ID_EKLE_ARSA:
        cariKod = GetSelectedItemText(0);
        OnAddLand(cariKod);
        break;
    case ID_EKLE_BAGVBAHCE:
        cariKod = GetSelectedItemText(0);
        OnAddVineYard(cariKod);
        break;
    case ID_EKLE_TICARIALAN:
        cariKod = GetSelectedItemText(0);
        OnAddCommerical(cariKod);
        break;
    case ID_EKLE_TARLA:
        cariKod = GetSelectedItemText(0);
        OnAddField(cariKod);
        break;
    case IDM_ADD_CAR:
        cariKod = GetSelectedItemText(0);
        OnAddCar(cariKod);
        break;
    case ID_EKLE_VILLA:
        cariKod = GetSelectedItemText(0);
        OnAddVilla(cariKod);
        break;
    case ID_EKLE_MUSTAKILHOME:
        cariKod = GetSelectedItemText(0);
        OnAddVilla(cariKod);
        break;
    case IDM_RANDEVU_OLUSTUR: // Resource.h'da bu ID tanımlı olmalı (Örn: 4001)
    {
        // 1. Seçili Müşteriyi Bul
        int nItem = GetSelectionMark();

        if (nItem != -1)
        {
            // 2. Bilgileri Al (Kolon sırasına dikkat: 0=CariKod, 1=AdSoyad varsayıldı)
            CString custID = GetItemText(nItem, 0);
            CString custName = GetItemText(nItem, 1);

            // 3. Diyaloğu Aç
            CAppointmentDlg dlg(custID, custName);
            dlg.DoModal(GetHwnd());
        }
        else
        {
            MessageBox(_T("Lütfen listeden bir müşteri seçiniz."), _T("Uyarı"), 0);
        }
    }
    break;

    // Mainfrm.cpp -> OnCommand

    case IDM_RANDEVU_GORUNTULE: // (Örn: 1002)
    {
        // Seçili Müşteriyi Bul
        int nItem = GetSelectionMark();
        if (nItem != -1)
        {
            CString custID = GetItemText(nItem, 0);
            CString custName = GetItemText(nItem, 1);

            // Listeyi Aç
            CAppointmentListDlg listDlg(custID, custName);
            listDlg.DoModal(GetHwnd());
        }
    }
    break;
    break;   

    case IDM_NEW_RECORD:
        OnNewCustomer();
        return TRUE;

    case IDM_RECORD_UPDATE:
        OnEditCustomer(GetSelectedItemText(0));
        return TRUE;
    case IDM_FILE_UPLOAD_CLOUD:
        GetContainerApp()->GetMainFrame().        OnCloudSyncTest();
       
        break;
    case IDM_GOOGLE_EXIT:
        MessageBox(_T("Çıkış yapılıyor"), _T(""), 0);
        GetContainerApp()->GetMainFrame().OnLogoutGoogle();
        break;
    case IDM_DELETE_IS_SELECTED:
        db_Manager.EnsureConnection();
        codesToDelete = GetSelectedCariCodes();
        message.Format(L"Seçili %d adet kaydı silmek istediğinizden emin misiniz? Bu işlem geri alınamaz.",
            (int)codesToDelete.size());

        if (::MessageBox(GetHwnd(), message, L"Onay Gerekiyor", MB_YESNO | MB_ICONWARNING) == IDYES)
        {
      if (db_Manager.DeleteCustomers(codesToDelete))
   {
   InsertItems();
                ::MessageBox(GetHwnd(), L"Seçili kayıtlar başarıyla silindi.",
              L"Başarılı", MB_ICONINFORMATION);
   }
        }
        break;
    }

    return FALSE;
}

Customer_cstr CListCustomerView::GetSelectedCustomerData()
{
    Customer_cstr c; // Okunan veriyi tutacak boş yapı

    // 1. Seçili satırın index'ini bul (LVNI_SELECTED)
    // -1'den başlayıp ilk seçili öğeyi arar.
    int iSelectedItem = GetNextItem(-1, LVNI_SELECTED);

    // Seçili öğe yoksa veya geçersizse boş yapıyı döndür
    if (iSelectedItem < 0)
    {
        TRACE(_T("HATA: ListView'da seçili bir öğe bulunamadı.\n"));
        return c;
    }

    // CListCustomerView::GetItemText metodu, Win32++'ta CString döndürür (veya CString& ile doldurur).

    // 2. TÜM SÜTUNLARI TEK TEK OKU VE YAPILANDIR

    // NOT: Müşteri bilgileri, InsertItems'daki gibi 0'dan 13'e kadar sıralanmıştır.

    // 0. Kolon: Cari Kod (Primary Key)
    c.Cari_Kod = GetItemText(iSelectedItem, 0);

    // 1. Kolon: Ad Soyad
    c.AdSoyad = GetItemText(iSelectedItem, 1);

    // 2. Kolon: Telefon
    c.Telefon = GetItemText(iSelectedItem, 2);

    // 3. Kolon: Telefon 2
    c.Telefon2 = GetItemText(iSelectedItem, 3);

    // 4. Kolon: E-Posta
    c.Email = GetItemText(iSelectedItem, 4);

    // 5. Kolon: Adres
    c.Adres = GetItemText(iSelectedItem, 5);

    // 6. Kolon: Şehir
    c.Sehir = GetItemText(iSelectedItem, 6);

    // 7. Kolon: İlçe
    c.Ilce = GetItemText(iSelectedItem, 7);

    // 8. Kolon: TCKN
    c.TCKN = GetItemText(iSelectedItem, 8);

    // 9. Kolon: Vergi No
    c.VergiNo = GetItemText(iSelectedItem, 9);

    // 10. Kolon: Müşteri Tipi
    c.MusteriTipi = GetItemText(iSelectedItem, 10);

    // 11. Kolon: Kayıt Tarihi
    c.KayitTarihi = GetItemText(iSelectedItem, 11);

    // 12. Kolon: Durum
    c.Durum = GetItemText(iSelectedItem, 12);

    // 13. Kolon: Notlar
    c.Notlar = GetItemText(iSelectedItem, 13);

	c.Calisma_Durumu = GetItemText(iSelectedItem, 14);

    TRACE(_T("ListView'dan müşteri verisi başarıyla okundu: %s\n"), c.Cari_Kod);

    return c;
}





VOID CListCustomerView::HandlePopupMenu(int x, int y)
{
    POINT pt = { GET_X_LPARAM(x), GET_Y_LPARAM(y) };
    //CRect rc;
    //rc = GetClientRect();
    TPMPARAMS tpm;
   // GetViewRect(rc);
    tpm.cbSize = sizeof(TPMPARAMS);
    tpm.rcExclude.left = pt.x;
    tpm.rcExclude.top = pt.y;


    CMenu TopMenu(IDR_MENU1); // Menü kaynağınızı kullanın
    CMenu pPopupMenu;
    pPopupMenu = TopMenu.GetSubMenu(0);

    // 🛑 KRİTİK: TPM_LEFTALIGN ve x, y (EKRAN KOORDİNATLARI) kullanılır.
    pPopupMenu.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL,
        x, y, this->GetHwnd(), NULL); // tpm parametresini atlayın veya doğru ayarlayın.
}

// 🛑 CListCustomerView::OnNotifyReflect metodu (SADECE default dönmeli)
LRESULT CListCustomerView::OnNotifyReflect(WPARAM wParam, LPARAM lParam)
{
    LPNMHDR pNMHDR = reinterpret_cast<LPNMHDR>(lParam);

    if (pNMHDR->code == NM_CUSTOMDRAW)
    {
        // Custom Draw'ı doğru Custom Draw işleyicinize yönlendirin
        return OnCustomDraw(reinterpret_cast<NMCUSTOMDRAW*>(lParam));
    }

    switch (pNMHDR->code)
    {
    case LVN_ITEMCHANGED:
        OnItemChanged(reinterpret_cast<NMLISTVIEW*>(lParam));
        return 0;
    }
    // List View'in diğer varsayılan işlemlerine (örneğin seçim) izin vermek için base class'a devret.
    return CListView::OnNotifyReflect(wParam, lParam);
}
// 🛑 CListCustomerView::DrawSubItem metodu (ARTIK GEREKSİZ, KALDIRIN)
// Bu metot artık kullanılmayacak, Report Mode çizimi OnCustomDraw'da inline yapılacak.
// Bu metodu kodunuzdan silebilirsiniz.

#include <algorithm> // dosya başına ekle

void CListCustomerView::DrawSubItem(NMLVCUSTOMDRAW* pLVCD)
{
    const int iItem = (int)pLVCD->nmcd.dwItemSpec;
    const int iSubItem = pLVCD->iSubItem;

    RECT rc{};
    ListView_GetSubItemRect(GetHwnd(), iItem, iSubItem, LVIR_BOUNDS, &rc);

    HDC hdc = pLVCD->nmcd.hdc;
    Graphics g(hdc);
    g.SetSmoothingMode(SmoothingModeAntiAlias);

    const bool selected = (ListView_GetItemState(GetHwnd(), iItem, LVIS_SELECTED) & LVIS_SELECTED) != 0;
    const bool hover = (iItem == m_hoverItem);

    // Dinamik sütun okuma
    CString statusStr = GetItemText(iItem, m_nStatusColumn);
    int statusID = _ttoi(statusStr);

    COLORREF rowColor = RGB(245, 248, 255);
    switch (statusID)
    {
    case 1: rowColor = RGB(255, 210, 210); break; // ayrıldı
    case 2: rowColor = RGB(210, 255, 210); break; // çalışıyor
    case 3: rowColor = RGB(255, 255, 180); break; // izinli
    default: rowColor = (iItem % 2 == 0) ? m_clrRowLight : m_clrRowDark; break;
    }

    if (hover)    rowColor = RGB(220, 235, 255);
    if (selected) rowColor = RGB(180, 210, 255);

    auto clamp0 = [](int v) { return std::max(v, 0); };

    Color top(GetRValue(rowColor), GetGValue(rowColor), GetBValue(rowColor));
    Color bot(clamp0(GetRValue(rowColor) - 15),
        clamp0(GetGValue(rowColor) - 15),
        clamp0(GetBValue(rowColor) - 15));

    LinearGradientBrush brush(
        Rect(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top),
        top, bot, LinearGradientModeVertical);

    const int radius = 8;
    GraphicsPath path;
    Rect r(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
    path.AddArc(r.X, r.Y, radius, radius, 180, 90);
    path.AddArc(r.GetRight() - radius, r.Y, radius, radius, 270, 90);
    path.AddArc(r.GetRight() - radius, r.GetBottom() - radius, radius, radius, 0, 90);
    path.AddArc(r.X, r.GetBottom() - radius, radius, radius, 90, 90);
    path.CloseFigure();

    g.FillPath(&brush, &path);

    Pen borderPen(Color(180, 180, 180));
    g.DrawPath(&borderPen, &path);

    Color textColor(40, 40, 60);
    if (statusID == 1) textColor = Color(160, 0, 0);
    else if (statusID == 2) textColor = Color(0, 100, 0);
    else if (statusID == 3) textColor = Color(130, 110, 0);
    if (selected)           textColor = Color(20, 20, 40);

    CString text = GetItemText(iItem, iSubItem);
    Gdiplus::Font font(L"Segoe UI", 9);
    SolidBrush textBrush(textColor);

    RectF layoutRect((REAL)rc.left + 8, (REAL)rc.top + 2,
        (REAL)(rc.right - rc.left - 10),
        (REAL)(rc.bottom - rc.top - 4));
    g.DrawString(text, -1, &font, layoutRect, nullptr, &textBrush);
}
void CListCustomerView::OnAddHome(const CString& cariKod)
{
    // 1. Müşteri kodu kontrolü
    if (cariKod.IsEmpty())
    {
        ::MessageBox(GetHwnd(),
            _T("Ev eklenemedi çünkü müşteri kodu boş.\nLütfen önce bir müşteri seçin."),
            _T("Eksik Bilgi"), MB_ICONWARNING);
        return;
    }

    // 2. Veritabanı bağlantısını kontrol et
    if (!db_Manager.IsConnected())
    {
        if (!db_Manager.InitializeDatabase())
        {
            ::MessageBox(GetHwnd(),
                _T("Veritabanı bağlantısı kurulamadı.\nEv kaydı başlatılamıyor."),
                _T("Bağlantı Hatası"), MB_ICONERROR);
            return;
        }
    }

    // 3. CHomeDialog'u EKLEME modunda aç
    CHomeDialog homeDlg(db_Manager, INEWUSER, cariKod, _T(""));

    // 4. Dialog açıldı → kullanıcı Kaydet derse kayıt yapılır
    INT_PTR result = homeDlg.DoModal(GetHwnd());

    if (result == IDOK)
    {
        // 5. Başarılı kayıt sonrası listeyi yenile (ileride)
        TRACE(_T("Yeni portföy kaydı başarılı. Müşteri Cari Kod: %s\n"), cariKod);

        // İsteğe bağlı: otomatik yenileme
        // if (auto* pPortView = GetPortfoyoListView())
        //     pPortView->RefreshItems();
    }
    else if (result == IDCANCEL)
    {
        TRACE(_T("Ev ekleme işlemi iptal edildi. Müşteri: %s\n"), cariKod);
    }
    else
    {
        TRACE(_T("Ev ekleme dialogu beklenmedik şekilde kapandı. (Result: %d)\n"), result);
    }
}


void CListCustomerView::OnAddLand(const CString& cariKod)
{
    // 1. Müşteri kodu kontrolü
    if (cariKod.IsEmpty())
    {
        ::MessageBox(GetHwnd(),
            _T("Ev eklenemedi çünkü müşteri kodu boş.\nLütfen önce bir müşteri seçin."),
            _T("Eksik Bilgi"), MB_ICONWARNING);
        return;
    }

    // 2. Veritabanı bağlantısını kontrol et
    if (!db_Manager.IsConnected())
    {
        if (!db_Manager.InitializeDatabase())
        {
            ::MessageBox(GetHwnd(),
                _T("Veritabanı bağlantısı kurulamadı.\nEv kaydı başlatılamıyor."),
                _T("Bağlantı Hatası"), MB_ICONERROR);
            return;
        }
    }

    // 3. CHomeDialog'u EKLEME modunda aç
    CLandDialog LandDlg(db_Manager, INEWUSER, cariKod, _T(""));

    // 4. Dialog açıldı → kullanıcı Kaydet derse kayıt yapılır
    INT_PTR result = LandDlg.DoModal(GetHwnd());

    if (result == IDOK)
    {
        // 5. Başarılı kayıt sonrası listeyi yenile (ileride)
        TRACE(_T("Yeni Arsa kaydı başarılı. Müşteri Cari Kod: %s\n"), cariKod);

        // İsteğe bağlı: otomatik yenileme
        // if (auto* pPortView = GetPortfoyoListView())
        //     pPortView->RefreshItems();
    }
    else if (result == IDCANCEL)
    {
        TRACE(_T("Arsa ekleme işlemi iptal edildi. Müşteri: %s\n"), cariKod);
    }
    else
    {
        TRACE(_T("Arsa ekleme dialogu beklenmedik şekilde kapandı. (Result: %d)\n"), result);
    }
}

void CListCustomerView::OnAddVineYard(const CString& cariKod)
{
    // 1. Müşteri kodu kontrolü
    if (cariKod.IsEmpty())
    {
        ::MessageBox(GetHwnd(),
            _T("Ev eklenemedi çünkü müşteri kodu boş.\nLütfen önce bir müşteri seçin."),
            _T("Eksik Bilgi"), MB_ICONWARNING);
        return;
    }

    // 2. Veritabanı bağlantısını kontrol et
    if (!db_Manager.IsConnected())
    {
        if (!db_Manager.InitializeDatabase())
        {
            ::MessageBox(GetHwnd(),
                _T("Veritabanı bağlantısı kurulamadı.\nEv kaydı başlatılamıyor."),
                _T("Bağlantı Hatası"), MB_ICONERROR);
            return;
        }
    }

    // 3. CHomeDialog'u EKLEME modunda aç
    CVineyardDialog VYDlg(db_Manager, INEWUSER, cariKod, _T(""));

    // 4. Dialog açıldı → kullanıcı Kaydet derse kayıt yapılır
    INT_PTR result = VYDlg.DoModal(GetHwnd());

    if (result == IDOK)
    {
        // 5. Başarılı kayıt sonrası listeyi yenile (ileride)
        TRACE(_T("Yeni Arsa kaydı başarılı. Müşteri Cari Kod: %s\n"), cariKod);

        // İsteğe bağlı: otomatik yenileme
        // if (auto* pPortView = GetPortfoyoListView())
        //     pPortView->RefreshItems();
    }
    else if (result == IDCANCEL)
    {
        TRACE(_T("Arsa ekleme işlemi iptal edildi. Müşteri: %s\n"), cariKod);
    }
    else
    {
        TRACE(_T("Arsa ekleme dialogu beklenmedik şekilde kapandı. (Result: %d)\n"), result);
    }
}




void CListCustomerView::OnAddCommerical(const CString& cariKod)
{
    // 1. Müşteri kodu kontrolü
    if (cariKod.IsEmpty())
    {
        ::MessageBox(GetHwnd(),
            _T("Ev eklenemedi çünkü müşteri kodu boş.\nLütfen önce bir müşteri seçin."),
            _T("Eksik Bilgi"), MB_ICONWARNING);
        return;
    }

    // 2. Veritabanı bağlantısını kontrol et
    if (!db_Manager.IsConnected())
    {
        if (!db_Manager.InitializeDatabase())
        {
            ::MessageBox(GetHwnd(),
                _T("Veritabanı bağlantısı kurulamadı.\nEv kaydı başlatılamıyor."),
                _T("Bağlantı Hatası"), MB_ICONERROR);
            return;
        }
    }

    // 3. CHomeDialog'u EKLEME modunda aç
    CCommercialDialog VYDlg(db_Manager, INEWUSER, cariKod, _T(""));

    // 4. Dialog açıldı → kullanıcı Kaydet derse kayıt yapılır
    INT_PTR result = VYDlg.DoModal(GetHwnd());

    if (result == IDOK)
    {
        // 5. Başarılı kayıt sonrası listeyi yenile (ileride)
        TRACE(_T("Yeni Arsa kaydı başarılı. Müşteri Cari Kod: %s\n"), cariKod);

        // İsteğe bağlı: otomatik yenileme
        // if (auto* pPortView = GetPortfoyoListView())
        //     pPortView->RefreshItems();
    }
    else if (result == IDCANCEL)
    {
        TRACE(_T("Arsa ekleme işlemi iptal edildi. Müşteri: %s\n"), cariKod);
    }
    else
    {
        TRACE(_T("Arsa ekleme dialogu beklenmedik şekilde kapandı. (Result: %d)\n"), result);
    }
}


void CListCustomerView::OnAddField(const CString& cariKod)
{
    // 1. Müşteri kodu kontrolü
    if (cariKod.IsEmpty())
    {
        ::MessageBox(GetHwnd(),
            _T("Ev eklenemedi çünkü müşteri kodu boş.\nLütfen önce bir müşteri seçin."),
            _T("Eksik Bilgi"), MB_ICONWARNING);
        return;
    }

    // 2. Veritabanı bağlantısını kontrol et
    if (!db_Manager.IsConnected())
    {
        if (!db_Manager.InitializeDatabase())
        {
            ::MessageBox(GetHwnd(),
                _T("Veritabanı bağlantısı kurulamadı.\nEv kaydı başlatılamıyor."),
                _T("Bağlantı Hatası"), MB_ICONERROR);
            return;
        }
    }

    // 3. CHomeDialog'u EKLEME modunda aç
    CFieldDialog VYDlg(db_Manager, INEWUSER, cariKod, _T(""));

    // 4. Dialog açıldı → kullanıcı Kaydet derse kayıt yapılır
    INT_PTR result = VYDlg.DoModal(GetHwnd());

    if (result == IDOK)
    {
        // 5. Başarılı kayıt sonrası listeyi yenile (ileride)
        TRACE(_T("Yeni Arsa kaydı başarılı. Müşteri Cari Kod: %s\n"), cariKod);

        // İsteğe bağlı: otomatik yenileme
        // if (auto* pPortView = GetPortfoyoListView())
        //     pPortView->RefreshItems();
    }
    else if (result == IDCANCEL)
    {
        TRACE(_T("Arsa ekleme işlemi iptal edildi. Müşteri: %s\n"), cariKod);
    }
    else
    {
        TRACE(_T("Arsa ekleme dialogu beklenmedik şekilde kapandı. (Result: %d)\n"), result);
    }
}


void CListCustomerView::OnAddCar(const CString& cariKod)
{
    if (cariKod.IsEmpty())
    {
        MessageBox(_T("Lütfen önce bir müşteri seçin."), _T("Uyarı"), MB_ICONWARNING);
        return;
    }

    DatabaseManager& db = DatabaseManager::GetInstance();
    CCarDialog dlg(db, INEWUSER, cariKod);
    if (dlg.DoModal(GetHwnd()) == IDOK)
    {
        OpenCustomerPropertiesInTreeList(cariKod);
    }
}



void CListCustomerView::OnAddVilla(const CString& cariKod)
{
    // 1. Müşteri kodu kontrolü
    if (cariKod.IsEmpty())
    {
        ::MessageBox(GetHwnd(),
            _T("Ev eklenemedi çünkü müşteri kodu boş.\nLütfen önce bir müşteri seçin."),
            _T("Eksik Bilgi"), MB_ICONWARNING);
        return;
    }

    // 2. Veritabanı bağlantısını kontrol et
    if (!db_Manager.IsConnected())
    {
        if (!db_Manager.InitializeDatabase())
        {
            ::MessageBox(GetHwnd(),
                _T("Veritabanı bağlantısı kurulamadı.\nEv kaydı başlatılamıyor."),
                _T("Bağlantı Hatası"), MB_ICONERROR);
            return;
        }
    }

    // 3. CHomeDialog'u EKLEME modunda aç
    CVillaDialog VYDlg(db_Manager, INEWUSER, cariKod, _T(""));

    // 4. Dialog açıldı → kullanıcı Kaydet derse kayıt yapılır
    INT_PTR result = VYDlg.DoModal(GetHwnd());

    if (result == IDOK)
    {
        // 5. Başarılı kayıt sonrası listeyi yenile (ileride)
        TRACE(_T("Yeni Arsa kaydı başarılı. Müşteri Cari Kod: %s\n"), cariKod);

        // İsteğe bağlı: otomatik yenileme
        // if (auto* pPortView = GetPortfoyoListView())
        //     pPortView->RefreshItems();
    }
    else if (result == IDCANCEL)
    {
        TRACE(_T("Arsa ekleme işlemi iptal edildi. Müşteri: %s\n"), cariKod);
    }
    else
    {
        TRACE(_T("Arsa ekleme dialogu beklenmedik şekilde kapandı. (Result: %d)\n"), result);
    }
}



//void CListCustomerView::OnAddMustakilHome(const CString& cariKod)
//{
//    // 1. Müşteri kodu kontrolü
//    if (cariKod.IsEmpty())
//    {
//        ::MessageBox(GetHwnd(),
//            _T("Ev eklenemedi çünkü müşteri kodu boş.\nLütfen önce bir müşteri seçin."),
//            _T("Eksik Bilgi"), MB_ICONWARNING);
//        return;
//    }
//
//    // 2. Veritabanı bağlantısını kontrol et
//    if (!db_Manager.IsConnected())
//    {
//        if (!db_Manager.InitializeDatabase())
//        {
//            ::MessageBox(GetHwnd(),
//                _T("Veritabanı bağlantısı kurulamadı.\nEv kaydı başlatılamıyor."),
//                _T("Bağlantı Hatası"), MB_ICONERROR);
//
//
//            return;
//        }
//    }
//
//    // 3. CHomeDialog'u EKLEME modunda aç
//    CVillaDialog VYDlg(db_Manager, INEWUSER, cariKod, _T(""));
//
    // 4. Dialog açıldı → kullanıcı Kaydet derse kayıt yapılır
//    INT_PTR result = VYDlg.DoModal(GetHwnd());
//
//    if (result == IDOK)
//    {
//        // 5. Başarılı kayıt sonrası listeyi yenile (ileride)
//        TRACE(_T("Yeni Arsa kaydı başarılı. Müşteri Cari Kod: %s\n"), cariKod);
//
//        // İsteğe bağlı: otomatik yenileme
//        // if (auto* pPortView = GetPortfoyoListView())
//        //     pPortView->RefreshItems();
//    }
//    else if (result == IDCANCEL)
//    {
//        TRACE(_T("Arsa ekleme işlemi iptal edildi. Müşteri: %s\n"), cariKod);
//    }
//    else
//    {
//        TRACE(_T("Arsa ekleme dialogu beklenmedik şekilde kapandı. (Result: %d)\n"), result);
//    }
//}
//


















void CListCustomerView::ExportToExcelCSV(const CString& filePath)
{
    HANDLE hFile = ::CreateFile(filePath,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        ::MessageBox(GetHwnd(), L"Dosya oluşturulamadı!", L"Hata", MB_ICONERROR);
        return;
    }

    // 🔹 UTF-16 BOM yaz (Excel otomatik algılar)
    const WORD bom = 0xFEFF;
    DWORD written = 0;
    ::WriteFile(hFile, &bom, sizeof(bom), &written, nullptr);

    HWND hHeader = ListView_GetHeader(GetHwnd());
    if (!hHeader)
    {
        ::CloseHandle(hFile);
        return;
    }

    const int colCount = Header_GetItemCount(hHeader);
    const int rowCount = GetItemCount();

    CStringW line;

    // 🔹 Başlık satırı
    for (int c = 0; c < colCount; ++c)
    {
        wchar_t buf[256] = { 0 };
        HDITEMW hdi{};
        hdi.mask = HDI_TEXT;
        hdi.pszText = buf;
        hdi.cchTextMax = ARRAYSIZE(buf);
        Header_GetItem(hHeader, c, &hdi);

        line += L"\"" + CStringW(buf) + L"\"";
        if (c < colCount - 1)
            line += L",";
    }
    line += L"\r\n";

    // 🔸 Yaz: karakter uzunluğunu bayta çevir
    ::WriteFile(hFile, line,
        static_cast<DWORD>(line.GetLength() * sizeof(wchar_t)),
        &written, nullptr);

    // 🔹 Tüm satırları yaz
    for (int i = 0; i < rowCount; ++i)
    {
        line.Empty();
        for (int c = 0; c < colCount; ++c)
        {
            CStringW text = GetItemText(i, c);
            text.Replace(L"\"", L"\"\"");
            line += L"\"" + text + L"\"";
            if (c < colCount - 1)
                line += L",";
        }
        line += L"\r\n";

        ::WriteFile(hFile, line,
            static_cast<DWORD>(line.GetLength() * sizeof(wchar_t)),
            &written, nullptr);
    }

    ::CloseHandle(hFile);

    CString msg;
    msg.Format(L"%d kayıt başarıyla kaydedildi:\n%s", rowCount, filePath);
    ::MessageBox(GetHwnd(), msg, L"Excel'e Aktarıldı", MB_ICONINFORMATION);
}


void CListCustomerView::ImportFromExcelCSV(const CString& filePath)
{
    HANDLE hFile = ::CreateFile(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        ::MessageBox(GetHwnd(), L"Dosya açılamadı!", L"Hata", MB_ICONERROR);
        return;
    }

    DWORD fileSize = ::GetFileSize(hFile, nullptr);
    if (fileSize == INVALID_FILE_SIZE || fileSize == 0)
    {
        ::CloseHandle(hFile);
        ::MessageBox(GetHwnd(), L"Dosya boş!", L"Hata", MB_ICONERROR);
        return;
    }

    std::vector<BYTE> buffer(fileSize + 2);
    DWORD bytesRead = 0;
    if (!::ReadFile(hFile, buffer.data(), fileSize, &bytesRead, nullptr))
    {
        ::CloseHandle(hFile);
        ::MessageBox(GetHwnd(), L"Dosya okunamadı!", L"Hata", MB_ICONERROR);
        return;
    }
    ::CloseHandle(hFile);

    WCHAR* textData = reinterpret_cast<WCHAR*>(buffer.data());
    int wcharCount = bytesRead / sizeof(WCHAR);

    if (wcharCount > 0 && textData[0] == 0xFEFF)
    {
        textData++;
        wcharCount--;
    }

    std::wstring content(textData, wcharCount);
    std::wstringstream ss(content);

    std::wstring line;
    bool firstLine = true;
    int rowIndex = 0;

    // ✅ Kolonlar oluşturulmamışsa oluştur
    if (Header_GetItemCount(ListView_GetHeader(GetHwnd())) == 0)
        SetColumns();

    DeleteAllItems();

    while (std::getline(ss, line))
    {
        if (!line.empty() && line.back() == L'\r')
            line.pop_back();

        if (line.empty())
            continue;

        if (firstLine)
        {
            firstLine = false;
            continue; // başlık
        }

        std::vector<std::wstring> columns;
        std::wstring token;
        bool insideQuotes = false;

        for (wchar_t ch : line)
        {
            if (ch == L'\"')
                insideQuotes = !insideQuotes;
            else if (ch == L',' && !insideQuotes)
            {
                columns.push_back(token);
                token.clear();
            }
            else
                token += ch;
        }
        if (!token.empty())
            columns.push_back(token);

        if (!columns.empty())
        {
            int item = InsertItem(rowIndex, columns[0].c_str());
            if (item != -1)
            {
                for (size_t c = 1; c < columns.size(); ++c)
                    SetItemText(item, static_cast<int>(c), columns[c].c_str());
                ++rowIndex;
            }
        }
    }

    Invalidate();

    CString msg;
    msg.Format(L"Toplam %d kayıt başarıyla yüklendi.", rowIndex);
    ::MessageBox(GetHwnd(), msg, L"Excel'den Yüklendi", MB_ICONINFORMATION);
}




void CListCustomerView::OnSaveListToDatabase()
{
    CFileDialog dlg(TRUE, L"mdb", NULL,
        OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST,
        L"Access Veritabanı (*.mdb;*.accdb)\0*.mdb;*.accdb\0Tüm Dosyalar (*.*)\0*.*\0\0");

    if (dlg.DoModal() != IDOK)
        return;

    CString dbPath = dlg.GetPathName();
    db_Manager.SetDatabasePath(dbPath);

    if (!db_Manager.EnsureConnection())
    {
        ::MessageBox(GetHwnd(), L"Veritabanı bağlantısı kurulamadı.", L"Hata", MB_ICONERROR);
        return;
    }

    int itemCount = GetItemCount();
    if (itemCount == 0)
    {
        ::MessageBox(GetHwnd(), L"Listede veri yok.", L"Bilgi", MB_ICONINFORMATION);
        return;
    }

    std::vector<Customer_cstr> customers;
    customers.reserve(itemCount);

    for (int i = 0; i < itemCount; ++i)
    {
        Customer_cstr c;
        c.Cari_Kod = GetItemText(i, 0);
        c.AdSoyad = GetItemText(i, 1);
        c.Telefon = GetItemText(i, 2);
        c.Telefon2 = GetItemText(i, 3);
        c.Email = GetItemText(i, 4);
        c.Adres = GetItemText(i, 5);
        c.Sehir = GetItemText(i, 6);
        c.Ilce = GetItemText(i, 7);
        c.TCKN = GetItemText(i, 8);
        c.VergiNo = GetItemText(i, 9);
        c.MusteriTipi = GetItemText(i, 10);
        c.KayitTarihi = GetItemText(i, 11);
        c.Durum = GetItemText(i, 12);
        c.Notlar = GetItemText(i, 13);
        c.Calisma_Durumu = GetItemText(i, 14);
        customers.push_back(c);
    }

    // Veritabanına aktar
    if (db_Manager.InsertCustomerBatch(customers))
    {
        CString msg;
        msg.Format(L"%d kayıt başarıyla eklendi.", itemCount);
        ::MessageBox(GetHwnd(), msg, L"Başarılı", MB_ICONINFORMATION);
    }
    else
    {
        ::MessageBox(GetHwnd(), L"Bazı kayıtlar eklenemedi. Loglara bakın.", L"Hata", MB_ICONWARNING);
    }
}


CString CListCustomerView::GetColumnFieldName(int subItem)
{
    switch (subItem)
    {
    case 0: return L"Cari_Kod";
    case 1: return L"AdSoyad";
    case 2: return L"Telefon";
    case 3: return L"Telefon2";
    case 4: return L"Email";
    case 5: return L"Adres";
    case 6: return L"Sehir";
    case 7: return L"Ilce";
    case 8: return L"TCKN";
    case 9: return L"VergiNo";
    case 10: return L"MusteriTipi";
    case 11: return L"KayitTarihi";
    case 12: return L"Durum";
    case 13: return L"Notlar";
    case 14: return L"Calisma_Durumu";
    default: return L"";
    }
}
void CListCustomerView::OnInlineEditCommit(const CString& newText)
{
    if (m_editItem < 0)
        return;

    // Dinamik olarak sütun al
    int subItem = GetSelectedSubItem();
    if (subItem < 0)
        subItem = m_editSubItem; // fallback

    CString cariKod = GetItemText(m_editItem, 0);
    CString field = GetColumnFieldName(subItem);

    if (field.IsEmpty())
    {
        TRACE(L"⚠️ Geçersiz sütun indexi: %d\n", subItem);
        return;
    }

    SetItemText(m_editItem, subItem, newText);

    bool ok = db_Manager.UpdateCustomerField(cariKod, field, newText);

    TRACE(L"✅ Inline edit commit: Satır=%d, Sütun=%d, Alan=%s, Değer=%s, Sonuç=%d\n",
        m_editItem, subItem, field.GetString(), newText.GetString(), ok);
}

void CListCustomerView::OnInlineComboCommit(const CString& newValue)
{
    if (m_editItem < 0 || m_editSubItem < 0)
        return;

    CString cariKod = GetItemText(m_editItem, 0);  // Cari Kod
    CString field = L"Calisma_Durumu";             // Kolon sabit: 14. sütun

    // ListView hücresini güncelle
    SetItemText(m_editItem, m_editSubItem, newValue);

    // Veritabanını güncelle
    db_Manager.UpdateCustomerField(cariKod, field, newValue);

    TRACE(L"Inline combo commit: Satır=%d, Sütun=%d, Yeni Değer=%s\n",
        m_editItem, m_editSubItem, newValue);
}


void CListCustomerView::HandleMouseHoverPopup(WPARAM wparam, LPARAM lparam)
{
    // 1. Shift Basılı Değilse -> Popup Gizle ve Çık
    if (!(GetKeyState(VK_SHIFT) & 0x8000))
    {
        m_infoPopup.Hide();
        // Eğer hover item değişirse repaint yapmak gerekebilir
        if (m_hoverItem != -1) {
            m_hoverItem = -1;
            Invalidate();
        }
        return;
    }

    // 2. Mouse Koordinatını Al
    POINT pt;
    GetCursorPos(&pt); // Ekran koordinatı
    POINT ptClient = pt;
    ::ScreenToClient(GetHwnd(), &ptClient); // İstemci koordinatı

    // 3. İLK DENEME: Standart HitTest
    LVHITTESTINFO ht = { 0 };
    ht.pt = ptClient;
    int index = ListView_HitTest(GetHwnd(), &ht);

    // 4. İKİNCİ DENEME: Manuel Hesaplama (Sadece Kart Modunda)
    if (index == -1 && GetViewMode() == 1) // 1 = VIEW_MODE_CARD
    {
        CRect rcCheck;
        const int TOTAL_CELL_HEIGHT = DpiScaleInt(130); // Kart yüksekliği
        int count = GetItemCount();

        // Performans için sadece görünür aralığı taramak daha iyidir
        // int first = ListView_GetTopIndex(GetHwnd());
        // int perPage = ListView_GetCountPerPage(GetHwnd());
        // int last = min(count, first + perPage + 2);

        // Şimdilik güvenli olması için tümünü tarıyoruz (Count az ise sorun olmaz)
        for (int i = 0; i < count; ++i)
        {
            if (ListView_GetItemRect(GetHwnd(), i, &rcCheck, LVIR_BOUNDS))
            {
                rcCheck.bottom = rcCheck.top + TOTAL_CELL_HEIGHT; // Rect'i genişlet
                if (rcCheck.PtInRect(ptClient))
                {
                    index = i;
                    break;
                }
            }
        }
    }

    // 5. Durum Değişimi Kontrolü
    if (index != m_hoverItem)
    {
        m_hoverItem = index;
        Invalidate(); // Hover efektini çizmek için

        if (index >= 0)
        {
            std::vector<CString> headers;
            std::vector<CString> values;
            HWND hHeader = ListView_GetHeader(GetHwnd());

            if (hHeader)
            {
                int colCount = Header_GetItemCount(hHeader);
                for (int i = 0; i < colCount; ++i)
                {
                    TCHAR buf[256] = { 0 };
                    HDITEM hdi = { 0 };
                    hdi.mask = HDI_TEXT;
                    hdi.pszText = buf;
                    hdi.cchTextMax = ARRAYSIZE(buf);
                    Header_GetItem(hHeader, i, &hdi);

                    headers.push_back(buf);
                    values.push_back(GetItemText(index, i));
                }
            }

            // Renk Hesaplama (Duruma Göre)
            // Not: Kolon indeksi (14) şemanıza göre doğru olmalı.
            CString statusStr = GetItemText(index, 14);
            int statusID = _ttoi(statusStr);
            COLORREF rowColor = RGB(245, 248, 255); // Varsayılan

            switch (statusID)
            {
            case 1: rowColor = RGB(255, 210, 210); break; // Kırmızımsı
            case 2: rowColor = RGB(210, 255, 210); break; // Yeşilimsi
            case 3: rowColor = RGB(255, 255, 180); break; // Sarımsı
            default:
                // Zebra deseni
                rowColor = (index % 2 == 0) ? RGB(255, 255, 255) : RGB(245, 245, 245);
                break;
            }

            // Popup'ı Göster
            // pt (ekran koordinatı) kullanıyoruz
            m_infoPopup.ShowInfo(GetHwnd(), headers, values, pt, rowColor);
        }
        else
        {
            // Boş alana gelindi
            m_infoPopup.Hide();
        }
    }
}
int CListCustomerView::GetSelectedSubItem() const
{
    // Geçerli mouse konumundan sütun tespiti
    POINT pt;
    ::GetCursorPos(&pt);
    ::ScreenToClient(GetHwnd(), &pt);

    LVHITTESTINFO ht = {};
    ht.pt = pt;
    ListView_SubItemHitTest(GetHwnd(), &ht);

    return (ht.iSubItem >= 0) ? ht.iSubItem : -1;
}


LRESULT CListCustomerView::WndProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
    // Sabitleri ve değişkenleri baştan tanımlayalım
    const int CARD_HEIGHT = DpiScaleInt(130); // Kart yüksekliği
    LRESULT result = 0;

    switch (msg)
    {
    case WM_SIZE:
        // Layout güncellemesi gerekiyorsa buraya eklenebilir
        break;

    case UWM_CUSTOMER_SELECTED:
    {
        // 1. Seçili Öğeyi Bul
        int nItem = GetNextItem(-1, LVNI_SELECTED);
        CString cariKod = _T("");

        if (nItem != -1)
        {
            // 2. Cari Kodu Çek (0. Kolon varsayıyoruz)
            cariKod = GetItemText(nItem, 0);
        }

        // 3. Veriyi Paketle ve Gönder
        if (!cariKod.IsEmpty())
        {
            COPYDATASTRUCT cds{};
            cds.dwData = 1;
            cds.cbData = (cariKod.GetLength() + 1) * sizeof(TCHAR);
            // WM_COPYDATA sender tarafındaki buffer çağrı boyunca geçerli olmalı.
            // SendMessage senkron olduğu için stack'teki CString buffer güvenlidir.
            cds.lpData = (void*)(LPCTSTR)cariKod;

            ::SendMessage(GetParent(), UWM_CUSTOMER_SELECTED, 0, (LPARAM)&cds);
        }
        return 0;
    }

    case WM_KEYDOWN:
        result = HandleKeyDown(wparam);
        if (result != -1) return result;
        break;

    case WM_SETFOCUS:
    case WM_KILLFOCUS:
    {
        // Odak değiştiğinde seçili satırın rengini (Turkuaz <-> Gri) güncellemek için yeniden çiz
        int nItem = GetNextItem(-1, LVNI_SELECTED);
        if (nItem != -1)
        {
            CRect rcItem;
            if (ListView_GetItemRect(GetHwnd(), nItem, &rcItem, LVIR_BOUNDS))
            {
                if (GetViewMode() == 1) rcItem.bottom = rcItem.top + CARD_HEIGHT;
                InvalidateRect(rcItem, FALSE);
            }
        }

        // Eğer odak edit/combo'ya gittiyse işlem yapma (Varsayılan davranış)
        HWND hFocus = ::GetFocus();
        if (msg == WM_KILLFOCUS && (hFocus == m_editBox.GetHwnd() || hFocus == m_comboBox.GetHwnd()))
            return 0;

        break;
    }
    
    case WM_LBUTTONUP:
        result = HandleLButtonUp(lparam);
        if (result != -1) return result;
		break;


    case WM_LBUTTONDOWN:
        result = HandleLButtonDown(lparam);
        if (result != -1) return result;
        break;

    case WM_RBUTTONDOWN:
        result = HandleRButtonDown(lparam); // ✅ Yeni fonksiyon çağrısı
        if (result != -1) return result;
        break;

    case WM_LBUTTONDBLCLK:
        result = HandleLButtonDblClk(lparam);
        if (result != -1) return result;
        break;

    case WM_MOUSEMOVE:
    {
        // Mouse Tracking'i sadece GEREKTİĞİNDE başlat
        if (!m_bTrackingMouse) // Sınıfa m_bTrackingMouse adında bool üye eklemeniz iyi olur
        {
            TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT), TME_LEAVE, GetHwnd(), 0 };
            if (::_TrackMouseEvent(&tme))
            {
                m_bTrackingMouse = true;
            }
        }

        HandleMouseHoverPopup(wparam, lparam);
        return WndProcDefault(msg, wparam, lparam);
    }

    case WM_MOUSELEAVE:
    {
        m_bTrackingMouse = false; // Takibi sıfırla

        if (m_hoverItem != -1)
        {
            m_hoverItem = -1;
            Invalidate(); // Hover efektini sil
        }
        m_infoPopup.Hide();
        return 0;
    }

    case WM_CONTEXTMENU:
    {
        // 1. Ekran Koordinatını Al
        CPoint ptScreen(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
        if (ptScreen.x == -1 && ptScreen.y == -1)
            GetCursorPos(&ptScreen); // Klavye ile açıldıysa

        // 2. İstemci Koordinatını Al (HitTest İçin)
        CPoint ptClient = ptScreen;
        ScreenToClient(ptClient);

        // 3. HİTTEST (Hibrit Mantık)
        int nItem = -1;
        LVHITTESTINFO lvhti = { 0 };
        lvhti.pt = ptClient;
        ListView_HitTest(GetHwnd(), &lvhti);
        nItem = lvhti.iItem;

        // Standart HitTest bulamadıysa Manuel Ara
        if (nItem == -1 && GetViewMode() == 1)
        {
            CRect rcCheck;
            int count = GetItemCount();
            for (int i = 0; i < count; ++i)
            {
                if (ListView_GetItemRect(GetHwnd(), i, &rcCheck, LVIR_BOUNDS))
                {
                    rcCheck.bottom = rcCheck.top + CARD_HEIGHT;
                    if (rcCheck.PtInRect(ptClient))
                    {
                        nItem = i;
                        break;
                    }
                }
            }
        }

        // 4. SEÇİMİ YAP (Eğer bir öğeye sağ tıklandıysa)
        if (nItem != -1)
        {
            // Eski seçimi temizle ve yenisini seç
            DeleteAllSelections();
            ListView_SetItemState(GetHwnd(), nItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

            // Görseli Yenile (Seçim rengi görünsün)
            CRect rcRedraw;
            if (ListView_GetItemRect(GetHwnd(), nItem, &rcRedraw, LVIR_BOUNDS))
            {
                if (GetViewMode() == 1) rcRedraw.bottom = rcRedraw.top + CARD_HEIGHT;
                InvalidateRect(rcRedraw, FALSE);
            }
        }

        // 5. MENÜYÜ AÇ
        // Not: HandlePopupMenu fonksiyonunuza nItem'i de parametre olarak göndermeniz gerekebilir.
        // Eğer boşluğa tıklandıysa nItem -1 olacaktır.
        HandlePopupMenu(ptScreen.x, ptScreen.y);

        return 0;
    }

    case WM_DPICHANGED_BEFOREPARENT:
        return OnDpiChangedBeforeParent(msg, wparam, lparam);
    }

    return WndProcDefault(msg, wparam, lparam);
}



// Files.cpp içine

// ATL include'unu kaldır (varsa sil): #include <atlbase.h> 

void CListCustomerView::LoadSettings()
{
    HKEY hKey = nullptr;
    // Registry'yi okuma modunda aç
    LONG lRes = ::RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\EmlakCRM\\ListViewSettings"), 0, KEY_READ, &hKey);

    if (lRes == ERROR_SUCCESS)
    {
        // 1. KATEGORİ FİLTRESİNİ OKU (DWORD)
        DWORD dwVal = 0;
        DWORD cbData = sizeof(DWORD);
        if (::RegQueryValueEx(hKey, _T("LastCategory"), nullptr, nullptr, (LPBYTE)&dwVal, &cbData) == ERROR_SUCCESS)
        {
            m_filterCategory = (int)dwVal;
            TRACE(_T("✅ REGISTRY (Win32): Kategori %d yüklendi.\n"), m_filterCategory);
        }
        else
        {
            m_filterCategory = 0; // Varsayılan: Hepsi
        }

        // 2. HARF FİLTRESİNİ OKU (STRING)
        TCHAR szBuffer[64]; // Yeterli büyüklükte buffer
        DWORD cbString = sizeof(szBuffer);
        if (::RegQueryValueEx(hKey, _T("LastLetter"), nullptr, nullptr, (LPBYTE)szBuffer, &cbString) == ERROR_SUCCESS)
        {
            m_filterLetter = szBuffer;
            TRACE(_T("✅ REGISTRY (Win32): Harf '%s' yüklendi.\n"), m_filterLetter.c_str());
        }
        else
        {
            m_filterLetter = _T("HEPSI");
        }

        // Anahtarı kapatmayı unutma
        ::RegCloseKey(hKey);
    }
    else
    {
        // İlk açılış veya anahtar yok
        m_filterLetter = _T("HEPSI");
        m_filterCategory = 0;
        TRACE(_T("ℹ️ REGISTRY (Win32): Ayar bulunamadı, varsayılanlar atandı.\n"));
    }
}

void CListCustomerView::SaveSettings()
{
    HKEY hKey = nullptr;
    DWORD dwDisposition = 0;

    // Anahtarı oluştur veya varsa aç (Yazma yetkisiyle)
    LONG lRes = ::RegCreateKeyEx(HKEY_CURRENT_USER, _T("Software\\EmlakCRM\\ListViewSettings"),
        0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, &dwDisposition);

    if (lRes == ERROR_SUCCESS)
    {
        // 1. KATEGORİYİ KAYDET (DWORD)
        DWORD dwVal = (DWORD)m_filterCategory;
        ::RegSetValueEx(hKey, _T("LastCategory"), 0, REG_DWORD, (const BYTE*)&dwVal, sizeof(DWORD));

        // 2. HARFİ KAYDET (STRING)
        // CString'den buffer'a geçiş (Win32++ CString, LPCTSTR operatörüne sahiptir)
        // String uzunluğu: (Karakter sayısı + 1 (null terminator)) * karakter boyutu
        ::RegSetValueEx(hKey, _T("LastLetter"), 0, REG_SZ,
            (const BYTE*)m_filterLetter.c_str(),
            (m_filterLetter.GetLength() + 1) * sizeof(TCHAR));

        ::RegCloseKey(hKey);

        TRACE(_T("💾 REGISTRY (Win32): Ayarlar başarıyla kaydedildi.\n"));
    }
    else
    {
        TRACE(_T("❌ REGISTRY (Win32): Kayıt hatası! Hata kodu: %d\n"), lRes);
    }
}
// Ribbon'dan kategori seçilince çalışan fonksiyon
void CListCustomerView::SetCategoryFilter(int catID)
{
    // Değeri güncelle
    m_filterCategory = catID;

    // ✅ ANINDA KAYDET (Registry'ye yaz)
    SaveSettings();

    // Listeyi yeni filtreyle yenile
    RefreshList();
}

// Durum filtresi: bu projede Calisma_Durumu alanı ile aynı mantık.
// İleride farklı bir sütun kullanılacaksa buradan ayrıştırılabilir.
void CListCustomerView::SetStatusFilter(int statusID)
{
    SetCategoryFilter(statusID);
}

// Harf çubuğundan harf seçilince çalışan fonksiyon
void CListCustomerView::SetLetterFilter(const CString& letter)
{
    // Değeri güncelle
    m_filterLetter = letter;

    // ✅ ANINDA KAYDET (Registry'ye yaz)
    SaveSettings();

    // Listeyi yeni filtreyle yenile
    RefreshList();
}








void CListCustomerView::RefreshList()
{
    SetRedraw(FALSE);
    DeleteAllItems();

    if (!db_Manager.IsConnected())
        db_Manager.EnsureConnection();

    std::vector<Customer_cstr> allCustomers = db_Manager.GetCustomers();

    for (const auto& c : allCustomers)
    {
        // 1. KATEGORİ FİLTRESİ
        bool bCatMatch = false;
        if (m_filterCategory == 0) // HEPSI
        {
            bCatMatch = true;
        }
        else
        {
            int cStatus = _ttoi(c.Calisma_Durumu);
            if (cStatus == m_filterCategory)
                bCatMatch = true;
        }

        if (!bCatMatch) continue;

        // 2. HARF FİLTRESİ
        bool bLetterMatch = false;
        if (m_filterLetter == _T("HEPSI"))
        {
            bLetterMatch = true;
        }
        else if (!c.AdSoyad.IsEmpty())
        {
            CString first = c.AdSoyad.Left(1);
            if (CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, first, 1, m_filterLetter, 1) == CSTR_EQUAL)
                bLetterMatch = true;
        }

        // 3. EŞLEŞME VARSA EKLE
        if (bLetterMatch)
        {
            AddItemFromStruct(c);
        }
    }

    SetRedraw(TRUE);
    Invalidate();
}



// 5️⃣ MERKEZİ LİSTELEME MANTIĞI (Hem Harf Hem Kategoriye Bakar)
//void CListCustomerView::RefreshList()
//{
//    SetRedraw(FALSE);
//    DeleteAllItems();
//
//    // Veritabanından verileri çek
//    std::vector<Customer_cstr> allCustomers = db_Manager.GetCustomers();
//
//    for (const auto& c : allCustomers)
//    {
//        // --- FİLTRE 1: KATEGORİ KONTROLÜ ---
//        bool bCatMatch = false;
//        if (m_filterCategory == 0) // 0 = HEPSI
//        {
//            bCatMatch = true;
//        }
//        else
//        {
//            // c.Calisma_Durumu string geliyor ("1", "2" vb.) integer'a çevirip bakıyoruz
//            int cStatus = _ttoi(c.Calisma_Durumu);
//            if (cStatus == m_filterCategory)
//                bCatMatch = true;
//        }
//
//        if (!bCatMatch) continue; // Kategori uymadıysa harfe bakmaya gerek yok, atla.
//
//        // --- FİLTRE 2: HARF KONTROLÜ ---
//        bool bLetterMatch = false;
//        if (m_filterLetter == _T("HEPSI"))
//        {
//            bLetterMatch = true;
//        }
//        else
//        {
//            if (!c.AdSoyad.IsEmpty())
//            {
//                // Türkçe uyumlu ilk harf karşılaştırması
//                CString first = c.AdSoyad.Left(1);
//                if (CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, first, 1, m_filterLetter, 1) == CSTR_EQUAL)
//                    bLetterMatch = true;
//            }
//        }
//
//        // İKİSİ DE UYUYORSA EKLE
//        if (bLetterMatch)
//        {
//            AddItemFromStruct(c);
//        }
//    }
//
//    SetRedraw(TRUE);
//    Invalidate();
//}



void CListCustomerView::RefreshCustomerList()
{
    // NOTE:
    // RefreshCustomerList daha önce AddItemFromStruct kullaniyordu.
    // AddItemFromStruct her eklemede LVIS_SELECTED/EnsureVisible yaptiği için
    // (özellikle ICON/CARD modunda) gereksiz layout + scroll maliyeti çıkarıyor.
    // Bu yüzden InsertItems() benzeri "bulk-fill" yaklaşımını kullanıyoruz.

    SetRedraw(FALSE);
    DeleteAllItems();

    if (!db_Manager.IsConnected())
        db_Manager.EnsureConnection();

    const std::vector<Customer_cstr> allCustomers = db_Manager.GetCustomers();

    int itemCount = 0;
    for (const auto& c : allCustomers)
    {
        bool bAdd = (m_currentFilter == _T("HEPSI"));

        if (!bAdd && !c.AdSoyad.IsEmpty())
        {
            CString firstLetter = c.AdSoyad.Left(1);
            int result = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE,
                firstLetter, 1,
                m_currentFilter, 1);
            bAdd = (result == CSTR_EQUAL);
        }

        if (!bAdd)
            continue;

        int item = InsertItem(itemCount, c.Cari_Kod);
        SetItemText(item, 1, c.AdSoyad);
        SetItemText(item, 2, c.Telefon);
        SetItemText(item, 3, c.Telefon2);
        SetItemText(item, 4, c.Email);
        SetItemText(item, 5, c.Adres);
        SetItemText(item, 6, c.Sehir);
        SetItemText(item, 7, c.Ilce);
        SetItemText(item, 8, c.TCKN);
        SetItemText(item, 9, c.VergiNo);
        SetItemText(item, 10, c.MusteriTipi);
        SetItemText(item, 11, c.KayitTarihi);
        SetItemText(item, 12, c.Durum);
        SetItemText(item, 13, c.Notlar);
        SetItemText(item, 14, c.Calisma_Durumu);
        itemCount++;
    }

    // Tek seferde çiz
    SetRedraw(TRUE);
    Invalidate();

    // ------------------------------------------------------------
    // StatusFilterPanel counts (visible list based)
    // ------------------------------------------------------------
    // We post counts to the container; it will update the top status bar
    // without tight coupling.
    {
        std::map<int, int> cnt;
        const int n = GetItemCount();
        cnt[0] = n; // HEPSI
        for (int i = 0; i < n; ++i)
        {
            int st = _ttoi(GetItemText(i, 14));
            cnt[st]++;
        }

        auto p = new std::vector<std::pair<int, int>>();
        p->reserve(cnt.size());
        for (const auto& kv : cnt)
            p->push_back({ kv.first, kv.second });

        HWND hParent = ::GetParent(GetHwnd());
        if (::IsWindow(hParent))
            ::PostMessage(hParent, UWM_STATUS_COUNTS, 0, reinterpret_cast<LPARAM>(p));
        else
            delete p;
    }
}


void CListCustomerView::OnSize(UINT, int cx, int cy)
{
    const int filterWidth = 48; // dokunmatik için geniş


}







/////////////////////////////////////
// CContainFiles function definitions
//
CContainFiles::CContainFiles()
{
    SetTabText(L"Müşteri Listesi");
    SetTabIcon(IDI_FILEVIEW);
    SetDockCaption(L"Müşteri Listesi ve Filtreleme");

    // 🛑 DİKKAT: SetView(m_viewFiles); satırını SİL veya YORUMA AL.
    // Çünkü görünümü biz OnAttach içinde elle yöneteceğiz.
    // SetView(m_viewFiles);  <-- SİLİNDİ
}

void CContainFiles::OnAttach()
{
    // 1) Alfabe paneli oluştur
    // Artık özel bir Create fonksiyonumuz var, CreateEx ile uğraşma.
    m_tabFilter.Create(GetHwnd(), IDC_ALPHA_FILTER);

    // 1.1) Durum/Kategori "tab" bar (ListView üstü)
    m_statusFilter.Create(GetHwnd(), IDC_STATUS_FILTER);

    // 2) ListView oluştur (Customer cards)
    if (!m_viewFiles.IsWindow())
        m_viewFiles.Create(*this);
    m_viewFiles.ShowWindow(SW_SHOW);

    // ListView ayarlarını (son seçilen harf/kategori) yükle ki üst bar'lar doğru açılsın
    m_viewFiles.LoadSettings();

    // 3) Portfolio TreeListView oluştur (Customer -> Properties)
    // NOTE: We keep it in the same container and swap visibility.
    if (!m_portfolioView.IsWindow())
        m_portfolioView.Create(*this);
    m_portfolioView.ShowWindow(SW_HIDE);

    // 4) Harfleri Yükle (Türkçe Alfabe)
    {
        auto letters = CAlphaFilterBarTop::BuildTurkishAlphabet(true);
        m_tabFilter.SetLetters(letters);
    }
    // 🛑 DÜZELTME: OnSelect yerine SetSelectedState kullanıyoruz.
    // OnSelect kullanırsan PostMessage atar ve ListView tekrar yüklenir (Double Refresh).
    // SetSelectedState sadece görsel olarak "HEPSI"yi seçili yapar.
    m_tabFilter.SetSelectedState(_T("HEPSİ"));
    // ✅ YENİ: ListView hangi harfte açıldıysa Bar'ı da ona ayarla!
    CString savedLetter = m_viewFiles.m_filterLetter; // View'dan o anki harfi al
    if (savedLetter.CompareNoCase(_T("HEPSI")) == 0) savedLetter = _T("HEPSİ");
    m_tabFilter.SetSelectedState(savedLetter);        // Bar'ı o harfe getir

    // 4.1) Durum/Kategori tab'lerini yükle
    {
        std::vector<StatusTabItem> tabs;
        tabs.push_back({ 0, _T("HEPSİ") });
        tabs.push_back({ STS_RANDEVU,      _T("Randevu") });
        tabs.push_back({ STS_TEKLIF,       _T("Teklif") });
        tabs.push_back({ STS_SICAK_ALICI,  _T("Sıcak Alıcı") });
        tabs.push_back({ STS_SICAK_SATICI, _T("Sıcak Satıcı") });
        tabs.push_back({ STS_ISLEM_TAMAM,  _T("İşlem Tamam") });

        tabs.push_back({ STS_YENI,         _T("Aktif") });
        tabs.push_back({ STS_DUSUNUYOR,    _T("Düşünüyor") });
        tabs.push_back({ STS_TAKIP,        _T("Takip") });
        tabs.push_back({ STS_ULASILAMADI,  _T("Ulaşılamadı") });

        tabs.push_back({ STS_PAHALI,       _T("Pahalı") });
        tabs.push_back({ STS_OLUMSUZ,      _T("Olumsuz") });
        tabs.push_back({ STS_PASIF,        _T("Pasif") });

        m_statusFilter.SetTabs(tabs);
        m_statusFilter.SetSelectedById(m_viewFiles.GetCategoryFilter());
    }


    if (!m_ToolBar.IsWindow())
    {
        m_ToolBar.Create(*this);
        SetupToolBar(); // Butonları ekle
    }


    RecalcLayout();

    // ListView zaten OnInitialUpdate içinde kendi verisini yüklüyor.
    // Ekstra bir yükleme komutuna gerek yok.
}
void CContainFiles::SetupToolBar()
{
    // 1. Stilleri Ayarla (Modern, Düz, Şeffaf)
    DWORD style = m_ToolBar.GetStyle();
    m_ToolBar.SetStyle(style | TBSTYLE_FLAT | CCS_NODIVIDER | TBSTYLE_TRANSPARENT);

    // 2. İkonları Yükle (Maskeleme ile temiz görüntü)
    m_ToolBarImages.Create(32, 32, ILC_COLOR32 | ILC_MASK, 0, 0);
    CBitmap bm(IDB_LISTVIEW);
    m_ToolBarImages.Add(bm, RGB(192, 192, 192)); // Gri arka planı şeffaf yap
    m_ToolBar.SetImageList(m_ToolBarImages);

    // 3. Buton Boyutları + Yazı (ikon altında)
    m_ToolBar.SetBitmapSize(32, 32);
    // Common-controls: TB_SETBUTTONSIZE expects a packed LPARAM (width/height).
    m_ToolBar.SendMessage(TB_SETBUTTONSIZE, 0, MAKELPARAM(78, 56));
    m_ToolBar.SendMessage(TB_SETMAXTEXTROWS, 1, 0);
    {
        DWORD ex = (DWORD)m_ToolBar.SendMessage(TB_GETEXTENDEDSTYLE, 0, 0);
        ex |= TBSTYLE_EX_MIXEDBUTTONS;
        m_ToolBar.SendMessage(TB_SETEXTENDEDSTYLE, 0, ex);
    }

    // --- NAV: Back (Portfolio -> Customers) ---
    m_ToolBar.AddButton(IDM_FILES_BACK);
    m_ToolBar.SetButtonText(IDM_FILES_BACK, L"Geri");
    m_ToolBar.EnableButton(IDM_FILES_BACK, FALSE);


    // --- GRUP 1: Temel Müşteri İşlemleri ---
    m_ToolBar.AddButton(IDM_NEW_RECORD);          // Yeni Müşteri
    m_ToolBar.AddButton(IDM_RECORD_UPDATE);       // Düzenle
    m_ToolBar.AddButton(ID_MAINLISTVIEWMENU_KAYD32781); // Görüntüle (Salt Okunur)
    m_ToolBar.AddButton(IDM_DELETE_IS_SELECTED);  // Sil

   //m_ToolBar.AddSeparator();

    // --- GRUP 2: Hızlı Mülk Ekleme (Seçili Müşteriye) ---
    // Bu butonlar müşteri seçiliyken hayat kurtarır
    m_ToolBar.AddButton(IDM_ADD_HOME);            // Ev Ekle
    m_ToolBar.AddButton(ID_EKLE_ARSA);            // Arsa Ekle
    m_ToolBar.AddButton(ID_EKLE_TARLA);           // Tarla Ekle
    m_ToolBar.AddButton(ID_EKLE_VILLA);           // Villa Ekle
    m_ToolBar.AddButton(ID_EKLE_TICARIALAN);      // Ticari Ekle
    m_ToolBar.AddButton(ID_EKLE_BAGVBAHCE);       // Bağ/Bahçe Ekle

    //m_ToolBar.AddSeparator();

    // --- GRUP 3: Filtreleme ve Durum ---
    m_ToolBar.AddButton(IDM_FLT_HEPSI);           // Filtreyi Temizle
    m_ToolBar.AddButton(IDM_FLT_YENI);            // Sadece Yeniler
    m_ToolBar.AddButton(IDM_FLT_SICAK_ALICI);     // Sıcak Alıcılar

    // İsterseniz hızlı durum atama butonları da ekleyebilirsiniz:
    // m_ToolBar.AddButton(IDM_SET_CALISIR);
    // m_ToolBar.AddButton(IDM_SET_DUSUNECEK);

    //m_ToolBar.AddSeparator();

    // --- GRUP 4: Araçlar ve Bulut ---
    m_ToolBar.AddButton(IDM_RANDEVU_OLUSTUR);     // Randevu Ver
    m_ToolBar.AddButton(IDM_FILE_UPLOAD_CLOUD);   // Buluta Gönder (Tekil)
    m_ToolBar.AddButton(IDM_FILE_UPLOAD_CLOUD_PUSH); // Senkronize Et (Genel)

    // --- Button captions (under the icons) ---
    m_ToolBar.SetButtonText(IDM_NEW_RECORD, L"Yeni");
    m_ToolBar.SetButtonText(IDM_RECORD_UPDATE, L"Düzenle");
    m_ToolBar.SetButtonText(ID_MAINLISTVIEWMENU_KAYD32781, L"Görüntüle");
    m_ToolBar.SetButtonText(IDM_DELETE_IS_SELECTED, L"Sil");
    m_ToolBar.SetButtonText(IDM_ADD_HOME, L"Ev");
    m_ToolBar.SetButtonText(ID_EKLE_ARSA, L"Arsa");
    m_ToolBar.SetButtonText(ID_EKLE_TARLA, L"Tarla");
    m_ToolBar.SetButtonText(ID_EKLE_VILLA, L"Villa");
    m_ToolBar.SetButtonText(ID_EKLE_TICARIALAN, L"Ticari");
    m_ToolBar.SetButtonText(ID_EKLE_BAGVBAHCE, L"Bağ/Bahçe");
    m_ToolBar.SetButtonText(IDM_FLT_HEPSI, L"Hepsi");
    m_ToolBar.SetButtonText(IDM_FLT_YENI, L"Yeni");
    m_ToolBar.SetButtonText(IDM_FLT_SICAK_ALICI, L"Sıcak");
    m_ToolBar.SetButtonText(IDM_RANDEVU_OLUSTUR, L"Randevu");
    m_ToolBar.SetButtonText(IDM_FILE_UPLOAD_CLOUD, L"Bulut");
    m_ToolBar.SetButtonText(IDM_FILE_UPLOAD_CLOUD_PUSH, L"Senkron");

    // Force SHOWTEXT flag on each button (Win32 toolbar requirement).
    int btnCount = (int)m_ToolBar.SendMessage(TB_BUTTONCOUNT, 0, 0);
    for (int i = 0; i < btnCount; ++i)
    {
        TBBUTTON b{};
        if (!m_ToolBar.SendMessage(TB_GETBUTTON, i, (LPARAM)&b)) continue;
        if (b.idCommand == 0 || b.fsStyle & TBSTYLE_SEP) continue;

        TBBUTTONINFO bi{};
        bi.cbSize = sizeof(bi);
        bi.dwMask = TBIF_STYLE;
        bi.fsStyle = (BYTE)(b.fsStyle | BTNS_SHOWTEXT);
        m_ToolBar.SendMessage(TB_SETBUTTONINFO, (WPARAM)b.idCommand, (LPARAM)&bi);
    }

    m_ToolBar.Autosize();
}
void CContainFiles::RecalcLayout()
{
    CRect rc = GetClientRect();
    if (rc.IsRectEmpty()) return;

    // 1. TOOLBAR (Daima en üstte)
    int toolbarHeight = 0;
    if (m_ToolBar.IsWindow()) {
        CSize sz = m_ToolBar.GetMaxSize();
        toolbarHeight = (sz.cy < 32) ? 32 : sz.cy;
        m_ToolBar.SetWindowPos(NULL, 0, 0, rc.Width(), toolbarHeight, SWP_NOZORDER);
    }
    rc.top += toolbarHeight;

    // 2. If portfolio is visible, it owns the entire content area
    //    under the toolbar (no alpha filter bar).
    if (m_showPortfolio && m_portfolioView.IsWindow())
    {
        if (m_tabFilter.IsWindow()) m_tabFilter.ShowWindow(SW_HIDE);
        if (m_viewFiles.IsWindow()) m_viewFiles.ShowWindow(SW_HIDE);

        m_portfolioView.SetWindowPos(NULL, rc.left, rc.top, rc.Width(), rc.Height(),
            SWP_NOZORDER | SWP_SHOWWINDOW);
        return;
    }

    // 3. HARF BARI VE LİSTE (Customers)
    if (m_tabFilter.IsWindow() && m_viewFiles.IsWindow()) {
        int barThickness = DpiScaleInt(36);
        CRect barRect, listRect;

        // Karşılaştırmaları enum class kurallarına göre yapıyoruz:
        if (m_barPos == BarPosition::Top) {
            barRect = CRect(rc.left, rc.top, rc.right, rc.top + barThickness);
            listRect = CRect(rc.left, rc.top + barThickness, rc.right, rc.bottom);
            m_tabFilter.SetVertical(false);
        }
        else // AlphaFilterBar layout
        if (m_barPos == BarPosition::Top)
        {
            // ✅ Top: horizontal, clean look (recommended)
            const int barH = DpiScaleInt(34);
            barRect = CRect(rc.left, rc.top, rc.right, rc.top + barH);
            listRect = CRect(rc.left, rc.top + barH, rc.right, rc.bottom);
            m_tabFilter.SetVertical(false);
        }
        else if (m_barPos == BarPosition::Left)
        {
            barRect = CRect(rc.left, rc.top, rc.left + barThickness, rc.bottom);
            listRect = CRect(rc.left + barThickness, rc.top, rc.right, rc.bottom);
            m_tabFilter.SetVertical(true);
        }
        else // BarPosition::Right
        {
            barRect = CRect(rc.right - barThickness, rc.top, rc.right, rc.bottom);
            listRect = CRect(rc.left, rc.top, rc.right - barThickness, rc.bottom);
            m_tabFilter.SetVertical(true);
        }

        // --- Status tab bar sits on top of listRect (inside the customer area)
        CRect statusRect = listRect;
        int statusHeight = 0;
        if (m_statusFilter.IsWindow())
        {
            // 1st pass: give a generous height so the control can compute wrapping
            const int tmpH = DpiScaleInt(120);
            m_statusFilter.SetWindowPos(NULL, statusRect.left, statusRect.top, statusRect.Width(), tmpH,
                SWP_NOZORDER | SWP_SHOWWINDOW);

            statusHeight = m_statusFilter.GetDesiredHeight();
            // clamp to tmpH to avoid negative or too-large values
            if (statusHeight <= 0) statusHeight = DpiScaleInt(36);
            if (statusHeight > tmpH) statusHeight = tmpH;

            statusRect.bottom = statusRect.top + statusHeight;
            // 2nd pass: set the final height
            m_statusFilter.SetWindowPos(NULL, statusRect, SWP_NOZORDER | SWP_SHOWWINDOW);

            // List starts after status bar
            listRect.top += statusHeight;
        }

        m_portfolioView.ShowWindow(SW_HIDE);
        m_tabFilter.SetWindowPos(NULL, barRect, SWP_NOZORDER | SWP_SHOWWINDOW);
        m_viewFiles.SetWindowPos(NULL, listRect, SWP_NOZORDER | SWP_SHOWWINDOW);
    }
}

/* Duran YALÇIN 17/01/2026
Fonksiyon Adı.:  CContainFiles::ShowCustomersView
Açıklama :       Portfolio(TreeList) görünümünden müşteri kartlarına geri döner.
*/
void CContainFiles::ShowCustomersView()
{
    // Switch mode BEFORE any layout happens.
    m_showPortfolio = false;

    // Back button should only be enabled in Portfolio view.
    if (m_ToolBar.IsWindow())
        m_ToolBar.EnableButton(IDM_FILES_BACK, FALSE);

    // 1) Portfolio/TreeList'i gizle
    if (m_portfolioView.IsWindow())
        m_portfolioView.ShowWindow(SW_HIDE);

    // 2) Customer ListView'i göster
    if (m_viewFiles.IsWindow())
        m_viewFiles.ShowWindow();

    // 3) Layout'u güncelle (çok önemli)
    RecalcLayout();        // sende varsa
    Invalidate();
    UpdateWindow();

    // 4) Fokus + state restore
    m_viewFiles.SetFocus();

    // Restore selection + scroll position (Explorer-like Back)
    if (m_viewFiles.IsWindow())
    {
        // Restore scroll first
        if (m_savedTopIndex >= 0)
        {
            ::SendMessage(m_viewFiles.GetHwnd(), LVM_ENSUREVISIBLE, (WPARAM)m_savedTopIndex, (LPARAM)FALSE);
        }

        // Restore selection (prefer CariKod match if available)
        int selectIndex = m_savedSelIndex;
        if (!m_savedCariKod.IsEmpty())
        {
            const int count = m_viewFiles.GetItemCount();
            for (int i = 0; i < count; ++i)
            {
                CString ck = m_viewFiles.GetItemText(i, 0);
                if (ck.CompareNoCase(m_savedCariKod) == 0)
                {
                    selectIndex = i;
                    break;
                }
            }
        }
        if (selectIndex >= 0 && selectIndex < m_viewFiles.GetItemCount())
        {
            m_viewFiles.SetItemState(selectIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
            ::SendMessage(m_viewFiles.GetHwnd(), LVM_ENSUREVISIBLE, (WPARAM)selectIndex, (LPARAM)TRUE);
        }
    }
}

/* Duran YALÇIN 17/01/2026
Fonksiyon Adı.:  CContainFiles::ShowPortfolioView
Açıklama :       Seçili müşteri için TreeListView portföy ekranını açar ve doldurur.
*/
void CContainFiles::ShowPortfolioView(const CString& cariKod)
{
    CString ck = cariKod;
    ck.Trim();
    if (ck.IsEmpty()) return;

    // Capture current Customers view state (selection + scroll) for Back
    if (m_viewFiles.IsWindow())
    {
        m_savedTopIndex = (int)::SendMessage(m_viewFiles.GetHwnd(), LVM_GETTOPINDEX, 0, 0);
        m_savedSelIndex = m_viewFiles.GetNextItem(-1, LVNI_SELECTED);
        if (m_savedSelIndex >= 0)
            m_savedCariKod = m_viewFiles.GetItemText(m_savedSelIndex, 0);
        else
            m_savedCariKod.Empty();
    }

    m_showPortfolio = true;
    m_activeCariKod = ck;

    // Back button
    if (m_ToolBar.IsWindow())
        m_ToolBar.EnableButton(IDM_FILES_BACK, TRUE);

    // Fill portfolio using TreeListView's own professional logic.
    if (m_portfolioView.IsWindow())
    {
        m_portfolioView.SetRedraw(FALSE);
        m_portfolioView.LoadPortfolioByCariKod(ck);
        m_portfolioView.SetRedraw(TRUE);
        m_portfolioView.Invalidate();
    }

    RecalcLayout();
}

/**************************************************/

// Files.cpp içine ekle:

// =========================================================================
// MAINFRAME KONTROL FONKSİYONLARI (WRAPPER)
// =========================================================================

// 1. Görünüm Modunu Değiştir (Büyük İkon, Liste, Rapor vb.)
void CContainFiles::SetViewMode(DWORD dwView)
{
    // CListView (m_ViewFiles) stilini değiştir
    DWORD dwStyle = m_viewFiles.GetStyle() & ~LVS_TYPEMASK;
    m_viewFiles.SetStyle(dwStyle | dwView);
}

void CContainFiles::ToggleViewMode()
{
    DWORD dwStyle = m_viewFiles.GetStyle() & LVS_TYPEMASK;
    DWORD dwNewStyle = LVS_REPORT;

    if (dwStyle == LVS_REPORT) dwNewStyle = LVS_ICON;
    else if (dwStyle == LVS_ICON) dwNewStyle = LVS_LIST;
    else if (dwStyle == LVS_LIST) dwNewStyle = LVS_SMALLICON;

    SetViewMode(dwNewStyle);
}

// 2. Listeye Yeni Dosya Ekle (Manuel Ekleme)
void CContainFiles::AddFileItem(const CString& name, const CString& size, const CString& type, int imageIndex)
{
    // ListView'e satır ekle
    int nItem = m_viewFiles.InsertItem(m_viewFiles.GetItemCount(), name, imageIndex);

    // Alt kolonları doldur (Rapor görünümü için)
    m_viewFiles.SetItemText(nItem, 1, size); // 2. Kolon (Boyut)
    m_viewFiles.SetItemText(nItem, 2, type); // 3. Kolon (Tür)
}

// Seçili Olanı Sil
void CContainFiles::RemoveSelectedItem()
{
    int nItem = m_viewFiles.GetNextItem(-1, LVNI_SELECTED);
    if (nItem != -1)
    {
        m_viewFiles.DeleteItem(nItem);
    }
}

// Tüm Listeyi Temizle
void CContainFiles::ClearAllItems()
{
    m_viewFiles.DeleteAllItems();
}

// Listeyi Yenile (Klasör içeriğini tekrar okur)
void CContainFiles::RefreshFileList()
{
    // Mevcut CViewFiles yapısındaki klasör okuma fonksiyonunu tetikler
    // Eğer CViewFiles içinde "Refresh()" varsa onu çağırın, yoksa:
    // m_ViewFiles.FillList(); // (Örnek fonksiyon ismi)

    // Not: Normalde CViewFiles içinde OnInitialUpdate benzeri bir yerde dolum yapılır.
    // Eğer CViewFiles::Refresh() yoksa, onu da CViewFiles içine eklemen gerekir.
    // Şimdilik sadece yeniden çizdiriyoruz:
    m_viewFiles.Invalidate();
}

// 3. Seçili Dosya Adını Al
CString CContainFiles::GetSelectedFileName() const
{
    int nItem = m_viewFiles.GetNextItem(-1, LVNI_SELECTED);
    if (nItem != -1)
    {
        return m_viewFiles.GetItemText(nItem, 0); // 0. Kolon (Dosya Adı)
    }
    return _T("");
}

bool CContainFiles::HasSelection() const
{
    return (m_viewFiles.GetNextItem(-1, LVNI_SELECTED) != -1);
}

// 4. Filtreleme Uygula (MainFrame araması veya Harf barı için)
void CContainFiles::ApplyFilter(const CString& filterText)
{
    // Bu fonksiyonu CViewFiles içinde tanımlayıp buradan çağırmak en doğrusudur.
    // Örneğin: m_ViewFiles.SetFilter(filterText);

    // Eğer CViewFiles'da yoksa basitçe buraya entegre edebilirsin.
    // Ancak genellikle bu logic CViewFiles::OnUpdate içinde olur.
}

void CListCustomerView::DeleteSelectedItems()
{
    // 1. Kullanıcıdan Onay Al
    if (MessageBox(_T("Seçili kayıtları silmek istediğinize emin misiniz?"), _T("Sil"), MB_YESNO | MB_ICONWARNING) != IDYES)
        return;

    // 2. Veritabanı Bağlantısı
    DatabaseManager& db = DatabaseManager::GetInstance();

    // 3. Çoklu Seçim Silme Döngüsü
    // İpucu: Listeden silerken indexler kaymasın diye TERSTEN (sondan başa) gitmek gerekmez,
    // ancak Win32++'da "GetNextItem" ile ilerleyip ID'leri toplamak daha güvenlidir.

    std::vector<int> itemsToDelete;
    int item = GetNextItem(-1, LVNI_SELECTED);
    while (item != -1)
    {
        itemsToDelete.push_back(item);
        item = GetNextItem(item, LVNI_SELECTED);
    }

    // Hiçbir şey seçili değilse çık
    if (itemsToDelete.empty()) return;

    // Sondan başa doğru sil (Index kaymasını önlemek için)
    for (auto it = itemsToDelete.rbegin(); it != itemsToDelete.rend(); ++it)
    {
        int nItem = *it;

        // a) Veritabanı ID'sini al (LPARAM'a sync_id veya benzeri bir pointer koyduysan)
        // Eğer LPARAM'da veri yoksa listedeki "Hidden Column"dan veya map'ten çekmelisin.
        // ÖRNEK: sync_id'nin ItemData olarak saklandığını varsayıyoruz.
        CString syncID = GetItemText(nItem, 0); // Veya GetItemData(nItem) ile pointer alıyorsanız onu kullanın.

        // b) Veritabanından Sil
        CString sql;
        sql.Format(_T("DELETE FROM Customer WHERE sync_id = '%s'"), syncID);
        db.ExecuteSQL(sql);
        // c) Listeden Görsel Olarak Sil
        DeleteItem(nItem);
    }
}

/*************************************************/


BOOL CContainFiles::OnCommand(WPARAM wparam, LPARAM lparam)
{
    const UINT id = LOWORD(wparam);

    // Local navigation command (handled INSIDE Files module)
    if (id == IDM_FILES_BACK)
    {
        ShowCustomersView();
        return TRUE;
    }

    // Default: forward to MainFrame so existing command routing keeps working.
    CRibbonFrameApp* pApp = GetContainerApp();
    if (!pApp) return FALSE;
    return pApp->GetMainFrame().SendMessage(WM_COMMAND, wparam, lparam);
}

// Pencere boyutu değişince elemanları yeniden hizala
LRESULT CContainFiles::WndProc(UINT msg, WPARAM wparam, LPARAM lparam)
{       
    
    const UINT id = LOWORD(wparam);

    switch (msg)
    {
    case WM_COMMAND:
    {
        if (id == IDM_FILES_BACK)
        {
            ShowCustomersView();   // <-- kesin dönüş burada
            return 0;
        }
        break;
    }
        // CContainFiles::WndProc içinde case UWM_ALPHA_FILTER bloğunu kontrol et:

    case UWM_ALPHA_FILTER:
    {
        // AlphaFilterPanel'den WPARAM olarak 'char' kodu geliyor.
        wchar_t ch = (wchar_t)wparam;

        CString filter;
        if (ch == 0)
            filter = _T("HEPSI");
        else
            filter.Format(_T("%c"), ch);

        // Listeye filtreyi uygula
        m_viewFiles.SetFilter(filter);
        return 0;
    }

    case UWM_STATUS_FILTER:
    {
        // StatusFilterPanel'den WPARAM olarak statusID geliyor.
        const int statusId = (int)wparam; // 0 => HEPSI

        // Liste filtrele (Calisma_Durumu)
        m_viewFiles.SetStatusFilter(statusId);

        // Bar görseli senkron kalsın
        if (m_statusFilter.IsWindow())
            m_statusFilter.SetSelectedById(statusId);

        return 0;
    }

    case UWM_STATUS_COUNTS:
    {
        // lParam carries heap-allocated std::vector<std::pair<int,int>>
        std::unique_ptr<std::vector<std::pair<int, int>>> pCounts(
            reinterpret_cast<std::vector<std::pair<int, int>>*>(lparam));
        if (pCounts && m_statusFilter.IsWindow())
            m_statusFilter.UpdateCounts(*pCounts);
        return 0;
    }

    case UWM_FILES_SHOW_PORTFOLIO:
    {
        // lParam carries CString* allocated by sender
        std::unique_ptr<CString> pCari(reinterpret_cast<CString*>(lparam));
        if (pCari)
            ShowPortfolioView(*pCari);
        return 0;
    }
    case UWM_FILES_SHOW_CUSTOMERS:
        ShowCustomersView();
        return 0;
    case WM_ERASEBKGND:
    {
        CDC dc((HDC)wparam);
        CRect rc = GetClientRect();
        // Tüm zemini standart pencere rengiyle (Gri/Beyaz) doldur
        dc.FillRect(rc, GetSysColorBrush(COLOR_3DFACE));
        return 1; // "Ben boyadım, sen karışma" diyoruz
    }

    case WM_SIZE:
    case WM_WINDOWPOSCHANGED: // <-- Bunu ekle
        RecalcLayout();
        Invalidate();
        break;
    }

    return WndProcDefault(msg, wparam, lparam);
}

// Tab Tıklamasını Yakala
LRESULT CContainFiles::OnNotify(WPARAM wparam, LPARAM lparam)
{
    LPNMHDR pHeader = reinterpret_cast<LPNMHDR>(lparam);

    // Eğer bildirim bizim TabControl'den geliyorsa ve Seçim Değişmişse (TCN_SELCHANGE)


    // Diğer bildirimleri (örneğin ListView tıklamalarını) varsayılan işleyiciye bırak
    return CDockContainer::OnNotify(wparam, lparam);
}
///////////////////////////////////
//  CDockFiles function definitions
//
CDockFiles::CDockFiles()
{
    SetView(m_files);
    SetBarWidth(8);
}

// Handle the window's messages.
LRESULT CDockFiles::WndProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
    try
    {
        // Pass unhandled messages on for default processing.
        return WndProcDefault(msg, wparam, lparam);
    }

    // Catch all unhandled CException types.
    catch (const CException& e)
    {
        // Display the exception and continue.
        CString str1;
        str1 << e.GetText() << L'\n' << e.GetErrorString();

        CString str2;
        str2 << "Error: " << e.what();
        TaskDialogBox(nullptr, str1, str2, TD_ERROR_ICON);
    }

    // Catch all unhandled std::exception types.
    catch (const std::exception& e)
    {
        // Display the exception and continue.
        CString str1 = e.what();
        ::MessageBox(nullptr, str1, L"Error: std::exception", MB_ICONERROR);
    }

    return 0;
}


void CListCustomerView::AddCustomerData(const std::vector<Customer_cstr>& customers)
{
    DeleteAllItems(); // Listeyi temizle

    // ListView'ın hızlı doldurulması için çizimi durdur
    SetRedraw(FALSE);

    for (size_t i = 0; i < customers.size(); ++i)
    {
        const Customer_cstr& c = customers[i];

        // 0. Satırı ekle (Cari Kod)
        int nItem = InsertItem(static_cast<int>(i), c.Cari_Kod);

        // 1. Alt öğe (SubItem) metinlerini ayarla
        SetItemText(nItem, 1, c.AdSoyad);
        SetItemText(nItem, 2, c.Telefon);
        SetItemText(nItem, 3, c.Telefon2);
        SetItemText(nItem, 4, c.Email);
        SetItemText(nItem, 5, c.Adres);
        SetItemText(nItem, 6, c.Sehir);
        SetItemText(nItem, 7, c.Ilce);
        SetItemText(nItem, 8, c.TCKN);
        SetItemText(nItem, 9, c.VergiNo);
        SetItemText(nItem, 10, c.MusteriTipi);
        SetItemText(nItem, 11, c.KayitTarihi); // Veritabanından gelen kayıt tarihi
        SetItemText(nItem, 12, c.Durum);
        SetItemText(nItem, 13, c.Notlar);
		SetItemText(nItem, 14, c.Calisma_Durumu);

        // ÖNEMLİ: Sağ Tık menüsü için kritik olan, bu satıra dizideki INDEX'i saklamak
        // (Veya Cari Kod'u saklamak. Bizim yapımızda m_customers vektörüne erişim için index saklamak daha iyidir.)
        SetItemData(nItem, i);
    }

    // Listeyi yeniden çiz
    SetRedraw(TRUE);
    Invalidate();
}


void CListCustomerView::OnItemChanged(NMLISTVIEW* pNM)
{
    // Selection change notification (LVN_ITEMCHANGED reflected).
    // IMPORTANT: Keep this lightweight. We only defer notifications.
    if (!pNM) return;

    const bool becameSelected =
        ((pNM->uChanged & LVIF_STATE) != 0) &&
        ((pNM->uNewState & LVIS_SELECTED) != 0) &&
        ((pNM->uOldState & LVIS_SELECTED) == 0);

    if (becameSelected)
    {
        m_editItem = pNM->iItem;
        m_editSubItem = GetSelectedSubItem();

        // Let the container/mainframe refresh command enable states.
        ::PostMessage(::GetParent(GetHwnd()), UWM_CONTEXT_CHANGED, 1 /*Customer*/, 0);

        // ✅ FIX (2026-01-21): When a customer becomes selected, refresh the portfolio TreeList (right/bottom panel).
        // We keep it deferred (PostMessage) to avoid UI freezes/re-entrancy.
        CString cariKod = GetItemText(pNM->iItem, 0);
        cariKod.Trim();
        if (!cariKod.IsEmpty())
        {
            HWND hMain = ::GetAncestor(GetHwnd(), GA_ROOT);
            if (hMain)
            {
                ::PostMessage(hMain, CListCustomerView::UWM_OPEN_CUSTOMER_PROPERTIES, 0,
                    reinterpret_cast<LPARAM>(new CString(cariKod)));
            }
        }
    }
}


// Files.cpp (CListCustomerView::OnCustomDraw metodu)














//LRESULT CListCustomerView::HandleCardViewDraw(LPNMLVCUSTOMDRAW pNMLVCD)
//{
//    switch (pNMLVCD->nmcd.dwDrawStage)
//    {
//    case CDDS_PREPAINT:
//        return CDRF_NOTIFYITEMDRAW; // Sadece öğe bazında bildirim iste
//
//    case CDDS_ITEMPREPAINT:
//    {
//     CDC dc(pNMLVCD->nmcd.hdc);
//
//        DrawCardItem(pNMLVCD, dc);
//        //DrawCardItemNormal(pNMLVCD, dc);
//         return CDRF_SKIPDEFAULT | CDRF_DODEFAULT; // Windows çizimini atla
//    }
//
//    default:
//        return CDRF_DODEFAULT;
//    }
//}

LRESULT CListCustomerView::HandleReportItemPrePaint(LPNMLVCUSTOMDRAW pNMLVCD)
{
    const COLORREF clrWindow = GetSysColor(COLOR_WINDOW);

    const int nRow = static_cast<int>(pNMLVCD->nmcd.dwItemSpec);
    const int statusID = _ttoi(GetItemText(nRow, 14));

    // Selected row uses system highlight
    if (pNMLVCD->nmcd.uItemState & CDIS_SELECTED)
    {
        pNMLVCD->clrTextBk = GetSysColor(COLOR_HIGHLIGHT);
        pNMLVCD->clrText   = GetSysColor(COLOR_HIGHLIGHTTEXT);
        return CDRF_NOTIFYSUBITEMDRAW;
    }

    // Base: subtle zebra
    const bool zebra = (nRow % 2) == 1;
    COLORREF bk = zebra ? RGB(248, 248, 248) : clrWindow;

    // Status tint (very soft) – keeps it corporate, not “rainbow”
    switch (statusID)
    {
    case STS_RANDEVU:     bk = RGB(246, 250, 255); break;
    case STS_TEKLIF:      bk = RGB(245, 255, 250); break;
    case STS_SICAK_ALICI: bk = RGB(255, 248, 238); break;
    case STS_SICAK_SATICI:bk = RGB(255, 246, 246); break;
    case STS_ISLEM_TAMAM: bk = RGB(244, 255, 244); break;
    default: break;
    }

    pNMLVCD->clrTextBk = bk;
    pNMLVCD->clrText   = GetSysColor(COLOR_WINDOWTEXT);
    return CDRF_NOTIFYSUBITEMDRAW;
}

LRESULT CListCustomerView::HandleReportSubItemPrePaint(LPNMLVCUSTOMDRAW pNMLVCD)
{
    // Corporate text palette per column (no random color math).
    const int nCol = pNMLVCD->iSubItem;

    COLORREF clr = GetSysColor(COLOR_WINDOWTEXT);

    // 0: Cari Kod (slightly muted)
    if (nCol == 0)
        clr = RGB(80, 110, 150);

    // 2-3: phones (muted)
    else if (nCol == 2 || nCol == 3)
        clr = RGB(90, 90, 90);

    // 6-7: city/district muted
    else if (nCol == 6 || nCol == 7)
        clr = RGB(100, 100, 100);

    // 14: status stronger
    else if (nCol == 14)
        clr = RGB(40, 40, 40);

    pNMLVCD->clrText = clr;
    return CDRF_NEWFONT;
}

// Files.cpp (CListCustomerView::DrawCardItemProfessional metodu - FİNAL VERSİYON)
#ifndef IDM_STATUS_ULASILAMADI
  #define IDM_STATUS_ULASILAMADI       88251
#endif
#ifndef IDM_STATUS_PAHALI
  #define IDM_STATUS_PAHALI            88252
#endif
#ifndef IDM_STATUS_OLUMSUZ
  #define IDM_STATUS_OLUMSUZ           88253
#endif

void CListCustomerView::DrawCardItemProfessional(LPNMLVCUSTOMDRAW pLVCD, CDC& dc)
{
    // GDI+ İsim Uzayı
    using namespace Gdiplus;

    // 1. Temel Kontroller ve Veri Hazırlığı
    int item = static_cast<int>(pLVCD->nmcd.dwItemSpec);
    if (item < 0 || item >= GetItemCount()) return;

    CRect rcItem;
    if (ListView_GetItemRect(GetHwnd(), item, &rcItem, LVIR_BOUNDS) == FALSE) return;
    if (rcItem.Width() <= 0 || rcItem.Height() <= 0) return;

    // Durumlar
    const bool selected = (ListView_GetItemState(GetHwnd(), item, LVIS_SELECTED) & LVIS_SELECTED) != 0;
    const bool hovered = (item == m_hoverItem);

    // --- SABİTLER (DPI Ölçekli) ---
    // Kart boyutlarını ve paddingleri ayarlıyoruz
    const int CARD_TOTAL_WIDTH = DpiScaleInt(270);
    const int CARD_TOTAL_HEIGHT = DpiScaleInt(130);
    const int PADDING = DpiScaleInt(12);
    const REAL RADIUS = (REAL)DpiScaleInt(8); // Daha modern, keskin köşeler

    // --- VERİ OKUMA ---
    CString cariKod = GetItemText(item, 0);
    CString ad = GetItemText(item, 1);
    CString tel = GetItemText(item, 2);
    CString mail = GetItemText(item, 4);
    CString tckimlik = GetItemText(item, 3);
    int durumID = _ttoi(GetItemText(item, 14));

    // --- DURUM RENK MANTIĞI (Modern Pastel Palet) ---
    CString statusText;
    Color cStatusMain;

    switch (durumID)
    {
    case STS_RANDEVU:      statusText = L"RANDEVU";     cStatusMain = Color(155, 89, 182); break; // Mor
    case STS_TEKLIF:       statusText = L"TEKLİF";      cStatusMain = Color(46, 204, 113); break; // Yeşil
    case STS_SICAK_ALICI:  statusText = L"SICAK ALICI"; cStatusMain = Color(231, 76, 60);  break; // Kırmızı
    case STS_SICAK_SATICI: statusText = L"SICAK SATICI"; cStatusMain = Color(230, 126, 34); break; // Turuncu
    case STS_ISLEM_TAMAM:  statusText = L"TAMAMLANDI";  cStatusMain = Color(26, 188, 156); break; // Teal
    case STS_DUSUNUYOR:    statusText = L"DÜŞÜNÜYOR";   cStatusMain = Color(241, 196, 15); break; // Sarı
    case STS_TAKIP:        statusText = L"TAKİPTE";     cStatusMain = Color(52, 152, 219); break; // Mavi
    case STS_ULASILAMADI:  statusText = L"ULAŞILAMADI"; cStatusMain = Color(211, 84, 0);   break; // Koyu Turuncu
    case STS_YENI:         statusText = L"AKTİF";       cStatusMain = Color(64, 115, 255); break; // Standart Mavi
    case STS_PAHALI:       statusText = L"PAHALI";      cStatusMain = Color(149, 165, 166); break; // Gri
    case STS_OLUMSUZ:      statusText = L"OLUMSUZ";     cStatusMain = Color(127, 140, 141); break; // Koyu Gri
    case STS_PASIF:        statusText = L"PASİF";       cStatusMain = Color(189, 195, 199); break; // Açık Gri
    default:               statusText = L"TANIMSIZ";    cStatusMain = Color(189, 195, 199); break;
    }

    Graphics g(dc.GetHDC());
    g.SetSmoothingMode(SmoothingModeAntiAlias);
    g.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

    // Çizim Alanı
    RectF rect(
        (REAL)rcItem.left + DpiScaleInt(6),
        (REAL)rcItem.top + DpiScaleInt(6),
        (REAL)CARD_TOTAL_WIDTH - DpiScaleInt(12),
        (REAL)CARD_TOTAL_HEIGHT - DpiScaleInt(12)
    );

    // =========================================================================
    // 1. GÖLGE (Soft Shadow)
    // =========================================================================
    // Performans için basit bir yol: Arka plana hafif genişletilmiş koyu bir dikdörtgen
    if (!selected)
    {
        RectF shadowRect = rect;
        shadowRect.Offset(0, 2);
        shadowRect.Inflate(1, 1);
        GraphicsPath shadowPath;
        shadowPath.AddRectangle(shadowRect); // Basit rect performans için
        SolidBrush shadowBrush(Color(20, 0, 0, 0)); // Çok hafif gölge
        g.FillRectangle(&shadowBrush, shadowRect);
    }

    // =========================================================================
    // 2. KART GÖVDESİ
    // =========================================================================
    GraphicsPath path;
    path.AddArc(rect.X, rect.Y, RADIUS * 2, RADIUS * 2, 180, 90);
    path.AddArc(rect.GetRight() - RADIUS * 2, rect.Y, RADIUS * 2, RADIUS * 2, 270, 90);
    path.AddArc(rect.GetRight() - RADIUS * 2, rect.GetBottom() - RADIUS * 2, RADIUS * 2, RADIUS * 2, 0, 90);
    path.AddArc(rect.X, rect.GetBottom() - RADIUS * 2, RADIUS * 2, RADIUS * 2, 90, 90);
    path.CloseFigure();

    // Arka Plan Rengi (Dark Theme)
    Color bgTop, bgBot;
    if (selected) {
        // Seçiliyse hafif maviye çalan koyu gri
        bgTop = Color(255, 60, 65, 80);
        bgBot = Color(255, 45, 50, 60);
    }
    else {
        // Normalde antrasit/koyu gri
        bgTop = Color(255, 50, 50, 50);
        bgBot = Color(255, 40, 40, 40);
    }

    LinearGradientBrush bgBrush(rect, bgTop, bgBot, LinearGradientModeVertical);
    g.FillPath(&bgBrush, &path);

    // Seçim Çerçevesi
    if (selected) {
        Pen selPen(Color(200, 64, 158, 255), 2.0f); // Mavi parlama
        g.DrawPath(&selPen, &path);
    }
    else {
        Pen borderPen(Color(255, 70, 70, 70), 1.0f); // İnce gri çerçeve
        g.DrawPath(&borderPen, &path);
    }

    // =========================================================================
    // 3. SOL DURUM ÇUBUĞU (Accent Stripe)
    // =========================================================================
    // Kavisli ribbon yerine sol kenarda şık, ince bir çizgi
    g.SetClip(&path, CombineModeIntersect);
    RectF stripeRect(rect.X, rect.Y, (REAL)DpiScaleInt(4), rect.Height);
    SolidBrush stripeBrush(cStatusMain);
    g.FillRectangle(&stripeBrush, stripeRect);
    g.ResetClip();

    // =========================================================================
    // 4. AVATAR (Dairesel - İsmin Baş Harfi)
    // =========================================================================
    REAL avatarSize = (REAL)DpiScaleInt(40);
    REAL avatarLeft = rect.X + (REAL)PADDING + (REAL)DpiScaleInt(6);
    REAL avatarTop = rect.Y + (REAL)PADDING;
    RectF avatarRect(avatarLeft, avatarTop, avatarSize, avatarSize);

    // Avatar Arkaplanı (Status renginin soluk hali veya sabit bir renk)
    SolidBrush avatarBgBrush(Color(255, 60, 60, 60)); // Koyu gri zemin
    g.FillEllipse(&avatarBgBrush, avatarRect);

    // Avatar Çerçevesi (Status renginde)
    Pen avatarPen(cStatusMain, 2.0f);
    g.DrawEllipse(&avatarPen, avatarRect);

    // Baş Harf
    CString initials = ad.Left(1);
    initials.MakeUpper();

    Gdiplus::Font avatarFont(L"Segoe UI", (REAL)DpiScaleInt(14), FontStyleBold);
    StringFormat centerFmt;
    centerFmt.SetAlignment(StringAlignmentCenter);
    centerFmt.SetLineAlignment(StringAlignmentCenter);
    SolidBrush textBrushWhite(Color(255, 255, 255, 255));

    g.DrawString(initials, -1, &avatarFont, avatarRect, &centerFmt, &textBrushWhite);

    // =========================================================================
    // 5. İÇERİK METİNLERİ
    // =========================================================================
    REAL contentLeft = avatarRect.GetRight() + (REAL)DpiScaleInt(12);
    REAL textTop = rect.Y + (REAL)PADDING;

    // -- İsim Soyisim --
    Gdiplus::Font nameFont(L"Segoe UI Semibold", (REAL)DpiScaleInt(11), FontStyleRegular);
    ad.MakeUpper();
    g.DrawString(ad, -1, &nameFont, PointF(contentLeft, textTop), &textBrushWhite);

    // -- Database Customer ID (Cari Kod - Prominent) --
    textTop += (REAL)DpiScaleInt(18);
    Gdiplus::Font refFont(L"Consolas", (REAL)DpiScaleInt(9), FontStyleBold);
    SolidBrush textBrushDim(Color(255, 150, 150, 150));
    SolidBrush textBrushAccent(Color(255, 100, 180, 255)); // Accent color for DB ID
    g.DrawString(L"ID: ", -1, &refFont, PointF(contentLeft, textTop), &textBrushDim);
    g.DrawString(cariKod, -1, &refFont, PointF(contentLeft + DpiScaleInt(20), textTop), &textBrushAccent);

    // -- TC Kimlik Number (with masking) --
    textTop += (REAL)DpiScaleInt(16);
    Gdiplus::Font idFont(L"Segoe UI", (REAL)DpiScaleInt(8), FontStyleRegular);
    bool isUnmasked = (m_unmaskedIdItems.find(item) != m_unmaskedIdItems.end());
    CString displayId = isUnmasked ? tckimlik : MaskIdNumber(tckimlik);
    g.DrawString(L"TC: ", -1, &idFont, PointF(contentLeft, textTop), &textBrushDim);
    g.DrawString(displayId, -1, &idFont, PointF(contentLeft + DpiScaleInt(22), textTop), &textBrushWhite);

    // -- Telefon --
    textTop += (REAL)DpiScaleInt(16);
    Gdiplus::Font infoFont(L"Segoe UI", (REAL)DpiScaleInt(9), FontStyleRegular);
    REAL iconOffset = (REAL)DpiScaleInt(20);
    g.DrawString(L"📞", -1, &infoFont, PointF(contentLeft, textTop), &textBrushDim);
    g.DrawString(tel, -1, &infoFont, PointF(contentLeft + iconOffset, textTop), &textBrushWhite);

    // =========================================================================
    // 6. STATUS ROZETİ (Badge - Sağ Alt)
    // =========================================================================
    // Metin boyutunu ölç
    Gdiplus::Font badgeFont(L"Segoe UI", (REAL)DpiScaleInt(8), FontStyleBold);
    RectF boundBox;
    g.MeasureString(statusText, -1, &badgeFont, PointF(0, 0), &boundBox);

    REAL badgeH = (REAL)DpiScaleInt(18);
    REAL badgeW = boundBox.Width + (REAL)DpiScaleInt(16); // Padding

    RectF badgeRect(
        rect.GetRight() - badgeW - (REAL)PADDING,
        rect.GetBottom() - badgeH - (REAL)PADDING,
        badgeW, badgeH
    );

    GraphicsPath badgePath;
    REAL badgeR = badgeH / 2.0f; // Tam yuvarlak köşeler (Pill shape)
    badgePath.AddLine(badgeRect.X + badgeR, badgeRect.Y, badgeRect.GetRight() - badgeR, badgeRect.Y);
    badgePath.AddArc(badgeRect.GetRight() - badgeH, badgeRect.Y, badgeH, badgeH, 270, 180);
    badgePath.AddLine(badgeRect.GetRight() - badgeR, badgeRect.GetBottom(), badgeRect.X + badgeR, badgeRect.GetBottom());
    badgePath.AddArc(badgeRect.X, badgeRect.Y, badgeH, badgeH, 90, 180);
    badgePath.CloseFigure();

    // Badge Arkaplanı (Yarı saydam renk)
    SolidBrush badgeBrush(Color(40, cStatusMain.GetR(), cStatusMain.GetG(), cStatusMain.GetB()));
    g.FillPath(&badgeBrush, &badgePath);

    // Badge Çerçevesi
    Pen badgePen(Color(150, cStatusMain.GetR(), cStatusMain.GetG(), cStatusMain.GetB()), 1.0f);
    g.DrawPath(&badgePen, &badgePath);

    // Badge Yazısı (Status Renginde)
    SolidBrush badgeTextBrush(Color(255, cStatusMain.GetR(), cStatusMain.GetG(), cStatusMain.GetB()));
    g.DrawString(statusText, -1, &badgeFont, badgeRect, &centerFmt, &badgeTextBrush);

    // =========================================================================
    // 7. ACTION BUTTONS (Top Right - Three Buttons)
    // =========================================================================
    // We now have THREE buttons:
    // 1. Add button (+) - leftmost
    // 2. Edit button (✎) - middle  
    // 3. ID Toggle button (👁) - rightmost
    
    REAL btnSize = (REAL)DpiScaleInt(22);
    REAL btnSpacing = (REAL)DpiScaleInt(4);
    int alpha = (hovered || selected) ? 255 : 80;

    // Calculate button positions (right to left)
    REAL startX = rect.GetRight() - btnSize - (REAL)PADDING;
    
    // ---- Button 3: ID Toggle (👁) ----
    RectF btnIdToggle(startX, rect.Y + (REAL)PADDING, btnSize, btnSize);
    Pen eyeCirclePen(Color(alpha, 255, 255, 255), 1.0f);
    g.DrawEllipse(&eyeCirclePen, btnIdToggle);
    if (hovered) {
        SolidBrush btnFill(Color(40, 255, 255, 255));
        g.FillEllipse(&btnFill, btnIdToggle);
    }
    // Eye icon (simplified)
    REAL eyeMidX = btnIdToggle.X + btnSize / 2.0f;
    REAL eyeMidY = btnIdToggle.Y + btnSize / 2.0f;
    Pen eyePen(Color(alpha, 255, 255, 255), 1.5f);
    RectF eyeOuter(eyeMidX - 5, eyeMidY - 3, 10, 6);
    g.DrawEllipse(&eyePen, eyeOuter);
    SolidBrush pupilBrush(Color(alpha, 255, 255, 255));
    RectF pupilRect(eyeMidX - 2, eyeMidY - 2, 4, 4);
    g.FillEllipse(&pupilBrush, pupilRect);
    
    // ---- Button 2: Edit (✎) ----
    startX -= (btnSize + btnSpacing);
    RectF btnEdit(startX, rect.Y + (REAL)PADDING, btnSize, btnSize);
    Pen editCirclePen(Color(alpha, 255, 255, 255), 1.0f);
    g.DrawEllipse(&editCirclePen, btnEdit);
    if (hovered) {
        SolidBrush btnFill(Color(40, 255, 255, 255));
        g.FillEllipse(&btnFill, btnEdit);
    }
    // Pencil icon (simplified)
    REAL editMidX = btnEdit.X + btnSize / 2.0f;
    REAL editMidY = btnEdit.Y + btnSize / 2.0f;
    Pen pencilPen(Color(alpha, 255, 255, 255), 1.5f);
    g.DrawLine(&pencilPen, editMidX - 3, editMidY + 3, editMidX + 3, editMidY - 3);
    g.DrawLine(&pencilPen, editMidX + 3, editMidY - 3, editMidX + 4, editMidY - 4);
    g.DrawLine(&pencilPen, editMidX - 4, editMidY + 4, editMidX - 3, editMidY + 3);
    
    // ---- Button 1: Add (+) ----
    startX -= (btnSize + btnSpacing);
    RectF btnAdd(startX, rect.Y + (REAL)PADDING, btnSize, btnSize);
    Pen plusCirclePen(Color(alpha, 255, 255, 255), 1.0f);
    g.DrawEllipse(&plusCirclePen, btnAdd);
    if (hovered) {
        SolidBrush btnFill(Color(40, 255, 255, 255));
        g.FillEllipse(&btnFill, btnAdd);
    }
    // Plus sign
    Pen plusPen(Color(alpha, 255, 255, 255), 1.5f);
    REAL plusMidX = btnAdd.X + btnSize / 2.0f;
    REAL plusMidY = btnAdd.Y + btnSize / 2.0f;
    REAL len = btnSize / 4.0f;
    g.DrawLine(&plusPen, plusMidX - len, plusMidY, plusMidX + len, plusMidY); // Horizontal
    g.DrawLine(&plusPen, plusMidX, plusMidY - len, plusMidX, plusMidY + len); // Vertical
}
void CListCustomerView::ShowQuickAddMenu(POINT pt, const CString& cariKod)
{
    CMenu menu;
    menu.CreatePopupMenu();

    // Menü başlığı (Görsel bilgi için)
    menu.AppendMenu(MF_STRING | MF_DISABLED | MF_GRAYED, 0, L"Mülk Ekle: " + cariKod);
    menu.AppendMenu(MF_SEPARATOR, 0, L"");

    // Seçenekler (Mevcut ID'lerini kullanıyoruz)
    menu.AppendMenu(MF_STRING, IDM_ADD_HOME, L"🏠 Konut/Ev Ekle");
    menu.AppendMenu(MF_STRING, IDM_ADD_CAR,  L"🚗 Araba Ekle");
    menu.AppendMenu(MF_STRING, ID_EKLE_ARSA, L"🏞️ Arsa Ekle");
    menu.AppendMenu(MF_STRING, ID_EKLE_VILLA, L"🏰 Villa Ekle");
    menu.AppendMenu(MF_STRING, ID_EKLE_TARLA, L"🚜 Tarla Ekle");
    menu.AppendMenu(MF_STRING, ID_EKLE_TICARIALAN, L"🏢 Ticari Alan Ekle");

    // Menüyü göster ve seçimi yakala
    // TPM_RETURNCMD kullanarak menünün kapanmasını bekler ve seçilen ID'yi döner
    UINT cmdID = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, pt.x, pt.y, GetHwnd());

    if (cmdID > 0)
    {
        // Seçilen komutu doğrudan View'ın kendi OnCommand mekanizmasına gönderiyoruz
        // Bu sayede OnAddHome(cariKod) gibi fonksiyonlar tetiklenir
        this->PostMessage(WM_COMMAND, cmdID, 0);
    }
}









// ============================================================================
// ✅ EKSIK İMPLEMENTASYONLAR (CListCustomerView)
// ============================================================================
LRESULT CListCustomerView::HandleLButtonUp(LPARAM lparam)
{
    POINT pt = { GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };
    int clickedItem = -1;

    // 1. Check ID Toggle Button (rightmost)
    if (IsPointInIdToggleButton(pt, clickedItem))
    {
        // Toggle the masked/unmasked state for this item
        if (m_unmaskedIdItems.find(clickedItem) != m_unmaskedIdItems.end())
        {
            // Currently unmasked, mask it
            m_unmaskedIdItems.erase(clickedItem);
        }
        else
        {
            // Currently masked, unmask it
            m_unmaskedIdItems.insert(clickedItem);
        }
        
        // Repaint the card
        CRect rcItem;
        ListView_GetItemRect(GetHwnd(), clickedItem, &rcItem, LVIR_BOUNDS);
        rcItem.bottom = rcItem.top + DpiScaleInt(130);
        InvalidateRect(rcItem, FALSE);
        return 0;
    }

    // 2. Check Edit Button (middle)
    if (IsPointInEditButton(pt, clickedItem))
    {
        // Select the item and open edit dialog
        DeleteAllSelections();
        ListView_SetItemState(GetHwnd(), clickedItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

        CString cariKod = GetItemText(clickedItem, 0);
        OnEditCustomer(cariKod);
        return 0;
    }

    // 3. Check Add Button (leftmost)
    if (IsPointInAddButton(pt, clickedItem))
    {
        // Select the item and show quick add menu
        DeleteAllSelections();
        ListView_SetItemState(GetHwnd(), clickedItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

        CString cariKod = GetItemText(clickedItem, 0);
        
        // Convert to screen coordinates
        CPoint screenPt = pt;
        ::ClientToScreen(GetHwnd(), &screenPt);

        // Show popup menu
        ShowQuickAddMenu(screenPt, cariKod);
        return 0;
    }

    return WndProcDefault(WM_LBUTTONUP, 0, lparam);
}
LRESULT CListCustomerView::HandleKeyDown(WPARAM wparam)
{
    // Seçili öğe var mı?
    int iItem = ListView_GetNextItem(GetHwnd(), -1, LVNI_SELECTED);
    if (iItem == -1) return -1;

    switch (wparam)
    {
    case VK_RETURN: // ENTER Tuşu
        // Detay aç (Double Click ile aynı mantık)
        // OnEditCustomer();
        return 0;

    case VK_DELETE: // DELETE Tuşu
    {
        if (MessageBox(_T("Seçili müşteriyi silmek istediğinize emin misiniz?"), _T("Silme Onayı"), MB_YESNO | MB_ICONWARNING) == IDYES)
        {
            CString cariKod = GetItemText(iItem, 0);

            std::vector<CString> ids;
            ids.push_back(cariKod); // Tek elemanlı vektör oluştur
            if (db_Manager.DeleteCustomers(ids)) {
                ListView_DeleteItem(GetHwnd(), iItem);
            }
        }
    }
    return 0;

    case VK_F5: // F5 Yenileme
        // RefreshList();
        return 0;
    }

    return -1; // Diğer tuşlar (Ok tuşları vs.) varsayılan işlensin
}

LRESULT CListCustomerView::HandleRButtonDown(LPARAM lparam)
{
    // 1. ODAK VE KOORDİNAT

    if (GetFocus() != GetHwnd()) SetFocus();
    POINT pt = { GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };

    int clickedItem = -1;
    if (IsPointInAddButton(pt, clickedItem))
    {
        CString cariKod = GetItemText(clickedItem, 0);
        CPoint screenPt = pt;
        ::ClientToScreen(GetHwnd(), &screenPt);
        ShowRadialMenuAt(screenPt, cariKod);
        return 0; // İşlem tamam, aşağıya (seçim koduna) gitme
    }



    // 2. ÖNCEKİ SEÇİLİ ÖĞEYİ BUL
    int nOldItem = GetNextItem(-1, LVNI_SELECTED);

    // 3. TIKLANAN ÖĞEYİ BUL
    LVHITTESTINFO lvhti = { 0 };
    lvhti.pt = pt;
    ListView_HitTest(GetHwnd(), &lvhti);
    int nNewItem = lvhti.iItem;

    // 4. MANUEL HESAPLAMA (HitTest başarısızsa ve Kart Modundaysak)
    if (nNewItem == -1 && GetViewMode() == 1)
    {
        CRect rcCheck;
        const int TOTAL_CELL_HEIGHT = DpiScaleInt(130); // Kart yüksekliği
        int count = GetItemCount();
        for (int i = 0; i < count; ++i)
        {
            if (ListView_GetItemRect(GetHwnd(), i, &rcCheck, LVIR_BOUNDS))
            {
                rcCheck.bottom = rcCheck.top + TOTAL_CELL_HEIGHT;
                if (rcCheck.PtInRect(pt)) {
                    nNewItem = i;
                    break;
                }
            }
        }
    }

    // 5. SEÇİM MANTIĞI
    // Eğer boşluğa tıklandıysa -> Seçimleri temizle
// BOŞLUĞA TIKLANDIYSA -> Hepsini temizle
    if (nNewItem == -1)
    {
        // 🛑 ÇÖZÜM BURADA:
        // Eğer daha önce seçili bir kart varsa, onun kapladığı TÜM alanı yeniden çizdirmeliyiz.
        if (nOldItem != -1)
        {
            CRect rcOld;
            ListView_GetItemRect(GetHwnd(), nOldItem, &rcOld, LVIR_BOUNDS);

            // Kart Modundaysak, temizlenecek alanı kartın gerçek yüksekliğine genişlet
            if (GetViewMode() == 1)
            {
                rcOld.bottom = rcOld.top + DpiScaleInt(130); // Kart yüksekliği (DrawItem ile aynı olmalı)
            }

            // O bölgeyi geçersiz kıl (TRUE = Arka planı da silerek temizle)
            InvalidateRect(rcOld, TRUE);

            // Hemen boyansın ki göz kırpma olmasın
            UpdateWindow();
        }

        DeleteAllSelections(); // Şimdi mantıksal seçimi kaldır
        return 0;
    }
    // Eğer tıklanan öğe zaten seçili değilse -> Onu seç
    // (Eğer zaten seçiliyse dokunmuyoruz, kullanıcı belki çoklu seçim grubuna sağ tıklamıştır)
    if (nNewItem != nOldItem)
    {
        // Eski seçimi temizle ve boya (Artifact kalmaması için)
        if (nOldItem != -1)
        {
            ListView_SetItemState(GetHwnd(), nOldItem, 0, LVIS_SELECTED | LVIS_FOCUSED);

            CRect rcOld;
            ListView_GetItemRect(GetHwnd(), nOldItem, &rcOld, LVIR_BOUNDS);
            if (GetViewMode() == 1) rcOld.bottom = rcOld.top + DpiScaleInt(130);
            InvalidateRect(rcOld, TRUE);
        }

        // Yeni öğeyi seç ve boya
        ListView_SetItemState(GetHwnd(), nNewItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        EnsureVisible(nNewItem, FALSE);

        CRect rcNew;
        ListView_GetItemRect(GetHwnd(), nNewItem, &rcNew, LVIR_BOUNDS);
        if (GetViewMode() == 1) rcNew.bottom = rcNew.top + DpiScaleInt(130);
        InvalidateRect(rcNew, TRUE);
    }

    // Not: Buradan sonra Windows otomatik olarak WM_CONTEXTMENU mesajını tetikler.
    // Seçim işlemini burada hallettiğimiz için menü doğru öğe üzerinde açılır.

    return 0;
}




#ifndef IDM_STATUS_ULASILAMADI
  #define IDM_STATUS_ULASILAMADI  91522
#endif
#ifndef IDM_STATUS_OLUMSUZ
  #define IDM_STATUS_OLUMSUZ      91523
#endif
#ifndef IDM_STATUS_NORMAAL
  #define IDM_STATUS_NORMAAL      91524
#endif
#ifndef IDM_STATUS_PAHALI
  #define IDM_STATUS_PAHALI       91525
#endif

void CListCustomerView::ShowRadialMenuAt(POINT pt, const CString& cariKod)
{
    // 1. Koordinatları Ekran Koordinatına Çevir (Eğer HandleLButtonDown'da yapılmadıysa)
    // pt buraya client koordinatı geliyorsa:
    // ::ClientToScreen(GetHwnd(), &pt);

    // 2. Radial Menu Nesnesini Oluştur
    CRadialMenu* pMenu = new CRadialMenu();

    // 3. Öğeleri Ekle
    pMenu->AddItem(IDM_ADD_HOME, L"Ev Ekle", IDM_ADD_HOME);
    pMenu->AddItem(IDM_ADD_CAR, L"Araba Ekle", IDM_ADD_CAR);
    pMenu->AddItem(ID_EKLE_ARSA, L"Arsa Ekle", ID_EKLE_ARSA);
    pMenu->AddItem(ID_EKLE_VILLA, L"Villa Ekle", ID_EKLE_VILLA);
    pMenu->AddItem(ID_EKLE_TARLA, L"Tarla Ekle", ID_EKLE_TARLA);

    // 4. Komut Seçildiğinde Çalışacak Mantık
    pMenu->OnCommandSelected = [this, cariKod](UINT cmdID) {
        // Cari kodu saklayıp diyalog açarken kullanmak için MainFrame'e veya yerel değişkene atayabilirsin
        this->SendMessage(WM_COMMAND, cmdID, 0);
        };

    // 5. KRİTİK: Menüyü göstermeden önce ListView'ın fare yakalamasını bırak
    if (::GetCapture() == GetHwnd())
        ::ReleaseCapture();

    // 6. Menüyü Göster
    pMenu->Show(pt, GetHwnd());
}// Yardımcı Fonksiyon:
bool CListCustomerView::IsPointInAddButton(POINT pt, int& outItem)
{
    // 1. Önce farenin hangi satır (item) üzerinde olduğunu bul
    LVHITTESTINFO lvhti = { 0 };
    lvhti.pt = pt;
    int nItem = ListView_HitTest(GetHwnd(), &lvhti);

    // 2. Eğer standart HitTest kart modunda boşluktan dolayı bulamazsa manuel ara
    if (nItem == -1 && GetViewMode() == 1)
    {
        CRect rcCheck;
        int count = GetItemCount();
        for (int i = 0; i < count; ++i)
        {
            if (ListView_GetItemRect(GetHwnd(), i, &rcCheck, LVIR_BOUNDS))
            {
                rcCheck.bottom = rcCheck.top + DpiScaleInt(130);
                if (rcCheck.PtInRect(pt)) {
                    nItem = i;
                    break;
                }
            }
        }
    }

    // 3. Eğer bir item bulunduysa, tıklanan nokta o item'ın buton alanında mı bak
    if (nItem != -1) {
        CRect rcBtn = GetAddButtonRect(nItem);
        if (rcBtn.PtInRect(pt)) {
            outItem = nItem;
            return true;
        }
    }
    return false;
}

// Helper: Mask ID Number (show only first 3 and last 2 digits)
CString CListCustomerView::MaskIdNumber(const CString& idNumber)
{
    if (idNumber.IsEmpty())
        return _T("***********");
    
    int len = idNumber.GetLength();
    if (len <= 5)
        return _T("***********");
    
    // Show first 3 and last 2, mask the rest
    CString masked;
    masked = idNumber.Left(3) + _T("******") + idNumber.Right(2);
    return masked;
}

// Helper: Check if ID is unmasked for this item
bool CListCustomerView::IsIdNumberUnmasked(int nItem) const
{
    return m_unmaskedIdItems.find(nItem) != m_unmaskedIdItems.end();
}

// Helper: Get Edit Button Rectangle
CRect CListCustomerView::GetEditButtonRect(int nItem)
{
    CRect rcItem;
    ListView_GetItemRect(GetHwnd(), nItem, &rcItem, LVIR_BOUNDS);

    const int CARD_TOTAL_WIDTH = DpiScaleInt(270);
    const int btnSize = DpiScaleInt(22);
    const int btnSpacing = DpiScaleInt(4);
    const int btnPad = DpiScaleInt(12);
    const int CARD_PADDING = DpiScaleInt(6);

    int cardRight = rcItem.left + CARD_PADDING + CARD_TOTAL_WIDTH - (2 * CARD_PADDING);
    
    // Edit button is the middle button (second from right)
    CRect rcBtn(
        cardRight - (2 * btnSize) - btnSpacing - btnPad,
        rcItem.top + CARD_PADDING + btnPad,
        cardRight - btnSize - btnSpacing - btnPad,
        rcItem.top + CARD_PADDING + btnSize + btnPad
    );

    return rcBtn;
}

// Helper: Get ID Toggle Button Rectangle
CRect CListCustomerView::GetIdToggleButtonRect(int nItem)
{
    CRect rcItem;
    ListView_GetItemRect(GetHwnd(), nItem, &rcItem, LVIR_BOUNDS);

    const int CARD_TOTAL_WIDTH = DpiScaleInt(270);
    const int btnSize = DpiScaleInt(22);
    const int btnPad = DpiScaleInt(12);
    const int CARD_PADDING = DpiScaleInt(6);

    int cardRight = rcItem.left + CARD_PADDING + CARD_TOTAL_WIDTH - (2 * CARD_PADDING);
    
    // ID toggle button is the rightmost button
    CRect rcBtn(
        cardRight - btnSize - btnPad,
        rcItem.top + CARD_PADDING + btnPad,
        cardRight - btnPad,
        rcItem.top + CARD_PADDING + btnSize + btnPad
    );

    return rcBtn;
}

// Helper: Check if point is in Edit button
bool CListCustomerView::IsPointInEditButton(POINT pt, int& outItem)
{
    LVHITTESTINFO lvhti = { 0 };
    lvhti.pt = pt;
    int nItem = ListView_HitTest(GetHwnd(), &lvhti);

    if (nItem == -1 && GetViewMode() == VIEW_MODE_CARD)
    {
        CRect rcCheck;
        int count = GetItemCount();
        for (int i = 0; i < count; ++i)
        {
            if (ListView_GetItemRect(GetHwnd(), i, &rcCheck, LVIR_BOUNDS))
            {
                rcCheck.bottom = rcCheck.top + DpiScaleInt(130);
                if (rcCheck.PtInRect(pt)) {
                    nItem = i;
                    break;
                }
            }
        }
    }

    if (nItem != -1) {
        CRect rcBtn = GetEditButtonRect(nItem);
        if (rcBtn.PtInRect(pt)) {
            outItem = nItem;
            return true;
        }
    }
    return false;
}

// Helper: Check if point is in ID Toggle button
bool CListCustomerView::IsPointInIdToggleButton(POINT pt, int& outItem)
{
    LVHITTESTINFO lvhti = { 0 };
    lvhti.pt = pt;
    int nItem = ListView_HitTest(GetHwnd(), &lvhti);

    if (nItem == -1 && GetViewMode() == VIEW_MODE_CARD)
    {
        CRect rcCheck;
        int count = GetItemCount();
        for (int i = 0; i < count; ++i)
        {
            if (ListView_GetItemRect(GetHwnd(), i, &rcCheck, LVIR_BOUNDS))
            {
                rcCheck.bottom = rcCheck.top + DpiScaleInt(130);
                if (rcCheck.PtInRect(pt)) {
                    nItem = i;
                    break;
                }
            }
        }
    }

    if (nItem != -1) {
        CRect rcBtn = GetIdToggleButtonRect(nItem);
        if (rcBtn.PtInRect(pt)) {
            outItem = nItem;
            return true;
        }
    }
    return false;
}





LRESULT CListCustomerView::HandleLButtonDown(LPARAM lparam)
{
    // 1. ODAK VE KOORDİNAT
    if (GetFocus() != GetHwnd()) SetFocus();
    POINT pt = { GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };

    // ============================================================
    // REPORT MODE: right-side action buttons (Add/Edit/Delete)
    // ============================================================
    if (GetViewMode() == VIEW_MODE_REPORT)
    {
        LVHITTESTINFO ht{};
        ht.pt = pt;
        int nItem = ListView_HitTest(GetHwnd(), &ht);
        if (nItem != -1)
        {
            // Compute the same rectangles used by DrawReportActionButtons
            CRect rcItem;
            if (ListView_GetItemRect(GetHwnd(), nItem, &rcItem, LVIR_BOUNDS))
            {
                const int btnSize = DpiScaleInt(24);
                const int margin = DpiScaleInt(4);
                const int rightEdge = rcItem.right - margin;
                const int y = rcItem.top + (rcItem.Height() - btnSize) / 2;

                CRect rcDel(rightEdge - btnSize, y, rightEdge, y + btnSize);
                CRect rcEdit(rcDel.left - btnSize - margin, y, rcDel.left - margin, y + btnSize);
                CRect rcAdd(rcEdit.left - btnSize - margin, y, rcEdit.left - margin, y + btnSize);

                if (rcAdd.PtInRect(pt))
                {
                    CString cariKod = GetItemText(nItem, 0);
                    CPoint ptScreen = pt; ::ClientToScreen(GetHwnd(), &ptScreen);
                    ShowRadialMenuAt(ptScreen, cariKod);
                    return 0;
                }
                if (rcEdit.PtInRect(pt))
                {
                    // Select the clicked item for consistency
                    ListView_SetItemState(GetHwnd(), nItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
                    EnsureVisible(nItem, FALSE);

                    CRibbonFrameApp* pApp = GetContainerApp();
                    if (pApp && pApp->GetMainFrame().IsWindow())
                        pApp->GetMainFrame().SendMessage(WM_COMMAND, IDM_RECORD_UPDATE, 0);
                    return 0;
                }
                if (rcDel.PtInRect(pt))
                {
                    // Select first (so user sees what will be deleted)
                    ListView_SetItemState(GetHwnd(), nItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
                    EnsureVisible(nItem, FALSE);

                    if (MessageBox(_T("Seçili müşteriyi silmek istediğinize emin misiniz?"), _T("Silme Onayı"), MB_YESNO | MB_ICONWARNING) == IDYES)
                    {
                        CString cariKod = GetItemText(nItem, 0);
                        std::vector<CString> ids;
                        ids.push_back(cariKod);
                        if (db_Manager.DeleteCustomers(ids))
                            ListView_DeleteItem(GetHwnd(), nItem);
                    }
                    return 0;
                }
            }
        }
    }
    // Files.cpp -> HandleLButtonDown fonksiyonunun başına ekle
    //int clickedItem = -1;
    //if (IsPointInAddButton(pt, clickedItem))
    //{
    //    CString cariKod = GetItemText(clickedItem, 0);
    //    CPoint screenPt = pt;
    //    ::ClientToScreen(GetHwnd(), &screenPt);
    //    ShowRadialMenuAt(screenPt, cariKod);
    //    return 0; // İşlem tamam, aşağıya (seçim koduna) gitme
    //}
    // 2. ÖNCEKİ SEÇİLİ ÖĞEYİ BUL (Temizlemek için lazım)
    int nOldItem = GetNextItem(-1, LVNI_SELECTED);

    // 3. TIKLANAN ÖĞEYİ BUL
    LVHITTESTINFO lvhti = { 0 };
    lvhti.pt = pt;
    ListView_HitTest(GetHwnd(), &lvhti);
    int nNewItem = lvhti.iItem;

    // Manuel HitTest (Boşluklar için - Sadece Kart Modunda)
    if (nNewItem == -1 && GetViewMode() == 1)
    {
        CRect rcCheck;
        // DrawCardItemProfessional ile aynı yükseklik olmalı
        const int TOTAL_CELL_HEIGHT = DpiScaleInt(130);
        int count = GetItemCount();
        for (int i = 0; i < count; ++i)
        {
            if (ListView_GetItemRect(GetHwnd(), i, &rcCheck, LVIR_BOUNDS))
            {
                // Standart Rect'i kart boyutuna genişlet
                rcCheck.bottom = rcCheck.top + TOTAL_CELL_HEIGHT;
                if (rcCheck.PtInRect(pt)) {
                    nNewItem = i;
                    break;
                }
            }
        }
    }

    // BOŞLUĞA TIKLANDIYSA
    if (nNewItem == -1)
    {
        // Döngüyle tüm seçili öğeleri bul, seçimlerini kaldır ve tam boyutuyla yeniden çizdir
        int nItem = -1;
        while ((nItem = GetNextItem(-1, LVNI_SELECTED)) != -1)
        {
            // 1. Kartın alanını hesapla
            CRect rcItem;
            ListView_GetItemRect(GetHwnd(), nItem, &rcItem, LVIR_BOUNDS);

            // Kart modundaysak alanı aşağıya doğru genişlet (Tam temizlik için)
            if (GetViewMode() == 1) {
                rcItem.bottom = rcItem.top + DpiScaleInt(130);
            }

            // 2. ÖNCE Seçimi Kaldır (Artık seçili değil)
            ListView_SetItemState(GetHwnd(), nItem, 0, LVIS_SELECTED | LVIS_FOCUSED);

            // 3. SONRA Boyama Emri Ver (Seçimsiz haliyle tüm kartı boyayacak)
            InvalidateRect(rcItem, TRUE);
        }

        // Tüm işlemler bitince ekranı tazele
        UpdateWindow();
        return 0;
    }

    // ============================================================
    // SEÇİM DEĞİŞİMİ VE GÖRSEL YENİLEME
    // ============================================================

    // A) Eski Seçili Kartı Temizle (Farklıysa)
    if (nOldItem != -1 && nOldItem != nNewItem)
    {
        ListView_SetItemState(GetHwnd(), nOldItem, 0, LVIS_SELECTED | LVIS_FOCUSED);

        CRect rcOld;
        ListView_GetItemRect(GetHwnd(), nOldItem, &rcOld, LVIR_BOUNDS);
        if (GetViewMode() == 1) rcOld.bottom = rcOld.top + DpiScaleInt(130);

        // Eski kartı temizle
        InvalidateRect(rcOld, TRUE);
    }

    // B) Buton Tıklama Kontrolü (Sadece Kart Modu)
    if (GetViewMode() == 1)
    {
        CRect rcItem;
        ListView_GetItemRect(GetHwnd(), nNewItem, &rcItem, LVIR_BOUNDS);

        // Koordinatlar DrawItem ile %100 aynı olmalı
        const int CARD_W = DpiScaleInt(270);
        const int CARD_H = DpiScaleInt(130);
        const int PAD = DpiScaleInt(6);

        // Kartın iç çizim alanı
        CRect rcCard(
            rcItem.left + PAD, rcItem.top + PAD,
            rcItem.left + PAD + CARD_W - (2 * PAD),
            rcItem.top + PAD + CARD_H - (2 * PAD)
        );

        int btnW = DpiScaleInt(90);
        int btnH = DpiScaleInt(20);

        CRect rcButton(
            rcCard.right - btnW - DpiScaleInt(10),
            rcCard.bottom - btnH - DpiScaleInt(10),
            rcCard.right - DpiScaleInt(10),
            rcCard.bottom - DpiScaleInt(10)
        );

        if (rcButton.PtInRect(pt))
        {
            // 1. Önce Seçimi Yap (Görsel geri bildirim)
            ListView_SetItemState(GetHwnd(), nNewItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

            // 2. HEMEN YENİDEN ÇİZ (Menü açılmadan önce!)
            CRect rcRedraw = rcItem;
            rcRedraw.bottom = rcRedraw.top + CARD_H;
            InvalidateRect(rcRedraw, TRUE);
            UpdateWindow();

            CPoint ptScreen = pt;
            ::ClientToScreen(GetHwnd(), &ptScreen);

            CMenu menu;
            menu.CreatePopupMenu();

            // GRUP 1: HEDEF (Müşteri AKTİF)
            menu.AppendMenu(MF_STRING | MF_DISABLED | MF_GRAYED, 0, _T("--- HEDEF & BAŞARI ---"));
            menu.AppendMenu(MF_STRING, STS_RANDEVU, _T("📅 Randevu Alındı"));
            menu.AppendMenu(MF_STRING, STS_TEKLIF, _T("💰 Teklif Verdi"));
            menu.AppendMenu(MF_STRING, STS_SICAK_ALICI, _T("🔥 Sıcak Alıcı"));
            menu.AppendMenu(MF_STRING, STS_SICAK_SATICI, _T("🏠 Sıcak Satıcı"));
            menu.AppendMenu(MF_STRING, STS_ISLEM_TAMAM, _T("🤝 İşlem Tamamlandı"));

            menu.AppendMenu(MF_SEPARATOR, 0, _T(""));

            // GRUP 2: SÜREÇ (Müşteri AKTİF)
            menu.AppendMenu(MF_STRING | MF_DISABLED | MF_GRAYED, 0, _T("--- SÜREÇ & TAKİP ---"));
            menu.AppendMenu(MF_STRING, STS_DUSUNUYOR, _T("🤔 Düşünüyor / Kararsız"));
            menu.AppendMenu(MF_STRING, STS_TAKIP, _T("📆 Takip / Ertele"));
            menu.AppendMenu(MF_STRING, STS_ULASILAMADI, _T("📵 Ulaşılamadı"));
            menu.AppendMenu(MF_STRING, STS_YENI, _T("✅ Durum: Aktif (Standart)"));

            menu.AppendMenu(MF_SEPARATOR, 0, _T(""));

            // GRUP 3: OLUMSUZ (Müşteri PASİF)
            menu.AppendMenu(MF_STRING | MF_DISABLED | MF_GRAYED, 0, _T("--- KAPANIŞ / OLUMSUZ ---"));
            menu.AppendMenu(MF_STRING, STS_PAHALI, _T("💸 Fiyatı Pahalı Buldu"));
            menu.AppendMenu(MF_STRING, STS_OLUMSUZ, _T("❌ İlgilenmiyor"));
            menu.AppendMenu(MF_STRING, STS_PASIF, _T("⛔ Pasif / İptal"));

            menu.AppendMenu(MF_SEPARATOR, 0, _T(""));
            menu.AppendMenu(MF_STRING, 1001, _T("🔍 Müşteri Kartını Aç"));

            // Menüyü çalıştır
            int cmd = menu.TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN, ptScreen.x, ptScreen.y, GetHwnd(), NULL);

            if (cmd > 0)
            {
                if (cmd == 1001) {
                    ::SendMessage(GetParent(), WM_COMMAND, IDM_RECORD_UPDATE, 0);
                }
                // Eğer seçilen komut bizim STS kodlarımızdan biriyse
                else if (cmd >= 5000 && cmd <= 5050)
                {
                    CString cariKod = GetItemText(nNewItem, 0);
                    // DİKKAT: Artık doğrudan cmd'yi gönderiyoruz, dönüşüme gerek yok!
                    UpdateCustomerStatus(cariKod, cmd);
                }
                // Eğer özel işlemler varsa (Örn: Randevu diyaloğu açmak gibi)
                else if (cmd == STS_RANDEVU) {
                    // Hem durumu güncelle hem diyaloğu aç isterseniz buraya özel kod ekleyebilirsiniz
                    // Şimdilik standart update yapıyoruz:
                    CString cariKod = GetItemText(nNewItem, 0);
                    UpdateCustomerStatus(cariKod, cmd);
                    // ::SendMessage(GetParent(), WM_COMMAND, IDM_RANDEVU_OLUSTUR, 0); // İsterseniz bunu da açın
                }
            }
            return 0;
        }
    }

    // C) Normal Seçim (Gövdeye Tıklama)
    if (nOldItem != nNewItem)
    {
        ListView_SetItemState(GetHwnd(), nNewItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        EnsureVisible(nNewItem, FALSE);

        CRect rcNew;
        ListView_GetItemRect(GetHwnd(), nNewItem, &rcNew, LVIR_BOUNDS);
        if (GetViewMode() == 1) rcNew.bottom = rcNew.top + DpiScaleInt(130);

        // Yeni kartı boya
        InvalidateRect(rcNew, TRUE);
    }

    return 0;
}


/* Duran YALÇIN 17/01/2026 
Fonksiyon Adı.:  CListCustomerView::HandleLButtonDblClk
Açıklama :

*/
LRESULT CListCustomerView::HandleLButtonDblClk(LPARAM lparam)
{
    // In Card view, double-click behaves like a navigator (Explorer-like):
    // Customer -> Properties (TreeListView).
    if (GetViewMode() == VIEW_MODE_CARD)
    {
        POINT pt = { GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };

        // 1) Standard hit test
        LVHITTESTINFO lvhti{};
        lvhti.pt = pt;
        ListView_HitTest(GetHwnd(), &lvhti);
        int nItem = lvhti.iItem;

        // 2) Fallback hit test (Card view safety)
        if (nItem == -1)
        {
            CRect rcCheck;
            const int TOTAL_CELL_HEIGHT = DpiScaleInt(130);
            int count = GetItemCount();
            for (int i = 0; i < count; ++i)
            {
                if (ListView_GetItemRect(GetHwnd(), i, &rcCheck, LVIR_BOUNDS))
                {
                    rcCheck.bottom = rcCheck.top + TOTAL_CELL_HEIGHT;
                    if (rcCheck.PtInRect(pt)) { nItem = i; break; }
                }
            }
        }

        if (nItem != -1)
        {
            // IMPORTANT:
            // Do NOT change selection state here. Doing so from WM_LBUTTONDBLCLK
            // can re-enter WM_NOTIFY/LVN_ITEMCHANGED and cause UI deadlocks.
            // Just resolve the Cari_Kod and post a deferred navigation message.

            CString cariKod = GetItemText(nItem, 0);
            cariKod.Trim();

            if (!cariKod.IsEmpty())
            {
                // ✅ Professional approach:
                // Navigation MUST be handled inside Files module (CContainFiles),
                // not by MainFrame dock routing. We simply post a message to our
                // direct parent container. The container swaps ListView <-> TreeList.

                HWND hParent = ::GetParent(GetHwnd());
                if (::IsWindow(hParent))
                {
                    ::PostMessage(hParent, UWM_FILES_SHOW_PORTFOLIO, 0,
                        reinterpret_cast<LPARAM>(new CString(cariKod)));
                    return 0;
                }
            }
        }

        // If we couldn't resolve a customer, let default behavior continue.
        return -1;
    }

    // Report/List view: keep the classic behavior (open edit).
    CRibbonFrameApp* pApp = GetContainerApp();
    if (pApp && pApp->GetMainFrame().IsWindow())
        pApp->GetMainFrame().SendMessage(WM_COMMAND, IDM_RECORD_UPDATE, 0);

    return 0;
}
void CListCustomerView::DeleteAllSelections()
{
    // Tüm seçimleri temizle
    int nItem = -1;
    while ((nItem = GetNextItem(nItem, LVNI_SELECTED)) != -1)
    {
      ListView_SetItemState(GetHwnd(), nItem, 0, LVIS_SELECTED);
    }
}


/* Duran YALÇIN 17/01/2026 
Fonksiyon Adı.:  CListCustomerView::OpenCustomerPropertiesInTreeList
Açıklama :

*/
void CListCustomerView::OpenCustomerPropertiesInTreeList(const CString& cariKod)
{
    // ✅ FIX (2026-01-21): Restore TreeList (Portfolio) fill for the docked TreeListView (CTreeListVDlg).
    // We keep the old Files-container navigation as a fallback, but primary target is the existing right/bottom panel.

    CString ck = cariKod;
    ck.Trim();
    if (ck.IsEmpty())
        return;

    // 1) Try the docked TreeListView (MainFrame -> ID_TREEDOCKER)
    CRibbonFrameApp* pApp = GetContainerApp();
    if (pApp)
    {
        CMainFrame& mf = pApp->GetMainFrame();
        CDocker* pDocker = mf.GetDockFromID(ID_TREEDOCKER);
        if (pDocker)
        {
            CDockAllRestate* pDock = static_cast<CDockAllRestate*>(pDocker);
            CMyTreeListView& tree = pDock->GetCDockFiles().GetDockAllRestateTreeListView();
            if (tree.IsWindow())
            {
                tree.LoadPortfolioByCariKod(ck);
                return;
            }
        }
    }

    // 2) Fallback: Files module internal navigation (embedded portfolio view)
    HWND hParent = ::GetParent(GetHwnd());
    if (::IsWindow(hParent))
    {
        ::PostMessage(hParent, UWM_FILES_SHOW_PORTFOLIO, 0,
            reinterpret_cast<LPARAM>(new CString(ck)));
    }
}


CRect CListCustomerView::GetAddButtonRect(int nItem)
{
    CRect rcItem;
    ListView_GetItemRect(GetHwnd(), nItem, &rcItem, LVIR_BOUNDS);

    // DrawCardItemProfessional içindeki sabitlerle aynı olmalı
    const int CARD_TOTAL_WIDTH = DpiScaleInt(270);
    const int btnSize = DpiScaleInt(22);
    const int btnSpacing = DpiScaleInt(4);
    const int btnPad = DpiScaleInt(12);
    const int CARD_PADDING = DpiScaleInt(6);

    // Kartın sağ kenarını hesapla
    int cardRight = rcItem.left + CARD_PADDING + CARD_TOTAL_WIDTH - (2 * CARD_PADDING);

    // Add button is now the leftmost of three buttons
    CRect rcBtn(
        cardRight - (3 * btnSize) - (2 * btnSpacing) - btnPad,
        rcItem.top + CARD_PADDING + btnPad,
        cardRight - (2 * btnSize) - (2 * btnSpacing) - btnPad,
        rcItem.top + CARD_PADDING + btnSize + btnPad
    );

    return rcBtn;
}




// Files.cpp içindeki SetViewMode veya IDM_VIEW_REPORT case bloğuna:

void CListCustomerView::UpdateColumnWidths()
{
    // Genişlik dizisi (SetColumns ile aynı olmalı)
    int reportWidths[] = { 100, 200, 120, 120, 180, 250, 100, 100, 100, 100, 100, 100, 80, 200, 20 };

    for (int i = 0; i < _countof(reportWidths); ++i)
    {
        if (m_viewMode == VIEW_MODE_CARD)
        {
            // Kart modunda 0. kolon kart genişliği, diğerleri 0
            if (i == 0) SetColumnWidth(i, DpiScaleInt(270));
            else SetColumnWidth(i, 0);
        }
        else
        {
            // Rapor modunda tüm kolonları aç
            SetColumnWidth(i, DpiScaleInt(reportWidths[i]));
        }
    }
}






void CListCustomerView::UpdateCustomerStatus(const CString& cariKod, int commandID)
{
    int finalStatusID = commandID;

    // EĞER KOMUT RIBBON'DAN (ESKİ IDM KODLARIYLA) GELDİYSE ÇEVİR:
    // (Burası, üst toolbar butonlarınızın hala çalışması için gereklidir)
    switch (commandID)
    {
    case IDM_FLT_SICAK_ALICI:  finalStatusID = STS_SICAK_ALICI; break;
    case IDM_FLT_SICAK_SATICI: finalStatusID = STS_SICAK_SATICI; break;
    case IDM_FLT_TEKLIF:       finalStatusID = STS_TEKLIF; break;
    case IDM_FLT_TAMAMLANDI:   finalStatusID = STS_ISLEM_TAMAM; break;
    case IDM_SET_DUSUNECEK:    finalStatusID = STS_DUSUNUYOR; break;
    case IDM_FLT_TAKIP:        finalStatusID = STS_TAKIP; break;
    case IDM_SET_CALISIR:      finalStatusID = STS_YENI; break;
    case IDM_SET_CALISMAZ:     finalStatusID = STS_PASIF; break;
        // Eğer zaten STS_... geldiyse (Kart menüsünden), default olarak aynen kalır.
    }

    // 1. ID'yi stringe çevir (Veritabanı için)
    CString strStatusID;
    strStatusID.Format(_T("%d"), finalStatusID);

    // 2. Veritabanını Güncelle
    if (db_Manager.UpdateCustomerStatus(cariKod, strStatusID))
    {
        // 3. UI'ı Güncelle (Sadece ilgili satırı)
        int totalItems = GetItemCount();
        for (int i = 0; i < totalItems; i++)
        {
            if (GetItemText(i, 0) == cariKod)
            {
                // Durum kolonunu (14) güncelle
                SetItemText(i, 14, strStatusID);

                // Satırı yeniden boya
                CRect rcItem;
                ListView_GetItemRect(GetHwnd(), i, &rcItem, LVIR_BOUNDS);
                if (GetViewMode() == 1) rcItem.bottom = rcItem.top + DpiScaleInt(130);
                InvalidateRect(rcItem, FALSE);
                break;
            }
        }
        TRACE(L"✅ Durum Güncellendi: %s -> %d\n", cariKod, finalStatusID);
    }
    else
    {
        MessageBox(_T("Veritabanı hatası!"), _T("Hata"), MB_ICONERROR);
    }
}// ============================================================================
// ✅ OnCustomDraw İmplementasyonu (KRİTİK!)
// ============================================================================

// ============================================================================
// ✅ OnCustomDraw: Ana Yönlendirici
// ============================================================================
LRESULT CListCustomerView::OnCustomDraw(NMCUSTOMDRAW* pNMCD)
{
    LPNMLVCUSTOMDRAW pNMLVCD = reinterpret_cast<LPNMLVCUSTOMDRAW>(pNMCD);
    CDC dc(pNMLVCD->nmcd.hdc);

    switch (pNMCD->dwDrawStage)
    {
    case CDDS_PREPAINT:
        return CDRF_NOTIFYITEMDRAW;

    case CDDS_ITEMPREPAINT:
        // 1. KART GÖRÜNÜMÜ
        if (m_viewMode == VIEW_MODE_CARD)
        {
            DrawCardItemProfessional(pNMLVCD, dc);
            return CDRF_SKIPDEFAULT;
        }
        // 2. RAPOR GÖRÜNÜMÜ (Pro Multi-Line)
        else
        {
            DrawReportItemMultiLine(pNMLVCD, dc);
            return CDRF_SKIPDEFAULT;
        }

    default:
        return CDRF_DODEFAULT;
    }
}

// ============================================================================
// 📊 DrawReportItem: Modern Liste Satırı Çizimi
// ============================================================================
void CListCustomerView::DrawReportItem(LPNMLVCUSTOMDRAW pLVCD, CDC& dc)
{
    using namespace Gdiplus;
    Graphics g(dc.GetHDC());
    g.SetSmoothingMode(SmoothingModeAntiAlias);
    g.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

    int item = (int)pLVCD->nmcd.dwItemSpec;
    if (item < 0 || item >= GetItemCount()) return;

    CRect rcItem;
    ListView_GetItemRect(GetHwnd(), item, &rcItem, LVIR_BOUNDS);

    bool isSelected = (ListView_GetItemState(GetHwnd(), item, LVIS_SELECTED) & LVIS_SELECTED);
    bool isHovered = (item == m_hoverItem);

    // 1. ARKA PLAN (Zebra & Selection)
    Color bgColor;
    if (isSelected)
        bgColor = Color(255, 45, 50, 65); // Seçili (Koyu Mavi/Gri)
    else if (isHovered)
        bgColor = Color(255, 50, 50, 50); // Hover (Biraz daha açık)
    else if (item % 2 == 1)
        bgColor = Color(255, 30, 30, 30); // Tek satırlar (Koyu Gri)
    else
        bgColor = Color(255, 35, 35, 35); // Çift satırlar (Biraz daha açık gri)

    SolidBrush bgBrush(bgColor);
    RectF rfItem((REAL)rcItem.left, (REAL)rcItem.top, (REAL)rcItem.Width(), (REAL)rcItem.Height());
    g.FillRectangle(&bgBrush, rfItem);

    // Seçim Kenarlığı
    if (isSelected) {
        Pen selPen(Color(255, 64, 158, 255), 1.0f); // İnce mavi çerçeve
        // İçeri taşma olmasın diye 1px içerden
        g.DrawRectangle(&selPen, rfItem.X + 0.5f, rfItem.Y + 0.5f, rfItem.Width - 1.0f, rfItem.Height - 1.0f);
    }

    // Sol Status Çizgisi (İnce Stripe)
    int durumID = _ttoi(GetItemText(item, 14)); // Durum ID'si
    Color cStripe = Color::Gray;
    // ... (Renk atama mantığı CardView ile aynı, switch-case aşağıda)
    switch (durumID) {
    case STS_RANDEVU: cStripe = Color(155, 89, 182); break;
    case STS_SICAK_ALICI: cStripe = Color(231, 76, 60); break;
    case STS_YENI: cStripe = Color(64, 115, 255); break;
    case STS_ISLEM_TAMAM: cStripe = Color(26, 188, 156); break;
        // ... diğer durumlar
    default: cStripe = Color(100, 100, 100); break;
    }

    SolidBrush stripeBrush(cStripe);
    g.FillRectangle(&stripeBrush, rfItem.X, rfItem.Y, 4.0f, rfItem.Height);

    // 2. HÜCRELERİ ÇİZ (SÜTUN SÜTUN)
    // Sütun sayısını al
    HWND hHeader = ListView_GetHeader(GetHwnd());
    int colCount = Header_GetItemCount(hHeader);

    for (int col = 0; col < colCount; ++col)
    {
        CRect rcCell;
        ListView_GetSubItemRect(GetHwnd(), item, col, LVIR_BOUNDS, &rcCell);

        // İlk sütun (0) için özel rect düzeltmesi gerekebilir, ama LVIR_BOUNDS genelde tüm satırı verir subitem 0 için.
        // Doğrusu LVIR_LABEL kullanmaktır ama biz Grid çiziyoruz.
        // Manuel hesap: Header'dan rect alıp satır pozisyonuna uyarlamak en garantisidir.
        CRect rcHeader;
        Header_GetItemRect(hHeader, col, &rcHeader);
        rcCell.left = rcHeader.left;
        rcCell.right = rcHeader.right;

        // Çizim alanını daralt (Padding)
        rcCell.DeflateRect(DpiScaleInt(6), 0);

        DrawReportCell(g, item, col, rcCell, isSelected, durumID);
    }

    // 3. HOVER BUTONLARI (Satırın en sağına)
    if (isHovered || isSelected)
    {
        DrawReportActionButtons(g, rcItem, isHovered, isSelected);
    }
}

// ============================================================================
// ✍️ DrawReportCell: Hücre İçeriği
// ============================================================================
void CListCustomerView::DrawReportCell(Graphics& g, int item, int col, CRect rcCell, bool isSelected, int statusID)
{
    CString text = GetItemText(item, col);
    RectF layoutRect((REAL)rcCell.left, (REAL)rcCell.top, (REAL)rcCell.Width(), (REAL)rcCell.Height());

    SolidBrush textBrush(Color(230, 230, 230)); // Varsayılan metin rengi (Beyazımsı)
    if (!isSelected) textBrush.SetColor(Color(200, 200, 200));

    StringFormat format;
    format.SetAlignment(StringAlignmentNear);
    format.SetLineAlignment(StringAlignmentCenter);
    format.SetTrimming(StringTrimmingEllipsisCharacter);
    format.SetFormatFlags(StringFormatFlagsNoWrap);

    // --- ÖZEL SÜTUN MANTIKLARI ---

    // COL 0: CARİ KOD (Monospace, Sönük)
    if (col == 0)
    {
        Gdiplus::Font font(L"Consolas", (REAL)DpiScaleInt(9), FontStyleRegular);
        SolidBrush dimBrush(Color(150, 150, 150)); // Gri
        g.DrawString(text, -1, &font, layoutRect, &format, &dimBrush);
    }
    // COL 1: AD SOYAD (Kalın, Parlak)
    else if (col == 1)
    {
        Gdiplus::Font font(L"Segoe UI Semibold", (REAL)DpiScaleInt(10), FontStyleRegular);
        SolidBrush brightBrush(Color(255, 255, 255));
        g.DrawString(text, -1, &font, layoutRect, &format, &brightBrush);
    }
    // COL 12 veya 14: DURUM (Rozet/Badge Olarak Çiz)
    else if (col == 12 || col == 14) // Durum Kolonu
    {
        // Metni Durum ID'sine göre renklendir
        Color cBadge;
        switch (statusID) {
        case STS_RANDEVU: cBadge = Color(155, 89, 182); break;
        case STS_TEKLIF: cBadge = Color(46, 204, 113); break;
        case STS_SICAK_ALICI: cBadge = Color(231, 76, 60); break;
        case STS_ISLEM_TAMAM: cBadge = Color(26, 188, 156); break;
        case STS_YENI: cBadge = Color(64, 115, 255); break;
        default: cBadge = Color(127, 140, 141); break;
        }

        // Metin Boyutunu Ölç
        Gdiplus::Font badgeFont(L"Segoe UI", (REAL)DpiScaleInt(8), FontStyleBold);
        RectF bound;
        g.MeasureString(text, -1, &badgeFont, PointF(0, 0), &bound);

        REAL badgeH = (REAL)DpiScaleInt(18);
        REAL badgeW = bound.Width + 20.0f;

        // Badge Rect (Sola hizalı ama dikey ortalı)
        RectF badgeRect(layoutRect.X, layoutRect.Y + (layoutRect.Height - badgeH) / 2, badgeW, badgeH);

        // Badge Çiz
        GraphicsPath path;
        path.AddArc(badgeRect.X, badgeRect.Y, badgeH, badgeH, 90, 180);
        path.AddArc(badgeRect.GetRight() - badgeH, badgeRect.Y, badgeH, badgeH, 270, 180);
        path.CloseFigure();

        SolidBrush bg(Color(40, cBadge.GetR(), cBadge.GetG(), cBadge.GetB())); // %15 Opacity
        Pen border(Color(180, cBadge.GetR(), cBadge.GetG(), cBadge.GetB()), 1.0f);

        g.FillPath(&bg, &path);
        g.DrawPath(&border, &path);

        // Yazı (Ortalı)
        SolidBrush txt(Color(255, cBadge.GetR(), cBadge.GetG(), cBadge.GetB()));
        format.SetAlignment(StringAlignmentCenter);
        g.DrawString(text, -1, &badgeFont, badgeRect, &format, &txt);
    }
    // DİĞERLERİ (Standart)
    else
    {
        Gdiplus::Font font(L"Segoe UI", (REAL)DpiScaleInt(9), FontStyleRegular);
        g.DrawString(text, -1, &font, layoutRect, &format, &textBrush);
    }
}

// Files.cpp içine yardımcı bir fonksiyon ekleyin veya OnInitialUpdate'e koyun:
void CListCustomerView::SetRowHeight(int height)
{
    // ListView satır yüksekliğini artırmanın en temiz yolu:
    // SMALL imagelist boyunu büyütmektir.
    // CRITICAL: Imagelist MUST live as long as the ListView.
    // If you create it on the stack and assign it to ListView, it will be destroyed
    // and ListView will crash on exit when it tries to free it.

    if (height <= 0) height = DpiScaleInt(60);

    if (m_reportRowHeightIL.GetHandle())
        m_reportRowHeightIL.Destroy();

    m_reportRowHeightIL.Create(1, height, ILC_COLOR32, 1, 1);
    ListView_SetImageList(GetHwnd(), m_reportRowHeightIL, LVSIL_SMALL);
}

// Bunu OnInitialUpdate içinde çağırın:
// SetRowHeight(DpiScaleInt(75)); // 3 satır metin sığacak kadar yer


// Files.cpp - OnCustomDraw içindeki yönlendirmeyi güncelleyin:
/*
    if (m_viewMode == VIEW_MODE_REPORT) {
        DrawReportItemMultiLine(pNMLVCD, dc);
        return CDRF_SKIPDEFAULT;
    }
*/

void CListCustomerView::DrawReportItemMultiLine(LPNMLVCUSTOMDRAW pLVCD, CDC& dc)
{
    using namespace Gdiplus;
    Graphics g(dc.GetHDC());

    // Kalite Ayarları
    g.SetSmoothingMode(SmoothingModeAntiAlias);
    g.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);
    g.SetInterpolationMode(InterpolationModeHighQualityBicubic);

    int item = (int)pLVCD->nmcd.dwItemSpec;
    if (item < 0 || item >= GetItemCount()) return;

    // 1. ALAN HESABI
    CRect rcItem;
    ListView_GetItemRect(GetHwnd(), item, &rcItem, LVIR_BOUNDS);

    // Kenar Boşlukları (Satırlar birbirine yapışmasın, arada boşluk olsun)
    int marginY = DpiScaleInt(4);
    int marginX = DpiScaleInt(8);

    // Çizim Alanı (Panel)
    RectF rect(
        (REAL)rcItem.left + marginX,
        (REAL)rcItem.top + marginY / 2.0f,
        (REAL)rcItem.Width() - (marginX * 2),
        (REAL)rcItem.Height() - marginY
    );

    // Durumlar
    bool isSelected = (ListView_GetItemState(GetHwnd(), item, LVIS_SELECTED) & LVIS_SELECTED);
    bool isHovered = (item == m_hoverItem);

    // Verileri Al
    CString cariKod = GetItemText(item, 0);
    CString ad = GetItemText(item, 1);
    CString tel = GetItemText(item, 2);
    CString mail = GetItemText(item, 4);
    CString tarih = GetItemText(item, 11);
    int durumID = _ttoi(GetItemText(item, 14));

    // --- RENK PALETİ ---
    Color cBg, cBorder;
    Color cTextMain = Color(30, 30, 30);      // Siyahımsı (Light Mode)
    Color cTextSub = Color(100, 100, 100);    // Gri
    Color cAccent;                            // Durum rengi

    // Durum Rengi Belirle
    switch (durumID) {
    case STS_RANDEVU: cAccent = Color(155, 89, 182); break; // Mor
    case STS_SICAK_ALICI: cAccent = Color(231, 76, 60); break; // Kırmızı
    case STS_YENI: cAccent = Color(52, 152, 219); break; // Mavi
    case STS_ISLEM_TAMAM: cAccent = Color(46, 204, 113); break; // Yeşil
    default: cAccent = Color(149, 165, 166); break; // Gri
    }

    // Seçim Durumuna Göre Arka Plan
    if (isSelected) {
        // Seçiliyse: Hafif Mavi/Gri degrade, belirgin çerçeve
        cBg = Color(255, 235, 245, 255);
        cBorder = Color(255, 52, 152, 219); // Mavi Çerçeve
    }
    else if (isHovered) {
        // Hover: Çok açık gri
        cBg = Color(255, 250, 250, 250);
        cBorder = Color(255, 200, 200, 200);
    }
    else {
        // Normal: Beyaz
        cBg = Color(255, 255, 255, 255);
        cBorder = Color(255, 230, 230, 230); // Çok silik çerçeve
    }

    // =========================================================================
    // 2. KAVİSLİ PANEL ÇİZİMİ (ROUNDED RECT)
    // =========================================================================
    GraphicsPath path;
    REAL r = (REAL)DpiScaleInt(8); // Köşe Yarıçapı (Kavis)

    // Sağ köşeler biraz daha kavisli, sol düz veya tam tersi yapılabilir.
    // Biz modern "Card" görünümü için 4 köşeyi de yuvarlıyoruz.
    path.AddArc(rect.X, rect.Y, r * 2, r * 2, 180, 90);
    path.AddArc(rect.GetRight() - r * 2, rect.Y, r * 2, r * 2, 270, 90);
    path.AddArc(rect.GetRight() - r * 2, rect.GetBottom() - r * 2, r * 2, r * 2, 0, 90);
    path.AddArc(rect.X, rect.GetBottom() - r * 2, r * 2, r * 2, 90, 90);
    path.CloseFigure();

    // Gölge (Hafif Derinlik)
    if (!isSelected) {
        RectF shadowRect = rect;
        shadowRect.Offset(0, 2);
        GraphicsPath shadowPath;
        shadowPath.AddRectangle(shadowRect); // Basit gölge
        SolidBrush shadowBrush(Color(10, 0, 0, 0));
        g.FillPath(&shadowBrush, &path); // Path ile aynı şekilde gölge
    }

    // Arka Planı Doldur
    SolidBrush bgBrush(cBg);
    g.FillPath(&bgBrush, &path);

    // Çerçeveyi Çiz
    Pen borderPen(cBorder, 1.0f);
    g.DrawPath(&borderPen, &path);

    // Sol Kenar Çizgisi (Status Strip - Kalın ve Renkli)
    // Gdiplus Clip kullanarak sadece kartın sol tarafını boyuyoruz
    g.SetClip(&path, CombineModeIntersect);
    RectF stripRect(rect.X, rect.Y, (REAL)DpiScaleInt(5), rect.Height);
    SolidBrush stripBrush(cAccent);
    g.FillRectangle(&stripBrush, stripRect);
    g.ResetClip();

    // =========================================================================
    // 3. İÇERİK YERLEŞİMİ (3 SATIRLI MİMARİ)
    // =========================================================================

    // Fontlar
    Gdiplus::Font fontTitle(L"Segoe UI", (REAL)DpiScaleInt(11), FontStyleBold);
    Gdiplus::Font fontDetail(L"Segoe UI", (REAL)DpiScaleInt(9), FontStyleRegular);
    Gdiplus::Font fontMeta(L"Consolas", (REAL)DpiScaleInt(8), FontStyleRegular);

    // Koordinatlar
    REAL leftPad = (REAL)DpiScaleInt(18); // Sol şeritten uzaklık
    REAL contentX = rect.X + leftPad;
    REAL rowH = rect.Height / 3.0f; // Kabaca 3 satıra böl
    REAL currentY = rect.Y + (REAL)DpiScaleInt(6); // Üst boşluk

    // --- SATIR 1: İSİM VE DURUM ---
    // İsim
    SolidBrush brushMain(cTextMain);
    g.DrawString(ad, -1, &fontTitle, PointF(contentX, currentY), &brushMain);

    // Sağ Üstte Tarih (Meta veri)
    SolidBrush brushMeta(Color(150, 150, 150));
    StringFormat rightAlign;
    rightAlign.SetAlignment(StringAlignmentFar);
    RectF dateRect(rect.GetRight() - 150, currentY + 2, 140, 20);
    g.DrawString(tarih, -1, &fontMeta, dateRect, &rightAlign, &brushMeta);

    // --- SATIR 2: İLETİŞİM (İKONLU) ---
    currentY += (REAL)DpiScaleInt(20);

    // Telefon
    g.DrawString(L"📞 " + tel, -1, &fontDetail, PointF(contentX, currentY), &brushMeta);

    // E-posta (Telefonun biraz sağına)
    REAL mailOffset = (REAL)DpiScaleInt(140);
    g.DrawString(L"✉ " + mail, -1, &fontDetail, PointF(contentX + mailOffset, currentY), &brushMeta);

    // --- SATIR 3: TEKNİK BİLGİ VE ETİKETLER ---
    currentY += (REAL)DpiScaleInt(20);

    // Cari Kod (Hap şeklinde etiket içinde)
    RectF tagRect(contentX, currentY, (REAL)DpiScaleInt(80), (REAL)DpiScaleInt(16));
    SolidBrush tagBg(Color(20, 0, 0, 0)); // Çok hafif gri
    g.FillRectangle(&tagBg, tagRect);
    g.DrawRectangle(&borderPen, tagRect); // İnce çerçeve

    StringFormat centerFmt;
    centerFmt.SetAlignment(StringAlignmentCenter);
    centerFmt.SetLineAlignment(StringAlignmentCenter);
    g.DrawString(cariKod, -1, &fontMeta, tagRect, &centerFmt, &brushMeta);

    // Sağ Altta Action Butonları (Sadece Hover veya Seçiliyse)
    if (isHovered || isSelected) {

        // Çağırırken:
        CRect rcBtnArea((int)rect.X, (int)rect.Y, (int)(rect.X + rect.Width), (int)(rect.Y + rect.Height));
        DrawReportActionButtons(g, rcBtnArea, isHovered, isSelected);
       // DrawReportActionButtons(g, rect, isHovered, isSelected); // Önceki koddaki fonksiyonu kullanır
    }

    // --- SEÇİM VURGUSU (GLOW) ---
    // Seçiliyse tüm kartın etrafına hafif bir parlama atalım
    if (isSelected) {
        Pen glowPen(Color(80, 52, 152, 219), 2.0f);
        g.DrawPath(&glowPen, &path);
    }
}


// ============================================================================
// 🛠️ DrawReportActionButtons: Satır Sonu İkonları
// ============================================================================
void CListCustomerView::DrawReportActionButtons(Graphics& g, CRect rcItem, bool isHovered, bool isSelected)
{
    // Butonları satırın en sağına yerleştir
    int btnSize = DpiScaleInt(24);
    int margin = DpiScaleInt(4);
    int rightEdge = rcItem.right - margin;

    // 3 Adet Buton: [Sil] [Düzenle] [Ekle (+)]
    // İkonları Unicode karakter veya basit şekillerle çizebiliriz, 
    // ya da ImageList'ten çekebiliriz. Şimdilik modern vektörel çizim yapalım.

    Color btnNormal(0, 0, 0, 0); // Görünmez
    Color btnHover(40, 255, 255, 255);
    Color iconColor(200, 255, 255, 255);

    // Buton 1: SİL (En sağda)
    RectF btn1((REAL)(rightEdge - btnSize), (REAL)(rcItem.top + (rcItem.Height() - btnSize) / 2), (REAL)btnSize, (REAL)btnSize);

    // Buton 2: DÜZENLE (Ortada)
    RectF btn2(btn1.X - btnSize - margin, btn1.Y, (REAL)btnSize, (REAL)btnSize);

    // Buton 3: EKLE (Solda)
    RectF btn3(btn2.X - btnSize - margin, btn2.Y, (REAL)btnSize, (REAL)btnSize);

    // Mouse pozisyonunu kontrol et (Hangi butondayız?)
    POINT pt; GetCursorPos(&pt); ::ScreenToClient(*this,&pt);
    PointF ptf((REAL)pt.x, (REAL)pt.y);

    auto DrawIconBtn = [&](RectF rect, CString symbol, Color cIcon)
        {
            bool mouseOver = rect.Contains(ptf);
            if (mouseOver) {
                SolidBrush bg(Color(60, 255, 255, 255));
                g.FillEllipse(&bg, rect);
                cIcon = Color(255, 255, 255, 255); // Parlak beyaz
            }
            else if (isSelected) {
                // Seçiliyse hafif belli olsun
                // SolidBrush bg(Color(20, 255, 255, 255));
                // g.FillEllipse(&bg, rect);
            }

            SolidBrush brush(cIcon);
            Gdiplus::Font iconFont(L"Segoe UI Symbol", (REAL)DpiScaleInt(11), FontStyleRegular);
            StringFormat fmt;
            fmt.SetAlignment(StringAlignmentCenter);
            fmt.SetLineAlignment(StringAlignmentCenter);
            g.DrawString(symbol, -1, &iconFont, rect, &fmt, &brush);

            Pen border(Color(50, 255, 255, 255), 1.0f);
            g.DrawEllipse(&border, rect);
        };

    DrawIconBtn(btn1, L"🗑", Color(200, 255, 80, 80)); // Çöp Kutusu (Kırmızımsı)
    DrawIconBtn(btn2, L"✎", Color(200, 80, 200, 255));  // Kalem (Mavi)
    DrawIconBtn(btn3, L"＋", Color(200, 180, 255, 180)); // Plus
}// ============================================================================
// ✅ DrawHoverBar İmplementasyonu
// ============================================================================

void CListCustomerView::DrawHoverBar(LPNMLVCUSTOMDRAW pLVCD, CDC& dc)
{
    using namespace Gdiplus;

    int item = static_cast<int>(pLVCD->nmcd.dwItemSpec);

    // 1. KONTROLLER
    // Sadece hover olan item çizilir
    if (item < 0 || item != m_hoverItem) return;

    // Seçiliyse Hover ÇİZME (Seçim efekti zaten baskın olmalı)
    if ((pLVCD->nmcd.uItemState & CDIS_SELECTED)) return;

    // --- SABİTLER (Ana kart ile birebir aynı olmalı) ---
    const int CARD_TOTAL_WIDTH = DpiScaleInt(270);
    const int CARD_TOTAL_HEIGHT = DpiScaleInt(130);
    const int CARD_PADDING = DpiScaleInt(6);
    const REAL CARD_RADIUS = (REAL)DpiScaleInt(12);

    // --- VERİ HAZIRLIĞI ---
    CString mail = GetItemText(item, 4);
    // String literalini CString(...) içine alarak belirsizliği gideriyoruz:
    CString infoText = mail.IsEmpty() ? CString(L"İletişim bilgisi yok") : (L"✉ " + mail);
    // 2. KOORDİNATLAR
    CRect rcItem;
    if (ListView_GetItemRect(GetHwnd(), item, &rcItem, LVIR_BOUNDS) == FALSE) return;

    // GDI+ Başlat
    Graphics g(dc.GetHDC());
    g.SetSmoothingMode(SmoothingModeAntiAlias);
    g.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

    // 3. ANA KART ŞEKLİNİ OLUŞTUR (Maskeleme İçin)
    RectF rect(
        (REAL)rcItem.left + CARD_PADDING,
        (REAL)rcItem.top + CARD_PADDING,
        (REAL)CARD_TOTAL_WIDTH - (2 * CARD_PADDING),
        (REAL)CARD_TOTAL_HEIGHT - (2 * CARD_PADDING)
    );

    GraphicsPath path;
    REAL d = CARD_RADIUS * 2;
    path.AddArc(rect.X, rect.Y, d, d, 180, 90);
    path.AddArc(rect.X + rect.Width - d, rect.Y, d, d, 270, 90);
    path.AddArc(rect.X + rect.Width - d, rect.Y + rect.Height - d, d, d, 0, 90);
    path.AddArc(rect.X, rect.Y + rect.Height - d, d, d, 90, 90);
    path.CloseFigure();

    // 4. MUCİZEVİ DOKUNUŞ: CLIPPING (MASKELEME)
    // Bu satır sayesinde ne çizersek çizelim kartın dışına taşmaz, köşeleri mükemmel olur.
    g.SetClip(&path, CombineModeIntersect);

    // 5. HOVER BAR ÇİZİMİ (Glassy Look)
    REAL barHeight = (REAL)DpiScaleInt(28);
    RectF barRect(rect.X, rect.GetBottom() - barHeight, rect.Width, barHeight);

    // Yarı saydam koyu zemin (Blur effect hissi verir)
    // Siyah yerine koyu lacivert/gri daha modern durur
    Color barColor1(210, 20, 25, 30);
    Color barColor2(240, 5, 5, 10);
    LinearGradientBrush barBrush(barRect, barColor1, barColor2, LinearGradientModeVertical);

    g.FillRectangle(&barBrush, barRect);

    // Bar'ın üstüne ince bir çizgi (Highlight)
    Pen topBorderPen(Color(100, 255, 255, 255), 1.0f);
    g.DrawLine(&topBorderPen, barRect.X, barRect.Y, barRect.GetRight(), barRect.Y);

    // 6. METİN YAZIMI (Ortalanmış)
    Gdiplus::Font infoFont(L"Segoe UI", (REAL)DpiScaleInt(9), FontStyleRegular);
    SolidBrush textBrush(Color(255, 255, 255, 255));

    StringFormat format;
    format.SetAlignment(StringAlignmentCenter);     // Yatay Ortala
    format.SetLineAlignment(StringAlignmentCenter); // Dikey Ortala
    format.SetTrimming(StringTrimmingEllipsisCharacter); // Sığmazsa ... koy
    format.SetFormatFlags(StringFormatFlagsNoWrap);

    // Text için biraz padding bırak
    RectF textLayout = barRect;
    textLayout.Inflate(-5.0f, 0.0f);

    g.DrawString(infoText, -1, &infoFont, textLayout, &format, &textBrush);

    // 7. HOVER ÇERÇEVESİ (Opsiyonel ama şık)
    // Kartın tüm etrafına hafif bir parlama ekleyelim (Clip'i kaldırıp çiziyoruz)
    g.ResetClip();
    Pen glowPen(Color(120, 100, 150, 255), 2.0f); // Hafif mavi parlama
    g.DrawPath(&glowPen, &path);
}

// CListCustomerView.h dosyasına ekle:
// int m_hoverItem = -1; // Sınıf üyesi olarak tanımlı olmalı

// CListCustomerView.cpp -> OnMouseMove
LRESULT CListCustomerView::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // Mouse koordinatını al
    CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

    // Hangi item üzerinde?
    LVHITTESTINFO lvhti = { 0 };
    lvhti.pt = pt;
    ListView_HitTest(GetHwnd(), &lvhti);
    int newItem = lvhti.iItem;

    // Eğer hover değiştiyse (Yeni bir karta geçildiyse)
    if (newItem != m_hoverItem)
    {
        // 1. Eski kartın hover efektini sil (Redraw)
        if (m_hoverItem != -1) {
            CRect rcOld;
            ListView_GetItemRect(GetHwnd(), m_hoverItem, &rcOld, LVIR_BOUNDS);
            InvalidateRect(rcOld, FALSE);
        }

        // 2. Yeni durumu kaydet
        m_hoverItem = newItem;

        // 3. Yeni kartı çiz (Hover efekti gelecek)
        if (m_hoverItem != -1) {
            CRect rcNew;
            ListView_GetItemRect(GetHwnd(), m_hoverItem, &rcNew, LVIR_BOUNDS);
            InvalidateRect(rcNew, FALSE);

            // Mouse pencereden çıkarsa haberdar olmak için takip başlat
            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof(TRACKMOUSEEVENT);
            tme.dwFlags = TME_LEAVE;
            tme.hwndTrack = GetHwnd();
            TrackMouseEvent(&tme);
        }
    }

    return FinalWindowProc(uMsg, wParam, lParam);
}

// CListCustomerView.cpp -> OnMouseLeave
LRESULT CListCustomerView::OnMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // Mouse pencereden çıktı, hover'ı temizle
    if (m_hoverItem != -1) {
        CRect rc;
        ListView_GetItemRect(GetHwnd(), m_hoverItem, &rc, LVIR_BOUNDS);
        InvalidateRect(rc, FALSE); // Hover'ı silmek için boya
        m_hoverItem = -1;
    }
    return FinalWindowProc(uMsg, wParam, lParam);
}



// Files.cpp
//void CListCustomerView::RefreshCustomerList()
//{
//    SetRedraw(FALSE);
//    DeleteAllItems();
//
//    // Veritabanından tüm listeyi çek (Hız için tek seferde)
//    std::vector<Customer_cstr> allData = db_Manager.GetCustomers();
//
//    for (const auto& c : allData)
//    {
//        // --- 1. HARF KONTROLÜ ---
//        bool bLetterMatch = false;
//        if (m_currentLetter == _T("HEPSI") || m_currentLetter.IsEmpty())
//            bLetterMatch = true;
//        else {
//            CString firstChar = c.AdSoyad.Left(1);
//            if (CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, firstChar, 1, m_currentLetter, 1) == CSTR_EQUAL)
//                bLetterMatch = true;
//        }
//
//        // --- 2. DURUM (STATUS) KONTROLÜ ---
//        bool bStatusMatch = false;
//        int customerStatus = _ttoi(c.Calisma_Durumu);
//        if (m_currentStatus == 0 || customerStatus == m_currentStatus)
//            bStatusMatch = true;
//
//        // --- SONUÇ: İKİSİ DE DOĞRUYSA LİSTEYE EKLE ---
//        if (bLetterMatch && bStatusMatch)
//        {
//            AddItemFromStruct(c);
//        }
//    }
//
//    SetRedraw(TRUE);
//    Invalidate();
//}





void CListCustomerView::OnSetFocus()
{
    // base çağırıyorsan çağır
    // CWnd::OnSetFocus();

    CRibbonFrameApp* pApp = GetContainerApp();
    if (pApp && pApp->GetMainFrame().IsWindow())
        ::PostMessage(pApp->GetMainFrame().GetHwnd(), UWM_CONTEXT_CHANGED, 1, 0);
}