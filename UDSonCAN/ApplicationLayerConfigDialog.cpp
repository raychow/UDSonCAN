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
	, m_csECUPhysicalAddress(_T(""))
	, m_csECUFunctionalAddress(_T(""))
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
	DDX_Text(pDX, IDC_MFCMASKEDEDIT_APPLICATIONLAYERCONFIG_ECUPHYSICALADDRESS, m_csECUPhysicalAddress);
	DDX_Text(pDX, IDC_MFCMASKEDEDIT_APPLICATIONLAYERCONFIG_ECUFUNCTIONALADDRESS, m_csECUFunctionalAddress);
	DDX_Text(pDX, IDC_EDIT_APPLICATIONLAYERCONFIG_P2CANCLIENT, m_nP2CANClient);
	DDX_Text(pDX, IDC_EDIT_APPLICATIONLAYERCONFIG_P2SCANCLIENT, m_nP2SCANClient);
	DDX_Text(pDX, IDC_EDIT_APPLICATIONLAYERCONFIG_P3CANCLIENTPHYS, m_nP3CANClientPhys);
	DDX_Text(pDX, IDC_EDIT_APPLICATIONLAYERCONFIG_P3SCANCLIENTFUNC, m_nP3CANClientFunc);
	DDX_Text(pDX, IDC_EDIT_APPLICATIONLAYERCONFIG_S3CLIENT, m_nS3Client);
}

// CApplicationLayerConfigDialog 消息处理程序


BOOL CApplicationLayerConfigDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CString csValidCharsHex;
	csValidCharsHex.LoadString(IDS_VALIDCHARS_HEX);
	static_cast<CMFCMaskedEdit *>(GetDlgItem(IDC_MFCMASKEDEDIT_APPLICATIONLAYERCONFIG_TESTERPHYSICALADDRESS))->SetValidChars(csValidCharsHex);
	static_cast<CMFCMaskedEdit *>(GetDlgItem(IDC_MFCMASKEDEDIT_APPLICATIONLAYERCONFIG_ECUPHYSICALADDRESS))->SetValidChars(csValidCharsHex);
	static_cast<CMFCMaskedEdit *>(GetDlgItem(IDC_MFCMASKEDEDIT_APPLICATIONLAYERCONFIG_ECUFUNCTIONALADDRESS))->SetValidChars(csValidCharsHex);

	_LoadConfig();

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CApplicationLayerConfigDialog::_LoadConfig()
{
	BYTE byTemp;
	byTemp = m_applicationLayer.GetTesterPhysicalAddress();
	m_csTesterPhysicalAddress.Format(_T("%04X"), byTemp);
	byTemp = m_applicationLayer.GetECUPhysicalAddress();
	m_csECUPhysicalAddress.Format(_T("%04X"), byTemp);
	byTemp = m_applicationLayer.GetECUFunctionalAddress();
	m_csECUFunctionalAddress.Format(_T("%04X"), byTemp);
	m_nP2CANClient = m_applicationLayer.GetP2CANClient();
	m_nP2SCANClient = m_applicationLayer.GetP2SCANClient();
	m_nP3CANClientPhys = m_applicationLayer.GetP3CANClientPhys();
	m_nP3CANClientFunc = m_applicationLayer.GetP3CANClientFunc();
	m_nS3Client = m_applicationLayer.GetS3Client();

	UpdateData(FALSE);
}

void CApplicationLayerConfigDialog::OnOK()
{
	UpdateData(TRUE);

	UINT nTemp = 0;
	_stscanf_s(m_csTesterPhysicalAddress, _T("%04X"), &nTemp);
	m_applicationLayer.SetTesterPhysicalAddress(nTemp);
	_stscanf_s(m_csECUPhysicalAddress, _T("%04X"), &nTemp);
	m_applicationLayer.SetECUPhysicalAddress(nTemp);
	_stscanf_s(m_csECUFunctionalAddress, _T("%04X"), &nTemp);
	m_applicationLayer.SetECUFunctionalAddress(nTemp);
	m_applicationLayer.SetP2CANClient(m_nP2CANClient);
	m_applicationLayer.SetP2SCANClient(m_nP2SCANClient);
	m_applicationLayer.SetP3CANClientPhys(m_nP3CANClientPhys);
	m_applicationLayer.SetP3CANClientFunc(m_nP3CANClientFunc);
	m_applicationLayer.SetS3Client(m_nS3Client);

	CDialogEx::OnOK();
}
