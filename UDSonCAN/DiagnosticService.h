#pragma once

#include <fstream>
#include <memory>
#include <vector>

#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>

#include "WatchWnd.h"

namespace DiagnosticStorage
{
	class CSection;
	class CItem;
};

namespace DiagnosticService
{
	enum
	{
		DiagnosticSessionControl		= 0x10,
		ECUReset						= 0x11,
		SecurityAccess					= 0x27,
		CommunicationControl			= 0x28,
		TesterPresent					= 0x3E,
		AccessTimingParameter			= 0x83,
		SecuredDataTransmission			= 0x84,
		ControlDTCSetting				= 0x85,
		ResponseOnEvent					= 0x86,
		LinkControl						= 0x87,
		ReadDataByIdentifier			= 0x22,
		ReadMemoryByAddress				= 0x23,
		ReadScalingDataByIdentifier		= 0x24,
		ReadDataByPeriodicIdentifier	= 0x2A,
		DynamicallyDefineDataIdentifier	= 0x2C,
		WriteDataByIdentifier			= 0x2E,
		WriteMemoryByAddress			= 0x3D,
		ClearDiagnosticInformation		= 0x14,
		ReadDTCInformation				= 0x19,
		InputOutputControlByIdentifier	= 0x2F,
		RoutineControl					= 0x31,
		RequestDownload					= 0x34,
		RequestUpload					= 0x35,
		TransferData					= 0x36,
		RequestTransferExit				= 0x37,
	};

	enum DataType
	{
		Enumerate,
		Binary
	};

	enum OtherServiceID
	{
		NoAction	= 0x10000,
		Download,
		Test,
	};

	class CSection;
	class CItem;
	class CServiceManager;

	typedef std::vector<BYTE> BYTEVector;
	typedef std::vector<CItem *> ItemVector;
	typedef std::vector<CSection *> SectionVector;

	// 服务中的某项
	class CSection
	{
	public:
		CSection(DataType dataType, BYTE bySectionID, LPCTSTR lpszCaption, BYTEVector::size_type stDataSizeRequired = 0);
		CSection(DataType dataType, BYTE bySectionID, UINT nCaptionID, BYTEVector::size_type stDataSizeRequired = 0);
		virtual ~CSection();

		DataType GetDataType() const;

		BYTE GetSectionID() const;

		const CString &GetCaption() const;

		virtual void AppendData(const DiagnosticStorage::CSection *pStorageSection, CString &csData) const;
		virtual BOOL AppendData(const DiagnosticStorage::CSection *pStorageSection, BYTEVector &vbyData) const;
		virtual CString GetOutputString(const DiagnosticStorage::CSection *pStorageSection) const;

		BYTEVector::size_type GetDataSizeRequired() const;
		void SetDataSizeRequired(BYTEVector::size_type stNewDataSizeRequired);

		CString GetItemCaptionString(BYTE byItemID) const;
		const CItem *GetItem(ItemVector::size_type stPosition) const;
		const CItem *GetItemByID(BYTE byID) const;

		void PushItem(CItem *pItem);
		void ClearItem();

		ItemVector::size_type GetItemCount() const;
		BOOL IsItemEmpty() const;
		const ItemVector &GetItemVector() const;
		
		const SectionVector &GetCurrentItemSectionVector(const DiagnosticStorage::CSection *pDiagnosticStorage) const;
	protected:
		DataType m_dataType;
		ItemVector m_vpItem;
		BYTE m_bySectionID;
		// UINT m_nCaptionID;
		CString m_csCaption;
		BYTEVector::size_type m_stDataSizeRequired;
	};

	// 服务
	class CItem
	{
	public:
		//CItem(BYTE byItemID, UINT nCaptionID, UINT nDescriptionID = 0)
		//	: m_byItemID(byItemID) { m_csCaption.LoadString(nCaptionID); if (0 != nDescriptionID) { m_csDescription.LoadString(nDescriptionID); } }
		
		CItem(BYTE byItemID, LPCTSTR lpszCaption, LPCTSTR lpszDescription = NULL);
		CItem(BYTE byItemID, UINT nCaptionID = 0, UINT nDescriptionID = 0);
		virtual ~CItem();

