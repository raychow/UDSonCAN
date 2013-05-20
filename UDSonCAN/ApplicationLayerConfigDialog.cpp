// ApplicationLayerConfigDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "UDSonCAN.h"
#include "ApplicationLayerConfigDialog.h"
#include "afxdialogex.h"

#include "ApplicationLayer.h"

// CApplicationLayerConfigDialog 对话框

IMPLEMENT_DYNAMIC(CApplicationLayerConfigDialog, CDialogEx)

CApplicationLayerConfigDialog::CApplicationLayerConfigDialog(CApplicationLayer &applicationLayer, CWnd* pParent /*=NULL*/)
	: CDialogEx(CApplicationLayerConfigDialog::IDD, pParent)
	, m_csTesterPhysicalAddress(_T(""))
	, m_csECUAddress(_T(""))
	, m_bECUFunctionalAddress(FALSE)
	, m_bRemoteDiagnostic(FALSE)
	, m_csRemoteDiagnosticAddress(_T(""))
	, m_nP2CANClient(0)
	, m_nP2SCANClient(0)
	, m_nP3CANClientPhys(0)
	, m_nP3CANClientFunc(0)
	, m_nS3Client(0)
	, m_applicationLayer(applicationLayer)
{

}

CApplicationLayerConfigDialog::~CApplicationLayerConfigDialog()
{
}

void CApplicationLayerConfigDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_MFCMASKEDEDIT_APPLICATIONLAYERCONFIG_TESTERPHYSICALADDRESS, m_csTesterPhysicalAddress);
	DDX_Text(pDX, IDC_MFCMASKEDEDIT_APPLICATIONLAYERCONFIG_ECUADDRESS, m_csECUAddress);
	DDX_Check(pDX, IDC_CHECK_APPLICATIONLAYERCONFIG_ECUFUNCTIONALADDRESS, m_bECUFunctionalAddress);
	DDX_Check(pDX, IDC_CHECK_APPLICATIONLAYERCONFIG_REMOTEDIAGNOSTIC, m_bRemoteDiagnostic);
	DDX_Text(pDX, IDC_MFCMASKEDEDIT_APPLICATIONLAYERCONFIG_REMOTEDIAGNOSTICADDRESS, m_csRemoteDiagnosticAddress);
	DDX_Text(pDX, IDC_EDIT_APPLICATIONLAYERCONFIG_P2CANCLIENT, m_nP2CANClient);
	DDX_Text(pDX, IDC_EDIT_APPLICATIONLAYERCONFIG_P2SCANCLIENT, m_nP2SCANClient);
	DDX_Text(pDX, IDC_EDIT_APPLICATIONLAYERCONFIG_P3CANCLIENTPHYS, m_nP3CANClientPhys);
	DDX_Text(pDX, IDC_EDIT_APPLICATIONLAYERCONFIG_P3SCANCLIENTFUNC, m_nP3CANClientFunc);
	DDX_Text(pDX, IDC_EDIT_APPLICATIONLAYERCONFIG_S3CLIENT, m_nS3Client);
}


BEGIN_MESSAGE_MAP(CApplicationLayerConfigDialog, CDialogEx)
	ON_BN_CLICKED(IDC_CHECK_APPLICATIONLAYERCONFIG_REMOTEDIAGNOSTIC, &CApplicationLayerConfigDialog::OnClickedCheckDiagnosticconfigRemotediagnostic)
END_MESSAGE_MAP()


// CApplicationLayerConfigDialog 消息处理程序


