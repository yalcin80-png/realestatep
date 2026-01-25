// wxx_treelistview.h
// Yeni Nesil TreeListView Çekirdeği (v2.1 - Full API Support, Model Tamamlandı)

#ifndef _WIN32XX_TREELISTVIEW_H_
#define _WIN32XX_TREELISTVIEW_H_

#include "stdafx.h"
#include <vector>
#include <map>
#include <memory>
#include <string>
#include <algorithm>
#include <commctrl.h>

#pragma comment(lib,"Comctl32.lib")
#pragma comment(lib,"Msimg32.lib")

namespace Win32xx
{




    // =====================================================
//  TreeListView – Icon Index Definitions
//  (TreeView ImageList sıralamasına göre)
// =====================================================

// 🌐 Genel kategori/header ikonu
    constexpr int TLV_ICON_CATEGORY = 0;

    // 🏠 Konut kategorisi (HomeTbl)
    constexpr int TLV_ICON_HOME = 1;

    // 🏞️ Arsa kategorisi (LandTbl)
    constexpr int TLV_ICON_LAND = 2;

    // 🌾 Tarla kategorisi (FieldTbl)
    constexpr int TLV_ICON_FIELD = 3;

    // 🍇 Bağ kategorisi (VineyardTbl)
    constexpr int TLV_ICON_VINEYARD = 4;

    // 🏛️ Villa kategorisi (VillaTbl)
    constexpr int TLV_ICON_VILLA = 5;

    // 🏢 Ticari (CommercialTbl)
    constexpr int TLV_ICON_COMMERCIAL = 6;

    // 📌 Genel ikon (para, m2, vs. özel kolon ikonları için)
    constexpr int TLV_ICON_COLUMN_GENERIC = 7;

















    using tstring = std::basic_string<TCHAR>;
    struct CStringLess { bool operator()(const CString& a, const CString& b) const { return a.Compare(b) < 0; } };
    struct CStringLessNoCase { bool operator()(const CString& a, const CString& b) const { return a.CompareNoCase(b) < 0; } };



    //===========================================================
    // 1) Edit handler interface
    //===========================================================
    class ITreeListEditHandler
    {
    public:
        virtual bool OnCellBeginEdit(HTREEITEM hItem, int subIndex, CString& inOutText) = 0;
        virtual bool OnCellEndEdit(HTREEITEM hItem, int subIndex, const CString& newText) = 0;
        virtual ~ITreeListEditHandler() {}
    };

    //===========================================================
    // 2) Edit control types
    //===========================================================
    enum EditControlType {
        Edit_None = 0,
        Edit_TextBox = 1,
        Edit_ComboBox = 2,
        Edit_DatePicker = 3
    };

    constexpr UINT TLV_MSG_INLINE_COMMIT = WM_USER + 201;
    constexpr UINT TLV_MSG_INLINE_CANCEL = WM_USER + 202;

    //===========================================================
    // 3) Inline Edit Controls (Helpers)
    //===========================================================
    class CTlvControlBase
    {
    public:
        void SetNotifyParent(HWND hParent) { m_hParent = hParent; }
    protected:
        HWND m_hParent{ nullptr };
        void SendCommit() { if (m_hParent) ::PostMessage(m_hParent, TLV_MSG_INLINE_COMMIT, 0, 0); }
        void SendCancel() { if (m_hParent) ::PostMessage(m_hParent, TLV_MSG_INLINE_CANCEL, 0, 0); }
    };

