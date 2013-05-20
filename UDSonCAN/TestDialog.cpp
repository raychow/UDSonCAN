// TestDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "UDSonCAN.h"
#include "TestDialog.h"
#include "afxdialogex.h"

#include <vector>

using std::vector;

// CTestDialog 对话框

IMPLEMENT_DYNAMIC(CTestDialog, CDialogEx)

CTestDialog::CTestDialog(CDiagnosticControl &diagnosticControl, CWnd* pParent /*=NULL*/)
	: CDialogEx(CTestDialog::IDD, pParent)
	, m_diagnosticControl(diagnosticControl)
	, m_csNrequestData(_T(""))
	, m_csNrequestSA(_T(""))
	, m_csNrequestTA(_T(""))
	, m_csFakeSFID(_T(""))
	, m_csFakeSFRequestData(_T(""))
	, m_pFakeCFThread(NULL)
{
	m_diagnosticControl.GetDataLinkLayer().SetNodeAddress(0x11);
}

CTestDialog::~CTestDialog()
{
}

void CTestDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_TEST_NREQUESTDATA, m_csNrequestData);
	DDX_Text(pDX, IDC_EDIT_TEST_NREQUESTSA, m_csNrequestSA);
	DDX_Text(pDX, IDC_EDIT_TEST_NREQUESTTA, m_csNrequestTA);
	DDX_Text(pDX, IDC_EDIT_TEST_FAKESFID, m_csFakeSFID);
	DDX_Text(pDX, IDC_EDIT_TEST_FAKESFREQUESTDATA, m_csFakeSFRequestData);
}


BEGIN_MESSAGE_MAP(CTestDialog, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_TEST_OPENDEVICE, &CTestDialog::OnClickedButtonTestOpendevice)
	ON_BN_CLICKED(IDC_BUTTON_TEST_CLOSEDEVICE, &CTestDialog::OnClickedButtonTestClosedevice)
	ON_BN_CLICKED(IDC_BUTTON_TEST_OPENCAN, &CTestDialog::OnClickedButtonTestOpencan)
	ON_BN_CLICKED(IDC_BUTTON_TEST_RESETCAN, &CTestDialog::OnClickedButtonTestResetcan)
	ON_BN_CLICKED(IDC_BUTTON_TEST_NREQUEST, &CTestDialog::OnBnClickedButtonTestNrequest)
	ON_BN_CLICKED(IDC_BUTTON_TEST_FAKESF, &CTestDialog::OnBnClickedButtonTestFakesf)
	ON_BN_CLICKED(IDC_BUTTON_TEST_FAKEMF, &CTestDialog::OnBnClickedButtonTestFakemf)
	ON_BN_CLICKED(IDC_BUTTON_TEST_FAKEFF, &CTestDialog::OnBnClickedButtonTestFakeff)
	ON_BN_CLICKED(IDC_BUTTON_TEST_FAKECF, &CTestDialog::OnBnClickedButtonTestFakecf)
	ON_BN_CLICKED(IDC_BUTTON_TEST_FAKEFC, &CTestDialog::OnBnClickedButtonTestFakefc)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CTestDialog 消息处理程序


void CTestDialog::OnClickedButtonTestOpendevice()
{
	_MessagePrompt(m_diagnosticControl.GetPhysicalLayer().OpenDevice());
}


void CTestDialog::OnClickedButtonTestClosedevice()
{
	_MessagePrompt(m_diagnosticControl.GetPhysicalLayer().CloseDevice());
}


void CTestDialog::OnClickedButtonTestOpencan()
{
	_MessagePrompt(m_diagnosticControl.GetPhysicalLayer().StartCAN());
}


void CTestDialog::OnClickedButtonTestResetcan()
{
	_MessagePrompt(m_diagnosticControl.GetPhysicalLayer().ResetCAN());
}

void CTestDialog::_MessagePrompt(BOOL bResult) const
{
	if (bResult)
	{
		TRACE(_T("Success.\n"));
	}
	else
	{
		TRACE(_T("Failed.\n"));
	}
}

