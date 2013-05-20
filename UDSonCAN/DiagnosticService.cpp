#include "stdafx.h"
#include "DiagnosticService.h"

#include "DiagnosticStorage.h"
#include "Resource.h"

using std::auto_ptr;

namespace DiagnosticService
{
	auto_ptr<CServiceManager> CServiceManager::m_apDiagnosticService;

	CSection::CSection(DataType dataType, BYTE bySectionID, LPCTSTR lpszCaption, BYTEVector::size_type stDataSizeRequired)
		: m_dataType(dataType), m_bySectionID(bySectionID), m_csCaption(lpszCaption), m_stDataSizeRequired(stDataSizeRequired)
	{

	}

	CSection::CSection(DataType dataType, BYTE bySectionID, UINT nCaptionID, BYTEVector::size_type stDataSizeRequired)
		: m_dataType(dataType), m_bySectionID(bySectionID), m_stDataSizeRequired(stDataSizeRequired)
	{
		m_csCaption.LoadString(nCaptionID);
	}

	CSection::~CSection()
	{
		ClearItem();
	}

	DataType CSection::GetDataType() const
	{
		return m_dataType;
	}

	BYTE CSection::GetSectionID() const
	{
		return m_bySectionID;
	}

	const CString &CSection::GetCaption() const
	{
		return m_csCaption;
	}

	void CSection::AppendData(const DiagnosticStorage::CSection *pStorageSection, CString &csData) const
	{
		ASSERT(pStorageSection);

		if (!csData.IsEmpty())
		{
			csData.AppendChar(_T(' '));
		}
		switch (m_dataType)
		{
		case DataType::Enumerate:
			{
				const SectionVector &vpSectionVector = GetCurrentItemSectionVector(pStorageSection);
				const DiagnosticStorage::CItem *pStorageItem = pStorageSection->GetItem(pStorageSection->GetEnumerate());
				csData.Append(CServiceManager::GetByteString(pStorageSection->GetEnumerate()));
				for (SectionVector::const_iterator iter = vpSectionVector.cbegin(); iter != vpSectionVector.cend(); ++iter)
				{
					(*iter)->AppendData(pStorageItem->GetSection((*iter)->GetSectionID()), csData);
				}
			}
			break;
		case DataType::Binary:
			csData.Append(CServiceManager::GetByteString(pStorageSection->GetDataVector()));
			break;
		}
	}
	
	BOOL CSection::AppendData(const DiagnosticStorage::CSection *pStorageSection, BYTEVector &vbyData) const
	{
		ASSERT(pStorageSection);
		BOOL bAllSectionFulfilled = TRUE;
		switch (m_dataType)
		{
		case DataType::Enumerate:
			{
				const SectionVector &vpSectionVector = GetCurrentItemSectionVector(pStorageSection);
				const DiagnosticStorage::CItem *pStorageItem = pStorageSection->GetItem(pStorageSection->GetEnumerate());
				vbyData.push_back(pStorageSection->GetEnumerate());
				for (SectionVector::const_iterator iter = vpSectionVector.cbegin(); iter != vpSectionVector.cend(); ++iter)
				{
					if (!(*iter)->AppendData(pStorageItem->GetSection((*iter)->GetSectionID()), vbyData) && bAllSectionFulfilled)
					{
						bAllSectionFulfilled = FALSE;
					}
				}
			}
			break;
		case DataType::Binary:
			const BYTEVector &vbyStorage = pStorageSection->GetDataVector();
			if (vbyStorage.empty() || m_stDataSizeRequired && (vbyStorage.size() < m_stDataSizeRequired))
			{
				bAllSectionFulfilled = FALSE;
			}
			else
			{
				vbyData.insert(vbyData.cend(), vbyStorage.cbegin(), vbyStorage.cend());
			}
			break;
		}
		return bAllSectionFulfilled;
	}

	CString CSection::GetOutputString(const DiagnosticStorage::CSection *pStorageSection) const
	{
		ASSERT(pStorageSection);

		CString csResult;
		switch (m_dataType)
		{
		case DataType::Enumerate:
			csResult = GetItemCaptionString(pStorageSection->GetEnumerate());
			break;
		case DataType::Binary:
			{
				CSection::AppendData(pStorageSection, csResult);
			}
			break;
		}
		return csResult;
	}

	BYTEVector::size_type CSection::GetDataSizeRequired() const
	{
		return m_stDataSizeRequired;
	
	}
	void CSection::SetDataSizeRequired(BYTEVector::size_type stNewDataSizeRequired)
	{
		m_stDataSizeRequired = stNewDataSizeRequired;
	}

	CString CSection::GetItemCaptionString(BYTE byItemID) const
	{
		CString csResult;
		for (ItemVector::const_iterator iter = m_vpItem.cbegin(); iter != m_vpItem.cend(); ++iter)
		{
			if ((*iter)->GetItemID() == byItemID)
			{
				csResult.Format(IDS_DIAGNOSTIC_ITEMFORMAT, byItemID, (*iter)->GetCaption());
				break;
			}
		}
		return csResult;
	}

	const CItem *CSection::GetItem(ItemVector::size_type stPosition) const
	{
		if (0 <= stPosition && m_vpItem.size() > stPosition)
		{
			return m_vpItem.at(stPosition);
		}
		else
		{
			return NULL;
		}
	}

	const CItem *CSection::GetItemByID(BYTE byID) const
	{
		for (ItemVector::const_iterator iter = m_vpItem.cbegin(); iter != m_vpItem.cend(); ++iter)
		{
			if (byID == (*iter)->GetItemID())
			{
				return *iter;
			}
		}
		return NULL;
	}

	void CSection::PushItem(CItem *pItem)
	{
		m_vpItem.push_back(pItem);
	}

	void CSection::ClearItem()
	{
		for (ItemVector::const_iterator iter = m_vpItem.cbegin(); iter != m_vpItem.cend(); ++iter)
		{
			delete *iter;
		}
	}

	ItemVector::size_type CSection::GetItemCount() const
	{
		return m_vpItem.size();
	}

	BOOL CSection::IsItemEmpty() const
	{
		return m_vpItem.empty();
	}

	const ItemVector &CSection::GetItemVector() const
	{
		return m_vpItem;
	}

	const SectionVector &CSection::GetCurrentItemSectionVector(const DiagnosticStorage::CSection *pDiagnosticStorage) const
	{
		ASSERT(pDiagnosticStorage);
		ASSERT(m_dataType == DataType::Enumerate);

		if (!pDiagnosticStorage->IsDataEmpty())
		{
			for (int i = 0; i != m_vpItem.size(); ++i)
			{
				if (m_vpItem[i]->GetItemID() == pDiagnosticStorage->GetEnumerate())
				{
					return m_vpItem[i]->GetSectionVector();
				}
			}
		}
		return m_vpItem[0]->GetSectionVector();
	}

