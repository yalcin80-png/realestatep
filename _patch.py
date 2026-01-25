from pathlib import Path
import re
p=Path('/mnt/data/work/Files.cpp')
text=p.read_text(encoding='utf-8', errors='ignore')
# Replace dblclk function
pat=re.compile(r"LRESULT\s+CListCustomerView::HandleLButtonDblClk\(LPARAM\s+lparam\)\s*\{.*?\n\}\n", re.S)
new_dblclk=r'''LRESULT CListCustomerView::HandleLButtonDblClk(LPARAM lparam)
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
            // Ensure selection is stable
            DeleteAllSelections();
            ListView_SetItemState(GetHwnd(), nItem, LVIS_SELECTED | LVIS_FOCUSED,
                LVIS_SELECTED | LVIS_FOCUSED);

            const Customer_cstr cust = GetSelectedCustomerData();
            if (!cust.Cari_Kod.IsEmpty())
            {
                OpenCustomerPropertiesInTreeList(cust.Cari_Kod);
                return 0;
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
'''
if not pat.search(text):
    raise SystemExit('dblclk function not found')
text=pat.sub(new_dblclk, text, count=1)

# Insert OpenCustomerPropertiesInTreeList implementation after DeleteAllSelections()
insert_pat=re.compile(r"void\s+CListCustomerView::DeleteAllSelections\(\)\s*\{.*?\n\}\n", re.S)
if not insert_pat.search(text):
    raise SystemExit('DeleteAllSelections not found')
