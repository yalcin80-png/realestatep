#include "stdafx.h"
#include "CarExpertiseCtrl.h"
//
//CCarExpertiseCtrl::CCarExpertiseCtrl() {
//}
//
//CCarExpertiseCtrl::~CCarExpertiseCtrl() {
//}
//
//void CCarExpertiseCtrl::SetDataString(const CString& data)
//{
//    m_PartStatus.clear();
//    // Parse string format: "1:2;3:0;" (ID:Status)
//    int curPos = 0;
//    CString token = data.Tokenize(_T(";"), curPos);
//    while (token != _T(""))
//    {
//        int sep = token.Find(_T(':'));
//        if (sep > 0)
//        {
//            int id = _ttoi(token.Left(sep));
//            int stat = _ttoi(token.Mid(sep + 1));
//            m_PartStatus[id] = (ExpertiseStatus)stat;
//        }
//        token = data.Tokenize(_T(";"), curPos);
//    }
//    Invalidate();
//}
//
//CString CCarExpertiseCtrl::GetDataString()
//{
//    CString strOut = _T("");
//    for (auto const& [id, status] : m_PartStatus)
//    {
//        if (status != EXP_ORJINAL) // Sadece hasarlıları kaydetsek yeter, yer tasarrufu
//        {
//            CString temp;
//            temp.Format(_T("%d:%d;"), id, (int)status);
//            strOut += temp;
//        }
//    }
//    return strOut;
//}
//
//CRect CCarExpertiseCtrl::GetPartRect(CarPartID id, CRect rc)
//{
//    // Arabanın şematik çizimi için koordinatlar (Oranlara göre)
//    // rc: Client Rect
//    int w = rc.Width();
//    int h = rc.Height();
//    int cx = w / 2;
//    int cy = h / 2;
//    
//    // Basit bir kuşbakışı araba şeması (Grid mantığı)
//    // Ön taraf yukarıda olsun varsayalım veya sol taraf.
//    // Yatay çizim: [Ön Tampon] [Kaput] [Tavan] [Bagaj] [Arka Tampon]
//    
//    int carL = w * 0.1; int carR = w * 0.9;
//    int carT = h * 0.2; int carB = h * 0.8;
//    int partH = (carB - carT) / 3; // Üst, Orta, Alt şeritler
//    int segW = (carR - carL) / 5;  // 5 dikey segment
//
//    // Koordinatları hesapla (Şematik)
//    switch(id)
//    {
//        case PART_ON_TAMPON: return CRect(carL, carT, carL+segW, carB);
//        case PART_KAPUT: return CRect(carL+segW, carT+partH, carL+2*segW, carT+2*partH);
//        
//        case PART_SOL_ON_CAMURLUK: return CRect(carL+segW, carT, carL+2*segW, carT+partH);
//        case PART_SAG_ON_CAMURLUK: return CRect(carL+segW, carB-partH, carL+2*segW, carB);
//
//        case PART_TAVAN: return CRect(carL+2*segW, carT+partH, carL+4*segW, carT+2*partH);
//
//        case PART_SOL_ON_KAPI: return CRect(carL+2*segW, carT, carL+3*segW, carT+partH);
//        case PART_SAG_ON_KAPI: return CRect(carL+2*segW, carB-partH, carL+3*segW, carB);
//
//        case PART_SOL_ARKA_KAPI: return CRect(carL+3*segW, carT, carL+4*segW, carT+partH);
//        case PART_SAG_ARKA_KAPI: return CRect(carL+3*segW, carB-partH, carL+4*segW, carB);
//        
//        case PART_SOL_ARKA_CAMURLUK: return CRect(carL+4*segW, carT, carL+5*segW, carT+partH);
//        case PART_SAG_ARKA_CAMURLUK: return CRect(carL+4*segW, carB-partH, carL+5*segW, carB);
//
//        case PART_BAGAJ: return CRect(carL+4*segW, carT+partH, carL+5*segW, carT+2*partH);
//        case PART_ARKA_TAMPON: return CRect(carL+5*segW, carT, carL+5*segW + (segW/2), carB);
//    }
//    return CRect(0,0,0,0);
//}
//
//CBrush CCarExpertiseCtrl::GetBrushForStatus(ExpertiseStatus status)
//{
//    switch(status) {
//        case EXP_BOYALI: return CBrush(RGB(255, 0, 0)); // Kırmızı
//        case EXP_LOKAL_BOYA: return CBrush(RGB(255, 165, 0)); // Turuncu/Sarı
//        case EXP_DEGISEN: return CBrush(RGB(0, 0, 255)); // Mavi
//        case EXP_PLASTIK: return CBrush(RGB(128, 128, 128)); // Gri
//        case EXP_EZIK_CIZIK: return CBrush(RGB(128, 0, 128)); // Mor
//        default: return CBrush(RGB(240, 240, 240)); // Beyaz/Gri (Orjinal)
//    }
//}
//
//void CCarExpertiseCtrl::DrawCarPart(CDC& dc, CarPartID id, CRect rect, CString label)
//{
//    ExpertiseStatus status = m_PartStatus[id]; // Varsayılan 0 (Orjinal)
//    CBrush br = GetBrushForStatus(status);
//    
//    dc.FillRect(rect, br);
//    dc.FrameRect(rect, CBrush(RGB(0,0,0))); // Siyah çerçeve
//    
//    // Yazıyı ortala
//    dc.SetBkMode(TRANSPARENT);
//    if(status != EXP_ORJINAL) dc.SetTextColor(RGB(255,255,255));
//    else dc.SetTextColor(RGB(0,0,0));
//    
//    dc.DrawText(label, -1, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
//}
//
//void CCarExpertiseCtrl::OnDraw(CDC& dc)
//{
//    CRect rc;
//    GetClientRect(rc);
//    
//    // Arka planı temizle
//    dc.FillRect(rc, CBrush(GetSysColor(COLOR_WINDOW)));
//
//    DrawCarPart(dc, PART_ON_TAMPON, GetPartRect(PART_ON_TAMPON, rc), _T("Ön Tampon"));
//    DrawCarPart(dc, PART_KAPUT, GetPartRect(PART_KAPUT, rc), _T("Kaput"));
//    DrawCarPart(dc, PART_TAVAN, GetPartRect(PART_TAVAN, rc), _T("Tavan"));
//    DrawCarPart(dc, PART_BAGAJ, GetPartRect(PART_BAGAJ, rc), _T("Bagaj"));
//    DrawCarPart(dc, PART_ARKA_TAMPON, GetPartRect(PART_ARKA_TAMPON, rc), _T("Arka T."));
//
//    DrawCarPart(dc, PART_SOL_ON_CAMURLUK, GetPartRect(PART_SOL_ON_CAMURLUK, rc), _T("Sol Ön Ç."));
//    DrawCarPart(dc, PART_SOL_ON_KAPI, GetPartRect(PART_SOL_ON_KAPI, rc), _T("Sol Ön K."));
//    DrawCarPart(dc, PART_SOL_ARKA_KAPI, GetPartRect(PART_SOL_ARKA_KAPI, rc), _T("Sol Arka K."));
//    DrawCarPart(dc, PART_SOL_ARKA_CAMURLUK, GetPartRect(PART_SOL_ARKA_CAMURLUK, rc), _T("Sol Arka Ç."));
//
//    DrawCarPart(dc, PART_SAG_ON_CAMURLUK, GetPartRect(PART_SAG_ON_CAMURLUK, rc), _T("Sağ Ön Ç."));
//    DrawCarPart(dc, PART_SAG_ON_KAPI, GetPartRect(PART_SAG_ON_KAPI, rc), _T("Sağ Ön K."));
//    DrawCarPart(dc, PART_SAG_ARKA_KAPI, GetPartRect(PART_SAG_ARKA_KAPI, rc), _T("Sağ Arka K."));
//    DrawCarPart(dc, PART_SAG_ARKA_CAMURLUK, GetPartRect(PART_SAG_ARKA_CAMURLUK, rc), _T("Sağ Arka Ç."));
//}
//
//void CCarExpertiseCtrl::CycleStatus(CarPartID id)
//{
//    int s = (int)m_PartStatus[id];
//    s++;
//    if(s > 3) s = 0; // Orjinal -> Boya -> Lokal -> Değişen -> Orjinal döngüsü
//    m_PartStatus[id] = (ExpertiseStatus)s;
//    Invalidate();
//}
//
//void CCarExpertiseCtrl::OnLButtonDown(POINT pt)
//{
//    CRect rc;
//    GetClientRect(rc);
//
//    // Hangi parçaya tıklandı?
//    for (int i = 1; i <= PART_ARKA_TAMPON; i++)
//    {
//        CRect partRect = GetPartRect((CarPartID)i, rc);
//        if (partRect.PtInRect(pt))
//        {
//            // Tıklandı!
//            CycleStatus((CarPartID)i);
//            break;
//        }
//    }
//}
//
//LRESULT CCarExpertiseCtrl::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
//{
//    switch (uMsg)
//    {
//    case WM_PAINT:
//        {
//            CPaintDC dc(*this);
//            OnDraw(dc);
//        }
//        return 0;
//    case WM_LBUTTONDOWN:
//        {
//            POINT pt;
//            pt.x = GET_X_LPARAM(lParam);
//            pt.y = GET_Y_LPARAM(lParam);
//            OnLButtonDown(pt);
//        }
//        return 0;
//    }
//    return WndProcDefault(uMsg, wParam, lParam);
//}