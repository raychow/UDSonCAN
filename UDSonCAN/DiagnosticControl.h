#pragma once

#include <memory>

#include "DiagnosticType.h"

#include "WatchWnd.h"

class CPhysicalLayer;
class CDataLinkLayer;
class CNetworkLayer;
class CApplicationLayer;

class CDiagnosticControl
{
	typedef CTIDCWatchWnd::EntryType EntryType;
	typedef CWatchWnd::Color Color;
public:
	enum LayerType
	{
		ApplicationLayer,
		NetworkLayer
	};

	CDiagnosticControl(void);
	virtual ~CDiagnosticControl(void);

	CPhysicalLayer		&GetPhysicalLayer();
	CDataLinkLayer		&GetDataLinkLayer();
	CNetworkLayer		&GetNetworkLayer();
	CApplicationLayer	&GetApplicationLayer();

	void SetApplicationLayerWatchWnd(CTIDCWatchWnd &applicationLayerWatchWnd);
	void SetNetworkLayerWatchWnd(CTIDCWatchWnd &networkLayerWatchWnd);

	void LoadConfig();
	void SaveConfig();

	void ResetTiming();

	void AddWatchEntry(LayerType layerType, EntryType entryType, UINT32 nID, LPCTSTR lpszDescription, Color color = Color::Black) const;
	void AddWatchEntry(LayerType layerType, EntryType entryType, UINT32 nID, const Diagnostic::BYTEVector &vbyData, Color color = Color::Black) const;
	void AddWatchEntry(LayerType layerType, EntryType entryType, UINT32 nID, UINT nDescriptionID, Color color = Color::Black) const;
	void AddWatchEntry(LayerType layerType, EntryType entryType, UINT32 nID, UINT nDescriptionID, int nData, Color color = Color::Black) const;
protected:
	std::unique_ptr<CPhysicalLayer> m_upPhysicalLayer;
	std::unique_ptr<CDataLinkLayer> m_upDataLinkLayer;
	std::unique_ptr<CNetworkLayer> m_upNetworkLayer;
	std::unique_ptr<CApplicationLayer> m_upApplicationLayer;
	
	boost::signals2::connection m_connectionPhysicalLayerTrasnmit;
	boost::signals2::connection m_connectionDataLinkLayerConfirm;
	boost::signals2::connection m_connectionDataLinkLayerIndication;
	boost::signals2::connection m_connectionDataLinkLayerRequest;
	boost::signals2::connection m_connectionNetworkLayerConfirm;
	boost::signals2::connection m_connectionNetworkLayerIndication;
	boost::signals2::connection m_connectionNetworkLayerRequest;
	boost::signals2::connection m_connectionApplicationLayerConfirm;
	boost::signals2::connection m_connectionApplicationLayerIndication;
	boost::signals2::connection m_connectionApplicationLayerFirstFrameIndication;

	CPhysicalLayer		*m_pPhysicalLayer;
	CDataLinkLayer		*m_pDataLinkLayer;
	CNetworkLayer		*m_pNetworkLayer;
	CApplicationLayer	*m_pApplicationLayer;

	DWORD m_dwStartTick;

	CTIDCWatchWnd *m_pApplicationLayerWatchWnd;
	CTIDCWatchWnd *m_pNetworkLayerWatchWnd;

	CCriticalSection m_csectionNetworkLayerWatchWnd;
	CCriticalSection m_csectionApplicationLayerWatchWnd;

	CStringA m_csaConfigFilename;

	void _AddWatchEntry(LayerType layerType, EntryType entryType, UINT32 nID, LPCTSTR lpszDescription, Color color = Color::Black) const;
};

