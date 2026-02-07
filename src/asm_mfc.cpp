#include "asm_mfc.h"
#include <time.h>


static const _PNH _pfnUninitialized = (_PNH)-1;

//5ddf54
void* __cdecl operator new(size_t nSize)
{
	void* pResult;
	_PNH pfnNewHandler = _pfnUninitialized;

	for (;;)
	{
		pResult = malloc(nSize);

		if (pResult != NULL)
			return pResult;

		if (pfnNewHandler == _pfnUninitialized)
		{
			AFX_MODULE_THREAD_STATE* pState = AfxGetModuleThreadState();
			pfnNewHandler = pState->m_pfnNewHandler;
		}
		if (pfnNewHandler == NULL || (*pfnNewHandler)(nSize) == 0)
			break;
	}
	return pResult;
}

//5ddf90
void __cdecl operator delete(void* p)
{
	free(p);
}

//609088
const AFX_DATADEF struct CRuntimeClass CObject::classCObject =
{ "CObject", sizeof(CObject), 0xffff, NULL, NULL, NULL };

//5ddf9b
CRuntimeClass* CObject::GetRuntimeClass() const
{
	return RUNTIME_CLASS(CObject);
}

//401900
CObject::CObject()
{}

//401950
CObject::~CObject()
{}

//401970
void CObject::Serialize(CArchive&)
{}


void* PASCAL CObject::operator new(size_t, void* p)
{
	return p;
}

//401860
void PASCAL CObject::operator delete(void* p)
{
	::operator delete(p);
}

//401880
void* PASCAL CObject::operator new(size_t nSize)
{
	return ::operator new(nSize);
}

//401980
void CObject::AssertValid() const
{
}

//401990
void CObject::Dump(CDumpContext&) const
{
}

//5ddfa1
BOOL CObject::IsKindOf(const CRuntimeClass* pClass) const
{
	ASSERT(this != NULL);
	// it better be in valid memory, at least for CObject size
	ASSERT(AfxIsValidAddress(this, sizeof(CObject)));

	// simple SI case
	CRuntimeClass* pClassThis = GetRuntimeClass();
	return pClassThis->IsDerivedFrom(pClass);
}

//5de019
BOOL CObject::IsSerializable() const
{
	return (GetRuntimeClass()->m_wSchema != 0xffff);
}




CObject* CRuntimeClass::CreateObject()
{
	if (m_pfnCreateObject == NULL)
	{
		TRACE(_T("Error: Trying to create object which is not ")
			_T("DECLARE_DYNCREATE \nor DECLARE_SERIAL: %hs.\n"),
			m_lpszClassName);
		return NULL;
	}

	CObject* pObject = NULL;
	TRY
	{
		pObject = (*m_pfnCreateObject)();
	}
		END_TRY

		return pObject;
}



BOOL CRuntimeClass::IsDerivedFrom(const CRuntimeClass* pBaseClass) const
{
	ASSERT(this != NULL);
	ASSERT(AfxIsValidAddress(this, sizeof(CRuntimeClass), FALSE));
	ASSERT(pBaseClass != NULL);
	ASSERT(AfxIsValidAddress(pBaseClass, sizeof(CRuntimeClass), FALSE));

	// simple SI case
	const CRuntimeClass* pClassThis = this;
	while (pClassThis != NULL)
	{
		if (pClassThis == pBaseClass)
			return TRUE;
		pClassThis = pClassThis->m_pBaseClass;
	}
	return FALSE;       // walked to the top, no match
}






//const CObject* AFX_CDECL AfxDynamicDownCast(CRuntimeClass* pClass, const CObject* pObject)
//{
//	return (const CObject*)AfxDynamicDownCast(pClass, (CObject*)pObject);
//}
//
//CObject* AFX_CDECL AfxDynamicDownCast(CRuntimeClass* pClass, CObject* pObject)
//{
//	if (pObject != NULL && pObject->IsKindOf(pClass))
//		return pObject;
//	else
//		return NULL;
//}
///*
//AFX_CLASSINIT::AFX_CLASSINIT(register CRuntimeClass* pNewClass)
//{
//	AFX_MODULE_STATE* pModuleState = AfxGetModuleState();
//	AfxLockGlobals(CRIT_RUNTIMECLASSLIST);
//	pModuleState->m_classList.AddHead(pNewClass);
//	AfxUnlockGlobals(CRIT_RUNTIMECLASSLIST);
//}*/
//





