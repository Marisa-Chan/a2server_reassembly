#include "asm_mfc.h"
#include <time.h>
#include <stdio.h>


#define wNullTag        ((WORD)0)           // special tag indicating NULL ptrs
#define wNewClassTag    ((WORD)0xFFFF)      // special tag indicating new CRuntimeClass
#define wClassTag       ((WORD)0x8000)      // 0x8000 indicates class tag (OR'd)
#define dwBigClassTag   ((DWORD)0x80000000) // 0x8000000 indicates big class tag (OR'd)
#define wBigObjectTag   ((WORD)0x7FFF)      // 0x7FFF indicates DWORD object tag
#define nMaxMapCount    ((DWORD)0x3FFFFFFE) // 0x3FFFFFFE last valid mapCount
#define VERSIONABLE_SCHEMA  (0x80000000)


// these globals are protected by the same critical section
#define CRIT_DYNLINKLIST    0
#define CRIT_RUNTIMECLASSLIST   0
#define CRIT_OBJECTFACTORYLIST  0
#define CRIT_LOCKSHARED 0
// these globals are not protected by independent critical sections
#define CRIT_REGCLASSLIST   1
#define CRIT_WAITCURSOR     2
#define CRIT_DROPSOURCE     3
#define CRIT_DROPTARGET     4
#define CRIT_RECTTRACKER    5
#define CRIT_EDITVIEW       6
#define CRIT_WINMSGCACHE    7
#define CRIT_HALFTONEBRUSH  8
#define CRIT_SPLITTERWND    9
#define CRIT_MINIFRAMEWND   10
#define CRIT_CTLLOCKLIST    11
#define CRIT_DYNDLLLOAD     12
#define CRIT_TYPELIBCACHE   13
#define CRIT_STOCKMASK      14
#define CRIT_ODBC           15
#define CRIT_PROCESSLOCAL   16
#define CRIT_MAX    17  // Note: above plus one!

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



