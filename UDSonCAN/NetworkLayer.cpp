#include "stdafx.h"
#include "NetworkLayer.h"

#include <algorithm>

#include "DiagnosticControl.h"
#include "resource.h"

using std::lock_guard;
using std::mutex;
using std::recursive_mutex;
using std::unique_lock;

using Diagnostic::BYTEVector;

CNetworkLayer::MessageBuffer::MessageBuffer()
	: status(Status::Idle)
	, stLocation(0)
	, pciType(PCIType::Unknown)
	, bySeparationTimeMin(0)
	, nRemainderFrameCount(0)
	, byExpectedSequenceNumber(0)
	, dwTimingStartTick(0)
	, timingType(TimingType::Idle)
{
}

BOOL CNetworkLayer::MessageBuffer::IsBusy()
{
	lock_guard<recursive_mutex> lg(rmutexMessageBuffer);
	return status != Status::Idle;
}

BOOL CNetworkLayer::MessageBuffer::IsRequest()
{
	lock_guard<recursive_mutex> lg(rmutexMessageBuffer);
	return  status == Status::TransmitInProgress && pciType != PCIType::FlowControl
		|| status == Status::ReceiveInProgress && pciType == PCIType::FlowControl;
}

void CNetworkLayer::MessageBuffer::ResetTiming(TimingType timingType, CEvent &eventTiming)
{
	TRACE(_T("CNetworkLayer::ResetTiming: %d\n"), timingType);
	
	{
		lock_guard<recursive_mutex> lg(rmutexMessageBuffer);

		this->timingType = timingType;
		dwTimingStartTick = GetTickCount();
	}

	if (timingType != TimingType::Idle)
	{
		eventTiming.SetEvent();
	}
}

void CNetworkLayer::MessageBuffer::ClearMessage()
{
	lock_guard<recursive_mutex> lg(rmutexMessageBuffer);
	status = Status::Idle;
	vbyData.clear();
	stLocation = 0;
	pciType = PCIType::Unknown;
	bySeparationTimeMin = 0;
	nRemainderFrameCount = 0;
	byExpectedSequenceNumber = 0;
	dwTimingStartTick = 0;
	timingType = TimingType::Idle;
}

CNetworkLayer::CNetworkLayer(void)
	: m_bySeparationTimeMin(0)
	, m_byBlockSize(0xFF)
	, m_nWaitFrameTransimissionMax(0)
	, m_pTimingThread(NULL)
	, m_eventTiming(FALSE, TRUE)
	, m_eventStopThread(FALSE, TRUE)
{
	ZeroMemory(m_anTimingParameters, sizeof(m_anTimingParameters));
	_StartThread();
}

CNetworkLayer::~CNetworkLayer(void)
{
	_StopThread();
	TRACE(_T("CNetworkLayer::~CNetworkLayer.\n"));
}

UINT CNetworkLayer::GetAs() const
{
	return m_anTimingParameters[0];
}

void CNetworkLayer::SetAs(UINT nAs)
{
	m_anTimingParameters[0] = nAs;
}

UINT CNetworkLayer::GetAr() const
{
	return m_anTimingParameters[1];
}

void CNetworkLayer::SetAr(UINT nAr)
{
	m_anTimingParameters[1] = nAr;
}

UINT CNetworkLayer::GetBs() const
{
	return m_anTimingParameters[2];
}

void CNetworkLayer::SetBs(UINT nBs)
{
	m_anTimingParameters[2] = nBs;
}

UINT CNetworkLayer::GetBr() const
{
	return m_anTimingParameters[3];
}

void CNetworkLayer::SetBr(UINT nBr)
{
	m_anTimingParameters[3] = nBr;
}

UINT CNetworkLayer::GetCs() const
{
	return m_anTimingParameters[4];
}

void CNetworkLayer::SetCs(UINT nCs)
{
	m_anTimingParameters[4] = nCs;
}

UINT CNetworkLayer::GetCr() const
{
	return m_anTimingParameters[5];
}

void CNetworkLayer::SetCr(UINT nCr)
{
	m_anTimingParameters[5] = nCr;
}

BYTE CNetworkLayer::GetSeparationTimeMin() const
{
	return m_bySeparationTimeMin;
}

void CNetworkLayer::SetSeparationTimeMin(BYTE bySeparationTimeMin)
{
	m_bySeparationTimeMin = bySeparationTimeMin;
}

BYTE CNetworkLayer::GetBlockSize() const
{
	return m_byBlockSize;
}

void CNetworkLayer::SetBlockSize(BYTE byBlockSize)
{
	m_byBlockSize = byBlockSize;
}

UINT CNetworkLayer::GetWaitFrameTransimissionMax() const
{
	return m_nWaitFrameTransimissionMax;
}

void CNetworkLayer::SetWaitFrameTransimissionMax(UINT nWaitFrameTransimissionMax)
{
	m_nWaitFrameTransimissionMax = nWaitFrameTransimissionMax;
}

