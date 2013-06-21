#include "stdafx.h"
#include "PhysicalLayer.h"

#include <boost/bind.hpp>

using Diagnostic::BYTEVector;

#define _VERIFYDEVICEISOPENED()		if (!m_bDeviceOpened)	{ return FALSE; }
#define _VERIFYDEVICEISNOTOPENED()	if (m_bDeviceOpened)	{ return FALSE; }
#define _VERIFYCANISSTARTED()		if (!m_bCANStarted)		{ return FALSE; }
#define _VERIFYCANISNOTSTARTED()	if (m_bCANStarted)		{ return FALSE; }

CPhysicalLayer::CPhysicalLayer(void)
	: m_bDeviceOpened(FALSE)
	, m_bCANStarted(FALSE)
	, m_dwDeviceType(3)
	, m_dwDeviceIndex(0)
	, m_dwDeviceReserved(0)
	, m_dwCANIndex(0)
	, m_nBaudRateType(0)
	, m_lReceivedLength(0)
{
	ZeroMemory(&m_initConfig, sizeof(m_initConfig));
}

CPhysicalLayer::~CPhysicalLayer(void)
{
	CloseDevice();
}

DWORD CPhysicalLayer::GetDeviceType() const
{
	return m_dwDeviceType;
}

void CPhysicalLayer::SetDeviceType(DWORD dwDeviceType)
{
	m_dwDeviceType = dwDeviceType;
}

DWORD CPhysicalLayer::GetDeviceIndex() const
{
	return m_dwDeviceIndex;
}

void CPhysicalLayer::SetDeviceIndex(DWORD dwDeviceIndex)
{
	m_dwDeviceIndex = dwDeviceIndex;
}

DWORD CPhysicalLayer::GetCANIndex() const
{
	return m_dwCANIndex;
}

void CPhysicalLayer::SetCANIndex(DWORD dwCANIndex)
{
	m_dwCANIndex = dwCANIndex;
}

DWORD CPhysicalLayer::GetAccCode() const
{
	return m_initConfig.AccCode;
}

void CPhysicalLayer::SetAccCode(DWORD dwAccCode)
{
	m_initConfig.AccCode = dwAccCode;
}

DWORD CPhysicalLayer::GetAccMask() const
{
	return m_initConfig.AccMask;
}

void CPhysicalLayer::SetAccMask(DWORD dwAccMask)
{
	m_initConfig.AccMask = dwAccMask;
}

UCHAR CPhysicalLayer::GetFilter() const
{
	return m_initConfig.Filter;
}

void CPhysicalLayer::SetFilter(UCHAR chFilter)
{
	m_initConfig.Filter = chFilter;
}

UINT CPhysicalLayer::GetBaudRateType() const
{
	return m_nBaudRateType;
}

void CPhysicalLayer::SetBaudRateType(UINT nBaudRateType)
{
	m_nBaudRateType = nBaudRateType;
}

UCHAR CPhysicalLayer::GetTiming0() const
{
	return m_initConfig.Timing0;
}

void CPhysicalLayer::SetTiming0(UCHAR chTiming0)
{
	m_initConfig.Timing0 = chTiming0;
}

UCHAR CPhysicalLayer::GetTiming1() const
{
	return m_initConfig.Timing1;
}

void CPhysicalLayer::SetTiming1(UCHAR chTiming1)
{
	m_initConfig.Timing1 = chTiming1;
}

UCHAR CPhysicalLayer::GetMode() const
{
	return m_initConfig.Mode;
}

void CPhysicalLayer::SetMode(UCHAR chMode)
{
	m_initConfig.Mode = chMode;
}

BOOL CPhysicalLayer::OpenDevice()
{
	_VERIFYDEVICEISNOTOPENED();

	if (VCI_OpenDevice(m_dwDeviceType, m_dwDeviceIndex, 0) != STATUS_OK)
	{
		return FALSE;
	}
	m_bDeviceOpened = TRUE;

	return TRUE;
}

BOOL CPhysicalLayer::CloseDevice()
{
	_VERIFYDEVICEISOPENED();
	
	if (m_bCANStarted && !ResetCAN() || !VCI_CloseDevice(m_dwDeviceType, m_dwDeviceIndex) == STATUS_OK)	// TODO: 是否相反？
	{
		return FALSE;
	}
	m_bDeviceOpened = FALSE;

	return TRUE;
}

BOOL CPhysicalLayer::IsDeviceOpened() const
{
	return m_bDeviceOpened;
}

UINT CPhysicalLayer::GetCANCount() const
{
	VCI_BOARD_INFO boardInfo;
	if (VCI_ReadBoardInfo(m_dwDeviceType, m_dwDeviceIndex, &boardInfo) != STATUS_OK)
	{
		return 0;
	}
	return boardInfo.can_Num;
}

