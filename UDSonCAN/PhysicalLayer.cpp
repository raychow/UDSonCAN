#include "stdafx.h"
#include "PhysicalLayer.h"

#include "DataLinkLayer.h"

#define _VERIFYDEVICEISOPENED()		if (!m_bDeviceOpened) { return FALSE; }
#define _VERIFYDEVICEISNOTOPENED()	if (m_bDeviceOpened) { return FALSE; }
#define _VERIFYCANISSTARTED()		if (!m_bCANStarted) { return FALSE; }
#define _VERIFYCANISNOTSTARTED()	if (m_bCANStarted) { return FALSE; }

CPhysicalLayer::CPhysicalLayer(void)
	: m_bDeviceOpened(FALSE)
	, m_bCANStarted(FALSE)
	, m_dwDeviceType(3)
	, m_dwDeviceIndex(0)
	, m_dwDeviceReserved(0)
	, m_dwCANIndex(0)
	, m_nBaudRateType(0)
	, m_pDataLinkLayer(NULL)
	, m_pDiagnosticControl(NULL)
	, m_pReceiveThread(NULL)
	, m_pConfirmThread(NULL)
	, m_eventExitReceive(FALSE, TRUE)
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
#ifdef TESTCODE
	_VERIFYDEVICEISNOTOPENED();

	m_bDeviceOpened = TRUE;
	return TRUE;
#endif

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
#ifdef TESTCODE
	_VERIFYDEVICEISOPENED();
	if (m_bCANStarted && !(ResetCAN()))
	{
		return FALSE;
	}
	m_bDeviceOpened = FALSE;

	return TRUE;
#endif

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
#ifdef TESTCODE
	return 2;
#endif

	VCI_BOARD_INFO boardInfo;
	if (VCI_ReadBoardInfo(m_dwDeviceType, m_dwDeviceIndex, &boardInfo) != STATUS_OK)
	{
		return 0;
	}
	return boardInfo.can_Num;
}

