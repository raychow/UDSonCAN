#pragma once

#include <vector>

#include "DiagnosticService.h"

namespace DiagnosticResponse
{
	enum ResponseType
	{
		Positive,
		Negative
	};

	class CSection;
	class CItem;
	class CServiceManager;

	typedef std::vector<BYTE> BYTEVector;
	typedef std::vector<CItem *> ItemVector;
	typedef std::vector<CSection *> SectionVector;
	typedef DiagnosticService::DataType DataType;

	class CSection
		: public DiagnosticService::CSection
	{
	public:
		CSection(DataType dataType, BYTE bySectionID, LPCTSTR lpszCaption = NULL, BYTEVector::size_type stDataSizeRequired = 0);
		CSection(DataType dataType, BYTE bySectionID, UINT nCaptionID = 0, BYTEVector::size_type stDataSizeRequired = 0);
		virtual ~CSection();
	};

	class CItem
		: public DiagnosticService::CItem
	{
	public:
		CItem(BYTE byItemID, LPCTSTR lpszDescription = NULL);
		CItem(BYTE byItemID, UINT nDescriptionID = 0);

		void SetSpecialProcess(UINT nSpecialProcess);

		virtual ~CItem();
	protected:
		UINT m_nSpecialProcess;	// 若不为 0，则此项按特定逻辑处理。
	};

	class CResponse
		: public CItem
	{
	public:
		CResponse(BYTE byItemID, ResponseType responseType, LPCTSTR lpszDescription = NULL);
		CResponse(BYTE byItemID, ResponseType responseType, UINT nDescriptionID = 0);
	protected:
		ResponseType m_responseType;
	};

	class CResponseManager
	{
	public:
		const CItem *GetItem(BYTE byDiagnosticServiceID) const;
		const ItemVector &GetItemVector() const;
		void SetItem(CItem *pItem);
		ItemVector::size_type FindItem(BYTE byDiagnosticResponseID) const;

		static CString GetByteString(BYTE byValue);
		static CString GetByteString(const BYTEVector &vbyValue);

		void GetWatchEntriesByData(const BYTEVector &vbyData, CStringArray &acsEntries) const;

		static CServiceManager *GetInstance();

		virtual ~CResponseManager();
	protected:
		ItemVector m_vpDiagnosticItem;

		void _BuildDiagnosticResponse();
		CResponseManager();

		friend class std::auto_ptr<CResponseManager>;
		static std::auto_ptr<CResponseManager> m_apDiagnosticService;
	};
};

