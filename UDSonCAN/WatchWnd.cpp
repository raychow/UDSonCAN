
#include "stdafx.h"
#include "WatchWnd.h"

#include "MainFrm.h"
#include "Resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWatchBar

IMPLEMENT_DYNAMIC(CWatchWnd, CDockablePane)

CWatchWnd::CWatchWnd()
{
}

CWatchWnd::~CWatchWnd()
{
}

BEGIN_MESSAGE_MAP(CWatchWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

int CWatchWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// 创建输出窗格:
	const DWORD dwStyle = LBS_NOINTEGRALHEIGHT | WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | LVS_REPORT;

	if (!m_wndWatchList.Create(dwStyle, rectDummy, this, 1))
	{
		TRACE0("未能创建输出窗口\n");
		return -1;      // 未能创建
	}
	
	m_wndWatchList.SetExtendedStyle(m_wndWatchList.GetExtendedStyle() | LVS_EX_FULLROWSELECT);

	UpdateFonts();

	return 0;
}

void CWatchWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	// 选项卡控件应覆盖整个工作区:
	m_wndWatchList.SetWindowPos (NULL, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
}

void CWatchWnd::AdjustHorzScroll(CListBox &wndListBox)
{
	CClientDC dc(this);
	CFont *pOldFont = dc.SelectObject(&afxGlobalData.fontRegular);

	int cxExtentMax = 0;

	for (int i = 0; i < wndListBox.GetCount(); i ++)
	{
		CString strItem;
		wndListBox.GetText(i, strItem);

		cxExtentMax = max(cxExtentMax, (int)dc.GetTextExtent(strItem).cx);
	}

	wndListBox.SetHorizontalExtent(cxExtentMax);
	dc.SelectObject(pOldFont);
}

void CWatchWnd::UpdateFonts()
{
	m_wndWatchList.SetFont(&afxGlobalData.fontRegular);
}

/////////////////////////////////////////////////////////////////////////////
// CWatchList

IMPLEMENT_DYNAMIC(CWatchList, CListCtrl)

CWatchList::CWatchList()
{
}

CWatchList::~CWatchList()
{
}

BEGIN_MESSAGE_MAP(CWatchList, CListCtrl)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	ON_COMMAND(ID_VIEW_NETWORKLAYERWATCHWND, OnViewWatch)
	ON_COMMAND(ID_VIEW_APPLICATIONLAYERWATCHWND, OnViewWatch)
	ON_WM_WINDOWPOSCHANGING()
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, &CWatchList::OnNMCustomdraw)
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// CWatchList 消息处理程序

void CWatchList::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	CMenu menu;
	menu.LoadMenu(IDR_WATCH_POPUP);

	CMenu *pSumMenu = menu.GetSubMenu(0);

	if (AfxGetMainWnd()->IsKindOf(RUNTIME_CLASS(CFrameWndEx)))
	{
		CMFCPopupMenu *pPopupMenu = new CMFCPopupMenu;

		if (!pPopupMenu->Create(this, point.x, point.y, (HMENU)pSumMenu->m_hMenu, FALSE, TRUE))
			return;

		((CFrameWndEx*)AfxGetMainWnd())->OnShowPopupMenu(pPopupMenu);
		UpdateDialogControls(this, FALSE);
	}

	SetFocus();
}

void CWatchList::OnEditCopy()
{
	MessageBox(_T("复制输出"));
}

void CWatchList::OnEditClear()
{
	DeleteAllItems();
}

void CWatchList::OnViewWatch()
{
	CDockablePane *pParentBar = DYNAMIC_DOWNCAST(CDockablePane, GetOwner());
	CFrameWndEx *pMainFrame = DYNAMIC_DOWNCAST(CFrameWndEx, GetTopLevelFrame());

	if (pMainFrame != NULL && pParentBar != NULL)
	{
		pMainFrame->SetFocus();
		pMainFrame->ShowPane(pParentBar, FALSE, FALSE, FALSE);
		pMainFrame->RecalcLayout();
	}
}

