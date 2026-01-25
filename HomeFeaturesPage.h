#pragma once

#include "stdafx.h"
#include <map>
#include <set>
#include <vector>

class CHomeFeaturesPage : public CWnd
{
public:
    enum class PageKind
    {
        Features1,
        Features2
    };

    CHomeFeaturesPage() = default;
    virtual ~CHomeFeaturesPage() override = default;

    BOOL Create(HWND hParent, const RECT& rc, UINT id, PageKind kind);

    void LoadFromMap(const std::map<CString, CString>& record);
    void SaveToMap(std::map<CString, CString>& record) const;

protected:
    virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
    struct CheckItem
    {
        CString groupKey;
        CString text;
        HWND    hWnd = nullptr;
    };

    struct GroupDef
    {
        CString title;
        CString key;
        std::vector<CString> items;
        int columns = 3; // isteðe baðlý "base" kolon
    };

private:
    PageKind m_kind = PageKind::Features1;

    std::vector<GroupDef>  m_groups;
    std::vector<CheckItem> m_checks;

    // Scroll state
    int m_scrollY = 0;
    int m_contentHeight = 0;

    // Layout güvenliði için (GW_CHILD ile gezme yok!)
    std::vector<HWND> m_titles;          // group title static'leri
    std::vector<int>  m_groupStartIndex; // group -> m_checks baþlangýcý

    // Font/cache
    HFONT m_hFont = nullptr;

    // ---- core ----
    void BuildGroups();
    void CreateControls();
    void LayoutControls(int cx, int cy);
    void UpdateScrollBar(int cx, int cy);

    // scrolling helpers
    void ClampScroll(int cy);
    void ScrollTo(int newPos, int cy, bool redraw);

    // check helpers
    std::set<CString> GetCheckedForKey(const CString& key) const;
    void SetCheckedForKey(const CString& key, const std::set<CString>& values);
};
