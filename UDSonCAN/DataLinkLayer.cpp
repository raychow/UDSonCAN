#include "stdafx.h"
#include "DataLinkLayer.h"

#include "NetworkLayer.h"
#include "PhysicalLayer.h"

CDataLinkLayer::CDataLinkLayer(void)
	: m_nNodeAddress(0)
	, m_pPhysicalLayer(NULL)
	, m_pNetworkLayer(NULL)
	, m_nBufferedID(0)
{
}

CDataLinkLayer::~CDataLinkLayer(void)
{
}

BOOL CDataLinkLayer::Request(INT32 nID, const BYTEVector &vbyData, BOOL bReverseAddress)
{
	ASSERT(m_pPhysicalLayer);
	TRACE(_T("CDataLinkLayer::Request: %X\n"), nID);

	// 15765-2: 7.4.1
	BYTEVector vbyDataTransmit = vbyData;
	vbyDataTransmit.resize(DLCREQUIRED, 0);
	return m_pPhysicalLayer->Transmit(nID, vbyDataTransmit, CPhysicalLayer::Normal, NULL, NULL, bReverseAddress);	// Confirm
}

void CDataLinkLayer::Confirm(INT32 nID, BYTE byAddressExtension, BOOL bReverseAddress) const
{
	ASSERT(m_pNetworkLayer);
	TRACE(_T("CDataLinkLayer::Confirm\n"), nID);

	m_pNetworkLayer->Confirm(nID);
}

void CDataLinkLayer::Indication(INT32 nID, const BYTEVector &vbyData) const
{
	ASSERT(m_pNetworkLayer);

	if (nID != m_nNodeAddress || vbyData.size() != DLCREQUIRED)
	{
		TRACE(_T("CDataLinkLayer::Indication: Discard received frame, the PS is 0x%X.\n"), nID);
		return;
	}

	TRACE(_T("CDataLinkLayer::Indication: 0x%X\n"), nID);

	m_pNetworkLayer->Indication(nID, vbyData);
	return;
}

void CDataLinkLayer::SetNodeAddress(INT32 nNodeAddress)
{
	m_nNodeAddress = nNodeAddress;
}

void CDataLinkLayer::SetPhysicalLayer(CPhysicalLayer &physicalLayer)
{
	m_pPhysicalLayer = &physicalLayer;
}

void CDataLinkLayer::SetNetworkLayer(CNetworkLayer &networkLayer)
{
	m_pNetworkLayer = &networkLayer;
}