		BYTE GetItemID() const;

		const CString &GetCaption() const;
		const CString &GetDescription() const;

		void PushSection(CSection *pSection);
		void ClearSection();

		void AppendItemData(IN const DiagnosticStorage::CItem *pStorageItem, OUT CString &csData) const;
		BOOL AppendItemData(IN const DiagnosticStorage::CItem *pStorageItem, OUT BYTEVector &vbyData) const;
		
		BYTEVector::size_type GetWatchEntriesByData(const BYTEVector &vbyData, BYTEVector::size_type stLocation, CStringArray &acsEntries) const;

		SectionVector::size_type GetSectionCount() const;
		BOOL IsSectionEmpty() const;

		const CSection *GetSection(SectionVector::size_type stPosition) const;
		const SectionVector &GetSectionVector() const;
	protected:
		BYTE m_byItemID;
		CString m_csCaption;
		CString m_csDescription;
		// UINT m_nCaptionID;
		SectionVector m_vpSection;
	};

	class CServiceManager
	{
	public:
		const CItem *GetItem(BYTE byDiagnosticServiceID) const;
		const ItemVector &GetItemVector() const;
		void SetItem(CItem *pItem);
		ItemVector::size_type FindItem(BYTE byDiagnosticServiceID) const;

		void BuildDiagnosticStorage(DiagnosticStorage::CSection *pStorageSection) const;

		static CString GetByteString(BYTE byValue);
		static CString GetByteString(const BYTEVector &vbyValue);

		void GetWatchEntriesByData(const BYTEVector &vbyData, CStringArray &acsEntries) const;

		static CServiceManager *GetInstance();

		virtual ~CServiceManager();
	protected:
		ItemVector m_vpDiagnosticItem;

		void _BuildDiagnosticService();
		CServiceManager();
	
		void _BuildDiagnosticStorage(const ItemVector &vpItem, DiagnosticStorage::CSection *pSectionRootStorage) const;