void CNetworkLayer::Request(UINT32 nID, const BYTEVector &vbyData)
{
	if (!_FillBuffer(Status::TransmitInProgress, nID, vbyData))	// 当前尚有未完成的发送任务。
	{
		return;
	}
	_Request();
	//if (vbyData.size() < DIAGNOSTICSFRAMEDATALENGTH)
	//{
	//	return _Request(PCIType::SingleFrame, nID, vbyData);
	//}
	//else
	//{
	//	return _Request(PCIType::FirstFrame, nID, vbyData);
	//}
}

BOOL CNetworkLayer::_FillBuffer(Status status, UINT32 nID, const BYTEVector &vbyData)
{
	std::unique_lock<recursive_mutex> ul(m_messageBuffer.rmutexMessageBuffer, std::try_to_lock);
	if (!ul.owns_lock() || m_messageBuffer.IsBusy())
	{
		return FALSE;
	}
	m_messageBuffer.status = status;
	m_messageBuffer.nID = nID;
	m_messageBuffer.vbyData = vbyData;
	m_messageBuffer.stLocation = 0;
	m_messageBuffer.bySeparationTimeMin = 0;
	m_messageBuffer.nRemainderFrameCount = 0;
	m_messageBuffer.byExpectedSequenceNumber = 0;
	m_messageBuffer.dwTimingStartTick = 0;
	m_messageBuffer.timingType = TimingType::Idle;

	if (vbyData.size() < DIAGNOSTICSFRAMEDATALENGTH)
	{
		m_messageBuffer.pciType = PCIType::SingleFrame;
	}
	else
	{
		m_messageBuffer.pciType = PCIType::FirstFrame;
	}
	return TRUE;
}

void CNetworkLayer::_Request()
{
	TRACE(_T("\nNetworkLayer::_Request.\n"));
	// _AddWatchEntry(EntryType::Transmit, nID, IDS_NETWORKLAYERWATCH_REQUEST);

	unique_lock<recursive_mutex> ul(m_messageBuffer.rmutexMessageBuffer);

	if (!m_messageBuffer.IsRequest())
	{
		TRACE(_T("Status is incorrect.\n"));
		ASSERT(FALSE);
		return;
	}

	// Data
	BYTEVector vbyPDU;
	vbyPDU.reserve(CANFRAMEDATALENGTHMAX);

	BYTE byPCIFirst;
	byPCIFirst = static_cast<BYTE>(m_messageBuffer.pciType) << 4;
	UINT nDataSize = m_messageBuffer.vbyData.size();
	UINT nPDUDataSize = DIAGNOSTICSFRAMEDATALENGTH;	// PDU 包含的数据长度，在首帧中需要减一，流控制帧中不包含数据帧。

	switch (m_messageBuffer.pciType)
	{
	case PCIType::SingleFrame:
		TRACE(_T("SingleFrame.\n"));
		_AddWatchEntry(EntryType::Transmit, m_messageBuffer.nID, IDS_NETWORKLAYERWATCH_REQUESTSINGLEFRAME);

		if (nDataSize > nPDUDataSize)
		{
			nDataSize = nPDUDataSize;
		}
		byPCIFirst = byPCIFirst | nDataSize;
		vbyPDU.push_back(byPCIFirst);
		vbyPDU.insert(vbyPDU.cend(), m_messageBuffer.vbyData.cbegin(), m_messageBuffer.vbyData.cbegin() + nDataSize);
		_AddWatchEntry(EntryType::Transmit, m_messageBuffer.nID, vbyPDU);
		break;
	case PCIType::FirstFrame:
		TRACE(_T("FirstFrame.\n"));
		if (nDataSize > 0xFFF)
		{
			nDataSize = 0xFFF;
		}
		_AddWatchEntry(EntryType::Transmit, m_messageBuffer.nID, IDS_NETWORKLAYERWATCH_REQUESTFIRSTFRAME);

		--nPDUDataSize;
		byPCIFirst = byPCIFirst | nDataSize >> 8;
		vbyPDU.push_back(byPCIFirst);
		vbyPDU.push_back(nDataSize & 0xFF);
		vbyPDU.insert(vbyPDU.cend(), m_messageBuffer.vbyData.cbegin(), m_messageBuffer.vbyData.cbegin() + nPDUDataSize);
		_AddWatchEntry(EntryType::Transmit, m_messageBuffer.nID, vbyPDU);
		m_messageBuffer.stLocation = nPDUDataSize;
		m_messageBuffer.nRemainderFrameCount = 0;
		m_messageBuffer.byExpectedSequenceNumber = 1;
		break;
	case PCIType::ConsecutiveFrame:
		TRACE(_T("ConsecutiveFrame.\n"));
		_AddWatchEntry(EntryType::Transmit, m_messageBuffer.nID, IDS_NETWORKLAYERWATCH_REQUESTCONSECUTIVEFRAME);
		if (!m_messageBuffer.nRemainderFrameCount || m_messageBuffer.vbyData.size() <= m_messageBuffer.stLocation)
		{
			TRACE(_T("No more remainder frame or data, request aborted.\n"));
			_AddWatchEntry(EntryType::Transmit, m_messageBuffer.nID, IDS_NETWORKLAYERWATCH_REQUESTNOREMAINDERFRAME, Color::Red);
			return;
		}
		byPCIFirst = byPCIFirst | m_messageBuffer.byExpectedSequenceNumber;           
		m_messageBuffer.byExpectedSequenceNumber = (m_messageBuffer.byExpectedSequenceNumber + 1) % 0x10;
		vbyPDU.push_back(byPCIFirst);
		nPDUDataSize = min(nPDUDataSize, m_messageBuffer.vbyData.size() - m_messageBuffer.stLocation);
		vbyPDU.insert(vbyPDU.cend(), m_messageBuffer.vbyData.cbegin() + m_messageBuffer.stLocation, m_messageBuffer.vbyData.cbegin() + m_messageBuffer.stLocation + nPDUDataSize);
		_AddWatchEntry(EntryType::Transmit, m_messageBuffer.nID, vbyPDU);
		m_messageBuffer.stLocation += nPDUDataSize;
		--m_messageBuffer.nRemainderFrameCount;
		break;
	case PCIType::FlowControl:
		TRACE(_T("FlowControl.\n"));
		_AddWatchEntry(EntryType::Transmit, m_messageBuffer.nID, IDS_NETWORKLAYERWATCH_REQUESTFLOWCONTROL, m_byBlockSize);

		// 暂时仅发送 ContinueToSend。
		byPCIFirst = byPCIFirst | 0x0;
		vbyPDU.push_back(byPCIFirst);
		vbyPDU.push_back(m_byBlockSize);
		vbyPDU.push_back(m_bySeparationTimeMin);
		_AddWatchEntry(EntryType::Transmit, m_messageBuffer.nID, vbyPDU);
		break;
	}
	
	if (m_messageBuffer.pciType == PCIType::FlowControl)
	{
		m_messageBuffer.ResetTiming(TimingType::Ar, m_eventTiming);
	}
	else
	{
		m_messageBuffer.ResetTiming(TimingType::As, m_eventTiming);
	}

	ul.unlock();

	m_signalRequest(m_messageBuffer.nID, vbyPDU);
}