    class CTlvEdit : public CEdit, public CTlvControlBase
    {
    protected:
        virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam) override {
            switch (msg) {
            case WM_KEYDOWN:
                if (wParam == VK_RETURN) { SendCommit(); return 0; }
                if (wParam == VK_ESCAPE) { SendCancel(); return 0; }
                break;
            case WM_KILLFOCUS:
                SendCommit();
                break;
            }
            return CEdit::WndProc(msg, wParam, lParam);
        }
    };

    class CTlvCombo : public CComboBox, public CTlvControlBase
    {
    protected:
        virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam) override {
            switch (msg) {
            case WM_KEYDOWN:
                if (wParam == VK_RETURN) { SendCommit(); return 0; }
                if (wParam == VK_ESCAPE) { SendCancel(); return 0; }
                break;
            case WM_KILLFOCUS:
                SendCommit();
                break;
            }
            return CComboBox::WndProc(msg, wParam, lParam);
        }
    };

    class CTlvDatePicker :public Win32xx::CDateTime, public CTlvControlBase
    {
    public:
        CTlvDatePicker() {}
    protected:
        virtual void PreCreate(CREATESTRUCT& cs) override {
            cs.lpszClass = DATETIMEPICK_CLASS;
            cs.style |= WS_CHILD | WS_VISIBLE | DTS_SHORTDATEFORMAT;
            CDateTime::PreCreate(cs);
        }



        virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam) override {
            switch (msg) {
            case WM_KEYDOWN:
                if (wParam == VK_RETURN) { SendCommit(); return 0; }
                if (wParam == VK_ESCAPE) { SendCancel(); return 0; }
                break;
            case WM_KILLFOCUS:
                SendCommit();
                break;
            }
            return CWnd::WndProc(msg, wParam, lParam);
        }
    };

    //===========================================================
    // 4) Tema ve Veri Modeli
    //===========================================================
    struct ThemeConfig {
        bool     dark{ false };

        COLORREF clrWnd{ RGB(255, 255, 255) };
        COLORREF clrText{ RGB(0, 0, 0) };
        COLORREF clrSelBk{ RGB(51, 153, 255) };
        COLORREF clrSelText{ RGB(255, 255, 255) };
        COLORREF clrHdrBk{ RGB(236, 236, 236) };
        // YENİ: Kategori Başlığı Rengi
        COLORREF clrCatText{ RGB(0, 102, 204) }; // Varsayılan: Koyu Mavi (Office Blue)
        //COLORREF clrHdrText{ RGB(0, 0, 0) };
        //COLORREF clrGrid{ RGB(200, 200, 200) };
        // 3D Header Renkleri
        COLORREF clrHdrGradTop{ RGB(245, 245, 245) };    // Üst kısım (Açık)
        COLORREF clrHdrGradBot{ RGB(225, 225, 225) };    // Alt kısım (Koyu)
        COLORREF clrHdrText{ RGB(50, 50, 50) };          // Koyu Gri (Daha profesyonel)
        COLORREF clrGrid{ RGB(210, 210, 210) };
        // 🔹 Yükseklikler (px)
        int      categoryHeaderHeight{ 24 };  // Kategori satırı (fake header)
        int      headerHeight{ 24 };          // Kolon başlığı
        int      subHeaderHeight{ 20 };       // Subheader satırı
        int      rowHeight{ 24 };             // Veri satırı

        void ApplyDark() {
            dark = true;
            clrWnd = RGB(32, 32, 36);
            clrText = RGB(230, 230, 230);
            clrSelBk = RGB(45, 100, 200);
            clrSelText = RGB(255, 255, 255);
            clrHdrBk = RGB(45, 45, 50);
            clrHdrText = RGB(230, 230, 230);
            clrGrid = RGB(70, 70, 75);
            clrCatText = RGB(100, 180, 255); // Dark mode için açık mavi
            categoryHeaderHeight = 26;
            headerHeight = 24;
            subHeaderHeight = 20;
            rowHeight = 24;
        }

        void ApplyLight() {
            dark = false;
            clrWnd = RGB(255, 255, 255);
            clrText = RGB(0, 0, 0);
            clrSelBk = RGB(51, 153, 255);
            clrSelText = RGB(255, 255, 255);
            clrHdrBk = RGB(236, 236, 236);
            //clrHdrText = RGB(0, 0, 0);
            //clrGrid = RGB(200, 200, 200);
            // Light Mode 3D (Office Style)
            clrHdrGradTop = RGB(252, 252, 252);
            clrHdrGradBot = RGB(230, 230, 232);
            clrHdrText = RGB(30, 30, 30);
            clrGrid = RGB(200, 200, 200);
            clrCatText = RGB(0, 102, 204); // Light mode için koyu mavi
            categoryHeaderHeight = 24;
            headerHeight = 24;
            subHeaderHeight = 20;
            rowHeight = 24;
        }
    };

    struct TLV_ItemStyle {
        COLORREF text{ CLR_NONE };
        COLORREF back{ CLR_NONE };
    };

    // --------------------------------------------------------
    // SubHeader Modeli
    // Ör: Fiyat kolonu altında ("TL", "USD") gibi ikili satır
    // --------------------------------------------------------
    struct TLV_SubHeaderInfo {
        CString      text;   // Subheader başlığı
        TLV_ItemStyle style; // Metin / arka plan rengi
    };

    struct PropertyColumnInfo {
        // 🔹 Mantıksal kolon kimliği (DB alan adı, JSON key, vs.)
        CString  key;               // Örn: "Price", "Home_Code"
        // 🔹 Kullanıcıya görünen başlık
        CString  title;             // Örn: "Fiyat", "Ev Kodu"

        int      width{ 120 };
        int      align{ DT_LEFT };
        bool     visible{ true };

        EditControlType editType{ Edit_None };
        CString  comboSource;       // "Kira;Satılık;Devren" gibi

        TLV_ItemStyle  style;       // Hücre default stili
        TLV_ItemStyle  headerStyle; // Kolon başlığı stili

        // 🔹 Çok satırlı header için subheader listesi
        std::vector<TLV_SubHeaderInfo> subHeaders;

        PropertyColumnInfo() = default;

        PropertyColumnInfo(LPCTSTR inKey,
            LPCTSTR inTitle,
            int inWidth = 120,
            int inAlign = DT_LEFT,
            bool inVisible = true,
            EditControlType inEdit = Edit_None)
            : key(inKey)
            , title(inTitle)
            , width(inWidth)
            , align(inAlign)
            , visible(inVisible)
            , editType(inEdit)
        {
        }
    };

    // Aksiyon Butonu Tanımı
    struct TLV_ActionButton {
        int     id;             // Butonu ayırt etmek için ID (Örn: 101, 102)
        CString icon;           // Gösterilecek İkon/Metin (Örn: L"✎", L"🖨")
        COLORREF colorNormal;   // Normal arka plan rengi
        COLORREF colorHover;    // Üzerine gelinceki renk
        CString tooltip;        // (Opsiyonel) İpucu metni

        // Constructor
        TLV_ActionButton(int _id, CString _ico, COLORREF _norm, COLORREF _hov)
            : id(_id), icon(_ico), colorNormal(_norm), colorHover(_hov) {
        }
    };

    struct TLV_CellInfo {
        CString      text;
        TLV_ItemStyle colors;
    };

    struct TLV_Row {
        HTREEITEM      hItem{ nullptr };
        CString        key;      // Satır kimliği (mülk kodu / kayıt ID vs.)
        LPARAM         param{ 0 }; // Kullanıcı datası
        TLV_ItemStyle  style;      // Satır geneli renk
        std::vector<TLV_CellInfo> cells;
    };

    struct TLV_Category {
        // 🔹 Mantıksal ad (tablo adı / kategori id)
        CString        name;          // Örn: "HomeTbl"
        // 🔹 Kullanıcıya görünen başlık
        CString        displayName;   // Örn: "Konut Portföyleri"
        // 🔹 Header için ikon (image list index)
        int            iconIndex{ -1 };

        HTREEITEM      hHeader{ nullptr };
        // 🎨 YENİ: Kategoriye özel renk (Varsayılan CLR_NONE)
        COLORREF       headerColor{ CLR_NONE };
        TLV_ItemStyle  style;         // Kategori satırının stil bilgisi

        // Kolonlar ve satırlar
        std::vector<PropertyColumnInfo>           columns;
        std::vector<std::unique_ptr<TLV_Row>>     rows;

        // Çok satırlı header yüksekliği için yardımcı
        int GetTotalSubHeaderDepth() const
        {
            int depth = 0;
            for (const auto& col : columns)
            {
                int sz = static_cast<int>(col.subHeaders.size());
                if (sz > depth)
                    depth = sz;
            }
            return depth;
        }
    };

    //===========================================================
    // 5) CTreeListView (ANA SINIF)
    //===========================================================
    class CTreeListView : public CTreeView
    {
    private:
        EditControlType m_activeEditType{ Edit_None };
        HTREEITEM       m_activeEditItem{ nullptr };
        int             m_activeEditCol{ -1 };

        CTlvEdit        m_editCtrl;
        CTlvCombo       m_comboCtrl;
        CTlvDatePicker  m_dateCtrl;

        ThemeConfig     m_theme;
        ITreeListEditHandler* m_editHandler{ nullptr };
        CImageList* m_pDragImage{ nullptr };
        // --- Column Drag & Drop ---
        bool      m_isDraggingColumn{ false };
        HTREEITEM m_dragHeaderItem{ nullptr }; // Sürüklenen kolonun ait olduğu kategori header'ı
        int       m_dragColumnIndex{ -1 };     // Sürüklenen kolonun indeksi
        HCURSOR   m_hCursorDrag{ nullptr };    // Sürükleme imleci

        // ...
        int       m_minColumnWidth{ 30 };    // İstersen 50 yap
        HCURSOR   m_hCursorSizeWE{ nullptr };

        // ----------------------------------------------------------
        // 🔹 Modern UI: Hover/Hot row durumu
        HTREEITEM m_hotItem{ nullptr };
        bool      m_isTrackingMouse{ false };


    public:

        // --- YENİ AKSİYON YÖNETİMİ ---
        std::vector<TLV_ActionButton> m_actionButtons; // Dinamik Buton Listesi
        int m_hoverButtonID = -1;       // Şu an üzerinde olunan butonun ID'si (-1: Yok)
        HTREEITEM m_hoverItem = nullptr; // Mouse hangi satırda?

        // Buton boyutları (Ayarlanabilir)
        int m_btnWidth = 28;
        int m_btnHeight = 22;
        int m_btnMargin = 4;

        // ✅ 1. DIŞARIDAN BUTON EKLEME FONKSİYONU (Çalışma zamanında çağrılabilir)
        void AddActionButton(int id, LPCTSTR iconText, COLORREF clrNormal, COLORREF clrHover) {
            m_actionButtons.emplace_back(id, iconText, clrNormal, clrHover);
        }

        // ✅ 2. BUTONLARI TEMİZLEME
        void ClearActionButtons() {
            m_actionButtons.clear();
        }

        // ✅ 3. SANAL OLAY YAKALAYICI (Türetilmiş sınıfta override edilecek)
        virtual void OnActionButtonClick(int btnID, HTREEITEM hItem) {
            // Varsayılan boş, türetilmiş sınıf dolduracak
        }

        CRect CalculateButtonRect(CRect rcRow, int buttonIndex) {
            CRect rcClient;
            ::GetClientRect(GetHwnd(), &rcClient);

            int rightEdge = rcClient.right - 5; // Sağdan 5px boşluk
            int totalOffset = (buttonIndex + 1) * (m_btnWidth + m_btnMargin);

            int left = rightEdge - totalOffset + m_btnMargin;
            int top = rcRow.top + (rcRow.Height() - m_btnHeight) / 2;

            return CRect(left, top, left + m_btnWidth, top + m_btnHeight);
        }


        struct TLV_ActionBar {
            CRect rcEdit;
            CRect rcPrint;
            HTREEITEM hActiveItem = nullptr;
            int hotButton = 0; // 0: None, 1: Edit, 2: Print
        } m_actionBar;





        std::vector<std::unique_ptr<TLV_Category>> m_categories;
        std::map<HTREEITEM, TLV_Category*>         m_catByHeader;
        std::map<HTREEITEM, TLV_Row*>              m_rowByHandle;
        // Ayarlar (Dilersen bunları Initialize içinde değiştirebilirsin)
        int m_actionBtnWidth = 26;
        bool m_showActionsOnHover = true;

        // ==========================================================
        // --- Kolon yeniden boyutlandırma durumu ---
        bool      m_isResizingColumn{ false };
        HTREEITEM m_resizeHeaderItem{ nullptr };
        int       m_resizeColumnIndex{ -1 };
        int       m_resizeStartX{ 0 };
        int       m_resizeStartWidth{ 0 };
       // int       m_minColumnWidth{ 30 };    // İstersen 50 yap
        //HCURSOR   m_hCursorSizeWE{ nullptr };

        // ----------------------------------------------------------





    public:
        CTreeListView() { m_theme.ApplyLight(); }
        virtual ~CTreeListView()
        {
            // Eğer sürükleme sırasında program kapanırsa temizle
            if (m_pDragImage) {
                delete m_pDragImage;
                m_pDragImage = nullptr;
            }
        }
        //using CTreeView::InsertItem;


        enum class HitPart { None, HeaderDivider, HeaderItem, RowCell, Background };

        struct HitTestResult {
            HitPart   part{ HitPart::None };
            HTREEITEM hItem{ nullptr };
            int       subIndex{ -1 };
        };
        HitTestResult HitTestEx(CPoint pt) const
        {
            HitTestResult res;

            // 1. Header Resize Çizgisi mi?
            HTREEITEM hResizeHeader = nullptr;
            int resizeCol = -1;
            if (const_cast<CTreeListView*>(this)->HitTestHeaderResize(pt, hResizeHeader, resizeCol)) // const_cast gerekebilir
            {
                res.part = HitPart::HeaderDivider;
                res.hItem = hResizeHeader;
                res.subIndex = resizeCol;
                return res;
            }

            // 2. Normal TreeView HitTest
            TVHITTESTINFO ht = { 0 };
            ht.pt = pt;
            HTREEITEM hHit = TreeView_HitTest(GetHwnd(), &ht);

            if (!hHit) {
                res.part = HitPart::Background;
                return res;
            }

            // 3. Header mı Veri mi?
            if (IsHeaderItem(hHit)) {
                res.part = HitPart::HeaderItem;
                res.hItem = hHit;
                // Kolon indeksini bul
                res.subIndex = const_cast<CTreeListView*>(this)->HitTestColumn(hHit, pt);
            }
            else {
                res.part = HitPart::RowCell;
                res.hItem = hHit;
                // Kolon indeksini bul
                res.subIndex = const_cast<CTreeListView*>(this)->HitTestColumn(hHit, pt);
            }

            return res;
        }

        void SetDarkTheme(bool on) { on ? m_theme.ApplyDark() : m_theme.ApplyLight(); Invalidate(); }
        void SetEditHandler(ITreeListEditHandler* h) { m_editHandler = h; }

        const ThemeConfig& GetTheme() const { return m_theme; }
        ThemeConfig& GetTheme() { return m_theme; }
        // --- FULL INSERTITEM OVERLOAD SET ---
        bool TryParseDate(const CString& txt, SYSTEMTIME& out)
        {
            CString s = txt;
            s.Trim();

            if (s.IsEmpty())
                return false;

            // Normalize delimiters
            s.Replace('/', '.');
            s.Replace('-', '.');

            int p1 = s.Find('.');
            int p2 = s.Find('.', p1 + 1);
            if (p1 < 0 || p2 < 0) return false;

            int d = _ttoi(s.Left(p1));
            int m = _ttoi(s.Mid(p1 + 1, p2 - p1 - 1));
            int y = _ttoi(s.Mid(p2 + 1));

            if (d <= 0 || m <= 0 || y <= 0) return false;

            SYSTEMTIME st{};
            st.wDay = d; st.wMonth = m; st.wYear = y;
            out = st;
            return true;
        }
        HTREEITEM InsertDataRow(HTREEITEM hParentCategory, const CString& key, const CString& firstColText)
        {
            // 1. Kategori Kontrolü
            TLV_Category* cat = FindCategoryByHeader(hParentCategory);
            if (!cat) return nullptr;

            // 2. Windows TreeView'e Ekle
            TVINSERTSTRUCT tvi{};
            tvi.hParent = hParentCategory;
            tvi.hInsertAfter = TVI_LAST;
            tvi.item.mask = TVIF_TEXT | TVIF_PARAM;
            tvi.item.pszText = const_cast<LPTSTR>((LPCTSTR)firstColText);
            tvi.item.lParam = 0; // İleride user data için kullanılabilir

            HTREEITEM hItem = (HTREEITEM)::SendMessage(GetHwnd(), TVM_INSERTITEM, 0, (LPARAM)&tvi);
            if (!hItem) return nullptr;

            // 3. MODEL KAYDI (İşte eksik olan parça buydu!)
            auto row = std::make_unique<TLV_Row>();
            row->hItem = hItem;
            row->key = key; // Primary Key (Home_Code vb.)

            // Hücreleri hazırla (Sütun sayısı kadar boş hücre oluştur)
            row->cells.resize(cat->columns.size());

            // İlk sütun verisini set et
            if (!row->cells.empty())
                row->cells[0].text = firstColText;

            TLV_Row* rawRow = row.get();

            // Modele ekle
            cat->rows.emplace_back(std::move(row));

            // Map'e ekle (DrawDataRow bunu kullanacak)
            m_rowByHandle[hItem] = rawRow;

            return hItem;
        }
        // 1) Basit string + icon
        HTREEITEM InsertItem(LPCTSTR text,
            int image,
            HTREEITEM parent = TVI_ROOT,
            HTREEITEM after = TVI_LAST)
        {
            return CTreeView::InsertItem(text, image, image, parent, after);
        }

        // 2) Sadece text
        HTREEITEM InsertItem(LPCTSTR text,
            HTREEITEM parent,
            HTREEITEM after = TVI_LAST)
        {
            return CTreeView::InsertItem(text, 0, 0, parent, after);
        }

        // 3) Sadece text (varsayılan parent/after)
        HTREEITEM InsertItem(LPCTSTR text)
        {
            return CTreeView::InsertItem(text, 0, 0, TVI_ROOT, TVI_LAST);
        }

        // 4) WIN32 TVINSERTSTRUCT ile
        HTREEITEM InsertItem(const TVINSERTSTRUCT& tv)
        {
            return (HTREEITEM)::SendMessage(GetHwnd(), TVM_INSERTITEM, 0, (LPARAM)&tv);
        }

        // 5) Çok kolonlu modeller için özel overload
        HTREEITEM InsertItem(const std::vector<tstring>& columns,
            int image,
            HTREEITEM parent = TVI_ROOT,
            HTREEITEM after = TVI_LAST)
        {
            CString text = columns.empty() ? _T("") : columns[0].c_str();
            HTREEITEM h = InsertItem(text, image, parent, after);

            // Model ekleme işlemleri
            // NOT: Gerçek satır oluşturma CMyTreeListView içinde yapılacak.
            return h;
        }

        // 6) CString versiyonu
        HTREEITEM InsertItem(const CString& text,
            int image,
            HTREEITEM parent = TVI_ROOT,
            HTREEITEM after = TVI_LAST)
        {
            return InsertItem(text, image, parent, after);
        }

        // ----------------------------------------------------
        //  MODEL OLUŞTURMA / KATEGORİ, KOLON, SATIR
        // ----------------------------------------------------
        TLV_Category* AddCategory(const CString& categoryName, HTREEITEM hHeaderItem)
        {
            auto cat = std::make_unique<TLV_Category>();
            cat->name = categoryName;
            cat->displayName = categoryName;  // varsayılan: aynı
            cat->hHeader = hHeaderItem;

            TLV_Category* raw = cat.get();
            m_categories.emplace_back(std::move(cat));
            m_catByHeader[hHeaderItem] = raw;
            return raw;
        }

        // overload: display name + icon
        TLV_Category* AddCategory(const CString& categoryName,
            const CString& displayName,
            HTREEITEM hHeaderItem,
            int iconIndex = -1)
        {
            auto cat = std::make_unique<TLV_Category>();
            cat->name = categoryName;
            cat->displayName = displayName;
            cat->hHeader = hHeaderItem;
            cat->iconIndex = iconIndex;

            TLV_Category* raw = cat.get();
            m_categories.emplace_back(std::move(cat));
            m_catByHeader[hHeaderItem] = raw;
            return raw;
        }

        // Basit kolon ekleme (sadece başlık ve genişlik)
        PropertyColumnInfo* AddColumn(TLV_Category* cat,
            const CString& title,
            int width,
            EditControlType editType = Edit_None)
        {
            if (!cat) return nullptr;
            PropertyColumnInfo col;
            col.key = title;     // varsayılan: key = title
            col.title = title;
            col.width = width;
            col.editType = editType;
            cat->columns.push_back(col);
            return &cat->columns.back();
        }

        // Tam tanımlı kolon ekleme (key + title)
        PropertyColumnInfo* AddColumn(TLV_Category* cat,
            const CString& key,
            const CString& title,
            int width,
            int align = DT_LEFT,
            bool visible = true,
            EditControlType editType = Edit_None)
        {
            if (!cat) return nullptr;
            PropertyColumnInfo col(key, title, width, align, visible, editType);
            cat->columns.push_back(col);
            return &cat->columns.back();
        }

        // Subheader ekleme (A adımının bir parçası: modeli tamamlamak için)
        bool AddSubHeader(TLV_Category* cat,
            int columnIndex,
            const CString& text,
            COLORREF textColor = CLR_NONE,
            COLORREF backColor = CLR_NONE)
        {
            if (!cat) return false;
            if (columnIndex < 0 || columnIndex >= (int)cat->columns.size())
                return false;

            TLV_SubHeaderInfo sh;
            sh.text = text;
            sh.style.text = textColor;
            sh.style.back = backColor;

            cat->columns[columnIndex].subHeaders.push_back(sh);
            return true;
        }

        TLV_Row* AddRow(HTREEITEM hHeader, HTREEITEM hItem, const CString& key)
        {
            TLV_Category* cat = FindCategoryByHeader(hHeader);
            if (!cat) return nullptr;

            auto row = std::make_unique<TLV_Row>();
            row->hItem = hItem;
            row->key = key;

            row->cells.resize(cat->columns.size());
            if (!row->cells.empty())
                row->cells[0].text = GetItemText(hItem);

            TLV_Row* raw = row.get();
            cat->rows.emplace_back(std::move(row));
            m_rowByHandle[hItem] = raw;
            return raw;
        }

        // ==========================================================
        //  GENİŞLETİLMİŞ API (GET/SET FONKSİYONLARI) - MODEL
        // ==========================================================

        // 0. KATEGORİ LİSTESİ (MODEL İNCELEME)
        size_t GetCategoryCount() const
        {
            return m_categories.size();
        }

        CString GetCategoryNameByIndex(size_t index) const
        {
            if (index >= m_categories.size())
                return CString();
            return m_categories[index]->name;
        }

        CString GetCategoryDisplayNameByIndex(size_t index) const
        {
            if (index >= m_categories.size())
                return CString();
            return m_categories[index]->displayName;
        }

        TLV_Category* GetCategoryByIndex(size_t index)
        {
            if (index >= m_categories.size())
                return nullptr;
            return m_categories[index].get();
        }

        const TLV_Category* GetCategoryByIndexConst(size_t index) const
        {
            if (index >= m_categories.size())
                return nullptr;
            return m_categories[index].get();
        }

        TLV_Category* FindCategoryByName(const CString& name)
        {
            for (auto& up : m_categories)
            {
                if (up->name.CompareNoCase(name) == 0)
                    return up.get();
            }
            return nullptr;
        }

        const TLV_Category* FindCategoryByNameConst(const CString& name) const
        {
            for (auto& up : m_categories)
            {
                if (up->name.CompareNoCase(name) == 0)
                    return up.get();
            }
            return nullptr;
        }

        // 1. ITEM DATA (LPARAM)
        void SetItemData(HTREEITEM hItem, LPARAM data)
        {
            TLV_Row* row = GetRow(hItem);
            if (row) row->param = data;
        }

        LPARAM GetItemData(HTREEITEM hItem)
        {
            TLV_Row* row = GetRow(hItem);
            return row ? row->param : 0;
        }

        // 2. SUB ITEM TEXT (Hücre Metni)
        void SetSubItemText(HTREEITEM hItem, int col, const CString& text)
        {
            TLV_Row* row = GetRow(hItem);
            if (!row) return;

            if (col >= 0 && col < (int)row->cells.size())
            {
                row->cells[col].text = text;
                // Eğer ilk kolonsa TreeView metnini de güncelle (senkronizasyon)
                if (col == 0) SetItemText(hItem, text);
                Invalidate();
            }
        }

        CString GetSubItemText(HTREEITEM hItem, int col)
        {
            TLV_Row* row = GetRow(hItem);
            if (!row) return _T("");
            if (col >= 0 && col < (int)row->cells.size())
                return row->cells[col].text;
            return _T("");
        }

        // 3. SUB ITEM COLOR (Hücre Rengi)
        void SetSubItemColor(HTREEITEM hItem, int col, COLORREF text, COLORREF back)
        {
            TLV_Row* row = GetRow(hItem);
            if (!row) return;
            if (col >= 0 && col < (int)row->cells.size())
            {
                row->cells[col].colors.text = text;
                row->cells[col].colors.back = back;
                Invalidate();
            }
        }

        bool GetSubItemColor(HTREEITEM hItem, int col, COLORREF& outText, COLORREF& outBack)
        {
            TLV_Row* row = GetRow(hItem);
            if (!row) return false;
            if (col >= 0 && col < (int)row->cells.size())
            {
                outText = row->cells[col].colors.text;
                outBack = row->cells[col].colors.back;
                return true;
            }
            return false;
        }

        // 4. ROW COLOR (Tüm Satır Rengi)
        void SetRowColor(HTREEITEM hItem, COLORREF text, COLORREF back)
        {
            TLV_Row* row = GetRow(hItem);
            if (!row) return;
            row->style.text = text;
            row->style.back = back;
            Invalidate();
        }

        bool GetRowColor(HTREEITEM hItem, COLORREF& outText, COLORREF& outBack)
        {
            TLV_Row* row = GetRow(hItem);
            if (!row) return false;
            outText = row->style.text;
            outBack = row->style.back;
            return true;
        }

        // 5. COLUMN MANAGEMENT (Kolon Ayarları)
        void SetColumnWidth(HTREEITEM hItemOrHeader, int colIndex, int width)
        {
            TLV_Category* cat = IsHeaderItem(hItemOrHeader)
                ? FindCategoryByHeader(hItemOrHeader)
                : GetCategoryForRow(hItemOrHeader);
            if (!cat) return;

            if (colIndex >= 0 && colIndex < (int)cat->columns.size())
            {
                cat->columns[colIndex].width = width;
                Invalidate();
            }
        }

        int GetColumnWidth(HTREEITEM hItemOrHeader, int colIndex)
        {
            TLV_Category* cat = IsHeaderItem(hItemOrHeader)
                ? FindCategoryByHeader(hItemOrHeader)
                : GetCategoryForRow(hItemOrHeader);
            if (!cat) return 0;
            if (colIndex >= 0 && colIndex < (int)cat->columns.size())
                return cat->columns[colIndex].width;
            return 0;
        }

        void SetColumnTitle(HTREEITEM hHeader, int colIndex, const CString& title)
        {
            TLV_Category* cat = FindCategoryByHeader(hHeader);
            if (!cat) return;
            if (colIndex >= 0 && colIndex < (int)cat->columns.size())
            {
                cat->columns[colIndex].title = title;
                Invalidate();
            }
        }

        void SetColumnVisible(HTREEITEM hHeader, int colIndex, bool visible)
        {
            TLV_Category* cat = FindCategoryByHeader(hHeader);
            if (!cat) return;
            if (colIndex >= 0 && colIndex < (int)cat->columns.size())
            {
                cat->columns[colIndex].visible = visible;
                Invalidate();
            }
        }

        // ==========================================================
        //  CORE HELPERS
        // ==========================================================
        TLV_Row* GetRow(HTREEITEM hItem) {
            auto it = m_rowByHandle.find(hItem);
            return (it != m_rowByHandle.end()) ? it->second : nullptr;
        }

        TLV_Category* FindCategoryByHeader(HTREEITEM hHeader) {
            auto it = m_catByHeader.find(hHeader);
            return (it != m_catByHeader.end()) ? it->second : nullptr;
        }
        // YENİ HIZLI KOD
// --- DÜZELTİLMİŞ KATEGORİ BULUCU (HIZLI) ---
        TLV_Category* GetCategoryForRow(HTREEITEM hItem)
        {
            // 1. Döngü yerine Parent (Üst Öğe) kontrolü yapıyoruz
            HTREEITEM hParent = GetParentItem(hItem);

            // 2. Eğer parent yoksa bu bir Root öğedir (yani Header olabilir)
            if (!hParent) return nullptr;

            // 3. Parent'ı (Header'ı) map içinde buluyoruz (O(1) hızında)
            return FindCategoryByHeader(hParent);
        }
        //TLV_Category* GetCategoryForRow(HTREEITEM hItem) {
        //    for (auto& c : m_categories) {
        //        for (auto& r : c->rows) {
        //            if (r->hItem == hItem) return c.get();
        //        }
        //    }
        //    return nullptr;
        //}

        bool IsHeaderItem(HTREEITEM hItem) const {
            return m_catByHeader.find(hItem) != m_catByHeader.end();
        }

        // ==========================================================
        //  INLINE EDITING LOGIC
        // ==========================================================
        void StartInlineEdit(HTREEITEM hItem, int col)
        {
            if (!m_editHandler) return;
            TLV_Category* cat = GetCategoryForRow(hItem);
            if (!cat || col < 0 || col >= (int)cat->columns.size()) return;

            PropertyColumnInfo& ci = cat->columns[col];
            if (!ci.visible || ci.editType == Edit_None) return;

            CString text = GetSubItemText(hItem, col);
            if (!m_editHandler->OnCellBeginEdit(hItem, col, text)) return;

            CRect rcCell;
            if (!GetCellRect(hItem, col, rcCell)) return;
            rcCell.DeflateRect(1, 1);

            m_activeEditItem = hItem;
            m_activeEditCol = col;
            m_activeEditType = ci.editType;
            HWND hParent = GetHwnd();

            if (ci.editType == Edit_TextBox) {
                if (!m_editCtrl.IsWindow()) {
                    m_editCtrl.Create(hParent);
                    m_editCtrl.SetNotifyParent(hParent);
                }
                m_editCtrl.SetWindowText(text);
                m_editCtrl.SetFont(GetFont());
                m_editCtrl.SetWindowPos(nullptr, rcCell, SWP_SHOWWINDOW | SWP_NOZORDER);
                m_editCtrl.SetFocus();
            }
            else if (ci.editType == Edit_ComboBox) {
                if (!m_comboCtrl.IsWindow()) {
                    m_comboCtrl.Create();
                    m_comboCtrl.SetNotifyParent(hParent);
                }
                m_comboCtrl.ResetContent();
                int pos = 0; CString tok;
                while (!(tok = ci.comboSource.Tokenize(_T(";"), pos)).IsEmpty())
                    m_comboCtrl.AddString(tok);
                m_comboCtrl.SelectString(-1, text);
                m_comboCtrl.SetFont(GetFont());
                m_comboCtrl.SetWindowPos(nullptr,
                    rcCell.left, rcCell.top,
                    rcCell.Width(), rcCell.Height() + 100,
                    SWP_SHOWWINDOW | SWP_NOZORDER);
                m_comboCtrl.SetFocus();
            }
            else if (ci.editType == Edit_DatePicker)
            {
                if (!m_dateCtrl.IsWindow())
                {
                    m_dateCtrl.Create(hParent);
                    m_dateCtrl.SetNotifyParent(hParent);
                }

                // Metni yakalayıp SYSTEMTIME’a çevir
                SYSTEMTIME st{};
                bool ok = TryParseDate(text, st);

                if (ok)
                    m_dateCtrl.SetTime(st);
                else
                    m_dateCtrl.SetTimeNone();

                m_dateCtrl.SetFormat(_T("dd.MM.yyyy"));
                m_dateCtrl.SetFont(GetFont());
                m_dateCtrl.SetWindowPos(nullptr, rcCell, SWP_SHOWWINDOW | SWP_NOZORDER);
                m_dateCtrl.SetFocus();
            }
        }

        void CommitInlineEdit(bool apply)
        {
            if (m_activeEditType == Edit_None || !m_activeEditItem) return;
            CString newText;

            if (m_activeEditType == Edit_TextBox) {
                newText = m_editCtrl.GetWindowText();
            }
            else if (m_activeEditType == Edit_ComboBox) {
                int s = m_comboCtrl.GetCurSel();
                if (s >= 0) {
                    TCHAR buf[512] = { 0 };
                    m_comboCtrl.GetLBText(s, buf);
                    newText = buf;
                }
                else {
                    newText = m_comboCtrl.GetWindowText();
                }
            }
            else if (m_activeEditType == Edit_DatePicker)
            {
                DWORD ret = 0;
                SYSTEMTIME st = m_dateCtrl.GetTime(&ret);

                if (ret == GDT_VALID)
                {
                    TCHAR buf[32];
                    wsprintf(buf, _T("%02d.%02d.%04d"), st.wDay, st.wMonth, st.wYear);
                    newText = buf;
                }
                else {
                    newText = _T("");
                }
            }

            if (m_editCtrl.IsWindow())  m_editCtrl.ShowWindow(SW_HIDE);
            if (m_comboCtrl.IsWindow()) m_comboCtrl.ShowWindow(SW_HIDE);
            if (m_dateCtrl.IsWindow())  m_dateCtrl.ShowWindow(SW_HIDE);

            if (apply && m_editHandler) {
                if (m_editHandler->OnCellEndEdit(m_activeEditItem, m_activeEditCol, newText)) {
                    SetSubItemText(m_activeEditItem, m_activeEditCol, newText);
                }
            }

            m_activeEditType = Edit_None;
            m_activeEditItem = nullptr;
            m_activeEditCol = -1;
            SetFocus();
        }
        HTREEITEM GetSelectedItem() const
        {
            return CTreeView::GetSelection();
        }
        bool GetCellRect(HTREEITEM hItem, int col, CRect& out)
        {
            TLV_Category* cat = GetCategoryForRow(hItem);
            if (!cat || col < 0 || col >= (int)cat->columns.size())
                return false;

            CRect rcRow;
            if (!GetItemRect(hItem, rcRow, FALSE))
                return false;

            int x = rcRow.left;
            for (int i = 0; i < (int)cat->columns.size(); ++i) {
                if (!cat->columns[i].visible) continue;
                int w = cat->columns[i].width;
                if (i == col) {
                    out = CRect(x, rcRow.top, x + w, rcRow.bottom);
                    return true;
                }
                x += w;
            }
            return false;
        }



        void SetActionButtonSize(int width, int height, int margin = 4)
        {
            m_btnWidth = width;
            m_btnHeight = height;
            m_btnMargin = margin;
        }


        virtual void OnActionBarClick(int btnIdx, HTREEITEM hItem)  {};
        virtual void OnRowCellDoubleClick(HTREEITEM, int) {}
        virtual void OnHeaderDoubleClick(HTREEITEM, CPoint) {}
        // CMyTreeListView sınıfı içinde:
        // ==========================================================
        //  WINDOWS MESAJLARI & ÇİZİM
        // ==========================================================
        virtual void PreCreate(CREATESTRUCT& cs) override {
            cs.style |= TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS | TVS_FULLROWSELECT;

            // ❗ TreeView kendi drag-drop'unu devre dışı bırak (yoksa kolon taşıma çalışmaz)
            cs.style &= ~TVS_DISABLEDRAGDROP;

            CTreeView::PreCreate(cs);
        }
        //void SetActionButtonSize(int width, int height, int margin = 4)
        //{
        //    m_btnWidth = width;
        //    m_btnHeight = height;
        //    m_btnMargin = margin;
        //}
        virtual LRESULT OnNotifyReflect(WPARAM, LPARAM lParam) override {
            LPNMHDR p = reinterpret_cast<LPNMHDR>(lParam);
            if (p->code == NM_CUSTOMDRAW)
                return HandleCustomDraw(reinterpret_cast<LPNMTVCUSTOMDRAW>(p));
            return 0;
        }

        virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam) override
        {
            switch (msg)
            {
            case WM_ERASEBKGND:
                return FALSE; // Titremeyi önle
            case WM_NCHITTEST:
            {
                LRESULT ht = CTreeView::WndProc(msg, wParam, lParam);

                POINT ptScreen{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
                POINT ptClient = ptScreen;
                ::ScreenToClient(GetHwnd(), &ptClient);

                HTREEITEM h = HitTestTreeItem(ptClient);
                if (h && IsHeaderItem(h))
                    return HTCLIENT;  // Header tıklamasını TreeView’e kaptırma

                return ht;
            }



            case WM_LBUTTONDOWN:
            {
                CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

                // =========================================================================
                // 1. AKSİYON BUTONLARI HIT-TEST (EN YÜKSEK ÖNCELİK - GÜÇLENDİRİLMİŞ)
                // =========================================================================
                // Sadece m_hoverButtonID'ye güvenmek yerine, tıklama anında kesin hesap yapıyoruz.
                {
                    // A) Hangi satıra tıklandı?
                    TVHITTESTINFO htBtn = { 0 };
                    htBtn.pt = pt;
                    HTREEITEM hHitRow = TreeView_HitTest(GetHwnd(), &htBtn);

                    // Sağ boşluk düzeltmesi
                    if (!hHitRow) {
                        htBtn.pt.x = 5;
                        hHitRow = TreeView_HitTest(GetHwnd(), &htBtn);
                    }

                    // B) Eğer geçerli bir satırsa ve header değilse butonları kontrol et
                    if (hHitRow && !IsHeaderItem(hHitRow))
                    {
                        CRect rcRow;
                        if (GetItemRect(hHitRow, rcRow, FALSE))
                        {
                            // Buton listesini dön ve tıklandı mı bak
                            for (int i = 0; i < (int)m_actionButtons.size(); ++i)
                            {
                                // CalculateButtonRect fonksiyonunu kullanıyoruz (Sınıfa eklemiştik)
                                CRect rcBtn = CalculateButtonRect(rcRow, i);

                                if (rcBtn.PtInRect(pt))
                                {
                                    // 🔥 BINGO! Butona tıklandı.
                                    // Türetilmiş sınıfa (CMyTreeListView) haber ver.
                                    OnActionButtonClick(m_actionButtons[i].id, hHitRow);

                                    // Olayı burada bitir (return 0). 
                                    // Böylece satır seçimi (blue highlight) veya drag başlamaz.
                                    return 0;
                                }
                            }
                        }
                    }
                }

                // =========================================================================
                // 2. HEADER RESIZE & DRAG KONTROLÜ (Eski Mimari %100 Korundu)
                // =========================================================================
                HTREEITEM hHeaderResize = nullptr;
                int colIndexResize = -1;

                // Normal HitTest (Satır bulmak için tekrar)
                HTREEITEM hHit = HitTestTreeItem(pt);

                // A) Header'a mı tıklandı?
                if (hHit && IsHeaderItem(hHit))
                {
                    int cIdx = HitTestColumn(hHit, pt);
                    if (cIdx >= 0)
                    {
                        // Sütun taşıma hazırlığı
                        m_dragHeaderItem = hHit;
                        m_dragColumnIndex = cIdx;
                        return 0;
                    }
                }

                // B) Header Resize Bölgesine mi tıklandı?
                if (HitTestHeaderResize(pt, hHeaderResize, colIndexResize))
                {
                    m_isResizingColumn = true;
                    m_resizeHeaderItem = hHeaderResize;
                    m_resizeColumnIndex = colIndexResize;
                    m_resizeStartX = pt.x;
                    m_resizeStartWidth = GetColumnWidth(hHeaderResize, colIndexResize);
                    SetCapture();
                    return 0;
                }

                // =========================================================================
                // 3. STANDART SATIR SEÇİMİ (Eski Mimari %100 Korundu)
                // =========================================================================
                TVHITTESTINFO ht = { 0 };
                ht.pt = pt;

                // Windows'un standart TreeView davranışını (Expand/Collapse [+]) koru
                TreeView_HitTest(GetHwnd(), &ht);

                if (ht.flags & TVHT_ONITEMBUTTON) {
                    break; // Windows yönetsin (Expand/Collapse işlemi)
                }

                // Sağ boşluğa tıklandıysa (hHit null ise), X'i sola çekip tekrar dene (Full Row Select)
                if (hHit == nullptr)
                {
                    TVHITTESTINFO htFix = { 0 };
                    htFix.pt = pt;
                    htFix.pt.x = 5;
                    hHit = TreeView_HitTest(GetHwnd(), &htFix);
                }

                // Eğer geçerli bir satırsa (ve Header değilse) seçimi yap
                if (hHit != nullptr && !IsHeaderItem(hHit))
                {
                    SelectItem(hHit); // Satırı seç (Mavi yap)
                    SetFocus();       // Odağı listeye ver
                    return 0;         // Varsayılan işleyiciyi atla (Inline edit açılmasın diye)
                }

                break;
            }











                // =================================================================
                // 1. SOL TIKLAMA (BAŞLANGIÇ NOKTASI)
                // =================================================================
            //case WM_LBUTTONDOWN:
            //{
            //    CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

            //    // =============================================================
            //    // 1. AKSİYON BUTONLARI KONTROLÜ (EN YÜKSEK ÖNCELİK - DİNAMİK)
            //    // =============================================================
            //    // MouseMove zaten m_hoverButtonID değişkenini ayarlamıştı.
            //    // Eğer mouse bir butonun üzerindeyse, doğrudan o butonun aksiyonunu tetikle.
            //    if (m_hoverButtonID != -1 && m_hoverItem)
            //    {
            //        // Türetilmiş sınıfa (CMyTreeListView) hangi butona basıldığını bildir
            //        OnActionButtonClick(m_hoverButtonID, m_hoverItem);
            //        return 0; // Tıklamayı burada bitir (Seçim veya Drag başlamasın)
            //    }

            //    // =============================================================
            //    // 2. HEADER RESIZE & DRAG KONTROLÜ (Eski Mantık %100 Korundu)
            //    // =============================================================
            //    HTREEITEM hHeaderResize = nullptr;
            //    int colIndexResize = -1;

            //    // Normal HitTest (Satır bulmak için)
            //    HTREEITEM hHit = HitTestTreeItem(pt);

            //    // A) Header'a mı tıklandı?
            //    if (hHit && IsHeaderItem(hHit))
            //    {
            //        int cIdx = HitTestColumn(hHit, pt);
            //        if (cIdx >= 0)
            //        {
            //            // Sütun taşıma hazırlığı
            //            m_dragHeaderItem = hHit;
            //            m_dragColumnIndex = cIdx;
            //            return 0;
            //        }
            //    }

            //    // B) Header Resize Bölgesine mi tıklandı?
            //    if (HitTestHeaderResize(pt, hHeaderResize, colIndexResize))
            //    {
            //        m_isResizingColumn = true;
            //        m_resizeHeaderItem = hHeaderResize;
            //        m_resizeColumnIndex = colIndexResize;
            //        m_resizeStartX = pt.x;
            //        m_resizeStartWidth = GetColumnWidth(hHeaderResize, colIndexResize);
            //        SetCapture();
            //        return 0;
            //    }

            //    // =============================================================
            //    // 3. SATIR SEÇİMİ VE HİLELİ HIT-TEST (Eski Mantık %100 Korundu)
            //    // =============================================================
            //    TVHITTESTINFO ht = { 0 };
            //    ht.pt = pt;

            //    // Windows'un standart TreeView davranışı (Expand butonları vb.)
            //    // Bunu çağırmak önemli, yoksa [+] butonları çalışmayabilir.
            //    // Ancak hHit zaten yukarıda bulundu, tekrar çağırmaya gerek yok ama 
            //    // flags kontrolü için TreeView_HitTest'i kullanıyoruz.
            //    TreeView_HitTest(GetHwnd(), &ht);

            //    if (ht.flags & TVHT_ONITEMBUTTON) {
            //        break; // Windows yönetsin (Expand/Collapse)
            //    }

            //    // Sağ boşluğa tıklandıysa (hHit null ise), X'i sola çekip tekrar dene
            //    if (hHit == nullptr)
            //    {
            //        TVHITTESTINFO htFix = { 0 };
            //        htFix.pt = pt;
            //        htFix.pt.x = 5;
            //        hHit = TreeView_HitTest(GetHwnd(), &htFix);
            //    }

            //    // Eğer geçerli bir satırsa (ve Header değilse)
            //    if (hHit != nullptr && !IsHeaderItem(hHit))
            //    {
            //        SelectItem(hHit); // Satırı seç
            //        SetFocus();       // Odağı listeye ver
            //        return 0;         // Varsayılan işleyiciyi atla (Inline edit açılmasın diye)
            //    }

            //    break;
            //}            // =================================================================
            // 2. FARE HAREKETİ (SÜRÜKLEME AKSİYONU)
            // =================================================================
            case WM_MOUSEMOVE:
            {
                CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

                // ====================================================================
                // A) Sütun Genişletme (Resize) DEVAM EDİYOR (Mevcut Kod)
                // ====================================================================
                if (m_isResizingColumn && m_resizeHeaderItem)
                {
                    int dx = pt.x - m_resizeStartX;
                    int newWidth = m_resizeStartWidth + dx;
                    if (newWidth < m_minColumnWidth) newWidth = m_minColumnWidth;
                    SetColumnWidth(m_resizeHeaderItem, m_resizeColumnIndex, newWidth);
                    return 0;
                }

                // ====================================================================
                // B) Kolon Taşıma (Drag) BAŞLATMA (Mevcut Kod)
                // ====================================================================
                if (!m_isDraggingColumn && m_dragHeaderItem && (wParam & MK_LBUTTON))
                {
                    m_isDraggingColumn = true;
                    m_pDragImage = CreateDragImage(m_dragHeaderItem, m_dragColumnIndex);
                    if (m_pDragImage)
                    {
                        m_pDragImage->BeginDrag(0, CPoint(10, 10));
                        m_pDragImage->DragEnter(NULL, pt);
                    }
                    SetCapture();
                    return 0;
                }

                // ====================================================================
                // C) Kolon Taşıma DEVAM EDİYOR (Mevcut Kod)
                // ====================================================================
                if (m_isDraggingColumn)
                {
                    if (m_pDragImage)
                        m_pDragImage->DragMove(CPoint(pt.x, pt.y));

                    ::SetCursor(::LoadCursor(NULL, IDC_SIZEALL));
                    return 0;
                }

                // ====================================================================
                // D) DİNAMİK AKSİYON BUTONLARI KONTROLÜ (PROFESYONEL MİMARİ)
                // ====================================================================
                if (!m_isDraggingColumn && !m_isResizingColumn)
                {
                    int oldHoverID = m_hoverButtonID;
                    m_hoverButtonID = -1; // Varsayılan: Hiçbir butonda değiliz

                    // 1. Farenin hangi satırda olduğunu bul
                    TVHITTESTINFO ht = { 0 };
                    ht.pt = pt;
                    HTREEITEM hHit = TreeView_HitTest(GetHwnd(), &ht);

                    // Hileli HitTest (Sağ boşluk için)
                    if (!hHit) {
                        ht.pt.x = 5;
                        hHit = TreeView_HitTest(GetHwnd(), &ht);
                    }

                    // 2. Butonlar sadece "Seçili" veya "Hover" olan satırda görünür/aktif olur.
                    //    (Burada hHit'in m_hotItem ile aynı olup olmadığını kontrol ediyoruz)
                    if (hHit != nullptr)
                    {
                        // Satırın koordinatlarını al
                        CRect rcRow;
                        if (GetItemRect(hHit, rcRow, FALSE))
                        {
                            // Dinamik Buton Listesini Dön
                            for (int i = 0; i < (int)m_actionButtons.size(); ++i)
                            {
                                // Helper fonksiyon ile butonun yerini hesapla
                                // (CalculateButtonRect fonksiyonunu sınıfa eklemiştik)
                                CRect rcBtn = CalculateButtonRect(rcRow, i);

                                if (rcBtn.PtInRect(pt))
                                {
                                    m_hoverButtonID = m_actionButtons[i].id;
                                    break; // Buton bulundu, döngüden çık
                                }
                            }
                        }
                    }

                    // 3. Durum değiştiyse sadece ilgili alanı (satırı) yeniden çiz
                    //    (Performans için sadece buton alanını da boyayabiliriz ama satır temizliği için full row daha güvenli)
                    if (oldHoverID != m_hoverButtonID)
                    {
                        if (hHit) {
                            CRect rcRedraw;
                            GetItemRect(hHit, rcRedraw, FALSE);
                            InvalidateRect(rcRedraw, FALSE);
                        }
                        // Eğer mouse önceki bir butondan çıktıysa ve yeni satıra geçtiyse eski satırı da temizle
                        if (m_hoverItem && m_hoverItem != hHit) {
                            CRect rcOld;
                            GetItemRect(m_hoverItem, rcOld, FALSE);
                            InvalidateRect(rcOld, FALSE);
                        }
                    }
                }

                // ====================================================================
                // E) HOVER / HOT-TRACK (Mevcut Kod - Satırın Mavi/Gri Olması)
                // ====================================================================
                if (!m_isDraggingColumn && !m_isResizingColumn)
                {
                    HTREEITEM hNewHot = HitTestTreeItem(pt);
                    if (hNewHot != m_hotItem)
                    {
                        HTREEITEM hOld = m_hotItem;
                        m_hotItem = hNewHot;

                        // Eski hover satırını temizle
                        if (hOld)
                        {
                            CRect rc;
                            if (GetItemRect(hOld, rc, FALSE))
                                InvalidateRect(rc, FALSE);
                        }
                        // Yeni hover satırını boya
                        if (m_hotItem && !IsHeaderItem(m_hotItem))
                        {
                            CRect rc;
                            if (GetItemRect(m_hotItem, rc, FALSE))
                                InvalidateRect(rc, FALSE);
                        }
                    }

                    // Mouse leave takibi başlat (Mouse pencereden çıkarsa hover'ı silmek için)
                    if (!m_isTrackingMouse)
                    {
                        TRACKMOUSEEVENT tme{};
                        tme.cbSize = sizeof(TRACKMOUSEEVENT);
                        tme.dwFlags = TME_LEAVE;
                        tme.hwndTrack = GetHwnd();
                        ::TrackMouseEvent(&tme);
                        m_isTrackingMouse = true;
                    }
                }

                break;
            }            // =================================================================
            // 3. SOL TIK BIRAKMA (YENİ YERE YERLEŞTİRME)
            // =================================================================
// =================================================================
// 3. SOL TUŞ BIRAKILDIĞINDA (LButtonUp)
// =================================================================
            case WM_LBUTTONUP:
            {
                CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

                // ---------------------------------------------------------
                // A) Eğer "Sütun Genişletme (Resize)" Modu Aktifse
                // ---------------------------------------------------------
                if (m_isResizingColumn)
                {
                    m_isResizingColumn = false;
                    m_resizeHeaderItem = nullptr;
                    ReleaseCapture();
                    return 0;
                }

                // ---------------------------------------------------------
                // B) Eğer "Kolon Taşıma (Drag)" Modu Aktifse
                // ---------------------------------------------------------
                if (m_isDraggingColumn)
                {
                    // Drag görselini yok et
                    if (m_pDragImage)
                    {
                        m_pDragImage->DragLeave(GetHwnd());
                        m_pDragImage->EndDrag();
                        delete m_pDragImage;
                        m_pDragImage = nullptr;
                    }

                    ReleaseCapture();

                    // ------------------------------
                    // B1) Bırakılan header satırını bul
                    // ------------------------------
                    TVHITTESTINFO ht = { 0 };
                    ht.pt = pt;
                    HTREEITEM hDropItem = TreeView_HitTest(GetHwnd(), &ht);

                    // Sağ tarafta null dönebilir → "y koordinasyonu sabit tutup X resetle"
                    if (!hDropItem)
                    {
                        TVHITTESTINFO htFix = { 0 };
                        htFix.pt = pt;
                        htFix.pt.x = 5;
                        hDropItem = TreeView_HitTest(GetHwnd(), &htFix);
                    }

                    // Header’a bırakılmalı
                    if (hDropItem && IsHeaderItem(hDropItem))
                    {
                        // ------------------------------
                        // B2) Gerçek kolon HitTest (YENİ)
                        // ------------------------------
                        int dropColIndex = HitTestHeaderColumn(hDropItem, pt);

                        // Eğer oranlar sağ tarafa yakınsa +1
                        if (dropColIndex >= 0)
                        {
                            TLV_Category* cat = FindCategoryByHeader(hDropItem);
                            if (cat)
                            {
                                int x = 0;
                                for (int i = 0; i <= dropColIndex; i++)
                                    x += cat->columns[i].width;

                                int colMid = x - (cat->columns[dropColIndex].width / 2);

                                if (pt.x > colMid)
                                    dropColIndex++;
                            }
                        }

                        // ------------------------------
                        // B3) Kolon Taşıma
                        // ------------------------------
                        if (dropColIndex >= 0)
                        {
                            MoveColumn(
                                m_dragHeaderItem,
                                m_dragColumnIndex,
                                dropColIndex
                            );

                            Invalidate();
                        }
                    }

                    // Drag state temizliği
                    m_isDraggingColumn = false;
                    m_dragHeaderItem = nullptr;
                    m_dragColumnIndex = -1;

                    return 0;
                }

                break;
            }

            // =================================================================
            // 4. DİĞERLERİ
            // =================================================================
            case WM_SETCURSOR:
            {
                if ((HWND)wParam == GetHwnd() && !m_isDraggingColumn)
                {
                    POINT ptScreen; ::GetCursorPos(&ptScreen);
                    POINT ptClient = ptScreen; ::ScreenToClient(GetHwnd(), &ptClient);

                    HTREEITEM hHdr = nullptr; int cIdx = -1;
                    if (HitTestHeaderResize(ptClient, hHdr, cIdx))
                    {
                        ::SetCursor(GetSizeWECursor());
                        return TRUE;
                    }
                }
                break;
            }
            case WM_LBUTTONDBLCLK:
            {
                CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

                // Expand butonuna tıklandıysa bırak Windows yapsın
                TVHITTESTINFO ht = { 0 }; ht.pt = pt;
                TreeView_HitTest(GetHwnd(), &ht);
                if (ht.flags & TVHT_ONITEMBUTTON) break;

                // Gelişmiş HitTest kullan
                auto hit = HitTestEx(pt);

                if (hit.part == HitPart::HeaderItem)
                {
                    // Header çift tıklama (İster Expand yap, ister sıralama)
                    OnHeaderDoubleClick(hit.hItem, pt);
                    return 0;
                }
                else if (hit.part == HitPart::RowCell)
                {
                    // Satır çift tıklama -> Türetilmiş sınıfa (CMyTreeListView) gönder
                    OnRowCellDoubleClick(hit.hItem, hit.subIndex);
                    return 0;
                }
                break;
            }

            case WM_MOUSELEAVE:
            {
                m_isTrackingMouse = false;

                // A) Aksiyon Butonlarını Sıfırla (Yeni)
                if (m_actionBar.hotButton != 0)
                {
                    m_actionBar.hotButton = 0;
                    Invalidate(); // Butonların rengini normale döndür
                }

                // B) Satır Vurgusunu Sıfırla (Eski Mantık Korundu)
                if (m_hotItem)
                {
                    CRect rc;
                    if (GetItemRect(m_hotItem, rc, FALSE))
                        InvalidateRect(rc, FALSE);
                    m_hotItem = nullptr;
                }
                break;
            }

            case TLV_MSG_INLINE_COMMIT: CommitInlineEdit(true); return 0;
            case TLV_MSG_INLINE_CANCEL: CommitInlineEdit(false); return 0;

                // -------------------------------------------------------------
                            // 7. TEMA DEĞİŞTİRME (F12) - GÜVENLİ BLOK
                            // -------------------------------------------------------------
            case WM_KEYDOWN:
            {   // <--- BU PARANTEZ ÇOK ÖNEMLİ! (Scope Başlangıcı)
                if (wParam == VK_F12)
                {
                    // 1. Tema durumunu değiştir
                    bool isDark = GetTheme().dark;
                    SetDarkTheme(!isDark);

                    // 2. Sadece renkleri güncelle, tüm yapıyı bozma
                    // DeleteAllItems() veya Initialize() ÇAĞIRMA! Sadece çizimi yenile.
                    Invalidate();
                    UpdateWindow();
                    return 0;
                }
                break;
            }   // <--- BU PARANTEZ ÇOK ÖNEMLİ! (Scope Bitişi)
                // Yansıtılan mesajlar (Gerekirse)
            case WM_NOTIFY :
            {
                LPNMHDR p = reinterpret_cast<LPNMHDR>(lParam);
                if (p->code == NM_CUSTOMDRAW)
                    return HandleCustomDraw(reinterpret_cast<LPNMTVCUSTOMDRAW>(p));
                break;
            }
            }

            return CTreeView::WndProc(msg, wParam, lParam);
        }


        LRESULT HandleCustomDraw(LPNMTVCUSTOMDRAW pCD) {
            if (pCD->nmcd.dwDrawStage == CDDS_PREPAINT)
                return CDRF_NOTIFYITEMDRAW;

            if (pCD->nmcd.dwDrawStage == CDDS_ITEMPREPAINT) {
                HTREEITEM hItem = (HTREEITEM)pCD->nmcd.dwItemSpec;
                CDC dc(pCD->nmcd.hdc);
                CRect rcRow;
                GetItemRect(hItem, rcRow, FALSE);

                if (IsHeaderItem(hItem)) {
                    DrawHeaderRow(dc, hItem, rcRow);
                }
                else {
                    // Standart satırı çiz
                    DrawDataRow(dc, hItem, rcRow, pCD);

                    // 🔥 DİNAMİK BUTON ÇİZİMİ (Loop)
                    // Sadece Seçili Satır veya Hover Satırı için çiz
                    if (hItem == GetSelection() || hItem == m_hoverItem)
                    {
                        // Butonları sağdan sola doğru dizeriz (Listenin sonu en sağda olur)
                        // Ters iterasyon yapıyoruz ki ekleme sırasına göre sağdan sola dizilsin
                        for (int i = 0; i < (int)m_actionButtons.size(); ++i)
                        {
                            // Butonun yerini hesapla
                            CRect rcBtn = CalculateButtonRect(rcRow, i);
                            const auto& btn = m_actionButtons[i];

                            bool isHover = (m_hoverButtonID == btn.id);

                            // Arka Plan (Brush ile)
                            COLORREF bgClr = isHover ? btn.colorHover : btn.colorNormal;
                            CBrush br(bgClr);
                            dc.FillRect(rcBtn, br);

                            // Kenarlık (Efektli)
                            dc.DrawEdge(rcBtn, isHover ? BDR_RAISEDINNER : BDR_SUNKENOUTER, BF_RECT);

                            // İkon/Yazı
                            dc.SetBkMode(TRANSPARENT);
                            dc.SetTextColor(isHover ? RGB(255, 255, 255) : RGB(50, 50, 50));

                            // Yazıyı ortala
                            dc.DrawText(btn.icon, -1, rcBtn, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                        }
                    }
                }
                return CDRF_SKIPDEFAULT;
            }
            return CDRF_DODEFAULT;
        }

        //LRESULT HandleCustomDraw(LPNMTVCUSTOMDRAW pCD) {
        //    if (pCD->nmcd.dwDrawStage == CDDS_PREPAINT)
        //        return CDRF_NOTIFYITEMDRAW;

        //    if (pCD->nmcd.dwDrawStage == CDDS_ITEMPREPAINT) {
        //        HTREEITEM hItem = (HTREEITEM)pCD->nmcd.dwItemSpec;
        //        CDC dc(pCD->nmcd.hdc);
        //        CRect rcRow;
        //        GetItemRect(hItem, rcRow, FALSE);

        //        if (IsHeaderItem(hItem)) {
        //            DrawHeaderRow(dc, hItem, rcRow);
        //        }
        //        else {
        //            // Mevcut satır çizimi (Kart veya Liste modu)
        //            DrawDataRow(dc, hItem, rcRow, pCD);

        //            // ------------------------------------------------------------
        //            // 🔥 AKSİYON BUTONLARI (ACTION BAR) - GÜVENLİ GDI ÇİZİMİ
        //            // ------------------------------------------------------------
        //            // Sadece seçili olan satıra butonları ekle
        //            if (hItem == GetSelection()) {
        //                CRect rcClient = GetClientRect();
        //                int btnSize = rcRow.Height() - 8; // Satırdan 4px yukarıdan-aşağıdan pay bırak
        //                int rightEdge = rcClient.right - 5;

        //                // Buton koordinatlarını hesapla
        //                m_actionBar.rcPrint = CRect(rightEdge - btnSize, rcRow.top + 4, rightEdge, rcRow.top + 4 + btnSize);
        //                m_actionBar.rcEdit = CRect(m_actionBar.rcPrint.left - btnSize - 4, rcRow.top + 4, m_actionBar.rcPrint.left - 4, rcRow.top + 4 + btnSize);
        //                m_actionBar.hActiveItem = hItem;


        //                // Çizim Lambdası (FillSolidRect yerine FillRect + CBrush kullanımı)
        //                auto DrawTlvBtn = [&](CRect rc, LPCTSTR icon, COLORREF hotColor, int idx) {
        //                    bool isHot = (m_actionBar.hotButton == idx);

        //                    // Renk seçimi ve Fırça (Brush) oluşturma
        //                    COLORREF finalBg = isHot ? hotColor : RGB(240, 240, 240);
        //                    CBrush br(finalBg);

        //                    // FillSolidRect yerine FillRect kullanıyoruz
        //                    dc.FillRect(rc, br);

        //                    // Kenarlık ve Yazı
        //                    dc.DrawEdge(rc, isHot ? BDR_SUNKENOUTER : BDR_RAISEDINNER, BF_RECT);
        //                    dc.SetTextColor(isHot ? RGB(255, 255, 255) : RGB(60, 60, 60));
        //                    dc.SetBkMode(TRANSPARENT);
        //                    dc.DrawText(icon, -1, rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        //                    };

        //                DrawTlvBtn(m_actionBar.rcEdit, L"Düzenle", RGB(0, 120, 215), 1);
        //                DrawTlvBtn(m_actionBar.rcPrint, L"Baskı Önizleme", RGB(34, 139, 34), 2);
        //            }
        //        }
        //        return CDRF_SKIPDEFAULT;
        //    }
        //    return CDRF_DODEFAULT;
        //}
        


        void DrawGradient(CDC& dc, const CRect& rc, COLORREF c1, COLORREF c2)
        {
            TRIVERTEX vert[2];
            GRADIENT_RECT gRect;

            vert[0].x = rc.left;
            vert[0].y = rc.top;
            vert[0].Red = (COLOR16)(GetRValue(c1) << 8);
            vert[0].Green = (COLOR16)(GetGValue(c1) << 8);
            vert[0].Blue = (COLOR16)(GetBValue(c1) << 8);
            vert[0].Alpha = 0x0000;

            vert[1].x = rc.right;
            vert[1].y = rc.bottom;
            vert[1].Red = (COLOR16)(GetRValue(c2) << 8);
            vert[1].Green = (COLOR16)(GetGValue(c2) << 8);
            vert[1].Blue = (COLOR16)(GetBValue(c2) << 8);
            vert[1].Alpha = 0x0000;

            gRect.UpperLeft = 0;
            gRect.LowerRight = 1;

            ::GradientFill(dc, vert, 2, &gRect, 1, GRADIENT_FILL_RECT_V);
        }
        // wxx_treelistview.h içine (CTreeListView class'ı protected bölümüne):

// wxx_treelistview.h -> CTreeListView -> DrawHeaderRow fonksiyonunun başı:
// wxx_treelistview.h -> CTreeListView -> protected:

// wxx_treelistview.h -> CTreeListView -> protected

        void DrawHeaderRow(CDC& dc, HTREEITEM hHeader, const CRect& rcRow)
        {
            // GÜVENLİK 1: Kategori pointer'ı geçerli mi?
            TLV_Category* cat = FindCategoryByHeader(hHeader);
            if (cat == nullptr) return;

            // =========================================================
            // 1. BÖLÜM: KATEGORİ BAŞLIĞI (Fake Header)
            // =========================================================
            CRect rcCat = rcRow;
            rcCat.bottom = rcCat.top + (m_theme.categoryHeaderHeight);

            COLORREF cTop, cBot, cText, cLine;

            // A) Kategori Rengi Belirleme (Özel Renk veya Tema)
            if (cat->headerColor != CLR_NONE)
            {
                COLORREF base = cat->headerColor;

                // Açık Ton (Top) - Basit hesaplama
                int r1 = GetRValue(base) + 90; if (r1 > 255) r1 = 255;
                int g1 = GetGValue(base) + 90; if (g1 > 255) g1 = 255;
                int b1 = GetBValue(base) + 90; if (b1 > 255) b1 = 255;
                cTop = RGB(r1, g1, b1);

                // Hafif Açık Ton (Bottom)
                int r2 = GetRValue(base) + 20; if (r2 > 255) r2 = 255;
                int g2 = GetGValue(base) + 20; if (g2 > 255) g2 = 255;
                int b2 = GetBValue(base) + 20; if (b2 > 255) b2 = 255;
                cBot = RGB(r2, g2, b2);

                cText = RGB(255, 255, 255); // Özel renklerde beyaz yazı
                cLine = base;
            }
            else
            {
                cTop = m_theme.dark ? RGB(50, 55, 60) : RGB(235, 245, 255);
                cBot = m_theme.dark ? RGB(35, 40, 45) : RGB(210, 230, 250);
                cText = m_theme.dark ? RGB(220, 220, 220) : m_theme.clrCatText;
                cLine = m_theme.dark ? RGB(100, 100, 100) : RGB(153, 180, 209);
            }

            // Kategori Arka Planı (Gradient)
            DrawGradient(dc, rcCat, cTop, cBot);

            // Kategori Alt Çizgisi
            CBrush brLine(cLine);
            dc.FillRect(CRect(rcCat.left, rcCat.bottom - 2, rcCat.right, rcCat.bottom), brLine);

            // Kategori Metni
            dc.SetBkMode(TRANSPARENT);
            dc.SetTextColor(cText);

            CRect rcText = rcCat;
            rcText.DeflateRect(8, 0);

            CString disp = cat->displayName.IsEmpty() ? cat->name : cat->displayName;

            // Metin Gölgesi (Sadece özel renklerde)
            if (cat->headerColor != CLR_NONE) {
                dc.SetTextColor(RGB(60, 60, 60));
                dc.DrawText(disp, -1, CRect(rcText.left + 1, rcText.top + 1, rcText.right, rcText.bottom + 1),
                    DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
                dc.SetTextColor(cText);
            }

            dc.DrawText(disp, -1, rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

            // =========================================================
            // 2. BÖLÜM: KOLON BAŞLIKLARI (SubHeader - 3D)
            // =========================================================
            CRect rcCols = rcRow;
            rcCols.top = rcCat.bottom;
            rcCols.bottom = rcCols.top + m_theme.headerHeight;

            COLORREF colTop = m_theme.dark ? RGB(65, 65, 70) : RGB(252, 252, 252);
            COLORREF colBot = m_theme.dark ? RGB(45, 45, 50) : RGB(225, 225, 228);

            // Kolon Arka Planı (Tüm Şerit)
            DrawGradient(dc, rcCols, colTop, colBot);

            // Üst ve Alt Çizgiler (Highlight & Shadow)
            CBrush brWhite(RGB(255, 255, 255));
            CBrush brGray(RGB(170, 170, 170));
            dc.FillRect(CRect(rcCols.left, rcCols.top, rcCols.right, rcCols.top + 1), brWhite);
            dc.FillRect(CRect(rcCols.left, rcCols.bottom - 1, rcCols.right, rcCols.bottom), brGray);

            int x = rcCols.left;

            // Kalemleri önceden oluştur (Performans için)
            CPen penShadow(PS_SOLID, 1, RGB(160, 160, 160));
            CPen penHighlight(PS_SOLID, 1, RGB(255, 255, 255));

            for (const auto& col : cat->columns)
            {
                if (!col.visible) continue;
                int w = col.width;

                CRect rcCol(x, rcCols.top, x + w, rcCols.bottom);

                // Dikey Ayırıcı (3D Etched Line)
                CPen pOld = dc.SelectObject(penShadow);
                dc.MoveTo(rcCol.right - 2, rcCol.top + 4);
                dc.LineTo(rcCol.right - 2, rcCol.bottom - 4);

                dc.SelectObject(penHighlight);
                dc.MoveTo(rcCol.right - 1, rcCol.top + 4);
                dc.LineTo(rcCol.right - 1, rcCol.bottom - 4);

                dc.SelectObject(pOld); // DC'yi temizle

                // Kolon Başlığı Metni
                dc.SetTextColor(m_theme.clrHdrText);

                CRect rcTxt = rcCol;
                rcTxt.DeflateRect(6, 0);

                UINT align = DT_LEFT;
                if (col.align == DT_CENTER) align = DT_CENTER;
                else if (col.align == DT_RIGHT) align = DT_RIGHT;

                dc.DrawText(col.title, -1, rcTxt, align | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

                x += w;
            }

            // 🔥 DÜZELTME: Sağ tarafta kalan boşluğu temizle (Hayaletleri önler)
            if (x < rcRow.right)
            {
                CRect rcEmpty = rcRow;
                rcEmpty.left = x;
                rcEmpty.top = rcCols.top;
                rcEmpty.bottom = rcCols.bottom;

                // Boşluğu Header stiliyle doldur (Devamlılık sağlar)
                DrawGradient(dc, rcEmpty, colTop, colBot);
                dc.FillRect(CRect(rcEmpty.left, rcEmpty.bottom - 1, rcEmpty.right, rcEmpty.bottom), brGray);
            }
        }


        //void DrawHeaderRow(CDC& dc, HTREEITEM hHeader, const CRect& rcRow)
        //{
        //    TLV_Category* cat = FindCategoryByHeader(hHeader);
        //    if (!cat) return;

        //    // =========================================================
        //    // 1. BÖLÜM: KATEGORİ BAŞLIĞI (Dinamik Renk Hesabı)
        //    // =========================================================
        //    CRect rcCat = rcRow;
        //    rcCat.bottom = rcCat.top + (m_theme.categoryHeaderHeight);

        //    COLORREF cTop, cBot, cText, cLine;

        //    // A) Kategoriye Özel Renk Var mı?
        //    if (cat->headerColor != CLR_NONE)
        //    {
        //        // Baz Rengi Al (Örn: Koyu Yeşil)
        //        COLORREF base = cat->headerColor;

        //        // Otomatik 3D Tonlama (Lighten / Darken)
        //        // Üst kısım için rengi %40 aç, Alt kısım için %10 aç
        //        // (Bu basit bir renk manipülasyonudur)
        //        auto Lighten = [](COLORREF c, int val) -> COLORREF {
        //            int r = GetRValue(c) + val; if (r > 255) r = 255;
        //            int g = GetGValue(c) + val; if (g > 255) g = 255;
        //            int b = GetBValue(c) + val; if (b > 255) b = 255;
        //            return RGB(r, g, b);
        //            };

        //        cTop = Lighten(base, 90); // Çok açık ton (Üst)
        //        cBot = Lighten(base, 20); // Hafif açık ton (Alt)
        //        cText = RGB(255, 255, 255); // Özel renkli başlıklarda Beyaz yazı şık durur
        //        cLine = base; // Alt çizgi baz renk olsun
        //    }
        //    else
        //    {
        //        // Yoksa Temayı Kullan (Eski kod)
        //        cTop = m_theme.dark ? RGB(50, 55, 60) : RGB(235, 245, 255);
        //        cBot = m_theme.dark ? RGB(35, 40, 45) : RGB(210, 230, 250);
        //        cText = m_theme.dark ? RGB(220, 220, 220) : m_theme.clrCatText;
        //        cLine = m_theme.dark ? RGB(100, 100, 100) : RGB(153, 180, 209);
        //    }

        //    // Çizimi Yap
        //    DrawGradient(dc, rcCat, cTop, cBot);
        //    dc.FillRect(CRect(rcCat.left, rcCat.bottom - 1, rcCat.right, rcCat.bottom), CBrush(cLine));

        //    dc.SetBkMode(TRANSPARENT);
        //    dc.SetTextColor(cText);

        //    CRect rcText = rcCat;
        //    rcText.DeflateRect(8, 0);
        //    CString disp = cat->displayName.IsEmpty() ? cat->name : cat->displayName;

        //    // Gölge efekti (Metin okunurluğunu artırır)
        //    if (cat->headerColor != CLR_NONE) {
        //        dc.SetTextColor(RGB(60, 60, 60)); // Gölge rengi
        //        dc.DrawText(disp, -1, CRect(rcText.left + 1, rcText.top + 1, rcText.right, rcText.bottom + 1),
        //            DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        //        dc.SetTextColor(cText); // Asıl renk
        //    }

        //    dc.DrawText(disp, -1, rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

        //    // ... (KODUN DEVAMI / 2. BÖLÜM KOLONLAR AYNEN KALACAK) ...
        //    // =========================================================
        //    // 2. BÖLÜM: KOLON BAŞLIKLARI (SubHeaders - Alt Kısım)
        //    // =========================================================
        //    // Burası 3D, Gri, Camsı görünüm olacak
        //    // =========================================================
        //    CRect rcCols = rcRow;
        //    rcCols.top = rcCat.bottom;
        //    rcCols.bottom = rcCols.top + m_theme.headerHeight;

        //    // A) Arka Plan (Klasik Windows/Office Gri Gradient)
        //    COLORREF colTop = m_theme.dark ? RGB(65, 65, 70) : RGB(252, 252, 252);
        //    COLORREF colBot = m_theme.dark ? RGB(45, 45, 50) : RGB(225, 225, 228);

        //    DrawGradient(dc, rcCols, colTop, colBot);

        //    // B) Çerçeve Efektleri (Highlight & Shadow)
        //    // En üste 1px beyaz parlama
        //    dc.FillRect(CRect(rcCols.left, rcCols.top, rcCols.right, rcCols.top + 1), CBrush(RGB(255, 255, 255)));
        //    // En alta 1px koyu gölge
        //    dc.FillRect(CRect(rcCols.left, rcCols.bottom - 1, rcCols.right, rcCols.bottom), CBrush(RGB(170, 170, 170)));

        //    // C) Kolonları Döngüyle Çiz
        //    int x = rcCols.left;

        //    // Kalemler (Ayırıcı çizgiler için)
        //    CPen penShadow(PS_SOLID, 1, RGB(160, 160, 160)); // Koyu gri
        //    CPen penHighlight(PS_SOLID, 1, RGB(255, 255, 255)); // Beyaz

        //    for (const auto& col : cat->columns)
        //    {
        //        if (!col.visible) continue;
        //        int w = col.width;

        //        CRect rcCol(x, rcCols.top, x + w, rcCols.bottom);

        //        // --- Dikey Ayırıcı (Separator) ---
        //        // Sağ kenara "Oyuk" (Etched) çizgi çekiyoruz

        //        // 1. Gölge Çizgisi
        //        CPen pOld = dc.SelectObject(penShadow);
        //        dc.MoveTo(rcCol.right - 2, rcCol.top + 4);    // Yukarıdan 4px boşluk
        //        dc.LineTo(rcCol.right - 2, rcCol.bottom - 4); // Aşağıdan 4px boşluk

        //        // 2. Parlama Çizgisi (Hemen sağına)
        //        dc.SelectObject(penHighlight);
        //        dc.MoveTo(rcCol.right - 1, rcCol.top + 4);
        //        dc.LineTo(rcCol.right - 1, rcCol.bottom - 4);

        //        dc.SelectObject(pOld);

        //        // --- Kolon Metni ---
        //        dc.SetTextColor(m_theme.clrHdrText); // Normal gri/siyah metin
        //        // Veya daha yumuşak bir gri: dc.SetTextColor(RGB(60, 60, 60));

        //        CRect rcTxt = rcCol;
        //        rcTxt.DeflateRect(6, 0); // Kenar boşluğu

        //        // Hizalama ayarı
        //        UINT align = DT_LEFT;
        //        if (col.align == DT_CENTER) align = DT_CENTER;
        //        else if (col.align == DT_RIGHT) align = DT_RIGHT;

        //        dc.DrawText(col.title, -1, rcTxt, align | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

        //        x += w;
        //    }

        //    // Son boşluk kalırsa orayı da temizle/boya (Gerekirse)
        //}


        //void DrawHeaderRow(CDC& dc, HTREEITEM hHeader, const CRect& rcRow) {
        //    TLV_Category* cat = FindCategoryByHeader(hHeader);
        //    if (!cat) return;

        //    // 1. KATEGORİ BAŞLIĞI (3D Gradient)
        //    CRect rcCat = rcRow;
        //    rcCat.bottom = rcCat.top + (m_theme.categoryHeaderHeight);

        //    // Arkaplan Gradient
        //    DrawGradient(dc, rcCat, m_theme.clrHdrGradTop, m_theme.clrHdrGradBot);

        //    // Altına ince bir çizgi (Bevel efekti için)
        //    dc.FillRect(CRect(rcCat.left, rcCat.bottom - 1, rcCat.right, rcCat.bottom), CBrush(RGB(180, 180, 180)));
        //    // Üstüne ince bir highlight (Parlama efekti)
        //    dc.FillRect(CRect(rcCat.left, rcCat.top, rcCat.right, rcCat.top + 1), CBrush(RGB(255, 255, 255)));

        //    // Metin çizimi
        //    dc.SetBkMode(TRANSPARENT);
        //    dc.SetTextColor(m_theme.clrHdrText);

        //    // İkon varsa çiz (Opsiyonel: İkon boyutuna göre ayarla)
        //    int textOffset = 6;
        //    // ... (İkon çizimi kodu buraya eklenebilir)

        //    // Fontu biraz kalınlaştırabilirsin (Opsiyonel)
        //    // CFont fontBold; fontBold.CreatePointFont(90, _T("Segoe UI Bold"), &dc);
        //    // CFont* pOldFont = dc.SelectObject(&fontBold);

        //    CRect rcText = rcCat;
        //    rcText.DeflateRect(textOffset, 0);
        //    CString disp = cat->displayName.IsEmpty() ? cat->name : cat->displayName;

        //    // Hafif gölgeli metin (3D hissi için) - İsteğe bağlı
        //    // dc.SetTextColor(RGB(255,255,255));
        //    // dc.DrawText(disp, -1, CRect(rcText.left+1, rcText.top+1, rcText.right, rcText.bottom), DT_LEFT | DT_VCENTER | DT_SINGLELINE);

        //    dc.SetTextColor(m_theme.clrHdrText);
        //    dc.DrawText(disp, -1, rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

        //    // dc.SelectObject(pOldFont); // Fontu geri al

        //    // 2. KOLON BAŞLIKLARI (Daha sade ama gridli)
        //    CRect rcCols = rcRow;
        //    rcCols.top = rcCat.bottom;
        //    rcCols.bottom = rcCols.top + m_theme.headerHeight;

        //    // Kolon başlıkları için daha yumuşak bir gradient
        //    COLORREF colGradTop = m_theme.dark ? RGB(50, 50, 50) : RGB(248, 248, 248);
        //    COLORREF colGradBot = m_theme.dark ? RGB(40, 40, 40) : RGB(235, 235, 235);

        //    // Arka planı boya (Tüm şerit)
        //    DrawGradient(dc, rcCols, colGradTop, colGradBot);

        //    int x = rcCols.left;
        //    for (const auto& col : cat->columns) {
        //        if (!col.visible) continue;
        //        int w = col.width;
        //        CRect rcCol(x, rcCols.top, x + w, rcCols.bottom);

        //        // Dikey ayırıcı çizgi (Vertical Separator) - 3D görünüm
        //        CPen pLight(PS_SOLID, 1, m_theme.dark ? RGB(80, 80, 80) : RGB(255, 255, 255));
        //        CPen pDark(PS_SOLID, 1, m_theme.clrGrid);

        //        // Sağ taraf gölgesi
        //        CPen o = dc.SelectObject(pDark);
        //        dc.MoveTo(rcCol.right - 1, rcCol.top);
        //        dc.LineTo(rcCol.right - 1, rcCol.bottom);

        //        // Sağ taraf parlaması (bir yanına)
        //        dc.SelectObject(pLight);
        //        dc.MoveTo(rcCol.right, rcCol.top);
        //        dc.LineTo(rcCol.right, rcCol.bottom);

        //        dc.SelectObject(o);

        //        // Metin
        //        dc.SetTextColor(m_theme.clrHdrText);
        //        CRect rcTxt = rcCol;
        //        rcTxt.DeflateRect(4, 0);
        //        dc.DrawText(col.title, -1, rcTxt, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        //        x += w;
        //    }

        //    // Alt Çizgi (Kategoriyi veriden ayırır)
        //    dc.FillRect(CRect(rcRow.left, rcCols.bottom - 1, rcRow.right, rcCols.bottom), CBrush(RGB(160, 160, 160)));

        //    // Subheader çizimi (varsa)... (Önceki kodun aynısı kalabilir veya gradient eklenebilir)
        //}



        //void DrawHeaderRow(CDC& dc, HTREEITEM hHeader, const CRect& rcRow) {
        //    TLV_Category* cat = FindCategoryByHeader(hHeader);
        //    if (!cat) return;

        //    CRect rcCat = rcRow;
        //    rcCat.bottom = rcCat.top + (m_theme.categoryHeaderHeight);

        //    COLORREF bk = (cat->style.back != CLR_NONE) ? cat->style.back : m_theme.clrHdrBk;
        //    COLORREF tx = (cat->style.text != CLR_NONE) ? cat->style.text : m_theme.clrHdrText;

        //    dc.FillRect(rcCat, CBrush(bk));
        //    dc.SetTextColor(tx);

        //    CRect rcText = rcCat;
        //    rcText.DeflateRect(4, 0);
        //    CString disp = cat->displayName.IsEmpty() ? cat->name : cat->displayName;
        //    dc.DrawText(disp, -1, rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

        //    // Kolon başlıkları kısmı
        //    CRect rcCols = rcRow;
        //    rcCols.top = rcCat.bottom;
        //    rcCols.bottom = rcCols.top + m_theme.headerHeight;

        //    int x = rcCols.left;
        //    int yTop = rcCols.top;
        //    int yBot = rcCols.bottom;

        //    for (const auto& col : cat->columns) {
        //        if (!col.visible) continue;
        //        int w = col.width;

        //        CRect rcCol(x, yTop, x + w, yBot);

        //        COLORREF hdrBk = (col.headerStyle.back != CLR_NONE) ? col.headerStyle.back : m_theme.clrHdrBk;
        //        COLORREF hdrTx = (col.headerStyle.text != CLR_NONE) ? col.headerStyle.text : m_theme.clrHdrText;

        //        dc.FillRect(rcCol, CBrush(hdrBk));
        //        dc.SetTextColor(hdrTx);

        //        CRect rcTxt = rcCol;
        //        rcTxt.DeflateRect(2, 0);
        //        dc.DrawText(col.title, -1, rcTxt, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        //        CPen g(PS_SOLID, 1, m_theme.clrGrid);
        //        CPen o = dc.SelectObject(g);
        //        dc.MoveTo(rcCol.right - 1, rcCol.top);
        //        dc.LineTo(rcCol.right - 1, rcCol.bottom);
        //        dc.SelectObject(o);

        //        x += w;
        //    }

        //    // Subheader’lar (varsa) – şimdilik basit çizim (A adımı: model hazır, çizim ileride zenginleştirilecek)
        //    int depth = cat->GetTotalSubHeaderDepth();
        //    if (depth > 0) {
        //        CRect rcSub = rcCols;
        //        rcSub.top = rcCols.bottom;
        //        rcSub.bottom = rcSub.top + depth * m_theme.subHeaderHeight;

        //        x = rcSub.left;
        //        for (const auto& col : cat->columns) {
        //            if (!col.visible) continue;
        //            int w = col.width;

        //            for (size_t i = 0; i < col.subHeaders.size(); ++i) {
        //                CRect rcCell(
        //                    x,
        //                    rcSub.top + (int)i * m_theme.subHeaderHeight,
        //                    x + w,
        //                    rcSub.top + (int)(i + 1) * m_theme.subHeaderHeight);

        //                COLORREF shBk = (col.subHeaders[i].style.back != CLR_NONE)
        //                    ? col.subHeaders[i].style.back : m_theme.clrHdrBk;
        //                COLORREF shTx = (col.subHeaders[i].style.text != CLR_NONE)
        //                    ? col.subHeaders[i].style.text : m_theme.clrHdrText;

        //                dc.FillRect(rcCell, CBrush(shBk));
        //                dc.SetTextColor(shTx);

        //                CRect rcTxt = rcCell;
        //                rcTxt.DeflateRect(2, 0);
        //                dc.DrawText(col.subHeaders[i].text, -1, rcTxt,
        //                    DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        //            }

        //            x += w;
        //        }
        //    }
        //}

// wxx_treelistview.h -> CTreeListView -> protected

// wxx_treelistview.h -> CTreeListView -> protected
// wxx_treelistview.h -> CTreeListView -> protected:

// wxx_treelistview.h -> CTreeListView -> protected:

// wxx_treelistview.h -> CTreeListView -> protected:

// wxx_treelistview.h -> CTreeListView -> protected:

void DrawDataRow(CDC& dc, HTREEITEM hItem, const CRect& rcRow, LPNMTVCUSTOMDRAW pCD)
{
    TLV_Row* row = GetRow(hItem);
    TLV_Category* cat = GetCategoryForRow(hItem);
    if (!row || !cat) return;

    bool sel = (pCD->nmcd.uItemState & CDIS_SELECTED) != 0;

    // 1. ZEMİNİ TEMİZLE
    dc.FillRect(rcRow, CBrush(m_theme.clrWnd));

    // 2. KAPSÜL ALANINI BELİRLE
    CRect rcCapsule = rcRow;
    rcCapsule.DeflateRect(4, 2, 4, 2);

    // Güvenlik: Negatif boyut hatasını önle
    if (rcCapsule.Width() <= 0 || rcCapsule.Height() <= 0) return;

    // 3. GÖLGE EFEKTİ
    CRect rcShadow = rcCapsule;
    rcShadow.OffsetRect(2, 2);

    CBrush brShadow(RGB(220, 220, 225));
    CPen penNull(PS_NULL, 0, 0);

    // DÜZELTME: Pointer (&) kaldırıldı, HGDIOBJ dönüş türü kullanıldı
    HGDIOBJ oldBrush = dc.SelectObject(brShadow);
    HGDIOBJ oldPen = dc.SelectObject(penNull);

    dc.RoundRect(rcShadow.left, rcShadow.top, rcShadow.right, rcShadow.bottom, 10, 10);

    dc.SelectObject((HRGN)oldBrush);
    dc.SelectObject((HPEN)oldPen);

    // 4. RENKLER
    COLORREF cTop, cBot, cBorder;
    if (sel) {
        cTop = RGB(225, 245, 255);
        cBot = RGB(190, 230, 255);
        cBorder = RGB(51, 153, 255);
    }
    else {
        cTop = RGB(255, 255, 255);
        cBot = RGB(240, 240, 245);
        cBorder = RGB(200, 200, 210);
    }

    // 5. ANA ŞEKLİ ÇİZ (Gradient + Clip)
    int savedDC = dc.SaveDC();

    CRgn rgn;
    rgn.CreateRoundRectRgn(rcCapsule.left, rcCapsule.top, rcCapsule.right, rcCapsule.bottom, 10, 10);
    dc.SelectClipRgn(rgn);

    DrawGradient(dc, rcCapsule, cTop, cBot);

    // Parlama Efekti
    CRect rcHighlight = rcCapsule;
    rcHighlight.bottom = rcHighlight.top + (rcCapsule.Height() / 2);
    // İsteğe bağlı: Üst yarıya hafif beyazlık eklenebilir

    dc.RestoreDC(savedDC);

    // 6. ÇERÇEVE
    // DÜZELTME: "CreateStockObject" hatası için "GetStockObject" kullanıyoruz
    // İçi boş (Hollow) fırça seçerek sadece kenarlık çizmesini sağlıyoruz

    CPen penBorder(PS_SOLID, 1, cBorder);

    // NULL_BRUSH seçimi (Stok nesne)
    oldBrush = dc.SelectObject((HBRUSH)::GetStockObject(NULL_BRUSH));
    // Pointer (&) kaldırıldı
    oldPen = dc.SelectObject(penBorder);

    dc.RoundRect(rcCapsule.left, rcCapsule.top, rcCapsule.right, rcCapsule.bottom, 10, 10);

    dc.SelectObject((HBRUSH)oldBrush);
    dc.SelectObject((HPEN)oldPen);

    // 7. HÜCRELER
    dc.SetBkMode(TRANSPARENT);
    int x = rcRow.left;

    CPen penSep(PS_SOLID, 1, sel ? RGB(160, 200, 240) : RGB(225, 225, 230));
    CPen penSepLight(PS_SOLID, 1, RGB(255, 255, 255));

    for (size_t i = 0; i < cat->columns.size(); ++i)
    {
        const auto& col = cat->columns[i];
        if (!col.visible) continue;

        int w = col.width;
        CRect rcCell(x, rcRow.top, x + w, rcRow.bottom);

        // Metin Rengi
        COLORREF cTx = m_theme.clrText;
        if (sel) cTx = RGB(0, 50, 120);
        if (!sel && i < row->cells.size() && row->cells[i].colors.text != CLR_NONE)
            cTx = row->cells[i].colors.text;

        dc.SetTextColor(cTx);

        CRect rcTxt = rcCell;
        rcTxt.DeflateRect(6, 0);

        UINT fmt = DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS;
        if (col.align == DT_RIGHT)      fmt |= DT_RIGHT;
        else if (col.align == DT_CENTER) fmt |= DT_CENTER;
        else                              fmt |= DT_LEFT;

        if (i < row->cells.size())
            dc.DrawText(row->cells[i].text, -1, rcTxt, fmt);

        // Ayıraçlar
        if (i < cat->columns.size() - 1)
        {
            int sepX = rcCell.right;
            int sepTop = rcCapsule.top + 4;
            int sepBot = rcCapsule.bottom - 4;

            if (sepTop < sepBot)
            {
                // DÜZELTME: Pointer (&) kaldırıldı
                oldPen = dc.SelectObject(penSep);
                dc.MoveTo(sepX - 1, sepTop);
                dc.LineTo(sepX - 1, sepBot);

                dc.SelectObject(penSepLight);
                dc.MoveTo(sepX, sepTop);
                dc.LineTo(sepX, sepBot);

                dc.SelectObject((HPEN)oldPen);
            }
        }
        x += w;
    }
}

//void DrawDataRow(CDC& dc, HTREEITEM hItem, const CRect& rcRow, LPNMTVCUSTOMDRAW pCD)
//{
//    TLV_Row* row = GetRow(hItem);
//    TLV_Category* cat = GetCategoryForRow(hItem);
//    if (!row || !cat) return;
//
//    bool sel = (pCD->nmcd.uItemState & CDIS_SELECTED) != 0;
//
//    // Alternatif satır arka planı
//    bool isAlt = ((pCD->nmcd.dwItemSpec & 1) == 1);
//    COLORREF baseBk = isAlt ? RGB(250, 250, 250) : RGB(242, 242, 242);
//
//    COLORREF rBk = baseBk;
//    COLORREF rTx = m_theme.clrText;
//
//    // ============================================================
//    // 1) SEÇİLİ SATIR ÖNCELİKLE ELE ALINIR
//    // ============================================================
//    if (sel)
//    {
//        rBk = m_theme.clrSelBk;      // Mavi arka plan
//        rTx = m_theme.clrSelText;    // Beyaz metin
//    }
//    else
//    {
//        // --- Satır renkleri (sadece seçili değilse) ---
//        if (row->style.back != CLR_NONE)
//            rBk = row->style.back;
//
//        if (row->style.text != CLR_NONE)
//            rTx = row->style.text;
//
//        // --- Hover efekti (sadece seçili değilse) ---
//        if (m_hotItem == hItem)
//            rBk = RGB(235, 245, 255);
//    }
//
//    // Arkaplanı boya
//    dc.FillRect(rcRow, CBrush(rBk));
//    dc.SetBkMode(TRANSPARENT);
//
//    int x = rcRow.left;
//
//    // Grid kalemi
//    CPen penGrid(PS_SOLID, 1, m_theme.clrGrid);
//
//    // ============================================================
//    // 2) HÜCRELERİ ÇİZ
//    // ============================================================
//    for (size_t i = 0; i < cat->columns.size(); ++i)
//    {
//        const auto& col = cat->columns[i];
//        if (!col.visible) continue;
//
//        int w = col.width;
//        CRect rcCell(x, rcRow.top, x + w, rcRow.bottom);
//
//        COLORREF cBk = rBk;
//        COLORREF cTx = rTx;
//
//        // Hücre özel renkleri sadece seçili DEĞİLSE çalışır
//        if (!sel && i < row->cells.size())
//        {
//            if (row->cells[i].colors.back != CLR_NONE)
//                cBk = row->cells[i].colors.back;
//
//            if (row->cells[i].colors.text != CLR_NONE)
//                cTx = row->cells[i].colors.text;
//        }
//
//        // Hücre arka planı özel ise boya
//        if (cBk != rBk)
//            dc.FillRect(rcCell, CBrush(cBk));
//
//        // Metni çiz
//        dc.SetTextColor(cTx);
//        CRect rcTxt = rcCell;
//        rcTxt.DeflateRect(4, 0);
//
//        UINT fmt = DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS;
//        if (col.align == DT_RIGHT)      fmt |= DT_RIGHT;
//        else if (col.align == DT_CENTER) fmt |= DT_CENTER;
//        else                              fmt |= DT_LEFT;
//
//        if (i < row->cells.size())
//            dc.DrawText(row->cells[i].text, -1, rcTxt, fmt);
//
//        // Dikey grid çizgisi
//        CPen pOld = dc.SelectObject(penGrid);
//        dc.MoveTo(rcCell.right - 1, rcCell.top);
//        dc.LineTo(rcCell.right - 1, rcCell.bottom);
//        dc.SelectObject(pOld);
//
//        x += w;
//    }
//
//    // ============================================================
//    // 3) ALT ÇİZGİ
//    // ============================================================
//    CPen pOld = dc.SelectObject(penGrid);
//    dc.MoveTo(rcRow.left, rcRow.bottom - 1);
//    dc.LineTo(rcRow.right, rcRow.bottom - 1);
//    dc.SelectObject(pOld);
//
//    // ============================================================
//    // 4) SAĞ TARAFTA BOŞ KALIRSA TEMİZLE
//    // ============================================================
//    if (x < rcRow.right)
//    {
//        CRect rcEmpty(x, rcRow.top, rcRow.right, rcRow.bottom);
//
//        // Boşluğu satır arka planı ile doldur
//        dc.FillRect(rcEmpty, CBrush(rBk));
//
//        // Alt çizgi devam etsin
//        CPen pOld2 = dc.SelectObject(penGrid);
//        dc.MoveTo(rcEmpty.left, rcEmpty.bottom - 1);
//        dc.LineTo(rcEmpty.right, rcEmpty.bottom - 1);
//        dc.SelectObject(pOld2);
//    }
//}

int HitTestHeaderColumn(HTREEITEM hHeader, const CPoint& pt)
{
    TLV_Category* cat = FindCategoryByHeader(hHeader);
    if (!cat) return -1;

    CRect rcRow;
    if (!GetItemRect(hHeader, rcRow, FALSE))
        return -1;

    int x = rcRow.left;
    for (size_t i = 0; i < cat->columns.size(); ++i)
    {
        const auto& col = cat->columns[i];
        if (!col.visible) continue;

        int right = x + col.width;

        if (pt.x >= x && pt.x < right)
            return (int)i;

        x = right;
    }

    return -1;
}

HTREEITEM HitTestTreeItem(const CPoint& pt) {
            TVHITTESTINFO ht{};
            ht.pt = pt;
            return TreeView_HitTest(GetHwnd(), &ht);
        }

        int HitTestColumn(HTREEITEM hItem, const CPoint& pt) {
            TLV_Category* cat = GetCategoryForRow(hItem);
            if (!cat) return -1;

            CRect rcRow;
            GetItemRect(hItem, rcRow, FALSE);

            int x = rcRow.left;
            for (size_t i = 0; i < cat->columns.size(); ++i) {
                const auto& col = cat->columns[i];
                if (!col.visible) continue;

                int w = col.width;
                if (pt.x >= x && pt.x < x + w)
                    return (int)i;

                x += w;
            }
            return -1;
        }

        // ==========================================================
//  HEADER / CATEGORY YÖNETİMİ (Temel API)
// ==========================================================

// Header + Category tek seferde ekler.
//  - treeText      : TreeView'de görünen metin (ör: "Konut Portföyleri")
//  - categoryName  : İç mantıksal isim (ör: "HomeTbl")
//  - hParent       : Root veya üst header (varsayılan TVI_ROOT)
//  - hAfter        : Yerleştirileceği konum (varsayılan TVI_LAST)
//  - image         : İkon index (opsiyonel)
//  - selectedImage : Seçiliyken ikon index (opsiyonel, -1 ise image ile aynı)
//
        HTREEITEM InsertHeader(const CString& treeText,
            const CString& categoryName,
            HTREEITEM hParent = TVI_ROOT,
            HTREEITEM hAfter = TVI_LAST,
            int       image = 0,
            int       selectedImage = -1)
        {
            TVINSERTSTRUCT tvi{};
            tvi.hParent = hParent;
            tvi.hInsertAfter = hAfter;

            tvi.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
            tvi.item.pszText = const_cast<LPTSTR>((LPCTSTR)treeText);
            tvi.item.iImage = image;
            tvi.item.iSelectedImage = (selectedImage >= 0) ? selectedImage : image;

            HTREEITEM hHeader = (HTREEITEM)::SendMessage(GetHwnd(), TVM_INSERTITEM, 0, (LPARAM)&tvi);
            if (!hHeader)
                return nullptr;

            // Model tarafında kategori kaydı
            AddCategory(categoryName, hHeader);
            return hHeader;
        }

        // Verilen header'ı ve altındaki tüm satırları siler.
        //  - Hem TreeView item'larını hem de TLV_Row modelini temizler.
        bool DeleteHeader(HTREEITEM hHeader)
        {
            TLV_Category* cat = FindCategoryByHeader(hHeader);
            if (!cat || !hHeader)
                return false;

            // 1) Alt satırları TreeView'den ve modelden sil
            HTREEITEM hChild = GetChild(hHeader);
            while (hChild)
            {
                HTREEITEM hNext = GetNextSibling(hChild);

                auto itRow = m_rowByHandle.find(hChild);
                if (itRow != m_rowByHandle.end())
                    m_rowByHandle.erase(itRow);

                DeleteItem(hChild);
                hChild = hNext;
            }

            // 2) Category modelini m_categories listesinden çıkar
            auto itCat = std::find_if(m_categories.begin(), m_categories.end(),
                [cat](const std::unique_ptr<TLV_Category>& up) { return up.get() == cat; });

            if (itCat != m_categories.end())
                m_categories.erase(itCat);

            // 3) Header → Category map'ini temizle
            m_catByHeader.erase(hHeader);

            // 4) Header Tree item'ını sil
            DeleteItem(hHeader);

            Invalidate();
            return true;
        }

        // Sadece TreeView üzerindeki header metnini değiştirir
        // (kategori iç ismi aynı kalır).
        bool RenameHeaderText(HTREEITEM hHeader, const CString& newTreeText)
        {
            if (!hHeader)
                return false;

            SetItemText(hHeader, newTreeText);
            Invalidate();
            return true;
        }

        // Kategori iç mantıksal ismini değiştirir (ör: "HomeTbl" -> "HomeTbl_v2").
        // DİKKAT: Bu isim senin DB tarafındaki tablo eşleştirmelerinde de kullanılıyorsa
        // oraları da güncellemen gerekir.
        bool RenameHeaderCategory(HTREEITEM hHeader, const CString& newCategoryName)
        {
            TLV_Category* cat = FindCategoryByHeader(hHeader);
            if (!cat)
                return false;

            cat->name = newCategoryName;
            Invalidate();
            return true;
        }

        // Header item'ına bağlı kategori adını (name) döndürür.
        CString GetHeaderCategoryName(HTREEITEM hHeader) const
        {
            auto it = m_catByHeader.find(hHeader);
            if (it == m_catByHeader.end() || !(it->second))
                return CString();

            return it->second->name;
        }

        // Kategori adına göre header HTREEITEM'ını bulur.
        // Ör: "HomeTbl" ver → o kategoriye ait header'ı döndür.
        HTREEITEM FindHeaderByCategoryName(const CString& categoryName) const
        {
            for (const auto& upCat : m_categories)
            {
                const TLV_Category* cat = upCat.get();
                if (!cat)
                    continue;

                if (cat->name.CompareNoCase(categoryName) == 0)
                    return cat->hHeader;
            }
            return nullptr;
        }

        HCURSOR GetSizeWECursor()
        {
            if (!m_hCursorSizeWE)
                m_hCursorSizeWE = ::LoadCursor(nullptr, IDC_SIZEWE);
            return m_hCursorSizeWE;
        }

        /*****************************************************************************************/
                // Header üzerindeki kolon kenarını yakalayan hittest
        bool HitTestHeaderResize(const CPoint& ptClient, HTREEITEM& outHeader, int& outColIndex)
        {
            outHeader = nullptr;
            outColIndex = -1;

            HTREEITEM hItem = HitTestTreeItem(ptClient);
            if (!hItem || !IsHeaderItem(hItem))
                return false;

            TLV_Category* cat = FindCategoryByHeader(hItem);
            if (!cat)
                return false;

            CRect rcRow;
            if (!GetItemRect(hItem, rcRow, FALSE))
                return false;

            int x = rcRow.left;

            for (size_t i = 0; i < cat->columns.size(); ++i)
            {
                const auto& col = cat->columns[i];
                if (!col.visible)
                    continue;

                int right = x + col.width;

                // Kenardan +/- 3 piksel içinde misin?
                int dx = ptClient.x - right;
                if (dx < 0) dx = -dx;

                if (dx <= 3)
                {
                    outHeader = hItem;
                    outColIndex = static_cast<int>(i);
                    return true;
                }

                x = right;
            }

            return false;
        }
        // Verilen noktadan hücreyi bul: item + column
        // Data satırı değilse false döner.
        bool HitTestCell(const CPoint& ptClient, HTREEITEM& outItem, int& outCol)
        {
            outItem = HitTestTreeItem(ptClient);
            outCol = -1;

            if (!outItem)
                return false;

            // Header'a tıklama değilse, kolon hittest uygula
            if (!IsHeaderItem(outItem))
            {
                outCol = HitTestColumn(outItem, ptClient);
                return (outCol >= 0);
            }

            return false;
        }

        // Kolonu ve verilerini yeni pozisyona taşır
        bool MoveColumn(HTREEITEM hHeader, int oldIndex, int newIndex)
        {
            TLV_Category* cat = FindCategoryByHeader(hHeader);
            if (!cat) return false;

            int n = (int)cat->columns.size();
            if (oldIndex < 0 || oldIndex >= n || newIndex < 0 || newIndex >= n)
                return false;

            if (oldIndex == newIndex)
                return false;

            // Kolonları kaydır
            auto col = cat->columns[oldIndex];
            cat->columns.erase(cat->columns.begin() + oldIndex);
            cat->columns.insert(cat->columns.begin() + newIndex, col);

            // Satır verilerini kaydır
            for (auto& row : cat->rows)
            {
                if (row->cells.size() != (size_t)n)
                    continue;

                auto cell = row->cells[oldIndex];
                row->cells.erase(row->cells.begin() + oldIndex);
                row->cells.insert(row->cells.begin() + newIndex, cell);
            }

            return true;
        }


        // wxx_treelistview.h -> CTreeListView -> protected bölümüne ekleyin:

// ---------------------------------------------------------
//  🎨 PROFESYONEL KART GÖRÜNÜMÜ İÇİN YARDIMCILAR
// ---------------------------------------------------------

// Durum Rozeti Çizimi (Active/Passive vb.)

        // ---------------------------------------------------------
        //  🃏 ANA KART ÇİZİM FONKSİYONU (DrawCardItemProfessional)
        // ---------------------------------------------------------
        void DrawCardItemProfessional(CDC& dc, HTREEITEM hItem, const CRect& rcRow, LPNMTVCUSTOMDRAW pCD)
        {
            TLV_Row* row = GetRow(hItem);
            TLV_Category* cat = GetCategoryForRow(hItem);
            if (!row || !cat) return;

            bool sel = (pCD->nmcd.uItemState & CDIS_SELECTED) != 0;

            // Arkaplanı temizle (TreeView zemin rengi)
            dc.FillRect(rcRow, CBrush(m_theme.clrWnd));

            // 1. KART ALANI HESAPLAMA
            // Satırın kenarlarından boşluk bırakarak kartı ortala
            CRect rcCard = rcRow;
            rcCard.DeflateRect(10, 4, 10, 4); // Sağdan/soldan 10px, üstten/alttan 4px boşluk

            // 2. GÖLGE EFEKTİ (3D Derinlik)
            CRect rcShadow = rcCard;
            rcShadow.OffsetRect(2, 2); // 2px sağ-aşağı kaydır
            CBrush brShadow(RGB(220, 220, 220)); // Açık gri gölge
            dc.FillRect(rcShadow, brShadow);

            // 3. KART GÖVDESİ VE ÇERÇEVE
            // Seçiliyse açık mavi gradient, değilse beyaz-gri geçiş
            COLORREF clrTop = sel ? RGB(240, 248, 255) : RGB(255, 255, 255);
            COLORREF clrBot = sel ? RGB(220, 240, 255) : RGB(245, 245, 245);

            DrawGradient(dc, rcCard, clrTop, clrBot);

            // Kart Çerçevesi
            COLORREF clrBorder = sel ? RGB(51, 153, 255) : RGB(200, 200, 200);
            CBrush brBorder(clrBorder);
            dc.FrameRect(rcCard, brBorder);

            // Sol tarafa "Vurgu Çizgisi" (Accent Line) - Modern dokunuş
            CRect rcAccent = rcCard;
            rcAccent.right = rcAccent.left + 4; // 4px genişliğinde şerit
            rcAccent.DeflateRect(1, 1); // Çerçevenin içinde kalsın

            // Kategorisine göre renk değiştirebiliriz (Opsiyonel)
            CBrush brAccent(sel ? RGB(0, 120, 215) : RGB(255, 165, 0)); // Seçiliyse Mavi, değilse Turuncu
            dc.FillRect(rcAccent, brAccent);

            // 4. İÇERİK YERLEŞİMİ
            int leftPadding = 20; // Accent + boşluk
            int contentLeft = rcCard.left + leftPadding;

            // A) İKON ÇİZİMİ (Varsa)
            // TreeView'in ImageList'ini al
            HIMAGELIST hImgList = (HIMAGELIST)::SendMessage(GetHwnd(), TVM_GETIMAGELIST, TVSIL_NORMAL, 0);
            int imgW = 0, imgH = 0;

            TVITEMEX tvi = { 0 };
            tvi.mask = TVIF_IMAGE;
            tvi.hItem = hItem;
            ::SendMessage(GetHwnd(), TVM_GETITEM, 0, (LPARAM)&tvi);

            if (hImgList && tvi.iImage >= 0)
            {
                ImageList_GetIconSize(hImgList, &imgW, &imgH);
                // İkonu dikey ortala
                int iconTop = rcCard.top + (rcCard.Height() - imgH) / 2;
                ImageList_Draw(hImgList, tvi.iImage, dc, contentLeft, iconTop, ILD_TRANSPARENT);

                contentLeft += imgW + 10; // İkon genişliği + boşluk kadar kaydır
            }
            else
            {
                // İkon yoksa varsayılan kutu çiz (Örn: Sarı kutu)
                CRect rcIcon(contentLeft, rcCard.top + 10, contentLeft + 32, rcCard.top + 42);
                dc.FillRect(rcIcon, CBrush(RGB(241, 196, 15)));
                // İçine basit grid
                CPen p(PS_SOLID, 1, RGB(200, 150, 0));
                CPen oldP = dc.SelectObject(p);
                dc.MoveTo(rcIcon.left + 10, rcIcon.top); dc.LineTo(rcIcon.left + 10, rcIcon.bottom);
                dc.MoveTo(rcIcon.right - 10, rcIcon.top); dc.LineTo(rcIcon.right - 10, rcIcon.bottom);
                dc.MoveTo(rcIcon.left, rcIcon.top + 10); dc.LineTo(rcIcon.right, rcIcon.top + 10);
                dc.MoveTo(rcIcon.left, rcIcon.top + 20); dc.LineTo(rcIcon.right, rcIcon.top + 20);
                dc.SelectObject(oldP);

                contentLeft += 42;
            }

            // B) BAŞLIK (Ana Metin) - İlk Kolon
            CString title = GetItemText(hItem);

            // Font Hazırlığı (Başlık için Bold ve Büyük)
            LOGFONT lf;
            HFONT hFontCurrent = (HFONT)dc.GetCurrentFont();
            ::GetObject(hFontCurrent, sizeof(LOGFONT), &lf);

            lf.lfHeight = -14; // Biraz daha büyük
            lf.lfWeight = FW_BOLD;
            CFont fontTitle;
            fontTitle.CreateFontIndirect(lf);

            CRect rcTitle(contentLeft, rcCard.top + 8, rcCard.right - 80, rcCard.top + 28);

            dc.SetBkMode(TRANSPARENT);
            dc.SetTextColor(m_theme.clrText);

            CFont pOldFont = dc.SelectObject(fontTitle);
            dc.DrawText(title, -1, rcTitle, DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS);
            dc.SelectObject(pOldFont);

            // C) ALT BİLGİLER (Diğer Kolonlar)
            // 2. Kolondan itibaren diğer bilgileri başlığın altına yaz
            CString detailText;
            for (size_t i = 1; i < cat->columns.size(); ++i) // 0. kolon başlıktı, 1'den başla
            {
                if (i >= row->cells.size()) break;

                CString val = row->cells[i].text;
                if (val.IsEmpty()) continue;

                // Eğer değer "ACTIVE", "PASSIVE" gibi durum ise onu geç (Rozet olarak çizeceğiz)
                CString upperVal = val; upperVal.MakeUpper();
                if (upperVal == _T("ACTIVE") || upperVal == _T("PASSIVE") || upperVal == _T("SATILDI") || upperVal == _T("AKTİF") || upperVal == _T("PASİF"))
                    continue;

                if (!detailText.IsEmpty()) detailText += _T("  |  ");
                detailText += val;
            }

            // Detay Metni Rengi (Gri)
            dc.SetTextColor(RGB(100, 100, 100));
            CRect rcDetail(contentLeft, rcTitle.bottom + 2, rcCard.right - 10, rcCard.bottom - 4);
            dc.DrawText(detailText, -1, rcDetail, DT_LEFT | DT_TOP | DT_WORDBREAK | DT_END_ELLIPSIS);

            // D) DURUM ROZETİ (BADGE)
            // Veri içinde "ACTIVE", "PASSIVE" vb. ara
            CString statusText;
            for (const auto& cell : row->cells)
            {
                CString s = cell.text;
                s.MakeUpper();
                if (s == _T("ACTIVE") || s == _T("PASSIVE") || s == _T("SATILDI") || s == _T("AKTİF") || s == _T("PASİF") || s == _T("SOLD"))
                {
                    statusText = cell.text;
                    break;
                }
            }

            if (!statusText.IsEmpty())
            {
                // Rozet Boyutları
                CSize szText = dc.GetTextExtentPoint32W(statusText);
                int badgeW = szText.cx + 16;
                int badgeH = 20;

                CRect rcBadge;
                rcBadge.right = rcCard.right - 10;
                rcBadge.left = rcBadge.right - badgeW;
                rcBadge.top = rcCard.top + 10; // Sağ üst köşe
                rcBadge.bottom = rcBadge.top + badgeH;

                DrawStatusBadge(dc, rcBadge, statusText);
            }
        }

        // wxx_treelistview.h -> CTreeListView -> protected:
// =========================================================
//  wxx_treelistview.h -> CTreeListView -> protected:
//  DÜZELTİLMİŞ & STANDARTLARA UYGUN ÇİZİM FONKSİYONLARI
// =========================================================

// Durum Rozeti Çizimi (Active/Passive vb.)
        void DrawStatusBadge(CDC& dc, CRect rc, const CString& text)
        {
            // 1. RENK BELİRLEME
            COLORREF bgCol = RGB(180, 180, 180); // Varsayılan Gri
            COLORREF txtCol = RGB(255, 255, 255); // Beyaz yazı

            CString upperTxt = text;
            upperTxt.MakeUpper();

            // UNICODE UYUMLU KARŞILAŞTIRMA (_T makrosu eklendi)
            if (upperTxt.Find(_T("ACTIVE")) >= 0 || upperTxt.Find(_T("AKTİF")) >= 0)
                bgCol = RGB(46, 204, 113); // Zümrüt Yeşili
            else if (upperTxt.Find(_T("PASSIVE")) >= 0 || upperTxt.Find(_T("PASİF")) >= 0)
                bgCol = RGB(231, 76, 60);  // Kırmızı
            else if (upperTxt.Find(_T("SOLD")) >= 0 || upperTxt.Find(_T("SATILDI")) >= 0)
                bgCol = RGB(52, 152, 219); // Mavi

            // 2. ROZET ARKA PLANI
            CBrush br(bgCol);
            CPen pen(PS_NULL, 0, 0); // Çerçevesiz

            // SelectObject: Pointer döndürür, HGDIOBJ değil
            CBrush pOldBrush = dc.SelectObject(br);
            CPen pOldPen = dc.SelectObject(pen);

            dc.RoundRect(rc.left, rc.top, rc.right, rc.bottom, 6, 6);

            // Nesneleri geri yükle
            dc.SelectObject(pOldBrush);
            dc.SelectObject(pOldPen);

            // 3. METİN ÇİZİMİ
            dc.SetBkMode(TRANSPARENT);
            dc.SetTextColor(txtCol);

            // Font oluşturma (Win32++ Standartlarına Uygun)
            LOGFONT lf = { 0 };
            // Mevcut fontu al (HGDIOBJ döner, CFont nesnesine gerek yok burada)
            HFONT hCurFont = (HFONT)::GetCurrentObject(dc.GetHDC(), OBJ_FONT);
            ::GetObject(hCurFont, sizeof(LOGFONT), &lf);

            lf.lfHeight = -10; // Küçük font
            lf.lfWeight = FW_BOLD;

            CFont fontBadge;
            // DÜZELTME: CreateFontIndirect pointer (&lf) değil, referans (lf) isteyebilir
            // Ancak Win32++ versiyonuna göre &lf genelde çalışır. 
            // Hata "tagLOGFONTW *" diyorsa referans verelim:
            fontBadge.CreateFontIndirect(lf);

            CFont pOldFont = dc.SelectObject(fontBadge);
            dc.DrawText(text, -1, rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            dc.SelectObject(pOldFont);
        }
        // wxx_treelistview.h -> CTreeListView -> protected:

// Yardımcı: Unicode uyumlu güvenli font oluşturucu
        void CreateSafeFont(CFont& font, LONG height, LONG weight, const TCHAR* faceName)
        {
            LOGFONT lf = { 0 };
            ::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
            lf.lfHeight = height;
            lf.lfWeight = weight;
            _tcscpy_s(lf.lfFaceName, 32, faceName);
            font.CreateFontIndirect(lf);
        }

        // Yeni Tasarım: "Icon Box" Property Card
// wxx_treelistview.h -> CTreeListView -> protected:

// 1. MODERN HEADER ÇİZİMİ (BÖLÜM BAŞLIĞI)
// wxx_treelistview.h -> CTreeListView -> protected:

// wxx_treelistview.h -> CTreeListView -> protected:

        void DrawGroupHeader(CDC& dc, HTREEITEM hItem, const CRect& rcRow)
        {
            // 1. ZEMİN (Hafif Gri)
            CBrush brBg(RGB(245, 247, 250));
            dc.FillRect(rcRow, brBg);

            // 2. AYIRICI ÇİZGİ
            CPen penLine(PS_SOLID, 1, RGB(220, 220, 220));
            CPen pOldPen = dc.SelectObject(penLine);
            dc.MoveTo(rcRow.left, rcRow.bottom - 1);
            dc.LineTo(rcRow.right, rcRow.bottom - 1);
            dc.SelectObject(pOldPen);

            // 3. BAŞLIK
            CString title = GetItemText(hItem);
            CFont fontHdr;
            CreateSafeFont(fontHdr, -14, FW_BOLD, _T("Segoe UI Semibold"));

            dc.SetBkMode(TRANSPARENT);
            dc.SetTextColor(RGB(60, 70, 80)); // Slate Gray

            CFont pOldFont = dc.SelectObject(fontHdr);
            CRect rcTxt = rcRow;
            rcTxt.DeflateRect(12, 0);

            // İkon
            HIMAGELIST hImgList = (HIMAGELIST)::SendMessage(GetHwnd(), TVM_GETIMAGELIST, TVSIL_NORMAL, 0);
            int imgW = 0;
            if (hImgList) {
                int iImage, iSelImage;
                GetItemImage(hItem, iImage, iSelImage);
                ImageList_Draw(hImgList, iImage, dc, rcTxt.left, rcRow.top + (rcRow.Height() - 16) / 2, ILD_TRANSPARENT);
                imgW = 24;
            }

            rcTxt.left += imgW;
            dc.DrawText(title, -1, rcTxt, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            dc.SelectObject(pOldFont);
        }        // 2. COMPACT MÜLK KARTI (Daha Sıkı Görünüm)


// wxx_treelistview.h -> CTreeListView -> protected:

// 2. COMPACT MÜLK KARTI (MODERN ETİKETLİ VERSİYON)
        void DrawPropertyCard(CDC& dc, HTREEITEM hItem, const CRect& rcRow, LPNMTVCUSTOMDRAW pCD)
        {
            TLV_Row* row = GetRow(hItem);
            if (!row) return;

            bool sel = (pCD->nmcd.uItemState & CDIS_SELECTED) != 0;

            // 1. ZEMİN ve KART ALANI
            dc.FillRect(rcRow, CBrush(m_theme.clrWnd));

            CRect rcCard = rcRow;
            rcCard.DeflateRect(2, 1, 2, 1);

            // 2. RENK VE TİP ANALİZİ
            CString fullText = GetItemText(hItem);
            for (const auto& cell : row->cells) fullText += _T(" ") + cell.text;
            fullText.MakeUpper();

            COLORREF clrBoxBg;
            int iconIndex = TLV_ICON_HOME;

            // Mülk tipine göre renk belirle
            if (fullText.Find(_T("ARSA")) >= 0) {
                clrBoxBg = RGB(41, 128, 185);   // Mavi
                iconIndex = TLV_ICON_LAND;
            }
            else if (fullText.Find(_T("TARLA")) >= 0 || fullText.Find(_T("BAĞ")) >= 0) {
                clrBoxBg = RGB(39, 174, 96);    // Yeşil
                iconIndex = TLV_ICON_FIELD;
            }
            else if (fullText.Find(_T("VILLA")) >= 0) {
                clrBoxBg = RGB(142, 68, 173);   // Mor
                iconIndex = TLV_ICON_VILLA;
            }
            else if (fullText.Find(_T("TICARI")) >= 0 || fullText.Find(_T("DÜKKAN")) >= 0) {
                clrBoxBg = RGB(192, 57, 43);    // Kırmızı
                iconIndex = TLV_ICON_COMMERCIAL;
            }
            else {
                clrBoxBg = RGB(52, 73, 94);     // Koyu Gri
                iconIndex = TLV_ICON_HOME;
            }

            // 3. KART GÖVDESİ
            COLORREF clrCardBg = sel ? RGB(235, 245, 255) : RGB(255, 255, 255);
            CBrush brCard(clrCardBg);
            dc.FillRect(rcCard, brCard);

            // Alt Çizgi
            CPen penLine(PS_SOLID, 1, sel ? RGB(100, 160, 220) : RGB(230, 230, 230));
            CPen pOldPen = dc.SelectObject(penLine);
            dc.MoveTo(rcCard.left, rcCard.bottom - 1);
            dc.LineTo(rcCard.right, rcCard.bottom - 1);

            // Sol Seçim Çizgisi
            if (sel) {
                CBrush brSel(RGB(0, 120, 215));
                CRect rcSelIndicator = rcCard;
                rcSelIndicator.right = rcSelIndicator.left + 4;
                dc.FillRect(rcSelIndicator, brSel);
            }
            dc.SelectObject(pOldPen);

            // 4. İKON KUTUSU (Sol Taraf)
            CRect rcBox;
            rcBox.left = rcCard.left + 10;
            rcBox.top = rcCard.top + (rcCard.Height() - 44) / 2;
            rcBox.right = rcBox.left + 44;
            rcBox.bottom = rcBox.top + 44;

            CBrush brBox(clrBoxBg);
            CBrush pOldBrush = dc.SelectObject(brBox);
            dc.SelectStockObject(NULL_PEN);
            dc.RoundRect(rcBox.left, rcBox.top, rcBox.right, rcBox.bottom, 5, 5);
            dc.SelectObject(pOldBrush);

            HIMAGELIST hImgList = (HIMAGELIST)::SendMessage(GetHwnd(), TVM_GETIMAGELIST, TVSIL_NORMAL, 0);
            if (hImgList) {
                int iconX = rcBox.left + (44 - 32) / 2;
                int iconY = rcBox.top + (44 - 32) / 2;
                ImageList_Draw(hImgList, iconIndex, dc, iconX, iconY, ILD_TRANSPARENT);
            }

            // 5. İÇERİK ALANI
            CRect rcContent = rcCard;
            rcContent.left = rcBox.right + 12;
            rcContent.right -= 5;
            rcContent.top += 6;

            // A) BAŞLIK (İlan Başlığı / Kod)
            CString title = GetItemText(hItem); // Genelde ilk kolon Mülk Kodu oluyor
            // İstersen 2. kolonu başlık yapabilirsin: row->cells[1].text

            CFont fontTitle;
            CreateSafeFont(fontTitle, -14, FW_BOLD, _T("Segoe UI"));

            dc.SetBkMode(TRANSPARENT);
            dc.SetTextColor(RGB(40, 40, 40));

            CFont pOldFont = dc.SelectObject(fontTitle);
            dc.DrawText(title, -1, CRect(rcContent.left, rcContent.top, rcContent.right - 150, rcContent.top + 20), DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS);
            dc.SelectObject(pOldFont);

            // B) DETAYLAR (ETİKET SİSTEMİ)
            int tagX = rcContent.left;
            int tagY = rcContent.top + 24;

            // Örnek etiket renkleri (Açık Mavi Zemin, Koyu Mavi Yazı)
            COLORREF tagBg = RGB(236, 240, 241);
            COLORREF tagTx = RGB(44, 62, 80);

            // Tüm kolonları gez, anlamlı olanları etiket yap
            // ÖNEMLİ: Burada kolon indexlerine göre (Fiyat hariç) filtreleme yapabilirsin.
            // Şimdilik basitçe ilk 5 anlamlı veriyi alıyoruz.
            int count = 0;
            CString priceText = _T(""); // Fiyatı ayrıca saklayacağız

            for (size_t i = 1; i < row->cells.size(); i++) {
                CString val = row->cells[i].text;
                if (val.IsEmpty() || val == _T("0") || val == _T("Yok") || val == _T("Belirtilmemiş")) continue;

                CString uVal = val; uVal.MakeUpper();

                // ID'leri atla
                bool startsWithDigit = (uVal.GetLength() > 0 && uVal[0] >= _T('0') && uVal[0] <= _T('9'));
                if (uVal.GetLength() > 15 && startsWithDigit) continue;

                // Durumları atla (Rozet olacak)
                if (uVal == _T("ACTIVE") || uVal == _T("PASSIVE") || uVal == _T("SATILDI")) continue;

                // Fiyatı Bul (Basit kontrol: içinde TL, USD veya çok büyük sayı varsa)
                // Senin verinde "3675000" gibi duruyor, yanında para birimi kolonu olabilir.
                // Bu örnekte varsayım: Eğer kolon başlığı "Fiyat" ise veya değeri para formatındaysa
                // (Bunu kendi kolon yapına göre özelleştirmen gerekebilir)
                /* Örnek: if (cat->columns[i].title == _T("Fiyat")) { priceText = val; continue; }
                */

                // Şimdilik ilk 4 veriyi etiket yapalım
                if (count < 4) {
                    tagX += DrawTag(dc, tagX, tagY, val, tagBg, tagTx);
                    count++;
                }
            }

            // C) SAĞ TARAFTA FİYAT VE DURUM
            // Fiyatı sağa yaslı büyük yaz
            // Not: Gerçek verinden fiyatı çekmelisin. Şimdilik örnek veri varsa onu kullanır.
            // row->cells[X] -> Fiyat kolonu indexin neyse onu yaz.
            // Örnek: CString f = row->cells[10].text + _T(" TL"); 

            // Sağ taraftaki alanı belirle
            CRect rcRight = rcCard;
            rcRight.left = rcRight.right - 140; // Sağdan 140px ayır
            rcRight.right -= 10;

            // 1. FİYAT (En önemli veri)
            // Fiyatı bulmaya çalışalım (Basitçe uzun sayı içeren bir kolon)
            if (priceText.IsEmpty()) {
                for (size_t i = 1; i < row->cells.size(); i++) {
                    CString v = row->cells[i].text;
                    // Sadece rakamdan oluşuyorsa ve uzunsa fiyattır diye tahmin et
                    if (v.GetLength() > 5 && _ttoi(v) > 0 && v.FindOneOf(_T("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz")) == -1) {
                        priceText = v + _T(" TL"); // Varsayım
                        break;
                    }
                }
            }

            if (!priceText.IsEmpty()) {
                CFont fontPrice;
                CreateSafeFont(fontPrice, -16, FW_BOLD, _T("Segoe UI"));
                dc.SetTextColor(RGB(39, 174, 96)); // Yeşil (Para)

                CFont pOldPriceFont = dc.SelectObject(fontPrice);
                dc.DrawText(priceText, -1, CRect(rcRight.left, rcCard.top + 8, rcRight.right, rcCard.top + 30), DT_RIGHT | DT_TOP | DT_SINGLELINE);
                dc.SelectObject(pOldPriceFont);
            }

            // 2. DURUM ROZETİ (Fiyatın altına)
            CString statusText;
            for (const auto& cell : row->cells) {
                CString s = cell.text; CString uS = s; uS.MakeUpper();
                if (uS == _T("ACTIVE") || uS == _T("PASSIVE") || uS == _T("SATILDI")) {
                    statusText = s; break;
                }
            }

            if (!statusText.IsEmpty()) {
                CSize sz = dc.GetTextExtentPoint32W(statusText);
                CRect rcBadge;
                rcBadge.right = rcCard.right - 10;
                rcBadge.left = rcBadge.right - (sz.cx + 16);
                rcBadge.top = rcCard.top + 32; // Fiyatın altı
                rcBadge.bottom = rcBadge.top + 20;
                DrawStatusBadge(dc, rcBadge, statusText);
            }
        }






        // wxx_treelistview.h -> CTreeListView -> protected:

// Yardımcı: Yuvarlak Köşeli Etiket Çizimi (Tag/Chip)
// Dönüş değeri: Bir sonraki etiketin başlayacağı X koordinatı
        int DrawTag(CDC& dc, int x, int y, const CString& text, COLORREF bgCol, COLORREF txtCol)
        {
            // Font oluştur (Biraz küçük ve kalın)
            CFont fontTag;
            CreateSafeFont(fontTag, -11, FW_BOLD, _T("Segoe UI"));

            // Metin boyutunu ölç
            CSize sz = dc.GetTextExtentPoint32W(text);
            int w = sz.cx + 16; // Yanlardan 8'er piksel boşluk
            int h = 20;         // Yükseklik sabit

            CRect rcTag(x, y, x + w, y + h);

            // Etiket Arka Planı (Yuvarlak Köşeli)
            CBrush br(bgCol);
            CPen pen(PS_NULL, 0, 0); // Çerçevesiz

            CBrush pOldBrush = dc.SelectObject(br);
            CPen pOldPen = dc.SelectObject(pen);

            dc.RoundRect(rcTag.left, rcTag.top, rcTag.right, rcTag.bottom, 6, 6);

            dc.SelectObject(pOldBrush);
            dc.SelectObject(pOldPen);

            // Metni Çiz
            dc.SetBkMode(TRANSPARENT);
            dc.SetTextColor(txtCol);

            CFont pOldFont = dc.SelectObject(fontTag);
            dc.DrawText(text, -1, rcTag, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            dc.SelectObject(pOldFont);

            return w + 6; // Etiket genişliği + 6 piksel boşluk döndür
        }










        // Sürüklenen kolonun görsel kopyasını oluşturur
        CImageList* CreateDragImage(HTREEITEM hHeader, int colIndex)
        {
            TLV_Category* cat = FindCategoryByHeader(hHeader);
            if (!cat) return nullptr;

            // Kolonun boyutlarını ve özelliklerini al
            PropertyColumnInfo& col = cat->columns[colIndex];
            int width = col.width;
            int height = m_theme.headerHeight;

            // Ekrana uyumlu bir hafıza DC'si oluştur
            CClientDC dcScreen(*this);
            CMemDC dcMem(dcScreen);

            // Bitmap oluştur
            CBitmap bitmap;
            bitmap.CreateCompatibleBitmap(dcScreen, width, height);
            dcMem.SelectObject(bitmap);

            // --- ÇİZİM İŞLEMİ (DrawHeaderRow ile aynı stilde) ---
            CRect rc(0, 0, width, height);

            // 1. Arka Plan Gradient (3D)
            COLORREF colTop = m_theme.dark ? RGB(60, 60, 65) : RGB(250, 250, 252);
            COLORREF colBot = m_theme.dark ? RGB(45, 45, 50) : RGB(225, 226, 230);
            DrawGradient(dcMem, rc, colTop, colBot);

            // 2. Çerçeve (Highlight/Shadow)
            dcMem.FillRect(CRect(0, 0, width, 1), CBrush(RGB(255, 255, 255))); // Üst
            dcMem.FillRect(CRect(0, height - 1, width, height), CBrush(RGB(172, 172, 172))); // Alt

            // Yan çizgiler (Daha belirgin olsun diye tam çerçeve)
            dcMem.FillRect(CRect(0, 0, 1, height), CBrush(RGB(200, 200, 200))); // Sol
            dcMem.FillRect(CRect(width - 1, 0, width, height), CBrush(RGB(160, 160, 160))); // Sağ

            // 3. Metin
            dcMem.SetBkMode(TRANSPARENT);
            dcMem.SetTextColor(RGB(60, 60, 60));

            CRect rcTxt = rc;
            rcTxt.DeflateRect(4, 0);

            UINT align = DT_LEFT;
            if (col.align == DT_CENTER) align = DT_CENTER;
            else if (col.align == DT_RIGHT) align = DT_RIGHT;

            dcMem.DrawText(col.title, -1, rcTxt, align | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

            // --- IMAGELIST OLUŞTUR ---
            CImageList* pIL = new CImageList();
            // ILC_COLOR32: Yüksek kalite renk, ILC_MASK: Şeffaflık desteği
            pIL->Create(width, height, ILC_COLOR32 | ILC_MASK, 0, 1);
            pIL->Add(bitmap, CLR_NONE); // Maske rengi yok, doğrudan ekle

            return pIL;
        }

    };
}

#endif // _WIN32XX_TREELISTVIEW_H_
