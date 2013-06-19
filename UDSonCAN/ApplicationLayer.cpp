#include "stdafx.h"
#include "ApplicationLayer.h"

#include "DiagnosticControl.h"
#include "DiagnosticService.h"

CApplicationLayer::CApplicationLayer(void)
	: m_nTesterPhysicalAddress(0)
	, m_nECUPhysicalAddress(0)
	, m_nECUFunctionalAddress(FALSE)
	, m_nTimingS3Client(0)
	, m_pDiagnosticService(DiagnosticService::CServiceManager::GetInstance())
{
	ZeroMemory(m_anTimingParameters, sizeof(m_anTimingParameters));
}

CApplicationLayer::~CApplicationLayer(void)
{
}

INT32 CApplicationLayer::GetTesterPhysicalAddress() const
{
	return m_nTesterPhysicalAddress;
}

void CApplicationLayer::SetTesterPhysicalAddress(INT32 nTesterPhysicalAddress)
{
	m_nTesterPhysicalAddress = nTesterPhysicalAddress;
}

INT32 CApplicationLayer::GetECUPhysicalAddress() const
{
	return m_nECUPhysicalAddress;
}

void CApplicationLayer::SetECUPhysicalAddress(INT32 nECUPhysicalAddress)
{
	m_nECUPhysicalAddress = nECUPhysicalAddress;
}

INT32 CApplicationLayer::GetECUFunctionalAddress() const
{
	return m_nECUPhysicalAddress;
}

void CApplicationLayer::SetECUFunctionalAddress(INT32 nECUFunctionalAddress)
{
	m_nECUFunctionalAddress = nECUFunctionalAddress;
}

UINT CApplicationLayer::GetP2CANClient() const
{
	return m_anTimingParameters[0];
}

void CApplicationLayer::SetP2CANClient(UINT nP2CANClient)
{
	m_anTimingParameters[0] = nP2CANClient;
}

UINT CApplicationLayer::GetP2SCANClient() const
{
	return m_anTimingParameters[1];
}

void CApplicationLayer::SetP2SCANClient(UINT nP2SCANClient)
{
	m_anTimingParameters[1] = nP2SCANClient;
}

UINT CApplicationLayer::GetP3CANClientPhys() const
{
	return m_anTimingParameters[2];
}

void CApplicationLayer::SetP3CANClientPhys(UINT nP3CANClientPhys)
{
	m_anTimingParameters[2] = nP3CANClientPhys;
}

UINT CApplicationLayer::GetP3CANClientFunc() const
{
	return m_anTimingParameters[3];
}

void CApplicationLayer::SetP3CANClientFunc(UINT nP3CANClientFunc)
{
	m_anTimingParameters[3] = nP3CANClientFunc;
}

UINT CApplicationLayer::GetS3Client() const
{
	return m_nTimingS3Client;
}

void CApplicationLayer::SetS3Client(UINT nS3Client)
{
	m_nTimingS3Client = nS3Client;
}

void CApplicationLayer::FirstFrameIndication(INT32 nID, UINT nLength)
{

}

void CApplicationLayer::Confirm(CNetworkLayer::Result result)
{

}

void CApplicationLayer::Confirm(INT32 nID, CNetworkLayer::Result result)
{

}

void CApplicationLayer::Indication(INT32 nID, const BYTEVector &vbyData, CNetworkLayer::Result result)
{

}

void CApplicationLayer::Indication(CNetworkLayer::Result result)
{

}

void CApplicationLayer::Request(const BYTEVector &vbyData) const
{
	if (vbyData.empty())
	{
		return;
	}
	TRACE(_T("CApplicationLayer::Request. Service ID is: %d.\n"), vbyData.at(0));
	CStringArray acsEntries;
	m_pDiagnosticService->GetWatchEntriesByData(vbyData, acsEntries);
	for (INT_PTR i = 0; i != acsEntries.GetSize(); ++i)
	{
		_AddWatchEntry(EntryType::Transmit, m_nECUPhysicalAddress, acsEntries.GetAt(i));	// TODO: 显然此处有问题，要根据具体服务判断。
	}
	m_pNetworkLayer->Request(m_nECUPhysicalAddress, vbyData);
}

void CApplicationLayer::SetNetworkLayer(CNetworkLayer &networkLayer)
{
	m_pNetworkLayer = &networkLayer;
}

void CApplicationLayer::SetDiagnosticControl(CDiagnosticControl &diagnosticControl)
{
	m_pDiagnosticControl = &diagnosticControl;
}

void CApplicationLayer::_AddWatchEntry(EntryType entryType, INT32 nID, LPCTSTR lpszDescription, Color color) const
{
	if (NULL != m_pDiagnosticControl)
	{
		m_pDiagnosticControl->AddWatchEntry(CDiagnosticControl::LayerType::ApplicationLayer, entryType, nID, lpszDescription, color);
	}
}

void CApplicationLayer::_AddWatchEntry(EntryType entryType, INT32 nID, UINT nDescriptionID, Color color) const
{
	if (NULL != m_pDiagnosticControl)
	{
		m_pDiagnosticControl->AddWatchEntry(CDiagnosticControl::LayerType::ApplicationLayer, entryType, nID, nDescriptionID, color);
	}
}

void CApplicationLayer::_AddWatchEntry(EntryType entryType, INT32 nID, const BYTEVector &vbyData, Color color) const
{
	if (NULL != m_pDiagnosticControl)
	{
		m_pDiagnosticControl->AddWatchEntry(CDiagnosticControl::LayerType::ApplicationLayer, entryType, nID, vbyData, color);
	}
}

void CApplicationLayer::_AddWatchEntry(EntryType entryType, INT32 nID, UINT nDescriptionID, int nData, Color color) const
{
	if (NULL != m_pDiagnosticControl)
	{
		m_pDiagnosticControl->AddWatchEntry(CDiagnosticControl::LayerType::ApplicationLayer, entryType, nID, nDescriptionID, nData, color);
	}
}