void CNetworkLayer::Confirm()
{
	TRACE(_T("CNetworkLayer::Confirm\n"));

	_AddWatchEntry(EntryType::Confirm, m_messageBuffer.nID, IDS_NETWORKLAYERWATCH_CONFIRM);

	unique_lock<recursive_mutex> ul(m_messageBuffer.rmutexMessageBuffer);

	if (!m_messageBuffer.IsRequest())
	{
		TRACE(_T("Status is incorrect.\n"));
		_AddWatchEntry(EntryType::Confirm, 0, IDS_NETWORKLAYERWATCH_CONFIRMUNEXPECTED, Color::Red);
		return;
	}

	switch (m_messageBuffer.pciType)
	{
	case PCIType::SingleFrame:
		TRACE(_T("SingleFrame.\n"));
		m_signalConfirm(Diagnostic::NetworkLayerResult::N_OK);
		m_messageBuffer.ResetTiming(TimingType::Idle, m_eventTiming);
		_AddWatchEntry(EntryType::Confirm, m_messageBuffer.nID, IDS_NETWORKLAYERWATCH_REQUESTFINISHED, Color::Green);
		m_messageBuffer.ClearMessage();
		break;
	case PCIType::FirstFrame:
		TRACE(_T("FirstFrame.\n"));
		m_messageBuffer.ResetTiming(TimingType::Bs, m_eventTiming);
		_AddWatchEntry(EntryType::Confirm, m_messageBuffer.nID, IDS_NETWORKLAYERWATCH_WAITFORFLOWCONTROL);
		m_messageBuffer.pciType = PCIType::FlowControl;
		break;
	case PCIType::ConsecutiveFrame:
		TRACE(_T("ConsecutiveFrame.\n"));
		if (m_messageBuffer.stLocation < m_messageBuffer.vbyData.size())
		{
			if (0 != m_messageBuffer.nRemainderFrameCount)		// 如果尚剩余未发送的帧，则会继续发送；
			{
				m_messageBuffer.ResetTiming(TimingType::Cs, m_eventTiming);
				TRACE(_T("Continue to request CF.\n"));
				// _AddWatchEntry(EntryType::Confirm, canID.bitField.PS, IDS_NETWORKLAYERWATCH_CONTINUETOREQUESTCF);
				if (WaitForSingleObject(m_eventStopThread.m_hObject, m_messageBuffer.bySeparationTimeMin) == WAIT_TIMEOUT)
				{
					_Request();
				}
			}
			else												// 如果没有未发送的帧，则会等待流控制帧。
			{
				m_messageBuffer.ResetTiming(TimingType::Bs, m_eventTiming);
				m_messageBuffer.pciType = PCIType::FlowControl;
				TRACE(_T("Wait for FC.\n"));
				_AddWatchEntry(EntryType::Transmit, m_messageBuffer.nID, IDS_NETWORKLAYERWATCH_WAITFORFLOWCONTROL);
			}
		}
		else
		{
			TRACE(_T("Multiframe request finished.\n"));
			_AddWatchEntry(EntryType::Confirm, m_messageBuffer.nID, IDS_NETWORKLAYERWATCH_REQUESTFINISHED, Color::Green);
			m_signalConfirm(Diagnostic::NetworkLayerResult::N_OK);
			m_messageBuffer.ClearMessage();
			//m_messageBuffer.ResetTiming(TimingType::Idle, m_eventTiming);
		}
		break;
	case PCIType::FlowControl:
		// 如果是发送的是等待帧，则应设置 Br 计时器；
		// 但程序假设永不等待，Br 实际未使用。
		TRACE(_T("FlowControl.\n"));
		m_messageBuffer.ResetTiming(TimingType::Cr, m_eventTiming);
		m_messageBuffer.nRemainderFrameCount = m_byBlockSize;
		m_messageBuffer.pciType = PCIType::ConsecutiveFrame;
		break;
	}
}

