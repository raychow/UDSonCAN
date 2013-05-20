#include "stdafx.h"
#include "DiagnosticStorage.h"

using std::lower_bound;
using std::sort;

#include "DiagnosticService.h"

namespace DiagnosticStorage
{
	CStorage::CStorage(BYTE byID)
		: m_byID(byID) { }

	CStorage::~CStorage() { Clear(); }

	UINT CStorage::GetID() const { return m_byID; }

	void CStorage::Clear()
	{
		for (StorageVector::const_iterator iter = m_vpStorage.cbegin(); iter != m_vpStorage.cend(); ++iter)
		{
			delete *iter;
		}
		m_vpStorage.clear();
	}

	void CStorage::Sort()
	{
		if (m_vpStorage.size() <= 1)
		{
			return;
		}
		sort(m_vpStorage.begin(), m_vpStorage.end(), Compare<CStorage>(CompareType::less));
		for (StorageVector::const_iterator iter = m_vpStorage.cbegin(); iter != m_vpStorage.end(); ++iter)
		{
			(*iter)->Sort();
		}
	}

	CStorage *CStorage::GetStorage(BYTE byID) { return const_cast<CStorage *>(_GetStorage(byID)); }

	const CStorage *CStorage::GetStorage(BYTE byID) const { return _GetStorage(byID); }

	const CStorage *CStorage::_GetStorage(BYTE byID) const
	{
		StorageVector::const_iterator iter = lower_bound(m_vpStorage.cbegin(), m_vpStorage.cend(), byID, Compare<CStorage>(CompareType::less));
		if (iter != m_vpStorage.cend() && (*iter)->GetID() == byID)
		{
			return *iter;
		}
		return NULL;
	}

	CSection::CSection(BYTE byID)
		: CStorage(byID) { }

	BYTE CSection::GetData(BYTEVector::size_type stPosition) const { ASSERT(m_pDiagnosticServiceSection->GetDataType() == DiagnosticService::DataType::Binary); return m_vbyData.at(stPosition); }

	BYTE CSection::GetEnumerate() const
	{
		ASSERT(m_pDiagnosticServiceSection->GetDataType() == DiagnosticService::DataType::Enumerate);
		if ( !m_vbyData.empty() ) {
			return m_vbyData.at(0);
		} else { 
			BYTE byID = m_pDiagnosticServiceSection->GetItem(0)->GetItemID();
			return byID;
		}
	}

	void CSection::PushData(BYTE byData) { m_vbyData.push_back(byData); }

	void CSection::ClearData() { m_vbyData.clear(); }

	BYTEVector::size_type CSection::GetDataSize() const { return m_vbyData.size(); }

	BOOL CSection::IsDataEmpty() const { return m_vbyData.empty(); }

	const BYTEVector &CSection::GetDataVector() const { return m_vbyData; }

	CItem *CSection::GetItem(BYTE byID) { return static_cast<CItem *>(GetStorage(byID)); }

	const CItem *CSection::GetItem(BYTE byID) const { return static_cast<const CItem *>(GetStorage(byID)); }

	CItem *CSection::AddItem(BYTE byID) { CItem *pItem = new CItem(byID); m_vpStorage.push_back(pItem); return pItem; }

	CItem *CSection::GetOrAddItem(BYTE byID)
	{
		CItem *pItem = GetItem(byID);
		if (pItem)
		{
			return pItem;
		}
		else
		{
			return AddItem(byID);
		}
	}

	const DiagnosticService::CSection *CSection::GetAssociatedDiagnosticServiceSection() const { return m_pDiagnosticServiceSection; }

	void CSection::SetAssociatedDiagnosticServiceSection(DiagnosticService::CSection *pDiagnosticServiceSection) { ASSERT(pDiagnosticServiceSection); m_pDiagnosticServiceSection = pDiagnosticServiceSection; }

	CItem::CItem(BYTE byID)
		: CStorage(byID) { }

	CSection *CItem::GetSection(BYTE byID) { return static_cast<CSection *>(GetStorage(byID)); }
	
	const CSection *CItem::GetSection(BYTE byID) const { return static_cast<const CSection *>(GetStorage(byID)); }
	
	CSection *CItem::AddSection(BYTE byID) { CSection *pSection = new CSection(byID); m_vpStorage.push_back(pSection); return pSection; }

	CSection *CItem::GetOrAddSection(BYTE byID)
	{
		CSection *pSection = GetSection(byID);
		if (pSection)
		{
			return pSection;
		}
		else
		{
			return AddSection(byID);
		}
	}

	const DiagnosticService::CItem *CItem::GetAssociatedDiagnosticServiceItem() const { return m_pDiagnosticServiceItem; }

	void CItem::SetAssociatedDiagnosticServiceItem(DiagnosticService::CItem *pDiagnosticServiceItem) { ASSERT(pDiagnosticServiceItem); m_pDiagnosticServiceItem = pDiagnosticServiceItem; }
}