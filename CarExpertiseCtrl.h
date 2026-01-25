//#pragma once
//#include "wxx_wincore.h"
//#include "dataIsMe.h"
//#include <map>
//
//
//
//
//
//
//// Araba Parçaları için ID'ler
//enum CarPartID {
//    PART_NONE = 0,
//    PART_KAPUT,
//    PART_TAVAN,
//    PART_BAGAJ,
//    PART_SOL_ON_CAMURLUK,
//    PART_SOL_ON_KAPI,
//    PART_SOL_ARKA_KAPI,
//    PART_SOL_ARKA_CAMURLUK,
//    PART_SAG_ON_CAMURLUK,
//    PART_SAG_ON_KAPI,
//    PART_SAG_ARKA_KAPI,
//    PART_SAG_ARKA_CAMURLUK,
//    PART_ON_TAMPON, // Plastik aksamlar
//    PART_ARKA_TAMPON
//};
//
//class CCarExpertiseCtrl : public CWnd
//{
//public:
//    CCarExpertiseCtrl();
//    virtual ~CCarExpertiseCtrl();
//
//    // Veritabanı string'inden durumu yükle
//    void SetDataString(const CString& data);
//    // Veritabanına kaydetmek için string üret
//    CString GetDataString();
//
//protected:
//    virtual void OnDraw(CDC& dc);
//    virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
//    
//    // Yardımcılar
//    void DrawCarPart(CDC& dc, CarPartID id, CRect rect, CString label);
//    CRect GetPartRect(CarPartID id, CRect clientRect);
//    CBrush GetBrushForStatus(ExpertiseStatus status);
//    CString GetStatusName(ExpertiseStatus status);
//
//    // Tıklama Yönetimi
//    void OnLButtonDown(POINT pt);
//    void CycleStatus(CarPartID id); // Sol tık: Durumu döngüye sok (Orj->Boya->Değişen)
//    void ShowDetailMenu(POINT pt, CarPartID id); // Sağ tık: Detay menüsü (Lokal, çizik vb.)
//
//private:
//    std::map<int, ExpertiseStatus> m_PartStatus; // Parça ID -> Durum
//};