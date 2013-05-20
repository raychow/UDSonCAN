#pragma once

#include <vector>
#include <list>

#include "WatchWnd.h"

//#define NETWORKLAYER_NOAE

typedef std::vector<BYTE> BYTEVector;

class CApplicationLayer;
class CDataLinkLayer;
class CDiagnosticControl;

class CNetworkLayer
{
	typedef CTIDCWatchWnd::EntryType EntryType;
	typedef CWatchWnd::Color Color;
public:
	enum struct MessageType : BYTE	// 15765-2: 5.3.1
	{
		Unknown = 2,
		Diagnostics = 0,
		RemoteDiagnostics,
	};

	enum struct TargetAddressType : BYTE
	{
		Unknown = 2,
		Physical = 0,
		Functional,
	};

	enum struct Result : BYTE
	{
		N_OK = 0,
		N_TIMEOUT_A,
		N_TIMEOUT_Bs,
		N_TIMEOUT_Cr,
		N_WRONG_SN,
		N_INVALID_FS,
		N_UNEXP_PDU,
		N_WFT_OVRN,
		N_BUFFER_OVFLW,
		N_ERROR
	};

	enum struct PCIType : BYTE
	{
		Unknown = 4,
		SingleFrame = 0,
		FirstFrame,
		ConsecutiveFrame,
		FlowControl,
	};

	enum struct Status : BYTE
	{
		TransmitInProgress = 0,
		ReceiveInProgress,
		Idle
	};

	enum struct FlowControlType : BYTE
	{
		ContinueToSend = 0,
		Wait,
		Overflow
	};

	enum struct TimingType : BYTE
	{
		As,
		Ar,
		Bs,
		Br,
		Cs,
		Cr,
		Idle
	};								// 对何种事件进行定时。

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

	// Status GetStatus() const;		// TODO: 需要修改。

	BOOL IsFullDuplex() const;
	void SetFullDuplex(BOOL bFullDuplex);

	BYTE GetSeparationTimeMin() const;
	void SetSeparationTimeMin(BYTE bySeparationTimeMin);

	BYTE GetBlockSize() const;
	void SetBlockSize(BYTE byBlockSize);

	UINT GetWaitFrameTransimissionMax() const;
	void SetWaitFrameTransimissionMax(UINT nWaitFrameTransimissionMax);

	// 15765-2 5.2
	BOOL Request(MessageType messageType, BYTE bySourceAddress, BYTE byTargetAddress, TargetAddressType targetAddressType, BYTE byAddressExtension, const BYTEVector &vbyData);

	BOOL RequestTesterPresent(BYTE bySourceAddress, BYTE byTargetAddress, TargetAddressType targetAddressType, BYTE byAddressExtension);
	// L_Data.confirm
	void Confirm(UINT nID, BYTE byAddressExtension, BOOL bReverseAddress = FALSE);

	// L_Data.indication
	void Indication(UINT nID, const BYTEVector &vbyData);

	void SetApplicationLayer(CApplicationLayer &applicationLayer);
	void SetDataLinkLayer(CDataLinkLayer &dataLinkLayer);
	void SetDiagnosticControl(CDiagnosticControl &diagnosticControl);
protected:
	enum : UINT
	{
		CANFRAMEDATALENGTHMAX				= 8,			// CAN 帧最大数据长度。
		DIAGNOSTICSFRAMEDATALENGTH			= 7,			// 诊断帧数据长度。
		REMOTEDIAGNOSTICSFRAMEDATALENGTH	= 6,			// 远程诊断帧数据长度。
		J1939IDTEMPLET						= 0x18000000,	// SAE J1939 ID 模板。
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


	struct Message								// 并行消息，ISO 15765-2: 6.8。
	{
		Message();

		CCriticalSection csectionProcess;		// 消息处理临界对象。

