// NetworkLayerConfigDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "UDSonCAN.h"
#include "NetworkLayerConfigDialog.h"
#include "afxdialogex.h"

#include "NetworkLayer.h"

// CNetworkLayerConfigDialog 对话框

IMPLEMENT_DYNAMIC(CNetworkLayerConfigDialog, CDialogEx)

CNetworkLayerConfigDialog::CNetworkLayerConfigDialog(CNetworkLayer &networkLayer, CWnd* pParent /*=NULL*/)
	: CDialogEx(CNetworkLayerConfigDialog::IDD, pParent)
	, m_nWorkMode(0)
	, m_bySeparationTimeMin(0)
	, m_nBlockSize(0)
	, m_nWaitFrameTransimissionMax(0)
	, m_nNAs(0)
	, m_nNAr(0)
	, m_nNBs(0)
	, m_nNBr(0)
	, m_nNCs(0)
	, m_nNCr(0)
	, m_networkLayer(networkLayer)
{

}

CNetworkLayerConfigDialog::~CNetworkLayerConfigDialog()
{
}

void CNetworkLayerConfigDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_CBIndex(pDX, IDC_COMBO_NETWORKLAYERCONFIG_WORKMODE, m_nWorkMode);
	DDX_Text(pDX, IDC_EDIT_NETWORKLAYERCONFIG_SEPARATIONTIMEMIN, m_bySeparationTimeMin);
	DDX_Text(pDX, IDC_EDIT_NETWORKLAYERCONFIG_BLOCKSIZE, m_nBlockSize);
	DDX_Text(pDX, IDC_EDIT_NETWORKLAYERCONFIG_FCWAITTRANSMISSIONMAX, m_nWaitFrameTransimissionMax);
	DDX_Text(pDX, IDC_EDIT_NETWORKLAYERCONFIG_NAS, m_nNAs);
	DDX_Text(pDX, IDC_EDIT_NETWORKLAYERCONFIG_NAR, m_nNAr);
	DDX_Text(pDX, IDC_EDIT_NETWORKLAYERCONFIG_NBS, m_nNBs);
	DDX_Text(pDX, IDC_EDIT_NETWORKLAYERCONFIG_NBR, m_nNBr);
	DDX_Text(pDX, IDC_EDIT_NETWORKLAYERCONFIG_NCS, m_nNCs);
	DDX_Text(pDX, IDC_EDIT_NETWORKLAYERCONFIG_NCR, m_nNCr);
}


BEGIN_MESSAGE_MAP(CNetworkLayerConfigDialog, CDialogEx)
END_MESSAGE_MAP()

void CNetworkLayerConfigDialog::_LoadConfig()
{
	m_nWorkMode = m_networkLayer.IsFullDuplex();
	m_bySeparationTimeMin = m_networkLayer.GetSeparationTimeMin();
	m_nBlockSize = m_networkLayer.GetBlockSize();
	m_nWaitFrameTransimissionMax = m_networkLayer.GetWaitFrameTransimissionMax();
	m_nNAs = m_networkLayer.GetAs();
	m_nNAr = m_networkLayer.GetAr();
	m_nNBs = m_networkLayer.GetBs();
	m_nNBr = m_networkLayer.GetBr();
	m_nNCs = m_networkLayer.GetCs();
	m_nNCr = m_networkLayer.GetCr();

	UpdateData(FALSE);
}

// CNetworkLayerConfigDialog 消息处理程序


BOOL CNetworkLayerConfigDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CComboBox *pComboBox;
	CString csTemp;
	pComboBox = static_cast<CComboBox *>(GetDlgItem(IDC_COMBO_NETWORKLAYERCONFIG_WORKMODE));
	csTemp.LoadString(IDS_NETWORKLAYER_WORKMODE_HALFDUPLEX);
	pComboBox->AddString(csTemp);
	csTemp.LoadString(IDS_NETWORKLAYER_WORKMODE_FULLDUPLEX);
	pComboBox->AddString(csTemp);
	_LoadConfig();

	return TRUE;  // return TRUE unless you set the focus to a control
}


void CNetworkLayerConfigDialog::OnOK()
{
	UpdateData(TRUE);

	m_networkLayer.SetFullDuplex(1 == m_nWorkMode);
	m_networkLayer.SetSeparationTimeMin(m_bySeparationTimeMin);
	m_networkLayer.SetBlockSize(m_nBlockSize);
	m_networkLayer.SetWaitFrameTransimissionMax(m_nWaitFrameTransimissionMax);
	m_networkLayer.SetAs(m_nNAs);
	m_networkLayer.SetAr(m_nNAr);
	m_networkLayer.SetBs(m_nNBs);
	m_networkLayer.SetBr(m_nNBr);
	m_networkLayer.SetCs(m_nNCs);
	m_networkLayer.SetCr(m_nNCr);

	CDialogEx::OnOK();
}