void CTestDialog::OnBnClickedButtonTestNrequest()
{
	UpdateData(TRUE);
	m_csNrequestData.Remove(_T(' '));
	UINT nDataLength = m_csNrequestData.GetLength();
	if ((nDataLength & 1) == 1)
	{
		m_csNrequestData.Insert(nDataLength - 1, _T('0'));
		++nDataLength;
	}
	UpdateData(FALSE);
	vector<BYTE> vbyData;
	CString csTemp;
	UINT nData;
	for (int i = 0; i != nDataLength; i += 2)
	{
		csTemp = m_csNrequestData.Mid(i, 2);
		_stscanf_s(csTemp, _T("%X"), &nData);
		vbyData.push_back(nData);
	}

	m_diagnosticControl.GetNetworkLayer().Request(CNetworkLayer::MessageType::Diagnostics, _GetNrequestSA(), _GetNrequestTA(), CNetworkLayer::TargetAddressType::Physical, 0x33, vbyData);
}

BYTE CTestDialog::_GetNrequestSA() const
{
	UINT nNrequestSA;
	_stscanf_s(m_csNrequestSA, _T("%2X"), &nNrequestSA);
	return nNrequestSA;
}

BYTE CTestDialog::_GetNrequestTA() const
{
	UINT nNrequestTA;
	_stscanf_s(m_csNrequestTA, _T("%2X"), &nNrequestTA);
	return nNrequestTA;
}

BOOL CTestDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_csNrequestSA = _T("11");
	m_csNrequestTA = _T("22");
	m_csFakeSFID = _T("18DA1122");
	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
}


void CTestDialog::OnBnClickedButtonTestFakesf()
{
	UpdateData(TRUE);
	CSingleLock lockReceiveData(&m_diagnosticControl.GetPhysicalLayer().m_csectionReceiveData);
	lockReceiveData.Lock();
	m_diagnosticControl.GetPhysicalLayer().m_lReceivedLength = 1;
	UINT nID;
	_stscanf_s(m_csFakeSFID, _T("%X"), &nID);

	CString csDataString(m_csFakeSFRequestData);
	CString csTemp;
	UINT nData;

	csDataString.Replace(_T(" "), _T(""));
	int nDataLength = csDataString.GetLength();
	if ((nDataLength & 1) == 1)
	{
		csDataString.Insert(nDataLength - 1, _T('0'));
		++nDataLength;
	}
	nDataLength = min(16, nDataLength);
	ZeroMemory(m_diagnosticControl.GetPhysicalLayer().m_canObj.Data, sizeof(m_diagnosticControl.GetPhysicalLayer().m_canObj.Data));
	for (int i = 0; i != nDataLength; i += 2)
	{
		csTemp = csDataString.Mid(i, 2);
		_stscanf_s(csTemp, _T("%X"), &nData);
		m_diagnosticControl.GetPhysicalLayer().m_canObj.Data[i / 2] = nData;
	}
	m_diagnosticControl.GetPhysicalLayer().m_canObj.DataLen = 8;
	m_diagnosticControl.GetPhysicalLayer().m_canObj.ExternFlag = 0;
	m_diagnosticControl.GetPhysicalLayer().m_canObj.RemoteFlag = 0;
	m_diagnosticControl.GetPhysicalLayer().m_canObj.SendType = 0;
	m_diagnosticControl.GetPhysicalLayer().m_canObj.ID = nID;
}


void CTestDialog::OnBnClickedButtonTestFakemf()
{
	OnBnClickedButtonTestFakeff();
	OnBnClickedButtonTestFakecf();
}


void CTestDialog::OnBnClickedButtonTestFakeff()
{
	CSingleLock lockReceiveData(&m_diagnosticControl.GetPhysicalLayer().m_csectionReceiveData);
	lockReceiveData.Lock();
	m_diagnosticControl.GetPhysicalLayer().m_canObj.Data[0] = 0x10;
	m_diagnosticControl.GetPhysicalLayer().m_canObj.Data[1] = 0x14;
	m_diagnosticControl.GetPhysicalLayer().m_canObj.Data[2] = 0x11;
	m_diagnosticControl.GetPhysicalLayer().m_canObj.Data[3] = 0x22;
	m_diagnosticControl.GetPhysicalLayer().m_canObj.Data[4] = 0x33;
	m_diagnosticControl.GetPhysicalLayer().m_canObj.Data[5] = 0x44;
	m_diagnosticControl.GetPhysicalLayer().m_canObj.Data[6] = 0x55;
	m_diagnosticControl.GetPhysicalLayer().m_canObj.Data[7] = 0x66;
	m_diagnosticControl.GetPhysicalLayer().m_canObj.DataLen = 8;
	m_diagnosticControl.GetPhysicalLayer().m_canObj.ExternFlag = 0;
	m_diagnosticControl.GetPhysicalLayer().m_canObj.ID = 0x18DA1122;
	m_diagnosticControl.GetPhysicalLayer().m_canObj.RemoteFlag = 0;
	m_diagnosticControl.GetPhysicalLayer().m_canObj.SendType = 0;
	m_diagnosticControl.GetPhysicalLayer().m_lReceivedLength = 1;
	lockReceiveData.Unlock();
}