BOOL CPhysicalLayer::StartCAN()
{
#ifdef TESTCODE
	_VERIFYDEVICEISOPENED();
	_VERIFYCANISNOTSTARTED();

	m_bCANStarted = TRUE;
	_StartThread();

	return TRUE;

#endif

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
#ifdef TESTCODE
	_VERIFYDEVICEISOPENED();
	_VERIFYCANISSTARTED();

	_StopThread();

	m_bCANStarted = FALSE;

	return TRUE;

#endif

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

BOOL CPhysicalLayer::Transmit(UINT nID, const BYTEVector &vbyData, SendType sendType, BOOL bRemoteFrame, BOOL bExternFrame, BOOL bConfirmReserveAddress)
{
#ifdef TESTCODE
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

	TRACE(_T("CPhysicalLayer::Transmit:\n\tData: "));
	for (int i = 0; i != MAXDATALENGTH; ++i)
	{
		TRACE(_T("%X "), canObj[0].Data[i]);
	}
	TRACE(_T("\n\tDataLen: %d, ExternFlag: %d, ID: 0x%X, RemoteFlag: %d, SendType: %d\n"), canObj[0].DataLen, canObj[0].ExternFlag, canObj[0].ID, canObj[0].RemoteFlag, canObj[0].SendType);

	CSingleLock lockTransmit(&m_csectionTransmit);
	lockTransmit.Lock();
	if (TRUE)
	{
		// TODO: 输出物理层发送成功讯息。
		ConfirmBuffer *pConfirmBuffer = new ConfirmBuffer();
		pConfirmBuffer->nID = nID;
		pConfirmBuffer->byDataFirstFrame = vbyData.at(0);
		pConfirmBuffer->bConfirmReverseAddress = bConfirmReserveAddress;
		m_lpConfirmBuffer.push_back(pConfirmBuffer);
		m_eventConfirm.SetEvent();
		return TRUE;	// Confirm
	}
	else
	{
		// TODO: 输出物理层发送失败讯息
	}
	lockTransmit.Unlock();

	return FALSE;
#elif

	_VERIFYDEVICEISOPENED();
	_VERIFYCANISSTARTED();

	VCI_CAN_OBJ canObj[1];
	ZeroMemory(canObj[0], sizeof(canObj[0]));

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

	CSingleLock lockTransmit(&m_csectionTransmit);
	lockTransmit.Lock();
	if (VCI_Transmit(m_deviceType, m_dwDeviceIndex, m_dwCANIndex, canObj, 1) == STATUS_OK)
	{
		// 输出物理层发送成功讯息
		m_nID = nID;
		m_byDataFirstFrame = vbyData.at(0);
		m_bConfirmReverse = bConfirmReserve;
		m_eventConfirm.SetEvent();
		return TRUE;	// Confirm
	}
	else
	{
		m_eventTransmitReady.SetEvent();
		// TODO: 输出物理层发送失败讯息
	}
	lockTransmit.Lock();

	return FALSE;

#endif
}

BOOL CPhysicalLayer::_StartThread()
{
	_VERIFYDEVICEISOPENED();
	_VERIFYCANISSTARTED();

	m_eventExitReceive.ResetEvent();
	if (!m_pReceiveThread)
	{
		m_pReceiveThread = AfxBeginThread(_ReceiveThread, this, 0U, 0U, CREATE_SUSPENDED);
		m_pReceiveThread->m_bAutoDelete = TRUE;
		m_pReceiveThread->ResumeThread();
	}
	if (!m_pConfirmThread)
	{
		m_pConfirmThread = AfxBeginThread(_ConfirmThread, this, 0U, 0U, CREATE_SUSPENDED);
		m_pConfirmThread->m_bAutoDelete = TRUE;
		m_pConfirmThread->ResumeThread();
	}

	return TRUE;
}

void CPhysicalLayer::_StopThread()
{
	m_eventExitReceive.SetEvent();

	UINT nUnexitedThreadsCount = 2;
	HANDLE *phThreadsExited = new HANDLE[nUnexitedThreadsCount];
	phThreadsExited[0] = m_eventConfirmThreadExited.m_hObject;
	phThreadsExited[1] = m_eventReceiveThreadExited.m_hObject;
	DWORD dwWaitResult = -1;
	MSG msg;
	while (nUnexitedThreadsCount)
	{
		dwWaitResult = MsgWaitForMultipleObjects(2, phThreadsExited, FALSE, INFINITE, QS_ALLINPUT);	// 等待所有线程退出。
		switch (dwWaitResult)
		{
		case WAIT_OBJECT_0:
			--nUnexitedThreadsCount;
			break;
		case WAIT_OBJECT_0 + 1:
			--nUnexitedThreadsCount;
			break;
		case WAIT_OBJECT_0 + 2:
			PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);  
			DispatchMessage(&msg);
			break;
		}
	}
	delete [] phThreadsExited;

	CSingleLock lockTransmit(&m_csectionTransmit);
	lockTransmit.Lock();
	while (!m_lpConfirmBuffer.empty())
	{
		delete m_lpConfirmBuffer.front();
		m_lpConfirmBuffer.pop_front();
	}
	lockTransmit.Unlock();
}

