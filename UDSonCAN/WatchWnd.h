
#pragma once

#include <list>

/////////////////////////////////////////////////////////////////////////////
// CWatchList 窗口

class CWatchList : public CListCtrl
{
	DECLARE_DYNAMIC(CWatchList)
// 构造
public:
	CWatchList();
	virtual ~CWatchList();

// 实现
public:

protected:
	afx_msg void OnContextMenu(CWnd *pWnd, CPoint point);
	afx_msg void OnEditCopy();
	afx_msg void OnEditClear();
	afx_msg void OnViewWatch();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);
};

class CWatchWnd : public CDockablePane
{
	DECLARE_DYNAMIC(CWatchWnd)
// 构造
public:
	CWatchWnd();
	virtual ~CWatchWnd();

// 特性
public:
	enum struct Color : DWORD
	{
		Black = RGB(0, 0, 0),
		Green = RGB(96, 140, 78),
		Red = RGB(255, 0, 0)
	};

// 操作
public:

// 实现
public:

	void UpdateFonts();
protected:
	CWatchList m_wndWatchList;

	void AdjustHorzScroll(CListBox &wndListBox);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()
};

class CTIDCWatchWnd : public CWatchWnd
{
	DECLARE_DYNAMIC(CTIDCWatchWnd)
// 构造
public:
	CTIDCWatchWnd();
	virtual ~CTIDCWatchWnd();

// 属性
	enum struct EntryType
	{
		Receive,
		Transmit,
		Confirm,
	};

	struct Entry
	{
		DWORD dwTick;
		EntryType entryType;
		UINT32 nID;
		CString csDescription;
		// LPCTSTR lpszDescription;
		Color color;
	};
	typedef std::list<Entry *> PEntryList;

// 操作
public:
	void AddEntry(DWORD dwTick, EntryType entryType, UINT32 nID, LPCTSTR lpszDescription, Color color = Color::Black);
// 实现
protected:
	PEntryList m_lpEntry;
	CCriticalSection m_csectionEntryList;
	afx_msg LRESULT OnWatchAddEntry(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};
