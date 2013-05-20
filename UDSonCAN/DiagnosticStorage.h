#pragma once

#include <algorithm>
#include <vector>

namespace DiagnosticService
{
	class CSection;
	class CItem;
}

namespace DiagnosticStorage
{
	class CStorage;
	class CSection;
	class CItem;

	typedef std::vector<BYTE> BYTEVector;
	typedef std::vector<CStorage *> StorageVector;
	typedef std::vector<CSection *> SectionVector;
	typedef std::vector<CItem *> ItemVector;

	class CStorage
	{
	public:
		UINT GetID() const;

		void Sort();

		void Clear();
	protected:
		StorageVector m_vpStorage;
		BYTE m_byID;

		CStorage *GetStorage(BYTE byID);
		const CStorage *GetStorage(BYTE byID) const;
		const CStorage *_GetStorage(BYTE byID) const;

		CStorage(BYTE byID);
		virtual ~CStorage();
	};

	class CSection
		: public CStorage
	{
	public:
		CSection(BYTE byID);

		BYTE GetData(BYTEVector::size_type stPosition) const;
		BYTE GetEnumerate() const;
		
		void PushData(BYTE byData);
		void ClearData();
		BYTEVector::size_type GetDataSize() const;
		BOOL IsDataEmpty() const;
		const BYTEVector &GetDataVector() const;

		CItem *GetItem(BYTE byID);
		const CItem *GetItem(BYTE byID) const;
		CItem *AddItem(BYTE byID);
		CItem *GetOrAddItem(BYTE byID);

		const DiagnosticService::CSection *GetAssociatedDiagnosticServiceSection() const;
		void SetAssociatedDiagnosticServiceSection(DiagnosticService::CSection *pDiagnosticServiceSection);
	protected:
		BYTEVector m_vbyData;
		DiagnosticService::CSection *m_pDiagnosticServiceSection;
	};

	class CItem
		: public CStorage
	{
	public:
		CItem(BYTE byID);

		CSection *GetSection(BYTE byID);
		const CSection *GetSection(BYTE byID) const;
		CSection *AddSection(BYTE byID);
		CSection *GetOrAddSection(BYTE byID);

		const DiagnosticService::CItem *GetAssociatedDiagnosticServiceItem() const;
		void SetAssociatedDiagnosticServiceItem(DiagnosticService::CItem *pDiagnosticServiceItem);
	protected:
		DiagnosticService::CItem *m_pDiagnosticServiceItem;
	};

	enum CompareType
	{
		equal,
		not_equal,
		less,
		greater,
		less_equal,
		greater_equal
	};

	template<class T>
	class Compare
	{
	public:
		Compare(CompareType compareType) { m_compareType = compareType; }
		BOOL operator()(const T *pCompare, BYTE byID) const
		{
			switch (m_compareType)
			{
			case CompareType::equal:
				return pCompare->GetID() == byID;
				break;
			case CompareType::not_equal:
				return pCompare->GetID() != byID;
				break;
			case CompareType::less:
				return pCompare->GetID() < byID;
				break;
			case CompareType::greater:
				return pCompare->GetID() > byID;
				break;
			case CompareType::less_equal:
				return pCompare->GetID() <= byID;
				break;
			case CompareType::greater_equal:
				return pCompare->GetID() >= byID;
				break;
			}
			ASSERT(FALSE);
			return FALSE;
		}
		BOOL operator()(const T *pCompare1, const T *pCompare2) const
		{
			return operator()(pCompare1, pCompare2->GetID());
		}
	protected:
		CompareType m_compareType;
	};
}