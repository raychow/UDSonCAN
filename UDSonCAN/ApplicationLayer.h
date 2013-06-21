#pragma once

#include "DiagnosticType.h"
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

	UINT32 GetTesterPhysicalAddress() const;
	void SetTesterPhysicalAddress(UINT32 nTesterPhysicalAddress);

	UINT32 GetECUPhysicalAddress() const;
	void SetECUPhysicalAddress(UINT32 nECUPhysicalAddress);

	UINT32 GetECUFunctionalAddress() const;
	void SetECUFunctionalAddress(UINT32 nECUFunctionalAddress);

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

	void FirstFrameIndication(UINT32 nID, UINT nLength);
	// N_USData.confirm
	void Confirm(Diagnostic::NetworkLayerResult result);
	// N_USData.indication
	// 通知接收到数据，仅在收到单帧（SF）、完成（或失败）接收分段消息后调用。
	void Indication(UINT32 nID, const Diagnostic::BYTEVector &vbyData, Diagnostic::NetworkLayerResult result);

	void Request(const Diagnostic::BYTEVector &vbyData) const;

	boost::signals2::connection ConnectRequest(const Diagnostic::RequestSignal::slot_type &subscriber);

	void SetDiagnosticControl(CDiagnosticControl &diagnosticControl);
protected:
	Diagnostic::RequestSignal m_signalRequest;

	CDiagnosticControl *m_pDiagnosticControl;
	DiagnosticService::CServiceManager *m_pDiagnosticService;

	UINT32 m_nTesterPhysicalAddress;
	UINT32 m_nECUPhysicalAddress;
	UINT32 m_nECUFunctionalAddress;

	UINT m_anTimingParameters[4];				// 定时参数
	UINT m_nTimingS3Client;

	void _AddWatchEntry(EntryType entryType, UINT32 nID, LPCTSTR lpszDescription, Color color = Color::Black) const;
	void _AddWatchEntry(EntryType entryType, UINT32 nID, UINT nDescriptionID, Color color = Color::Black) const;
	void _AddWatchEntry(EntryType entryType, UINT32 nID, const Diagnostic::BYTEVector &vbyData, Color color = Color::Black) const;
	void _AddWatchEntry(EntryType entryType, UINT32 nID, UINT nDescriptionID, int nData, Color color = Color::Black) const;
};
