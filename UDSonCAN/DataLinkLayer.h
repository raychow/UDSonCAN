#pragma once

#include <memory>
#include <vector>

typedef std::vector<BYTE> BYTEVector;

class CPhysicalLayer;
class CNetworkLayer;

class CDataLinkLayer
{
public:
	union CANID
	{
		struct
		{
			unsigned SA: 8;
			unsigned PS: 8;
			unsigned PF: 8;
			unsigned DP: 1;
			unsigned R : 1;
			unsigned P : 3;
		} bitField;
		UINT nID;
	};

	// 15765-2: 7.1
	BOOL Request(UINT nID, const BYTEVector &vbyData, BOOL bReverseAddress = FALSE);

	// CPhysicalLayer.confirm
	void Confirm(UINT nID, BYTE byAddressExtension = 0, BOOL bReverseAddress = FALSE) const;

	// CPhysicalLayer.indication
	void Indication(UINT nID, const BYTEVector &vbyData) const;

	void SetNodeAddress(BYTE byNodeAddress);

	void SetPhysicalLayer(CPhysicalLayer &physicalLayer);
	void SetNetworkLayer(CNetworkLayer &networkLayer);

	CDataLinkLayer(void);
	virtual ~CDataLinkLayer(void);
protected:
	enum
	{
		DLCREQUIRED = 8
	};

	BYTE m_byNodeAddress;

	CPhysicalLayer *m_pPhysicalLayer;
	CNetworkLayer *m_pNetworkLayer;

	UINT m_nBufferedID;
	CCriticalSection m_csectionBufferedID;
};

