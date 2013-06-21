#include "stdafx.h"
#include "DataLinkLayer.h"

using Diagnostic::BYTEVector;

CDataLinkLayer::CDataLinkLayer(void)
	: m_nNodeAddress(0)
	, m_nBufferedID(0)
{
}

CDataLinkLayer::~CDataLinkLayer(void)
{
}

void CDataLinkLayer::Request(UINT32 nID, const Diagnostic::BYTEVector &vbyData)
{
	TRACE(_T("CDataLinkLayer::Request: %X\n"), nID);

	// 15765-2: 7.4.1
	BYTEVector vbyDataTransmit = vbyData;
	vbyDataTransmit.resize(DLCREQUIRED, 0);
	m_signalTransmit(nID, vbyDataTransmit, Diagnostic::PhysicalLayerSendType::Normal, NULL, NULL);
}

void CDataLinkLayer::Confirm() const
{
	TRACE(_T("CDataLinkLayer::Confirm"));

	m_signalConfirm();
}

void CDataLinkLayer::Indication(UINT32 nID, const Diagnostic::BYTEVector &vbyData) const
{
	if (nID != m_nNodeAddress || vbyData.size() != DLCREQUIRED)
	{
		TRACE(_T("CDataLinkLayer::Indication: Discard received frame, the ID is 0x%X.\n"), nID);
		return;
	}

	TRACE(_T("CDataLinkLayer::Indication.\n"), nID);

	m_signalIndication(nID, vbyData);
	return;
}

void CDataLinkLayer::SetNodeAddress(UINT32 nNodeAddress)
{
	m_nNodeAddress = nNodeAddress;
}

boost::signals2::connection CDataLinkLayer::ConnectIndication(const Diagnostic::IndicationSignal::slot_type &subscriber)
{
	return m_signalIndication.connect(subscriber);
}

boost::signals2::connection CDataLinkLayer::ConnectConfirm(const Diagnostic::ConfirmSignal::slot_type &subscriber)
{
	return m_signalConfirm.connect(subscriber);
}

boost::signals2::connection CDataLinkLayer::ConnectTransmit(const Diagnostic::PhysicalLayerTrasnmitSignal::slot_type &subscriber)
{
	return m_signalTransmit.connect(subscriber);
}
