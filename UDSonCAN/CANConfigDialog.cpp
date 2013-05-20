// CANConfigDialog.cpp : 实现文件
//

#include "stdafx.h"

#include "UDSonCAN.h"
#include "CANConfigDialog.h"
#include "afxdialogex.h"

#include "PhysicalLayer.h"

// CCANConfigDialog 对话框

IMPLEMENT_DYNAMIC(CCANConfigDialog, CDialogEx)

CCANConfigDialog::CCANConfigDialog(CPhysicalLayer &physicalLayer, CWnd* pParent /*=NULL*/)
	: CDialogEx(CCANConfigDialog::IDD, pParent)
	, m_nDeviceType(0)
	, m_nDeviceIndex(0)
	, m_nCANIndex(0)
	, m_csAccCode(_T(""))
	, m_csAccMask(_T(""))
	, m_nBaudRateType(0)
	, m_csTiming0(_T(""))
	, m_csTiming1(_T(""))
	, m_nFilter(0)
	, m_nMode(0)
	, m_physicalLayer(physicalLayer)
{
}

CCANConfigDialog::~CCANConfigDialog()
{
}

void CCANConfigDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_CBIndex(pDX, IDC_COMBO_CANCONFIG_DEVICETYPE, m_nDeviceType);
	DDX_CBIndex(pDX, IDC_COMBO_CANCONFIG_DEVICEINDEX, m_nDeviceIndex);
	DDX_CBIndex(pDX, IDC_COMBO_CANCONFIG_CANINDEX, m_nCANIndex);
	DDX_Text(pDX, IDC_MFCMASKEDEDIT_CANCONFIG_ACCCODE, m_csAccCode);
	DDX_Text(pDX, IDC_MFCMASKEDEDIT_CANCONFIG_ACCMASK, m_csAccMask);
	DDX_CBIndex(pDX, IDC_COMBO_CANCONFIG_BAUDRATETYPE, m_nBaudRateType);
	DDX_Text(pDX, IDC_MFCMASKEDEDIT_CANCONFIG_TIMING0, m_csTiming0);
	DDX_Text(pDX, IDC_MFCMASKEDEDIT_CANCONFIG_TIMING1, m_csTiming1);
	DDX_CBIndex(pDX, IDC_COMBO_CANCONFIG_FILTER, m_nFilter);
	DDX_CBIndex(pDX, IDC_COMBO_CANCONFIG_MODE, m_nMode);
}


BEGIN_MESSAGE_MAP(CCANConfigDialog, CDialogEx)
	ON_CBN_SELCHANGE(IDC_COMBO_CANCONFIG_BAUDRATETYPE, &CCANConfigDialog::OnCbnSelchangeComboCanconfigBaudratetype)
END_MESSAGE_MAP()


// CCANConfigDialog 消息处理程序


BOOL CCANConfigDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	CString csTemp;
	CMFCMaskedEdit *pMFCMaskedEdit;
	csTemp.LoadString(IDS_VALIDCHARS_HEX);
	pMFCMaskedEdit = static_cast<CMFCMaskedEdit *>(GetDlgItem(IDC_MFCMASKEDEDIT_CANCONFIG_ACCCODE));
	pMFCMaskedEdit->SetValidChars(csTemp);
	pMFCMaskedEdit = static_cast<CMFCMaskedEdit *>(GetDlgItem(IDC_MFCMASKEDEDIT_CANCONFIG_ACCMASK));
	pMFCMaskedEdit->SetValidChars(csTemp);
	pMFCMaskedEdit = static_cast<CMFCMaskedEdit *>(GetDlgItem(IDC_MFCMASKEDEDIT_CANCONFIG_TIMING0));
	pMFCMaskedEdit->SetValidChars(csTemp);
	pMFCMaskedEdit = static_cast<CMFCMaskedEdit *>(GetDlgItem(IDC_MFCMASKEDEDIT_CANCONFIG_TIMING1));
	pMFCMaskedEdit->SetValidChars(csTemp);

	CComboBox *pComboBox;
	pComboBox = static_cast<CComboBox *>(GetDlgItem(IDC_COMBO_CANCONFIG_DEVICETYPE));
	for (int i = 0; i != DEVICETYPECOUNT; ++i)
	{
		csTemp.LoadString(IDS_DEVICETYPE + i);
		pComboBox->AddString(csTemp);
	}
	pComboBox = static_cast<CComboBox *>(GetDlgItem(IDC_COMBO_CANCONFIG_DEVICEINDEX));
	for (int i = 0; i != DEVICEINDEXCOUNT; ++i)
	{
		csTemp.Format(_T("%d"), i);
		pComboBox->AddString(csTemp);
	}
	pComboBox = static_cast<CComboBox *>(GetDlgItem(IDC_COMBO_CANCONFIG_CANINDEX));
	for (UINT i = 0, nCANCount = m_physicalLayer.GetCANCount(); i != nCANCount; ++i)
	{
		csTemp.Format(_T("%d"), i);
		pComboBox->AddString(csTemp);
	}
	pComboBox = static_cast<CComboBox *>(GetDlgItem(IDC_COMBO_CANCONFIG_BAUDRATETYPE));
	for (int i = 0; i != BAUDRATETYPECOUNT; ++i)
	{
		csTemp.LoadString(IDS_BAUDRATETYPE + i);
		pComboBox->AddString(csTemp);
	}
	pComboBox = static_cast<CComboBox *>(GetDlgItem(IDC_COMBO_CANCONFIG_FILTER));
	csTemp.LoadString(IDS_FILTER_SINGLE);
	pComboBox->AddString(csTemp);
	csTemp.LoadString(IDS_FILTER_DUAL);
	pComboBox->AddString(csTemp);
	pComboBox = static_cast<CComboBox *>(GetDlgItem(IDC_COMBO_CANCONFIG_MODE));
	csTemp.LoadString(IDS_MODE_NORMAL);
	pComboBox->AddString(csTemp);
	csTemp.LoadString(IDS_MODE_LISTENONLY);
	pComboBox->AddString(csTemp);

	_LoadConfig();
	
	return TRUE;  // return TRUE unless you set the focus to a control

}