int AFXAPI AfxLoadString(UINT nID, LPTSTR lpszBuf, UINT nMaxBuf)
{
	ASSERT(AfxIsValidAddress(lpszBuf, nMaxBuf * sizeof(TCHAR)));

	int nLen = ::LoadString(AfxGetResourceHandle(), nID, lpszBuf, nMaxBuf);
	if (nLen == 0)
		lpszBuf[0] = '\0';
	return nLen;
}

/////////////////////////////////////////////////////////////////////////////

BOOL AFXAPI AfxExtractSubString(CString& rString, LPCTSTR lpszFullString,
	int iSubString, TCHAR chSep)
{
	if (lpszFullString == NULL)
		return FALSE;

	while (iSubString--)
	{
		lpszFullString = _tcschr(lpszFullString, chSep);
		if (lpszFullString == NULL)
		{
			rString.Empty();        // return empty string as well
			return FALSE;
		}
		lpszFullString++;       // point past the separator
	}
	LPCTSTR lpchEnd = _tcschr(lpszFullString, chSep);
	int nLen = (lpchEnd == NULL) ?
		lstrlen(lpszFullString) : (int)(lpchEnd - lpszFullString);
	ASSERT(nLen >= 0);
	memcpy(rString.GetBufferSetLength(nLen), lpszFullString, nLen * sizeof(TCHAR));
	return TRUE;
}


void AFXAPI AfxFormatStrings(CString& rString, UINT nIDS, LPCTSTR const* rglpsz, int nString)
{
	TCHAR szFormat[256];
	if (!AfxLoadString(nIDS, szFormat) != 0)
	{
		TRACE1("Error: failed to load AfxFormatString string 0x%04x.\n", nIDS);
		ASSERT(FALSE);
		return;
	}
	AfxFormatStrings(rString, szFormat, rglpsz, nString);
}

void AFXAPI AfxFormatStrings(CString& rString, LPCTSTR lpszFormat, LPCTSTR const* rglpsz, int nString)
{
	// determine length of destination string
	int nTotalLen = 0;
	LPCTSTR pchSrc = lpszFormat;
	while (*pchSrc != '\0')
	{
		if (pchSrc[0] == '%' &&
			((pchSrc[1] >= '0' && pchSrc[1] <= '9') ||
				(pchSrc[1] >= 'A' && pchSrc[1] <= 'Z')))
		{
			// %A comes after %9 -- we'll need it someday
			int i;
			if (pchSrc[1] > '9')
				i = 9 + (pchSrc[1] - 'A');
			else
				i = pchSrc[1] - '1';
			pchSrc += 2;
			if (i >= nString)
				++nTotalLen;
			else if (rglpsz[i] != NULL)
				nTotalLen += lstrlen(rglpsz[i]);
		}
		else
		{
			if (_istlead(*pchSrc))
				++nTotalLen, ++pchSrc;
			++pchSrc;
			++nTotalLen;
		}
	}

	pchSrc = lpszFormat;
	LPTSTR pchDest = rString.GetBuffer(nTotalLen);
	while (*pchSrc != '\0')
	{
		if (pchSrc[0] == '%' &&
			((pchSrc[1] >= '0' && pchSrc[1] <= '9') ||
				(pchSrc[1] >= 'A' && pchSrc[1] <= 'Z')))
		{
			// %A comes after %9 -- we'll need it someday
			int i;
			if (pchSrc[1] > '9')
				i = 9 + (pchSrc[1] - 'A');
			else
				i = pchSrc[1] - '1';
			pchSrc += 2;
			if (i >= nString)
			{
				TRACE1("Error: illegal string index requested %d.\n", i);
				*pchDest++ = '?';
			}
			else if (rglpsz[i] != NULL)
			{
				lstrcpy(pchDest, rglpsz[i]);
				pchDest += lstrlen(pchDest);
			}
		}
		else
		{
			if (_istlead(*pchSrc))
				*pchDest++ = *pchSrc++; // copy first of 2 bytes
			*pchDest++ = *pchSrc++;
		}
	}
	rString.ReleaseBuffer((int)((LPCTSTR)pchDest - (LPCTSTR)rString));
	// ReleaseBuffer will assert if we went too far
}

void AFXAPI AfxFormatString1(CString& rString, UINT nIDS, LPCTSTR lpsz1)
{
	AfxFormatStrings(rString, nIDS, &lpsz1, 1);
}

