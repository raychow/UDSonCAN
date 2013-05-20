#pragma once

#include "NetworkLayer.h"
#include "WatchWnd.h"

class CDiagnosticControl;
namespace DiagnosticService
{
	class CServiceManager;
};

class CApplicationLayer
{
	typedef CTIDCWatchWnd::EntryType EntryType;
	typedef CWatchWnd::Color Color;
public:
	enum struct TimingType : BYTE
	{
		P2CANClient,
		P2SCANClient,
		P3CANClientPhys,
		P3CANClientFunc,
	};								// 对何种事件进行定时。

	CApplicationLayer(void);
	virtual ~CApplicationLayer(void);

	BYTE GetTesterPhysicalAddress() const;
	void SetTesterPhysicalAddress(BYTE byTesterPhysicalAddress);

	BYTE GetECUAddress() const;
	void SetECUAddress(BYTE byECUAddress);

	BOOL IsECUFunctionalAddress() const;
	void SetECUFunctionalAddress(BOOL bECUFunctionalAddress);

	BOOL IsRemoteDiagnostic() const;
	void SetRemoteDiagnostic(BOOL bRemoteDiagnostic);

	BYTE GetRemoteDiagnosticAddress() const;
	void SetRemoteDiagnosticAddress(BYTE byRemoteDiagnosticAddress);

	UINT GetP2CANClient() const;
	void SetP2CANClient(UINT nP2CANClient);

	UINT GetP2SCANClient() const;
	void SetP2SCANClient(UINT nP2SCANClient);

	UINT GetP3CANClientPhys() const;
	void SetP3CANClientPhys(UINT nP3CANClientPhys);

	UINT GetP3CANClientFunc() const;
	void SetP3CANClientFunc(UINT nP3CANClientFunc);

	UINT GetS3Client() const;
	void SetS3Client(UINT nS3Client);

	void FirstFrameIndication(CNetworkLayer::MessageType messageType, BYTE bySourceAddress, BYTE byTargetAddress, CNetworkLayer::TargetAddressType targetAddressType, BYTE addressExtension, UINT nLength);
	// N_USData.confirm
	void Confirm(CNetworkLayer::Result result);
	void Confirm(CNetworkLayer::MessageType messageType, BYTE bySourceAddress, BYTE byTargetAddress, CNetworkLayer::TargetAddressType byTargetAddressType, BYTE addressExtension, CNetworkLayer::Result result);
	// N_USData.indication
	// 通知接收到数据，仅在收到单帧（SF）、完成（或失败）接收分段消息后调用。
	void Indication(CNetworkLayer::MessageType messageType, BYTE bySourceAddress, BYTE byTargetAddress, CNetworkLayer::TargetAddressType targetAddressType, BYTE byAddressExtension, const BYTEVector &vbyData, CNetworkLayer::Result result);
	void Indication(CNetworkLayer::Result result);

	void Request(const BYTEVector &vbyData) const;

	void SetNetworkLayer(CNetworkLayer &networkLayer);
	void SetDiagnosticControl(CDiagnosticControl &diagnosticControl);
protected:
	CNetworkLayer *m_pNetworkLayer;
	CDiagnosticControl *m_pDiagnosticControl;
	DiagnosticService::CServiceManager *m_pDiagnosticService;

	BYTE m_byTesterPhysicalAddress;
	BYTE m_byECUAddress;
	BOOL m_bECUFunctionalAddress;
	BOOL m_bRemoteDiagnostic;
	BYTE m_byRemoteDiagnosticAddress;

	UINT m_anTimingParameters[4];				// 定时参数
	UINT m_nTimingS3Client;

	void _AddWatchEntry(EntryType entryType, UINT nID, LPCTSTR lpszDescription, Color color = Color::Black) const;
	void _AddWatchEntry(EntryType entryType, UINT nID, UINT nDescriptionID, Color color = Color::Black) const;
	void _AddWatchEntry(EntryType entryType, UINT nID, const BYTEVector &vbyData, Color color = Color::Black) const;
	void _AddWatchEntry(EntryType entryType, UINT nID, UINT nDescriptionID, int nData, Color color = Color::Black) const;
};