void CCANConfigDialog::_LoadConfig()
{
	m_nDeviceType = m_physicalLayer.GetDeviceType() - 1;
	m_nDeviceIndex = m_physicalLayer.GetDeviceIndex();
	m_nCANIndex = m_physicalLayer.GetCANIndex();
	DWORD dwTemp;
	UCHAR chTemp;
	dwTemp = m_physicalLayer.GetAccCode();
	m_csAccCode.Format(_T("%08X"), dwTemp);
	dwTemp = m_physicalLayer.GetAccMask();
	m_csAccMask.Format(_T("%08X"), dwTemp);
	m_nBaudRateType = m_physicalLayer.GetBaudRateType();
	chTemp = m_physicalLayer.GetTiming0();
	m_csTiming0.Format(_T("%02X"), chTemp);
	chTemp = m_physicalLayer.GetTiming1();
	m_csTiming1.Format(_T("%02X"), chTemp);
	m_nFilter = m_physicalLayer.GetFilter();
	m_nMode = m_physicalLayer.GetMode();

	UpdateData(FALSE);
	_UpdateUIBaudRate();
}


void CCANConfigDialog::OnOK()
{
	UpdateData(TRUE);
	m_physicalLayer.SetDeviceType(m_nDeviceType + 1);
	m_physicalLayer.SetDeviceIndex(m_nDeviceIndex);
	m_physicalLayer.SetCANIndex(m_nCANIndex);
	UINT nTemp = 0;
	_stscanf_s(m_csAccCode, _T("%08X"), &nTemp);
	m_physicalLayer.SetAccCode(nTemp);
	_stscanf_s(m_csAccMask, _T("%08X"), &nTemp);
	m_physicalLayer.SetAccMask(nTemp);
	m_physicalLayer.SetBaudRateType(m_nBaudRateType);
	_stscanf_s(m_csTiming0, _T("%02X"), &nTemp);
	m_physicalLayer.SetTiming0(nTemp);
	_stscanf_s(m_csTiming1, _T("%02X"), &nTemp);
	m_physicalLayer.SetTiming1(nTemp);
	m_physicalLayer.SetFilter(m_nFilter);
	m_physicalLayer.SetMode(m_nMode);

	CDialogEx::OnOK();
}


void CCANConfigDialog::OnCbnSelchangeComboCanconfigBaudratetype()
{
	_UpdateUIBaudRate();
}

void CCANConfigDialog::_UpdateUIBaudRate()
{
	CComboBox *pComboBox = static_cast<CComboBox *>(GetDlgItem(IDC_COMBO_CANCONFIG_BAUDRATETYPE));
	CMFCMaskedEdit *pTiming0Edit = static_cast<CMFCMaskedEdit *>(GetDlgItem(IDC_MFCMASKEDEDIT_CANCONFIG_TIMING0));
	CMFCMaskedEdit *pTiming1Edit = static_cast<CMFCMaskedEdit *>(GetDlgItem(IDC_MFCMASKEDEDIT_CANCONFIG_TIMING1));
	if (BAUDRATETYPECOUNT - 1 == pComboBox->GetCurSel())
	{
		pTiming0Edit->EnableWindow(TRUE);
		pTiming1Edit->EnableWindow(TRUE);
	}
	else
	{
		pTiming0Edit->EnableWindow(FALSE);
		pTiming1Edit->EnableWindow(FALSE);
	}
}