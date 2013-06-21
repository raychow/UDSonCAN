#pragma once

#include <vector>
#include <boost/signals2.hpp>

namespace Diagnostic
{
	enum PhysicalLayerSendType : BYTE
	{
		Normal		= 0,
		Single,
		LoopBack,
		SingleLoopBack,
	};

	enum struct NetworkLayerResult : BYTE	// ÍøÂç²ãÇëÇó½á¹û¡£
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

	typedef std::vector<BYTE> BYTEVector;

	typedef boost::signals2::signal<void (UINT32, const BYTEVector &)> IndicationSignal;
	typedef boost::signals2::signal<void ()> ConfirmSignal;

	typedef boost::signals2::signal<void (UINT32, const BYTEVector &, NetworkLayerResult)> IndicationASignal;	// Application Layer
	typedef boost::signals2::signal<void (NetworkLayerResult)> ConfirmASignal;									// Application Layer
	typedef boost::signals2::signal<void (UINT32, UINT)> IndicationAFSignal;									// Application Layer

	typedef boost::signals2::signal<BOOL (UINT32, const Diagnostic::BYTEVector &, PhysicalLayerSendType, BOOL, BOOL)> PhysicalLayerTrasnmitSignal;
	typedef boost::signals2::signal<void (UINT32, const Diagnostic::BYTEVector &)> RequestSignal;
	
}