#include "stdafx.h"
#include "RayPropertyGridCtrl.h"

#include "MainFrm.h"
#include "UDSonCANDoc.h"
#include "UDSonCANView.h"


CRayPropertyGridCtrl::CRayPropertyGridCtrl(void)
{
}

CRayPropertyGridCtrl::~CRayPropertyGridCtrl(void)
{
}

void CRayPropertyGridCtrl::SetLeftColumnWidth(int nNewWidth)
{
	if (nNewWidth >= 0)
	{
		m_nLeftColumnWidth = nNewWidth;
		AdjustLayout();
	}
}

void CRayPropertyGridCtrl::OnPropertyChanged(CMFCPropertyGridProperty *pProp) const
{
	((CUDSonCANView*)((CMainFrame*)AfxGetMainWnd())->GetActiveView())->SwitchProperty(pProp);
	return CMFCPropertyGridCtrl::OnPropertyChanged(pProp);
}

//int CRayPropertyGridCtrl::OnDrawProperty(CDC* pDC, CMFCPropertyGridProperty* pProp)
//{
//	int nVisibleHeight = 0;
//	int nPropertyCount = GetPropertyCount();
//	int nVisiblePropertyCount = 0;
//	CRect rect;
//	GetWindowRect(&rect);
//	if (IsHeaderCtrl())
//	{
//		nVisibleHeight += GetHeaderHeight();
//	}
//	if (IsDescriptionArea())
//	{
//		nVisibleHeight += GetDescriptionHeight();
//	}
//	for (int i = 0; i != nPropertyCount; ++i)
//	{
//		if (GetProperty(i)->IsVisible())
//		{
//			++nVisiblePropertyCount;
//		}
//	}
//	nVisibleHeight += nVisiblePropertyCount * GetRowHeight();
//
//	GetScrollBarCtrl(SB_VERT)->ShowScrollBar(nVisibleHeight > rect.Height());
//	// ShowScrollBar(SB_VERT, nVisibleHeight > rect.Height());
//	// GetScrollBarCtrl(SB_VERT)->ShowWindow(nVisibleHeight > rect.Height() ? SW_SHOW : SW_HIDE);
//	
//	return CMFCPropertyGridCtrl::OnDrawProperty(pDC, pProp);
//}

BEGIN_MESSAGE_MAP(CRayPropertyGridCtrl, CMFCPropertyGridCtrl)
	ON_WM_SIZE()
END_MESSAGE_MAP()


void CRayPropertyGridCtrl::OnSize(UINT nType, int cx, int cy)
{
	// 阻止调整大小时列宽变为一半。
	// 请参见 atlmfc\src\mfc\afxpropertygridctrl.cpp: CMFCPropertyGridCtrl::OnSize(UINT nType, int cx, int cy)
	// m_nLeftColumnWidth = cx / 2;

	CWnd::OnSize(nType, cx, cy);

	EndEditItem();

	AdjustLayout();
}