void AFXAPI AfxFormatString2(CString& rString, UINT nIDS, LPCTSTR lpsz1, LPCTSTR lpsz2)
{
	LPCTSTR rglpsz[2];
	rglpsz[0] = lpsz1;
	rglpsz[1] = lpsz2;
	AfxFormatStrings(rString, nIDS, rglpsz, 2);
}






CWinApp* AFXAPI AfxGetApp()
{
	return afxCurrentWinApp;
}
HINSTANCE AFXAPI AfxGetInstanceHandle()
{
	ASSERT(afxCurrentInstanceHandle != NULL);
	return afxCurrentInstanceHandle;
}
HINSTANCE AFXAPI AfxGetResourceHandle()
{
	ASSERT(afxCurrentResourceHandle != NULL);
	return afxCurrentResourceHandle;
}
void AFXAPI AfxSetResourceHandle(HINSTANCE hInstResource)
{
	ASSERT(hInstResource != NULL); afxCurrentResourceHandle = hInstResource;
}
LPCTSTR AFXAPI AfxGetAppName()
{
	ASSERT(afxCurrentAppName != NULL); return afxCurrentAppName;
}
COleMessageFilter* AFXAPI AfxOleGetMessageFilter()
{
	ASSERT_VALID(AfxGetThread()); return AfxGetThread()->m_pMessageFilter;
}

CWnd* AFXAPI AfxGetMainWnd()
{
	return AfxGetThread() != NULL ? AfxGetThread()->GetMainWnd() : NULL;
}










CTime::CTime()
{}
CTime::CTime(time_t time)
{
	m_time = time;
}
#if !defined(_AFX_CORE_IMPL) || !defined(_AFXDLL) || defined(_DEBUG)
CTime::CTime(const CTime& timeSrc)
{
	m_time = timeSrc.m_time;
}
#endif
const CTime& CTime::operator=(const CTime& timeSrc)
{
	m_time = timeSrc.m_time; return *this;
}
const CTime& CTime::operator=(time_t t)
{
	m_time = t; return *this;
}
time_t CTime::GetTime() const
{
	return m_time;
}
int CTime::GetYear() const
{
	return (GetLocalTm(NULL)->tm_year) + 1900;
}
int CTime::GetMonth() const
{
	return GetLocalTm(NULL)->tm_mon + 1;
}
int CTime::GetDay() const
{
	return GetLocalTm(NULL)->tm_mday;
}
int CTime::GetHour() const
{
	return GetLocalTm(NULL)->tm_hour;
}
int CTime::GetMinute() const
{
	return GetLocalTm(NULL)->tm_min;
}
int CTime::GetSecond() const
{
	return GetLocalTm(NULL)->tm_sec;
}
int CTime::GetDayOfWeek() const
{
	return GetLocalTm(NULL)->tm_wday + 1;
}
/*CTimeSpan CTime::operator-(CTime time) const
{
	return CTimeSpan(m_time - time.m_time);
}
CTime CTime::operator-(CTimeSpan timeSpan) const
{
	return CTime(m_time - timeSpan.m_timeSpan);
}
CTime CTime::operator+(CTimeSpan timeSpan) const
{
	return CTime(m_time + timeSpan.m_timeSpan);
}
const CTime& CTime::operator+=(CTimeSpan timeSpan)
{
	m_time += timeSpan.m_timeSpan; return *this;
}
const CTime& CTime::operator-=(CTimeSpan timeSpan)
{
	m_time -= timeSpan.m_timeSpan; return *this;
}*/
BOOL CTime::operator==(CTime time) const
{
	return m_time == time.m_time;
}
BOOL CTime::operator!=(CTime time) const
{
	return m_time != time.m_time;
}
BOOL CTime::operator<(CTime time) const
{
	return m_time < time.m_time;
}
BOOL CTime::operator>(CTime time) const
{
	return m_time > time.m_time;
}
BOOL CTime::operator<=(CTime time) const
{
	return m_time <= time.m_time;
}
BOOL CTime::operator>=(CTime time) const
{
	return m_time >= time.m_time;
}

CTime::CTime(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec,
	int nDST)
{
	struct tm atm;
	atm.tm_sec = nSec;
	atm.tm_min = nMin;
	atm.tm_hour = nHour;
	ASSERT(nDay >= 1 && nDay <= 31);
	atm.tm_mday = nDay;
	ASSERT(nMonth >= 1 && nMonth <= 12);
	atm.tm_mon = nMonth - 1;        // tm_mon is 0 based
	ASSERT(nYear >= 1900);
	atm.tm_year = nYear - 1900;     // tm_year is 1900 based
	atm.tm_isdst = nDST;
	m_time = mktime(&atm);
	ASSERT(m_time != -1);       // indicates an illegal input time
}