	CItem::CItem(BYTE byItemID, LPCTSTR lpszCaption, LPCTSTR lpszDescription)
		: m_byItemID(byItemID), m_csCaption(lpszCaption), m_csDescription(lpszDescription)
	{
	}
	
	CItem::CItem(BYTE byItemID, UINT nCaptionID, UINT nDescriptionID)
		: m_byItemID(byItemID)
	{
		if (0 == nCaptionID)
		{
			m_csCaption.LoadString(IDS_DIAGNOSTICSERVICE + byItemID);
		}
		else
		{
			m_csCaption.LoadString(nCaptionID);
		}
		if (0 != nDescriptionID)
		{
			m_csDescription.LoadString(nDescriptionID);
		} 
	}

	CItem::~CItem()
	{
		ClearSection();
	}

	BYTE CItem::GetItemID() const
	{
		return m_byItemID;
	}

	const CString &CItem::GetCaption() const
	{
		return m_csCaption;
	}

	const CString &CItem::GetDescription() const
	{
		return m_csDescription;
	}

	void CItem::PushSection(CSection *pSection)
	{
		ASSERT(pSection->GetDataType() != DataType::Enumerate || pSection->GetItemCount());
		m_vpSection.push_back(pSection);
	}

	void CItem::ClearSection()
	{
		while (!m_vpSection.empty())
		{
			delete m_vpSection.back(); m_vpSection.pop_back();
		}
	}

	void CItem::AppendItemData(IN const DiagnosticStorage::CItem *pStorageItem, OUT CString &csData) const
	{
		ASSERT(pStorageItem);
		csData.Append(CServiceManager::GetByteString(m_byItemID));
		for (SectionVector::const_iterator iter = m_vpSection.cbegin(); iter != m_vpSection.cend(); ++iter)
		{
			(*iter)->AppendData(pStorageItem->GetSection((*iter)->GetSectionID()), csData);
		}
	}

	BOOL CItem::AppendItemData(IN const DiagnosticStorage::CItem *pStorageItem, OUT BYTEVector &vbyData) const
	{
		ASSERT(pStorageItem);
		BOOL bAllSectionFulfilled = TRUE;
		vbyData.push_back(m_byItemID);
		for (SectionVector::const_iterator iter = m_vpSection.cbegin(); iter != m_vpSection.cend(); ++iter)
		{
			if (!(*iter)->AppendData(pStorageItem->GetSection((*iter)->GetSectionID()), vbyData) && bAllSectionFulfilled)
			{
				bAllSectionFulfilled = FALSE;
			}
		}
		return bAllSectionFulfilled;
	}

	BYTEVector::size_type CItem::GetWatchEntriesByData(const BYTEVector &vbyData, BYTEVector::size_type stLocation, CStringArray &acsEntries) const
	{
		if (!m_csDescription.IsEmpty())
		{
			acsEntries.Add(m_csDescription);
		}
		if (vbyData.size() > ++stLocation)
		{
			const CItem *pItem = NULL;
			int nDataSizeRequired = 0;
			for (SectionVector::const_iterator iter = m_vpSection.cbegin(); iter != m_vpSection.cend(); ++iter)
			{
				if (DataType::Enumerate == (*iter)->GetDataType())
				{
					if (pItem = (*iter)->GetItemByID(vbyData.at(stLocation)))
					{
						stLocation = pItem->GetWatchEntriesByData(vbyData, stLocation, acsEntries);
					}
				}
				else
				{
					nDataSizeRequired = (*iter)->GetDataSizeRequired();
					if (nDataSizeRequired)
					{
						stLocation += nDataSizeRequired;
					}
					else
					{
						return vbyData.size();
					}
				}
			}
		}
		return stLocation;
	}

	SectionVector::size_type CItem::GetSectionCount() const
	{
		return m_vpSection.size();
	}

	BOOL CItem::IsSectionEmpty() const
	{
		return m_vpSection.empty();
	}

	const CSection *CItem::GetSection(SectionVector::size_type stPosition) const
	{
		if (0 <= stPosition && m_vpSection.size() > stPosition)
		{
			return m_vpSection.at(stPosition);
		}
		else
		{
			return NULL;
		}
	}
	const SectionVector &CItem::GetSectionVector() const
	{
		return m_vpSection;
	}

	CServiceManager::CServiceManager()
	{
		_BuildDiagnosticService();
	}

	CServiceManager::~CServiceManager()
	{
		for (ItemVector::const_iterator iter = m_vpDiagnosticItem.cbegin(); iter != m_vpDiagnosticItem.cend(); ++iter)
		{
			delete *iter;
		}
		m_apDiagnosticService.release();
	}

	CServiceManager *CServiceManager::GetInstance()
	{
		if (m_apDiagnosticService.get() == 0)
		{
			m_apDiagnosticService.reset(new CServiceManager());
		}
		return m_apDiagnosticService.get();
	}

	void CServiceManager::BuildDiagnosticStorage(DiagnosticStorage::CSection *pStorageSection) const
	{
		ASSERT(pStorageSection);
		pStorageSection->Clear();
		_BuildDiagnosticStorage(m_vpDiagnosticItem, pStorageSection);
		pStorageSection->Sort();
	}

	void CServiceManager::_BuildDiagnosticStorage(const ItemVector &vpItem, DiagnosticStorage::CSection *pSectionRootStorage) const
	{
		for (ItemVector::const_iterator iterItem /* 各列表 */ = vpItem.cbegin(); iterItem != vpItem.cend(); ++iterItem)
		{
			if ((*iterItem)->GetSectionCount())
			{
				DiagnosticStorage::CItem *pItemStorage = pSectionRootStorage->AddItem((*iterItem)->GetItemID());
				pItemStorage->SetAssociatedDiagnosticServiceItem(*iterItem);
				const SectionVector &sectionVector = (*iterItem)->GetSectionVector();
				for (SectionVector::const_iterator iterSection /* 各项 */ = sectionVector.cbegin(); iterSection != sectionVector.cend(); ++iterSection)
				{
					DiagnosticStorage::CSection *pSectionStorage = pItemStorage->AddSection((*iterSection)->GetSectionID());
					pSectionStorage->SetAssociatedDiagnosticServiceSection(*iterSection);
					_BuildDiagnosticStorage((*iterSection)->GetItemVector(), pSectionStorage);
				}
			}
		}
	}

	CString CServiceManager::GetByteString(BYTE byValue)
	{
		CString csResult;
		csResult.Format(IDS_DIAGNOSTIC_BYTEFORMAT, byValue);
		return csResult;
	}

	CString CServiceManager::GetByteString(const BYTEVector &vbyValue)
	{
		CString csResult;
		for (BYTEVector::const_iterator iter = vbyValue.cbegin(); iter != vbyValue.cend(); ++iter)
		{
			if (iter != vbyValue.cbegin())
			{
				csResult.AppendChar(_T(' '));
			}
			csResult.Append(GetByteString(*iter));
		}
		return csResult;
	}

