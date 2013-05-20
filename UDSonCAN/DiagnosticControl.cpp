#include "stdafx.h"
#include "DiagnosticControl.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "UDSonCAN.h"
#include "PhysicalLayer.h"
#include "DataLinkLayer.h"
#include "NetworkLayer.h"
#include "ApplicationLayer.h"

CDiagnosticControl::CDiagnosticControl(void)
{
	m_pPhysicalLayer	= new CPhysicalLayer();
	m_pDataLinkLayer	= new CDataLinkLayer();
	m_pNetworkLayer		= new CNetworkLayer();
	m_pApplicationLayer	= new CApplicationLayer();

	m_pPhysicalLayer->SetDataLinkLayer(*m_pDataLinkLayer);
	m_pDataLinkLayer->SetNetworkLayer(*m_pNetworkLayer);
	m_pNetworkLayer->SetApplicationLayer(*m_pApplicationLayer);

	m_pDataLinkLayer->SetPhysicalLayer(*m_pPhysicalLayer);
	m_pNetworkLayer->SetDataLinkLayer(*m_pDataLinkLayer);
	m_pApplicationLayer->SetNetworkLayer(*m_pNetworkLayer);

	m_pPhysicalLayer->SetDiagnosticControl(*this);
	m_pNetworkLayer->SetDiagnosticControl(*this);
	m_pApplicationLayer->SetDiagnosticControl(*this);

	TCHAR pcFilePath[MAX_PATH];
	GetModuleFileName(NULL, pcFilePath, MAX_PATH);
	CStringA csaTemp;
	m_csaConfigFilename = pcFilePath;
	m_csaConfigFilename = m_csaConfigFilename.Left(m_csaConfigFilename.ReverseFind('\\') + 1);
	csaTemp.LoadString(IDS_CANCONFIG_PATH);
	m_csaConfigFilename.Append(csaTemp);

	LoadConfig();
}


CDiagnosticControl::~CDiagnosticControl(void)
{
	m_pPhysicalLayer->CloseDevice();	// 物理层必须在网络层之前关闭，否则可能会在网络层析构后通知网络层。

	delete m_pPhysicalLayer;
	delete m_pDataLinkLayer;
	delete m_pNetworkLayer;
	delete m_pApplicationLayer;
}

CPhysicalLayer &CDiagnosticControl::GetPhysicalLayer()
{
	return *m_pPhysicalLayer;
}

CDataLinkLayer &CDiagnosticControl::GetDataLinkLayer()
{
	return *m_pDataLinkLayer;
}

CNetworkLayer &CDiagnosticControl::GetNetworkLayer()
{
	return *m_pNetworkLayer;
}

CApplicationLayer &CDiagnosticControl::GetApplicationLayer()
{
	return *m_pApplicationLayer;
}

void CDiagnosticControl::SetApplicationLayerWatchWnd(CTIDCWatchWnd &applicationLayerWatchWnd)
{
	m_pApplicationLayerWatchWnd = &applicationLayerWatchWnd;
}

void CDiagnosticControl::SetNetworkLayerWatchWnd(CTIDCWatchWnd &networkLayerWatchWnd)
{
	m_pNetworkLayerWatchWnd = &networkLayerWatchWnd;
}

