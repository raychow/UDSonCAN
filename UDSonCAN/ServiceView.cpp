#include "stdafx.h"
#include "ServiceView.h"

#include "MainFrm.h"
#include "Resource.h"
#include "UDSonCAN.h"

class CServiceViewMenuButton : public CMFCToolBarMenuButton
{
	friend class CServiceView;

	DECLARE_SERIAL(CServiceViewMenuButton)

public:
	CServiceViewMenuButton(HMENU hMenu = NULL) : CMFCToolBarMenuButton((UINT)-1, hMenu, -1)
	{
	}

	virtual void OnDraw(CDC *pDC, const CRect &rect, CMFCToolBarImages *pImages, BOOL bHorz = TRUE,
		BOOL bCustomizeMode = FALSE, BOOL bHighlight = FALSE, BOOL bDrawBorder = TRUE, BOOL bGrayDisabledButtons = TRUE)
	{
		pImages = CMFCToolBar::GetImages();

		CAfxDrawState ds;
		pImages->PrepareDrawImage(ds);

		CMFCToolBarMenuButton::OnDraw(pDC, rect, pImages, bHorz, bCustomizeMode, bHighlight, bDrawBorder, bGrayDisabledButtons);

		pImages->EndDrawImage(ds);
	}
};

IMPLEMENT_SERIAL(CServiceViewMenuButton, CMFCToolBarMenuButton, 1)

//////////////////////////////////////////////////////////////////////
// 构造/析构
//////////////////////////////////////////////////////////////////////

CServiceView::CServiceView()
{
	m_nCurrSort = ID_SORTING_GROUPBYTYPE;
}

CServiceView::~CServiceView()
{
}

BEGIN_MESSAGE_MAP(CServiceView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_COMMAND_RANGE(ID_SORTING_GROUPBYTYPE, ID_SORTING_SORTBYACCESS, OnSort)
	ON_UPDATE_COMMAND_UI_RANGE(ID_SORTING_GROUPBYTYPE, ID_SORTING_SORTBYACCESS, OnUpdateSort)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CServiceView 消息处理程序

int CServiceView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// 创建视图:
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS |TVS_SHOWSELALWAYS | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	if (!m_wndServiceView.Create(dwViewStyle, rectDummy, this, 2))
	{
		TRACE0("未能创建服务视图\n");
		return -1;      // 未能创建
	}

	// 加载图像:
	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_SORT);
	m_wndToolBar.LoadToolBar(IDR_SORT, 0, 0, TRUE /* 已锁定*/);

	OnChangeVisualStyle();

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));

	m_wndToolBar.SetOwner(this);

	// 所有命令将通过此控件路由，而不是通过主框架路由:
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	CMenu menuSort;
	menuSort.LoadMenu(IDR_POPUP_SORT);

	m_wndToolBar.ReplaceButton(ID_SORT_MENU, CServiceViewMenuButton(menuSort.GetSubMenu(0)->GetSafeHmenu()));

	CServiceViewMenuButton *pButton =  DYNAMIC_DOWNCAST(CServiceViewMenuButton, m_wndToolBar.GetButton(0));

	if (pButton != NULL)
	{
		pButton->m_bText = FALSE;
		pButton->m_bImage = TRUE;
		pButton->SetImage(GetCmdMgr()->GetCmdImage(m_nCurrSort));
		pButton->SetMessageWnd(this);
	}

	FillServiceView();

	return 0;
}

void CServiceView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CServiceView::FillServiceView()
{
	CString csItem;
	csItem.LoadString(IDS_SERVICEVIEW_DIAGNOSTICSERVICE);
	HTREEITEM hRoot = m_wndServiceView.InsertItem(csItem, 0, 0);
	m_wndServiceView.SetItemData(hRoot, static_cast<UINT>(DiagnosticService::OtherServiceID::NoAction));

	HTREEITEM hChild;
	BYTE byID;
	const DiagnosticService::ItemVector &vpItem = theApp.GetDiagnosticService().GetItemVector();
	DiagnosticService::ItemVector::size_type stItem = vpItem.size();
	for (DiagnosticService::ItemVector::size_type i = 0; i != stItem; ++i)
	{
		byID = vpItem.at(i)->GetItemID();
		csItem.Format(IDS_DIAGNOSTIC_ITEMFORMAT, byID, vpItem.at(i)->GetCaption()); 
		hChild = m_wndServiceView.InsertItem(csItem, 1, 1, hRoot);
		m_wndServiceView.SetItemData(hChild, byID);
	}

	m_wndServiceView.Expand(hRoot, TVE_EXPAND);

	csItem.LoadString(IDS_SERVICEVIEW_DOWNLOAD);
	hRoot = m_wndServiceView.InsertItem(csItem, 0, 0);
	m_wndServiceView.SetItemData(hRoot, static_cast<UINT>(DiagnosticService::OtherServiceID::Download));

	csItem.LoadString(IDS_SERVICEVIEW_TEST);
	hRoot = m_wndServiceView.InsertItem(csItem, 0, 0);
	m_wndServiceView.SetItemData(hRoot, static_cast<UINT>(DiagnosticService::OtherServiceID::Test));
}