	void CServiceManager::GetWatchEntriesByData(const BYTEVector &vbyData, CStringArray &acsEntries) const
	{
		if (vbyData.empty())
		{
			return;
		}
		const CItem *pItem = NULL;
		if (pItem = GetItem(vbyData.at(0)))
		{
			pItem->GetWatchEntriesByData(vbyData, 0, acsEntries);
		}
	}

	const CItem *CServiceManager::GetItem(BYTE byDiagnosticServiceID) const
	{
		ItemVector::size_type stLocation = FindItem(byDiagnosticServiceID);
		if (stLocation != m_vpDiagnosticItem.size())
		{
			return m_vpDiagnosticItem.at(FindItem(byDiagnosticServiceID));
		}
		else
		{
			return NULL;
		}
	}

	const ItemVector &CServiceManager::GetItemVector() const
	{
		return m_vpDiagnosticItem;
	}

	void CServiceManager::SetItem(CItem *pItem) 
	{
		ItemVector::size_type stLocation = FindItem(pItem->GetItemID());
		if (stLocation == m_vpDiagnosticItem.size())
		{
			m_vpDiagnosticItem.push_back(pItem);
		}
		else
		{
			delete m_vpDiagnosticItem.at(stLocation);
			m_vpDiagnosticItem.at(stLocation) = pItem;
		}
	}

	ItemVector::size_type CServiceManager::FindItem(BYTE byDiagnosticServiceID) const
	{
		ItemVector::size_type stItemVector = m_vpDiagnosticItem.size();
		for (ItemVector::size_type i = 0; i != stItemVector; ++i)
		{
			if (m_vpDiagnosticItem[i]->GetItemID() == byDiagnosticServiceID)
			{
				return i;
			}
		}
		return stItemVector;
	}