void CDiagnosticControl::LoadConfig()
{
	using boost::property_tree::ptree;
	using boost::property_tree::xml_parser::read_xml;

	ptree ptRoot;
	ptree *pptChild = NULL;
	const ptree *pptNode = NULL;
	ptree ptEmpty;

	try
	{
		read_xml(m_csaConfigFilename.GetBuffer(), ptRoot);
	}
	catch (std::exception)
	{
	}
	pptChild = &ptRoot.get_child("UDSonCAN", ptEmpty);

	pptNode = &pptChild->get_child("CPhysicalLayer", ptEmpty);
	m_pPhysicalLayer->SetDeviceType(pptNode->get<DWORD>("DeviceType", 3));
	m_pPhysicalLayer->SetDeviceIndex(pptNode->get<DWORD>("DeviceIndex", 0));
	m_pPhysicalLayer->SetCANIndex(pptNode->get<DWORD>("CANIndex", 0));
	m_pPhysicalLayer->SetAccMask(pptNode->get<DWORD>("AccMask", 0));
	m_pPhysicalLayer->SetAccCode(pptNode->get<DWORD>("AccCode", 0));
	m_pPhysicalLayer->SetBaudRateType(pptNode->get<UINT>("BaudRateType", 0));
	m_pPhysicalLayer->SetTiming0(pptNode->get<UCHAR>("Timing0", 0));
	m_pPhysicalLayer->SetTiming1(pptNode->get<UCHAR>("Timing1", 0));
	m_pPhysicalLayer->SetFilter(pptNode->get<UCHAR>("Filter", 0));
	m_pPhysicalLayer->SetMode(pptNode->get<UCHAR>("Mode", 0));

	pptNode = &pptChild->get_child("CNetworkLayer", ptEmpty);
	m_pNetworkLayer->SetFullDuplex(pptNode->get<BOOL>("FullDuplex", FALSE));
	m_pNetworkLayer->SetSeparationTimeMin(pptNode->get<BYTE>("SeparationTimeMin", 0));
	m_pNetworkLayer->SetBlockSize(pptNode->get<BYTE>("BlockSize", 0));
	m_pNetworkLayer->SetWaitFrameTransimissionMax(pptNode->get<UINT>("WaitFrameTransimissionMax", 0));
	m_pNetworkLayer->SetAs(pptNode->get<UINT>("As", 1000));
	m_pNetworkLayer->SetAr(pptNode->get<UINT>("Ar", 1000));
	m_pNetworkLayer->SetBs(pptNode->get<UINT>("Bs", 1000));
	m_pNetworkLayer->SetBr(pptNode->get<UINT>("Br", 400));
	m_pNetworkLayer->SetCs(pptNode->get<UINT>("Cs", 400));
	m_pNetworkLayer->SetCr(pptNode->get<UINT>("Cr", 1000));

	pptNode = &pptChild->get_child("CApplicationLayer", ptEmpty);
	m_pApplicationLayer->SetTesterPhysicalAddress(pptNode->get<BYTE>("TesterPhysicalAddress", 0));
	m_pApplicationLayer->SetECUAddress(pptNode->get<BYTE>("ECUAddress", 0));
	m_pApplicationLayer->SetECUFunctionalAddress(pptNode->get<BOOL>("ECUFunctionalAddress", FALSE));
	m_pApplicationLayer->SetRemoteDiagnostic(pptNode->get<BOOL>("RemoteDiagnostic", FALSE));
	m_pApplicationLayer->SetRemoteDiagnosticAddress(pptNode->get<BYTE>("RemoteDiagnosticAddress", 0));
	m_pApplicationLayer->SetP2CANClient(pptNode->get<UINT>("P2CANClient", 1100));
	m_pApplicationLayer->SetP2SCANClient(pptNode->get<UINT>("P2SCANClient", 6000));
	m_pApplicationLayer->SetP3CANClientPhys(pptNode->get<UINT>("P3CANClientPhys", 50));
	m_pApplicationLayer->SetP3CANClientFunc(pptNode->get<UINT>("P3CANClientFunc", 50));
	m_pApplicationLayer->SetS3Client(pptNode->get<UINT>("S3Client", 2000));
}

