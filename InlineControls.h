
#pragma once

#ifndef INLINECONTROLS_H
#define INLINECONTROLS_H

#include "stdafx.h"
#include "resource.h"



class CListCustomerView;


// ---- CInlineEdit ------------------------------------------------
class CInlineEdit : public CEdit
{
public:
    CListCustomerView* m_pParent = nullptr;
    LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam) override; // sadece prototip
};

// ---- CInlineCombo -----------------------------------------------
class CInlineCombo : public CComboBox
{
public:
    CListCustomerView* m_pParent = nullptr;
    LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam) override; // sadece prototip
};



#endif // INLINECONTROLS_H