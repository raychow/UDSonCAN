
// UDSonCANDoc.cpp : CUDSonCANDoc 类的实现
//

#include "stdafx.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "UDSonCAN.h"
#endif

#include "UDSonCANDoc.h"

#include <propkey.h>

#include "DiagnosticControl.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CUDSonCANDoc

IMPLEMENT_DYNCREATE(CUDSonCANDoc, CDocument)

BEGIN_MESSAGE_MAP(CUDSonCANDoc, CDocument)
END_MESSAGE_MAP()


// CUDSonCANDoc 构造/析构

CUDSonCANDoc::CUDSonCANDoc()
	: m_diagnosticStorage(0)
	, m_pDiagnosticControl(NULL)
{
	// TODO: 在此添加一次性构造代码

}

CUDSonCANDoc::~CUDSonCANDoc()
{
}

DiagnosticStorage::CSection *CUDSonCANDoc::GetDiagnosticStorage()
{
	return &m_diagnosticStorage;
}

CDiagnosticControl &CUDSonCANDoc::GetDiagnosticControl()
{
	return *m_pDiagnosticControl;
}

BOOL CUDSonCANDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	theApp.GetDiagnosticService().BuildDiagnosticStorage(&m_diagnosticStorage);
	if (NULL != m_pDiagnosticControl)
	{
		delete m_pDiagnosticControl;
	}
	m_pDiagnosticControl = new CDiagnosticControl();
	m_pDiagnosticControl->SetNetworkLayerWatchWnd(static_cast<CMainFrame *>(AfxGetMainWnd())->GetNetworkLayerWatchWnd());
	m_pDiagnosticControl->SetApplicationLayerWatchWnd(static_cast<CMainFrame *>(AfxGetMainWnd())->GetApplicationLayerWatchWnd());

	return TRUE;
}

// CUDSonCANDoc 序列化

//void CUDSonCANDoc::Serialize(CArchive &ar)
//{
//	if (ar.IsStoring())
//	{
//		// TODO: 在此添加存储代码
//	}
//	else
//	{
//		// TODO: 在此添加加载代码
//	}
//}

#ifdef SHARED_HANDLERS

// 缩略图的支持
void CUDSonCANDoc::OnDrawThumbnail(CDC &dc, LPRECT lprcBounds)
{
	// 修改此代码以绘制文档数据
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont *pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont *pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// 搜索处理程序的支持
void CUDSonCANDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// 从文档数据设置搜索内容。
	// 内容部分应由“;”分隔

	// 例如:  strSearchContent = _T("point;rectangle;circle;ole object;")；
	SetSearchContent(strSearchContent);
}

void CUDSonCANDoc::SetSearchContent(const CString &value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CUDSonCANDoc 诊断

#ifdef _DEBUG
void CUDSonCANDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CUDSonCANDoc::Dump(CDumpContext &dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CUDSonCANDoc 命令

BOOL CUDSonCANDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	
	return TRUE;
}


BOOL CUDSonCANDoc::OnOpenDocument(LPCTSTR lpszPathName)
{

	return TRUE;
}


void CUDSonCANDoc::OnCloseDocument()
{
	if (NULL != m_pDiagnosticControl)
	{
		delete m_pDiagnosticControl;
		m_pDiagnosticControl = NULL;
	}
	CDocument::OnCloseDocument();
}
