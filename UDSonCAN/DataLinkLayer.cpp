#include "stdafx.h"
#include "DataLinkLayer.h"

#include "NetworkLayer.h"
#include "PhysicalLayer.h"

CDataLinkLayer::CDataLinkLayer(void)
	: m_byNodeAddress(0)
	, m_pPhysicalLayer(NULL)
	, m_pNetworkLayer(NULL)
	, m_nBufferedID(0)
{
}

CDataLinkLayer::~CDataLinkLayer(void)
{
}

BOOL CDataLinkLayer::Request(UINT nID, const BYTEVector &vbyData, BOOL bReverseAddress)
{
	ASSERT(m_pPhysicalLayer);
	TRACE(_T("CDataLinkLayer::Request: %X\n"), nID);

	// 15765-2: 7.4.1
	BYTEVector vbyDataTransmit = vbyData;
	vbyDataTransmit.resize(DLCREQUIRED, 0);
	return m_pPhysicalLayer->Transmit(nID, vbyDataTransmit, CPhysicalLayer::Normal, NULL, NULL, bReverseAddress);	// Confirm
}

void CDataLinkLayer::Confirm(UINT nID, BYTE byAddressExtension, BOOL bReverseAddress) const
{
	ASSERT(m_pNetworkLayer);
	TRACE(_T("CDataLinkLayer::Confirm\n"), nID);

	m_pNetworkLayer->Confirm(nID, byAddressExtension, bReverseAddress);
}

void CDataLinkLayer::Indication(UINT nID, const BYTEVector &vbyData) const
{
	ASSERT(m_pNetworkLayer);

	CANID canID;
	canID.nID = nID;
	if (canID.bitField.PS != m_byNodeAddress || vbyData.size() != DLCREQUIRED)
	{
		TRACE(_T("CDataLinkLayer::Indication: Discard received frame, the PS is 0x%X.\n"), canID.bitField.PS);
		return;
	}

	TRACE(_T("CDataLinkLayer::Indication: 0x%X\n"), nID);

	m_pNetworkLayer->Indication(nID, vbyData);
	return;
}

void CDataLinkLayer::SetNodeAddress(BYTE byNodeAddress)
{
	m_byNodeAddress = byNodeAddress;
}

void CDataLinkLayer::SetPhysicalLayer(CPhysicalLayer &physicalLayer)
{
	m_pPhysicalLayer = &physicalLayer;
}

void CDataLinkLayer::SetNetworkLayer(CNetworkLayer &networkLayer)
{
	m_pNetworkLayer = &networkLayer;
}
