#pragma once

#include <list>
#include <vector>

#include <boost/thread.hpp>

#include "ControlCAN\ControlCAN.h"
#include "DiagnosticType.h"

#define TESTCODE		// 生成在无硬件支援时测试使用的虚拟代码。

class CPhysicalLayer
{
public:
	enum : UINT
	{
		MAXDATALENGTH			= 8,			// CAN 帧最大数据长度。
		CANRECEIVECOUNT			= 10,			// 每次最多接收帧数。
		CANRECEIVECYCLE			= 10,			// 接收周期。
		CANRECEIVEFAILEDFALG	= 0xFFFFFFFF	// 接收失败标志。
	};

	CPhysicalLayer(void);
	virtual ~CPhysicalLayer(void);

	DWORD GetDeviceType() const;
	void SetDeviceType(DWORD dwDeviceType);
	
	DWORD GetDeviceIndex() const;
	void SetDeviceIndex(DWORD dwDeviceIndex);

	DWORD GetCANIndex() const;
	void SetCANIndex(DWORD dwCANIndex);

	DWORD GetAccCode() const;
	void SetAccCode(DWORD dwAccCode);

	DWORD GetAccMask() const;
	void SetAccMask(DWORD dwAccMask);

	UCHAR GetFilter() const;
	void SetFilter(UCHAR chFilter);

	UINT GetBaudRateType() const;
	void SetBaudRateType(UINT nBaudRateType);

	UCHAR GetTiming0() const;
	void SetTiming0(UCHAR chTiming0);

	UCHAR GetTiming1() const;
	void SetTiming1(UCHAR chTiming1);

	UCHAR GetMode() const;
	void SetMode(UCHAR chMode);

	BOOL OpenDevice();
	BOOL CloseDevice();
	BOOL IsDeviceOpened() const;

	UINT GetCANCount() const;

	BOOL StartCAN();
	BOOL ResetCAN();
	BOOL IsCANStarted() const;

	BOOL Transmit(UINT32 nID, const Diagnostic::BYTEVector &vbyData, Diagnostic::PhysicalLayerSendType sendType = Diagnostic::PhysicalLayerSendType::Normal, BOOL bRemoteFrame = FALSE, BOOL bExternFrame = FALSE);
	
	boost::signals2::connection ConnectIndication(const Diagnostic::IndicationSignal::slot_type &subscriber);
	boost::signals2::connection ConnectConfirm(const Diagnostic::ConfirmSignal::slot_type &subscriber);

#ifdef TESTCODE
	CCriticalSection m_csectionReceiveData;
	VCI_CAN_OBJ m_canObj;
	ULONG m_lReceivedLength;
#endif
protected:
	typedef std::list<UINT32> ConfirmList;

	BOOL m_bDeviceOpened;
	BOOL m_bCANStarted;

	DWORD m_dwDeviceType;
	DWORD m_dwDeviceIndex;
	DWORD m_dwDeviceReserved;
	DWORD m_dwCANIndex;

	UINT m_nBaudRateType;

	VCI_INIT_CONFIG m_initConfig;
	
	Diagnostic::IndicationSignal m_signalIndication;
	Diagnostic::ConfirmSignal m_signalConfirm;

	boost::thread m_threadReceive;
	BOOL _StartThread();
	void _StopThread();

	void _ReceiveThread();

	ConfirmList m_lstConfirm;
	boost::condition_variable m_condConfirm;
	boost::mutex m_mutexConfirm;
	boost::thread m_threadConfirm;
	/***************************************************************
	 * 验证线程，实现定时要求验证作为线程。
	 ***************************************************************/
	void _ConfirmThread();

	boost::condition_variable m_condConfirmThreadExited;
	boost::mutex m_mutexConfirmThreadExited;
	boost::condition_variable m_condReceiveThreadExited;
	boost::mutex m_mutexReceiveThreadExited;
};

