#pragma once

#include "DiagnosticControl.h"
#include "PhysicalLayer.h"
#include "DataLinkLayer.h"
#include "NetworkLayer.h"
#include "ApplicationLayer.h"

// CTestDialog 对话框

class CTestDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CTestDialog)

public:
	CTestDialog(CDiagnosticControl &diagnosticControl, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CTestDialog();

// 对话框数据
	enum { IDD = IDD_TEST };

protected:
	CDiagnosticControl &m_diagnosticControl;

	void _MessagePrompt(BOOL bResult) const;
	INT32 _GetNrequestID() const;

	static UINT FakeCFThread(LPVOID lpParam);
	CWinThread *m_pFakeCFThread;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClickedButtonTestOpendevice();
	afx_msg void OnClickedButtonTestClosedevice();
	afx_msg void OnClickedButtonTestOpencan();
	afx_msg void OnClickedButtonTestResetcan();
	afx_msg void OnBnClickedButtonTestNrequest();
protected:
	CString m_csNrequestData;
	CString m_csNrequestID;
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonTestFakesf();
protected:
	CString m_csFakeSFID;
	CString m_csFakeSFRequestData;
public:
	afx_msg void OnBnClickedButtonTestFakemf();
	afx_msg void OnBnClickedButtonTestFakeff();
	afx_msg void OnBnClickedButtonTestFakecf();
	afx_msg void OnBnClickedButtonTestFakefc();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnDestroy();
};