void CDiagnosticControl::SaveConfig()
{
	using boost::property_tree::ptree;
	using boost::property_tree::xml_parser::write_xml;

	ptree ptRoot;
	ptree &ptChild = ptRoot.add("UDSonCAN", "");
	ptree *pptNode;

	pptNode = &ptChild.add("CPhysicalLayer", "");
	pptNode->put("DeviceType", m_pPhysicalLayer->GetDeviceType());
	pptNode->put("DeviceIndex", m_pPhysicalLayer->GetDeviceIndex());
	pptNode->put("CANIndex", m_pPhysicalLayer->GetCANIndex());
	pptNode->put("AccMask", m_pPhysicalLayer->GetAccMask());
	pptNode->put("AccCode", m_pPhysicalLayer->GetAccCode());
	pptNode->put("BaudRateType", m_pPhysicalLayer->GetBaudRateType());
	pptNode->put("Timing0", m_pPhysicalLayer->GetTiming0());
	pptNode->put("Timing1", m_pPhysicalLayer->GetTiming1());
	pptNode->put("Filter", m_pPhysicalLayer->GetFilter());
	pptNode->put("Mode", m_pPhysicalLayer->GetMode());

	pptNode = &ptChild.add("CNetworkLayer", "");
	pptNode->put("FullDuplex", m_pNetworkLayer->IsFullDuplex());
	pptNode->put("SeparationTimeMin", m_pNetworkLayer->GetSeparationTimeMin());
	pptNode->put("BlockSize", m_pNetworkLayer->GetBlockSize());
	pptNode->put("WaitFrameTransimissionMax", m_pNetworkLayer->GetWaitFrameTransimissionMax());
	pptNode->put("As", m_pNetworkLayer->GetAs());
	pptNode->put("Ar", m_pNetworkLayer->GetAr());
	pptNode->put("Bs", m_pNetworkLayer->GetBs());
	pptNode->put("Br", m_pNetworkLayer->GetBr());
	pptNode->put("Cs", m_pNetworkLayer->GetCs());
	pptNode->put("Cr", m_pNetworkLayer->GetCr());

	pptNode = &ptChild.add("CApplicationLayer", "");
	pptNode->put("TesterPhysicalAddress", m_pApplicationLayer->GetTesterPhysicalAddress());
	pptNode->put("ECUAddress", m_pApplicationLayer->GetECUAddress());
	pptNode->put("ECUFunctionalAddress", m_pApplicationLayer->GetECUAddress());
	pptNode->put("RemoteDiagnostic", m_pApplicationLayer->IsRemoteDiagnostic());
	pptNode->put("RemoteDiagnosticAddress", m_pApplicationLayer->GetRemoteDiagnosticAddress());
	pptNode->put("P2CANClient", m_pApplicationLayer->GetP2CANClient());
	pptNode->put("P2SCANClient", m_pApplicationLayer->GetP2SCANClient());
	pptNode->put("P3CANClientPhys", m_pApplicationLayer->GetP3CANClientPhys());
	pptNode->put("P3CANClientFunc", m_pApplicationLayer->GetP3CANClientFunc());
	pptNode->put("S3Client", m_pApplicationLayer->GetS3Client());

	try
	{
		write_xml(m_csaConfigFilename.GetBuffer(), ptRoot);
	}
	catch (std::exception)
	{
		return;
	}
}

void CDiagnosticControl::ResetTiming()
{
	m_dwStartTick = GetTickCount();
}

void CDiagnosticControl::AddWatchEntry(LayerType layerType, EntryType entryType, UINT nID, LPCTSTR lpszDescription, Color color) const
{
	_AddWatchEntry(layerType, entryType, nID, lpszDescription, color);
}

void CDiagnosticControl::AddWatchEntry(LayerType layerType, EntryType entryType, UINT nID, const BYTEVector &vbyData, Color color) const
{
	CString csDescription;
	for (BYTEVector::const_iterator iter = vbyData.cbegin(); iter != vbyData.cend(); ++iter)
	{
		if (iter != vbyData.cbegin())
		{
			csDescription.AppendChar(_T(' '));
		}
		csDescription.AppendFormat(IDS_DIAGNOSTIC_BYTEFORMAT, *iter);
	}
	_AddWatchEntry(layerType, entryType, nID, csDescription, color);
}

void CDiagnosticControl::AddWatchEntry(LayerType layerType, EntryType entryType, UINT nID, UINT nDescriptionID, Color color) const
{
	CString csDescription;
	csDescription.LoadString(nDescriptionID);
	_AddWatchEntry(layerType, entryType, nID, csDescription, color);
}

void CDiagnosticControl::AddWatchEntry(LayerType layerType, EntryType entryType, UINT nID, UINT nDescriptionID, int nData, Color color) const
{
	CString csDescription;
	csDescription.Format(nDescriptionID, nData);
	_AddWatchEntry(layerType, entryType, nID, csDescription, color);
}

void CDiagnosticControl::_AddWatchEntry(LayerType layerType, EntryType entryType, UINT nID, LPCTSTR lpszDescription, Color color) const
{
	CTIDCWatchWnd *pWatchWnd = NULL;
	switch (layerType)
	{
	case LayerType::ApplicationLayer:
		pWatchWnd = m_pApplicationLayerWatchWnd;
		break;
	case LayerType::NetworkLayer:
		pWatchWnd = m_pNetworkLayerWatchWnd;
		break;
	}
	pWatchWnd->AddEntry(GetTickCount() - m_dwStartTick, entryType, nID, lpszDescription, color);
	pWatchWnd->PostMessage(WM_WATCH_ADDENTRY, NULL, NULL);
}