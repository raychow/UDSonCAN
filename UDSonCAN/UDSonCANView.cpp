
// UDSonCANView.cpp : CUDSonCANView 类的实现
//

#include "stdafx.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "UDSonCAN.h"
#endif

#include "ApplicationLayerConfigDialog.h"
#include "CANConfigDialog.h"
#include "DiagnosticControl.h"
#include "MainFrm.h"
#include "NetworkLayerConfigDialog.h"
#include "TestDialog.h"
#include "UDSonCANDoc.h"
#include "UDSonCANView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CUDSonCANView

IMPLEMENT_DYNCREATE(CUDSonCANView, CView)

BEGIN_MESSAGE_MAP(CUDSonCANView, CView)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_SIZE()
	ON_COMMAND(ID_TOOLS_TEST, &CUDSonCANView::OnToolsTest)
	ON_COMMAND(ID_CONFIG_PHYSICALLAYER, &CUDSonCANView::OnConfigPhysicalLayer)
	ON_COMMAND(ID_CONFIG_NETWORKLAYER, &CUDSonCANView::OnConfigNetworkLayer)
	ON_COMMAND(ID_CONFIG_APPLICATIONLAYER, &CUDSonCANView::OnConfigApplicationLayer)
	ON_COMMAND(ID_DIAGNOSTIC_BEGINDIAGNOSTIC, &CUDSonCANView::OnDiagnosticBeginDiagnostic)
	ON_UPDATE_COMMAND_UI(ID_DIAGNOSTIC_BEGINDIAGNOSTIC, &CUDSonCANView::OnUpdateDiagnosticBeginDiagnostic)
	ON_COMMAND(ID_DIAGNOSTIC_STOPDIAGNOSTIC, &CUDSonCANView::OnDiagnosticStopDiagnostic)
	ON_UPDATE_COMMAND_UI(ID_DIAGNOSTIC_STOPDIAGNOSTIC, &CUDSonCANView::OnUpdateDiagnosticStopDiagnostic)
	ON_COMMAND(ID_DIAGNOSTIC_EXECUTE, &CUDSonCANView::OnDiagnosticExecute)
	ON_UPDATE_COMMAND_UI(ID_DIAGNOSTIC_EXECUTE, &CUDSonCANView::OnUpdateDiagnosticExecute)
	ON_WM_CREATE()
END_MESSAGE_MAP()

// CUDSonCANView 构造/析构

CUDSonCANView::CUDSonCANView()
	: m_nServiceID(DiagnosticService::OtherServiceID::NoAction)
{
	// TODO: 在此处添加构造代码

}

CUDSonCANView::~CUDSonCANView()
{
}

// CUDSonCANView 绘制

void CUDSonCANView::OnDraw(CDC* /*pDC*/)
{
	CUDSonCANDoc *pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 在此处为本机数据添加绘制代码
}

void CUDSonCANView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CUDSonCANView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CUDSonCANView 诊断

#ifdef _DEBUG
void CUDSonCANView::AssertValid() const
{
	CView::AssertValid();
}

void CUDSonCANView::Dump(CDumpContext&dc) const
{
	CView::Dump(dc);
}

CUDSonCANDoc *CUDSonCANView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CUDSonCANDoc)));
	return (CUDSonCANDoc*)m_pDocument;
}
#endif //_DEBUG


// CUDSonCANView 消息处理程序


void CUDSonCANView::OnInitialUpdate()
{
	CView::OnInitialUpdate();
	InitPropertyGridCtrl();
}

void CUDSonCANView::InitPropertyGridCtrl()
{
	CRect rectClient;
	GetClientRect(rectClient);
	m_propertyGridCtrl.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), SWP_NOACTIVATE | SWP_NOZORDER);

	m_propertyGridCtrl.EnableDescriptionArea(FALSE);
	m_propertyGridCtrl.SetLeftColumnWidth(200);
	m_propertyGridCtrl.SetVSDotNetLook(TRUE);
}

void CUDSonCANView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	if (m_propertyGridCtrl.GetSafeHwnd())
	{
		CRect rectClient;
		GetClientRect(rectClient);
		m_propertyGridCtrl.MoveWindow(rectClient);
	}
}