		union Address							// 地址信息。
		{
			struct
			{
				BYTE bySourceAddress				: 8;
				BYTE byTargetAddress				: 8;
				TargetAddressType targetAddressType : 7;
				BOOL bTesterPresent					: 1;
				BYTE byAddressExtension				: 8;		// 无法确定具体格式，暂不支持此扩展地址。
			} bitField;
			UINT nUnionAddress;
		} address;
		
		class Compare
		{
		public:
			Compare(UINT nAddress);
			BOOL operator()(const Message *pMessage) const;
		protected:
			UINT m_nAddress;
		};

		Status status;							// 指示收 / 发 / 空闲的状态。
		BYTEVector vbyData;						// 将要发送，或是正在接收的数据。
		BYTEVector::size_type stLocation;		// 将要发送或接收的位置。
		MessageType messageType;				// 消息类型：诊断帧 / 远程诊断帧。
		PCIType pciType;						// 用于 Confirm 的 PCI 类型：SF / FF / CF / FC。
		BYTE bySeparationTimeMin;				// 连续帧发送的最小等待时间。
		UINT nRemainderFrameCount;				// 本次尚要发送的帧数，或是尚要接收的帧数；
												// 发送状态下为 0 表示正在等待流控制帧；
												// 接受状态下为 0 表示将要发送流控制帧。
		BYTE byExpectedSequenceNumber;			// 期望发送或收到的分段序列号。
		DWORD dwTimingStartTick;				// 定时器开始的时间。
		TimingType timingType;					// 对何种事件进行定时。

		void ResetTiming(TimingType timingType, CEvent &eventTiming);
		void AbortMessage();					// 终止消息收发。
	};

	typedef std::list<Message *> PMessageList;
	PMessageList m_lpMessage;

	BOOL m_bFullDuplex;							// 是否工作在全双工状态。

	BYTE m_bySeparationTimeMin;					// 流控制帧包含的发送间隔时间。
	BYTE m_byBlockSize;							// 流控制帧包含的发送块大小。

	UINT m_nWaitFrameTransimissionMax;			// 15765-2: 6.6, （由发送方保证的）流控制等待帧发送的最大次数。

	UINT m_anTimingParameters[6];				// 定时参数 As, Ar, Bs, Br, Cs, Cr

	CApplicationLayer *m_pApplicationLayer;
	CDataLinkLayer *m_pDataLinkLayer;
	CDiagnosticControl *m_pDiagnosticControl;

	CCriticalSection m_csectionProcess;			// 收发处理临界对象。
	CCriticalSection m_csectionMessageList;		// 消息队列临界对象。

	CWinThread *m_pTimingThread;				// 定时线程的指针，此线程应自动删除。

	Message *FindMessage(BOOL bAddIfNotFound, BYTE bySourceAddress, BYTE byTargetAddress, TargetAddressType targetAddressType, BYTE byAddressExtension, BOOL bTesterPresent = FALSE);	// 返回一个指定的消息缓存，如果不存在则添加（并初始化）。
	MessageType GetMessageType(BYTE byPF) const;
	TargetAddressType GetTargetAddressType(BYTE byPF) const;

	BOOL _Request(PCIType pciType, MessageType messageType, BYTE bySourceAddress, BYTE byTargetAddress, TargetAddressType targetAddressType, BYTE byAddressExtension, const BYTEVector &vbyData);

	static UINT _TimingThread(LPVOID lpParam);
	CEvent m_eventTiming;

	CEvent m_eventStopThread;					// 指示正停止线程。
	CEvent m_eventTimingThreadExited;			// 指示 Timing 线程已经退出。

	void _AddWatchEntry(EntryType entryType, UINT nID, UINT nDescriptionID, Color color = Color::Black);
	void _AddWatchEntry(EntryType entryType, UINT nID, const BYTEVector &vbyData, Color color = Color::Black);
	void _AddWatchEntry(EntryType entryType, UINT nID, UINT nDescriptionID, int nData, Color color = Color::Black);

	void _StartThread();
	void _StopThread();
};