		friend class std::auto_ptr<CServiceManager>;
		static std::auto_ptr<CServiceManager> m_apDiagnosticService;
	};
}
//
//class DiagnosticService
//{
//public:
//	enum
//	{
//		DiagnosticSessionControl		= 0x10,
//		ECUReset						= 0x11,
//		SecurityAccess					= 0x27,
//		CommunicationControl			= 0x28,
//		TesterPresent					= 0x3E,
//		AccessTimingParameter			= 0x83,
//		SecuredDataTransmission			= 0x84,
//		ControlDTCSetting				= 0x85,
//		ResponseOnEvent					= 0x86,
//		LinkControl						= 0x87,
//		ReadDataByIdentifier			= 0x22,
//		ReadMemoryByAddress				= 0x23,
//		ReadScalingDataByIdentifier		= 0x24,
//		ReadDataByPeriodicIdentifier	= 0x2A,
//		DynamicallyDefineDataIdentifier	= 0x2C,
//		WriteDataByIdentifier			= 0x2E,
//		WriteMemoryByAddress			= 0x3D,
//		ClearDiagnosticInformation		= 0x14,
//		ReadDTCInformation				= 0x19,
//		InputOutputControlByIdentifier	= 0x2F,
//		RoutineControl					= 0x31,
//		RequestDownload					= 0x34,
//		RequestUpload					= 0x35,
//		TransferData					= 0x36,
//		RequestTransferExit				= 0x37,
//	};
//
//	enum DataType
//	{
//		Enumerate,
//		Binary
//	};
//
//	class CItem;
//	class CSection;
//
//	typedef vector<BYTE> BYTEVector;
//	typedef vector<CItem *> ItemVector;
//	typedef vector<CSection *> SectionVector;
//
//	// 服务中的某项
//	class CSection
//	{
//	public:
//		CSection() {}
//		CSection(DataType dataType, UINT nCaptionID, UINT nDataSizeRequired = 0)
//			: m_dataType(dataType), m_nCaptionID(nCaptionID), m_nDataSizeRequired(nDataSizeRequired) {}
//		virtual ~CSection() { ClearItem(); }
//
//		DataType GetDataType() const { return m_dataType; }
//
//		UINT GetCaptionID() const { return m_nCaptionID; }
//		void SetCaptionID(UINT nNewCaptionID) { m_nCaptionID = nNewCaptionID; }
//
//		virtual void AppendDataString(const DiagnosticStorage::CSection *pStorageSection, CString &csResult) const;
//		virtual CString GetOutputString(const DiagnosticStorage::CSection *pStorageSection) const;
//
//		int GetDataSizeRequired() const { return m_nDataSizeRequired; }
//		void SetDataSizeRequired(UINT nNewDataSizeRequired) { m_nDataSizeRequired = nNewDataSizeRequired; }
//
//		CString GetItemCaptionString(BYTE byItemID) const;
//		const CItem &GetItem(ItemVector::size_type stPosition) const { return *m_vpItem.at(stPosition); }
//		void PushItem(CItem *pItem) { m_vpItem.push_back(pItem); }
//		void ClearItem() { for (ItemVector::const_iterator iter = m_vpItem.cbegin(); iter != m_vpItem.cend(); ++iter) { delete *iter; } }
//
//		ItemVector::size_type GetItemCount() const { return m_vpItem.size(); }
//		BOOL IsItemEmpty() const { return m_vpItem.empty(); }
//		const ItemVector &GetItemVector() const { return m_vpItem; }
//		
//		BYTE GetSelectedItemID(const DiagnosticStorage::CSection *pStorageSection) const;
//		const SectionVector &GetCurrentItemSectionVector(const DiagnosticStorage::CSection *pDiagnosticStorage) const;
//	protected:
//		DataType m_dataType;
//		ItemVector m_vpItem;
//		UINT m_nCaptionID;
//		int m_nDataSizeRequired;
//	};
//
//	// 服务
//	class CItem
//	{
//	public:
//		CItem() {}
//		CItem(BYTE byItemID, UINT nCaptionID)
//			: m_byItemID(byItemID), m_nCaptionID(nCaptionID) {}
//		CItem(BYTE byItemID);
//		virtual ~CItem() { ClearSection(); }
//
//		BYTE GetItemID() const { return m_byItemID; }
//
//		UINT GetCaptionID() const { return m_nCaptionID; }
//		void SetCaptionID(UINT nCaptionID) { m_nCaptionID = nCaptionID; }
//
//		void PushSection(CSection *pSection) { ASSERT(pSection->GetDataType() != DataType::Enumerate || pSection->GetItemCount()); m_vpSection.push_back(pSection); }
//		void ClearSection() { while (!m_vpSection.empty()) { delete m_vpSection.back(); m_vpSection.pop_back(); } }
//
//		CString GetItemString(const DiagnosticStorage::CItem *pStorageItem) const;
//
//		SectionVector::size_type GetSectionCount() const { return m_vpSection.size(); }
//		BOOL IsSectionEmpty() const { return m_vpSection.empty(); }
//
//		const SectionVector &GetSectionVector() const { return m_vpSection; }
//	protected:
//		BYTE m_byItemID;
//		UINT m_nCaptionID;
//		SectionVector m_vpSection;
//	};
//
//	const CItem &GetItem(BYTE byDiagnosticServiceID) const;
//	const ItemVector &GetItemVector() const;
//	void SetItem(CItem *pItem);
//	ItemVector::size_type FindItem(BYTE byDiagnosticServiceID) const;
//
//	void BuildDiagnosticStorage(DiagnosticStorage::CSection *pStorageSection) const;
//
//	static CString GetByteString(BYTE byValue);
//	static CString GetByteString(const BYTEVector &vbyValue);
//
//	static DiagnosticService &GetInstance();
//protected:
//	ItemVector m_vpDiagnosticItem;
//
//	void _BuildDiagnosticService();
//	DiagnosticService();
//	virtual ~DiagnosticService();
//	
//	void _BuildDiagnosticService(const ItemVector &vpItem, DiagnosticStorage::CSection *pRootStorageSection) const;
//
//	friend class std::auto_ptr<DiagnosticService>;
//	static std::auto_ptr<DiagnosticService> m_apDiagnosticService;
//};
