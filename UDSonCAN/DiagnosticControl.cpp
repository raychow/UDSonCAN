#include "stdafx.h"
#include "DiagnosticControl.h"

#include <boost/bind.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "UDSonCAN.h"
#include "PhysicalLayer.h"
#include "DataLinkLayer.h"
#include "NetworkLayer.h"
#include "ApplicationLayer.h"

using Diagnostic::BYTEVector;

CDiagnosticControl::CDiagnosticControl(void)
	: m_upPhysicalLayer(new CPhysicalLayer())
	, m_upDataLinkLayer(new CDataLinkLayer())
	, m_upNetworkLayer(new CNetworkLayer())
	, m_upApplicationLayer(new CApplicationLayer())
{
	m_connectionDataLinkLayerConfirm = m_upPhysicalLayer->ConnectConfirm(boost::bind(&CDataLinkLayer::Confirm, m_upDataLinkLayer.get()));
	m_connectionNetworkLayerConfirm = m_upDataLinkLayer->ConnectConfirm(boost::bind(&CNetworkLayer::Confirm, m_upNetworkLayer.get()));
	m_connectionApplicationLayerConfirm = m_upNetworkLayer->ConnectConfirm(boost::bind(&CApplicationLayer::Confirm, m_upApplicationLayer.get(), ::_1));
	
	m_connectionDataLinkLayerIndication = m_upPhysicalLayer->ConnectIndication(boost::bind(&CDataLinkLayer::Indication, m_upDataLinkLayer.get(), ::_1, ::_2));
	m_connectionNetworkLayerIndication = m_upDataLinkLayer->ConnectIndication(boost::bind(&CNetworkLayer::Indication, m_upNetworkLayer.get(), ::_1, ::_2));
	m_connectionApplicationLayerIndication = m_upNetworkLayer->ConnectIndication(boost::bind(&CApplicationLayer::Indication, m_upApplicationLayer.get(), ::_1, ::_2, ::_3));
	m_connectionApplicationLayerFirstFrameIndication = m_upNetworkLayer->ConnectFirstFrameIndication(boost::bind(&CApplicationLayer::FirstFrameIndication, m_upApplicationLayer.get(), ::_1, ::_2));
	
	m_connectionPhysicalLayerTrasnmit = m_upDataLinkLayer->ConnectTransmit(boost::bind(&CPhysicalLayer::Transmit, m_upPhysicalLayer.get(), ::_1, ::_2, ::_3, ::_4, ::_5));
	m_connectionDataLinkLayerRequest = m_upNetworkLayer->ConnectRequest(boost::bind(&CDataLinkLayer::Request, m_upDataLinkLayer.get(), ::_1, ::_2));
	m_connectionNetworkLayerRequest = m_upApplicationLayer->ConnectRequest(boost::bind(&CNetworkLayer::Request, m_upNetworkLayer.get(), ::_1, ::_2));

	m_upNetworkLayer->SetDiagnosticControl(*this);
	m_upApplicationLayer->SetDiagnosticControl(*this);

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
	m_connectionDataLinkLayerConfirm.disconnect();
	m_connectionDataLinkLayerIndication.disconnect();
	m_connectionNetworkLayerConfirm.disconnect();
	m_connectionNetworkLayerIndication.disconnect();
	m_connectionApplicationLayerConfirm.disconnect();
	m_connectionApplicationLayerIndication.disconnect();
	m_connectionApplicationLayerFirstFrameIndication.disconnect();
	//m_upPhysicalLayer->CloseDevice();	// 物理层必须在网络层之前关闭，否则可能会在网络层析构后通知网络层。
}

CPhysicalLayer &CDiagnosticControl::GetPhysicalLayer()
{
	return *m_upPhysicalLayer;
}

CDataLinkLayer &CDiagnosticControl::GetDataLinkLayer()
{
	return *m_upDataLinkLayer;
}

CNetworkLayer &CDiagnosticControl::GetNetworkLayer()
{
	return *m_upNetworkLayer;
}

CApplicationLayer &CDiagnosticControl::GetApplicationLayer()
{
	return *m_upApplicationLayer;
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
	m_upPhysicalLayer->SetDeviceType(pptNode->get<DWORD>("DeviceType", 3));
	m_upPhysicalLayer->SetDeviceIndex(pptNode->get<DWORD>("DeviceIndex", 0));
	m_upPhysicalLayer->SetCANIndex(pptNode->get<DWORD>("CANIndex", 0));
	m_upPhysicalLayer->SetAccMask(pptNode->get<DWORD>("AccMask", 0));
	m_upPhysicalLayer->SetAccCode(pptNode->get<DWORD>("AccCode", 0));
	m_upPhysicalLayer->SetBaudRateType(pptNode->get<UINT>("BaudRateType", 0));
	m_upPhysicalLayer->SetTiming0(pptNode->get<UCHAR>("Timing0", 0));
	m_upPhysicalLayer->SetTiming1(pptNode->get<UCHAR>("Timing1", 0));
	m_upPhysicalLayer->SetFilter(pptNode->get<UCHAR>("Filter", 0));
	m_upPhysicalLayer->SetMode(pptNode->get<UCHAR>("Mode", 0));

	pptNode = &pptChild->get_child("CNetworkLayer", ptEmpty);
	m_upNetworkLayer->SetSeparationTimeMin(pptNode->get<BYTE>("SeparationTimeMin", 0));
	m_upNetworkLayer->SetBlockSize(pptNode->get<BYTE>("BlockSize", 0xFF));
	m_upNetworkLayer->SetWaitFrameTransimissionMax(pptNode->get<UINT>("WaitFrameTransimissionMax", 0));
	m_upNetworkLayer->SetAs(pptNode->get<UINT>("As", 1000));
	m_upNetworkLayer->SetAr(pptNode->get<UINT>("Ar", 1000));
	m_upNetworkLayer->SetBs(pptNode->get<UINT>("Bs", 1000));
	m_upNetworkLayer->SetBr(pptNode->get<UINT>("Br", 400));
	m_upNetworkLayer->SetCs(pptNode->get<UINT>("Cs", 400));
	m_upNetworkLayer->SetCr(pptNode->get<UINT>("Cr", 1000));

	pptNode = &pptChild->get_child("CApplicationLayer", ptEmpty);
	m_upApplicationLayer->SetTesterPhysicalAddress(pptNode->get<UINT32>("TesterPhysicalAddress", 0x0780));
	m_upApplicationLayer->SetECUPhysicalAddress(pptNode->get<UINT32>("ECUPhysicalAddress", 0x0760));
	m_upApplicationLayer->SetECUFunctionalAddress(pptNode->get<UINT32>("ECUFunctionalAddress", 0));
	m_upApplicationLayer->SetP2CANClient(pptNode->get<UINT>("P2CANClient", 1100));
	m_upApplicationLayer->SetP2SCANClient(pptNode->get<UINT>("P2SCANClient", 6000));
	m_upApplicationLayer->SetP3CANClientPhys(pptNode->get<UINT>("P3CANClientPhys", 50));
	m_upApplicationLayer->SetP3CANClientFunc(pptNode->get<UINT>("P3CANClientFunc", 50));
	m_upApplicationLayer->SetS3Client(pptNode->get<UINT>("S3Client", 2000));
}

