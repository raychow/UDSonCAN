
// UDSonCANView.h : CUDSonCANView 类的接口
//

#pragma once

#include "DiagnosticService.h"
#include "RayPropertyGridCtrl.h"

class CUDSonCANView : public CView
{
// 构造
protected: // 仅从序列化创建
	CUDSonCANView();
	DECLARE_DYNCREATE(CUDSonCANView)
	virtual ~CUDSonCANView();

// 特性
public:
	CUDSonCANDoc *GetDocument() const;

protected:
	CRayPropertyGridCtrl m_propertyGridCtrl;
// 操作
public:
	void SwitchService(UINT nServiceID);
	void SwitchProperty(CMFCPropertyGridProperty *pProp);
	BYTE PickupSubItemID(LPCTSTR lpszSubItemString) const;

// 重写
public:
	virtual void OnDraw(CDC *pDC);  // 重写以绘制该视图
protected:

// 实现
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext &dc) const;
#endif

protected:
	UINT m_nServiceID;

	void InitPropertyGridCtrl();
	void PushEnumerateData(DiagnosticStorage::CSection *pStorageSection, LPCTSTR lpszDataString);
	void PushBinaryData(DiagnosticStorage::CSection *pStorageSection, LPCTSTR lpszDataString);
	void ShowSubItemSection(const DiagnosticStorage::CItem *pStorageItem, BOOL bShow = TRUE) const;
	// void ShowSubItemSection(const DiagnosticService::SectionVector &vpSection, BOOL bShow = TRUE) const;
	void AddProperties(const DiagnosticService::CItem *pItem, DiagnosticStorage::CItem *pStorageItem, BOOL bShow);

// 生成的消息映射函数
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd *pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnToolsTest();
	afx_msg void OnConfigPhysicalLayer();
	afx_msg void OnConfigNetworkLayer();
	afx_msg void OnConfigApplicationLayer();
	afx_msg void OnDiagnosticBeginDiagnostic();
	afx_msg void OnUpdateDiagnosticBeginDiagnostic(CCmdUI *pCmdUI);
	afx_msg void OnDiagnosticStopDiagnostic();
	afx_msg void OnUpdateDiagnosticStopDiagnostic(CCmdUI *pCmdUI);
	afx_msg void OnDiagnosticExecute();
	afx_msg void OnUpdateDiagnosticExecute(CCmdUI *pCmdUI);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};

#ifndef _DEBUG  // UDSonCANView.cpp 中的调试版本
inline CUDSonCANDoc *CUDSonCANView::GetDocument() const
   { return reinterpret_cast<CUDSonCANDoc*>(m_pDocument); }
#endif