UINT CTestDialog::FakeCFThread(LPVOID lpParam)
{
	CTestDialog *pThis = static_cast<CTestDialog *>(lpParam);
	CSingleLock lockReceiveData(&pThis->m_diagnosticControl.GetPhysicalLayer().m_csectionReceiveData);
	Sleep(400);
	lockReceiveData.Lock();
	pThis->m_diagnosticControl.GetPhysicalLayer().m_canObj.Data[0] = 0x21;
	pThis->m_diagnosticControl.GetPhysicalLayer().m_canObj.Data[1] = 0x77;
	pThis->m_diagnosticControl.GetPhysicalLayer().m_canObj.Data[2] = 0x88;
	pThis->m_diagnosticControl.GetPhysicalLayer().m_canObj.Data[3] = 0x99;
	pThis->m_diagnosticControl.GetPhysicalLayer().m_canObj.Data[4] = 0xAA;
	pThis->m_diagnosticControl.GetPhysicalLayer().m_canObj.Data[5] = 0xBB;
	pThis->m_diagnosticControl.GetPhysicalLayer().m_canObj.Data[6] = 0xCC;
	pThis->m_diagnosticControl.GetPhysicalLayer().m_canObj.Data[7] = 0xDD;
	pThis->m_diagnosticControl.GetPhysicalLayer().m_canObj.DataLen = 8;
	pThis->m_diagnosticControl.GetPhysicalLayer().m_canObj.ExternFlag = 0;
	pThis->m_diagnosticControl.GetPhysicalLayer().m_canObj.ID = 0x18DA1122;
	pThis->m_diagnosticControl.GetPhysicalLayer().m_canObj.RemoteFlag = 0;
	pThis->m_diagnosticControl.GetPhysicalLayer().m_canObj.SendType = 0;
	pThis->m_diagnosticControl.GetPhysicalLayer().m_lReceivedLength = 1;
	lockReceiveData.Unlock();
	Sleep(400);
	lockReceiveData.Lock();
	pThis->m_diagnosticControl.GetPhysicalLayer().m_canObj.Data[0] = 0x21;
	pThis->m_diagnosticControl.GetPhysicalLayer().m_canObj.Data[1] = 0x77;
	pThis->m_diagnosticControl.GetPhysicalLayer().m_canObj.Data[2] = 0x88;
	pThis->m_diagnosticControl.GetPhysicalLayer().m_canObj.Data[3] = 0x99;
	pThis->m_diagnosticControl.GetPhysicalLayer().m_canObj.Data[4] = 0xAA;
	pThis->m_diagnosticControl.GetPhysicalLayer().m_canObj.Data[5] = 0xBB;
	pThis->m_diagnosticControl.GetPhysicalLayer().m_canObj.Data[6] = 0xCC;
	pThis->m_diagnosticControl.GetPhysicalLayer().m_canObj.Data[7] = 0xDD;
	pThis->m_diagnosticControl.GetPhysicalLayer().m_canObj.DataLen = 8;
	pThis->m_diagnosticControl.GetPhysicalLayer().m_canObj.ExternFlag = 0;
	pThis->m_diagnosticControl.GetPhysicalLayer().m_canObj.ID = 0x18DA1122;
	pThis->m_diagnosticControl.GetPhysicalLayer().m_canObj.RemoteFlag = 0;
	pThis->m_diagnosticControl.GetPhysicalLayer().m_canObj.SendType = 0;
	pThis->m_diagnosticControl.GetPhysicalLayer().m_lReceivedLength = 1;
	lockReceiveData.Unlock();
	Sleep(400);
	lockReceiveData.Lock();
	pThis->m_diagnosticControl.GetPhysicalLayer().m_canObj.Data[0] = 0x22;
	pThis->m_diagnosticControl.GetPhysicalLayer().m_canObj.Data[1] = 0x77;
	pThis->m_diagnosticControl.GetPhysicalLayer().m_canObj.Data[2] = 0x88;
	pThis->m_diagnosticControl.GetPhysicalLayer().m_canObj.Data[3] = 0x99;
	pThis->m_diagnosticControl.GetPhysicalLayer().m_canObj.Data[4] = 0xAA;
	pThis->m_diagnosticControl.GetPhysicalLayer().m_canObj.Data[5] = 0xBB;
	pThis->m_diagnosticControl.GetPhysicalLayer().m_canObj.Data[6] = 0xCC;
	pThis->m_diagnosticControl.GetPhysicalLayer().m_canObj.Data[7] = 0xDD;
	pThis->m_diagnosticControl.GetPhysicalLayer().m_canObj.DataLen = 8;
	pThis->m_diagnosticControl.GetPhysicalLayer().m_canObj.ExternFlag = 0;
	pThis->m_diagnosticControl.GetPhysicalLayer().m_canObj.ID = 0x18DA1122;
	pThis->m_diagnosticControl.GetPhysicalLayer().m_canObj.RemoteFlag = 0;
	pThis->m_diagnosticControl.GetPhysicalLayer().m_canObj.SendType = 0;
	pThis->m_diagnosticControl.GetPhysicalLayer().m_lReceivedLength = 1;
	lockReceiveData.Unlock();
	pThis->m_pFakeCFThread = NULL;
	return 0;
}