void CUDSonCANView::SwitchService(UINT nServiceID)
{
	TRACE(_T("Switch to ServiceID: 0x%X.\n"), nServiceID);
	m_nServiceID = nServiceID;
	m_propertyGridCtrl.SetRedraw(FALSE);
	m_propertyGridCtrl.RemoveAll();
	CString csHexValidChars;
	csHexValidChars.LoadString(IDS_VALIDCHARS_HEXSPACE);
	CString csName;
	csName.LoadString(IDS_DIAGNOSTIC_PDU);
	CString csItemString;
	if (nServiceID < 0x100)
	{
		const DiagnosticService::CItem *pItem = theApp.GetDiagnosticService().GetItem(nServiceID);
		DiagnosticStorage::CItem *pStorageItem = GetDocument()->GetDiagnosticStorage()->GetItem(pItem->GetItemID());
		csItemString.Empty();
		pItem->AppendItemData(pStorageItem, csItemString);
		CMFCPropertyGridProperty *pProperty = new CMFCPropertyGridProperty(csName, static_cast<_variant_t>(csItemString), (LPCTSTR)0, 0UL, (LPCTSTR)0, (LPCTSTR)0, csHexValidChars);
		pProperty->AllowEdit(FALSE);
		m_propertyGridCtrl.AddProperty(pProperty);
		AddProperties(pItem, pStorageItem, TRUE);
	}
	else
	{
		switch (nServiceID)
		{
		case DiagnosticService::OtherServiceID::NoAction:
			TRACE(_T("No Action.\n"));
			break;
		case DiagnosticService::OtherServiceID::Download:
			TRACE(_T("Download.\n"));
			break;
		case DiagnosticService::OtherServiceID::Test:
			TRACE(_T("Test.\n"));
			break;
		default:
			TRACE(_T("Not implemented.\n"));
			// ASSERT(FALSE);
		}
	}
	m_propertyGridCtrl.SetRedraw(TRUE);
	m_propertyGridCtrl.RedrawWindow();
}

void CUDSonCANView::AddProperties(const DiagnosticService::CItem *pItem, DiagnosticStorage::CItem *pStorageItem, BOOL bShow)
{
	if (pStorageItem)
	{
		CMFCPropertyGridProperty *pProperty;
		CString csHexValidChars;
		csHexValidChars.LoadString(IDS_VALIDCHARS_HEXSPACE);
		const DiagnosticService::SectionVector &vpSection = pItem->GetSectionVector();
		DiagnosticStorage::CSection *pStorageSection;
		for (DiagnosticService::SectionVector::const_iterator iterSection = vpSection.cbegin(); iterSection != vpSection.cend(); ++iterSection)
		{
			pStorageSection = pStorageItem->GetSection((*iterSection)->GetSectionID());
			switch ((*iterSection)->GetDataType())
			{
			case DiagnosticService::DataType::Enumerate:
				{
					pProperty = new CMFCPropertyGridProperty((*iterSection)->GetCaption(), (_variant_t)(*iterSection)->GetOutputString(pStorageSection), (LPCTSTR)0, reinterpret_cast<DWORD_PTR>(pStorageSection));
					pProperty->AllowEdit(FALSE);
					const DiagnosticService::ItemVector &vpItem = (*iterSection)->GetItemVector();
					for (DiagnosticService::ItemVector::const_iterator iterItem = vpItem.cbegin(); iterItem != vpItem.cend(); ++iterItem)
					{
						pProperty->AddOption((*iterSection)->GetItemCaptionString((*iterItem)->GetItemID()));
					}
					m_propertyGridCtrl.AddProperty(pProperty);
					for (DiagnosticService::ItemVector::const_iterator iterItem = vpItem.cbegin(); iterItem != vpItem.cend(); ++iterItem)
					{
						AddProperties(*iterItem, pStorageSection->GetItem((*iterItem)->GetItemID()), bShow && pStorageSection->GetEnumerate() == (*iterItem)->GetItemID());
					}
					break;
				}
			case DiagnosticService::DataType::Binary:
				{
					CString csDataString;
					(*iterSection)->AppendData(pStorageSection, csDataString);
					pProperty = new CMFCPropertyGridProperty((*iterSection)->GetCaption(), (_variant_t)csDataString, (LPCTSTR)0, reinterpret_cast<DWORD_PTR>(pStorageSection), (LPCTSTR)0, (LPCTSTR)0, csHexValidChars);
					pProperty->Show(bShow);
					m_propertyGridCtrl.AddProperty(pProperty);
					break;
				}
			default:
				ASSERT(FALSE);
			}
		}
	}
}

