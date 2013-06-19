#pragma once

#include <memory>
#include <vector>

typedef std::vector<BYTE> BYTEVector;

class CPhysicalLayer;
class CNetworkLayer;

class CDataLinkLayer
{
public:
	// 15765-2: 7.1
	BOOL Request(INT32 nID, const BYTEVector &vbyData, BOOL bReverseAddress = FALSE);

	// CPhysicalLayer.confirm
	void Confirm(INT32 nID, BYTE byAddressExtension = 0, BOOL bReverseAddress = FALSE) const;

	// CPhysicalLayer.indication
	void Indication(INT32 nID, const BYTEVector &vbyData) const;

	void SetNodeAddress(INT32 nNodeAddress);

	void SetPhysicalLayer(CPhysicalLayer &physicalLayer);
	void SetNetworkLayer(CNetworkLayer &networkLayer);

	CDataLinkLayer(void);
	virtual ~CDataLinkLayer(void);
protected:
	enum
	{
		DLCREQUIRED = 8
	};

	INT32 m_nNodeAddress;

	CPhysicalLayer *m_pPhysicalLayer;
	CNetworkLayer *m_pNetworkLayer;

	UINT m_nBufferedID;
	CCriticalSection m_csectionBufferedID;
};