UINT CPhysicalLayer::_ReceiveThread(LPVOID lpParam)
{

#ifdef TESTCODE

	CPhysicalLayer *pThis = static_cast<CPhysicalLayer *>(lpParam);
	ASSERT(pThis->m_pDataLinkLayer);

	DWORD dwWaitResult;
	// 清空 CAN 接收缓冲区；
	ULONG lReceivedLength;
	VCI_CAN_OBJ canObj[CANRECEIVECOUNT];
	CSingleLock lockReceiveData(&pThis->m_csectionReceiveData);
	do
	{
		lockReceiveData.Lock();

		lReceivedLength = pThis->m_lReceivedLength;
		pThis->m_lReceivedLength = 0;
		canObj[0] = pThis->m_canObj;

		lockReceiveData.Unlock();
		if (lReceivedLength == CANRECEIVEFAILEDFALG)
		{
			// 如果没有读到数据，必须调用此函数来读取出当前的错误码。
			// TODO: 输出 errInfo 包含的错误讯息。
		}
		else if (lReceivedLength)
		{
			for (ULONG i = 0; i != 1; ++i)
			{
				if (!canObj[i].RemoteFlag)
				{
					// TODO: 输出物理层跟踪信息
					TRACE(_T("\nPhysicalLayer::_ReceiveThread:\nData (Hex): "));
					for (int j = 0; j != MAXDATALENGTH; ++j)
					{
						TRACE(_T("%X "), canObj[i].Data[j]);
					}
					TRACE(_T("\nDataLen: %d, ExternFlag: %d, ID: 0x%X, RemoteFlag: %d, SendType: %d\n"), canObj[i].DataLen, canObj[i].ExternFlag, canObj[i].ID, canObj[i].RemoteFlag, canObj[i].SendType);

					// 通知链路层
					BYTEVector vbyData(canObj[i].Data, canObj[i].Data + canObj[i].DataLen);
					pThis->m_pDataLinkLayer->Indication(canObj[i].ID, vbyData);
				}
			}
		}
		lockReceiveData.Unlock();
		dwWaitResult = WaitForSingleObject(pThis->m_eventExitReceive.m_hObject, CANRECEIVECYCLE);
	} while (dwWaitResult != WAIT_OBJECT_0);
	pThis->m_eventReceiveThreadExited.SetEvent();
	pThis->m_pReceiveThread = NULL;
	return 0;

#elif

	CPhysicalLayer *pThis = static_cast<CPhysicalLayer *>(lpParam);
	ASSERT(pThis->m_pDataLinkLayer);

	DWORD dwWaitResult;
	// 清空 CAN 接收缓冲区；
	VCI_ClearBuffer(pThis->m_dwDeviceIndex, pThis->m_dwDeviceIndex, pThis->m_dwCANIndex);
	ULONG lReceivedLength;
	VCI_CAN_OBJ canObj[CANRECEIVECOUNT];
	VCI_ERR_INFO errInfo;
	do
	{
		lReceivedLength = VCI_Receive(pThis->m_dwDeviceIndex, pThis->m_dwDeviceIndex, pThis->m_dwCANIndex, canObj, CANRECEIVECOUNT, 0);
		if (lReceivedLength == CANRECEIVEFAILEDFALG)
		{
			// 如果没有读到数据，必须调用此函数来读取出当前的错误码。
			VCI_ReadErrInfo(pThis->m_deviceType, pThis->m_dwDeviceIndex, pThis->m_dwCANIndex, &errInfo);
			// TODO: 输出 errInfo 包含的错误讯息。
		}
		else if (lReceivedLength)
		{
			for (ULONG i = 0; i != lReceivedLength; ++i)
			{
				if (!canObj[i].RemoteFlag)
				{
					// TODO: 输出物理层跟踪信息

					// 通知链路层
					BYTEVector vbyData(canObj[i].Data, canObj[i].Data + canObj[i].DataLen);
					pThis->m_pDataLinkLayer->Indication(canObj[i].ID, vbyData);
				}
			}
		}
		dwWaitResult = WaitForSingleObject(pThis->m_eventExitReceive.m_hObject, CANRECEIVECYCLE);
	} while (dwWaitResult != WAIT_OBJECT_0);
	pThis->m_eventReceiveThreadExited.SetEvent();
	pThis->m_pReceiveThread = NULL;
	return 0;

#endif
}

UINT CPhysicalLayer::_ConfirmThread(LPVOID lpParam)
{
	CPhysicalLayer *pThis = static_cast<CPhysicalLayer *>(lpParam);
	ASSERT(pThis->m_pConfirmThread);
	ASSERT(pThis->m_pDataLinkLayer);

	DWORD dwWaitResult = -1;
	HANDLE hEvents[] = { pThis->m_eventExitReceive.m_hObject, pThis->m_eventConfirm.m_hObject };
	CSingleLock lockTransmit(&pThis->m_csectionTransmit);
	ConfirmBuffer *pConfirmBuffer;

	while (dwWaitResult != WAIT_OBJECT_0)
	{
		dwWaitResult = WaitForMultipleObjects(sizeof(hEvents) / sizeof(HANDLE), hEvents, FALSE, INFINITE);

		if (dwWaitResult == WAIT_OBJECT_0 + 1)
		{
			lockTransmit.Lock();
			if (!pThis->m_lpConfirmBuffer.empty())
			{
				pConfirmBuffer = pThis->m_lpConfirmBuffer.front();
				pThis->m_lpConfirmBuffer.pop_front();
				TRACE(_T("\nPhysicalLayer::Confirm: 0x%X\n"), pConfirmBuffer->nID);
				lockTransmit.Unlock();
				pThis->m_pDataLinkLayer->Confirm(pConfirmBuffer->nID, pConfirmBuffer->byDataFirstFrame, pConfirmBuffer->bConfirmReverseAddress);
				delete pConfirmBuffer;
				lockTransmit.Lock();
			}
			lockTransmit.Unlock();
		}
	}
	pThis->m_eventConfirmThreadExited.SetEvent();
	pThis->m_pConfirmThread = NULL;
	return 0;
}


void CPhysicalLayer::SetDataLinkLayer(CDataLinkLayer &dataLinkLayer)
{
	m_pDataLinkLayer = &dataLinkLayer;
}

void CPhysicalLayer::SetDiagnosticControl(CDiagnosticControl &diagnosticControl)
{
	m_pDiagnosticControl = &diagnosticControl;
}

