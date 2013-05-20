#include "stdafx.h"
#include "ApplicationLayer.h"

#include "DiagnosticControl.h"
#include "DiagnosticService.h"

CApplicationLayer::CApplicationLayer(void)
	: m_byTesterPhysicalAddress(0)
	, m_byECUAddress(0)
	, m_bECUFunctionalAddress(FALSE)
	, m_bRemoteDiagnostic(FALSE)
	, m_byRemoteDiagnosticAddress(0)
	, m_nTimingS3Client(0)
	, m_pDiagnosticService(DiagnosticService::CServiceManager::GetInstance())
{
	ZeroMemory(m_anTimingParameters, sizeof(m_anTimingParameters));
}

CApplicationLayer::~CApplicationLayer(void)
{
}

BYTE CApplicationLayer::GetTesterPhysicalAddress() const
{
	return m_byTesterPhysicalAddress;
}

void CApplicationLayer::SetTesterPhysicalAddress(BYTE byTesterPhysicalAddress)
{
	m_byTesterPhysicalAddress = byTesterPhysicalAddress;
}

BYTE CApplicationLayer::GetECUAddress() const
{
	return m_byECUAddress;
}

void CApplicationLayer::SetECUAddress(BYTE byECUAddress)
{
	m_byECUAddress = byECUAddress;
}

BOOL CApplicationLayer::IsECUFunctionalAddress() const
{
	return m_bECUFunctionalAddress;
}

void CApplicationLayer::SetECUFunctionalAddress(BOOL bECUFunctionalAddress)
{
	m_bECUFunctionalAddress = bECUFunctionalAddress;
}

BOOL CApplicationLayer::IsRemoteDiagnostic() const
{
	return m_bRemoteDiagnostic;
}

void CApplicationLayer::SetRemoteDiagnostic(BOOL bRemoteDiagnostic)
{
	m_bRemoteDiagnostic = bRemoteDiagnostic;
}

BYTE CApplicationLayer::GetRemoteDiagnosticAddress() const
{
	return m_byRemoteDiagnosticAddress;
}

void CApplicationLayer::SetRemoteDiagnosticAddress(BYTE byRemoteDiagnosticAddress)
{
	byRemoteDiagnosticAddress = byRemoteDiagnosticAddress;
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

void CApplicationLayer::FirstFrameIndication(CNetworkLayer::MessageType messageType, BYTE bySourceAddress, BYTE byTargetAddress, CNetworkLayer::TargetAddressType targetAddressType, BYTE addressExtension, UINT nLength)
{

}

void CApplicationLayer::Confirm(CNetworkLayer::Result result)
{

}

void CApplicationLayer::Confirm(CNetworkLayer::MessageType messageType, BYTE bySourceAddress, BYTE byTargetAddress, CNetworkLayer::TargetAddressType byTargetAddressType, BYTE addressExtension, CNetworkLayer::Result result)
{

}

void CApplicationLayer::Indication(CNetworkLayer::MessageType messageType, BYTE bySourceAddress, BYTE byTargetAddress, CNetworkLayer::TargetAddressType targetAddressType, BYTE byAddressExtension, const BYTEVector &vbyData, CNetworkLayer::Result result)
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
		_AddWatchEntry(EntryType::Transmit, m_byECUAddress, acsEntries.GetAt(i));
	}
	m_pNetworkLayer->Request(m_bRemoteDiagnostic ? CNetworkLayer::MessageType::RemoteDiagnostics : CNetworkLayer::MessageType::Diagnostics,
		m_byTesterPhysicalAddress,
		m_byECUAddress,
		m_bECUFunctionalAddress ? CNetworkLayer::TargetAddressType::Functional : CNetworkLayer::TargetAddressType::Physical,
		m_byRemoteDiagnosticAddress,
		vbyData);
}

void CApplicationLayer::SetNetworkLayer(CNetworkLayer &networkLayer)
{
	m_pNetworkLayer = &networkLayer;
}

void CApplicationLayer::SetDiagnosticControl(CDiagnosticControl &diagnosticControl)
{
	m_pDiagnosticControl = &diagnosticControl;
}

void CApplicationLayer::_AddWatchEntry(EntryType entryType, UINT nID, LPCTSTR lpszDescription, Color color) const
{
	if (NULL != m_pDiagnosticControl)
	{
		m_pDiagnosticControl->AddWatchEntry(CDiagnosticControl::LayerType::ApplicationLayer, entryType, nID, lpszDescription, color);
	}
}

void CApplicationLayer::_AddWatchEntry(EntryType entryType, UINT nID, UINT nDescriptionID, Color color) const
{
	if (NULL != m_pDiagnosticControl)
	{
		m_pDiagnosticControl->AddWatchEntry(CDiagnosticControl::LayerType::ApplicationLayer, entryType, nID, nDescriptionID, color);
	}
}

void CApplicationLayer::_AddWatchEntry(EntryType entryType, UINT nID, const BYTEVector &vbyData, Color color) const
{
	if (NULL != m_pDiagnosticControl)
	{
		m_pDiagnosticControl->AddWatchEntry(CDiagnosticControl::LayerType::ApplicationLayer, entryType, nID, vbyData, color);
	}
}

void CApplicationLayer::_AddWatchEntry(EntryType entryType, UINT nID, UINT nDescriptionID, int nData, Color color) const
{
	if (NULL != m_pDiagnosticControl)
	{
		m_pDiagnosticControl->AddWatchEntry(CDiagnosticControl::LayerType::ApplicationLayer, entryType, nID, nDescriptionID, nData, color);
	}
}