BOOL CPhysicalLayer::StartCAN()
{
	_VERIFYDEVICEISOPENED();
	_VERIFYCANISNOTSTARTED();

	// 初始化 CAN
	if (VCI_InitCAN(m_dwDeviceType, m_dwDeviceIndex, m_dwCANIndex, &m_initConfig) != STATUS_OK)
	{
		return FALSE;
	}

	// 开始 CAN
	if (VCI_StartCAN(m_dwDeviceType, m_dwDeviceIndex, m_dwCANIndex) != STATUS_OK)
	{
		return FALSE;
	}

	m_bCANStarted = TRUE;
	_StartThread();

	return TRUE;
}

BOOL CPhysicalLayer::ResetCAN()
{
	_VERIFYDEVICEISOPENED();
	_VERIFYCANISSTARTED();

	_StopThread();
	if(VCI_ResetCAN(m_dwDeviceType, m_dwDeviceIndex, m_dwCANIndex) != STATUS_OK)
	{
		return FALSE;
	}

	m_bCANStarted = FALSE;

	return TRUE;
}

BOOL CPhysicalLayer::IsCANStarted() const
{
	return m_bCANStarted;
}

BOOL CPhysicalLayer::Transmit(UINT32 nID, const BYTEVector &vbyData, Diagnostic::PhysicalLayerSendType sendType, BOOL bRemoteFrame, BOOL bExternFrame)
{
	_VERIFYDEVICEISOPENED();
	_VERIFYCANISSTARTED();

	VCI_CAN_OBJ canObj[1];
	ZeroMemory(&canObj[0], sizeof(canObj[0]));

	// TODO: 输出物理层待发送讯息

	for (int i = 0; i != MAXDATALENGTH; ++i)
	{
		canObj[0].Data[i] = vbyData.at(i);
	}
	canObj[0].DataLen = vbyData.size();
	canObj[0].ExternFlag = bExternFrame;
	canObj[0].ID = nID;
	canObj[0].RemoteFlag = bRemoteFrame;
	canObj[0].SendType = sendType;

	boost::lock_guard<boost::mutex> lockGuard(m_mutexConfirm);
	if (VCI_Transmit(m_dwDeviceType, m_dwDeviceIndex, m_dwCANIndex, canObj, 1) == STATUS_OK)
	{
		// 输出物理层发送成功讯息
		m_lstConfirm.push_back(nID);
		m_condConfirm.notify_one();	// m_mutexConfirm 在 m_condConfirm.wait 期间应是解锁的。
		return TRUE;	// Confirm
	}
	else
	{
		// TODO: 输出物理层发送失败讯息
	}

	return FALSE;
}

BOOL CPhysicalLayer::_StartThread()
{
	_VERIFYDEVICEISOPENED();
	_VERIFYCANISSTARTED();

	m_threadReceive = boost::thread(boost::bind(&CPhysicalLayer::_ReceiveThread, this));
	m_threadConfirm = boost::thread(boost::bind(&CPhysicalLayer::_ConfirmThread, this));

	return TRUE;
}

void CPhysicalLayer::_StopThread()
{
	m_threadReceive.interrupt();
	m_threadConfirm.interrupt();

	//UINT nUnexitedThreadsCount = 2;
	//std::unique_ptr<HANDLE[]> upahandleThreadsExited(new HANDLE[nUnexitedThreadsCount]);

	boost::unique_lock<boost::mutex> uniqueLockCTE(m_mutexConfirmThreadExited);
	boost::unique_lock<boost::mutex> uniqueLockRTE(m_mutexReceiveThreadExited);

	m_condConfirmThreadExited.wait(uniqueLockCTE);
	m_condReceiveThreadExited.wait(uniqueLockRTE);

	//DWORD dwWaitResult = -1;
	//MSG msg;
	//while (nUnexitedThreadsCount)
	//{
	//	dwWaitResult = MsgWaitForMultipleObjects(2, upahandleThreadsExited.get(), FALSE, INFINITE, QS_ALLINPUT);	// 等待所有线程退出。
	//	switch (dwWaitResult)
	//	{
	//	case WAIT_OBJECT_0:
	//		--nUnexitedThreadsCount;
	//		break;
	//	case WAIT_OBJECT_0 + 1:
	//		--nUnexitedThreadsCount;
	//		break;
	//	case WAIT_OBJECT_0 + 2:
	//		PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);  
	//		DispatchMessage(&msg);
	//		break;
	//	}
	//}
}