	void CServiceManager::_BuildDiagnosticService()
	{
		CItem *pItem;
		CSection *pSection;
		CItem *pSubFunction;

		// DiagnosticSessionControl
		pItem = new CItem(DiagnosticSessionControl, 0, IDS_APPLICATIONLAYERWATCH_REQUESTDIAGNOSTICSESSIONCONTROL);

			pSection = new CSection(DataType::Enumerate, 0x00, IDS_DIAGNOSTICSECTION_DIAGNOSTICSESSIONTYPE);

				pSection->PushItem(new CItem(0x01, IDS_DIAGNOSTICSUBFUNCTION_DEFUALTSESSION, IDS_APPLICATIONLAYERWATCH_REQUESTDEFUALTSESSION));
				pSection->PushItem(new CItem(0x02, IDS_DIAGNOSTICSUBFUNCTION_PROGRAMMINGSESSION, IDS_APPLICATIONLAYERWATCH_REQUESTPROGRAMMINGSESSION));
				pSection->PushItem(new CItem(0x03, IDS_DIAGNOSTICSUBFUNCTION_EXTENDDIAGNOSTICSESSION, IDS_APPLICATIONLAYERWATCH_REQUESTEXTENDDIAGNOSTICSESSION));

			pItem->PushSection(pSection);

		SetItem(pItem);

		// ECUReset
		pItem = new CItem(ECUReset, 0, IDS_APPLICATIONLAYERWATCH_REQUESTECURESET);

			pSection = new CSection(DataType::Enumerate, 0x00, IDS_DIAGNOSTICSECTION_RESETTYPE);

				pSection->PushItem(new CItem(0x01, IDS_DIAGNOSTICSUBFUNCTION_HARDRESET, IDS_APPLICATIONLAYERWATCH_REQUESTHARDRESET));
				pSection->PushItem(new CItem(0x02, IDS_DIAGNOSTICSUBFUNCTION_KEYOFFONRESET, IDS_APPLICATIONLAYERWATCH_REQUESTKEYOFFONRESET));
				pSection->PushItem(new CItem(0x03, IDS_DIAGNOSTICSUBFUNCTION_SOFTRESET, IDS_APPLICATIONLAYERWATCH_REQUESTSOFTRESET));
				pSection->PushItem(new CItem(0x04, IDS_DIAGNOSTICSUBFUNCTION_ENABLERAPIDPOWERSHUTDOWN, IDS_APPLICATIONLAYERWATCH_REQUESTENABLERAPIDPOWERSHUTDOWN));
				pSection->PushItem(new CItem(0x05, IDS_DIAGNOSTICSUBFUNCTION_DISABLERAPIDPOWERSHUTDOWN, IDS_APPLICATIONLAYERWATCH_REQUESTDISABLERAPIDPOWERSHUTDOWN));

			pItem->PushSection(pSection);

		SetItem(pItem);

		// SecurityAccess
		pItem = new CItem(SecurityAccess, 0, IDS_APPLICATIONLAYERWATCH_REQUESTSECURITYACCESS);

			pItem->PushSection(new CSection(DataType::Binary, 0x00, IDS_DIAGNOSTICSECTION_SECURITYACCESSTYPE, 1));
			pItem->PushSection(new CSection(DataType::Binary, 0x01, IDS_DIAGNOSTICSERVICE_SECURITYACCESSDATARECORDORSECURITYKEY));

			SetItem(pItem);

		// CommunicationControl
		pItem = new CItem(CommunicationControl, 0, IDS_APPLICATIONLAYERWATCH_REQUESTCOMMUNICATIONCONTROL);

			pSection = new CSection(DataType::Enumerate, 0x00, IDS_DIAGNOSTICSECTION_CONTROLTYPE);

				pSection->PushItem(new CItem(0x00, IDS_DIAGNOSTICSUBFUNCTION_ENABLERXANDTX, IDS_APPLICATIONLAYERWATCH_REQUESTENABLERXANDTX));
				pSection->PushItem(new CItem(0x01, IDS_DIAGNOSTICSUBFUNCTION_ENABLERXANDDISABLETX, IDS_APPLICATIONLAYERWATCH_REQUESTENABLERXANDDISABLETX));
				pSection->PushItem(new CItem(0x02, IDS_DIAGNOSTICSUBFUNCTION_DISABLERXANDENABLETX, IDS_APPLICATIONLAYERWATCH_REQUESTDISABLERXANDENABLETX));
				pSection->PushItem(new CItem(0x03, IDS_DIAGNOSTICSUBFUNCTION_DISABLERXANDTX, IDS_APPLICATIONLAYERWATCH_REQUESTDISABLERXANDTX));

			pItem->PushSection(pSection);

			pItem->PushSection(new CSection(DataType::Binary, 0x01, IDS_DIAGNOSTICSERVICE_COMMUNICATIONTYPE, 1));

		SetItem(pItem);

		// TesterPresent
		pItem = new CItem(TesterPresent, 0, IDS_APPLICATIONLAYERWATCH_REQUESTTESTERPRESENT);

			pSection = new CSection(DataType::Enumerate, 0x00, IDS_DIAGNOSTICSECTION_ZEROSUBFUNCTION);

				pSection->PushItem(new CItem(0x00, IDS_DIAGNOSTICSUBFUNCTION_ZEROSUBFUNCTION));

			pItem->PushSection(pSection);

		SetItem(pItem);

		// AccessTimingParameter
		pItem = new CItem(AccessTimingParameter, 0, IDS_APPLICATIONLAYERWATCH_REQUESTACCESSTIMINGPARAMETER);

			pSection = new CSection(DataType::Enumerate, 0x00, IDS_DIAGNOSTICSECTION_TIMINGPARAMETERACCESSTYPE);

				pSection->PushItem(new CItem(0x01, IDS_DIAGNOSTICSUBFUNCTION_READEXTENDEDTIMINGPARAMETERSET, IDS_APPLICATIONLAYERWATCH_REQUESTREADEXTENDEDTIMINGPARAMETERSET));
				pSection->PushItem(new CItem(0x02, IDS_DIAGNOSTICSUBFUNCTION_SETTIMINGPARAMETERSTODEFAULTVALUES, IDS_APPLICATIONLAYERWATCH_REQUESTSETTIMINGPARAMETERSTODEFAULTVALUES));
				pSection->PushItem(new CItem(0x03, IDS_DIAGNOSTICSUBFUNCTION_READCURRENTLYACTIVETIMINGPARAMETERS, IDS_APPLICATIONLAYERWATCH_REQUESTREADCURRENTLYACTIVETIMINGPARAMETERS));
				pSection->PushItem(new CItem(0x04, IDS_DIAGNOSTICSUBFUNCTION_SETTIMINGPARAMETERSTOGIVENVALUES, IDS_APPLICATIONLAYERWATCH_REQUESTSETTIMINGPARAMETERSTOGIVENVALUES));

			pItem->PushSection(pSection);

			pItem->PushSection(new CSection(DataType::Binary, 0x01, IDS_DIAGNOSTICSERVICE_TIMINGPARAMETERREQUESTRECORD));

		SetItem(pItem);

		// SecuredDataTransmission
		pItem = new CItem(SecuredDataTransmission, 0, IDS_APPLICATIONLAYERWATCH_REQUESTSECUREDDATATRANSMISSION);

			pItem->PushSection(new CSection(DataType::Binary, 0x00, IDS_DIAGNOSTICSERVICE_SECURITYDATAREQUESTRECORD));

		SetItem(pItem);

		// ControlDTCSetting
		pItem = new CItem(ControlDTCSetting, 0, IDS_APPLICATIONLAYERWATCH_REQUESTCONTROLDTCSETTING);

			pSection = new CSection(DataType::Enumerate, 0x00, IDS_DIAGNOSTICSECTION_DTCSETTINGTYPE);

				pSection->PushItem(new CItem(0x01, IDS_DIAGNOSTICSUBFUNCTION_ON, IDS_APPLICATIONLAYERWATCH_REQUESTON));
				pSection->PushItem(new CItem(0x02, IDS_DIAGNOSTICSUBFUNCTION_OFF, IDS_APPLICATIONLAYERWATCH_REQUESTOFF));

			pItem->PushSection(pSection);

			pItem->PushSection(new CSection(DataType::Binary, 0x01, IDS_DIAGNOSTICSERVICE_TIMINGPARAMETERREQUESTRECORD));

		SetItem(pItem);
	
		// ResponseOnEvent
		// ***** CAUTION *****
		pItem = new CItem(ResponseOnEvent, 0, IDS_APPLICATIONLAYERWATCH_REQUESTRESPONSEONEVENT);

			// TODO: 分栏，如下列注释所示。
			pSection = new CSection(DataType::Enumerate, 0x00, IDS_DIAGNOSTICSECTION_STORAGESTATE);

				pSection->PushItem(new CItem(0x00, IDS_DIAGNOSTICSUBFUNCTION_DONOTSTOREEVENT_STOPRESPONSEONEVENT, IDS_APPLICATIONLAYERWATCH_REQUESTDONOTSTOREEVENT_STOPRESPONSEONEVENT));
				pSection->PushItem(new CItem(0x01, IDS_DIAGNOSTICSUBFUNCTION_DONOTSTOREEVENT_ONDTCSTATUSCHANGE, IDS_APPLICATIONLAYERWATCH_REQUESTDONOTSTOREEVENT_ONDTCSTATUSCHANGE));
				pSection->PushItem(new CItem(0x02, IDS_DIAGNOSTICSUBFUNCTION_DONOTSTOREEVENT_ONTIMERINTERRUPT, IDS_APPLICATIONLAYERWATCH_REQUESTDONOTSTOREEVENT_ONTIMERINTERRUPT));
				pSection->PushItem(new CItem(0x03, IDS_DIAGNOSTICSUBFUNCTION_DONOTSTOREEVENT_ONCHANGEOFDATAIDENTIFIER, IDS_APPLICATIONLAYERWATCH_REQUESTDONOTSTOREEVENT_ONCHANGEOFDATAIDENTIFIER));
				pSection->PushItem(new CItem(0x04, IDS_DIAGNOSTICSUBFUNCTION_DONOTSTOREEVENT_REPORTACTIVATEDEVENTS, IDS_APPLICATIONLAYERWATCH_REQUESTDONOTSTOREEVENT_REPORTACTIVATEDEVENTS));
				pSection->PushItem(new CItem(0x05, IDS_DIAGNOSTICSUBFUNCTION_DONOTSTOREEVENT_STARTRESPONSEONEVENT, IDS_APPLICATIONLAYERWATCH_REQUESTDONOTSTOREEVENT_STARTRESPONSEONEVENT));
				pSection->PushItem(new CItem(0x06, IDS_DIAGNOSTICSUBFUNCTION_DONOTSTOREEVENT_CLEARRESPONSEONEVENT, IDS_APPLICATIONLAYERWATCH_REQUESTDONOTSTOREEVENT_CLEARRESPONSEONEVENT));
				pSection->PushItem(new CItem(0x07, IDS_DIAGNOSTICSUBFUNCTION_DONOTSTOREEVENT_ONCOMPARISONOFVALUES, IDS_APPLICATIONLAYERWATCH_REQUESTDONOTSTOREEVENT_ONCOMPARISONOFVALUES));
				pSection->PushItem(new CItem(0x80, IDS_DIAGNOSTICSUBFUNCTION_STOREEVENT_STOPRESPONSEONEVENT, IDS_APPLICATIONLAYERWATCH_REQUESTSTOREEVENT_STOPRESPONSEONEVENT));
				pSection->PushItem(new CItem(0x81, IDS_DIAGNOSTICSUBFUNCTION_STOREEVENT_ONDTCSTATUSCHANGE, IDS_APPLICATIONLAYERWATCH_REQUESTSTOREEVENT_ONDTCSTATUSCHANGE));
				pSection->PushItem(new CItem(0x82, IDS_DIAGNOSTICSUBFUNCTION_STOREEVENT_ONTIMERINTERRUPT, IDS_APPLICATIONLAYERWATCH_REQUESTSTOREEVENT_ONTIMERINTERRUPT));
				pSection->PushItem(new CItem(0x83, IDS_DIAGNOSTICSUBFUNCTION_STOREEVENT_ONCHANGEOFDATAIDENTIFIER, IDS_APPLICATIONLAYERWATCH_REQUESTSTOREEVENT_ONCHANGEOFDATAIDENTIFIER));
				pSection->PushItem(new CItem(0x84, IDS_DIAGNOSTICSUBFUNCTION_STOREEVENT_REPORTACTIVATEDEVENTS, IDS_APPLICATIONLAYERWATCH_REQUESTSTOREEVENT_REPORTACTIVATEDEVENTS));
				pSection->PushItem(new CItem(0x85, IDS_DIAGNOSTICSUBFUNCTION_STOREEVENT_STARTRESPONSEONEVENT, IDS_APPLICATIONLAYERWATCH_REQUESTSTOREEVENT_STARTRESPONSEONEVENT));
				pSection->PushItem(new CItem(0x86, IDS_DIAGNOSTICSUBFUNCTION_STOREEVENT_CLEARRESPONSEONEVENT, IDS_APPLICATIONLAYERWATCH_REQUESTSTOREEVENT_CLEARRESPONSEONEVENT));
				pSection->PushItem(new CItem(0x87, IDS_DIAGNOSTICSUBFUNCTION_STOREEVENT_ONCOMPARISONOFVALUES, IDS_APPLICATIONLAYERWATCH_REQUESTSTOREEVENT_ONCOMPARISONOFVALUES));

			pItem->PushSection(pSection);

			//pSection = new CSection(DataType::Enumerate, 0x00, IDS_DIAGNOSTICSECTION_STORAGESTATE);

			//	pSection->PushItem(new CItem(0x00, IDS_DIAGNOSTICSUBFUNCTION_DONOTSTOREEVENT));
			//	pSection->PushItem(new CItem(0x01, IDS_DIAGNOSTICSUBFUNCTION_STOREEVENT));

			//pItem->PushSection(pSection);

			//pSection = new CSection(DataType::Enumerate, 0x01, IDS_DIAGNOSTICSECTION_EVENTTYPE);

			//	pSection->PushItem(new CItem(0x00, IDS_DIAGNOSTICSUBFUNCTION_STOPRESPONSEONEVENT));
			//	pSection->PushItem(new CItem(0x01, IDS_DIAGNOSTICSUBFUNCTION_ONDTCSTATUSCHANGE));
			//	pSection->PushItem(new CItem(0x02, IDS_DIAGNOSTICSUBFUNCTION_ONTIMERINTERRUPT));
			//	pSection->PushItem(new CItem(0x03, IDS_DIAGNOSTICSUBFUNCTION_ONCHANGEOFDATAIDENTIFIER));
			//	pSection->PushItem(new CItem(0x04, IDS_DIAGNOSTICSUBFUNCTION_REPORTACTIVATEDEVENTS));
			//	pSection->PushItem(new CItem(0x05, IDS_DIAGNOSTICSUBFUNCTION_STARTRESPONSEONEVENT));
			//	pSection->PushItem(new CItem(0x06, IDS_DIAGNOSTICSUBFUNCTION_CLEARRESPONSEONEVENT));
			//	pSection->PushItem(new CItem(0x07, IDS_DIAGNOSTICSUBFUNCTION_ONCOMPARISONOFVALUES));

			//pItem->PushSection(pSection);

			pItem->PushSection(new CSection(DataType::Binary, 0x01, IDS_DIAGNOSTICSERVICE_EVENTWINDOWTIME, 1));

			pItem->PushSection(new CSection(DataType::Binary, 0x02, IDS_DIAGNOSTICSERVICE_EVENTTYPERECORD));

			pItem->PushSection(new CSection(DataType::Binary, 0x03, IDS_DIAGNOSTICSERVICE_SERVICETORESPONDTORECORD));

		SetItem(pItem);

		// LinkControl
		pItem = new CItem(LinkControl, 0, IDS_APPLICATIONLAYERWATCH_REQUESTLINKCONTROL);

			pSection = new CSection(DataType::Enumerate, 0x00, IDS_DIAGNOSTICSECTION_LINKCONTROLTYPE);
		
				pSubFunction = new CItem(0x01, IDS_DIAGNOSTICSUBFUNCTION_VERIFYBAUDRATETRANSITIONWITHFIXEDBAUDRATE, IDS_APPLICATIONLAYERWATCH_REQUESTVERIFYBAUDRATETRANSITIONWITHFIXEDBAUDRATE);
					pSubFunction->PushSection(new CSection(DataType::Binary, 0x00, IDS_DIAGNOSTICSUBFUNCTIONSECTION_BAUDRATEIDENTIFIER, 1));
				pSection->PushItem(pSubFunction);

				pSubFunction = new CItem(0x02, IDS_DIAGNOSTICSUBFUNCTION_VERIFYBAUDRATETRANSITIONWITHSPECIFICBAUDRATE, IDS_APPLICATIONLAYERWATCH_REQUESTVERIFYBAUDRATETRANSITIONWITHSPECIFICBAUDRATE);
					pSubFunction->PushSection(new CSection(DataType::Binary, 0x00, IDS_DIAGNOSTICSUBFUNCTIONSECTION_LINKBAUDRATERECORD, 3));
				pSection->PushItem(pSubFunction);

				pSection->PushItem(new CItem(0x03, IDS_DIAGNOSTICSUBFUNCTION_TRANSITIONBAUDRATE, IDS_APPLICATIONLAYERWATCH_REQUESTTRANSITIONBAUDRATE));

			pItem->PushSection(pSection);

		SetItem(pItem);

		// ReadDataByIdentifier
		pItem = new CItem(ReadDataByIdentifier, 0, IDS_APPLICATIONLAYERWATCH_REQUESTREADDATABYIDENTIFIER);

			pItem->PushSection(new CSection(DataType::Binary, 0x00, IDS_DIAGNOSTICSECTION_DATAIDENTIFIER));

		SetItem(pItem);

		// ReadMemoryByAddress
		pItem = new CItem(ReadMemoryByAddress, 0, IDS_APPLICATIONLAYERWATCH_REQUESTREADMEMORYBYADDRESS);

			pItem->PushSection(new CSection(DataType::Binary, 0x00, IDS_DIAGNOSTICSECTION_ADDRESSANDLENGTHFORMATIDENTIFIER, 1));
			pItem->PushSection(new CSection(DataType::Binary, 0x01, IDS_DIAGNOSTICSECTION_MEMORYADDRESS, 15));
			pItem->PushSection(new CSection(DataType::Binary, 0x02, IDS_DIAGNOSTICSECTION_MEMORYSIZE, 15));

		SetItem(pItem);

		// ReadScalingDataByIdentifier
		pItem = new CItem(ReadScalingDataByIdentifier, 0, IDS_APPLICATIONLAYERWATCH_REQUESTREADSCALINGDATABYIDENTIFIER);

			pItem->PushSection(new CSection(DataType::Binary, 0x00, IDS_DIAGNOSTICSECTION_DATAIDENTIFIER, 2));

		SetItem(pItem);

		// ReadDataByPeriodicIdentifier
		pItem = new CItem(ReadDataByPeriodicIdentifier, 0, IDS_APPLICATIONLAYERWATCH_REQUESTREADDATABYPERIODICIDENTIFIER);

			pItem->PushSection(new CSection(DataType::Binary, 0x00, IDS_DIAGNOSTICSECTION_TRANSMISSIONMODE, 1));
			pItem->PushSection(new CSection(DataType::Binary, 0x01, IDS_DIAGNOSTICSECTION_PERIODICDATAIDENTIFIER, 0));

		SetItem(pItem);

		// DynamicallyDefineDataIdentifier
		pItem = new CItem(DynamicallyDefineDataIdentifier, 0, IDS_APPLICATIONLAYERWATCH_REQUESTDYNAMICALLYDEFINEDATAIDENTIFIER);

			pSection = new CSection(DataType::Enumerate, 0x00, IDS_DIAGNOSTICSECTION_SUBFUNCTION);

				pSection->PushItem(new CItem(0x01, IDS_DIAGNOSTICSUBFUNCTION_DEFINEBYIDENTIFIER, IDS_APPLICATIONLAYERWATCH_REQUESTDEFINEBYIDENTIFIER));
				pSection->PushItem(new CItem(0x02, IDS_DIAGNOSTICSUBFUNCTION_DEFINEBYMEMORYADDRESS, IDS_APPLICATIONLAYERWATCH_REQUESTDEFINEBYMEMORYADDRESS));
				pSection->PushItem(new CItem(0x03, IDS_DIAGNOSTICSUBFUNCTION_CLEARDYNAMICALLYDEFINEDDATAIDENTIFIER, IDS_APPLICATIONLAYERWATCH_REQUESTCLEARDYNAMICALLYDEFINEDDATAIDENTIFIER));

			pItem->PushSection(pSection);

			pItem->PushSection(new CSection(DataType::Binary, 0x01, IDS_DIAGNOSTICSECTION_DATA, 0));

		SetItem(pItem);

		// WriteDataByIdentifier
		pItem = new CItem(WriteDataByIdentifier, 0, IDS_APPLICATIONLAYERWATCH_REQUESTWRITEDATABYIDENTIFIER);

			pItem->PushSection(new CSection(DataType::Binary, 0x00, IDS_DIAGNOSTICSECTION_DATAIDENTIFIER, 2));
			pItem->PushSection(new CSection(DataType::Binary, 0x01, IDS_DIAGNOSTICSECTION_DATARECORD, 0));

		SetItem(pItem);

		// WriteMemoryByAddress
		pItem = new CItem(WriteMemoryByAddress, 0, IDS_APPLICATIONLAYERWATCH_REQUESTWRITEMEMORYBYADDRESS);

			pItem->PushSection(new CSection(DataType::Binary, 0x00, IDS_DIAGNOSTICSECTION_ADDRESSANDLENGTHFORMATIDENTIFIER, 1));
			pItem->PushSection(new CSection(DataType::Binary, 0x01, IDS_DIAGNOSTICSECTION_MEMORYADDRESS, 15));
			pItem->PushSection(new CSection(DataType::Binary, 0x02, IDS_DIAGNOSTICSECTION_MEMORYSIZE, 15));
			pItem->PushSection(new CSection(DataType::Binary, 0x03, IDS_DIAGNOSTICSECTION_DATARECORD));

		SetItem(pItem);

		// ClearDiagnosticInformation
		pItem = new CItem(ClearDiagnosticInformation, 0, IDS_APPLICATIONLAYERWATCH_REQUESTCLEARDIAGNOSTICINFORMATION);

			pItem->PushSection(new CSection(DataType::Binary, 0x00, IDS_DIAGNOSTICSECTION_GROUPOFDTC, 3));

		SetItem(pItem);

		// ReadDTCInformation
		pItem = new CItem(ReadDTCInformation, 0, IDS_APPLICATIONLAYERWATCH_REQUESTREADDTCINFORMATION);

			pSection = new CSection(DataType::Enumerate, 0x00, IDS_DIAGNOSTICSECTION_SUBFUNCTION);
		
				pSubFunction = new CItem(0x01, IDS_DIAGNOSTICSUBFUNCTION_REPORTNUMBEROFDTCBYSTATUSMASK, IDS_APPLICATIONLAYERWATCH_REQUESTREPORTNUMBEROFDTCBYSTATUSMASK);
					pSubFunction->PushSection(new CSection(DataType::Binary, 0x00, IDS_DIAGNOSTICSUBFUNCTIONSECTION_DTCSTATUSMASK, 1));
				pSection->PushItem(pSubFunction);

				pSubFunction = new CItem(0x02, IDS_DIAGNOSTICSUBFUNCTION_REPORTDTCBYSTATUSMASK, IDS_APPLICATIONLAYERWATCH_REQUESTREPORTDTCBYSTATUSMASK);
					pSubFunction->PushSection(new CSection(DataType::Binary, 0x00, IDS_DIAGNOSTICSUBFUNCTIONSECTION_DTCSTATUSMASK, 1));
				pSection->PushItem(pSubFunction);

				pSubFunction = new CItem(0x03, IDS_DIAGNOSTICSUBFUNCTION_REPORTDTCSNAPSHOTIDENTIFICATION, IDS_APPLICATIONLAYERWATCH_REQUESTREPORTDTCSNAPSHOTIDENTIFICATION);
					pSubFunction->PushSection(new CSection(DataType::Binary, 0x00, IDS_DIAGNOSTICSUBFUNCTIONSECTION_DTCMASKRECORD, 3));
					pSubFunction->PushSection(new CSection(DataType::Binary, 0x01, IDS_DIAGNOSTICSUBFUNCTIONSECTION_DTCSNAPSHOTRECORDNUMBER, 1));
				pSection->PushItem(pSubFunction);

				pSubFunction = new CItem(0x04, IDS_DIAGNOSTICSUBFUNCTION_REPORTDTCSNAPSHOTRECORDBYDTCNUMBER, IDS_APPLICATIONLAYERWATCH_REQUESTREPORTDTCSNAPSHOTRECORDBYDTCNUMBER);
					pSubFunction->PushSection(new CSection(DataType::Binary, 0x00, IDS_DIAGNOSTICSUBFUNCTIONSECTION_DTCMASKRECORD, 3));
					pSubFunction->PushSection(new CSection(DataType::Binary, 0x01, IDS_DIAGNOSTICSUBFUNCTIONSECTION_DTCSNAPSHOTRECORDNUMBER, 1));
				pSection->PushItem(pSubFunction);

				pSubFunction = new CItem(0x05, IDS_DIAGNOSTICSUBFUNCTION_REPORTDTCSNAPSHOTRECORDBYRECORDNUMBER, IDS_APPLICATIONLAYERWATCH_REQUESTREPORTDTCSNAPSHOTRECORDBYRECORDNUMBER);
					pSubFunction->PushSection(new CSection(DataType::Binary, 0x00, IDS_DIAGNOSTICSUBFUNCTIONSECTION_DTCSNAPSHOTRECORDNUMBER, 1));
				pSection->PushItem(pSubFunction);

				pSubFunction = new CItem(0x06, IDS_DIAGNOSTICSUBFUNCTION_REPORTDTCEXTENDEDDATARECORDBYDTCNUMBER, IDS_APPLICATIONLAYERWATCH_REQUESTREPORTDTCEXTENDEDDATARECORDBYDTCNUMBER);
					pSubFunction->PushSection(new CSection(DataType::Binary, 0x00, IDS_DIAGNOSTICSUBFUNCTIONSECTION_DTCMASKRECORD, 3));
					pSubFunction->PushSection(new CSection(DataType::Binary, 0x01, IDS_DIAGNOSTICSUBFUNCTIONSECTION_DTCEXTENDEDDATARECORDNUMBER, 1));
				pSection->PushItem(pSubFunction);

				pSubFunction = new CItem(0x07, IDS_DIAGNOSTICSUBFUNCTION_REPORTNUMBEROFDTCBYSEVERITYMASKRECORD, IDS_APPLICATIONLAYERWATCH_REQUESTREPORTNUMBEROFDTCBYSEVERITYMASKRECORD);
					pSubFunction->PushSection(new CSection(DataType::Binary, 0x00, IDS_DIAGNOSTICSUBFUNCTIONSECTION_DTCSEVERITYMASK, 1));
					pSubFunction->PushSection(new CSection(DataType::Binary, 0x01, IDS_DIAGNOSTICSUBFUNCTIONSECTION_DTCSTATUSMASK, 1));
				pSection->PushItem(pSubFunction);

				pSubFunction = new CItem(0x08, IDS_DIAGNOSTICSUBFUNCTION_REPORTDTCBYSEVERITYMASKRECORD, IDS_APPLICATIONLAYERWATCH_REQUESTREPORTDTCBYSEVERITYMASKRECORD);
					pSubFunction->PushSection(new CSection(DataType::Binary, 0x00, IDS_DIAGNOSTICSUBFUNCTIONSECTION_DTCSEVERITYMASK, 1));
					pSubFunction->PushSection(new CSection(DataType::Binary, 0x01, IDS_DIAGNOSTICSUBFUNCTIONSECTION_DTCSTATUSMASK, 1));
				pSection->PushItem(pSubFunction);

				pSubFunction = new CItem(0x09, IDS_DIAGNOSTICSUBFUNCTION_REPORTSEVERITYINFORMATIONOFDTC, IDS_APPLICATIONLAYERWATCH_REQUESTREPORTSEVERITYINFORMATIONOFDTC);
					pSubFunction->PushSection(new CSection(DataType::Binary, 0x00, IDS_DIAGNOSTICSUBFUNCTIONSECTION_DTCMASKRECORD, 3));
				pSection->PushItem(pSubFunction);

				pSubFunction = new CItem(0x0A, IDS_DIAGNOSTICSUBFUNCTION_REPORTSUPPORTEDDTC, IDS_APPLICATIONLAYERWATCH_REQUESTREPORTSUPPORTEDDTC);
				pSection->PushItem(pSubFunction);

				pSubFunction = new CItem(0x0B, IDS_DIAGNOSTICSUBFUNCTION_REPORTFIRSTTESTFAILEDDTC, IDS_APPLICATIONLAYERWATCH_REQUESTREPORTFIRSTTESTFAILEDDTC);
				pSection->PushItem(pSubFunction);

				pSubFunction = new CItem(0x0C, IDS_DIAGNOSTICSUBFUNCTION_REPORTFIRSTCONFIRMEDDTC, IDS_APPLICATIONLAYERWATCH_REQUESTREPORTFIRSTCONFIRMEDDTC);
				pSection->PushItem(pSubFunction);

				pSubFunction = new CItem(0x0D, IDS_DIAGNOSTICSUBFUNCTION_REPORTMOSTRECENTTESTFAILEDDTC, IDS_APPLICATIONLAYERWATCH_REQUESTREPORTMOSTRECENTTESTFAILEDDTC);
				pSection->PushItem(pSubFunction);

				pSubFunction = new CItem(0x0E, IDS_DIAGNOSTICSUBFUNCTION_REPORTMOSTRECENTCONFIRMEDDTC, IDS_APPLICATIONLAYERWATCH_REQUESTREPORTMOSTRECENTCONFIRMEDDTC);
				pSection->PushItem(pSubFunction);

				pSubFunction = new CItem(0x0F, IDS_DIAGNOSTICSUBFUNCTION_REPORTMIRRORMEMORYDTCBYSTATUSMASK, IDS_APPLICATIONLAYERWATCH_REQUESTREPORTMIRRORMEMORYDTCBYSTATUSMASK);
					pSubFunction->PushSection(new CSection(DataType::Binary, 0x00, IDS_DIAGNOSTICSUBFUNCTIONSECTION_DTCSTATUSMASK, 1));
				pSection->PushItem(pSubFunction);

				pSubFunction = new CItem(0x10, IDS_DIAGNOSTICSUBFUNCTION_REPORTMIRRORMEMORYDTCEXTENDEDDATARECORDBYDTCNUMBER, IDS_APPLICATIONLAYERWATCH_REQUESTREPORTMIRRORMEMORYDTCEXTENDEDDATARECORDBYDTCNUMBER);
					pSubFunction->PushSection(new CSection(DataType::Binary, 0x00, IDS_DIAGNOSTICSUBFUNCTIONSECTION_DTCMASKRECORD, 3));
					pSubFunction->PushSection(new CSection(DataType::Binary, 0x01, IDS_DIAGNOSTICSUBFUNCTIONSECTION_DTCEXTENDEDDATARECORDNUMBER, 1));
				pSection->PushItem(pSubFunction);

				pSubFunction = new CItem(0x11, IDS_DIAGNOSTICSUBFUNCTION_REPORTNUMBEROFMIRRORMEMORYDTCBYSTATUSMASK, IDS_APPLICATIONLAYERWATCH_REQUESTREPORTNUMBEROFMIRRORMEMORYDTCBYSTATUSMASK);
					pSubFunction->PushSection(new CSection(DataType::Binary, 0x00, IDS_DIAGNOSTICSUBFUNCTIONSECTION_DTCSTATUSMASK, 1));
				pSection->PushItem(pSubFunction);

				pSubFunction = new CItem(0x12, IDS_DIAGNOSTICSUBFUNCTION_REPORTNUMBEROFEMISSIONSRELATEDOBDDTCBYSTATUSMASK, IDS_APPLICATIONLAYERWATCH_REQUESTREPORTNUMBEROFEMISSIONSRELATEDOBDDTCBYSTATUSMASK);
					pSubFunction->PushSection(new CSection(DataType::Binary, 0x00, IDS_DIAGNOSTICSUBFUNCTIONSECTION_DTCSTATUSMASK, 1));
				pSection->PushItem(pSubFunction);

				pSubFunction = new CItem(0x13, IDS_DIAGNOSTICSUBFUNCTION_REPORTEMISSIONSRELATEDOBDDTCBYSTATUSMASK, IDS_APPLICATIONLAYERWATCH_REQUESTREPORTEMISSIONSRELATEDOBDDTCBYSTATUSMASK);
					pSubFunction->PushSection(new CSection(DataType::Binary, 0x00, IDS_DIAGNOSTICSUBFUNCTIONSECTION_DTCSTATUSMASK, 1));
				pSection->PushItem(pSubFunction);

				pSubFunction = new CItem(0x14, IDS_DIAGNOSTICSUBFUNCTION_REPORTDTCFAULTDETECTIONCOUNTER, IDS_APPLICATIONLAYERWATCH_REQUESTREPORTDTCFAULTDETECTIONCOUNTER);
				pSection->PushItem(pSubFunction);

				pSubFunction = new CItem(0x15, IDS_DIAGNOSTICSUBFUNCTION_REPORTDTCWITHPERMANENTSTATUS, IDS_APPLICATIONLAYERWATCH_REQUESTREPORTDTCWITHPERMANENTSTATUS);
				pSection->PushItem(pSubFunction);
			pItem->PushSection(pSection);

		SetItem(pItem);

		// InputOutputControlByIdentifier
		pItem = new CItem(InputOutputControlByIdentifier, 0, IDS_APPLICATIONLAYERWATCH_REQUESTINPUTOUTPUTCONTROLBYIDENTIFIER);

			pItem->PushSection(new CSection(DataType::Binary, 0x00, IDS_DIAGNOSTICSECTION_DATAIDENTIFIER, 2));
			pItem->PushSection(new CSection(DataType::Binary, 0x01, IDS_DIAGNOSTICSECTION_CONTROLSTATUSRECORD));

		SetItem(pItem);

		// RoutineControl
		pItem = new CItem(RoutineControl, 0, IDS_APPLICATIONLAYERWATCH_REQUESTROUTINECONTROL);

			pSection = new CSection(DataType::Enumerate, 0x00, IDS_DIAGNOSTICSECTION_ROUTINECONTROLTYPE);

				pSection->PushItem(new CItem(0x01, IDS_DIAGNOSTICSUBFUNCTION_STARTROUTINE, IDS_APPLICATIONLAYERWATCH_REQUESTSTARTROUTINE));
				pSection->PushItem(new CItem(0x02, IDS_DIAGNOSTICSUBFUNCTION_STOPROUTINE, IDS_APPLICATIONLAYERWATCH_REQUESTSTOPROUTINE));
				pSection->PushItem(new CItem(0x03, IDS_DIAGNOSTICSUBFUNCTION_REQUESTROUTINERESULTS, IDS_APPLICATIONLAYERWATCH_REQUESTREQUESTROUTINERESULTS));

			pItem->PushSection(pSection);

			pItem->PushSection(new CSection(DataType::Binary, 0x01, IDS_DIAGNOSTICSECTION_ROUTINEIDENTIFIER, 2));
			pItem->PushSection(new CSection(DataType::Binary, 0x02, IDS_DIAGNOSTICSECTION_ROUTINECONTROLOPTIONRECORD));

		SetItem(pItem);

		// RequestDownload
		pItem = new CItem(RequestDownload, 0, IDS_APPLICATIONLAYERWATCH_REQUESTREQUESTDOWNLOAD);

			pItem->PushSection(new CSection(DataType::Binary, 0x00, IDS_DIAGNOSTICSECTION_DATAFORMATIDENTIFIER, 1));
			pItem->PushSection(new CSection(DataType::Binary, 0x01, IDS_DIAGNOSTICSECTION_ADDRESSANDLENGTHFORMATIDENTIFIER, 1));
			pItem->PushSection(new CSection(DataType::Binary, 0x02, IDS_DIAGNOSTICSECTION_MEMORYADDRESS));
			pItem->PushSection(new CSection(DataType::Binary, 0x03, IDS_DIAGNOSTICSECTION_MEMORYSIZE));

		SetItem(pItem);

		// RequestUpload
		pItem = new CItem(RequestUpload, 0, IDS_APPLICATIONLAYERWATCH_REQUESTREQUESTUPLOAD);

			pItem->PushSection(new CSection(DataType::Binary, 0x00, IDS_DIAGNOSTICSECTION_DATAFORMATIDENTIFIER, 1));
			pItem->PushSection(new CSection(DataType::Binary, 0x01, IDS_DIAGNOSTICSECTION_ADDRESSANDLENGTHFORMATIDENTIFIER, 1));
			pItem->PushSection(new CSection(DataType::Binary, 0x02, IDS_DIAGNOSTICSECTION_MEMORYADDRESS, 15));
			pItem->PushSection(new CSection(DataType::Binary, 0x03, IDS_DIAGNOSTICSECTION_MEMORYSIZE, 15));

		SetItem(pItem);

		// TransferData
		pItem = new CItem(TransferData, 0, IDS_APPLICATIONLAYERWATCH_REQUESTTRANSFERDATA);

			pItem->PushSection(new CSection(DataType::Binary, 0x00, IDS_DIAGNOSTICSECTION_BLOCKSEQUENCECOUNTER, 1));
			pItem->PushSection(new CSection(DataType::Binary, 0x01, IDS_DIAGNOSTICSECTION_TRANSFERREQUESTPARAMETERRECORD));

		SetItem(pItem);

		// RequestTransferExit
		pItem = new CItem(RequestTransferExit, 0, IDS_APPLICATIONLAYERWATCH_REQUESTREQUESTTRANSFEREXIT);

			pItem->PushSection(new CSection(DataType::Binary, 0x00, IDS_DIAGNOSTICSECTION_TRANSFERREQUESTPARAMETERRECORD));

		SetItem(pItem);
	}
}