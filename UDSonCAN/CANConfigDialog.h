#pragma once

class CPhysicalLayer;

// CCANConfigDialog 对话框

class CCANConfigDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CCANConfigDialog)

public:
	CCANConfigDialog(CPhysicalLayer &physicalLayer, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CCANConfigDialog();

// 对话框数据
	enum { IDD = IDD_CANCONFIG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
protected:
	enum
	{
		DEVICETYPECOUNT		= 22,	// 设备类型总数。
		DEVICEINDEXCOUNT	= 8,	// 设备索引总数。
		BAUDRATETYPECOUNT	= 16,	// 预设波特率总数。
	};

	int m_nDeviceType;
	int m_nDeviceIndex;
	int m_nCANIndex;
	CString m_csAccCode;
	CString m_csAccMask;
	int m_nBaudRateType;
	CString m_csTiming0;
	CString m_csTiming1;
	int m_nFilter;
	int m_nMode;

	CPhysicalLayer &m_physicalLayer;

	void _LoadConfig();
	virtual void OnOK();

	void _UpdateUIBaudRate();
public:
	afx_msg void OnCbnSelchangeComboCanconfigBaudratetype();
};