BOOL CApplicationLayerConfigDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CString csTemp;
	CMFCMaskedEdit *pMFCMaskedEdit;
	csTemp.LoadString(IDS_VALIDCHARS_HEX);
	pMFCMaskedEdit = static_cast<CMFCMaskedEdit *>(GetDlgItem(IDC_MFCMASKEDEDIT_APPLICATIONLAYERCONFIG_TESTERPHYSICALADDRESS));
	pMFCMaskedEdit->SetValidChars(csTemp);
	pMFCMaskedEdit->SetValidChars(csTemp);
	pMFCMaskedEdit = static_cast<CMFCMaskedEdit *>(GetDlgItem(IDC_MFCMASKEDEDIT_APPLICATIONLAYERCONFIG_ECUADDRESS));
	pMFCMaskedEdit->SetValidChars(csTemp);
	pMFCMaskedEdit = static_cast<CMFCMaskedEdit *>(GetDlgItem(IDC_MFCMASKEDEDIT_APPLICATIONLAYERCONFIG_REMOTEDIAGNOSTICADDRESS));
	pMFCMaskedEdit->SetValidChars(csTemp);

	_LoadConfig();

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CApplicationLayerConfigDialog::_LoadConfig()
{
	BYTE byTemp;
	byTemp = m_applicationLayer.GetTesterPhysicalAddress();
	m_csTesterPhysicalAddress.Format(_T("%02X"), byTemp);
	byTemp = m_applicationLayer.GetECUAddress();
	m_csECUAddress.Format(_T("%02X"), byTemp);
	m_bECUFunctionalAddress = m_applicationLayer.IsECUFunctionalAddress();
	m_bRemoteDiagnostic = m_applicationLayer.IsRemoteDiagnostic();
	byTemp = m_applicationLayer.GetRemoteDiagnosticAddress();
	m_csRemoteDiagnosticAddress.Format(_T("%02X"), byTemp);
	m_nP2CANClient = m_applicationLayer.GetP2CANClient();
	m_nP2SCANClient = m_applicationLayer.GetP2SCANClient();
	m_nP3CANClientPhys = m_applicationLayer.GetP3CANClientPhys();
	m_nP3CANClientFunc = m_applicationLayer.GetP3CANClientFunc();
	m_nS3Client = m_applicationLayer.GetS3Client();

	UpdateData(FALSE);
	_UpdateUIRemoteDiagnostic();
}

void CApplicationLayerConfigDialog::_UpdateUIRemoteDiagnostic()
{
	CButton *pButton = static_cast<CButton *>(GetDlgItem(IDC_CHECK_APPLICATIONLAYERCONFIG_REMOTEDIAGNOSTIC));
	CMFCMaskedEdit *pMFCMaskedEdit = static_cast<CMFCMaskedEdit *>(GetDlgItem(IDC_MFCMASKEDEDIT_APPLICATIONLAYERCONFIG_REMOTEDIAGNOSTICADDRESS));
	if (BST_CHECKED == pButton->GetCheck())
	{
		pMFCMaskedEdit->EnableWindow(TRUE);
	}
	else
	{
		pMFCMaskedEdit->EnableWindow(FALSE);
	}
}

void CApplicationLayerConfigDialog::OnClickedCheckDiagnosticconfigRemotediagnostic()
{
	_UpdateUIRemoteDiagnostic();
}


void CApplicationLayerConfigDialog::OnOK()
{
	UpdateData(TRUE);

	UINT nTemp = 0;
	_stscanf_s(m_csTesterPhysicalAddress, _T("%02X"), &nTemp);
	m_applicationLayer.SetTesterPhysicalAddress(nTemp);
	_stscanf_s(m_csECUAddress, _T("%02X"), &nTemp);
	m_applicationLayer.SetECUAddress(nTemp);
	m_applicationLayer.SetECUFunctionalAddress(m_bECUFunctionalAddress);
	m_applicationLayer.SetRemoteDiagnostic(m_bRemoteDiagnostic);
	_stscanf_s(m_csRemoteDiagnosticAddress, _T("%02X"), &nTemp);
	m_applicationLayer.SetRemoteDiagnosticAddress(nTemp);
	m_applicationLayer.SetP2CANClient(m_nP2CANClient);
	m_applicationLayer.SetP2SCANClient(m_nP2SCANClient);
	m_applicationLayer.SetP3CANClientPhys(m_nP3CANClientPhys);
	m_applicationLayer.SetP3CANClientFunc(m_nP3CANClientFunc);
	m_applicationLayer.SetS3Client(m_nS3Client);

	CDialogEx::OnOK();
}
