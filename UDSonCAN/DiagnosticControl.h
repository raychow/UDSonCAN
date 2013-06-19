#pragma once

#include <vector>

#include "WatchWnd.h"

class CPhysicalLayer;
class CDataLinkLayer;
class CNetworkLayer;
class CApplicationLayer;

class CDiagnosticControl
{
	typedef std::vector<BYTE> BYTEVector;
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

	void AddWatchEntry(LayerType layerType, EntryType entryType, INT32 nID, LPCTSTR lpszDescription, Color color = Color::Black) const;
	void AddWatchEntry(LayerType layerType, EntryType entryType, INT32 nID, const BYTEVector &vbyData, Color color = Color::Black) const;
	void AddWatchEntry(LayerType layerType, EntryType entryType, INT32 nID, UINT nDescriptionID, Color color = Color::Black) const;
	void AddWatchEntry(LayerType layerType, EntryType entryType, INT32 nID, UINT nDescriptionID, int nData, Color color = Color::Black) const;
protected:
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

	void _AddWatchEntry(LayerType layerType, EntryType entryType, INT32 nID, LPCTSTR lpszDescription, Color color = Color::Black) const;
};

