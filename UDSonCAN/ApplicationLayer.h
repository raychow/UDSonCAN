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

	INT32 GetTesterPhysicalAddress() const;
	void SetTesterPhysicalAddress(INT32 nTesterPhysicalAddress);

	INT32 GetECUPhysicalAddress() const;
	void SetECUPhysicalAddress(INT32 nECUPhysicalAddress);

	INT32 GetECUFunctionalAddress() const;
	void SetECUFunctionalAddress(INT32 nECUFunctionalAddress);

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

	void FirstFrameIndication(INT32 nID, UINT nLength);
	// N_USData.confirm
	void Confirm(CNetworkLayer::Result result);
	void Confirm(INT32 nID, CNetworkLayer::Result result);
	// N_USData.indication
	// 通知接收到数据，仅在收到单帧（SF）、完成（或失败）接收分段消息后调用。
	void Indication(INT32 nID, const BYTEVector &vbyData, CNetworkLayer::Result result);
	void Indication(CNetworkLayer::Result result);

	void Request(const BYTEVector &vbyData) const;

	void SetNetworkLayer(CNetworkLayer &networkLayer);
	void SetDiagnosticControl(CDiagnosticControl &diagnosticControl);
protected:
	CNetworkLayer *m_pNetworkLayer;
	CDiagnosticControl *m_pDiagnosticControl;
	DiagnosticService::CServiceManager *m_pDiagnosticService;

	INT32 m_nTesterPhysicalAddress;
	INT32 m_nECUPhysicalAddress;
	INT32 m_nECUFunctionalAddress;

	UINT m_anTimingParameters[4];				// 定时参数
	UINT m_nTimingS3Client;

	void _AddWatchEntry(EntryType entryType, INT32 nID, LPCTSTR lpszDescription, Color color = Color::Black) const;
	void _AddWatchEntry(EntryType entryType, INT32 nID, UINT nDescriptionID, Color color = Color::Black) const;
	void _AddWatchEntry(EntryType entryType, INT32 nID, const BYTEVector &vbyData, Color color = Color::Black) const;
	void _AddWatchEntry(EntryType entryType, INT32 nID, UINT nDescriptionID, int nData, Color color = Color::Black) const;
};