IMPLEMENT_DYNAMIC(CTIDCWatchWnd, CWatchWnd)

CTIDCWatchWnd::CTIDCWatchWnd()
{
}

CTIDCWatchWnd::~CTIDCWatchWnd()
{
}


void CTIDCWatchWnd::AddEntry(DWORD dwTick, EntryType entryType, INT32 nID, LPCTSTR lpszDescription, Color color)
{
	Entry *entry = new Entry();
	entry->dwTick = dwTick;
	entry->entryType = entryType;
	entry->nID = nID;
	entry->csDescription = lpszDescription;
	entry->color = color;
	CSingleLock lockEntryList(&m_csectionEntryList);
	lockEntryList.Lock();
	m_lpEntry.push_back(entry);
	lockEntryList.Unlock();
}

BEGIN_MESSAGE_MAP(CTIDCWatchWnd, CWatchWnd)
	ON_WM_CREATE()
	ON_MESSAGE(WM_WATCH_ADDENTRY, &CTIDCWatchWnd::OnWatchAddEntry)
END_MESSAGE_MAP()

int CTIDCWatchWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWatchWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CString csTemp;
	csTemp.LoadString(IDS_WATCH_TIME);
	m_wndWatchList.InsertColumn(0, csTemp, 0, 60);
	csTemp.LoadString(IDS_WATCH_TYPE);
	m_wndWatchList.InsertColumn(1, csTemp, 0, 50);
	csTemp.LoadString(IDS_WATCH_ID);
	m_wndWatchList.InsertColumn(2, csTemp, 0, 50);
	csTemp.LoadString(IDS_WATCH_DATA);
	m_wndWatchList.InsertColumn(3, csTemp, 0, 500);
	return 0;
}


void CWatchList::OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVCUSTOMDRAW pNMCD = reinterpret_cast<LPNMLVCUSTOMDRAW>(pNMHDR);
	*pResult = CDRF_DODEFAULT;
	switch (pNMCD->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		*pResult = CDRF_NOTIFYITEMDRAW;
		break;
	case CDDS_ITEMPREPAINT :
		COLORREF clrText = static_cast<COLORREF>(GetItemData(pNMCD->nmcd.dwItemSpec));
		pNMCD->clrText = clrText;
	}
}

LRESULT CTIDCWatchWnd::OnWatchAddEntry(WPARAM wParam, LPARAM lParam)
{
	if (m_lpEntry.empty())
	{
		return 0;
	}

	Entry *entry = NULL;
	CString csTemp;
	int nItem = 0;

	m_wndWatchList.SetRedraw(FALSE);
	CSingleLock lockEntryList(&m_csectionEntryList);
	lockEntryList.Lock();
	while (!m_lpEntry.empty())
	{
		entry = m_lpEntry.front();
		csTemp.Format(_T("%.4f"), static_cast<float>(entry->dwTick) / 1000);
		nItem = m_wndWatchList.InsertItem(m_wndWatchList.GetItemCount(), csTemp);
		switch (entry->entryType)
		{
		case EntryType::Receive:
			csTemp.LoadString(IDS_RECEIVE);
			break;
		case EntryType::Transmit:
			csTemp.LoadString(IDS_TRANSMIT);
			break;
		case EntryType::Confirm:
			csTemp.LoadString(IDS_CONFIRM);
			break;
		}
		m_wndWatchList.SetItemText(nItem, 1, csTemp);
		csTemp.Format(_T("%04X"), entry->nID);
		m_wndWatchList.SetItemText(nItem, 2, csTemp);
		m_wndWatchList.SetItemText(nItem, 3, entry->csDescription);
		m_wndWatchList.SetItemData(nItem, static_cast<DWORD_PTR>(entry->color));
		delete entry;
		m_lpEntry.pop_front();
	}
	lockEntryList.Unlock();
	m_wndWatchList.SetRedraw(TRUE);
	m_wndWatchList.EnsureVisible(nItem, FALSE);
	m_wndWatchList.RedrawWindow();
	return 0;
}