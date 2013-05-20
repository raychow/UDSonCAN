#pragma once
#include "afxpropertygridctrl.h"
class CRayPropertyGridCtrl :
	public CMFCPropertyGridCtrl
{
public:
	CRayPropertyGridCtrl(void);
	virtual ~CRayPropertyGridCtrl(void);
	void SetLeftColumnWidth(int nNewWidth);
	virtual void OnPropertyChanged(CMFCPropertyGridProperty *pProp) const;
	//virtual int OnDrawProperty(CDC* pDC, CMFCPropertyGridProperty* pProp);
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize(UINT nType, int cx, int cy);

protected:
	int GetTotalItems(BOOL bIncludeHidden) const;	// Òþ²Ø¸¸Ààº¯Êý£¬ÓÐ´ý¿¼²ì¡£
};