CTime::CTime(WORD wDosDate, WORD wDosTime, int nDST)
{
	struct tm atm;
	atm.tm_sec = (wDosTime & ~0xFFE0) << 1;
	atm.tm_min = (wDosTime & ~0xF800) >> 5;
	atm.tm_hour = wDosTime >> 11;

	atm.tm_mday = wDosDate & ~0xFFE0;
	atm.tm_mon = ((wDosDate & ~0xFE00) >> 5) - 1;
	atm.tm_year = (wDosDate >> 9) + 80;
	atm.tm_isdst = nDST;
	m_time = mktime(&atm);
	ASSERT(m_time != -1);       // indicates an illegal input time
}

CTime::CTime(const SYSTEMTIME& sysTime, int nDST)
{
	if (sysTime.wYear < 1900)
	{
		time_t time0 = 0L;
		CTime timeT(time0);
		*this = timeT;
	}
	else
	{
		CTime timeT(
			(int)sysTime.wYear, (int)sysTime.wMonth, (int)sysTime.wDay,
			(int)sysTime.wHour, (int)sysTime.wMinute, (int)sysTime.wSecond,
			nDST);
		*this = timeT;
	}
}

CTime::CTime(const FILETIME& fileTime, int nDST)
{
	// first convert file time (UTC time) to local time
	FILETIME localTime;
	if (!FileTimeToLocalFileTime(&fileTime, &localTime))
	{
		m_time = 0;
		return;
	}

	// then convert that time to system time
	SYSTEMTIME sysTime;
	if (!FileTimeToSystemTime(&localTime, &sysTime))
	{
		m_time = 0;
		return;
	}

	// then convert the system time to a time_t (C-runtime local time)
	CTime timeT(sysTime, nDST);
	*this = timeT;
}

CTime PASCAL CTime::GetCurrentTime()
// return the current system time
{
	return CTime(::time(NULL));
}

struct tm* CTime::GetGmtTm(struct tm* ptm) const
{
	if (ptm != NULL)
	{
		*ptm = *gmtime(&m_time);
		return ptm;
	}
	else
		return gmtime(&m_time);
}

struct tm* CTime::GetLocalTm(struct tm* ptm) const
{
	if (ptm != NULL)
	{
		struct tm* ptmTemp = localtime(&m_time);
		if (ptmTemp == NULL)
			return NULL;    // indicates the m_time was not initialized!

		*ptm = *ptmTemp;
		return ptm;
	}
	else
		return localtime(&m_time);
}


CArchive& AFXAPI operator <<(CArchive& ar, CTime time)
{
	return ar << (DWORD)time.m_time;
}

CArchive& AFXAPI operator >>(CArchive& ar, CTime& rtime)
{
	return ar >> (DWORD&)rtime.m_time;
}

#define maxTimeBufferSize       128
	// Verifies will fail if the needed buffer size is too large

CString CTime::Format(LPCTSTR pFormat) const
{
	TCHAR szBuffer[maxTimeBufferSize];

	struct tm* ptmTemp = localtime(&m_time);
	if (ptmTemp == NULL ||
		!_tcsftime(szBuffer, _countof(szBuffer), pFormat, ptmTemp))
		szBuffer[0] = '\0';
	return szBuffer;
}

CString CTime::FormatGmt(LPCTSTR pFormat) const
{
	TCHAR szBuffer[maxTimeBufferSize];

	struct tm* ptmTemp = gmtime(&m_time);
	if (ptmTemp == NULL ||
		!_tcsftime(szBuffer, _countof(szBuffer), pFormat, ptmTemp))
		szBuffer[0] = '\0';
	return szBuffer;
}

CString CTime::Format(UINT nFormatID) const
{
	CString strFormat;
	VERIFY(strFormat.LoadString(nFormatID) != 0);
	return Format(strFormat);
}

CString CTime::FormatGmt(UINT nFormatID) const
{
	CString strFormat;
	VERIFY(strFormat.LoadString(nFormatID) != 0);
	return FormatGmt(strFormat);
}


void CException::Delete()
{
	if (this != NULL && m_bAutoDelete > 0) {
		delete this;
	}
}
