#pragma once

#include <memory>
#include <vector>

#include "DiagnosticType.h"

class CPhysicalLayer;
class CNetworkLayer;

class CDataLinkLayer
{
public:
	// 15765-2: 7.1
	void Request(UINT32 nID, const Diagnostic::BYTEVector &vbyData);

	// CPhysicalLayer.confirm
	void Confirm() const;

	// CPhysicalLayer.indication
	void Indication(UINT32 nID, const Diagnostic::BYTEVector &vbyData) const;

	void SetNodeAddress(UINT32 nNodeAddress);

	boost::signals2::connection ConnectIndication(const Diagnostic::IndicationSignal::slot_type &subscriber);
	boost::signals2::connection ConnectConfirm(const Diagnostic::ConfirmSignal::slot_type &subscriber);
	boost::signals2::connection ConnectTransmit(const Diagnostic::PhysicalLayerTrasnmitSignal::slot_type &subscriber);

	CDataLinkLayer(void);
	virtual ~CDataLinkLayer(void);
protected:
	enum
	{
		DLCREQUIRED = 8
	};

	UINT32 m_nNodeAddress;

	Diagnostic::IndicationSignal m_signalIndication;
	Diagnostic::ConfirmSignal m_signalConfirm;
	Diagnostic::PhysicalLayerTrasnmitSignal m_signalTransmit;

	UINT m_nBufferedID;
	CCriticalSection m_csectionBufferedID;
};