void CServiceView::OnContextMenu(CWnd *pWnd, CPoint point)
{
	CTreeCtrl *pWndTree = (CTreeCtrl*)&m_wndServiceView;
	ASSERT_VALID(pWndTree);

	if (pWnd != pWndTree)
	{
		CDockablePane::OnContextMenu(pWnd, point);
		return;
	}

	if (point != CPoint(-1, -1))
	{
		// 选择已单击的项:
		CPoint ptTree = point;
		pWndTree->ScreenToClient(&ptTree);

		UINT flags = 0;
		HTREEITEM hTreeItem = pWndTree->HitTest(ptTree, &flags);
		if (hTreeItem != NULL)
		{
			pWndTree->SelectItem(hTreeItem);
		}
	}

	pWndTree->SetFocus();
	CMenu menu;
	menu.LoadMenu(IDR_POPUP_SORT);

	CMenu *pSumMenu = menu.GetSubMenu(0);

	if (AfxGetMainWnd()->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
	{
		CMFCPopupMenu *pPopupMenu = new CMFCPopupMenu;

		if (!pPopupMenu->Create(this, point.x, point.y, (HMENU)pSumMenu->m_hMenu, FALSE, TRUE))
			return;

		((CMDIFrameWndEx*)AfxGetMainWnd())->OnShowPopupMenu(pPopupMenu);
		UpdateDialogControls(this, FALSE);
	}
}

void CServiceView::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndServiceView.SetWindowPos(NULL, rectClient.left + 1, rectClient.top + cyTlb + 1, rectClient.Width() - 2, rectClient.Height() - cyTlb - 2, SWP_NOACTIVATE | SWP_NOZORDER);
}

BOOL CServiceView::PreTranslateMessage(MSG *pMsg)
{
	return CDockablePane::PreTranslateMessage(pMsg);
}

void CServiceView::OnSort(UINT id)
{
	if (m_nCurrSort == id)
	{
		return;
	}

	m_nCurrSort = id;

	CServiceViewMenuButton *pButton =  DYNAMIC_DOWNCAST(CServiceViewMenuButton, m_wndToolBar.GetButton(0));

	if (pButton != NULL)
	{
		pButton->SetImage(GetCmdMgr()->GetCmdImage(id));
		m_wndToolBar.Invalidate();
		m_wndToolBar.UpdateWindow();
	}
}

void CServiceView::OnUpdateSort(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(pCmdUI->m_nID == m_nCurrSort);
}

void CServiceView::OnPaint()
{
	CPaintDC dc(this); // 用于绘制的设备上下文

	CRect rectTree;
	m_wndServiceView.GetWindowRect(rectTree);
	ScreenToClient(rectTree);

	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void CServiceView::OnSetFocus(CWnd *pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

	m_wndServiceView.SetFocus();
}

void CServiceView::OnChangeVisualStyle()
{
	m_ServiceViewImages.DeleteImageList();

	UINT uiBmpId = theApp.m_bHiColorIcons ? IDB_SERVICE_VIEW_24 : IDB_SERVICE_VIEW;

	CBitmap bmp;
	if (!bmp.LoadBitmap(uiBmpId))
	{
		TRACE(_T("无法加载位图: %x\n"), uiBmpId);
		ASSERT(FALSE);
		return;
	}

	BITMAP bmpObj;
	bmp.GetBitmap(&bmpObj);

	UINT nFlags = ILC_MASK;

	nFlags |= (theApp.m_bHiColorIcons) ? ILC_COLOR24 : ILC_COLOR4;

	m_ServiceViewImages.Create(16, bmpObj.bmHeight, nFlags, 0, 0);
	m_ServiceViewImages.Add(&bmp, RGB(255, 0, 0));

	m_wndServiceView.SetImageList(&m_ServiceViewImages, TVSIL_NORMAL);

	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_SORT_24 : IDR_SORT, 0, 0, TRUE /* 锁定*/);
}

UINT CServiceView::GetCurrentServiceID() const
{
	HTREEITEM hItem = m_wndServiceView.GetSelectedItem();
	return m_wndServiceView.GetItemData(hItem);
}