#pragma once

#include <mutex>
#include <list>
#include <vector>

#include "DiagnosticType.h"
#include "WatchWnd.h"

//#define NETWORKLAYER_NOAE

class CDiagnosticControl;

class CNetworkLayer
{
	typedef CTIDCWatchWnd::EntryType EntryType;
	typedef CWatchWnd::Color Color;
public:
	enum struct PCIType : BYTE	// 单帧 / 首帧 / 连续帧 / 流控制帧。
	{
		Unknown = 4,
		SingleFrame = 0,
		FirstFrame,
		ConsecutiveFrame,
		FlowControl,
	};

	enum struct Status : BYTE	// 空闲 / 收 / 发。
	{
		Idle							= 0,
		ReceiveInProgress				= 1,
		TransmitInProgress				= 2
	};

	enum struct FlowControlType : BYTE	// 流控制帧类型。
	{
		ContinueToSend = 0,
		Wait,
		Overflow
	};

	enum struct TimingType : BYTE	// 对何种事件进行定时。
	{
		As,
		Ar,
		Bs,
		Br,
		Cs,
		Cr,
		Idle
	};

	CNetworkLayer(void);
	virtual ~CNetworkLayer(void);

	UINT GetAs() const;
	void SetAs(UINT nAs);

	UINT GetAr() const;
	void SetAr(UINT nAr);

	UINT GetBs() const;
	void SetBs(UINT nBs);

	UINT GetBr() const;
	void SetBr(UINT nBr);

	UINT GetCs() const;
	void SetCs(UINT nCs);

	UINT GetCr() const;
	void SetCr(UINT nCr);

	BYTE GetSeparationTimeMin() const;
	void SetSeparationTimeMin(BYTE bySeparationTimeMin);

	BYTE GetBlockSize() const;
	void SetBlockSize(BYTE byBlockSize);

	UINT GetWaitFrameTransimissionMax() const;
	void SetWaitFrameTransimissionMax(UINT nWaitFrameTransimissionMax);

	// 15765-2 5.2
	void Request(UINT32 nID, const Diagnostic::BYTEVector &vbyData);

	void RequestTesterPresent(UINT32 nID);
	// L_Data.confirm
	void Confirm();

	// L_Data.indication
	void Indication(UINT32 nID, const Diagnostic::BYTEVector &vbyData);

	boost::signals2::connection ConnectIndication(const Diagnostic::IndicationASignal::slot_type &subscriber);
	boost::signals2::connection ConnectFirstFrameIndication(const Diagnostic::IndicationAFSignal::slot_type &subscriber);
	boost::signals2::connection ConnectConfirm(const Diagnostic::ConfirmASignal::slot_type &subscriber);
	boost::signals2::connection ConnectRequest(const Diagnostic::RequestSignal::slot_type &subscriber);

	void SetDiagnosticControl(CDiagnosticControl &diagnosticControl);
protected:
	enum : UINT
	{
		CANFRAMEDATALENGTHMAX				= 8,			// CAN 帧最大数据长度。
		DIAGNOSTICSFRAMEDATALENGTH			= 7,			// 诊断帧数据长度。
		REMOTEDIAGNOSTICSFRAMEDATALENGTH	= 6,			// 远程诊断帧数据长度。
		TESETRPRESENTSERVICEID				= 0x3E,			// TP Service ID。
		TIMINGCYCLE							= 100			// 检查定时的周期。
	};

	enum struct J1939ParameterGroupNumber : BYTE
	{
		MixedAddressingFunctional		= 205,
		MixedAddressingPhysical			= 206,
		NormalFixedAddressingPhysical	= 218,
		NormalFixedAddressingFunctional	= 219
	};

	struct MessageBuffer							// 消息缓存，已退化为半双工。
													// 是具有此功能的设计，目前取消了多 ECU 支持，只对单一 ECU 操作，但仍用此结构。
	{
		MessageBuffer();
		BOOL IsBusy();								// [线程安全] 正忙检测。
		BOOL IsRequest();							// [线程安全] 是否发送（发送状态，或接收状态下发流控制帧）。

		// CCriticalSection csectionProcess;		// 消息处理临界对象。
		std::recursive_mutex rmutexMessageBuffer;	// 访问互斥量。
		Status status;
		UINT32 nID;									// ID
		Diagnostic::BYTEVector vbyData;							// 将要发送，或是正在接收的数据。
		Diagnostic::BYTEVector::size_type stLocation;			// 将要发送或接收的位置。
		PCIType pciType;							// 发送 / 验证 / 期望收到的 PCI 类型：SF / FF / CF / FC。
		BYTE bySeparationTimeMin;					// 连续帧发送的最小等待时间。
		UINT nRemainderFrameCount;					// 本次尚要发送的帧数，或是尚要接收的帧数；
													// 发送状态下为 0 表示正在等待流控制帧；
													// 接受状态下为 0 表示将要发送流控制帧。
		BYTE byExpectedSequenceNumber;				// 期望发送或收到的分段序列号。
		DWORD dwTimingStartTick;					// 定时器开始的时间。
		TimingType timingType;						// 对何种事件进行定时。

		void ResetTiming(TimingType timingType, CEvent &eventTiming);	// [线程安全] 重置定时器。
		void ClearMessage();						// [线程安全] 终止消息收发。
	} m_messageBuffer;

	BYTE m_bySeparationTimeMin;					// 流控制帧包含的发送间隔时间。
	BYTE m_byBlockSize;							// 流控制帧包含的发送块大小。

	UINT m_nWaitFrameTransimissionMax;			// 15765-2: 6.6, （由发送方保证的）流控制等待帧发送的最大次数。

	UINT m_anTimingParameters[6];				// 定时参数 As, Ar, Bs, Br, Cs, Cr

	Diagnostic::IndicationASignal m_signalIndication;
	Diagnostic::IndicationAFSignal m_signalFirstFrameIndication;
	Diagnostic::ConfirmASignal m_signalConfirm;
	Diagnostic::RequestSignal m_signalRequest;

	CDiagnosticControl *m_pDiagnosticControl;

	CCriticalSection m_csectionProcess;			// 收发处理临界对象。

	CWinThread *m_pTimingThread;				// 定时线程的指针，此线程应自动删除。

	BOOL _FillBuffer(Status status, UINT32 nID, const Diagnostic::BYTEVector &vbyData);
	void _Request();

	static UINT _TimingThread(LPVOID lpParam);
	CEvent m_eventTiming;

	CEvent m_eventStopThread;					// 指示正停止线程。
	CEvent m_eventTimingThreadExited;			// 指示 Timing 线程已经退出。

	void _AddWatchEntry(EntryType entryType, UINT32 nID, UINT nDescriptionID, Color color = Color::Black);
	void _AddWatchEntry(EntryType entryType, UINT32 nID, const Diagnostic::BYTEVector &vbyData, Color color = Color::Black);
	void _AddWatchEntry(EntryType entryType, UINT32 nID, UINT nDescriptionID, int nData, Color color = Color::Black);

	void _StartThread();
	void _StopThread();
};
