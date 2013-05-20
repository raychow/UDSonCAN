
// UDSonCANDoc.h : CUDSonCANDoc 类的接口
//

#pragma once

#include "DiagnosticStorage.h"

class CDiagnosticControl;

class CUDSonCANDoc : public CDocument
{
protected: // 仅从序列化创建
	CUDSonCANDoc();
	DECLARE_DYNCREATE(CUDSonCANDoc)

// 特性
public:
	DiagnosticStorage::CSection *GetDiagnosticStorage();
	CDiagnosticControl &GetDiagnosticControl();

// 操作
public:

// 重写
public:
	virtual BOOL OnNewDocument();
	//virtual void Serialize(CArchive &ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC &dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// 实现
public:
	virtual ~CUDSonCANDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext &dc) const;
#endif

protected:
	CDiagnosticControl *m_pDiagnosticControl;
	DiagnosticStorage::CSection m_diagnosticStorage;	// 根节点

// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// 用于为搜索处理程序设置搜索内容的 Helper 函数
	void SetSearchContent(const CString &value);
#endif // SHARED_HANDLERS
public:
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void OnCloseDocument();
};