impl=r'''\g<0>

/* Duran YALCIN 17/01/2026
Fonksiyon Adi.: CListCustomerView::OpenCustomerPropertiesInTreeList
Aciklama :
  Loads the selected customer\'s properties into the application\'s TreeListView.
  The TreeListView keeps its own "category header + rows" model and action buttons.
*/
void CListCustomerView::OpenCustomerPropertiesInTreeList(const CString& cariKod)
{
    if (cariKod.IsEmpty())
        return;

    CRibbonFrameApp* pApp = GetContainerApp();
    if (!pApp) return;

    CMainFrame& mf = pApp->GetMainFrame();

    // Ensure dockers are visible (Split UI). If you later add a toggle button,
    // MainFrame can decide whether to hide the left docker and show only the TreeList.
    mf.ShowAllDockers();

    CMyTreeListView* pView = GetTreeListView();
    if (!pView || !pView->IsWindow())
        return;

    auto& db = DatabaseManager::GetInstance();

    // ---- Reset view + model ----
    TreeView_DeleteAllItems(pView->GetHwnd());
    pView->m_categories.clear();
    pView->m_catByHeader.clear();
    pView->m_rowByHandle.clear();

    // Optional: re-add action buttons if needed (keep what derived view wants)
    // (We do not touch the buttons list here, because CMyTreeListView may manage it.)

    // Helper: add a category and fill rows for a record type.
    auto addCategoryHeader = [&](const CString& tableName,
                                 const CString& displayName,
                                 int iconIndex,
                                 int count) -> HTREEITEM
    {
        CString hdr;
        hdr.Format(_T("%s (%d)"), displayName.GetString(), count);
        HTREEITEM hHeader = pView->InsertItem(hdr, iconIndex, TVI_ROOT, TVI_LAST);
        pView->AddCategory(tableName, hdr, hHeader, iconIndex);
        return hHeader;
    };

    auto setupColumnsFromDefinition = [&](HTREEITEM hHeader, const CString& tableName)
    {
        TLV_Category* cat = pView->FindCategoryByHeader(hHeader);
        if (!cat) return;

        // Take the table definition from the derived view (central place).
        std::vector<Win32xx::PropertyColumnInfo> defs = pView->GetTableDefinition(tableName);

        // If no defs, fallback to a minimal layout.
        if (defs.empty())
        {
            pView->AddColumn(cat, _T("Code"), _T("Kod"), 140);
            pView->AddColumn(cat, _T("Title"), _T("Baslik"), 320);
            return;
        }

        // Professional default: keep it compact (max 6 visible columns).
        int added = 0;
        for (const auto& d : defs)
        {
            if (!d.visible) continue;
            pView->AddColumn(cat, d.key, d.title, d.width, d.align, true, d.editType);
            if (++added >= 6) break;
        }

        if (cat->columns.empty())
        {
            // Fallback safety
            pView->AddColumn(cat, _T("Code"), _T("Kod"), 140);
            pView->AddColumn(cat, _T("Title"), _T("Baslik"), 320);
        }
    };

    auto fillRowFromMap = [&](HTREEITEM hHeader, HTREEITEM hRow,
                              const CString& tableName,
                              const CString& codeField,
                              const CString& codeValue)
    {
        TLV_Category* cat = pView->FindCategoryByHeader(hHeader);
        if (!cat) return;

        std::map<CString, CString> m = db.FetchRecordMap(tableName, codeField, codeValue);

        for (int col = 0; col < (int)cat->columns.size(); ++col)
        {
            const CString& key = cat->columns[col].key;
            auto it = m.find(key);
            if (it != m.end())
                pView->SetSubItemText(hRow, col, it->second);
        }
    };

    // ---- HOME ----
    {
        const auto records = db.GetHomesByCariKod(cariKod);
        if (!records.empty())
        {
            HTREEITEM hHeader = addCategoryHeader(PropertyTrait<Home_cstr>::GetTableName(), _T("Ev"), TLV_ICON_HOME, (int)records.size());
            setupColumnsFromDefinition(hHeader, PropertyTrait<Home_cstr>::GetTableName());

            for (const auto& r : records)
            {
                const CString code = PropertyTrait<Home_cstr>::GetCodeValue(r);
                HTREEITEM hRow = pView->InsertDataRow(hHeader, code, code);
                if (hRow)
                    fillRowFromMap(hHeader, hRow,
                                   PropertyTrait<Home_cstr>::GetTableName(),
                                   PropertyTrait<Home_cstr>::GetCodeFieldName(),
                                   code);
            }
            pView->Expand(hHeader, TVE_EXPAND);
        }
    }

    // ---- LAND ----
    {
        const auto records = db.GetLandsByCariKod(cariKod);
        if (!records.empty())
        {
            HTREEITEM hHeader = addCategoryHeader(PropertyTrait<Land_cstr>::GetTableName(), _T("Arsa"), TLV_ICON_LAND, (int)records.size());
            setupColumnsFromDefinition(hHeader, PropertyTrait<Land_cstr>::GetTableName());

            for (const auto& r : records)
            {
                const CString code = PropertyTrait<Land_cstr>::GetCodeValue(r);
                HTREEITEM hRow = pView->InsertDataRow(hHeader, code, code);
                if (hRow)
                    fillRowFromMap(hHeader, hRow,
                                   PropertyTrait<Land_cstr>::GetTableName(),
                                   PropertyTrait<Land_cstr>::GetCodeFieldName(),
                                   code);
            }
            pView->Expand(hHeader, TVE_EXPAND);
        }
    }

    // ---- FIELD ----
    {
        const auto records = db.GetFieldsByCariKod(cariKod);
        if (!records.empty())
        {
            HTREEITEM hHeader = addCategoryHeader(PropertyTrait<Field_cstr>::GetTableName(), _T("Tarla"), TLV_ICON_FIELD, (int)records.size());
            setupColumnsFromDefinition(hHeader, PropertyTrait<Field_cstr>::GetTableName());

            for (const auto& r : records)
            {
                const CString code = PropertyTrait<Field_cstr>::GetCodeValue(r);
                HTREEITEM hRow = pView->InsertDataRow(hHeader, code, code);
                if (hRow)
                    fillRowFromMap(hHeader, hRow,
                                   PropertyTrait<Field_cstr>::GetTableName(),
                                   PropertyTrait<Field_cstr>::GetCodeFieldName(),
                                   code);
            }
            pView->Expand(hHeader, TVE_EXPAND);
        }
    }

    // ---- VINEYARD ----
    {
        const auto records = db.GetVineyardsByCariKod(cariKod);
        if (!records.empty())
        {
            HTREEITEM hHeader = addCategoryHeader(PropertyTrait<Vineyard_cstr>::GetTableName(), _T("Bag / Bahce"), TLV_ICON_VINEYARD, (int)records.size());
            setupColumnsFromDefinition(hHeader, PropertyTrait<Vineyard_cstr>::GetTableName());

            for (const auto& r : records)
            {
                const CString code = PropertyTrait<Vineyard_cstr>::GetCodeValue(r);
                HTREEITEM hRow = pView->InsertDataRow(hHeader, code, code);
                if (hRow)
                    fillRowFromMap(hHeader, hRow,
                                   PropertyTrait<Vineyard_cstr>::GetTableName(),
                                   PropertyTrait<Vineyard_cstr>::GetCodeFieldName(),
                                   code);
            }
            pView->Expand(hHeader, TVE_EXPAND);
        }
    }

    // ---- VILLA ----
    {
        const auto records = db.GetVillasByCariKod(cariKod);
        if (!records.empty())
        {
            HTREEITEM hHeader = addCategoryHeader(PropertyTrait<Villa_cstr>::GetTableName(), _T("Villa"), TLV_ICON_VILLA, (int)records.size());
            setupColumnsFromDefinition(hHeader, PropertyTrait<Villa_cstr>::GetTableName());

            for (const auto& r : records)
            {
                const CString code = PropertyTrait<Villa_cstr>::GetCodeValue(r);
                HTREEITEM hRow = pView->InsertDataRow(hHeader, code, code);
                if (hRow)
                    fillRowFromMap(hHeader, hRow,
                                   PropertyTrait<Villa_cstr>::GetTableName(),
                                   PropertyTrait<Villa_cstr>::GetCodeFieldName(),
                                   code);
            }
            pView->Expand(hHeader, TVE_EXPAND);
        }
    }

    // ---- COMMERCIAL ----
    {
        const auto records = db.GetCommercialsByCariKod(cariKod);
        if (!records.empty())
        {
            HTREEITEM hHeader = addCategoryHeader(PropertyTrait<Commercial_cstr>::GetTableName(), _T("Ticari"), TLV_ICON_COMMERCIAL, (int)records.size());
            setupColumnsFromDefinition(hHeader, PropertyTrait<Commercial_cstr>::GetTableName());

            for (const auto& r : records)
            {
                const CString code = PropertyTrait<Commercial_cstr>::GetCodeValue(r);
                HTREEITEM hRow = pView->InsertDataRow(hHeader, code, code);
                if (hRow)
                    fillRowFromMap(hHeader, hRow,
                                   PropertyTrait<Commercial_cstr>::GetTableName(),
                                   PropertyTrait<Commercial_cstr>::GetCodeFieldName(),
                                   code);
            }
            pView->Expand(hHeader, TVE_EXPAND);
        }
    }

    // Focus TreeListView for keyboard navigation.
    pView->SetFocus();
}
'''
text=insert_pat.sub(impl, text, count=1)

p.write_text(text, encoding='utf-8')
print('patched Files.cpp')