void CNetworkLayer::Indication(UINT32 nID, const BYTEVector &vbyData)
{
	TRACE(_T("CNetworkLayer::Indication.\n"));

	BYTE byPCIFirst = vbyData.at(0);
	BYTE byPCIType = (byPCIFirst & 0xF0) >> 4;
	TRACE(_T("PCIType: %d;\n"), byPCIType);

	unique_lock<recursive_mutex> ul(m_messageBuffer.rmutexMessageBuffer);

	// 对于收到非期望 N_PDU 的处理。
	// 15765-2: 6.7.3
	switch (byPCIType)
	{
	case PCIType::SingleFrame:
		TRACE(_T("SingleFrame.\n"));
		_AddWatchEntry(EntryType::Receive, nID, IDS_NETWORKLAYERWATCH_INDICATIONSINGLEFRAME);
		switch (m_messageBuffer.status)
		{
		case Status::TransmitInProgress:
			TRACE(_T("TransmitInProgress, discard received frame.\n"));
			_AddWatchEntry(EntryType::Receive, nID, IDS_NETWORKLAYERWATCH_TRANSMITINPROGRESSDISCARDRECEIVEDFRAME, Color::Red);
			return;
			break;
		case Status::ReceiveInProgress:
			TRACE(_T("ReceiveInProgress, discard previous receive.\n"));
			_AddWatchEntry(EntryType::Receive, nID, IDS_NETWORKLAYERWATCH_RECEIVEINPROGRESSDISCARDPREVIOUSRECEIVE, Color::Red);
			m_signalIndication(nID, m_messageBuffer.vbyData, Diagnostic::NetworkLayerResult::N_UNEXP_PDU);
			m_messageBuffer.ClearMessage();
			break;
		case Status::Idle:
			break;
		}
		break;
	case PCIType::FirstFrame:
		TRACE(_T("FirstFrame.\n"));
		_AddWatchEntry(EntryType::Receive, nID, IDS_NETWORKLAYERWATCH_INDICATIONFIRSTFRAME);
		switch (m_messageBuffer.status)
		{
		case Status::TransmitInProgress:
			TRACE(_T("TransmitInProgress, discard received frame.\n"));
			_AddWatchEntry(EntryType::Receive, nID, IDS_NETWORKLAYERWATCH_TRANSMITINPROGRESSDISCARDRECEIVEDFRAME, Color::Red);
			return;
			break;
		case Status::ReceiveInProgress:
			TRACE(_T("ReceiveInProgress, discard previous receive.\n"));
			_AddWatchEntry(EntryType::Receive, nID, IDS_NETWORKLAYERWATCH_RECEIVEINPROGRESSDISCARDRECEIVEDFRAME, Color::Red);
			m_signalIndication(nID, m_messageBuffer.vbyData, Diagnostic::NetworkLayerResult::N_UNEXP_PDU);
			m_messageBuffer.ClearMessage();
			break;
		case Status::Idle:
			break;
		}
		break;
	case PCIType::ConsecutiveFrame:
		TRACE(_T("ConsecutiveFrame.\n"));
		_AddWatchEntry(EntryType::Receive, nID, IDS_NETWORKLAYERWATCH_INDICATIONCONSECUTIVEFRAME);
		switch (m_messageBuffer.status)
		{
		case Status::TransmitInProgress:
			TRACE(_T("TransmitInProgress, discard received frame.\n"));
			_AddWatchEntry(EntryType::Receive, nID, IDS_NETWORKLAYERWATCH_TRANSMITINPROGRESSDISCARDRECEIVEDFRAME, Color::Red);
			return;
			break;
		case Status::ReceiveInProgress:
			if (m_messageBuffer.pciType != PCIType::ConsecutiveFrame)
			{
				TRACE(_T("Received an unexpected CF.\n"));
				_AddWatchEntry(EntryType::Receive, nID, IDS_NETWORKLAYERWATCH_INDICATIONUNEXPECTEDCFFRAME, Color::Red);
				return;
			}
			if (!m_messageBuffer.nRemainderFrameCount)
			{
				TRACE(_T("No remainder frame, discard received frame.\n"));
				_AddWatchEntry(EntryType::Receive, nID, IDS_NETWORKLAYERWATCH_RECEIVENOREMAINDERFRAME, Color::Red);
				return;
			}
			break;
		case Status::Idle:
			TRACE(_T("Idle now, discard received frame.\n"));
			_AddWatchEntry(EntryType::Receive, nID, IDS_NETWORKLAYERWATCH_IDLEDISCARDRECEIVEDFRAME, Color::Red);
			return;
			break;
		}
		break;
	case PCIType::FlowControl:
		TRACE(_T("FlowControl.\n"));
		_AddWatchEntry(EntryType::Receive, nID, IDS_NETWORKLAYERWATCH_INDICATIONFLOWCONTROL);
		switch (m_messageBuffer.status)
		{
		case Status::TransmitInProgress:
			if (m_messageBuffer.pciType != PCIType::FlowControl)
			{
				TRACE(_T("Received an unexpected FC.\n"));
				_AddWatchEntry(EntryType::Receive, nID, IDS_NETWORKLAYERWATCH_INDICATIONUNEXPECTEDFCFRAME, Color::Red);
				return;
			}
			if (m_messageBuffer.nRemainderFrameCount)
			{
				TRACE(_T("Still have reminder frame(s), discard received frame.\n"));
				_AddWatchEntry(EntryType::Receive, nID, IDS_NETWORKLAYERWATCH_REMAINDERFRAMES, Color::Red);
				return;
			}
			break;
		case Status::ReceiveInProgress:
			TRACE(_T("ReceiveInProgress, discard received frame.\n"));
			_AddWatchEntry(EntryType::Receive, nID, IDS_NETWORKLAYERWATCH_RECEIVEINPROGRESSDISCARDRECEIVEDFRAME, Color::Red);
			return;
			break;
		case Status::Idle:
			TRACE(_T("Idle now, discard received frame.\n"));
			_AddWatchEntry(EntryType::Receive, nID, IDS_NETWORKLAYERWATCH_IDLEDISCARDRECEIVEDFRAME, Color::Red);
			return;
			break;
		}
		break;
	default:
		// Unknown N_PDU
		TRACE(_T("Unknown PDU, discard received frame.\n"));
		_AddWatchEntry(EntryType::Receive, nID, IDS_NETWORKLAYERWATCH_UNKNOWNPDU, Color::Red);
		return;
	}

	_AddWatchEntry(EntryType::Receive, nID, vbyData);
	// 处理收到的 N_PDU。
	UINT nDataIndex;
	UINT nApplicationLayerDataLength;
	switch (byPCIType)
	{
	case PCIType::SingleFrame:
		{
			// 15675-2: 6.5.2
			nApplicationLayerDataLength = byPCIFirst & 0x0F;
			_AddWatchEntry(EntryType::Receive, nID, IDS_NETWORKLAYERWATCH_APPLICATIONLAYERDATALENGTH, nApplicationLayerDataLength);

			// 错误处理。
			if (!nApplicationLayerDataLength)
			{
				TRACE(_T("Length is 0, discard this frame.\n"));
				_AddWatchEntry(EntryType::Receive, nID, IDS_NETWORKLAYERWATCH_NOAPPLICATIONLAYERDATA, Color::Red);
				return;
			}
			else if (nApplicationLayerDataLength > DIAGNOSTICSFRAMEDATALENGTH)
			{
				TRACE(_T("Length is too long, discard this frame.\n"));
				_AddWatchEntry(EntryType::Receive, nID, IDS_NETWORKLAYERWATCH_APPLICATIONLAYERDATALENGTHTOOLONG, Color::Red);
				return;
			}

			// 定时操作。
			m_messageBuffer.ResetTiming(TimingType::Idle, m_eventTiming);

			nDataIndex = 1;
			m_messageBuffer.vbyData.insert(m_messageBuffer.vbyData.cend(), vbyData.cbegin() + nDataIndex, vbyData.cbegin() + nDataIndex + nApplicationLayerDataLength);
			m_signalIndication(nID, m_messageBuffer.vbyData, Diagnostic::NetworkLayerResult::N_OK);
			_AddWatchEntry(EntryType::Confirm, nID, IDS_NETWORKLAYERWATCH_RECEIVEFINISHED, Color::Green);
			break;
		}
	case PCIType::FirstFrame:
		{
			// 15765-2: 6.5.3
			nApplicationLayerDataLength = (byPCIFirst & 0x0F) << 8 | vbyData.at(1);
			TRACE(_T("Length is %d.\n"), nApplicationLayerDataLength);
			_AddWatchEntry(EntryType::Receive, nID, IDS_NETWORKLAYERWATCH_APPLICATIONLAYERDATALENGTH, nApplicationLayerDataLength);
			// 错误处理。IDS_NETWORKLAYERWATCH_APPLICATIONLAYERDATALENGTH
			if (nApplicationLayerDataLength < CANFRAMEDATALENGTHMAX)
			{
				TRACE(_T("Length is too short, discard this frame.\n"));
				_AddWatchEntry(EntryType::Receive, nID, IDS_NETWORKLAYERWATCH_APPLICATIONLAYERDATALENGTHTOOSHORT, Color::Red);
				return;
			}

			// 定时操作。
			m_messageBuffer.ResetTiming(TimingType::Br, m_eventTiming);

			nDataIndex = 2;
			// 假设不会溢出。
			m_messageBuffer.vbyData.clear();
			m_messageBuffer.vbyData.resize(nApplicationLayerDataLength);
			for (int i = nDataIndex; i != CANFRAMEDATALENGTHMAX; ++i)
			{
				m_messageBuffer.vbyData.at(i - nDataIndex) = vbyData.at(i);
			}

			m_messageBuffer.status = Status::ReceiveInProgress;
			m_messageBuffer.nID = nID;
			m_messageBuffer.stLocation = CANFRAMEDATALENGTHMAX - nDataIndex;
			m_messageBuffer.pciType = PCIType::FlowControl;
			m_messageBuffer.nRemainderFrameCount = 0;
			m_messageBuffer.byExpectedSequenceNumber = 1;
			m_signalFirstFrameIndication(nID, nApplicationLayerDataLength);

			_Request();
			break;
		}
	case PCIType::ConsecutiveFrame:
		{
			// 15765-2: 6.5.4

			// 错误处理。
			if (m_messageBuffer.byExpectedSequenceNumber != (byPCIFirst & 0x0F))
			{
				TRACE(_T("Wrong SN, discard this frame.\n"));
				_AddWatchEntry(EntryType::Receive, nID, IDS_NETWORKLAYERWATCH_INVALIDSN, Color::Red);
				m_signalIndication(nID, m_messageBuffer.vbyData, Diagnostic::NetworkLayerResult::N_WRONG_SN);
				return;
			}
			m_messageBuffer.byExpectedSequenceNumber = (m_messageBuffer.byExpectedSequenceNumber + 1) % 0x10;

			nDataIndex = 1;
			UINT nReceiveLength = min(7, m_messageBuffer.vbyData.size() - m_messageBuffer.stLocation);
			for (int i = 0; i != nReceiveLength; ++i)
			{
				m_messageBuffer.vbyData.at(m_messageBuffer.stLocation + i) = vbyData.at(i);
			}
			m_messageBuffer.stLocation += nReceiveLength;
			--m_messageBuffer.nRemainderFrameCount;
			if (m_messageBuffer.stLocation >= m_messageBuffer.vbyData.size())
			{
				TRACE(_T("Multiframe receive finished.\n"));
				_AddWatchEntry(EntryType::Receive, nID, IDS_NETWORKLAYERWATCH_RECEIVEFINISHED, Color::Green);
				BYTEVector vbyData = m_messageBuffer.vbyData;
				m_messageBuffer.ClearMessage();
				m_signalIndication(nID, m_messageBuffer.vbyData, Diagnostic::NetworkLayerResult::N_OK);
			}
			else
			{
				m_messageBuffer.ResetTiming(TimingType::Cr, m_eventTiming);
				if (!m_messageBuffer.nRemainderFrameCount)
				{
					m_messageBuffer.pciType = PCIType::FlowControl;
					_Request();
				}
			}
			break;
		}
	case PCIType::FlowControl:
		{
			// 15765-2: 6.5.5
			BYTE byFlowControlType = byPCIFirst & 0x0F;
			// 错误处理与流控制类型判断。
			switch(byFlowControlType)
			{
			case FlowControlType::ContinueToSend:
				{
					TRACE(_T("FlowControl: ContinueToSend.\n"));
					m_messageBuffer.ResetTiming(TimingType::Cs, m_eventTiming);
					m_messageBuffer.pciType = PCIType::ConsecutiveFrame;
					m_messageBuffer.bySeparationTimeMin = vbyData.at(2);
					if (m_messageBuffer.bySeparationTimeMin >= 0x80 && m_messageBuffer.bySeparationTimeMin <= 0xF0 || m_messageBuffer.bySeparationTimeMin >= 0xFA)
					{
						m_messageBuffer.bySeparationTimeMin = 0x7F;
					}
					m_messageBuffer.nRemainderFrameCount = vbyData.at(1);
					_AddWatchEntry(EntryType::Receive, nID, IDS_NETWORKLAYERWATCH_FLOWCONTROL_CONTINUETOSEND, m_messageBuffer.nRemainderFrameCount);
					
					// 15765-2: 6.5.5.4, Table 14
					if (!m_messageBuffer.nRemainderFrameCount)
					{
						m_messageBuffer.nRemainderFrameCount = -1;	// 取最大值
					}

					DWORD dwWaitResult = WaitForSingleObject(m_eventStopThread.m_hObject, m_messageBuffer.bySeparationTimeMin);
					if (dwWaitResult == WAIT_TIMEOUT)
					{
						_Request();
					}
					break;
				}
			case FlowControlType::Wait:
				TRACE(_T("FlowControl: Wait.\n"));
				_AddWatchEntry(EntryType::Receive, nID, IDS_NETWORKLAYERWATCH_FLOWCONTROL_WAIT);
				m_messageBuffer.ResetTiming(TimingType::Bs, m_eventTiming);
				return;
				break;
			case FlowControlType::Overflow:
				TRACE(_T("FlowControl: Overflow.\n"));
				_AddWatchEntry(EntryType::Receive, nID, IDS_NETWORKLAYERWATCH_FLOWCONTROL_OVERFLOW, Color::Red);
				m_messageBuffer.ClearMessage();
				m_signalConfirm(Diagnostic::NetworkLayerResult::N_BUFFER_OVFLW);
				return;
			default:
				TRACE(_T("FlowControl: Invalid flowcontrol.\n"));
				_AddWatchEntry(EntryType::Receive, nID, IDS_NETWORKLAYERWATCH_FLOWCONTROL_INVALID, Color::Red);
				m_messageBuffer.ResetTiming(TimingType::Idle, m_eventTiming);
				m_messageBuffer.ClearMessage();
				m_signalConfirm(Diagnostic::NetworkLayerResult::N_INVALID_FS);
				return;
			}
			break;
		}
	}
	return;
}

