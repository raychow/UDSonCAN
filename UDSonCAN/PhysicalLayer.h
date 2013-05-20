#pragma once

#include <vector>
#include <list>

#include "ControlCAN\ControlCAN.h"

#define TESTCODE		// 生成在无硬件支援时测试使用的虚拟代码。

typedef std::vector<BYTE> BYTEVector;

class CDataLinkLayer;
class CDiagnosticControl;

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

	enum SendType
	{
		Normal		= 0,
		Single,
		LoopBack,
		SingleLoopBack,
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

	BOOL Transmit(UINT nID, const BYTEVector &vbyData, SendType sendType = SendType::Normal, BOOL bRemoteFrame = FALSE, BOOL bExternFrame = FALSE, BOOL bConfirmReserveAddress = FALSE);
	
	void SetDataLinkLayer(CDataLinkLayer &dataLinkLayer);
	void SetDiagnosticControl(CDiagnosticControl &diagnosticControl);

#ifdef TESTCODE
	CCriticalSection m_csectionReceiveData;
	VCI_CAN_OBJ m_canObj;
	ULONG m_lReceivedLength;
#endif
protected:
	struct ConfirmBuffer
	{
		UINT nID;						// 待 Confirm 的 ID。
		BYTE byDataFirstFrame;			// 数据第一帧，用于验证扩展地址。
		BOOL bConfirmReverseAddress;	// Confirm 时反转源地址与目标地址，针对 FC。
	};
	typedef std::list<ConfirmBuffer *> PConfirmBufferList;

	BOOL m_bDeviceOpened;
	BOOL m_bCANStarted;

	DWORD m_dwDeviceType;
	DWORD m_dwDeviceIndex;
	DWORD m_dwDeviceReserved;
	DWORD m_dwCANIndex;

	UINT m_nBaudRateType;

	VCI_INIT_CONFIG m_initConfig;
	
	CDataLinkLayer *m_pDataLinkLayer;
	CDiagnosticControl *m_pDiagnosticControl;
	
	CWinThread *m_pReceiveThread;
	BOOL _StartThread();
	void _StopThread();
	static UINT _ReceiveThread(LPVOID lpParam);

	CCriticalSection m_csectionTransmit;
	PConfirmBufferList m_lpConfirmBuffer;
	CEvent m_eventConfirm;
	CWinThread *m_pConfirmThread;
	/***************************************************************
	 * 验证线程，防止出现过深函数调用栈:
	 * 例如，网络层发送连续帧，若在物理层立即返回，则网络层立即发送
	 * 下个连续帧，导致调用栈过深。
	 ***************************************************************/
	static UINT _ConfirmThread(LPVOID lpParam);

	CEvent m_eventExitReceive;
	CEvent m_eventConfirmThreadExited;
	CEvent m_eventReceiveThreadExited;
};