void CUDSonCANView::SwitchProperty(CMFCPropertyGridProperty *pProp)
{
	if (!pProp->GetData())
	{
		return;
	}
	BOOL bUpdatePropertyGrid = 0 != m_propertyGridCtrl.GetPropertyCount();
	DiagnosticStorage::CSection *pStorageSection = reinterpret_cast<DiagnosticStorage::CSection *>(pProp->GetData());
	const DiagnosticService::CSection *pSection = pStorageSection->GetAssociatedDiagnosticServiceSection();
	
	if (pSection->GetDataType() == DiagnosticService::DataType::Enumerate)	// 属于 DiagnosticService::DataType::Enumerate
	{
		if (bUpdatePropertyGrid)
		{
			m_propertyGridCtrl.SetRedraw(FALSE);
			ShowSubItemSection(pStorageSection->GetItem(pStorageSection->GetEnumerate()), FALSE);
		}
		PushEnumerateData(pStorageSection, pProp->GetValue().bstrVal);
		if (bUpdatePropertyGrid)
		{
			ShowSubItemSection(pStorageSection->GetItem(pStorageSection->GetEnumerate()), TRUE);
			m_propertyGridCtrl.SetRedraw(TRUE);
			m_propertyGridCtrl.RedrawWindow();
		}
	}
	else
	{
		PushBinaryData(pStorageSection, pProp->GetValue().bstrVal);
		pProp->SetValue(static_cast<_variant_t>(pSection->GetOutputString(pStorageSection)));
	}
	if (bUpdatePropertyGrid)
	{
		CMFCPropertyGridProperty *pPropPDU = m_propertyGridCtrl.GetProperty(0);
		const DiagnosticService::CItem *pRootItem = theApp.GetDiagnosticService().GetItem(((CMainFrame *)AfxGetMainWnd())->GetCurrentServiceID());
		CString csItemString;
		pRootItem->AppendItemData(
					GetDocument()->GetDiagnosticStorage()->GetItem(pRootItem->GetItemID()),
					csItemString
				);
		pPropPDU->SetValue(static_cast<_variant_t>(csItemString));
	}
}

void CUDSonCANView::ShowSubItemSection(const DiagnosticStorage::CItem *pStorageItem, BOOL bShow) const
{
	if (!pStorageItem)
	{
		return;
	}

	CMFCPropertyGridProperty *pPropSection;
	const DiagnosticService::CItem *pServiceItem = pStorageItem->GetAssociatedDiagnosticServiceItem();
	for (DiagnosticService::SectionVector::const_iterator iter = pServiceItem->GetSectionVector().cbegin(); iter != pServiceItem->GetSectionVector().cend(); ++iter)
	{
		if (pPropSection = m_propertyGridCtrl.FindItemByData(reinterpret_cast<DWORD_PTR>(pStorageItem->GetSection((*iter)->GetSectionID()))))
		{
			pPropSection->Show(bShow);
		}
	}
}

void CUDSonCANView::PushEnumerateData(DiagnosticStorage::CSection *pStorageSection, LPCTSTR lpszDataString)
{
	ASSERT(pStorageSection->GetAssociatedDiagnosticServiceSection()->GetDataType() == DiagnosticService::DataType::Enumerate);
	pStorageSection->ClearData();
	pStorageSection->PushData(PickupSubItemID(lpszDataString));
}

void CUDSonCANView::PushBinaryData(DiagnosticStorage::CSection *pStorageSection, LPCTSTR lpszDataString)
{
	const DiagnosticService::CSection *pSection = pStorageSection->GetAssociatedDiagnosticServiceSection();
	ASSERT(pSection->GetDataType() == DiagnosticService::DataType::Binary);
	int nData = 0;
	CString csDataString(lpszDataString);
	CString csTemp;
	pStorageSection->ClearData();
	if (csDataString.IsEmpty())
	{
		return;
	}
	csDataString.Replace(_T(" "), _T(""));
	if (pSection->GetDataSizeRequired())
	{
		int nDoubleDataSizeRequired = pSection->GetDataSizeRequired() * 2;
		if (csDataString.GetLength() > nDoubleDataSizeRequired)
		{
			csDataString.Delete(nDoubleDataSizeRequired, csDataString.GetLength() - nDoubleDataSizeRequired);
		}
	}
	int nDataLength = csDataString.GetLength();
	if ((nDataLength & 1) == 1)
	{
		csDataString.Insert(nDataLength - 1, _T('0'));
		++nDataLength;
	}
	for (int i = 0; i != nDataLength; i += 2)
	{
		csTemp = csDataString.Mid(i, 2);
		_stscanf_s(csTemp, _T("%X"), &nData);
		pStorageSection->PushData(nData);
	}
}