int AfxMessageBox(LPCTSTR lpszText, UINT nType, UINT nIDHelp)
{
	printf("AfxMessageBox: %s\n", lpszText);
	return AfxGetModuleState()->m_pCurrentWinApp->DoMessageBox(lpszText, nType, nIDHelp);
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








LRESULT CWnd::Default()
{
	// call DefWindowProc with the last message
	_AFX_THREAD_STATE* pThreadState = _afxThreadState.GetData();
	return DefWindowProc(pThreadState->m_lastSentMsg.message,
		pThreadState->m_lastSentMsg.wParam, pThreadState->m_lastSentMsg.lParam);
}







// CSize
CSize::CSize()
{ /* random filled */
}
CSize::CSize(int initCX, int initCY)
{
	cx = initCX; cy = initCY;
}
CSize::CSize(SIZE initSize)
{
	*(SIZE*)this = initSize;
}
CSize::CSize(POINT initPt)
{
	*(POINT*)this = initPt;
}
CSize::CSize(DWORD dwSize)
{
	cx = (short)LOWORD(dwSize);
	cy = (short)HIWORD(dwSize);
}
BOOL CSize::operator==(SIZE size) const
{
	return (cx == size.cx && cy == size.cy);
}
BOOL CSize::operator!=(SIZE size) const
{
	return (cx != size.cx || cy != size.cy);
}
void CSize::operator+=(SIZE size)
{
	cx += size.cx; cy += size.cy;
}
void CSize::operator-=(SIZE size)
{
	cx -= size.cx; cy -= size.cy;
}
CSize CSize::operator+(SIZE size) const
{
	return CSize(cx + size.cx, cy + size.cy);
}
CSize CSize::operator-(SIZE size) const
{
	return CSize(cx - size.cx, cy - size.cy);
}
CSize CSize::operator-() const
{
	return CSize(-cx, -cy);
}
CPoint CSize::operator+(POINT point) const
{
	return CPoint(cx + point.x, cy + point.y);
}
CPoint CSize::operator-(POINT point) const
{
	return CPoint(cx - point.x, cy - point.y);
}
CRect CSize::operator+(const RECT* lpRect) const
{
	return CRect(lpRect) + *this;
}
CRect CSize::operator-(const RECT* lpRect) const
{
	return CRect(lpRect) - *this;
}


// CPoint
CPoint::CPoint()
{ /* random filled */
}
CPoint::CPoint(int initX, int initY)
{
	x = initX; y = initY;
}
#if !defined(_AFX_CORE_IMPL) || !defined(_AFXDLL) || defined(_DEBUG)
CPoint::CPoint(POINT initPt)
{
	*(POINT*)this = initPt;
}
#endif
CPoint::CPoint(SIZE initSize)
{
	*(SIZE*)this = initSize;
}
CPoint::CPoint(DWORD dwPoint)
{
	x = (short)LOWORD(dwPoint);
	y = (short)HIWORD(dwPoint);
}
void CPoint::Offset(int xOffset, int yOffset)
{
	x += xOffset; y += yOffset;
}
void CPoint::Offset(POINT point)
{
	x += point.x; y += point.y;
}
void CPoint::Offset(SIZE size)
{
	x += size.cx; y += size.cy;
}
BOOL CPoint::operator==(POINT point) const
{
	return (x == point.x && y == point.y);
}
BOOL CPoint::operator!=(POINT point) const
{
	return (x != point.x || y != point.y);
}
void CPoint::operator+=(SIZE size)
{
	x += size.cx; y += size.cy;
}
void CPoint::operator-=(SIZE size)
{
	x -= size.cx; y -= size.cy;
}
void CPoint::operator+=(POINT point)
{
	x += point.x; y += point.y;
}
void CPoint::operator-=(POINT point)
{
	x -= point.x; y -= point.y;
}
CPoint CPoint::operator+(SIZE size) const
{
	return CPoint(x + size.cx, y + size.cy);
}
CPoint CPoint::operator-(SIZE size) const
{
	return CPoint(x - size.cx, y - size.cy);
}
CPoint CPoint::operator-() const
{
	return CPoint(-x, -y);
}
CPoint CPoint::operator+(POINT point) const
{
	return CPoint(x + point.x, y + point.y);
}
CSize CPoint::operator-(POINT point) const
{
	return CSize(x - point.x, y - point.y);
}
CRect CPoint::operator+(const RECT* lpRect) const
{
	return CRect(lpRect) + *this;
}
CRect CPoint::operator-(const RECT* lpRect) const
{
	return CRect(lpRect) - *this;
}

// CRect
CRect::CRect()
{ /* random filled */
}
CRect::CRect(int l, int t, int r, int b)
{
	left = l; top = t; right = r; bottom = b;
}
CRect::CRect(const RECT& srcRect)
{
	::CopyRect(this, &srcRect);
}
CRect::CRect(LPCRECT lpSrcRect)
{
	::CopyRect(this, lpSrcRect);
}
CRect::CRect(POINT point, SIZE size)
{
	right = (left = point.x) + size.cx; bottom = (top = point.y) + size.cy;
}
CRect::CRect(POINT topLeft, POINT bottomRight)
{
	left = topLeft.x; top = topLeft.y;
	right = bottomRight.x; bottom = bottomRight.y;
}
int CRect::Width() const
{
	return right - left;
}
int CRect::Height() const
{
	return bottom - top;
}
CSize CRect::Size() const
{
	return CSize(right - left, bottom - top);
}
CPoint& CRect::TopLeft()
{
	return *((CPoint*)this);
}
CPoint& CRect::BottomRight()
{
	return *((CPoint*)this + 1);
}
const CPoint& CRect::TopLeft() const
{
	return *((CPoint*)this);
}
const CPoint& CRect::BottomRight() const
{
	return *((CPoint*)this + 1);
}
CPoint CRect::CenterPoint() const
{
	return CPoint((left + right) / 2, (top + bottom) / 2);
}
CRect::operator LPRECT()
{
	return this;
}
CRect::operator LPCRECT() const
{
	return this;
}
BOOL CRect::IsRectEmpty() const
{
	return ::IsRectEmpty(this);
}
BOOL CRect::IsRectNull() const
{
	return (left == 0 && right == 0 && top == 0 && bottom == 0);
}
BOOL CRect::PtInRect(POINT point) const
{
	return ::PtInRect(this, point);
}
void CRect::SetRect(int x1, int y1, int x2, int y2)
{
	::SetRect(this, x1, y1, x2, y2);
}
void CRect::SetRect(POINT topLeft, POINT bottomRight)
{
	::SetRect(this, topLeft.x, topLeft.y, bottomRight.x, bottomRight.y);
}
void CRect::SetRectEmpty()
{
	::SetRectEmpty(this);
}
void CRect::CopyRect(LPCRECT lpSrcRect)
{
	::CopyRect(this, lpSrcRect);
}
BOOL CRect::EqualRect(LPCRECT lpRect) const
{
	return ::EqualRect(this, lpRect);
}
void CRect::InflateRect(int x, int y)
{
	::InflateRect(this, x, y);
}
void CRect::InflateRect(SIZE size)
{
	::InflateRect(this, size.cx, size.cy);
}
void CRect::DeflateRect(int x, int y)
{
	::InflateRect(this, -x, -y);
}
void CRect::DeflateRect(SIZE size)
{
	::InflateRect(this, -size.cx, -size.cy);
}
void CRect::OffsetRect(int x, int y)
{
	::OffsetRect(this, x, y);
}
void CRect::OffsetRect(POINT point)
{
	::OffsetRect(this, point.x, point.y);
}
void CRect::OffsetRect(SIZE size)
{
	::OffsetRect(this, size.cx, size.cy);
}
BOOL CRect::IntersectRect(LPCRECT lpRect1, LPCRECT lpRect2)
{
	return ::IntersectRect(this, lpRect1, lpRect2);
}
BOOL CRect::UnionRect(LPCRECT lpRect1, LPCRECT lpRect2)
{
	return ::UnionRect(this, lpRect1, lpRect2);
}
void CRect::operator=(const RECT& srcRect)
{
	::CopyRect(this, &srcRect);
}
BOOL CRect::operator==(const RECT& rect) const
{
	return ::EqualRect(this, &rect);
}
BOOL CRect::operator!=(const RECT& rect) const
{
	return !::EqualRect(this, &rect);
}
void CRect::operator+=(POINT point)
{
	::OffsetRect(this, point.x, point.y);
}
void CRect::operator+=(SIZE size)
{
	::OffsetRect(this, size.cx, size.cy);
}
void CRect::operator+=(LPCRECT lpRect)
{
	InflateRect(lpRect);
}
void CRect::operator-=(POINT point)
{
	::OffsetRect(this, -point.x, -point.y);
}
void CRect::operator-=(SIZE size)
{
	::OffsetRect(this, -size.cx, -size.cy);
}
void CRect::operator-=(LPCRECT lpRect)
{
	DeflateRect(lpRect);
}
void CRect::operator&=(const RECT& rect)
{
	::IntersectRect(this, this, &rect);
}
void CRect::operator|=(const RECT& rect)
{
	::UnionRect(this, this, &rect);
}
CRect CRect::operator+(POINT pt) const
{
	CRect rect(*this); ::OffsetRect(&rect, pt.x, pt.y); return rect;
}
CRect CRect::operator-(POINT pt) const
{
	CRect rect(*this); ::OffsetRect(&rect, -pt.x, -pt.y); return rect;
}
CRect CRect::operator+(SIZE size) const
{
	CRect rect(*this); ::OffsetRect(&rect, size.cx, size.cy); return rect;
}
CRect CRect::operator-(SIZE size) const
{
	CRect rect(*this); ::OffsetRect(&rect, -size.cx, -size.cy); return rect;
}
CRect CRect::operator+(LPCRECT lpRect) const
{
	CRect rect(this); rect.InflateRect(lpRect); return rect;
}
CRect CRect::operator-(LPCRECT lpRect) const
{
	CRect rect(this); rect.DeflateRect(lpRect); return rect;
}
CRect CRect::operator&(const RECT& rect2) const
{
	CRect rect; ::IntersectRect(&rect, this, &rect2);
	return rect;
}
CRect CRect::operator|(const RECT& rect2) const
{
	CRect rect; ::UnionRect(&rect, this, &rect2);
	return rect;
}
BOOL CRect::SubtractRect(LPCRECT lpRectSrc1, LPCRECT lpRectSrc2)
{
	return ::SubtractRect(this, lpRectSrc1, lpRectSrc2);
}


void CRect::NormalizeRect()
{
	int nTemp;
	if (left > right)
	{
		nTemp = left;
		left = right;
		right = nTemp;
	}
	if (top > bottom)
	{
		nTemp = top;
		top = bottom;
		bottom = nTemp;
	}
}

void CRect::InflateRect(LPCRECT lpRect)
{
	left -= lpRect->left;
	top -= lpRect->top;
	right += lpRect->right;
	bottom += lpRect->bottom;
}

void CRect::InflateRect(int l, int t, int r, int b)
{
	left -= l;
	top -= t;
	right += r;
	bottom += b;
}

void CRect::DeflateRect(LPCRECT lpRect)
{
	left += lpRect->left;
	top += lpRect->top;
	right -= lpRect->right;
	bottom -= lpRect->bottom;
}

void CRect::DeflateRect(int l, int t, int r, int b)
{
	left += l;
	top += t;
	right -= r;
	bottom -= b;
}

CRect CRect::MulDiv(int nMultiplier, int nDivisor) const
{
	return CRect(
		::MulDiv(left, nMultiplier, nDivisor),
		::MulDiv(top, nMultiplier, nDivisor),
		::MulDiv(right, nMultiplier, nDivisor),
		::MulDiv(bottom, nMultiplier, nDivisor));
}







void CArchive::CheckCount()
{
	if (m_nMapCount >= nMaxMapCount)
		AfxThrowArchiveException(CArchiveException::badIndex, m_strFileName);
}

void CArchive::WriteObject(const CObject* pOb)
{
	//printf("WriteObject %s at %x\n", !pOb ? "" : pOb->GetRuntimeClass()->m_lpszClassName, m_pFile->GetPosition() + (m_lpBufCur - m_lpBufStart));

	// object can be NULL
	ASSERT(IsStoring());    // proper direction

	DWORD nObIndex;
	ASSERT(sizeof(nObIndex) == 4);
	ASSERT(sizeof(wNullTag) == 2);
	ASSERT(sizeof(wBigObjectTag) == 2);
	ASSERT(sizeof(wNewClassTag) == 2);

	// make sure m_pStoreMap is initialized
	MapObject(NULL);

	if (pOb == NULL)
	{
		// save out null tag to represent NULL pointer
		*this << wNullTag;
	}
	else if ((nObIndex = (DWORD)(*m_pStoreMap)[(void*)pOb]) != 0)
		// assumes initialized to 0 map
	{
		// save out index of already stored object
		if (nObIndex < wBigObjectTag)
			*this << (WORD)nObIndex;
		else
		{
			*this << wBigObjectTag;
			*this << nObIndex;
		}
	}
	else
	{
		// write class of object first
		CRuntimeClass* pClassRef = pOb->GetRuntimeClass();
		WriteClass(pClassRef);

		// enter in stored object table, checking for overflow
		CheckCount();
		(*m_pStoreMap)[(void*)pOb] = (void*)m_nMapCount++;

		// cause the object to serialize itself
		((CObject*)pOb)->Serialize(*this);
	}
}

CObject* CArchive::ReadObject(const CRuntimeClass* pClassRefRequested)
{
	uint32_t pos = m_pFile->GetPosition() - m_nBufSize + (m_lpBufCur - m_lpBufStart);
	//printf("ReadObj %s at %x\n", pClassRefRequested->m_lpszClassName, pos);
	
	ASSERT(pClassRefRequested == NULL ||
		AfxIsValidAddress(pClassRefRequested, sizeof(CRuntimeClass), FALSE));
	ASSERT(IsLoading());    // proper direction
	ASSERT(wNullTag == 0);
	ASSERT((wClassTag << 16) == dwBigClassTag);
	ASSERT((wNewClassTag & wClassTag) == wClassTag);

	// attempt to load next stream as CRuntimeClass
	UINT nSchema;
	DWORD obTag;
	CRuntimeClass* pClassRef = ReadClass(pClassRefRequested, &nSchema, &obTag);

	// check to see if tag to already loaded object
	CObject* pOb;
	if (pClassRef == NULL)
	{
		if (obTag > (DWORD)m_pLoadArray->GetUpperBound())
		{
			// tag is too large for the number of objects read so far
			AfxThrowArchiveException(CArchiveException::badIndex,
				m_strFileName);
		}

		pOb = (CObject*)m_pLoadArray->GetAt(obTag);
		if (pOb != NULL && pClassRefRequested != NULL &&
			!pOb->IsKindOf(pClassRefRequested))
		{
			// loaded an object but of the wrong class
			AfxThrowArchiveException(CArchiveException::badClass,
				m_strFileName);
		}
	}
	else
	{
		// allocate a new object based on the class just acquired
		pOb = pClassRef->CreateObject();
		if (pOb == NULL)
			AfxThrowMemoryException();

		// Add to mapping array BEFORE de-serializing
		CheckCount();
		m_pLoadArray->InsertAt(m_nMapCount++, pOb);

		// Serialize the object with the schema number set in the archive
		UINT nSchemaSave = m_nObjectSchema;
		m_nObjectSchema = nSchema;
		pOb->Serialize(*this);
		m_nObjectSchema = nSchemaSave;
		ASSERT_VALID(pOb);
	}

	return pOb;
}

/////////////////////////////////////////////////////////////////////////////
// advanced versioning and back-pointer support

UINT CArchive::GetObjectSchema()
{
	UINT nResult = m_nObjectSchema;
	m_nObjectSchema = (UINT)-1; // can only be called once per Serialize
	return nResult;
}

void CArchive::MapObject(const CObject* pOb)
{
	if (IsStoring())
	{
		if (m_pStoreMap == NULL)
		{
			// initialize the storage map
			//  (use CMapPtrToPtr because it is used for HANDLE maps too)
			m_pStoreMap = new CMapPtrToPtr(m_nGrowSize);
			m_pStoreMap->InitHashTable(m_nHashSize);
			m_pStoreMap->SetAt(NULL, (void*)(DWORD)wNullTag);
			m_nMapCount = 1;
		}
		if (pOb != NULL)
		{
			CheckCount();
			(*m_pStoreMap)[(void*)pOb] = (void*)m_nMapCount++;
		}
	}
	else
	{
		if (m_pLoadArray == NULL)
		{
			// initialize the loaded object pointer array and set special values
			m_pLoadArray = new CPtrArray;
			m_pLoadArray->SetSize(1, m_nGrowSize);
			ASSERT(wNullTag == 0);
			m_pLoadArray->SetAt(wNullTag, NULL);
			m_nMapCount = 1;
		}
		if (pOb != NULL)
		{
			CheckCount();
			m_pLoadArray->InsertAt(m_nMapCount++, (void*)pOb);
		}
	}
}

void CArchive::WriteClass(const CRuntimeClass* pClassRef)
{
	ASSERT(pClassRef != NULL);
	ASSERT(IsStoring());    // proper direction

	if (pClassRef->m_wSchema == 0xFFFF)
	{
		TRACE1("Warning: Cannot call WriteClass/WriteObject for %hs.\n",
			pClassRef->m_lpszClassName);
		AfxThrowNotSupportedException();
	}

	// make sure m_pStoreMap is initialized
	MapObject(NULL);

	// write out class id of pOb, with high bit set to indicate
	// new object follows

	// ASSUME: initialized to 0 map
	DWORD nClassIndex;
	if ((nClassIndex = (DWORD)(*m_pStoreMap)[(void*)pClassRef]) != 0)
	{
		// previously seen class, write out the index tagged by high bit
		if (nClassIndex < wBigObjectTag)
			*this << (WORD)(wClassTag | nClassIndex);
		else
		{
			*this << wBigObjectTag;
			*this << (dwBigClassTag | nClassIndex);
		}
	}
	else
	{
		// store new class
		*this << wNewClassTag;
		pClassRef->Store(*this);

		// store new class reference in map, checking for overflow
		CheckCount();
		(*m_pStoreMap)[(void*)pClassRef] = (void*)m_nMapCount++;
	}
}

CRuntimeClass* CArchive::ReadClass(const CRuntimeClass* pClassRefRequested,
	UINT* pSchema, DWORD* pObTag)
{
	ASSERT(pClassRefRequested == NULL ||
		AfxIsValidAddress(pClassRefRequested, sizeof(CRuntimeClass), FALSE));
	ASSERT(IsLoading());    // proper direction

	if (pClassRefRequested != NULL && pClassRefRequested->m_wSchema == 0xFFFF)
	{
		TRACE1("Warning: Cannot call ReadClass/ReadObject for %hs.\n",
			pClassRefRequested->m_lpszClassName);
		AfxThrowNotSupportedException();
	}

	// make sure m_pLoadArray is initialized
	MapObject(NULL);

	// read object tag - if prefixed by wBigObjectTag then DWORD tag follows
	DWORD obTag;
	WORD wTag;
	*this >> wTag;
	if (wTag == wBigObjectTag)
		*this >> obTag;
	else
		obTag = ((wTag & wClassTag) << 16) | (wTag & ~wClassTag);

	// check for object tag (throw exception if expecting class tag)
	if (!(obTag & dwBigClassTag))
	{
		if (pObTag == NULL)
			AfxThrowArchiveException(CArchiveException::badIndex, m_strFileName);

		*pObTag = obTag;
		return NULL;
	}

	CRuntimeClass* pClassRef;
	UINT nSchema;
	if (wTag == wNewClassTag)
	{
		// new object follows a new class id
		if ((pClassRef = CRuntimeClass::Load(*this, &nSchema)) == NULL)
			AfxThrowArchiveException(CArchiveException::badClass, m_strFileName);

		// check nSchema against the expected schema
		if ((pClassRef->m_wSchema & ~VERSIONABLE_SCHEMA) != nSchema)
		{
			if (!(pClassRef->m_wSchema & VERSIONABLE_SCHEMA))
			{
				// schema doesn't match and not marked as VERSIONABLE_SCHEMA
				AfxThrowArchiveException(CArchiveException::badSchema,
					m_strFileName);
			}
			else
			{
				// they differ -- store the schema for later retrieval
				if (m_pSchemaMap == NULL)
					m_pSchemaMap = new CMapPtrToPtr;
				ASSERT_VALID(m_pSchemaMap);
				m_pSchemaMap->SetAt(pClassRef, (void*)nSchema);
			}
		}
		CheckCount();
		m_pLoadArray->InsertAt(m_nMapCount++, pClassRef);
	}
	else
	{
		// existing class index in obTag followed by new object
		DWORD nClassIndex = (obTag & ~dwBigClassTag);
		if (nClassIndex == 0 || nClassIndex > (DWORD)m_pLoadArray->GetUpperBound())
			AfxThrowArchiveException(CArchiveException::badIndex,
				m_strFileName);

		pClassRef = (CRuntimeClass*)m_pLoadArray->GetAt(nClassIndex);
		ASSERT(pClassRef != NULL);

		// determine schema stored against objects of this type
		void* pTemp;
		if (m_pSchemaMap != NULL && (pTemp = m_pSchemaMap->GetValueAt(pClassRef)) != NULL)
			nSchema = (UINT)pTemp;
		else
			nSchema = pClassRef->m_wSchema & ~VERSIONABLE_SCHEMA;
	}

	// check for correct derivation
	if (pClassRefRequested != NULL &&
		!pClassRef->IsDerivedFrom(pClassRefRequested))
	{
		AfxThrowArchiveException(CArchiveException::badClass, m_strFileName);
	}

	// store nSchema for later examination
	if (pSchema != NULL)
		*pSchema = nSchema;
	else
		m_nObjectSchema = nSchema;

	// store obTag for later examination
	if (pObTag != NULL)
		*pObTag = obTag;

	// return the resulting CRuntimeClass*
	return pClassRef;
}

void CArchive::SerializeClass(const CRuntimeClass* pClassRef)
{
	if (IsStoring())
		WriteClass(pClassRef);
	else
		ReadClass(pClassRef);
}

void AFXAPI AfxThrowArchiveException(int cause,
	LPCTSTR lpszArchiveName /* = NULL */)
{
	THROW(new CArchiveException(cause, lpszArchiveName));
}


IMPLEMENT_DYNAMIC(CArchiveException, CException)

CArchiveException::CArchiveException(int cause,
	LPCTSTR lpszFileName /* = NULL */)
{
	m_cause = cause; m_strFileName = lpszFileName;
}

CArchiveException::~CArchiveException()
{
}

BOOL CArchiveException::GetErrorMessage(LPTSTR lpszError, UINT nMaxError,
	PUINT pnHelpContext)
{
	ASSERT(lpszError != NULL && AfxIsValidString(lpszError, nMaxError));

	if (pnHelpContext != NULL)
		*pnHelpContext = m_cause + 0xf1b0;

	// we can use CString here; archive errors aren't caused
	// by being out of memory.

	CString strMessage;
	CString strFileName = m_strFileName;
	if (strFileName.IsEmpty())
		strFileName.LoadString(AFX_IDS_UNNAMED_FILE);
	AfxFormatString1(strMessage,
		m_cause + 0xf1b0, strFileName);
	lstrcpyn(lpszError, strMessage, nMaxError);

	return TRUE;
}



void AFXAPI AfxLockGlobals(int nLockType);
void AFXAPI AfxUnlockGlobals(int nLockType);

CRuntimeClass* PASCAL CRuntimeClass::Load(CArchive& ar, UINT* pwSchemaNum)
// loads a runtime class description
{
	WORD nLen;
	char szClassName[64];
	CRuntimeClass* pClass;

	WORD wTemp;
	ar >> wTemp; *pwSchemaNum = wTemp;
	ar >> nLen;

	if (nLen >= _countof(szClassName) ||
		ar.Read(szClassName, nLen * sizeof(char)) != nLen * sizeof(char))
	{
		return NULL;
	}
	szClassName[nLen] = '\0';

	// search app specific classes
	AFX_MODULE_STATE* pModuleState = AfxGetModuleState();
	AfxLockGlobals(CRIT_RUNTIMECLASSLIST);
	for (pClass = pModuleState->m_classList; pClass != NULL;
		pClass = pClass->m_pNextClass)
	{
		if (lstrcmpA(szClassName, pClass->m_lpszClassName) == 0)
		{
			AfxUnlockGlobals(CRIT_RUNTIMECLASSLIST);
			return pClass;
		}
	}
	AfxUnlockGlobals(CRIT_RUNTIMECLASSLIST);

	TRACE1("Warning: Cannot load %hs from archive.  Class not defined.\n",
		szClassName);

	return NULL; // not found
}

void CRuntimeClass::Store(CArchive& ar) const
// stores a runtime class description
{
	WORD nLen = (WORD)lstrlenA(m_lpszClassName);
	ar << (WORD)m_wSchema << nLen;
	ar.Write(m_lpszClassName, nLen * sizeof(char));
}