void CNetworkLayer::SetDiagnosticControl(CDiagnosticControl &diagnosticControl)
{
	m_pDiagnosticControl = &diagnosticControl;
}

boost::signals2::connection CNetworkLayer::ConnectIndication(const Diagnostic::IndicationASignal::slot_type &subscriber)
{
	return m_signalIndication.connect(subscriber);
}

boost::signals2::connection CNetworkLayer::ConnectFirstFrameIndication(const Diagnostic::IndicationAFSignal::slot_type &subscriber)
{
	return m_signalFirstFrameIndication.connect(subscriber);
}

boost::signals2::connection CNetworkLayer::ConnectConfirm(const Diagnostic::ConfirmASignal::slot_type &subscriber)
{
	return m_signalConfirm.connect(subscriber);
}

boost::signals2::connection CNetworkLayer::ConnectRequest(const Diagnostic::RequestSignal::slot_type &subscriber)
{
	return m_signalRequest.connect(subscriber);
}

UINT CNetworkLayer::_TimingThread(LPVOID lpParam)
{
	CNetworkLayer *pThis = static_cast<CNetworkLayer *>(lpParam);
	HANDLE hEvents[] = { pThis->m_eventStopThread, pThis->m_eventTiming };
	DWORD dwWaitResult = -1;
	DWORD dwTimingTickSpan;	// 为兼容 XP，存在几率极小的 49.7 天归零问题。
	BOOL bTimeout;
	BOOL bAbort;
	while (dwWaitResult != WAIT_OBJECT_0)
	{
		dwWaitResult = WaitForMultipleObjects(sizeof(hEvents) / sizeof(HANDLE), hEvents, FALSE, INFINITE);
		if (dwWaitResult == WAIT_OBJECT_0 + 1)
		{
			WaitForSingleObject(pThis->m_eventStopThread.m_hObject, TIMINGCYCLE);
			unique_lock<recursive_mutex> ul(pThis->m_messageBuffer.rmutexMessageBuffer, std::try_to_lock);
			if (ul.owns_lock())
			{
				if (!pThis->m_messageBuffer.IsBusy() || pThis->m_messageBuffer.timingType == TimingType::Idle)
				{
					pThis->m_eventTiming.ResetEvent();
					ul.unlock();
					continue;
				}
				dwTimingTickSpan = GetTickCount() - pThis->m_messageBuffer.dwTimingStartTick;
				bTimeout = pThis->m_anTimingParameters[static_cast<UINT>(pThis->m_messageBuffer.timingType)] < dwTimingTickSpan;
				if (bTimeout)
				{
					bAbort = TRUE;
					// 15765-2: 6.7.2
					switch (pThis->m_messageBuffer.timingType)
					{
					case TimingType::As:
						TRACE(_T("\nNetworkLayer.Timeout.As\n"));
						pThis->m_signalConfirm(Diagnostic::NetworkLayerResult::N_TIMEOUT_A);
						pThis->_AddWatchEntry(EntryType::Transmit, pThis->m_messageBuffer.nID, IDS_NETWORKLAYERWATCH_TIMEOUT_AS, Color::Red);
						break;
					case TimingType::Ar:
						TRACE(_T("\nNetworkLayer.Timeout.Ar\n"));
						pThis->m_signalIndication(pThis->m_messageBuffer.nID, pThis->m_messageBuffer.vbyData, Diagnostic::NetworkLayerResult::N_TIMEOUT_A);
						pThis->_AddWatchEntry(EntryType::Receive, pThis->m_messageBuffer.nID, IDS_NETWORKLAYERWATCH_TIMEOUT_AR, Color::Red);
						break;
					case TimingType::Bs:
						TRACE(_T("\nNetworkLayer.Timeout.Bs\n"));
						pThis->m_signalConfirm(Diagnostic::NetworkLayerResult::N_TIMEOUT_Bs);
						pThis->_AddWatchEntry(EntryType::Transmit, pThis->m_messageBuffer.nID, IDS_NETWORKLAYERWATCH_TIMEOUT_BS, Color::Red);
						break;
					case TimingType::Br:
						TRACE(_T("\nNetworkLayer.Timeout.Br\n"));
						pThis->m_messageBuffer.ResetTiming(TimingType::Br, pThis->m_eventTiming);
						bAbort = FALSE;
						pThis->_AddWatchEntry(EntryType::Receive, pThis->m_messageBuffer.nID, IDS_NETWORKLAYERWATCH_TIMEOUT_BR, Color::Red);
						// 流量控制帧发送过慢，应避免。
						break;
					case TimingType::Cs:
						TRACE(_T("\nNetworkLayer.Timeout.Cs\n"));
						pThis->m_messageBuffer.ResetTiming(TimingType::Cs, pThis->m_eventTiming);
						bAbort = FALSE;
						pThis->_AddWatchEntry(EntryType::Transmit, pThis->m_messageBuffer.nID, IDS_NETWORKLAYERWATCH_TIMEOUT_CS, Color::Red);
						// 连续帧发送过慢，应避免。
						break;
					case TimingType::Cr:
						TRACE(_T("\nNetworkLayer.Timeout.Cr\n"));
						pThis->m_signalIndication(pThis->m_messageBuffer.nID, pThis->m_messageBuffer.vbyData, Diagnostic::NetworkLayerResult::N_TIMEOUT_Cr);
						pThis->_AddWatchEntry(EntryType::Receive, pThis->m_messageBuffer.nID, IDS_NETWORKLAYERWATCH_TIMEOUT_CR, Color::Red);
						break;
					}
					if (bAbort)
					{
						pThis->m_messageBuffer.ClearMessage();
					}
				}
			}
		}
	}
	pThis->m_pTimingThread = NULL;
	pThis->m_eventTimingThreadExited.SetEvent();
	return 0;
}