void CDiagnosticControl::SaveConfig()
{
	using boost::property_tree::ptree;
	using boost::property_tree::xml_parser::write_xml;

	ptree ptRoot;
	ptree &ptChild = ptRoot.add("UDSonCAN", "");
	ptree *pptNode;

	pptNode = &ptChild.add("CPhysicalLayer", "");
	pptNode->put("DeviceType", m_upPhysicalLayer->GetDeviceType());
	pptNode->put("DeviceIndex", m_upPhysicalLayer->GetDeviceIndex());
	pptNode->put("CANIndex", m_upPhysicalLayer->GetCANIndex());
	pptNode->put("AccMask", m_upPhysicalLayer->GetAccMask());
	pptNode->put("AccCode", m_upPhysicalLayer->GetAccCode());
	pptNode->put("BaudRateType", m_upPhysicalLayer->GetBaudRateType());
	pptNode->put("Timing0", m_upPhysicalLayer->GetTiming0());
	pptNode->put("Timing1", m_upPhysicalLayer->GetTiming1());
	pptNode->put("Filter", m_upPhysicalLayer->GetFilter());
	pptNode->put("Mode", m_upPhysicalLayer->GetMode());

	pptNode = &ptChild.add("CNetworkLayer", "");
	pptNode->put("SeparationTimeMin", m_upNetworkLayer->GetSeparationTimeMin());
	pptNode->put("BlockSize", m_upNetworkLayer->GetBlockSize());
	pptNode->put("WaitFrameTransimissionMax", m_upNetworkLayer->GetWaitFrameTransimissionMax());
	pptNode->put("As", m_upNetworkLayer->GetAs());
	pptNode->put("Ar", m_upNetworkLayer->GetAr());
	pptNode->put("Bs", m_upNetworkLayer->GetBs());
	pptNode->put("Br", m_upNetworkLayer->GetBr());
	pptNode->put("Cs", m_upNetworkLayer->GetCs());
	pptNode->put("Cr", m_upNetworkLayer->GetCr());

	pptNode = &ptChild.add("CApplicationLayer", "");
	pptNode->put("TesterPhysicalAddress", m_upApplicationLayer->GetTesterPhysicalAddress());
	pptNode->put("ECUPhysicalAddress", m_upApplicationLayer->GetECUPhysicalAddress());
	pptNode->put("ECUFunctionalAddress", m_upApplicationLayer->GetECUFunctionalAddress());
	pptNode->put("P2CANClient", m_upApplicationLayer->GetP2CANClient());
	pptNode->put("P2SCANClient", m_upApplicationLayer->GetP2SCANClient());
	pptNode->put("P3CANClientPhys", m_upApplicationLayer->GetP3CANClientPhys());
	pptNode->put("P3CANClientFunc", m_upApplicationLayer->GetP3CANClientFunc());
	pptNode->put("S3Client", m_upApplicationLayer->GetS3Client());

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

void CDiagnosticControl::AddWatchEntry(LayerType layerType, EntryType entryType, UINT32 nID, LPCTSTR lpszDescription, Color color) const
{
	_AddWatchEntry(layerType, entryType, nID, lpszDescription, color);
}

void CDiagnosticControl::AddWatchEntry(LayerType layerType, EntryType entryType, UINT32 nID, const BYTEVector &vbyData, Color color) const
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

void CDiagnosticControl::AddWatchEntry(LayerType layerType, EntryType entryType, UINT32 nID, UINT nDescriptionID, Color color) const
{
	CString csDescription;
	csDescription.LoadString(nDescriptionID);
	_AddWatchEntry(layerType, entryType, nID, csDescription, color);
}

void CDiagnosticControl::AddWatchEntry(LayerType layerType, EntryType entryType, UINT32 nID, UINT nDescriptionID, int nData, Color color) const
{
	CString csDescription;
	csDescription.Format(nDescriptionID, nData);
	_AddWatchEntry(layerType, entryType, nID, csDescription, color);
}

void CDiagnosticControl::_AddWatchEntry(LayerType layerType, EntryType entryType, UINT32 nID, LPCTSTR lpszDescription, Color color) const
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