BYTE CUDSonCANView::PickupSubItemID(LPCTSTR lpszSubItemString) const
{
	int nData;
	CString csDataString(lpszSubItemString);
	CString csFormat;
	csFormat.LoadString(IDS_DIAGNOSTIC_SCANFORMAT);
	_stscanf_s(csDataString, csFormat, &nData);
	return nData;
}


void CUDSonCANView::OnToolsTest()
{
	CTestDialog *pTestDialog = new CTestDialog(GetDocument()->GetDiagnosticControl());
	pTestDialog->Create(IDD_TEST, NULL);
	pTestDialog->ShowWindow(SW_SHOW);
}


void CUDSonCANView::OnConfigPhysicalLayer()
{
	CDiagnosticControl &diagnosticControl = GetDocument()->GetDiagnosticControl();
	CCANConfigDialog canConfigDialog(diagnosticControl.GetPhysicalLayer());
	if (canConfigDialog.DoModal() == IDOK)
	{
		diagnosticControl.SaveConfig();
	}
}


void CUDSonCANView::OnConfigNetworkLayer()
{
	CDiagnosticControl &diagnosticControl = GetDocument()->GetDiagnosticControl();
	CNetworkLayerConfigDialog networkLayerConfigDialog(diagnosticControl.GetNetworkLayer());
	if (networkLayerConfigDialog.DoModal() == IDOK)
	{
		diagnosticControl.SaveConfig();
	}
}


void CUDSonCANView::OnConfigApplicationLayer()
{
	CDiagnosticControl &diagnosticControl = GetDocument()->GetDiagnosticControl();
	CApplicationLayerConfigDialog applicationLayerConfigDialog(diagnosticControl.GetApplicationLayer());
	if (applicationLayerConfigDialog.DoModal() == IDOK)
	{
		diagnosticControl.SaveConfig();
	}
}


void CUDSonCANView::OnDiagnosticBeginDiagnostic()
{
	CPhysicalLayer &physicalLayer = GetDocument()->GetDiagnosticControl().GetPhysicalLayer();
	if (physicalLayer.OpenDevice())
	{
		if (physicalLayer.StartCAN())
		{
			GetDocument()->GetDiagnosticControl().GetDataLinkLayer().SetNodeAddress(GetDocument()->GetDiagnosticControl().GetApplicationLayer().GetTesterPhysicalAddress());	// TODO: 将此处设定移动到 DiagnosticControl 中统一管理。
			GetDocument()->GetDiagnosticControl().ResetTiming();
			return;
		}
		physicalLayer.ResetCAN();
	}
	physicalLayer.CloseDevice();
	AfxMessageBox(IDS_BEGINDIAGNOSTICFAILED);
}


void CUDSonCANView::OnUpdateDiagnosticBeginDiagnostic(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(!GetDocument()->GetDiagnosticControl().GetPhysicalLayer().IsCANStarted());
}


void CUDSonCANView::OnDiagnosticStopDiagnostic()
{
	GetDocument()->GetDiagnosticControl().GetPhysicalLayer().ResetCAN();
	GetDocument()->GetDiagnosticControl().GetPhysicalLayer().CloseDevice();
}


void CUDSonCANView::OnUpdateDiagnosticStopDiagnostic(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(GetDocument()->GetDiagnosticControl().GetPhysicalLayer().IsCANStarted());
}


void CUDSonCANView::OnDiagnosticExecute()
{
	m_propertyGridCtrl.EndEditItem();
	BYTEVector vbyData;
	const CApplicationLayer &applicationLayer = GetDocument()->GetDiagnosticControl().GetApplicationLayer();
	const DiagnosticService::CItem *pRootItem = theApp.GetDiagnosticService().GetItem(m_nServiceID);
	if (pRootItem->AppendItemData(GetDocument()->GetDiagnosticStorage()->GetItem(pRootItem->GetItemID()), vbyData))
	{
		applicationLayer.Request(vbyData);
	}
	else
	{
		AfxMessageBox(IDS_NOTALLSECTIONFULFILLED);
	}
}


void CUDSonCANView::OnUpdateDiagnosticExecute(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(GetDocument()->GetDiagnosticControl().GetPhysicalLayer().IsCANStarted()
		&& m_nServiceID != DiagnosticService::OtherServiceID::NoAction);
}


int CUDSonCANView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	if (!m_propertyGridCtrl.Create(WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_BORDER, rectDummy, this, 1))
	{
		OutputDebugString(_T("未能创建属性网格\n"));
	}
	InitPropertyGridCtrl();

	return 0;
}