void CNetworkLayer::_AddWatchEntry(EntryType entryType, UINT32 nID, UINT nDescriptionID, Color color)
{
	if (NULL != m_pDiagnosticControl)
	{
		m_pDiagnosticControl->AddWatchEntry(CDiagnosticControl::LayerType::NetworkLayer, entryType, nID, nDescriptionID, color);
	}
}

void CNetworkLayer::_AddWatchEntry(EntryType entryType, UINT32 nID, const BYTEVector &vbyData, Color color)
{
	if (NULL != m_pDiagnosticControl)
	{
		m_pDiagnosticControl->AddWatchEntry(CDiagnosticControl::LayerType::NetworkLayer, entryType, nID, vbyData, color);
	}
}

void CNetworkLayer::_AddWatchEntry(EntryType entryType, UINT32 nID, UINT nDescriptionID, int nData, Color color)
{
	if (NULL != m_pDiagnosticControl)
	{
		m_pDiagnosticControl->AddWatchEntry(CDiagnosticControl::LayerType::NetworkLayer, entryType, nID, nDescriptionID, nData, color);
	}
}

void CNetworkLayer::_StartThread()
{
	m_pTimingThread = AfxBeginThread(_TimingThread, this, THREAD_PRIORITY_ABOVE_NORMAL, 0U, CREATE_SUSPENDED);
	m_pTimingThread->m_bAutoDelete = TRUE;
	m_pTimingThread->ResumeThread();
}

void CNetworkLayer::_StopThread()
{
	if (NULL != m_pTimingThread)
	{
		m_eventStopThread.SetEvent();

		UINT nUnexitedThreadsCount = 1;
		HANDLE *phThreadsExited = new HANDLE[nUnexitedThreadsCount];
		phThreadsExited[0] = m_eventTimingThreadExited.m_hObject;
		DWORD dwWaitResult = -1;
		MSG msg;
		while (nUnexitedThreadsCount)
		{
			dwWaitResult = MsgWaitForMultipleObjects(nUnexitedThreadsCount, phThreadsExited, FALSE, INFINITE, QS_ALLINPUT);	// 等待所有线程退出。
			switch (dwWaitResult)
			{
			case WAIT_OBJECT_0:
				--nUnexitedThreadsCount;
				break;
			case WAIT_OBJECT_0 + 1:
				PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);  
				DispatchMessage(&msg);
				break;
			}
		}
		delete [] phThreadsExited;
		m_messageBuffer.ClearMessage();
	}
}
