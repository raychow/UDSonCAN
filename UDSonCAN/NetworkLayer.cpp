#include "stdafx.h"
#include "NetworkLayer.h"

#include <algorithm>

#include "ApplicationLayer.h"
#include "DataLinkLayer.h"
#include "DiagnosticControl.h"
#include "resource.h"

CNetworkLayer::Message::Message()
	: status(Status::Idle)
	, stLocation(0)
	, messageType(MessageType::Unknown)
	, pciType(PCIType::Unknown)
	, bySeparationTimeMin(0)
	, nRemainderFrameCount(0)
	, byExpectedSequenceNumber(0)
	, dwTimingStartTick(0)
	, timingType(TimingType::Idle)
{
	address.nUnionAddress = 0;
}

CNetworkLayer::Message::Compare::Compare(UINT nAddress)
{
	m_nAddress = nAddress;
}

BOOL CNetworkLayer::Message::Compare::operator()(const Message *pMessage) const
{
	return pMessage->address.nUnionAddress == m_nAddress;
}

void CNetworkLayer::Message::ResetTiming(TimingType timingType, CEvent &eventTiming)
{
	TRACE(_T("CNetworkLayer::ResetTiming 0x%X: %d\n"), address.nUnionAddress, timingType);
	
	CSingleLock lockProcess(&csectionProcess);

	lockProcess.Lock();
	this->timingType = timingType;
	dwTimingStartTick = GetTickCount();
	lockProcess.Unlock();

	if (timingType != TimingType::Idle)
	{
		eventTiming.SetEvent();
	}
}

void CNetworkLayer::Message::AbortMessage()
{
	status = Status::Idle;
	vbyData.clear();
	stLocation = 0;
	messageType = MessageType::Unknown;
	pciType = PCIType::Unknown;
	bySeparationTimeMin = 0;
	nRemainderFrameCount = 0;
	byExpectedSequenceNumber = 0;
	dwTimingStartTick = 0;
	timingType = TimingType::Idle;
}