void CPhysicalLayer::_ReceiveThread()
{
	ULONG lReceivedLength;
	// 清空 CAN 接收缓冲区；
	VCI_ClearBuffer(m_dwDeviceIndex, m_dwDeviceIndex, m_dwCANIndex);
	VCI_CAN_OBJ canObj[CANRECEIVECOUNT];
	VCI_ERR_INFO errInfo;
	try
	{
		while (TRUE)
		{
#ifdef TESTCODE
			CSingleLock lockReceiveData(&m_csectionReceiveData);
			lockReceiveData.Lock();
			if (lReceivedLength = m_lReceivedLength)
			{
				m_lReceivedLength = 0;
				canObj[0] = m_canObj;
			}
#elif
			lReceivedLength = VCI_Receive(m_dwDeviceIndex, m_dwDeviceIndex, m_dwCANIndex, canObj, CANRECEIVECOUNT, 0);
#endif
			if (lReceivedLength == CANRECEIVEFAILEDFALG)
			{
				// 如果没有读到数据，必须调用此函数来读取出当前的错误码。
				VCI_ReadErrInfo(m_dwDeviceType, m_dwDeviceIndex, m_dwCANIndex, &errInfo);
				// TODO: 输出 errInfo 包含的错误讯息。
			}
			else if (lReceivedLength)
			{
				for (ULONG i = 0; i != lReceivedLength; ++i)
				{
					if (!canObj[i].RemoteFlag)
					{
						TRACE(_T("\nPhysicalLayer::_ReceiveThread:\nData (Hex): "));
						for (int j = 0; j != MAXDATALENGTH; ++j)
						{
							TRACE(_T("%X "), canObj[i].Data[j]);
						}
						TRACE(_T("\nDataLen: %d, ExternFlag: %d, ID: 0x%X, RemoteFlag: %d, SendType: %d\n"), canObj[i].DataLen, canObj[i].ExternFlag, canObj[i].ID, canObj[i].RemoteFlag, canObj[i].SendType);

						// 通知链路层
						BYTEVector vbyData(canObj[i].Data, canObj[i].Data + canObj[i].DataLen);
						m_signalIndication(canObj[i].ID, vbyData);
					}
				}
			}
			lockReceiveData.Unlock();
			boost::this_thread::sleep(boost::posix_time::microseconds(CANRECEIVECYCLE));
		}
	}
	catch (const boost::thread_interrupted &)
	{
		m_condReceiveThreadExited.notify_all();
	}
}

void CPhysicalLayer::_ConfirmThread()
{
	UINT32 nID;
	try
	{
		boost::unique_lock<boost::mutex> uniqueLock(m_mutexConfirm);	// m_mutexConfirm 在 wait 之后仍保持锁定。
		while (TRUE)
		{
			m_condConfirm.wait(uniqueLock);
			if (!m_lstConfirm.empty())
			{
				nID = m_lstConfirm.front();
				m_lstConfirm.pop_front();
				TRACE(_T("\nPhysicalLayer::Confirm: 0x%X\n"), nID);
				uniqueLock.unlock();
				m_signalConfirm();
			}
			uniqueLock.lock();
		}
	}
	catch (const boost::thread_interrupted &)
	{
		m_condConfirmThreadExited.notify_all();
	}
}

boost::signals2::connection CPhysicalLayer::ConnectIndication(const Diagnostic::IndicationSignal::slot_type &subscriber)
{
	return m_signalIndication.connect(subscriber);
}

boost::signals2::connection CPhysicalLayer::ConnectConfirm(const Diagnostic::ConfirmSignal::slot_type &subscriber)
{
	return m_signalConfirm.connect(subscriber);
}

#ifdef TESTCODE
DWORD __stdcall VCI_OpenDevice(DWORD DeviceType, DWORD DeviceInd, DWORD Reserved)
{
	return STATUS_OK;
}

DWORD __stdcall VCI_CloseDevice(DWORD DeviceType, DWORD DeviceInd)
{
	return STATUS_OK;
}

DWORD __stdcall VCI_ReadBoardInfo(DWORD DeviceType, DWORD DeviceInd, PVCI_BOARD_INFO pInfo)
{
	pInfo->can_Num = 2;
	return STATUS_OK;
}

DWORD __stdcall VCI_InitCAN(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_INIT_CONFIG pInitConfig)
{
	return STATUS_OK;
}

DWORD __stdcall VCI_StartCAN(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd)
{
	return STATUS_OK;
}

DWORD __stdcall VCI_ResetCAN(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd)
{
	return STATUS_OK;
}

ULONG __stdcall VCI_Transmit(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_CAN_OBJ pSend, ULONG Len)
{
	return STATUS_OK;
}

DWORD __stdcall VCI_ReadErrInfo(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_ERR_INFO pErrInfo)
{
	return STATUS_OK;
}
#endif