void CTestDialog::OnBnClickedButtonTestFakecf()
{
	if (!m_pFakeCFThread)
	{
		m_pFakeCFThread = AfxBeginThread(FakeCFThread, this, 0U, CREATE_SUSPENDED);
		m_pFakeCFThread->m_bAutoDelete = TRUE;
		m_pFakeCFThread->ResumeThread();
	}
}

void CTestDialog::OnBnClickedButtonTestFakefc()
{
	CSingleLock lockReceiveData(&m_diagnosticControl.GetPhysicalLayer().m_csectionReceiveData);
	lockReceiveData.Lock();
	m_diagnosticControl.GetPhysicalLayer().m_canObj.Data[0] = 0x30;
	m_diagnosticControl.GetPhysicalLayer().m_canObj.Data[1] = 0x00;
	m_diagnosticControl.GetPhysicalLayer().m_canObj.Data[2] = 0x7F;
	m_diagnosticControl.GetPhysicalLayer().m_canObj.Data[3] = 0x00;
	m_diagnosticControl.GetPhysicalLayer().m_canObj.Data[4] = 0x00;
	m_diagnosticControl.GetPhysicalLayer().m_canObj.Data[5] = 0x00;
	m_diagnosticControl.GetPhysicalLayer().m_canObj.Data[6] = 0x00;
	m_diagnosticControl.GetPhysicalLayer().m_canObj.Data[7] = 0x00;
	m_diagnosticControl.GetPhysicalLayer().m_canObj.DataLen = 8;
	m_diagnosticControl.GetPhysicalLayer().m_canObj.ExternFlag = 0;
	m_diagnosticControl.GetPhysicalLayer().m_canObj.ID = 0x18DA1122;
	m_diagnosticControl.GetPhysicalLayer().m_canObj.RemoteFlag = 0;
	m_diagnosticControl.GetPhysicalLayer().m_canObj.SendType = 0;
	m_diagnosticControl.GetPhysicalLayer().m_lReceivedLength = 1;
	lockReceiveData.Unlock();
}


void CTestDialog::OnOK()
{
	CDialogEx::OnOK();
	//OnDestroy();
}


void CTestDialog::OnCancel()
{
	CDialogEx::OnCancel();
	//OnDestroy();
}


void CTestDialog::OnDestroy()
{
	CDialogEx::OnDestroy();
	delete this;
}
