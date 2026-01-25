#pragma once
#ifndef OFFER_LIST_DLG_H
#define OFFER_LIST_DLG_H

#include "stdafx.h"
#include "resource.h"

class COfferListDlg : public CDialog
{
public:
    // Hangi mülkün teklifleri? (PropertyID)
    COfferListDlg(const CString& propID);
    virtual ~COfferListDlg() {}

protected:
    virtual BOOL OnInitDialog() override;
    
    void SetupColumns();
    void LoadData();

private:
    CListView m_list;
    CString m_propertyID;
};

#endif