CNetworkLayer::CNetworkLayer(void)
	: m_bFullDuplex(FALSE)
	, m_bySeparationTimeMin(0)
	, m_byBlockSize(0xFF)
	, m_nWaitFrameTransimissionMax(0)
	, m_pApplicationLayer(NULL)
	, m_pDataLinkLayer(NULL)
	, m_pTimingThread(NULL)
	, m_eventTiming(FALSE, TRUE)
	, m_eventStopThread(FALSE, TRUE)
{
	ZeroMemory(m_anTimingParameters, sizeof(m_anTimingParameters));
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
//
//CNetworkLayer::Status CNetworkLayer::GetStatus() const
//{
//	return m_status;
//}

BOOL CNetworkLayer::IsFullDuplex() const
{
	return m_bFullDuplex;
}

void CNetworkLayer::SetFullDuplex(BOOL bFullDuplex)
{
	m_bFullDuplex = bFullDuplex;
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

BOOL CNetworkLayer::Request(MessageType messageType, BYTE bySourceAddress, BYTE byTargetAddress, TargetAddressType targetAddressType, BYTE byAddressExtension, const BYTEVector &vbyData)
{
	if (messageType == MessageType::Diagnostics && vbyData.size() < DIAGNOSTICSFRAMEDATALENGTH || messageType == MessageType::RemoteDiagnostics && vbyData.size() < REMOTEDIAGNOSTICSFRAMEDATALENGTH)
	{
		return _Request(PCIType::SingleFrame, messageType, bySourceAddress, byTargetAddress, targetAddressType, byAddressExtension, vbyData);
	}
	else
	{
		return _Request(PCIType::FirstFrame, messageType, bySourceAddress, byTargetAddress, targetAddressType, byAddressExtension, vbyData);
	}
}

CNetworkLayer::Message *CNetworkLayer::FindMessage(BOOL bAddIfNotFound, BYTE bySourceAddress, BYTE byTargetAddress, TargetAddressType targetAddressType, BYTE byAddressExtension, BOOL bTesterPresent)
{
#ifdef NETWORKLAYER_NOAE
	byAddressExtension = 0;
#endif

	Message::Address address;
	address.bitField.bySourceAddress = bySourceAddress;
	address.bitField.byTargetAddress = byTargetAddress;
	address.bitField.targetAddressType = targetAddressType;
	address.bitField.bTesterPresent = bTesterPresent;
	address.bitField.byAddressExtension = byAddressExtension;

	CSingleLock lockMessageList(&m_csectionMessageList);
	lockMessageList.Lock();
	PMessageList::const_iterator iterMessage = std::find_if(m_lpMessage.cbegin(), m_lpMessage.cend(), Message::Compare(address.nUnionAddress));
	if (iterMessage == m_lpMessage.cend())
	{
		if (bAddIfNotFound)
		{
			m_lpMessage.push_back(new Message());
			Message *pMessage = m_lpMessage.back();
			pMessage->address.nUnionAddress = address.nUnionAddress;
			return pMessage;
		}
		else
		{
			return NULL;
		}
	}
	return *iterMessage;
}

CNetworkLayer::MessageType CNetworkLayer::GetMessageType(BYTE byPF) const
{
	if (byPF == static_cast<BYTE>(J1939ParameterGroupNumber::NormalFixedAddressingPhysical) || byPF == static_cast<BYTE>(J1939ParameterGroupNumber::NormalFixedAddressingFunctional))
	{
		return MessageType::Diagnostics;
	}
	else if (byPF == static_cast<BYTE>(J1939ParameterGroupNumber::MixedAddressingPhysical) || byPF ==static_cast<BYTE>( J1939ParameterGroupNumber::MixedAddressingFunctional))
	{
		return MessageType::RemoteDiagnostics;
	}
	else
	{
		return MessageType::Unknown;
	}
}

CNetworkLayer::TargetAddressType CNetworkLayer::GetTargetAddressType(BYTE byPF) const
{
	if (byPF == static_cast<BYTE>(J1939ParameterGroupNumber::NormalFixedAddressingPhysical) || byPF == static_cast<BYTE>(J1939ParameterGroupNumber::MixedAddressingPhysical))
	{
		return TargetAddressType::Physical;
	}
	else if (byPF == static_cast<BYTE>(J1939ParameterGroupNumber::NormalFixedAddressingFunctional) || byPF == static_cast<BYTE>(J1939ParameterGroupNumber::MixedAddressingFunctional))
	{
		return TargetAddressType::Functional;
	}
	else
	{
		return TargetAddressType::Unknown;
	}
}

BOOL CNetworkLayer::_Request(PCIType pciType, MessageType messageType, BYTE bySourceAddress, BYTE byTargetAddress, TargetAddressType targetAddressType, BYTE byAddressExtension, const BYTEVector &vbyData)
{
	ASSERT(m_pDataLinkLayer);

	TRACE(_T("\nNetworkLayer::_Request.\n"));
	// _AddWatchEntry(EntryType::Transmit, byTargetAddress, IDS_NETWORKLAYERWATCH_REQUEST);

	if (messageType == MessageType::Diagnostics)
	{
		byAddressExtension = 0;
	}
	else
	{
		if (pciType != PCIType::SingleFrame)
		{
			TRACE(_T("A functionally addressed request message shall only be a single-frame message. Request aborted.\n"));
			_AddWatchEntry(EntryType::Transmit, byTargetAddress, IDS_NETWORKLAYERWATCH_REQUESTFUNCTIONALADDRESSISNOTSINGLEFRAME, Color::Red);
			return FALSE;
		}
	}

#ifdef NETWORKLAYER_NOAE
	byAddressExtension = 0;
#endif

	Message *pMessage;
	if (pciType == PCIType::FlowControl)
	{
		pMessage = FindMessage(
			FALSE,
			byTargetAddress,
			bySourceAddress,
			targetAddressType,
			byAddressExtension);
	}
	else
	{
		pMessage = FindMessage(
			pciType != PCIType::ConsecutiveFrame,
			bySourceAddress,
			byTargetAddress,
			targetAddressType,
			byAddressExtension,
			vbyData.empty() ? FALSE : vbyData.at(messageType == MessageType::RemoteDiagnostics) == TESETRPRESENTSERVICEID);
	}

	if (!pMessage)
	{
		TRACE(_T("Requested an unexpected CF or FC frame, request aborted.\n"));
		_AddWatchEntry(EntryType::Transmit, byTargetAddress, IDS_NETWORKLAYERWATCH_REQUESTNOTEXPECTED, Color::Red);
		return FALSE;
	}
	// ID
	UINT nID = J1939IDTEMPLET;
	UINT nPDUFormat = 0;
	switch (messageType)
	{
	case MessageType::Diagnostics:
		switch (targetAddressType)
		{
		case TargetAddressType::Physical:
			nPDUFormat = static_cast<BYTE>(J1939ParameterGroupNumber::NormalFixedAddressingPhysical);
			break;
		case TargetAddressType::Functional:
			nPDUFormat = static_cast<BYTE>(J1939ParameterGroupNumber::NormalFixedAddressingFunctional);
			break;
		default:
			TRACE(_T("Unknown TargetAddressType.\n"));
			_AddWatchEntry(EntryType::Transmit, byTargetAddress, IDS_NETWORKLAYERWATCH_UNKNOWNTARGETADDRESSTYPE, Color::Red);
			return FALSE;
		}
		break;
	case MessageType::RemoteDiagnostics:
		switch (targetAddressType)
		{
		case TargetAddressType::Physical:
			nPDUFormat = static_cast<BYTE>(J1939ParameterGroupNumber::MixedAddressingPhysical);
			break;
		case TargetAddressType::Functional:
			nPDUFormat = static_cast<BYTE>(J1939ParameterGroupNumber::MixedAddressingFunctional);
			break;
		default:
			TRACE(_T("Unknown TargetAddressType.\n"));
			_AddWatchEntry(EntryType::Transmit, byTargetAddress, IDS_NETWORKLAYERWATCH_UNKNOWNTARGETADDRESSTYPE, Color::Red);
			return FALSE;
		}
		break;
	default:
		TRACE(_T("Unknown MessageType.\n"));
		_AddWatchEntry(EntryType::Transmit, byTargetAddress, IDS_NETWORKLAYERWATCH_UNKNOWNMESSAGETYPE, Color::Red);
		return FALSE;
	}
	nPDUFormat = nPDUFormat << 16;
	UINT nTargetAddress = byTargetAddress;
	nTargetAddress = nTargetAddress << 8;
	nID = nID | nPDUFormat | nTargetAddress | bySourceAddress;

	CSingleLock lockProcess(&pMessage->csectionProcess);
	lockProcess.Lock();

	// Data
	BYTEVector vbyPDU;
	vbyPDU.reserve(CANFRAMEDATALENGTHMAX);
	if (messageType == MessageType::RemoteDiagnostics)
	{
		vbyPDU.push_back(byAddressExtension);
	}
	BYTE byPCIFirst;
	byPCIFirst = static_cast<BYTE>(pciType) << 4;
	UINT nDataSize = vbyData.size();
	UINT nPDUDataSize;	// PDU 包含的数据长度，在首帧中需要减一，流控制帧中不包含数据帧。
	if (messageType == MessageType::Diagnostics)
	{
		nPDUDataSize = DIAGNOSTICSFRAMEDATALENGTH;
	}
	else
	{
		nPDUDataSize = REMOTEDIAGNOSTICSFRAMEDATALENGTH;
	}
	switch (pciType)
	{
	case PCIType::SingleFrame:
		TRACE(_T("SingleFrame.\n"));
		_AddWatchEntry(EntryType::Transmit, byTargetAddress, IDS_NETWORKLAYERWATCH_REQUESTSINGLEFRAME);
		pMessage->AbortMessage();

		if (nDataSize > nPDUDataSize)
		{
			nDataSize = nPDUDataSize;
		}
		byPCIFirst = byPCIFirst | nDataSize;
		vbyPDU.push_back(byPCIFirst);
		vbyPDU.insert(vbyPDU.cend(), vbyData.cbegin(), vbyData.cbegin() + nDataSize);
		_AddWatchEntry(EntryType::Transmit, byTargetAddress, vbyPDU);
		pMessage->pciType = pciType;
		break;
	case PCIType::FirstFrame:
		TRACE(_T("FirstFrame.\n"));
		if (nDataSize > 0xFFF)
		{
			nDataSize = 0xFFF;
		}
		_AddWatchEntry(EntryType::Transmit, byTargetAddress, IDS_NETWORKLAYERWATCH_REQUESTFIRSTFRAME);

		pMessage->AbortMessage();

		--nPDUDataSize;
		byPCIFirst = byPCIFirst | nDataSize >> 8;
		vbyPDU.push_back(byPCIFirst);
		vbyPDU.push_back(nDataSize & 0xFF);
		vbyPDU.insert(vbyPDU.cend(), vbyData.cbegin(), vbyData.cbegin() + nPDUDataSize);
		_AddWatchEntry(EntryType::Transmit, byTargetAddress, vbyPDU);
		pMessage->status = Status::TransmitInProgress;
		pMessage->vbyData.insert(pMessage->vbyData.cend(), vbyData.cbegin(), vbyData.cbegin() + nDataSize);
		pMessage->stLocation = nPDUDataSize;
		pMessage->nRemainderFrameCount = 0;
		pMessage->byExpectedSequenceNumber = 1;
		break;
	case PCIType::ConsecutiveFrame:
		{
			TRACE(_T("ConsecutiveFrame.\n"));
			_AddWatchEntry(EntryType::Transmit, byTargetAddress, IDS_NETWORKLAYERWATCH_REQUESTCONSECUTIVEFRAME);
			if (pMessage->status != Status::TransmitInProgress)
			{
				TRACE(_T("Status is not TransmitInProgress, request aborted.\n"));
				_AddWatchEntry(EntryType::Transmit, byTargetAddress, IDS_NETWORKLAYERWATCH_NOTTRANSMITINPROGRESS, Color::Red);
				return FALSE;
			}
			if (!pMessage->nRemainderFrameCount)
			{
				TRACE(_T("No more remainder frame, request aborted.\n"));
				_AddWatchEntry(EntryType::Transmit, byTargetAddress, IDS_NETWORKLAYERWATCH_REQUESTNOREMAINDERFRAME, Color::Red);
				return FALSE;
			}
			byPCIFirst = byPCIFirst | pMessage->byExpectedSequenceNumber;           
			pMessage->byExpectedSequenceNumber = (pMessage->byExpectedSequenceNumber + 1) % 0x10;
			vbyPDU.push_back(byPCIFirst);
			nPDUDataSize = min(nPDUDataSize, pMessage->vbyData.size() - pMessage->stLocation);
			vbyPDU.insert(vbyPDU.cend(), pMessage->vbyData.cbegin() + pMessage->stLocation, pMessage->vbyData.cbegin() + pMessage->stLocation + nPDUDataSize);
			_AddWatchEntry(EntryType::Transmit, byTargetAddress, vbyPDU);
			pMessage->stLocation += nPDUDataSize;
			--pMessage->nRemainderFrameCount;
			if (pMessage->vbyData.size() <= pMessage->stLocation)
			{
				pMessage->status = Status::Idle;
			}
			break;
		}
	case PCIType::FlowControl:
		TRACE(_T("FlowControl.\n"));
		_AddWatchEntry(EntryType::Transmit, byTargetAddress, IDS_NETWORKLAYERWATCH_REQUESTFLOWCONTROL, m_byBlockSize);

		if (pMessage->status != Status::ReceiveInProgress)
		{
			TRACE(_T("Status is not ReceiveInProgress, request aborted.\n"));
			_AddWatchEntry(EntryType::Transmit, byTargetAddress, IDS_NETWORKLAYERWATCH_NOTRECEIVEINPROGRESS, Color::Red);
			return FALSE;
		}

		pMessage->nRemainderFrameCount = m_byBlockSize;
		// 暂时仅发送 ContinueToSend。
		byPCIFirst = byPCIFirst | 0x0;
		vbyPDU.push_back(byPCIFirst);
		vbyPDU.push_back(m_byBlockSize);
		vbyPDU.push_back(m_bySeparationTimeMin);
		_AddWatchEntry(EntryType::Transmit, byTargetAddress, vbyPDU);
		break;
	}
	pMessage->messageType = messageType;
	pMessage->pciType = pciType;
	
	BOOL bFlowControl = pciType == PCIType::FlowControl;
	if (bFlowControl)
	{
		pMessage->ResetTiming(TimingType::Ar, m_eventTiming);
	}
	else
	{
		pMessage->ResetTiming(TimingType::As, m_eventTiming);
	}

	lockProcess.Unlock();

	return m_pDataLinkLayer->Request(nID, vbyPDU, bFlowControl);
}

void CNetworkLayer::Confirm(UINT nID, BYTE byAddressExtension, BOOL bReverseAddress)
{
	ASSERT(m_pApplicationLayer);
	CDataLinkLayer::CANID canID;
	canID.nID = nID;

	if (GetMessageType(canID.bitField.PF) == MessageType::Diagnostics)
	{
		byAddressExtension = 0;
	}

#ifdef NETWORKLAYER_NOAE
	byAddressExtension = 0;
#endif

	TRACE(_T("CNetworkLayer::Confirm\n"));

	if (bReverseAddress)
	{
		BYTE byTemp = canID.bitField.SA;
		canID.bitField.SA = canID.bitField.PS;
		canID.bitField.PS = byTemp;
	}

	_AddWatchEntry(EntryType::Confirm, canID.bitField.PS, IDS_NETWORKLAYERWATCH_CONFIRM);

	Message *pMessage = FindMessage(FALSE, canID.bitField.SA, canID.bitField.PS, GetTargetAddressType(canID.bitField.PF), byAddressExtension);
	if (!pMessage)
	{
		TRACE(_T("Can not confirm a frame not in message list.\n"));
		_AddWatchEntry(EntryType::Confirm, canID.bitField.PS, IDS_NETWORKLAYERWATCH_CONFIRMNOTINMESSAGELIST, Color::Red);
		return;
	}

	CSingleLock lockProcess(&m_csectionProcess);
	lockProcess.Lock();
	
	switch (pMessage->pciType)
	{
	case PCIType::SingleFrame:
		TRACE(_T("SingleFrame.\n"));
		m_pApplicationLayer->Confirm(
			pMessage->messageType,
			pMessage->address.bitField.bySourceAddress,
			pMessage->address.bitField.byTargetAddress,
			pMessage->address.bitField.targetAddressType,
			byAddressExtension,
			Result::N_OK);
		pMessage->ResetTiming(TimingType::Idle, m_eventTiming);
		_AddWatchEntry(EntryType::Confirm, canID.bitField.PS, IDS_NETWORKLAYERWATCH_REQUESTFINISHED, Color::Green);
		break;
	case PCIType::FirstFrame:
		TRACE(_T("FirstFrame.\n"));
		pMessage->ResetTiming(TimingType::Bs, m_eventTiming);
		_AddWatchEntry(EntryType::Confirm, canID.bitField.PS, IDS_NETWORKLAYERWATCH_WAITFORFLOWCONTROL);
		break;
	case PCIType::ConsecutiveFrame:
		{
			TRACE(_T("ConsecutiveFrame.\n"));
			if (Status::TransmitInProgress == pMessage->status)
			{
				if (0 != pMessage->nRemainderFrameCount)				// 如果尚剩余未发送的帧，则会继续发送；
				{
					pMessage->ResetTiming(TimingType::Cs, m_eventTiming);
					TRACE(_T("Continue to request CF.\n"));
					// _AddWatchEntry(EntryType::Confirm, canID.bitField.PS, IDS_NETWORKLAYERWATCH_CONTINUETOREQUESTCF);
					if (WaitForSingleObject(m_eventStopThread.m_hObject, pMessage->bySeparationTimeMin) == WAIT_TIMEOUT)
					{
						_Request(
							PCIType::ConsecutiveFrame,
							pMessage->messageType,
							pMessage->address.bitField.bySourceAddress,
							pMessage->address.bitField.byTargetAddress,
							pMessage->address.bitField.targetAddressType,
							byAddressExtension,
							pMessage->vbyData);
					}
				}
				else											// 如果没有未发送的帧，则会等待流控制帧。
				{
					pMessage->ResetTiming(TimingType::Bs, m_eventTiming);
					TRACE(_T("Wait for FC.\n"));
					_AddWatchEntry(EntryType::Transmit, canID.bitField.PS, IDS_NETWORKLAYERWATCH_WAITFORFLOWCONTROL);
				}
			}
			else if (Status::Idle == pMessage->status)
			{
				if (pMessage->vbyData.size() <= pMessage->stLocation)	// 如果多帧已经发送完毕。
				{
					TRACE(_T("Multiframe request finished.\n"));
					_AddWatchEntry(EntryType::Confirm, canID.bitField.PS, IDS_NETWORKLAYERWATCH_REQUESTFINISHED, Color::Green);
					m_pApplicationLayer->Confirm(
						pMessage->messageType,
						pMessage->address.bitField.bySourceAddress,
						pMessage->address.bitField.byTargetAddress,
						pMessage->address.bitField.targetAddressType,
						byAddressExtension,
						Result::N_OK);
					pMessage->ResetTiming(TimingType::Idle, m_eventTiming);
				}
			}
		}
		break;
	case PCIType::FlowControl:
		// 如果是发送的是等待帧，则应设置 Br 计时器；
		// 但程序假设永不等待，Br 实际未使用。
		TRACE(_T("FlowControl.\n"));
		pMessage->ResetTiming(TimingType::Cr, m_eventTiming);
		break;
	}
}

void CNetworkLayer::Indication(UINT nID, const BYTEVector &vbyData)
{
	ASSERT(m_pApplicationLayer);
	TRACE(_T("CNetworkLayer::Indication: 0x%X\n"), nID);

	CDataLinkLayer::CANID canID;
	canID.nID = nID;

	MessageType messageType = GetMessageType(canID.bitField.PF);
	TargetAddressType targetAddressType = GetTargetAddressType(canID.bitField.PF);
	if (messageType == MessageType::Unknown || targetAddressType == TargetAddressType::Unknown)
	{
		TRACE(_T("Unknown CANID.PF"));
		_AddWatchEntry(EntryType::Receive, canID.bitField.SA, IDS_NETWORKLAYERWATCH_UNKNOWNCANID, Color::Red);
		return;
	}

	UINT nPCIIndex = messageType == MessageType::RemoteDiagnostics;
	BYTE byPCIFirst = vbyData.at(nPCIIndex);
	BYTE byPCIType = (byPCIFirst & 0xF0) >> 4;
	TRACE(_T("MessageType: %d, TargetAddressType: %d, PCIType: %d;\n"), messageType, targetAddressType, byPCIType);

	BYTE byAddressExtension = 0;
	if (messageType == MessageType::RemoteDiagnostics)
	{
		if (byPCIType != static_cast<BYTE>(PCIType::SingleFrame))
		{
			TRACE(_T("A functionally addressed request message shall only be a single-frame message. Indication aborted.\n"));
			_AddWatchEntry(EntryType::Receive, canID.bitField.SA, IDS_NETWORKLAYERWATCH_INDICATIONFUNCTIONALADDRESSISNOTSINGLEFRAME, Color::Red);
			return;
		}
		byAddressExtension = vbyData.at(1);
	}

#ifdef NETWORKLAYER_NOAE
	byAddressExtension = 0;
#endif

	Message *pMessage = NULL;
	
	if (static_cast<BYTE>(PCIType::FlowControl) == byPCIType)
	{
		if (!(pMessage = FindMessage(FALSE, canID.bitField.PS, canID.bitField.SA, targetAddressType, byAddressExtension)))
		{
			TRACE(_T("Received an unexpected FC Frame.\n"));
			_AddWatchEntry(EntryType::Receive, canID.bitField.SA, IDS_NETWORKLAYERWATCH_INDICATIONUNEXPECTEDFCFRAME, Color::Red);
			return;
		}
	}
	else if (static_cast<BYTE>(PCIType::ConsecutiveFrame) == byPCIType)
	{
		if (!(pMessage = FindMessage(FALSE, canID.bitField.SA, canID.bitField.PS, targetAddressType, byAddressExtension)))
		{
			TRACE(_T("Received an unexpected CF Frame.\n"));
			_AddWatchEntry(EntryType::Receive, canID.bitField.SA, IDS_NETWORKLAYERWATCH_INDICATIONUNEXPECTEDCFFRAME, Color::Red);
			return;
		}
	}
	else
	{
		pMessage = FindMessage(TRUE, canID.bitField.SA, canID.bitField.PS, targetAddressType, byAddressExtension);
	}
	
	CSingleLock lockProcess(&pMessage->csectionProcess);
	lockProcess.Lock();

	// 对于收到非期望 N_PDU 的处理。
	// 15765-2: 6.7.3
	switch (byPCIType)
	{
	case PCIType::SingleFrame:
		TRACE(_T("SingleFrame.\n"));
		_AddWatchEntry(EntryType::Receive, canID.bitField.SA, IDS_NETWORKLAYERWATCH_INDICATIONSINGLEFRAME);
		switch (pMessage->status)
		{
		case Status::TransmitInProgress:
			TRACE(_T("TransmitInProgress, discard received frame.\n"));
			_AddWatchEntry(EntryType::Receive, canID.bitField.SA, IDS_NETWORKLAYERWATCH_TRANSMITINPROGRESSDISCARDRECEIVEDFRAME, Color::Red);
			return;
			break;
		case Status::ReceiveInProgress:
			TRACE(_T("ReceiveInProgress, discard previous receive.\n"));
			_AddWatchEntry(EntryType::Receive, canID.bitField.SA, IDS_NETWORKLAYERWATCH_RECEIVEINPROGRESSDISCARDPREVIOUSRECEIVE, Color::Red);
			m_pApplicationLayer->Indication(Result::N_UNEXP_PDU);
			pMessage->AbortMessage();
			break;
		case Status::Idle:
			break;
		}
		break;
	case PCIType::FirstFrame:
		TRACE(_T("FirstFrame.\n"));
		_AddWatchEntry(EntryType::Receive, canID.bitField.SA, IDS_NETWORKLAYERWATCH_INDICATIONFIRSTFRAME);
		switch (pMessage->status)
		{
		case Status::TransmitInProgress:
			TRACE(_T("TransmitInProgress, discard received frame.\n"));
			_AddWatchEntry(EntryType::Receive, canID.bitField.SA, IDS_NETWORKLAYERWATCH_TRANSMITINPROGRESSDISCARDRECEIVEDFRAME, Color::Red);
			return;
			break;
		case Status::ReceiveInProgress:
			TRACE(_T("ReceiveInProgress, discard previous receive.\n"));
			_AddWatchEntry(EntryType::Receive, canID.bitField.SA, IDS_NETWORKLAYERWATCH_RECEIVEINPROGRESSDISCARDRECEIVEDFRAME, Color::Red);
			m_pApplicationLayer->Indication(Result::N_UNEXP_PDU);
			pMessage->AbortMessage();
			break;
		case Status::Idle:
			break;
		}
		break;
	case PCIType::ConsecutiveFrame:
		TRACE(_T("ConsecutiveFrame.\n"));
		_AddWatchEntry(EntryType::Receive, canID.bitField.SA, IDS_NETWORKLAYERWATCH_INDICATIONCONSECUTIVEFRAME);
		switch (pMessage->status)
		{
		case Status::TransmitInProgress:
			TRACE(_T("TransmitInProgress, discard received frame.\n"));
			_AddWatchEntry(EntryType::Receive, canID.bitField.SA, IDS_NETWORKLAYERWATCH_TRANSMITINPROGRESSDISCARDRECEIVEDFRAME, Color::Red);
			return;
			break;
		case Status::ReceiveInProgress:
			if (!pMessage->nRemainderFrameCount)
			{
				TRACE(_T("No remainder frame, discard received frame..\n"));
				_AddWatchEntry(EntryType::Receive, canID.bitField.SA, IDS_NETWORKLAYERWATCH_RECEIVENOREMAINDERFRAME, Color::Red);
				return;
			}
			break;
		case Status::Idle:
			TRACE(_T("Idle now, discard received frame.\n"));
			_AddWatchEntry(EntryType::Receive, canID.bitField.SA, IDS_NETWORKLAYERWATCH_IDLEDISCARDRECEIVEDFRAME, Color::Red);
			return;
			break;
		}
		break;
	case PCIType::FlowControl:
		TRACE(_T("FlowControl.\n"));
		_AddWatchEntry(EntryType::Receive, canID.bitField.SA, IDS_NETWORKLAYERWATCH_INDICATIONFLOWCONTROL);
		switch (pMessage->status)
		{
		case Status::TransmitInProgress:
			if (pMessage->nRemainderFrameCount)
			{
				TRACE(_T("Still have reminder frame(s), discard received frame.\n"));
				_AddWatchEntry(EntryType::Receive, canID.bitField.SA, IDS_NETWORKLAYERWATCH_REMAINDERFRAMES, Color::Red);
				return;
			}
			break;
		case Status::ReceiveInProgress:
			TRACE(_T("ReceiveInProgress, discard received frame.\n"));
			_AddWatchEntry(EntryType::Receive, canID.bitField.SA, IDS_NETWORKLAYERWATCH_RECEIVEINPROGRESSDISCARDRECEIVEDFRAME, Color::Red);
			return;
			break;
		case Status::Idle:
			TRACE(_T("Idle now, discard received frame.\n"));
			_AddWatchEntry(EntryType::Receive, canID.bitField.SA, IDS_NETWORKLAYERWATCH_IDLEDISCARDRECEIVEDFRAME, Color::Red);
			return;
			break;
		}
		break;
	default:
		// Unknown N_PDU
		TRACE(_T("Unknown PDU, discard received frame.\n"));
		_AddWatchEntry(EntryType::Receive, canID.bitField.SA, IDS_NETWORKLAYERWATCH_UNKNOWNPDU, Color::Red);
		return;
	}

	_AddWatchEntry(EntryType::Receive, canID.bitField.SA, vbyData);
	// 处理收到的 N_PDU。
	UINT nDataIndex;
	UINT nApplicationLayerDataLength;
	switch (byPCIType)
	{
	case PCIType::SingleFrame:
		{
			// 15675-2: 6.5.2
			nApplicationLayerDataLength = byPCIFirst & 0x0F;
			_AddWatchEntry(EntryType::Receive, canID.bitField.SA, IDS_NETWORKLAYERWATCH_APPLICATIONLAYERDATALENGTH, nApplicationLayerDataLength);

			// 错误处理。
			if (!nApplicationLayerDataLength)
			{
				TRACE(_T("Length is 0, discard this frame.\n"));
				_AddWatchEntry(EntryType::Receive, canID.bitField.SA, IDS_NETWORKLAYERWATCH_NOAPPLICATIONLAYERDATA, Color::Red);
				return;
			}
			else if (nApplicationLayerDataLength > 7 || nApplicationLayerDataLength == 7 && messageType == CNetworkLayer::MessageType::RemoteDiagnostics)
			{
				TRACE(_T("Length is too long, discard this frame.\n"));
				_AddWatchEntry(EntryType::Receive, canID.bitField.SA, IDS_NETWORKLAYERWATCH_APPLICATIONLAYERDATALENGTHTOOLONG, Color::Red);
				return;
			}

			// 定时操作。
			pMessage->ResetTiming(TimingType::Idle, m_eventTiming);

			nDataIndex = nPCIIndex + 1;
			pMessage->vbyData.insert(pMessage->vbyData.cend(), vbyData.cbegin() + nDataIndex, vbyData.cbegin() + nDataIndex + nApplicationLayerDataLength);
			m_pApplicationLayer->Indication(messageType, canID.bitField.SA, canID.bitField.PS, targetAddressType, byAddressExtension, pMessage->vbyData, Result::N_OK);
			_AddWatchEntry(EntryType::Confirm, canID.bitField.PS, IDS_NETWORKLAYERWATCH_RECEIVEFINISHED, Color::Green);
			break;
		}
	case PCIType::FirstFrame:
		{
			// 15765-2: 6.5.3

			nApplicationLayerDataLength = (byPCIFirst & 0x0F) << 8 | vbyData.at(nPCIIndex + 1);
			TRACE(_T("Length is %d.\n"), nApplicationLayerDataLength);
			_AddWatchEntry(EntryType::Receive, canID.bitField.SA, IDS_NETWORKLAYERWATCH_APPLICATIONLAYERDATALENGTH, nApplicationLayerDataLength);
			// 错误处理。IDS_NETWORKLAYERWATCH_APPLICATIONLAYERDATALENGTH
			if (nApplicationLayerDataLength < CANFRAMEDATALENGTHMAX && messageType == MessageType::Diagnostics || nApplicationLayerDataLength < 7 && messageType == MessageType::RemoteDiagnostics)
			{
				TRACE(_T("Length is too short, discard this frame.\n"));
				_AddWatchEntry(EntryType::Receive, canID.bitField.SA, IDS_NETWORKLAYERWATCH_APPLICATIONLAYERDATALENGTHTOOSHORT, Color::Red);
				return;
			}

			// 定时操作。
			pMessage->ResetTiming(TimingType::Br, m_eventTiming);

			nDataIndex = nPCIIndex + 2;
			// 假设不会溢出。
			pMessage->vbyData.clear();
			pMessage->vbyData.resize(nApplicationLayerDataLength);
			for (int i = nDataIndex; i != CANFRAMEDATALENGTHMAX; ++i)
			{
				pMessage->vbyData.at(i - nDataIndex) = vbyData.at(i);
			}

			pMessage->status = Status::ReceiveInProgress;
			pMessage->stLocation = CANFRAMEDATALENGTHMAX - nDataIndex;
			pMessage->messageType = messageType;
			pMessage->nRemainderFrameCount = 0;
			pMessage->byExpectedSequenceNumber = 1;
			m_pApplicationLayer->FirstFrameIndication(messageType, canID.bitField.SA, canID.bitField.PS, targetAddressType, vbyData.at(0), nApplicationLayerDataLength);

			_Request(PCIType::FlowControl, messageType, canID.bitField.PS, canID.bitField.SA, targetAddressType, vbyData.at(0), pMessage->vbyData);
			break;
		}
	case PCIType::ConsecutiveFrame:
		{
			// 15765-2: 6.5.4

			// 错误处理。
			if (pMessage->byExpectedSequenceNumber != (byPCIFirst & 0x0F))
			{
				TRACE(_T("Wrong SN, discard this frame.\n"));
				_AddWatchEntry(EntryType::Receive, canID.bitField.SA, IDS_NETWORKLAYERWATCH_INVALIDSN, Color::Red);
				m_pApplicationLayer->Indication(Result::N_WRONG_SN);
				return;
			}
			pMessage->byExpectedSequenceNumber = (pMessage->byExpectedSequenceNumber + 1) % 0x10;

			nDataIndex = nPCIIndex + 1;
			UINT nReceiveLength = min(7, pMessage->vbyData.size() - pMessage->stLocation);
			for (int i = 0; i != nReceiveLength; ++i)
			{
				pMessage->vbyData.at(pMessage->stLocation + i) = vbyData.at(i);
			}
			pMessage->stLocation += nReceiveLength;
			--pMessage->nRemainderFrameCount;
			if (pMessage->stLocation >= pMessage->vbyData.size())
			{
				TRACE(_T("Multiframe receive finished.\n"));
				_AddWatchEntry(EntryType::Receive, canID.bitField.SA, IDS_NETWORKLAYERWATCH_RECEIVEFINISHED, Color::Green);
				pMessage->ResetTiming(TimingType::Idle, m_eventTiming);
				pMessage->status = Status::Idle;
				m_pApplicationLayer->Indication(messageType, canID.bitField.SA, canID.bitField.PS, targetAddressType, vbyData.at(0), pMessage->vbyData, Result::N_OK);
			}
			else
			{
				pMessage->ResetTiming(TimingType::Cr, m_eventTiming);
				if (!pMessage->nRemainderFrameCount)
				{
					_Request(PCIType::FlowControl, messageType, canID.bitField.PS, canID.bitField.SA, targetAddressType, vbyData.at(0), pMessage->vbyData);
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
					pMessage->ResetTiming(TimingType::Cs, m_eventTiming);

					pMessage->bySeparationTimeMin = vbyData.at(nPCIIndex + 2);
					if (pMessage->bySeparationTimeMin >= 0x80 && pMessage->bySeparationTimeMin <= 0xF0 || pMessage->bySeparationTimeMin >= 0xFA)
					{
						pMessage->bySeparationTimeMin = 0x7F;
					}
					pMessage->nRemainderFrameCount = vbyData.at(nPCIIndex + 1);
					_AddWatchEntry(EntryType::Receive, canID.bitField.SA, IDS_NETWORKLAYERWATCH_FLOWCONTROL_CONTINUETOSEND, pMessage->nRemainderFrameCount);
					
					// 15765-2: 6.5.5.4, Table 14
					if (!pMessage->nRemainderFrameCount)
					{
						pMessage->nRemainderFrameCount = -1;	// 取最大值
					}

					DWORD dwWaitResult = WaitForSingleObject(m_eventStopThread.m_hObject, pMessage->bySeparationTimeMin);
					if (dwWaitResult == WAIT_TIMEOUT)
					{
						_Request(PCIType::ConsecutiveFrame, pMessage->messageType, pMessage->address.bitField.bySourceAddress, pMessage->address.bitField.byTargetAddress, pMessage->address.bitField.targetAddressType, pMessage->address.bitField.byAddressExtension, pMessage->vbyData);
					}
					break;
				}
			case FlowControlType::Wait:
				TRACE(_T("FlowControl: Wait.\n"));
				_AddWatchEntry(EntryType::Receive, canID.bitField.SA, IDS_NETWORKLAYERWATCH_FLOWCONTROL_WAIT);
				pMessage->ResetTiming(TimingType::Bs, m_eventTiming);
				return;
				break;
			case FlowControlType::Overflow:
				TRACE(_T("FlowControl: Overflow.\n"));
				_AddWatchEntry(EntryType::Receive, canID.bitField.SA, IDS_NETWORKLAYERWATCH_FLOWCONTROL_OVERFLOW, Color::Red);
				pMessage->ResetTiming(TimingType::Idle, m_eventTiming);
				m_pApplicationLayer->Confirm(messageType, canID.bitField.SA, canID.bitField.PS, targetAddressType, vbyData.at(0), Result::N_BUFFER_OVFLW);
				return;
			default:
				TRACE(_T("FlowControl: Invalid flowcontrol.\n"));
				_AddWatchEntry(EntryType::Receive, canID.bitField.SA, IDS_NETWORKLAYERWATCH_FLOWCONTROL_INVALID, Color::Red);
				pMessage->ResetTiming(TimingType::Idle, m_eventTiming);
				m_pApplicationLayer->Confirm(messageType, canID.bitField.SA, canID.bitField.PS, targetAddressType, vbyData.at(0), Result::N_INVALID_FS);
				return;
			}
			break;
		}
	}
	return;
}

void CNetworkLayer::SetApplicationLayer(CApplicationLayer &applicationLayer)
{
	m_pApplicationLayer = &applicationLayer;

	_StopThread();
	_StartThread();
}

void CNetworkLayer::SetDataLinkLayer(CDataLinkLayer &dataLinkLayer)
{
	m_pDataLinkLayer = &dataLinkLayer;
}

void CNetworkLayer::SetDiagnosticControl(CDiagnosticControl &diagnosticControl)
{
	m_pDiagnosticControl = &diagnosticControl;
}

UINT CNetworkLayer::_TimingThread(LPVOID lpParam)
{
	CNetworkLayer *pThis = static_cast<CNetworkLayer *>(lpParam);
	CApplicationLayer *pApplicationLayer = pThis->m_pApplicationLayer;
	ASSERT(pApplicationLayer);
	HANDLE hEvents[] = { pThis->m_eventStopThread, pThis->m_eventTiming };
	CSingleLock lockMessageList(&pThis->m_csectionMessageList);
	DWORD dwWaitResult = -1;
	PMessageList::const_iterator iter;
	DWORD dwTimingTickSpan;	// 存在几率极小的 49.7 天归零问题。
	BOOL bTimeout;
	BOOL bAbort;
	while (dwWaitResult != WAIT_OBJECT_0)
	{
		dwWaitResult = WaitForMultipleObjects(sizeof(hEvents) / sizeof(HANDLE), hEvents, FALSE, INFINITE);
		if (dwWaitResult == WAIT_OBJECT_0 + 1)
		{
			WaitForSingleObject(pThis->m_eventStopThread.m_hObject, TIMINGCYCLE);
			lockMessageList.Lock();
			if (pThis->m_lpMessage.empty())
			{
				pThis->m_eventTiming.ResetEvent();
				lockMessageList.Unlock();
				continue;
			}
			iter = pThis->m_lpMessage.cbegin();
			while (iter != pThis->m_lpMessage.cend())
			{
				Message &message = **iter;
				if (TryEnterCriticalSection(&message.csectionProcess.m_sect))	// 若检测的消息正被访问，则暂时跳过。
				{
					if (message.timingType == TimingType::Idle)				// 空闲就清除消息。
					{
						TRACE(_T("\nIdle now, delete message 0x%X.\n"), message.address.nUnionAddress);
						PMessageList::const_iterator iterDelete = iter;
						++iter;
						delete *iterDelete;
						pThis->m_lpMessage.erase(iterDelete);
						continue;
					}
					dwTimingTickSpan = GetTickCount() - message.dwTimingStartTick;
					bTimeout = pThis->m_anTimingParameters[static_cast<UINT>(message.timingType)] < dwTimingTickSpan;
					if (bTimeout)
					{
						bAbort = TRUE;
						// 15765-2: 6.7.2
						switch (message.timingType)
						{
						case TimingType::As:
							TRACE(_T("\nNetworkLayer.Timeout.As\n"));
							pApplicationLayer->Confirm(Result::N_TIMEOUT_A);
							pThis->_AddWatchEntry(EntryType::Transmit, message.address.bitField.byTargetAddress, IDS_NETWORKLAYERWATCH_TIMEOUT_AS, Color::Red);
							break;
						case TimingType::Ar:
							TRACE(_T("\nNetworkLayer.Timeout.Ar\n"));
							pApplicationLayer->Indication(Result::N_TIMEOUT_A);
							pThis->_AddWatchEntry(EntryType::Receive, message.address.bitField.bySourceAddress, IDS_NETWORKLAYERWATCH_TIMEOUT_AR, Color::Red);
							break;
						case TimingType::Bs:
							TRACE(_T("\nNetworkLayer.Timeout.Bs\n"));
							pApplicationLayer->Confirm(Result::N_TIMEOUT_Bs);
							pThis->_AddWatchEntry(EntryType::Transmit, message.address.bitField.byTargetAddress, IDS_NETWORKLAYERWATCH_TIMEOUT_BS, Color::Red);
							break;
						case TimingType::Br:
							TRACE(_T("\nNetworkLayer.Timeout.Br\n"));
							message.ResetTiming(TimingType::Br, pThis->m_eventTiming);
							bAbort = FALSE;
							pThis->_AddWatchEntry(EntryType::Receive, message.address.bitField.bySourceAddress, IDS_NETWORKLAYERWATCH_TIMEOUT_BR, Color::Red);
							// 流量控制帧发送过慢，应避免。
							break;
						case TimingType::Cs:
							TRACE(_T("\nNetworkLayer.Timeout.Cs\n"));
							message.ResetTiming(TimingType::Cs, pThis->m_eventTiming);
							bAbort = FALSE;
							pThis->_AddWatchEntry(EntryType::Transmit, message.address.bitField.byTargetAddress, IDS_NETWORKLAYERWATCH_TIMEOUT_CS, Color::Red);
							// 连续帧发送过慢，应避免。
							break;
						case TimingType::Cr:
							TRACE(_T("\nNetworkLayer.Timeout.Cr\n"));
							pApplicationLayer->Indication(Result::N_TIMEOUT_Cr);
							pThis->_AddWatchEntry(EntryType::Receive, message.address.bitField.bySourceAddress, IDS_NETWORKLAYERWATCH_TIMEOUT_CR, Color::Red);
							break;
						}
						if (bAbort)
						{
							message.AbortMessage();
							message.ResetTiming(TimingType::Idle, pThis->m_eventTiming);
						}
					}
					message.csectionProcess.Unlock();
				}
				++iter;
			}
			lockMessageList.Unlock();
		}
	}
	pThis->m_pTimingThread = NULL;
	pThis->m_eventTimingThreadExited.SetEvent();
	return 0;
}

void CNetworkLayer::_AddWatchEntry(EntryType entryType, UINT nID, UINT nDescriptionID, Color color)
{
	if (NULL != m_pDiagnosticControl)
	{
		m_pDiagnosticControl->AddWatchEntry(CDiagnosticControl::LayerType::NetworkLayer, entryType, nID, nDescriptionID, color);
	}
}

void CNetworkLayer::_AddWatchEntry(EntryType entryType, UINT nID, const BYTEVector &vbyData, Color color)
{
	if (NULL != m_pDiagnosticControl)
	{
		m_pDiagnosticControl->AddWatchEntry(CDiagnosticControl::LayerType::NetworkLayer, entryType, nID, vbyData, color);
	}
}

void CNetworkLayer::_AddWatchEntry(EntryType entryType, UINT nID, UINT nDescriptionID, int nData, Color color)
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

		CSingleLock lockMessageList(&m_csectionMessageList);
		lockMessageList.Lock();
		while (!m_lpMessage.empty())
		{
			Message *pMessage = m_lpMessage.back();
			pMessage->csectionProcess.Lock();
			delete pMessage;
			m_lpMessage.pop_back();
		}
		lockMessageList.Unlock();
	}
}
