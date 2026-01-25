
/* Duran YALCIN 17/01/2026
Fonksiyon Adi.: CListCustomerView::CanGoBack
Aciklama : Geri seviye var mi.
*/
bool CListCustomerView::CanGoBack() const
{
    return !m_backStack.empty();
}

static int GetTopIndexLV(HWND h)
{
    return (int)::SendMessage(h, LVM_GETTOPINDEX, 0, 0);
}

static int FindRowByColText(HWND h, int col, const CString& key)
{
    const int count = ListView_GetItemCount(h);
    for (int i = 0; i < count; ++i)
    {
        wchar_t buf[512] = {0};
        LVITEMW it{};
        it.iItem = i;
        it.iSubItem = col;
        it.mask = LVIF_TEXT;
        it.pszText = buf;
        it.cchTextMax = 511;
        ListView_GetItem(h, &it);
        if (CString(buf) == key)
            return i;
    }
    return -1;
}

/* Duran YALCIN 17/01/2026
Fonksiyon Adi.: CListCustomerView::GoBack
Aciklama : Geri seviye.
*/
void CListCustomerView::GoBack()
{
    if (m_backStack.empty())
        return;

    NavSnapshot snap = m_backStack.back();
    m_backStack.pop_back();

    m_navLevel = snap.level;
    m_navCariKod = snap.cariKod;
    m_navCategoryId = snap.categoryId;

    m_currentFilter = snap.filterLetter;
    m_currentStatus = snap.statusId;

    // Rebuild view
    if (snap.level == NavLevel::Customers)
    {
        m_viewMode = VIEW_MODE_CARD;
        ShowCustomersRoot();
        // restore scroll + selection by cari kod
        if (!snap.selectedKey.IsEmpty())
        {
            int idx = FindRowByColText(GetHwnd(), 0, snap.selectedKey);
            if (idx >= 0)
            {
                ListView_SetItemState(GetHwnd(), idx, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
                ListView_EnsureVisible(GetHwnd(), idx, FALSE);
            }
        }
        if (snap.topIndex >= 0)
            ListView_EnsureVisible(GetHwnd(), snap.topIndex, FALSE);
    }
    else if (snap.level == NavLevel::Categories)
    {
        m_viewMode = VIEW_MODE_REPORT;
        ShowCustomerCategoryHub(m_navCariKod);
    }
    else
    {
        m_viewMode = VIEW_MODE_REPORT;
        ShowCustomerPropertiesList(m_navCariKod, m_navCategoryId);
    }

    Invalidate();
}

