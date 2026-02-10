#ifndef ASM_MFC_H
#define ASM_MFC_H

#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#include <new>

#include "assert_offset.h"

typedef int(__cdecl* _PNH)(size_t);


class AFX_MODULE_THREAD_STATE;

#define _AFX_INLINE inline

#define AFXAPI __stdcall
#define AFX_CDECL __cdecl
#define AFX_DATA
#define AFX_DATADEF


#define ASSERT(f)          ((void)0)
#define VERIFY(f)          ((void)(f))
#define ASSERT_VALID(pOb)  ((void)0)
#define DEBUG_ONLY(f)      ((void)0)
inline void AFX_CDECL AfxTrace(LPCTSTR, ...) {}
#define TRACE              1 ? (void)0 : ::AfxTrace
#define TRACE0(sz)
#define TRACE1(sz, p1)
#define TRACE2(sz, p1, p2)
#define TRACE3(sz, p1, p2, p3)

#define UNUSED(x) x


struct CRuntimeClass;          // object type information

class CObject;                        // the root of all objects classes

class CException;                 // the root of all exceptions
class CArchiveException;      // archive exception
class CFileException;         // file exception
class CSimpleException;
class CMemoryException;       // out-of-memory exception
class CNotSupportedException; // feature not supported exception

class CFile;                      // raw binary file
class CStdioFile;             // buffered stdio text/binary file
class CMemFile;               // memory based file

// Non CObject classes
class CString;                        // growable string type
class CTimeSpan;                      // time/date difference
class CTime;                          // absolute time/date
struct CFileStatus;                   // file status information
struct CMemoryState;                  // diagnostic memory support

class CArchive;                       // object persistence tool
class CDumpContext;                   // object diagnostic dumping

class CCmdTarget;
class CWnd;
class CWinApp;
class COleMessageFilter;
class CView;
class CFrameWnd;
class CToolTipCtrl;
class CControlBar;


struct __POSITION {};
typedef __POSITION* POSITION;

#define BEFORE_START_POSITION ((POSITION)-1L)

struct _AFX_DOUBLE { BYTE doubleBits[sizeof(double)]; };
struct _AFX_FLOAT { BYTE floatBits[sizeof(float)]; };

#undef FALSE
#undef TRUE
#undef NULL

#define FALSE   0
#define TRUE    1
#define NULL    0


// generate static object constructor for class registration
struct AFX_CLASSINIT
{
	AFX_CLASSINIT(CRuntimeClass* pNewClass);
};

struct CRuntimeClass
{
	// Attributes
	LPCSTR m_lpszClassName;
	int m_nObjectSize;
	UINT m_wSchema; // schema number of the loaded class
	CObject* (PASCAL* m_pfnCreateObject)(); // NULL => abstract class

	const CRuntimeClass* m_pBaseClass;


	// Operations
	CObject* CreateObject();
	BOOL IsDerivedFrom(const CRuntimeClass* pBaseClass) const;

	// Implementation
	void Store(CArchive& ar) const;
	static CRuntimeClass* PASCAL Load(CArchive& ar, UINT* pwSchemaNum);

	// CRuntimeClass objects linked together in simple list
	CRuntimeClass* m_pNextClass;       // linked list of registered classes
};

struct CStringData
{
	long nRefs;     // reference count
	int nDataLength;
	int nAllocLength;
	// TCHAR data[nAllocLength]

	TCHAR* data()
	{
		return (TCHAR*)(this + 1);
	}
};

class CString
{
public:
	// Constructors
	CString();
	CString(const CString& stringSrc);
	CString(TCHAR ch, int nRepeat = 1);
	CString(LPCSTR lpsz);
	CString(LPCWSTR lpsz);
	CString(LPCTSTR lpch, int nLength);
	CString(const unsigned char* psz);

	// Attributes & Operations
		// as an array of characters
	int GetLength() const;
	BOOL IsEmpty() const;
	void Empty();                       // free up the data

	TCHAR GetAt(int nIndex) const;      // 0 based
	TCHAR operator[](int nIndex) const; // same as GetAt
	void SetAt(int nIndex, TCHAR ch);
	operator LPCTSTR() const;           // as a C string

	// overloaded assignment
	const CString& operator=(const CString& stringSrc);
	const CString& operator=(TCHAR ch);
#ifdef _UNICODE
	const CString& operator=(char ch);
#endif
	const CString& operator=(LPCSTR lpsz);
	const CString& operator=(LPCWSTR lpsz);
	const CString& operator=(const unsigned char* psz);

	// string concatenation
	const CString& operator+=(const CString& string);
	const CString& operator+=(TCHAR ch);
#ifdef _UNICODE
	const CString& operator+=(char ch);
#endif
	const CString& operator+=(LPCTSTR lpsz);

	friend CString AFXAPI operator+(const CString& string1,
		const CString& string2);
	friend CString AFXAPI operator+(const CString& string, TCHAR ch);
	friend CString AFXAPI operator+(TCHAR ch, const CString& string);
#ifdef _UNICODE
	friend CString AFXAPI operator+(const CString& string, char ch);
	friend CString AFXAPI operator+(char ch, const CString& string);
#endif
	friend CString AFXAPI operator+(const CString& string, LPCTSTR lpsz);
	friend CString AFXAPI operator+(LPCTSTR lpsz, const CString& string);

	// string comparison
	int Compare(LPCTSTR lpsz) const;         // straight character
	int CompareNoCase(LPCTSTR lpsz) const;   // ignore case
	int Collate(LPCTSTR lpsz) const;         // NLS aware

	// simple sub-string extraction
	CString Mid(int nFirst, int nCount) const;
	CString Mid(int nFirst) const;
	CString Left(int nCount) const;
	CString Right(int nCount) const;

	CString SpanIncluding(LPCTSTR lpszCharSet) const;
	CString SpanExcluding(LPCTSTR lpszCharSet) const;

	// upper/lower/reverse conversion
	void MakeUpper();
	void MakeLower();
	void MakeReverse();

	// trimming whitespace (either side)
	void TrimRight();
	void TrimLeft();

	// searching (return starting index, or -1 if not found)
	// look for a single character match
	int Find(TCHAR ch) const;               // like "C" strchr
	int ReverseFind(TCHAR ch) const;
	int FindOneOf(LPCTSTR lpszCharSet) const;

	// look for a specific sub-string
	int Find(LPCTSTR lpszSub) const;        // like "C" strstr

	// simple formatting
	void AFX_CDECL Format(LPCTSTR lpszFormat, ...);
	void AFX_CDECL Format(UINT nFormatID, ...);

#ifndef _MAC
	// formatting for localization (uses FormatMessage API)
	void AFX_CDECL FormatMessage(LPCTSTR lpszFormat, ...);
	void AFX_CDECL FormatMessage(UINT nFormatID, ...);
#endif

	// input and output
#ifdef _DEBUG
	friend CDumpContext& AFXAPI operator<<(CDumpContext& dc,
		const CString& string);
#endif
	friend CArchive& AFXAPI operator<<(CArchive& ar, const CString& string);
	friend CArchive& AFXAPI operator>>(CArchive& ar, CString& string);

	// Windows support
	BOOL LoadString(UINT nID);          // load from string resource
	// 255 chars max
#ifndef _UNICODE
	// ANSI <-> OEM support (convert string in place)
	void AnsiToOem();
	void OemToAnsi();
#endif

#ifndef _AFX_NO_BSTR_SUPPORT
	// OLE BSTR support (use for OLE automation)
	BSTR AllocSysString() const;
	BSTR SetSysString(BSTR* pbstr) const;
#endif

	// Access to string implementation buffer as "C" character array
	LPTSTR GetBuffer(int nMinBufLength);
	void ReleaseBuffer(int nNewLength = -1);
	LPTSTR GetBufferSetLength(int nNewLength);
	void FreeExtra();

	// Use LockBuffer/UnlockBuffer to turn refcounting off
	LPTSTR LockBuffer();
	void UnlockBuffer();

	// Implementation
public:
	~CString();
	int GetAllocLength() const;

protected:
	LPTSTR m_pchData;   // pointer to ref counted string data

	// implementation helpers
	CStringData* GetData() const;
	void Init();
	void AllocCopy(CString& dest, int nCopyLen, int nCopyIndex, int nExtraLen) const;
	void AllocBuffer(int nLen);
	void AssignCopy(int nSrcLen, LPCTSTR lpszSrcData);
	void ConcatCopy(int nSrc1Len, LPCTSTR lpszSrc1Data, int nSrc2Len, LPCTSTR lpszSrc2Data);
	void ConcatInPlace(int nSrcLen, LPCTSTR lpszSrcData);
	void FormatV(LPCTSTR lpszFormat, va_list argList);
	void CopyBeforeWrite();
	void AllocBeforeWrite(int nLen);
	void Release();
	static void PASCAL Release(CStringData* pData);
	static int PASCAL SafeStrlen(LPCTSTR lpsz);
};

// Compare helpers
bool AFXAPI operator==(const CString& s1, const CString& s2);
bool AFXAPI operator==(const CString& s1, LPCTSTR s2);
bool AFXAPI operator==(LPCTSTR s1, const CString& s2);
bool AFXAPI operator!=(const CString& s1, const CString& s2);
bool AFXAPI operator!=(const CString& s1, LPCTSTR s2);
bool AFXAPI operator!=(LPCTSTR s1, const CString& s2);
bool AFXAPI operator<(const CString& s1, const CString& s2);
bool AFXAPI operator<(const CString& s1, LPCTSTR s2);
bool AFXAPI operator<(LPCTSTR s1, const CString& s2);
bool AFXAPI operator>(const CString& s1, const CString& s2);
bool AFXAPI operator>(const CString& s1, LPCTSTR s2);
bool AFXAPI operator>(LPCTSTR s1, const CString& s2);
bool AFXAPI operator<=(const CString& s1, const CString& s2);
bool AFXAPI operator<=(const CString& s1, LPCTSTR s2);
bool AFXAPI operator<=(LPCTSTR s1, const CString& s2);
bool AFXAPI operator>=(const CString& s1, const CString& s2);
bool AFXAPI operator>=(const CString& s1, LPCTSTR s2);
bool AFXAPI operator>=(LPCTSTR s1, const CString& s2);

// conversion helpers
int AFX_CDECL _wcstombsz(char* mbstr, const wchar_t* wcstr, size_t count);
int AFX_CDECL _mbstowcsz(wchar_t* wcstr, const char* mbstr, size_t count);

// Globals
extern TCHAR afxChNil;
const CString& AFXAPI AfxGetEmptyString();
#define afxEmptyString AfxGetEmptyString()


// CString
_AFX_INLINE CStringData* CString::GetData() const
{
	ASSERT(m_pchData != NULL); return ((CStringData*)m_pchData) - 1;
}
_AFX_INLINE void CString::Init()
{
	m_pchData = afxEmptyString.m_pchData;
}
_AFX_INLINE CString::CString(const unsigned char* lpsz)
{
	Init(); *this = (LPCSTR)lpsz;
}
_AFX_INLINE const CString& CString::operator=(const unsigned char* lpsz)
{
	*this = (LPCSTR)lpsz; return *this;
}
#ifdef _UNICODE
_AFX_INLINE const CString& CString::operator+=(char ch)
{
	*this += (TCHAR)ch; return *this;
}
_AFX_INLINE const CString& CString::operator=(char ch)
{
	*this = (TCHAR)ch; return *this;
}
_AFX_INLINE CString AFXAPI operator+(const CString& string, char ch)
{
	return string + (TCHAR)ch;
}
_AFX_INLINE CString AFXAPI operator+(char ch, const CString& string)
{
	return (TCHAR)ch + string;
}
#endif

_AFX_INLINE int CString::GetLength() const
{
	return GetData()->nDataLength;
}
_AFX_INLINE int CString::GetAllocLength() const
{
	return GetData()->nAllocLength;
}
_AFX_INLINE BOOL CString::IsEmpty() const
{
	return GetData()->nDataLength == 0;
}
_AFX_INLINE CString::operator LPCTSTR() const
{
	return m_pchData;
}
_AFX_INLINE int PASCAL CString::SafeStrlen(LPCTSTR lpsz)
{
	return (lpsz == NULL) ? 0 : lstrlen(lpsz);
}

// CString support (windows specific)
_AFX_INLINE int CString::Compare(LPCTSTR lpsz) const
{
	return _tcscmp(m_pchData, lpsz);
}    // MBCS/Unicode aware
_AFX_INLINE int CString::CompareNoCase(LPCTSTR lpsz) const
{
	return _tcsicmp(m_pchData, lpsz);
}   // MBCS/Unicode aware
// CString::Collate is often slower than Compare but is MBSC/Unicode
//  aware as well as locale-sensitive with respect to sort order.
_AFX_INLINE int CString::Collate(LPCTSTR lpsz) const
{
	return _tcscoll(m_pchData, lpsz);
}   // locale sensitive

_AFX_INLINE TCHAR CString::GetAt(int nIndex) const
{
	ASSERT(nIndex >= 0);
	ASSERT(nIndex < GetData()->nDataLength);
	return m_pchData[nIndex];
}
_AFX_INLINE TCHAR CString::operator[](int nIndex) const
{
	// same as GetAt
	ASSERT(nIndex >= 0);
	ASSERT(nIndex < GetData()->nDataLength);
	return m_pchData[nIndex];
}
_AFX_INLINE bool AFXAPI operator==(const CString& s1, const CString& s2)
{
	return s1.Compare(s2) == 0;
}
_AFX_INLINE bool AFXAPI operator==(const CString& s1, LPCTSTR s2)
{
	return s1.Compare(s2) == 0;
}
_AFX_INLINE bool AFXAPI operator==(LPCTSTR s1, const CString& s2)
{
	return s2.Compare(s1) == 0;
}
_AFX_INLINE bool AFXAPI operator!=(const CString& s1, const CString& s2)
{
	return s1.Compare(s2) != 0;
}
_AFX_INLINE bool AFXAPI operator!=(const CString& s1, LPCTSTR s2)
{
	return s1.Compare(s2) != 0;
}
_AFX_INLINE bool AFXAPI operator!=(LPCTSTR s1, const CString& s2)
{
	return s2.Compare(s1) != 0;
}
_AFX_INLINE bool AFXAPI operator<(const CString& s1, const CString& s2)
{
	return s1.Compare(s2) < 0;
}
_AFX_INLINE bool AFXAPI operator<(const CString& s1, LPCTSTR s2)
{
	return s1.Compare(s2) < 0;
}
_AFX_INLINE bool AFXAPI operator<(LPCTSTR s1, const CString& s2)
{
	return s2.Compare(s1) > 0;
}
_AFX_INLINE bool AFXAPI operator>(const CString& s1, const CString& s2)
{
	return s1.Compare(s2) > 0;
}
_AFX_INLINE bool AFXAPI operator>(const CString& s1, LPCTSTR s2)
{
	return s1.Compare(s2) > 0;
}
_AFX_INLINE bool AFXAPI operator>(LPCTSTR s1, const CString& s2)
{
	return s2.Compare(s1) < 0;
}
_AFX_INLINE bool AFXAPI operator<=(const CString& s1, const CString& s2)
{
	return s1.Compare(s2) <= 0;
}
_AFX_INLINE bool AFXAPI operator<=(const CString& s1, LPCTSTR s2)
{
	return s1.Compare(s2) <= 0;
}
_AFX_INLINE bool AFXAPI operator<=(LPCTSTR s1, const CString& s2)
{
	return s2.Compare(s1) >= 0;
}
_AFX_INLINE bool AFXAPI operator>=(const CString& s1, const CString& s2)
{
	return s1.Compare(s2) >= 0;
}
_AFX_INLINE bool AFXAPI operator>=(const CString& s1, LPCTSTR s2)
{
	return s1.Compare(s2) >= 0;
}
_AFX_INLINE bool AFXAPI operator>=(LPCTSTR s1, const CString& s2)
{
	return s2.Compare(s1) <= 0;
}



/////////////////////////////////////////////////////////////////////////////
// class CObject is the root of all compliant objects

class CObject
{
public:

	// Object model (types, destruction, allocation)
	virtual CRuntimeClass* GetRuntimeClass() const;
	virtual ~CObject();  // virtual destructors are necessary

	// Diagnostic allocations
	void* PASCAL operator new(size_t nSize);

	void* PASCAL operator new(size_t, void* p);

	void PASCAL operator delete(void* p);


	// Disable the copy constructor and assignment by default so you will get
	//   compiler errors instead of unexpected behaviour if you pass objects
	//   by value or assign objects.
protected:
	CObject();
private:
	CObject(const CObject& objectSrc);              // no implementation
	void operator=(const CObject& objectSrc);       // no implementation

	// Attributes
public:
	BOOL IsSerializable() const;
	BOOL IsKindOf(const CRuntimeClass* pClass) const;

	// Overridables
	virtual void Serialize(CArchive& ar);

	// Diagnostic Support
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;

	// Implementation
public:
	static const AFX_DATA CRuntimeClass classCObject;
};

#define RUNTIME_CLASS(class_name) ((CRuntimeClass*)(&class_name::class##class_name))
#define ASSERT_KINDOF(class_name, object) \
	ASSERT((object)->IsKindOf(RUNTIME_CLASS(class_name)))

// RTTI helper macros/functions
const CObject* AFX_CDECL AfxDynamicDownCast(CRuntimeClass* pClass, const CObject* pObject);
CObject* AFX_CDECL AfxDynamicDownCast(CRuntimeClass* pClass, CObject* pObject);
#define DYNAMIC_DOWNCAST(class_name, object) \
	(class_name*)AfxDynamicDownCast(RUNTIME_CLASS(class_name), object)

#define STATIC_DOWNCAST(class_name, object) ((class_name*)object)

#define DECLARE_DYNAMIC(class_name) \
public: \
	static const AFX_DATA CRuntimeClass class##class_name; \
	virtual CRuntimeClass* GetRuntimeClass() const; \

#define _DECLARE_DYNAMIC(class_name) \
public: \
	static AFX_DATA CRuntimeClass class##class_name; \
	virtual CRuntimeClass* GetRuntimeClass() const override; \

#define DECLARE_DYNCREATE(class_name) \
	DECLARE_DYNAMIC(class_name) \
	static CObject* PASCAL CreateObject();

#define _DECLARE_DYNCREATE(class_name) \
	_DECLARE_DYNAMIC(class_name) \
	static CObject* PASCAL CreateObject();

#define DECLARE_SERIAL(class_name) \
	_DECLARE_DYNCREATE(class_name) \
	friend CArchive& AFXAPI operator>>(CArchive& ar, class_name* &pOb);

#define IMPLEMENT_RUNTIMECLASS(class_name, base_class_name, wSchema, pfnNew) \
	const AFX_DATADEF CRuntimeClass class_name::class##class_name = { \
		#class_name, sizeof(class class_name), wSchema, pfnNew, \
			RUNTIME_CLASS(base_class_name), NULL }; \
	CRuntimeClass* class_name::GetRuntimeClass() const \
		{ return RUNTIME_CLASS(class_name); } \

#define _IMPLEMENT_RUNTIMECLASS(class_name, base_class_name, wSchema, pfnNew) \
	AFX_DATADEF CRuntimeClass class_name::class##class_name = { \
		#class_name, sizeof(class class_name), wSchema, pfnNew, \
			RUNTIME_CLASS(base_class_name), NULL }; \
	CRuntimeClass* class_name::GetRuntimeClass() const \
		{ return RUNTIME_CLASS(class_name); } \

#define IMPLEMENT_DYNAMIC(class_name, base_class_name) \
	IMPLEMENT_RUNTIMECLASS(class_name, base_class_name, 0xFFFF, NULL)

#define IMPLEMENT_DYNCREATE(class_name, base_class_name) \
	CObject* PASCAL class_name::CreateObject() \
		{ return new class_name; } \
	IMPLEMENT_RUNTIMECLASS(class_name, base_class_name, 0xFFFF, \
		class_name::CreateObject)

#define IMPLEMENT_SERIAL(class_name, base_class_name, wSchema) \
	CObject* PASCAL class_name::CreateObject() \
		{ return new class_name; } \
	_IMPLEMENT_RUNTIMECLASS(class_name, base_class_name, wSchema, \
		class_name::CreateObject) \
	static const AFX_CLASSINIT _init_##class_name(RUNTIME_CLASS(class_name)); \
	CArchive& AFXAPI operator>>(CArchive& ar, class_name* &pOb) \
		{ pOb = (class_name*) ar.ReadObject(RUNTIME_CLASS(class_name)); \
			return ar; } \
			


void AFXAPI AfxTryCleanup();

struct AFX_EXCEPTION_LINK
{
	AFX_EXCEPTION_LINK* m_pLinkPrev;    // previous top, next in handler chain
	CException* m_pException;   // current exception (NULL in TRY block)

	AFX_EXCEPTION_LINK();       // for initialization and linking
	~AFX_EXCEPTION_LINK()       // for cleanup and unlinking
	{
		AfxTryCleanup();
	};
};

// Exception global state - never access directly
struct AFX_EXCEPTION_CONTEXT
{
	AFX_EXCEPTION_LINK* m_pLinkTop;

	// Note: most of the exception context is now in the AFX_EXCEPTION_LINK
};


#define TRY { AFX_EXCEPTION_LINK _afxExceptionLink; try {

#define CATCH(class, e) } catch (class* e) \
	{ ASSERT(e->IsKindOf(RUNTIME_CLASS(class))); \
		_afxExceptionLink.m_pException = e;

#define AND_CATCH(class, e) } catch (class* e) \
	{ ASSERT(e->IsKindOf(RUNTIME_CLASS(class))); \
		_afxExceptionLink.m_pException = e;

#define END_CATCH } }

#define THROW(e) throw e
#define THROW_LAST() (AfxThrowLastCleanup(), throw)

// Advanced macros for smaller code
#define CATCH_ALL(e) } catch (CException* e) \
	{ { ASSERT(e->IsKindOf(RUNTIME_CLASS(CException))); \
		_afxExceptionLink.m_pException = e;

#define AND_CATCH_ALL(e) } catch (CException* e) \
	{ { ASSERT(e->IsKindOf(RUNTIME_CLASS(CException))); \
		_afxExceptionLink.m_pException = e;

#define END_CATCH_ALL } } }

#define END_TRY } catch (CException* e) \
	{ ASSERT(e->IsKindOf(RUNTIME_CLASS(CException))); \
		_afxExceptionLink.m_pException = e; } }

















class CPtrArray;
class CMapPtrToPtr;
class CDocument;

class CArchive
{
public:
	// Flag values
	enum Mode { store = 0, load = 1, bNoFlushOnDelete = 2, bNoByteSwap = 4 };

	CArchive(CFile* pFile, UINT nMode, int nBufSize = 4096, void* lpBuf = NULL);
	~CArchive();

	// Attributes
	BOOL IsLoading() const;
	BOOL IsStoring() const;
	BOOL IsByteSwapping() const;
	BOOL IsBufferEmpty() const;

	CFile* GetFile() const;
	UINT GetObjectSchema(); // only valid when reading a CObject*
	void SetObjectSchema(UINT nSchema);

	// pointer to document being serialized -- must set to serialize
	//  COleClientItems in a document!
	CDocument* m_pDocument;

	// Operations
	UINT Read(void* lpBuf, UINT nMax);
	void Write(const void* lpBuf, UINT nMax);
	void Flush();
	void Close();
	void Abort();   // close and shutdown without exceptions

	// reading and writing strings
	void WriteString(LPCTSTR lpsz);
	LPTSTR ReadString(LPTSTR lpsz, UINT nMax);
	BOOL ReadString(CString& rString);

public:
	// Object I/O is pointer based to avoid added construction overhead.
	// Use the Serialize member function directly for embedded objects.
	friend CArchive& AFXAPI operator<<(CArchive& ar, const CObject* pOb);

	friend CArchive& AFXAPI operator>>(CArchive& ar, CObject*& pOb);
	friend CArchive& AFXAPI operator>>(CArchive& ar, const CObject*& pOb);

	// insertion operations
	CArchive& operator<<(BYTE by);
	CArchive& operator<<(WORD w);
	CArchive& operator<<(LONG l);
	CArchive& operator<<(DWORD dw);
	CArchive& operator<<(float f);
	CArchive& operator<<(double d);

	CArchive& operator<<(int i);
	CArchive& operator<<(short w);
	CArchive& operator<<(char ch);
	CArchive& operator<<(unsigned u);

	// extraction operations
	CArchive& operator>>(BYTE& by);
	CArchive& operator>>(WORD& w);
	CArchive& operator>>(DWORD& dw);
	CArchive& operator>>(LONG& l);
	CArchive& operator>>(float& f);
	CArchive& operator>>(double& d);

	CArchive& operator>>(int& i);
	CArchive& operator>>(short& w);
	CArchive& operator>>(char& ch);
	CArchive& operator>>(unsigned& u);

	// object read/write
	CObject* ReadObject(const CRuntimeClass* pClass);
	void WriteObject(const CObject* pOb);
	// advanced object mapping (used for forced references)
	void MapObject(const CObject* pOb);

	// advanced versioning support
	void WriteClass(const CRuntimeClass* pClassRef);
	CRuntimeClass* ReadClass(const CRuntimeClass* pClassRefRequested = NULL,
		UINT* pSchema = NULL, DWORD* pObTag = NULL);
	void SerializeClass(const CRuntimeClass* pClassRef);

	// advanced operations (used when storing/loading many objects)
	void SetStoreParams(UINT nHashSize = 2053, UINT nBlockSize = 128);
	void SetLoadParams(UINT nGrowBy = 1024);

	// Implementation
public:
	BOOL m_bForceFlat;  // for COleClientItem implementation (default TRUE)
	BOOL m_bDirectBuffer;   // TRUE if m_pFile supports direct buffering
	void FillBuffer(UINT nBytesNeeded);
	void CheckCount();  // throw exception if m_nMapCount is too large

	// special functions for reading and writing (16-bit compatible) counts
	DWORD ReadCount();
	void WriteCount(DWORD dwCount);

	// public for advanced use
	UINT m_nObjectSchema;
	CString m_strFileName;

protected:
	// archive objects cannot be copied or assigned
	CArchive(const CArchive& arSrc);
	void operator=(const CArchive& arSrc);

	BOOL m_nMode;
	BOOL m_bUserBuf;
	int m_nBufSize;
	CFile* m_pFile;
	BYTE* m_lpBufCur;
	BYTE* m_lpBufMax;
	BYTE* m_lpBufStart;

	// array/map for CObject* and CRuntimeClass* load/store
	UINT m_nMapCount;
	union
	{
		CPtrArray* m_pLoadArray;
		CMapPtrToPtr* m_pStoreMap;
	};
	// map to keep track of mismatched schemas
	CMapPtrToPtr* m_pSchemaMap;

	// advanced parameters (controls performance with large archives)
	UINT m_nGrowSize;
	UINT m_nHashSize;
};

_AFX_INLINE BOOL CArchive::IsLoading() const
{
	return (m_nMode & CArchive::load) != 0;
}
_AFX_INLINE BOOL CArchive::IsStoring() const
{
	return (m_nMode & CArchive::load) == 0;
}
_AFX_INLINE BOOL CArchive::IsByteSwapping() const
{
	return (m_nMode & CArchive::bNoByteSwap) == 0;
}
_AFX_INLINE BOOL CArchive::IsBufferEmpty() const
{
	return m_lpBufCur == m_lpBufMax;
}
_AFX_INLINE CFile* CArchive::GetFile() const
{
	return m_pFile;
}
_AFX_INLINE void CArchive::SetObjectSchema(UINT nSchema)
{
	m_nObjectSchema = nSchema;
}
_AFX_INLINE void CArchive::SetStoreParams(UINT nHashSize, UINT nBlockSize)
{
	ASSERT(IsStoring());
	ASSERT(m_pStoreMap == NULL);    // must be before first object written
	m_nHashSize = nHashSize;
	m_nGrowSize = nBlockSize;
}
_AFX_INLINE void CArchive::SetLoadParams(UINT nGrowBy)
{
	ASSERT(IsLoading());
	ASSERT(m_pLoadArray == NULL);   // must be before first object read
	m_nGrowSize = nGrowBy;
}
_AFX_INLINE CArchive& CArchive::operator<<(int i)
{
	return CArchive::operator<<((LONG)i);
}
_AFX_INLINE CArchive& CArchive::operator<<(unsigned u)
{
	return CArchive::operator<<((LONG)u);
}
_AFX_INLINE CArchive& CArchive::operator<<(short w)
{
	return CArchive::operator<<((WORD)w);
}
_AFX_INLINE CArchive& CArchive::operator<<(char ch)
{
	return CArchive::operator<<((BYTE)ch);
}
_AFX_INLINE CArchive& CArchive::operator<<(BYTE by)
{
	if (m_lpBufCur + sizeof(BYTE) > m_lpBufMax) Flush();
	*(UNALIGNED BYTE*)m_lpBufCur = by; m_lpBufCur += sizeof(BYTE); return *this;
}

_AFX_INLINE CArchive& CArchive::operator<<(WORD w)
{
	if (m_lpBufCur + sizeof(WORD) > m_lpBufMax) Flush();
	*(UNALIGNED WORD*)m_lpBufCur = w; m_lpBufCur += sizeof(WORD); return *this;
}
_AFX_INLINE CArchive& CArchive::operator<<(LONG l)
{
	if (m_lpBufCur + sizeof(LONG) > m_lpBufMax) Flush();
	*(UNALIGNED LONG*)m_lpBufCur = l; m_lpBufCur += sizeof(LONG); return *this;
}
_AFX_INLINE CArchive& CArchive::operator<<(DWORD dw)
{
	if (m_lpBufCur + sizeof(DWORD) > m_lpBufMax) Flush();
	*(UNALIGNED DWORD*)m_lpBufCur = dw; m_lpBufCur += sizeof(DWORD); return *this;
}
_AFX_INLINE CArchive& CArchive::operator<<(float f)
{
	if (m_lpBufCur + sizeof(float) > m_lpBufMax) Flush();
	*(UNALIGNED _AFX_FLOAT*)m_lpBufCur = *(_AFX_FLOAT*)&f; m_lpBufCur += sizeof(float); return *this;
}
_AFX_INLINE CArchive& CArchive::operator<<(double d)
{
	if (m_lpBufCur + sizeof(double) > m_lpBufMax) Flush();
	*(UNALIGNED _AFX_DOUBLE*)m_lpBufCur = *(_AFX_DOUBLE*)&d; m_lpBufCur += sizeof(double); return *this;
}

_AFX_INLINE CArchive& CArchive::operator>>(int& i)
{
	return CArchive::operator>>((LONG&)i);
}
_AFX_INLINE CArchive& CArchive::operator>>(unsigned& u)
{
	return CArchive::operator>>((LONG&)u);
}
_AFX_INLINE CArchive& CArchive::operator>>(short& w)
{
	return CArchive::operator>>((WORD&)w);
}
_AFX_INLINE CArchive& CArchive::operator>>(char& ch)
{
	return CArchive::operator>>((BYTE&)ch);
}
_AFX_INLINE CArchive& CArchive::operator>>(BYTE& by)
{
	if (m_lpBufCur + sizeof(BYTE) > m_lpBufMax)
		FillBuffer(sizeof(BYTE) - (UINT)(m_lpBufMax - m_lpBufCur));
	by = *(UNALIGNED BYTE*)m_lpBufCur; m_lpBufCur += sizeof(BYTE); return *this;
}

_AFX_INLINE CArchive& CArchive::operator>>(WORD& w)
{
	if (m_lpBufCur + sizeof(WORD) > m_lpBufMax)
		FillBuffer(sizeof(WORD) - (UINT)(m_lpBufMax - m_lpBufCur));
	w = *(UNALIGNED WORD*)m_lpBufCur; m_lpBufCur += sizeof(WORD); return *this;
}
_AFX_INLINE CArchive& CArchive::operator>>(DWORD& dw)
{
	if (m_lpBufCur + sizeof(DWORD) > m_lpBufMax)
		FillBuffer(sizeof(DWORD) - (UINT)(m_lpBufMax - m_lpBufCur));
	dw = *(UNALIGNED DWORD*)m_lpBufCur; m_lpBufCur += sizeof(DWORD); return *this;
}
_AFX_INLINE CArchive& CArchive::operator>>(float& f)
{
	if (m_lpBufCur + sizeof(float) > m_lpBufMax)
		FillBuffer(sizeof(float) - (UINT)(m_lpBufMax - m_lpBufCur));
	*(_AFX_FLOAT*)&f = *(UNALIGNED _AFX_FLOAT*)m_lpBufCur; m_lpBufCur += sizeof(float); return *this;
}
_AFX_INLINE CArchive& CArchive::operator>>(double& d)
{
	if (m_lpBufCur + sizeof(double) > m_lpBufMax)
		FillBuffer(sizeof(double) - (UINT)(m_lpBufMax - m_lpBufCur));
	*(_AFX_DOUBLE*)&d = *(UNALIGNED _AFX_DOUBLE*)m_lpBufCur; m_lpBufCur += sizeof(double); return *this;
}
_AFX_INLINE CArchive& CArchive::operator>>(LONG& l)
{
	if (m_lpBufCur + sizeof(LONG) > m_lpBufMax)
		FillBuffer(sizeof(LONG) - (UINT)(m_lpBufMax - m_lpBufCur));
	l = *(UNALIGNED LONG*)m_lpBufCur; m_lpBufCur += sizeof(LONG); return *this;
}

_AFX_INLINE CArchive::CArchive(const CArchive& /* arSrc */)
{
}
_AFX_INLINE void CArchive::operator=(const CArchive& /* arSrc */)
{
}
_AFX_INLINE CArchive& AFXAPI operator<<(CArchive& ar, const CObject* pOb)
{
	ar.WriteObject(pOb); return ar;
}
_AFX_INLINE CArchive& AFXAPI operator>>(CArchive& ar, CObject*& pOb)
{
	pOb = ar.ReadObject(NULL); return ar;
}
_AFX_INLINE CArchive& AFXAPI operator>>(CArchive& ar, const CObject*& pOb)
{
	pOb = ar.ReadObject(NULL); return ar;
}






BOOL AFXAPI AfxFullPath(LPTSTR lpszPathOut, LPCTSTR lpszFileIn);
void AFXAPI AfxGetRoot(LPCTSTR lpszPath, CString& strRoot);

class CTime
{
public:

	// Constructors
	static CTime PASCAL GetCurrentTime();

	CTime();
	CTime(time_t time);
	CTime(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec,
		int nDST = -1);
	CTime(WORD wDosDate, WORD wDosTime, int nDST = -1);
	CTime(const CTime& timeSrc);

	CTime(const SYSTEMTIME& sysTime, int nDST = -1);
	CTime(const FILETIME& fileTime, int nDST = -1);
	const CTime& operator=(const CTime& timeSrc);
	const CTime& operator=(time_t t);

	// Attributes
	struct tm* GetGmtTm(struct tm* ptm = NULL) const;
	struct tm* GetLocalTm(struct tm* ptm = NULL) const;

	time_t GetTime() const;
	int GetYear() const;
	int GetMonth() const;       // month of year (1 = Jan)
	int GetDay() const;         // day of month
	int GetHour() const;
	int GetMinute() const;
	int GetSecond() const;
	int GetDayOfWeek() const;   // 1=Sun, 2=Mon, ..., 7=Sat

	// Operations
		// time math
	CTimeSpan operator-(CTime time) const;
	CTime operator-(CTimeSpan timeSpan) const;
	CTime operator+(CTimeSpan timeSpan) const;
	const CTime& operator+=(CTimeSpan timeSpan);
	const CTime& operator-=(CTimeSpan timeSpan);
	BOOL operator==(CTime time) const;
	BOOL operator!=(CTime time) const;
	BOOL operator<(CTime time) const;
	BOOL operator>(CTime time) const;
	BOOL operator<=(CTime time) const;
	BOOL operator>=(CTime time) const;

	// formatting using "C" strftime
	CString Format(LPCTSTR pFormat) const;
	CString FormatGmt(LPCTSTR pFormat) const;
	CString Format(UINT nFormatID) const;
	CString FormatGmt(UINT nFormatID) const;

#ifdef _UNICODE
	// for compatibility with MFC 3.x
	CString Format(LPCSTR pFormat) const;
	CString FormatGmt(LPCSTR pFormat) const;
#endif


	friend CArchive& AFXAPI operator<<(CArchive& ar, CTime time);
	friend CArchive& AFXAPI operator>>(CArchive& ar, CTime& rtime);

private:
	time_t m_time;
};

struct CFileStatus
{
	CTime m_ctime;          // creation date/time of file
	CTime m_mtime;          // last modification date/time of file
	CTime m_atime;          // last access date/time of file
	LONG m_size;            // logical size of file in bytes
	BYTE m_attribute;       // logical OR of CFile::Attribute enum values
	BYTE _m_padding;        // pad the structure to a WORD
	TCHAR m_szFullName[_MAX_PATH]; // absolute path name
};

class CFile : public CObject
{
	DECLARE_DYNAMIC(CFile)

public:
	// Flag values
	enum OpenFlags {
		modeRead = 0x0000,
		modeWrite = 0x0001,
		modeReadWrite = 0x0002,
		shareCompat = 0x0000,
		shareExclusive = 0x0010,
		shareDenyWrite = 0x0020,
		shareDenyRead = 0x0030,
		shareDenyNone = 0x0040,
		modeNoInherit = 0x0080,
		modeCreate = 0x1000,
		modeNoTruncate = 0x2000,
		typeText = 0x4000, // typeText and typeBinary are used in
		typeBinary = (int)0x8000 // derived classes only
	};

	enum Attribute {
		normal = 0x00,
		readOnly = 0x01,
		hidden = 0x02,
		system = 0x04,
		volume = 0x08,
		directory = 0x10,
		archive = 0x20
	};

	enum SeekPosition { begin = 0x0, current = 0x1, end = 0x2 };

	enum { hFileNull = -1 };

	// Constructors
	CFile();
	CFile(int hFile);
	CFile(LPCTSTR lpszFileName, UINT nOpenFlags);

	// Attributes
	UINT m_hFile;
	operator HFILE() const;
	BOOL GetStatus(CFileStatus& rStatus) const;

public:
	virtual ~CFile();

	virtual DWORD GetPosition() const;
	virtual CString GetFileName() const;
	virtual CString GetFileTitle() const;
	virtual CString GetFilePath() const;
	virtual void SetFilePath(LPCTSTR lpszNewName);
	virtual BOOL Open(LPCTSTR lpszFileName, UINT nOpenFlags, CFileException* pError = NULL);

	virtual CFile* Duplicate() const;

	virtual LONG Seek(LONG lOff, UINT nFrom);
	virtual void SetLength(DWORD dwNewLen);
	virtual DWORD GetLength() const;

	virtual UINT Read(void* lpBuf, UINT nCount);
	virtual void Write(const void* lpBuf, UINT nCount);

	virtual void LockRange(DWORD dwPos, DWORD dwCount);
	virtual void UnlockRange(DWORD dwPos, DWORD dwCount);

	virtual void Abort();
	virtual void Flush();
	virtual void Close();

	virtual UINT GetBufferPtr(UINT nCommand, UINT nCount = 0, void** ppBufStart = NULL, void** ppBufMax = NULL);


public:
	static void PASCAL Rename(LPCTSTR lpszOldName,	LPCTSTR lpszNewName);
	static void PASCAL Remove(LPCTSTR lpszFileName);
	static BOOL PASCAL GetStatus(LPCTSTR lpszFileName,	CFileStatus& rStatus);
	static void PASCAL SetStatus(LPCTSTR lpszFileName,	const CFileStatus& status);

	DWORD SeekToEnd();
	void SeekToBegin();

	DWORD ReadHuge(void* lpBuffer, DWORD dwCount);
	void WriteHuge(const void* lpBuffer, DWORD dwCount);

	

	// Implementation
public:

	enum BufferCommand { bufferRead, bufferWrite, bufferCommit, bufferCheck };

protected:
	BOOL m_bCloseOnDelete;
	CString m_strFileName;
};

ASSERT_SIZE(CFile, 0x10);


/////////////////////////////////////////////////////////////////////////////
// STDIO file implementation

class CStdioFile : public CFile
{
	DECLARE_DYNAMIC(CStdioFile)

public:
	// Constructors
	CStdioFile();
	CStdioFile(FILE* pOpenStream);
	CStdioFile(LPCTSTR lpszFileName, UINT nOpenFlags);

	// Attributes
	FILE* m_pStream;    // stdio FILE
	// m_hFile from base class is _fileno(m_pStream)

// Operations
	// reading and writing strings
	virtual void WriteString(LPCTSTR lpsz);
	virtual LPTSTR ReadString(LPTSTR lpsz, UINT nMax);
	virtual BOOL ReadString(CString& rString);

	// Implementation
public:
	virtual ~CStdioFile();

	virtual DWORD GetPosition() const;
	virtual BOOL Open(LPCTSTR lpszFileName, UINT nOpenFlags,
		CFileException* pError = NULL);
	virtual UINT Read(void* lpBuf, UINT nCount);
	virtual void Write(const void* lpBuf, UINT nCount);
	virtual LONG Seek(LONG lOff, UINT nFrom);
	virtual void Abort();
	virtual void Flush();
	virtual void Close();

	// Unsupported APIs
	virtual CFile* Duplicate() const;
	virtual void LockRange(DWORD dwPos, DWORD dwCount);
	virtual void UnlockRange(DWORD dwPos, DWORD dwCount);
};

////////////////////////////////////////////////////////////////////////////
// Memory based file implementation

class CMemFile : public CFile
{
	DECLARE_DYNAMIC(CMemFile)

public:
	// Constructors
	CMemFile(UINT nGrowBytes = 1024);
	CMemFile(BYTE* lpBuffer, UINT nBufferSize, UINT nGrowBytes = 0);

	// Operations
	void Attach(BYTE* lpBuffer, UINT nBufferSize, UINT nGrowBytes = 0);
	BYTE* Detach();

	// Advanced Overridables
protected:
	virtual BYTE* Alloc(DWORD nBytes);
	virtual BYTE* Realloc(BYTE* lpMem, DWORD nBytes);
	virtual BYTE* Memcpy(BYTE* lpMemTarget, const BYTE* lpMemSource, UINT nBytes);
	virtual void Free(BYTE* lpMem);
	virtual void GrowFile(DWORD dwNewLen);

	// Implementation
protected:
	UINT m_nGrowBytes;
	DWORD m_nPosition;
	DWORD m_nBufferSize;
	DWORD m_nFileSize;
	BYTE* m_lpBuffer;
	BOOL m_bAutoDelete;

public:
	virtual ~CMemFile();

	virtual DWORD GetPosition() const;
	BOOL GetStatus(CFileStatus& rStatus) const;
	virtual LONG Seek(LONG lOff, UINT nFrom);
	virtual void SetLength(DWORD dwNewLen);
	virtual UINT Read(void* lpBuf, UINT nCount);
	virtual void Write(const void* lpBuf, UINT nCount);
	virtual void Abort();
	virtual void Flush();
	virtual void Close();
	virtual UINT GetBufferPtr(UINT nCommand, UINT nCount = 0,
		void** ppBufStart = NULL, void** ppBufMax = NULL);

	// Unsupported APIs
	virtual CFile* Duplicate() const;
	virtual void LockRange(DWORD dwPos, DWORD dwCount);
	virtual void UnlockRange(DWORD dwPos, DWORD dwCount);
};

////////////////////////////////////////////////////////////////////////////
// Local file searches

class CFileFind : public CObject
{
public:
	CFileFind();
	virtual ~CFileFind();

	// Attributes
public:
	DWORD GetLength() const;
#if defined(_X86_) || defined(_ALPHA_)
	__int64 GetLength64() const;
#endif
	virtual CString GetFileName() const;
	virtual CString GetFilePath() const;
	virtual CString GetFileTitle() const;
	virtual CString GetFileURL() const;
	virtual CString GetRoot() const;

	virtual BOOL GetLastWriteTime(FILETIME* pTimeStamp) const;
	virtual BOOL GetLastAccessTime(FILETIME* pTimeStamp) const;
	virtual BOOL GetCreationTime(FILETIME* pTimeStamp) const;
	virtual BOOL GetLastWriteTime(CTime& refTime) const;
	virtual BOOL GetLastAccessTime(CTime& refTime) const;
	virtual BOOL GetCreationTime(CTime& refTime) const;

	virtual BOOL MatchesMask(DWORD dwMask) const;

	virtual BOOL IsDots() const;
	// these aren't virtual because they all use MatchesMask(), which is
	BOOL IsReadOnly() const;
	BOOL IsDirectory() const;
	BOOL IsCompressed() const;
	BOOL IsSystem() const;
	BOOL IsHidden() const;
	BOOL IsTemporary() const;
	BOOL IsNormal() const;
	BOOL IsArchived() const;

	// Operations
	void Close();
	virtual BOOL FindFile(LPCTSTR pstrName = NULL, DWORD dwUnused = 0);
	virtual BOOL FindNextFile();

protected:
	virtual void CloseContext();

	// Implementation
protected:
	void* m_pFoundInfo;
	void* m_pNextInfo;
	HANDLE m_hContext;
	BOOL m_bGotLast;
	CString m_strRoot;
	TCHAR m_chDirSeparator;     // not '\\' for Internet classes


	DECLARE_DYNAMIC(CFileFind)
};







class CSimpleList
{
public:
	CSimpleList(int nNextOffset = 0);
	void Construct(int nNextOffset);

	// Operations
	BOOL IsEmpty() const;
	void AddHead(void* p);
	void RemoveAll();
	void* GetHead() const;
	void* GetNext(void* p) const;
	BOOL Remove(void* p);

	// Implementation
	void* m_pHead;
	size_t m_nNextOffset;

	void** GetNextPtr(void* p) const;   // somewhat trusting...
};

inline CSimpleList::CSimpleList(int nNextOffset)
{
	m_pHead = NULL; m_nNextOffset = nNextOffset;
}
inline void CSimpleList::Construct(int nNextOffset)
{
	ASSERT(m_pHead == NULL); m_nNextOffset = nNextOffset;
}
inline BOOL CSimpleList::IsEmpty() const
{
	return m_pHead == NULL;
}
inline void** CSimpleList::GetNextPtr(void* p) const
{
	ASSERT(p != NULL); return (void**)((BYTE*)p + m_nNextOffset);
}
inline void CSimpleList::RemoveAll()
{
	m_pHead = NULL;
}
inline void* CSimpleList::GetHead() const
{
	return m_pHead;
}
inline void* CSimpleList::GetNext(void* prevElement) const
{
	return *GetNextPtr(prevElement);
}

template<class TYPE>
class CTypedSimpleList : public CSimpleList
{
public:
	CTypedSimpleList(int nNextOffset = 0)
		: CSimpleList(nNextOffset) {
	}
	void AddHead(TYPE p)
	{
		CSimpleList::AddHead(p);
	}
	TYPE GetHead()
	{
		return (TYPE)CSimpleList::GetHead();
	}
	TYPE GetNext(TYPE p)
	{
		return (TYPE)CSimpleList::GetNext(p);
	}
	BOOL Remove(TYPE p)
	{
		return CSimpleList::Remove((TYPE)p);
	}
	operator TYPE()
	{
		return (TYPE)CSimpleList::GetHead();
	}
};






class CNoTrackObject
{
public:
	void* PASCAL operator new(size_t nSize);
	void PASCAL operator delete(void*);
	virtual ~CNoTrackObject() {}
};











/* AFXCOLL */



/////////////////////////////////////////////////////////////////////////////
// Classes declared in this file

//CObject
	// Arrays
class CByteArray;           // array of BYTE
class CWordArray;           // array of WORD
class CDWordArray;          // array of DWORD
class CUIntArray;           // array of UINT
class CPtrArray;            // array of void*
class CObArray;             // array of CObject*

// Lists
class CPtrList;             // list of void*
class CObList;              // list of CObject*

// Maps (aka Dictionaries)
class CMapWordToOb;         // map from WORD to CObject*
class CMapWordToPtr;        // map from WORD to void*
class CMapPtrToWord;        // map from void* to WORD
class CMapPtrToPtr;         // map from void* to void*

// Special String variants
class CStringArray;         // array of CStrings
class CStringList;          // list of CStrings
class CMapStringToPtr;      // map from CString to void*
class CMapStringToOb;       // map from CString to CObject*
class CMapStringToString;   // map from CString to CString

/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////

class CByteArray : public CObject
{

	DECLARE_SERIAL(CByteArray)
public:

	// Construction
	CByteArray();

	// Attributes
	int GetSize() const;
	int GetUpperBound() const;
	void SetSize(int nNewSize, int nGrowBy = -1);

	// Operations
		// Clean up
	void FreeExtra();
	void RemoveAll();

	// Accessing elements
	BYTE GetAt(int nIndex) const;
	void SetAt(int nIndex, BYTE newElement);
	BYTE& ElementAt(int nIndex);

	// Direct Access to the element data (may return NULL)
	const BYTE* GetData() const;
	BYTE* GetData();

	// Potentially growing the array
	void SetAtGrow(int nIndex, BYTE newElement);
	int Add(BYTE newElement);
	int Append(const CByteArray& src);
	void Copy(const CByteArray& src);

	// overloaded operator helpers
	BYTE operator[](int nIndex) const;
	BYTE& operator[](int nIndex);

	// Operations that move elements around
	void InsertAt(int nIndex, BYTE newElement, int nCount = 1);
	void RemoveAt(int nIndex, int nCount = 1);
	void InsertAt(int nStartIndex, CByteArray* pNewArray);

	// Implementation
protected:
	BYTE* m_pData;   // the actual array of data
	int m_nSize;     // # of elements (upperBound - 1)
	int m_nMaxSize;  // max allocated
	int m_nGrowBy;   // grow amount

public:
	~CByteArray();

	void Serialize(CArchive&);
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif

protected:
	// local typedefs for class templates
	typedef BYTE BASE_TYPE;
	typedef BYTE BASE_ARG_TYPE;
};


////////////////////////////////////////////////////////////////////////////

class CWordArray : public CObject
{

	DECLARE_SERIAL(CWordArray)
public:

	// Construction
	CWordArray();

	// Attributes
	int GetSize() const;
	int GetUpperBound() const;
	void SetSize(int nNewSize, int nGrowBy = -1);

	// Operations
		// Clean up
	void FreeExtra();
	void RemoveAll();

	// Accessing elements
	WORD GetAt(int nIndex) const;
	void SetAt(int nIndex, WORD newElement);
	WORD& ElementAt(int nIndex);

	// Direct Access to the element data (may return NULL)
	const WORD* GetData() const;
	WORD* GetData();

	// Potentially growing the array
	void SetAtGrow(int nIndex, WORD newElement);
	int Add(WORD newElement);
	int Append(const CWordArray& src);
	void Copy(const CWordArray& src);

	// overloaded operator helpers
	WORD operator[](int nIndex) const;
	WORD& operator[](int nIndex);

	// Operations that move elements around
	void InsertAt(int nIndex, WORD newElement, int nCount = 1);
	void RemoveAt(int nIndex, int nCount = 1);
	void InsertAt(int nStartIndex, CWordArray* pNewArray);

	// Implementation
protected:
	WORD* m_pData;   // the actual array of data
	int m_nSize;     // # of elements (upperBound - 1)
	int m_nMaxSize;  // max allocated
	int m_nGrowBy;   // grow amount

public:
	~CWordArray();

	void Serialize(CArchive&);
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif

protected:
	// local typedefs for class templates
	typedef WORD BASE_TYPE;
	typedef WORD BASE_ARG_TYPE;
};


////////////////////////////////////////////////////////////////////////////

class CDWordArray : public CObject
{

	DECLARE_SERIAL(CDWordArray)
public:

	// Construction
	CDWordArray();

	// Attributes
	int GetSize() const;
	int GetUpperBound() const;
	void SetSize(int nNewSize, int nGrowBy = -1);

	// Operations
		// Clean up
	void FreeExtra();
	void RemoveAll();

	// Accessing elements
	DWORD GetAt(int nIndex) const;
	void SetAt(int nIndex, DWORD newElement);
	DWORD& ElementAt(int nIndex);

	// Direct Access to the element data (may return NULL)
	const DWORD* GetData() const;
	DWORD* GetData();

	// Potentially growing the array
	void SetAtGrow(int nIndex, DWORD newElement);
	int Add(DWORD newElement);
	int Append(const CDWordArray& src);
	void Copy(const CDWordArray& src);

	// overloaded operator helpers
	DWORD operator[](int nIndex) const;
	DWORD& operator[](int nIndex);

	// Operations that move elements around
	void InsertAt(int nIndex, DWORD newElement, int nCount = 1);
	void RemoveAt(int nIndex, int nCount = 1);
	void InsertAt(int nStartIndex, CDWordArray* pNewArray);

	// Implementation
protected:
	DWORD* m_pData;   // the actual array of data
	int m_nSize;     // # of elements (upperBound - 1)
	int m_nMaxSize;  // max allocated
	int m_nGrowBy;   // grow amount

public:
	~CDWordArray();

	void Serialize(CArchive&);
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif

protected:
	// local typedefs for class templates
	typedef DWORD BASE_TYPE;
	typedef DWORD BASE_ARG_TYPE;
};


////////////////////////////////////////////////////////////////////////////

class CUIntArray : public CObject
{

	DECLARE_DYNAMIC(CUIntArray)
public:

	// Construction
	CUIntArray();

	// Attributes
	int GetSize() const;
	int GetUpperBound() const;
	void SetSize(int nNewSize, int nGrowBy = -1);

	// Operations
		// Clean up
	void FreeExtra();
	void RemoveAll();

	// Accessing elements
	UINT GetAt(int nIndex) const;
	void SetAt(int nIndex, UINT newElement);
	UINT& ElementAt(int nIndex);

	// Direct Access to the element data (may return NULL)
	const UINT* GetData() const;
	UINT* GetData();

	// Potentially growing the array
	void SetAtGrow(int nIndex, UINT newElement);
	int Add(UINT newElement);
	int Append(const CUIntArray& src);
	void Copy(const CUIntArray& src);

	// overloaded operator helpers
	UINT operator[](int nIndex) const;
	UINT& operator[](int nIndex);

	// Operations that move elements around
	void InsertAt(int nIndex, UINT newElement, int nCount = 1);
	void RemoveAt(int nIndex, int nCount = 1);
	void InsertAt(int nStartIndex, CUIntArray* pNewArray);

	// Implementation
protected:
	UINT* m_pData;   // the actual array of data
	int m_nSize;     // # of elements (upperBound - 1)
	int m_nMaxSize;  // max allocated
	int m_nGrowBy;   // grow amount

public:
	~CUIntArray();
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif

protected:
	// local typedefs for class templates
	typedef UINT BASE_TYPE;
	typedef UINT BASE_ARG_TYPE;
};


////////////////////////////////////////////////////////////////////////////

class CPtrArray : public CObject
{

	DECLARE_DYNAMIC(CPtrArray)
public:

	// Construction
	CPtrArray();

	// Attributes
	int GetSize() const;
	int GetUpperBound() const;
	void SetSize(int nNewSize, int nGrowBy = -1);

	// Operations
		// Clean up
	void FreeExtra();
	void RemoveAll();

	// Accessing elements
	void* GetAt(int nIndex) const;
	void SetAt(int nIndex, void* newElement);
	void*& ElementAt(int nIndex);

	// Direct Access to the element data (may return NULL)
	const void** GetData() const;
	void** GetData();

	// Potentially growing the array
	void SetAtGrow(int nIndex, void* newElement);
	int Add(void* newElement);
	int Append(const CPtrArray& src);
	void Copy(const CPtrArray& src);

	// overloaded operator helpers
	void* operator[](int nIndex) const;
	void*& operator[](int nIndex);

	// Operations that move elements around
	void InsertAt(int nIndex, void* newElement, int nCount = 1);
	void RemoveAt(int nIndex, int nCount = 1);
	void InsertAt(int nStartIndex, CPtrArray* pNewArray);

	// Implementation
protected:
	void** m_pData;   // the actual array of data
	int m_nSize;     // # of elements (upperBound - 1)
	int m_nMaxSize;  // max allocated
	int m_nGrowBy;   // grow amount

public:
	~CPtrArray();
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif

protected:
	// local typedefs for class templates
	typedef void* BASE_TYPE;
	typedef void* BASE_ARG_TYPE;
};


////////////////////////////////////////////////////////////////////////////

class CObArray : public CObject
{

	DECLARE_SERIAL(CObArray)
public:

	// Construction
	CObArray();

	// Attributes
	int GetSize() const;
	int GetUpperBound() const;
	void SetSize(int nNewSize, int nGrowBy = -1);

	// Operations
		// Clean up
	void FreeExtra();
	void RemoveAll();

	// Accessing elements
	CObject* GetAt(int nIndex) const;
	void SetAt(int nIndex, CObject* newElement);
	CObject*& ElementAt(int nIndex);

	// Direct Access to the element data (may return NULL)
	const CObject** GetData() const;
	CObject** GetData();

	// Potentially growing the array
	void SetAtGrow(int nIndex, CObject* newElement);
	int Add(CObject* newElement);
	int Append(const CObArray& src);
	void Copy(const CObArray& src);

	// overloaded operator helpers
	CObject* operator[](int nIndex) const;
	CObject*& operator[](int nIndex);

	// Operations that move elements around
	void InsertAt(int nIndex, CObject* newElement, int nCount = 1);
	void RemoveAt(int nIndex, int nCount = 1);
	void InsertAt(int nStartIndex, CObArray* pNewArray);

	// Implementation
protected:
	CObject** m_pData;   // the actual array of data
	int m_nSize;     // # of elements (upperBound - 1)
	int m_nMaxSize;  // max allocated
	int m_nGrowBy;   // grow amount

public:
	~CObArray();

	void Serialize(CArchive&);
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif

protected:
	// local typedefs for class templates
	typedef CObject* BASE_TYPE;
	typedef CObject* BASE_ARG_TYPE;
};


////////////////////////////////////////////////////////////////////////////

class CStringArray : public CObject
{

	DECLARE_SERIAL(CStringArray)
public:

	// Construction
	CStringArray();

	// Attributes
	int GetSize() const;
	int GetUpperBound() const;
	void SetSize(int nNewSize, int nGrowBy = -1);

	// Operations
		// Clean up
	void FreeExtra();
	void RemoveAll();

	// Accessing elements
	CString GetAt(int nIndex) const;
	void SetAt(int nIndex, LPCTSTR newElement);
	CString& ElementAt(int nIndex);

	// Direct Access to the element data (may return NULL)
	const CString* GetData() const;
	CString* GetData();

	// Potentially growing the array
	void SetAtGrow(int nIndex, LPCTSTR newElement);
	int Add(LPCTSTR newElement);
	int Append(const CStringArray& src);
	void Copy(const CStringArray& src);

	// overloaded operator helpers
	CString operator[](int nIndex) const;
	CString& operator[](int nIndex);

	// Operations that move elements around
	void InsertAt(int nIndex, LPCTSTR newElement, int nCount = 1);
	void RemoveAt(int nIndex, int nCount = 1);
	void InsertAt(int nStartIndex, CStringArray* pNewArray);

	// Implementation
protected:
	CString* m_pData;   // the actual array of data
	int m_nSize;     // # of elements (upperBound - 1)
	int m_nMaxSize;  // max allocated
	int m_nGrowBy;   // grow amount

public:
	~CStringArray();

	void Serialize(CArchive&);
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif

protected:
	// local typedefs for class templates
	typedef CString BASE_TYPE;
	typedef LPCTSTR BASE_ARG_TYPE;
};


/////////////////////////////////////////////////////////////////////////////

class CPtrList : public CObject
{

	DECLARE_DYNAMIC(CPtrList)

protected:
	struct CNode
	{
		CNode* pNext;
		CNode* pPrev;
		void* data;
	};
public:

	// Construction
	CPtrList(int nBlockSize = 10);

	// Attributes (head and tail)
		// count of elements
	int GetCount() const;
	BOOL IsEmpty() const;

	// peek at head or tail
	void*& GetHead();
	void* GetHead() const;
	void*& GetTail();
	void* GetTail() const;

	// Operations
		// get head or tail (and remove it) - don't call on empty list!
	void* RemoveHead();
	void* RemoveTail();

	// add before head or after tail
	POSITION AddHead(void* newElement);
	POSITION AddTail(void* newElement);

	// add another list of elements before head or after tail
	void AddHead(CPtrList* pNewList);
	void AddTail(CPtrList* pNewList);

	// remove all elements
	void RemoveAll();

	// iteration
	POSITION GetHeadPosition() const;
	POSITION GetTailPosition() const;
	void*& GetNext(POSITION& rPosition); // return *Position++
	void* GetNext(POSITION& rPosition) const; // return *Position++
	void*& GetPrev(POSITION& rPosition); // return *Position--
	void* GetPrev(POSITION& rPosition) const; // return *Position--

	// getting/modifying an element at a given position
	void*& GetAt(POSITION position);
	void* GetAt(POSITION position) const;
	void SetAt(POSITION pos, void* newElement);
	void RemoveAt(POSITION position);

	// inserting before or after a given position
	POSITION InsertBefore(POSITION position, void* newElement);
	POSITION InsertAfter(POSITION position, void* newElement);

	// helper functions (note: O(n) speed)
	POSITION Find(void* searchValue, POSITION startAfter = NULL) const;
	// defaults to starting at the HEAD
	// return NULL if not found
	POSITION FindIndex(int nIndex) const;
	// get the 'nIndex'th element (may return NULL)

// Implementation
protected:
	CNode* m_pNodeHead;
	CNode* m_pNodeTail;
	int m_nCount;
	CNode* m_pNodeFree;
	struct CPlex* m_pBlocks;
	int m_nBlockSize;

	CNode* NewNode(CNode*, CNode*);
	void FreeNode(CNode*);

public:
	~CPtrList();
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif
	// local typedefs for class templates
	typedef void* BASE_TYPE;
	typedef void* BASE_ARG_TYPE;
};


/////////////////////////////////////////////////////////////////////////////

class CObList : public CObject
{

	DECLARE_SERIAL(CObList)

protected:
	struct CNode
	{
		CNode* pNext;
		CNode* pPrev;
		CObject* data;
	};
public:

	// Construction
	CObList(int nBlockSize = 10);

	// Attributes (head and tail)
		// count of elements
	int GetCount() const;
	BOOL IsEmpty() const;

	// peek at head or tail
	CObject*& GetHead();
	CObject* GetHead() const;
	CObject*& GetTail();
	CObject* GetTail() const;

	// Operations
		// get head or tail (and remove it) - don't call on empty list!
	CObject* RemoveHead();
	CObject* RemoveTail();

	// add before head or after tail
	POSITION AddHead(CObject* newElement);
	POSITION AddTail(CObject* newElement);

	// add another list of elements before head or after tail
	void AddHead(CObList* pNewList);
	void AddTail(CObList* pNewList);

	// remove all elements
	void RemoveAll();

	// iteration
	POSITION GetHeadPosition() const;
	POSITION GetTailPosition() const;
	CObject*& GetNext(POSITION& rPosition); // return *Position++
	CObject* GetNext(POSITION& rPosition) const; // return *Position++
	CObject*& GetPrev(POSITION& rPosition); // return *Position--
	CObject* GetPrev(POSITION& rPosition) const; // return *Position--

	// getting/modifying an element at a given position
	CObject*& GetAt(POSITION position);
	CObject* GetAt(POSITION position) const;
	void SetAt(POSITION pos, CObject* newElement);
	void RemoveAt(POSITION position);

	// inserting before or after a given position
	POSITION InsertBefore(POSITION position, CObject* newElement);
	POSITION InsertAfter(POSITION position, CObject* newElement);

	// helper functions (note: O(n) speed)
	POSITION Find(CObject* searchValue, POSITION startAfter = NULL) const;
	// defaults to starting at the HEAD
	// return NULL if not found
	POSITION FindIndex(int nIndex) const;
	// get the 'nIndex'th element (may return NULL)

// Implementation
protected:
	CNode* m_pNodeHead;
	CNode* m_pNodeTail;
	int m_nCount;
	CNode* m_pNodeFree;
	struct CPlex* m_pBlocks;
	int m_nBlockSize;

	CNode* NewNode(CNode*, CNode*);
	void FreeNode(CNode*);

public:
	~CObList();

	void Serialize(CArchive&);
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif
	// local typedefs for class templates
	typedef CObject* BASE_TYPE;
	typedef CObject* BASE_ARG_TYPE;
};


/////////////////////////////////////////////////////////////////////////////

class CStringList : public CObject
{

	DECLARE_SERIAL(CStringList)

protected:
	struct CNode
	{
		CNode* pNext;
		CNode* pPrev;
		CString data;
	};
public:

	// Construction
	CStringList(int nBlockSize = 10);

	// Attributes (head and tail)
		// count of elements
	int GetCount() const;
	BOOL IsEmpty() const;

	// peek at head or tail
	CString& GetHead();
	CString GetHead() const;
	CString& GetTail();
	CString GetTail() const;

	// Operations
		// get head or tail (and remove it) - don't call on empty list!
	CString RemoveHead();
	CString RemoveTail();

	// add before head or after tail
	POSITION AddHead(LPCTSTR newElement);
	POSITION AddTail(LPCTSTR newElement);

	// add another list of elements before head or after tail
	void AddHead(CStringList* pNewList);
	void AddTail(CStringList* pNewList);

	// remove all elements
	void RemoveAll();

	// iteration
	POSITION GetHeadPosition() const;
	POSITION GetTailPosition() const;
	CString& GetNext(POSITION& rPosition); // return *Position++
	CString GetNext(POSITION& rPosition) const; // return *Position++
	CString& GetPrev(POSITION& rPosition); // return *Position--
	CString GetPrev(POSITION& rPosition) const; // return *Position--

	// getting/modifying an element at a given position
	CString& GetAt(POSITION position);
	CString GetAt(POSITION position) const;
	void SetAt(POSITION pos, LPCTSTR newElement);
	void RemoveAt(POSITION position);

	// inserting before or after a given position
	POSITION InsertBefore(POSITION position, LPCTSTR newElement);
	POSITION InsertAfter(POSITION position, LPCTSTR newElement);

	// helper functions (note: O(n) speed)
	POSITION Find(LPCTSTR searchValue, POSITION startAfter = NULL) const;
	// defaults to starting at the HEAD
	// return NULL if not found
	POSITION FindIndex(int nIndex) const;
	// get the 'nIndex'th element (may return NULL)

// Implementation
protected:
	CNode* m_pNodeHead;
	CNode* m_pNodeTail;
	int m_nCount;
	CNode* m_pNodeFree;
	struct CPlex* m_pBlocks;
	int m_nBlockSize;

	CNode* NewNode(CNode*, CNode*);
	void FreeNode(CNode*);

public:
	~CStringList();

	void Serialize(CArchive&);
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif
	// local typedefs for class templates
	typedef CString BASE_TYPE;
	typedef LPCTSTR BASE_ARG_TYPE;
};


/////////////////////////////////////////////////////////////////////////////

class CMapWordToPtr : public CObject
{

	DECLARE_DYNAMIC(CMapWordToPtr)
protected:
	// Association
	struct CAssoc
	{
		CAssoc* pNext;

		WORD key;
		void* value;
	};

public:

	// Construction
	CMapWordToPtr(int nBlockSize = 10);

	// Attributes
		// number of elements
	int GetCount() const;
	BOOL IsEmpty() const;

	// Lookup
	BOOL Lookup(WORD key, void*& rValue) const;

	// Operations
		// Lookup and add if not there
	void*& operator[](WORD key);

	// add a new (key, value) pair
	void SetAt(WORD key, void* newValue);

	// removing existing (key, ?) pair
	BOOL RemoveKey(WORD key);
	void RemoveAll();

	// iterating all (key, value) pairs
	POSITION GetStartPosition() const;
	void GetNextAssoc(POSITION& rNextPosition, WORD& rKey, void*& rValue) const;

	// advanced features for derived classes
	UINT GetHashTableSize() const;
	void InitHashTable(UINT hashSize, BOOL bAllocNow = TRUE);

	// Overridables: special non-virtual (see map implementation for details)
		// Routine used to user-provided hash keys
	UINT HashKey(WORD key) const;

	// Implementation
protected:
	CAssoc** m_pHashTable;
	UINT m_nHashTableSize;
	int m_nCount;
	CAssoc* m_pFreeList;
	struct CPlex* m_pBlocks;
	int m_nBlockSize;

	CAssoc* NewAssoc();
	void FreeAssoc(CAssoc*);
	CAssoc* GetAssocAt(WORD, UINT&) const;

public:
	~CMapWordToPtr();
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif


protected:
	// local typedefs for CTypedPtrMap class template
	typedef WORD BASE_KEY;
	typedef WORD BASE_ARG_KEY;
	typedef void* BASE_VALUE;
	typedef void* BASE_ARG_VALUE;
};


/////////////////////////////////////////////////////////////////////////////

class CMapPtrToWord : public CObject
{

	DECLARE_DYNAMIC(CMapPtrToWord)
protected:
	// Association
	struct CAssoc
	{
		CAssoc* pNext;

		void* key;
		WORD value;
	};

public:

	// Construction
	CMapPtrToWord(int nBlockSize = 10);

	// Attributes
		// number of elements
	int GetCount() const;
	BOOL IsEmpty() const;

	// Lookup
	BOOL Lookup(void* key, WORD& rValue) const;

	// Operations
		// Lookup and add if not there
	WORD& operator[](void* key);

	// add a new (key, value) pair
	void SetAt(void* key, WORD newValue);

	// removing existing (key, ?) pair
	BOOL RemoveKey(void* key);
	void RemoveAll();

	// iterating all (key, value) pairs
	POSITION GetStartPosition() const;
	void GetNextAssoc(POSITION& rNextPosition, void*& rKey, WORD& rValue) const;

	// advanced features for derived classes
	UINT GetHashTableSize() const;
	void InitHashTable(UINT hashSize, BOOL bAllocNow = TRUE);

	// Overridables: special non-virtual (see map implementation for details)
		// Routine used to user-provided hash keys
	UINT HashKey(void* key) const;

	// Implementation
protected:
	CAssoc** m_pHashTable;
	UINT m_nHashTableSize;
	int m_nCount;
	CAssoc* m_pFreeList;
	struct CPlex* m_pBlocks;
	int m_nBlockSize;

	CAssoc* NewAssoc();
	void FreeAssoc(CAssoc*);
	CAssoc* GetAssocAt(void*, UINT&) const;

public:
	~CMapPtrToWord();
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif


protected:
	// local typedefs for CTypedPtrMap class template
	typedef void* BASE_KEY;
	typedef void* BASE_ARG_KEY;
	typedef WORD BASE_VALUE;
	typedef WORD BASE_ARG_VALUE;
};


/////////////////////////////////////////////////////////////////////////////

class CMapPtrToPtr : public CObject
{

	DECLARE_DYNAMIC(CMapPtrToPtr)
protected:
	// Association
	struct CAssoc
	{
		CAssoc* pNext;

		void* key;
		void* value;
	};

public:

	// Construction
	CMapPtrToPtr(int nBlockSize = 10);

	// Attributes
		// number of elements
	int GetCount() const;
	BOOL IsEmpty() const;

	// Lookup
	BOOL Lookup(void* key, void*& rValue) const;

	// Operations
		// Lookup and add if not there
	void*& operator[](void* key);

	// add a new (key, value) pair
	void SetAt(void* key, void* newValue);

	// removing existing (key, ?) pair
	BOOL RemoveKey(void* key);
	void RemoveAll();

	// iterating all (key, value) pairs
	POSITION GetStartPosition() const;
	void GetNextAssoc(POSITION& rNextPosition, void*& rKey, void*& rValue) const;

	// advanced features for derived classes
	UINT GetHashTableSize() const;
	void InitHashTable(UINT hashSize, BOOL bAllocNow = TRUE);

	// Overridables: special non-virtual (see map implementation for details)
		// Routine used to user-provided hash keys
	UINT HashKey(void* key) const;

	// Implementation
protected:
	CAssoc** m_pHashTable;
	UINT m_nHashTableSize;
	int m_nCount;
	CAssoc* m_pFreeList;
	struct CPlex* m_pBlocks;
	int m_nBlockSize;

	CAssoc* NewAssoc();
	void FreeAssoc(CAssoc*);
	CAssoc* GetAssocAt(void*, UINT&) const;

public:
	~CMapPtrToPtr();
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif

	void* GetValueAt(void* key) const;


protected:
	// local typedefs for CTypedPtrMap class template
	typedef void* BASE_KEY;
	typedef void* BASE_ARG_KEY;
	typedef void* BASE_VALUE;
	typedef void* BASE_ARG_VALUE;
};


/////////////////////////////////////////////////////////////////////////////

class CMapWordToOb : public CObject
{

	DECLARE_SERIAL(CMapWordToOb)
protected:
	// Association
	struct CAssoc
	{
		CAssoc* pNext;

		WORD key;
		CObject* value;
	};

public:

	// Construction
	CMapWordToOb(int nBlockSize = 10);

	// Attributes
		// number of elements
	int GetCount() const;
	BOOL IsEmpty() const;

	// Lookup
	BOOL Lookup(WORD key, CObject*& rValue) const;

	// Operations
		// Lookup and add if not there
	CObject*& operator[](WORD key);

	// add a new (key, value) pair
	void SetAt(WORD key, CObject* newValue);

	// removing existing (key, ?) pair
	BOOL RemoveKey(WORD key);
	void RemoveAll();

	// iterating all (key, value) pairs
	POSITION GetStartPosition() const;
	void GetNextAssoc(POSITION& rNextPosition, WORD& rKey, CObject*& rValue) const;

	// advanced features for derived classes
	UINT GetHashTableSize() const;
	void InitHashTable(UINT hashSize, BOOL bAllocNow = TRUE);

	// Overridables: special non-virtual (see map implementation for details)
		// Routine used to user-provided hash keys
	UINT HashKey(WORD key) const;

	// Implementation
protected:
	CAssoc** m_pHashTable;
	UINT m_nHashTableSize;
	int m_nCount;
	CAssoc* m_pFreeList;
	struct CPlex* m_pBlocks;
	int m_nBlockSize;

	CAssoc* NewAssoc();
	void FreeAssoc(CAssoc*);
	CAssoc* GetAssocAt(WORD, UINT&) const;

public:
	~CMapWordToOb();

	void Serialize(CArchive&);
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif


protected:
	// local typedefs for CTypedPtrMap class template
	typedef WORD BASE_KEY;
	typedef WORD BASE_ARG_KEY;
	typedef CObject* BASE_VALUE;
	typedef CObject* BASE_ARG_VALUE;
};


/////////////////////////////////////////////////////////////////////////////

class CMapStringToPtr : public CObject
{

	DECLARE_DYNAMIC(CMapStringToPtr)
protected:
	// Association
	struct CAssoc
	{
		CAssoc* pNext;
		UINT nHashValue;  // needed for efficient iteration
		CString key;
		void* value;
	};

public:

	// Construction
	CMapStringToPtr(int nBlockSize = 10);

	// Attributes
		// number of elements
	int GetCount() const;
	BOOL IsEmpty() const;

	// Lookup
	BOOL Lookup(LPCTSTR key, void*& rValue) const;
	BOOL LookupKey(LPCTSTR key, LPCTSTR& rKey) const;

	// Operations
		// Lookup and add if not there
	void*& operator[](LPCTSTR key);

	// add a new (key, value) pair
	void SetAt(LPCTSTR key, void* newValue);

	// removing existing (key, ?) pair
	BOOL RemoveKey(LPCTSTR key);
	void RemoveAll();

	// iterating all (key, value) pairs
	POSITION GetStartPosition() const;
	void GetNextAssoc(POSITION& rNextPosition, CString& rKey, void*& rValue) const;

	// advanced features for derived classes
	UINT GetHashTableSize() const;
	void InitHashTable(UINT hashSize, BOOL bAllocNow = TRUE);

	// Overridables: special non-virtual (see map implementation for details)
		// Routine used to user-provided hash keys
	UINT HashKey(LPCTSTR key) const;

	// Implementation
protected:
	CAssoc** m_pHashTable;
	UINT m_nHashTableSize;
	int m_nCount;
	CAssoc* m_pFreeList;
	struct CPlex* m_pBlocks;
	int m_nBlockSize;

	CAssoc* NewAssoc();
	void FreeAssoc(CAssoc*);
	CAssoc* GetAssocAt(LPCTSTR, UINT&) const;

public:
	~CMapStringToPtr();
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif

protected:
	// local typedefs for CTypedPtrMap class template
	typedef CString BASE_KEY;
	typedef LPCTSTR BASE_ARG_KEY;
	typedef void* BASE_VALUE;
	typedef void* BASE_ARG_VALUE;
};


/////////////////////////////////////////////////////////////////////////////

class CMapStringToOb : public CObject
{

	DECLARE_SERIAL(CMapStringToOb)
protected:
	// Association
	struct CAssoc
	{
		CAssoc* pNext;
		UINT nHashValue;  // needed for efficient iteration
		CString key;
		CObject* value;
	};

public:

	// Construction
	CMapStringToOb(int nBlockSize = 10);

	// Attributes
		// number of elements
	int GetCount() const;
	BOOL IsEmpty() const;

	// Lookup
	BOOL Lookup(LPCTSTR key, CObject*& rValue) const;
	BOOL LookupKey(LPCTSTR key, LPCTSTR& rKey) const;

	// Operations
		// Lookup and add if not there
	CObject*& operator[](LPCTSTR key);

	// add a new (key, value) pair
	void SetAt(LPCTSTR key, CObject* newValue);

	// removing existing (key, ?) pair
	BOOL RemoveKey(LPCTSTR key);
	void RemoveAll();

	// iterating all (key, value) pairs
	POSITION GetStartPosition() const;
	void GetNextAssoc(POSITION& rNextPosition, CString& rKey, CObject*& rValue) const;

	// advanced features for derived classes
	UINT GetHashTableSize() const;
	void InitHashTable(UINT hashSize, BOOL bAllocNow = TRUE);

	// Overridables: special non-virtual (see map implementation for details)
		// Routine used to user-provided hash keys
	UINT HashKey(LPCTSTR key) const;

	// Implementation
protected:
	CAssoc** m_pHashTable;
	UINT m_nHashTableSize;
	int m_nCount;
	CAssoc* m_pFreeList;
	struct CPlex* m_pBlocks;
	int m_nBlockSize;

	CAssoc* NewAssoc();
	void FreeAssoc(CAssoc*);
	CAssoc* GetAssocAt(LPCTSTR, UINT&) const;

public:
	~CMapStringToOb();

	void Serialize(CArchive&);
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif

protected:
	// local typedefs for CTypedPtrMap class template
	typedef CString BASE_KEY;
	typedef LPCTSTR BASE_ARG_KEY;
	typedef CObject* BASE_VALUE;
	typedef CObject* BASE_ARG_VALUE;
};


/////////////////////////////////////////////////////////////////////////////

class CMapStringToString : public CObject
{

	DECLARE_SERIAL(CMapStringToString)
protected:
	// Association
	struct CAssoc
	{
		CAssoc* pNext;
		UINT nHashValue;  // needed for efficient iteration
		CString key;
		CString value;
	};

public:

	// Construction
	CMapStringToString(int nBlockSize = 10);

	// Attributes
		// number of elements
	int GetCount() const;
	BOOL IsEmpty() const;

	// Lookup
	BOOL Lookup(LPCTSTR key, CString& rValue) const;
	BOOL LookupKey(LPCTSTR key, LPCTSTR& rKey) const;

	// Operations
		// Lookup and add if not there
	CString& operator[](LPCTSTR key);

	// add a new (key, value) pair
	void SetAt(LPCTSTR key, LPCTSTR newValue);

	// removing existing (key, ?) pair
	BOOL RemoveKey(LPCTSTR key);
	void RemoveAll();

	// iterating all (key, value) pairs
	POSITION GetStartPosition() const;
	void GetNextAssoc(POSITION& rNextPosition, CString& rKey, CString& rValue) const;

	// advanced features for derived classes
	UINT GetHashTableSize() const;
	void InitHashTable(UINT hashSize, BOOL bAllocNow = TRUE);

	// Overridables: special non-virtual (see map implementation for details)
		// Routine used to user-provided hash keys
	UINT HashKey(LPCTSTR key) const;

	// Implementation
protected:
	CAssoc** m_pHashTable;
	UINT m_nHashTableSize;
	int m_nCount;
	CAssoc* m_pFreeList;
	struct CPlex* m_pBlocks;
	int m_nBlockSize;

	CAssoc* NewAssoc();
	void FreeAssoc(CAssoc*);
	CAssoc* GetAssocAt(LPCTSTR, UINT&) const;

public:
	~CMapStringToString();

	void Serialize(CArchive&);
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif

protected:
	// local typedefs for CTypedPtrMap class template
	typedef CString BASE_KEY;
	typedef LPCTSTR BASE_ARG_KEY;
	typedef CString BASE_VALUE;
	typedef LPCTSTR BASE_ARG_VALUE;
};


































/*************************************************************************/

class CWinThread;
class CFrameWnd;
class CHandleMap;




class AFX_MODULE_THREAD_STATE : public CNoTrackObject
{
public:
	AFX_MODULE_THREAD_STATE();
	virtual ~AFX_MODULE_THREAD_STATE();

	// current CWinThread pointer
	CWinThread* m_pCurrentWinThread;

	// list of CFrameWnd objects for thread
	CTypedSimpleList<CFrameWnd*> m_frameList;

	// temporary/permanent map state
	DWORD m_nTempMapLock;           // if not 0, temp maps locked
	CHandleMap* m_pmapHWND;
	CHandleMap* m_pmapHMENU;
	CHandleMap* m_pmapHDC;
	CHandleMap* m_pmapHGDIOBJ;
	CHandleMap* m_pmapHIMAGELIST;

	// thread-local MFC new handler (separate from C-runtime)
	_PNH m_pfnNewHandler;

	// WinSock specific thread state
	HWND m_hSocketWindow;
	CMapPtrToPtr m_mapSocketHandle;
	CMapPtrToPtr m_mapDeadSockets;
	CPtrList m_listSocketNotifications;
};

ASSERT_SIZE(AFX_MODULE_THREAD_STATE, 0x84);



class CThreadLocalObject
{
public:
	// Attributes
	CNoTrackObject* GetData(CNoTrackObject* (AFXAPI* pfnCreateObject)());
	CNoTrackObject* GetDataNA();

	// Implementation
	int m_nSlot;
	~CThreadLocalObject();
};

class CProcessLocalObject
{
public:
	// Attributes
	CNoTrackObject* GetData(CNoTrackObject* (AFXAPI* pfnCreateObject)());

	// Implementation
	CNoTrackObject* volatile m_pObject;
	~CProcessLocalObject();
};

template<class TYPE>
class CThreadLocal : public CThreadLocalObject
{
	// Attributes
public:
	inline TYPE* GetData()
	{
		TYPE* pData = (TYPE*)CThreadLocalObject::GetData(&CreateObject);
		ASSERT(pData != NULL);
		return pData;
	}
	inline TYPE* GetDataNA()
	{
		TYPE* pData = (TYPE*)CThreadLocalObject::GetDataNA();
		return pData;
	}
	inline operator TYPE* ()
	{
		return GetData();
	}
	inline TYPE* operator->()
	{
		return GetData();
	}

	// Implementation
public:
	static CNoTrackObject* AFXAPI CreateObject()
	{
		return new TYPE;
	}
};




AFX_MODULE_THREAD_STATE* AFXAPI AfxGetModuleThreadState();




void AFXAPI AfxThrowMemoryException();
void AFXAPI AfxThrowNotSupportedException();
void AFXAPI AfxThrowArchiveException(int cause,
	LPCTSTR lpszArchiveName = NULL);
void AFXAPI AfxThrowFileException(int cause, LONG lOsError = -1,
	LPCTSTR lpszFileName = NULL);





class CException : public CObject
{
	// abstract class for dynamic type checking
	DECLARE_DYNAMIC(CException)

public:
	// Constructors
	CException();   // 5DFB9A
	CException(BOOL bAutoDelete);   // sets m_bAutoDelete = bAutoDelete

	// Operations
	void Delete();  // use to delete exception in 'catch' block

	virtual BOOL GetErrorMessage(LPTSTR lpszError, UINT nMaxError,
		PUINT pnHelpContext = NULL);
	virtual int ReportError(UINT nType = MB_OK, UINT nMessageID = 0);

	// Implementation (setting m_bAutoDelete to FALSE is advanced)
public:
	virtual ~CException();
	BOOL m_bAutoDelete;
};

ASSERT_SIZE(CException, 8);



class CSimpleException : public CException
{
	// base class for resource-critical MFC exceptions
	// handles ownership and initialization of an error message

public:
	// Constructors
	CSimpleException();
	CSimpleException(BOOL bAutoDelete);

	// Operations
	virtual BOOL GetErrorMessage(LPTSTR lpszError, UINT nMaxError,
		PUINT pnHelpContext = NULL);

	// Implementation (setting m_bAutoDelete to FALSE is advanced)
public:
	virtual ~CSimpleException();
	BOOL m_bAutoDelete;

	void InitString();      // used during MFC initialization

protected:
	BOOL m_bInitialized;
	BOOL m_bLoaded;
	TCHAR m_szMessage[128];
	UINT m_nResourceID;

};


class CMemoryException : public CSimpleException
{
	DECLARE_DYNAMIC(CMemoryException)
public:
	CMemoryException();

	// Implementation
public:
	CMemoryException(BOOL bAutoDelete);
	CMemoryException(BOOL bAutoDelete, UINT nResourceID);
	virtual ~CMemoryException();
};

class CNotSupportedException : public CSimpleException
{
	DECLARE_DYNAMIC(CNotSupportedException)
public:
	CNotSupportedException();

	// Implementation
public:
	CNotSupportedException(BOOL bAutoDelete);
	CNotSupportedException(BOOL bAutoDelete, UINT nResourceID);
	virtual ~CNotSupportedException();
};

class CArchiveException : public CException
{
	DECLARE_DYNAMIC(CArchiveException)
public:
	enum {
		none,
		generic,
		readOnly,
		endOfFile,
		writeOnly,
		badIndex,
		badClass,
		badSchema
	};

	// Constructor
	CArchiveException(int cause = CArchiveException::none,
		LPCTSTR lpszArchiveName = NULL);

	// Attributes
	int m_cause;
	CString m_strFileName;

	// Implementation
public:
	virtual ~CArchiveException();
	virtual BOOL GetErrorMessage(LPTSTR lpszError, UINT nMaxError,
		PUINT pnHelpContext = NULL);
};



#define AFX_IDP_FILE_NONE               0xF1A0
#define AFX_IDS_UNNAMED_FILE            0xF006

class CFileException : public CException
{
	DECLARE_DYNAMIC(CFileException)

public:
	enum {
		none,
		generic,
		fileNotFound,
		badPath,
		tooManyOpenFiles,
		accessDenied,
		invalidFile,
		removeCurrentDir,
		directoryFull,
		badSeek,
		hardIO,
		sharingViolation,
		lockViolation,
		diskFull,
		endOfFile
	};

	// Constructor
	CFileException(int cause = CFileException::none, LONG lOsError = -1,
		LPCTSTR lpszArchiveName = NULL);

	// Attributes
	int     m_cause;
	LONG    m_lOsError;
	CString m_strFileName;

	// Operations
		// convert a OS dependent error code to a Cause
	static int PASCAL OsErrorToException(LONG lOsError);
	static int PASCAL ErrnoToException(int nErrno);

	// helper functions to throw exception after converting to a Cause
	static void PASCAL ThrowOsError(LONG lOsError, LPCTSTR lpszFileName = NULL);
	static void PASCAL ThrowErrno(int nErrno, LPCTSTR lpszFileName = NULL);

	// Implementation
public:
	virtual ~CFileException();
	virtual BOOL GetErrorMessage(LPTSTR lpszError, UINT nMaxError,
		PUINT pnHelpContext = NULL);
};

#define THREAD_LOCAL(class_name, ident_name) \
	AFX_DATADEF CThreadLocal<class_name> ident_name;
#define EXTERN_THREAD_LOCAL(class_name, ident_name) \
	extern AFX_DATA THREAD_LOCAL(class_name, ident_name)






class CTypeLibCache
{
public:
	CTypeLibCache() : m_cRef(0), m_lcid((LCID)-1), m_ptlib(NULL), m_ptinfo(NULL) {}
	void Lock();
	void Unlock();
	BOOL Lookup(LCID lcid, LPTYPELIB* pptlib);
	void Cache(LCID lcid, LPTYPELIB ptlib);
	BOOL LookupTypeInfo(LCID lcid, REFGUID guid, LPTYPEINFO* pptinfo);
	void CacheTypeInfo(LCID lcid, REFGUID guid, LPTYPEINFO ptinfo);
	const GUID* m_pTypeLibID;

protected:
	LCID m_lcid;
	LPTYPELIB m_ptlib;
	GUID m_guidInfo;
	LPTYPEINFO m_ptinfo;
	long m_cRef;
};


class AFX_MODULE_STATE : public CNoTrackObject
{
public:
	AFX_MODULE_STATE(BOOL bDLL);
	~AFX_MODULE_STATE();

	CWinApp* m_pCurrentWinApp;
	HINSTANCE m_hCurrentInstanceHandle;
	HINSTANCE m_hCurrentResourceHandle;
	LPCTSTR m_lpszCurrentAppName;
	BYTE m_bDLL;    // TRUE if module is a DLL, FALSE if it is an EXE
	BYTE m_bSystem; // TRUE if module is a "system" module, FALSE if not
	BYTE m_bReserved[2]; // padding

	short m_fRegisteredClasses; // flags for registered window classes

	// runtime class data
	CTypedSimpleList<CRuntimeClass*> m_classList;

	CTypedSimpleList<void*> m_factoryList;

	// number of locked OLE objects
	long m_nObjectCount;
	BOOL m_bUserCtrl;

	// AfxRegisterClass and AfxRegisterWndClass data
	TCHAR m_szUnregisterList[4096];


	// variables related to a given process in a module
	//  (used to be AFX_MODULE_PROCESS_STATE)

	void (PASCAL* m_pfnFilterToolTipMessage)(MSG*, CWnd*);


	// OLE control container manager
	void* m_pOccManager;
	// locked OLE controls
	CTypedSimpleList<void*> m_lockList;

	void* m_pDaoState;

	// Type library caches
	CTypeLibCache m_typeLibCache;
	CMapPtrToPtr* m_pTypeLibCacheMap;

	AFX_DATADEF CThreadLocal<AFX_MODULE_THREAD_STATE> m_thread;
};

ASSERT_SIZE(AFX_MODULE_STATE, 0x1074);




#define afxCurrentWinApp    AfxGetModuleState()->m_pCurrentWinApp
#define afxCurrentInstanceHandle    AfxGetModuleState()->m_hCurrentInstanceHandle
#define afxCurrentResourceHandle    AfxGetModuleState()->m_hCurrentResourceHandle
#define afxCurrentAppName   AfxGetModuleState()->m_lpszCurrentAppName
#define afxContextIsDLL     AfxGetModuleState()->m_bDLL
#define afxRegisteredClasses    AfxGetModuleState()->m_fRegisteredClasses


AFX_MODULE_STATE* AFXAPI AfxGetModuleState();
CWinApp* AFXAPI AfxGetApp();
HINSTANCE AFXAPI AfxGetInstanceHandle();
HINSTANCE AFXAPI AfxGetResourceHandle();
void AFXAPI AfxSetResourceHandle(HINSTANCE hInstResource);
LPCTSTR AFXAPI AfxGetAppName();
COleMessageFilter* AFXAPI AfxOleGetMessageFilter();
CWnd* AFXAPI AfxGetMainWnd();



class _AFX_THREAD_STATE : public CNoTrackObject
{
public:
	_AFX_THREAD_STATE();
	virtual ~_AFX_THREAD_STATE();

	// override for m_pModuleState in _AFX_APP_STATE
	AFX_MODULE_STATE* m_pModuleState;
	AFX_MODULE_STATE* m_pPrevModuleState;

	// memory safety pool for temp maps
	void* m_pSafetyPoolBuffer;    // current buffer

	// thread local exception context
	AFX_EXCEPTION_CONTEXT m_exceptionContext;

	// CWnd create, gray dialog hook, and other hook data
	CWnd* m_pWndInit;
	CWnd* m_pAlternateWndInit;      // special case commdlg hooking
	DWORD m_dwPropStyle;
	DWORD m_dwPropExStyle;
	HWND m_hWndInit;
	BOOL m_bDlgCreate;
	HHOOK m_hHookOldCbtFilter;
	HHOOK m_hHookOldMsgFilter;

	// other CWnd modal data
	MSG m_lastSentMsg;              // see CWnd::WindowProc
	HWND m_hTrackingWindow;         // see CWnd::TrackPopupMenu
	HMENU m_hTrackingMenu;
	TCHAR m_szTempClassName[96];    // see AfxRegisterWndClass
	HWND m_hLockoutNotifyWindow;    // see CWnd::OnCommand
	BOOL m_bInMsgFilter;

	// other framework modal data
	CView* m_pRoutingView;          // see CCmdTarget::GetRoutingView
	CFrameWnd* m_pRoutingFrame;     // see CCmdTarget::GetRoutingFrame

	// MFC/DB thread-local data
	BOOL m_bWaitForDataSource;

	// common controls thread state
	CToolTipCtrl* m_pToolTip;
	CWnd* m_pLastHit;       // last window to own tooltip
	int m_nLastHit;         // last hittest code
	struct {
		UINT cbSize;
		UINT uFlags;
		HWND hwnd;
		UINT_PTR uId;
		RECT rect;
		HINSTANCE hinst;
		LPSTR lpszText;
		LPARAM lParam;
	} m_lastInfo;    // last TOOLINFO structure
	int m_nLastStatus;      // last flyby status message
	CControlBar* m_pLastStatus; // last flyby status control bar

	// OLE control thread-local data
	CWnd* m_pWndPark;       // "parking space" window
	long m_nCtrlRef;        // reference count on parking window
	BOOL m_bNeedTerm;       // TRUE if OleUninitialize needs to be called
};

ASSERT_OFFSET(_AFX_THREAD_STATE, m_nLastHit, 0xd4);
ASSERT_SIZE(_AFX_THREAD_STATE, 0x118);

EXTERN_THREAD_LOCAL(_AFX_THREAD_STATE, _afxThreadState)

_AFX_THREAD_STATE* AFXAPI AfxGetThreadState();


struct AFX_CMDHANDLERINFO;



class CSize;
class CPoint;
class CRect;


class CSize : public tagSIZE
{
public:

	// Constructors
	CSize();
	CSize(int initCX, int initCY);
	CSize(SIZE initSize);
	CSize(POINT initPt);
	CSize(DWORD dwSize);

	// Operations
	BOOL operator==(SIZE size) const;
	BOOL operator!=(SIZE size) const;
	void operator+=(SIZE size);
	void operator-=(SIZE size);

	// Operators returning CSize values
	CSize operator+(SIZE size) const;
	CSize operator-(SIZE size) const;
	CSize operator-() const;

	// Operators returning CPoint values
	CPoint operator+(POINT point) const;
	CPoint operator-(POINT point) const;

	// Operators returning CRect values
	CRect operator+(const RECT* lpRect) const;
	CRect operator-(const RECT* lpRect) const;
};

class CPoint : public tagPOINT
{
public:

	// Constructors
	CPoint();
	CPoint(int initX, int initY);
	CPoint(POINT initPt);
	CPoint(SIZE initSize);
	CPoint(DWORD dwPoint);

	// Operations
	void Offset(int xOffset, int yOffset);
	void Offset(POINT point);
	void Offset(SIZE size);
	BOOL operator==(POINT point) const;
	BOOL operator!=(POINT point) const;
	void operator+=(SIZE size);
	void operator-=(SIZE size);
	void operator+=(POINT point);
	void operator-=(POINT point);

	// Operators returning CPoint values
	CPoint operator+(SIZE size) const;
	CPoint operator-(SIZE size) const;
	CPoint operator-() const;
	CPoint operator+(POINT point) const;

	// Operators returning CSize values
	CSize operator-(POINT point) const;

	// Operators returning CRect values
	CRect operator+(const RECT* lpRect) const;
	CRect operator-(const RECT* lpRect) const;
};


typedef const RECT* LPCRECT;    // pointer to read/only RECT

class CRect : public tagRECT
{
public:

	// Constructors
	CRect();
	CRect(int l, int t, int r, int b);
	CRect(const RECT& srcRect);
	CRect(LPCRECT lpSrcRect);
	CRect(POINT point, SIZE size);
	CRect(POINT topLeft, POINT bottomRight);

	// Attributes (in addition to RECT members)
	int Width() const;
	int Height() const;
	CSize Size() const;
	CPoint& TopLeft();
	CPoint& BottomRight();
	const CPoint& TopLeft() const;
	const CPoint& BottomRight() const;
	CPoint CenterPoint() const;

	// convert between CRect and LPRECT/LPCRECT (no need for &)
	operator LPRECT();
	operator LPCRECT() const;

	BOOL IsRectEmpty() const;
	BOOL IsRectNull() const;
	BOOL PtInRect(POINT point) const;

	// Operations
	void SetRect(int x1, int y1, int x2, int y2);
	void SetRect(POINT topLeft, POINT bottomRight);
	void SetRectEmpty();
	void CopyRect(LPCRECT lpSrcRect);
	BOOL EqualRect(LPCRECT lpRect) const;

	void InflateRect(int x, int y);
	void InflateRect(SIZE size);
	void InflateRect(LPCRECT lpRect);
	void InflateRect(int l, int t, int r, int b);
	void DeflateRect(int x, int y);
	void DeflateRect(SIZE size);
	void DeflateRect(LPCRECT lpRect);
	void DeflateRect(int l, int t, int r, int b);

	void OffsetRect(int x, int y);
	void OffsetRect(SIZE size);
	void OffsetRect(POINT point);
	void NormalizeRect();

	// operations that fill '*this' with result
	BOOL IntersectRect(LPCRECT lpRect1, LPCRECT lpRect2);
	BOOL UnionRect(LPCRECT lpRect1, LPCRECT lpRect2);
	BOOL SubtractRect(LPCRECT lpRectSrc1, LPCRECT lpRectSrc2);

	// Additional Operations
	void operator=(const RECT& srcRect);
	BOOL operator==(const RECT& rect) const;
	BOOL operator!=(const RECT& rect) const;
	void operator+=(POINT point);
	void operator+=(SIZE size);
	void operator+=(LPCRECT lpRect);
	void operator-=(POINT point);
	void operator-=(SIZE size);
	void operator-=(LPCRECT lpRect);
	void operator&=(const RECT& rect);
	void operator|=(const RECT& rect);

	// Operators returning CRect values
	CRect operator+(POINT point) const;
	CRect operator-(POINT point) const;
	CRect operator+(LPCRECT lpRect) const;
	CRect operator+(SIZE size) const;
	CRect operator-(SIZE size) const;
	CRect operator-(LPCRECT lpRect) const;
	CRect operator&(const RECT& rect2) const;
	CRect operator|(const RECT& rect2) const;
	CRect MulDiv(int nMultiplier, int nDivisor) const;
};



typedef void (CCmdTarget::* AFX_PMSG)(void);
typedef void (CWnd::* AFX_PMSGW)(void);
typedef void (CWinThread::* AFX_PMSGT)(void);

struct AFX_MSGMAP_ENTRY
{
	UINT nMessage;   // windows message
	UINT nCode;      // control code or WM_NOTIFY code
	UINT nID;        // control ID (or 0 for windows messages)
	UINT nLastID;    // used for entries specifying a range of control id's
	UINT nSig;       // signature type (action) or pointer to message #
	AFX_PMSG pfn;    // routine to call (or special value)
};

struct AFX_MSGMAP
{
	const AFX_MSGMAP* pBaseMap;
	const AFX_MSGMAP_ENTRY* lpEntries;
};


typedef UINT(AFX_CDECL* AFX_THREADPROC)(LPVOID);
struct IConnectionPoint;
typedef IConnectionPoint* LPCONNECTIONPOINT;

class CCmdTarget : public CObject
{
	DECLARE_DYNAMIC(CCmdTarget)
public:
	CCmdTarget();

	virtual ~CCmdTarget();

	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual void OnFinalRelease();
	virtual BOOL IsInvokeAllowed(DISPID dispid);
	virtual BOOL GetDispatchIID(IID* pIID);
	virtual UINT GetTypeInfoCount();
	virtual CTypeLibCache* GetTypeLibCache();
	virtual HRESULT GetTypeLib(LCID lcid, LPTYPELIB* ppTypeLib);
	virtual const AFX_MSGMAP* GetMessageMap() const;
	virtual const void* GetCommandMap() const;
	virtual const void* GetDispatchMap() const;
	virtual const void* GetConnectionMap() const;
	virtual const void* GetInterfaceMap() const;
	virtual const void* GetEventSinkMap() const;
	virtual BOOL OnCreateAggregates();
	virtual LPUNKNOWN GetInterfaceHook(const void*);
	virtual BOOL GetExtraConnectionPoints(CPtrArray* pConnPoints);
	virtual LPCONNECTIONPOINT GetConnectionHook(const IID& iid);

	LPDISPATCH GetIDispatch(BOOL bAddRef);
	static CCmdTarget* PASCAL FromIDispatch(LPDISPATCH lpDispatch);
	BOOL IsResultExpected();
	void EnableAutomation();
	void EnableConnections();
	void BeginWaitCursor();
	void EndWaitCursor();
	void RestoreWaitCursor();       // call after messagebox
	BOOL EnumOleVerbs(LPENUMOLEVERB* ppenumOleVerb);
	BOOL DoOleVerb(LONG iVerb, LPMSG lpMsg, HWND hWndParent, LPCRECT lpRect);
	void EnableTypeLib();
	HRESULT GetTypeInfoOfGuid(LCID lcid, const GUID& guid, LPTYPEINFO* ppTypeInfo);
	void GetNotSupported();
	void SetNotSupported();

public:
	long m_dwRef;
	LPUNKNOWN m_pOuterUnknown;  // external controlling unknown if != NULL
	DWORD m_xInnerUnknown;  // place-holder for inner controlling unknown

	struct XDispatch
	{
		DWORD m_vtbl;   // place-holder for IDispatch vtable
	} m_xDispatch;

	BOOL m_bResultExpected;

	struct XConnPtContainer
	{
		DWORD m_vtbl;   // place-holder for IConnectionPointContainer vtable
	} m_xConnPtContainer;




public:
	friend class CView;
	friend class COleCmdUI;
	friend class COleDispatchImpl;
	friend class COleConnPtContainer;

	CView* GetRoutingView();
	CFrameWnd* GetRoutingFrame();

	void EnableAggregation();       // call to enable aggregation
	void ExternalDisconnect();      // forcibly disconnect
	LPUNKNOWN GetControllingUnknown();

	DWORD InternalQueryInterface(const void*, LPVOID* ppvObj);
	DWORD InternalAddRef();
	DWORD InternalRelease();

	DWORD ExternalQueryInterface(const void*, LPVOID* ppvObj);
	DWORD ExternalAddRef();
	DWORD ExternalRelease();

	LPUNKNOWN GetInterface(const void*);
	LPUNKNOWN QueryAggregates(const void*);

	void GetStandardProp(const void* pEntry, VARIANT* pvarResult, UINT* puArgErr);
	SCODE SetStandardProp(const void* pEntry, DISPPARAMS* pDispParams, UINT* puArgErr);

	static UINT PASCAL GetEntryCount(const void* pDispMap);
	const void* PASCAL GetDispEntry(LONG memid);
	static LONG PASCAL MemberIDFromName(const void* pDispMap, LPCTSTR lpszName);

	static UINT PASCAL GetStackSize(const BYTE* pbParams, VARTYPE vtResult);

	SCODE PushStackArgs(BYTE* pStack, const BYTE* pbParams, void* pResult, VARTYPE vtResult, DISPPARAMS* pDispParams, UINT* puArgErr, VARIANT* rgTempVars);

	SCODE CallMemberFunc(const void* pEntry, WORD wFlags, VARIANT* pvarResult, DISPPARAMS* pDispParams, UINT* puArgErr);

	BOOL OnEvent(UINT idCtrl, void* pEvent, AFX_CMDHANDLERINFO* pHandlerInfo);

	const void* PASCAL GetEventSinkEntry(UINT idCtrl, void* pEvent);
};

ASSERT_SIZE(CCmdTarget, 0x1C);


class CWinThread : public CCmdTarget
{
	DECLARE_DYNAMIC(CWinThread)

public:
	// Constructors
	CWinThread();
	BOOL CreateThread(DWORD dwCreateFlags = 0, UINT nStackSize = 0, LPSECURITY_ATTRIBUTES lpSecurityAttrs = NULL);

	// Attributes
	CWnd* m_pMainWnd;       // main window (usually same AfxGetApp()->m_pMainWnd)
	CWnd* m_pActiveWnd;     // active main window (may not be m_pMainWnd)
	BOOL m_bAutoDelete;     // enables 'delete this' after thread termination

	// only valid while running
	HANDLE m_hThread;       // this thread's HANDLE
	operator HANDLE() const;
	DWORD m_nThreadID;      // this thread's ID

	int GetThreadPriority();
	BOOL SetThreadPriority(int nPriority);

	// Operations
	DWORD SuspendThread();
	DWORD ResumeThread();
	BOOL PostThreadMessage(UINT message, WPARAM wParam, LPARAM lParam);

	// Overridables
		// thread initialization
	virtual BOOL InitInstance();

	// running and idle processing
	virtual int Run();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL PumpMessage();     // low level message pump
	virtual BOOL OnIdle(LONG lCount); // return TRUE if more idle processing
	virtual BOOL IsIdleMessage(MSG* pMsg);  // checks for special messages

	// thread termination
	virtual int ExitInstance(); // default will 'delete this'

	// Advanced: exception handling
	virtual LRESULT ProcessWndProcException(CException* e, const MSG* pMsg);

	// Advanced: handling messages sent to message filter hook
	virtual BOOL ProcessMessageFilter(int code, LPMSG lpMsg);

	// Advanced: virtual access to m_pMainWnd
	virtual CWnd* GetMainWnd();

	// Implementation
public:
	virtual ~CWinThread();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
	int m_nDisablePumpCount; // Diagnostic trap to detect illegal re-entrancy
#endif
	void CommonConstruct();
	virtual void Delete();
	// 'delete this' only if m_bAutoDelete == TRUE

// message pump for Run
	MSG m_msgCur;                   // current message

public:
	// constructor used by implementation of AfxBeginThread
	CWinThread(AFX_THREADPROC pfnThreadProc, LPVOID pParam);

	// valid after construction
	LPVOID m_pThreadParams; // generic parameters passed to starting function
	AFX_THREADPROC m_pfnThreadProc;

	// set after OLE is initialized
	void (AFXAPI* m_lpfnOleTermOrFreeLib)(BOOL, BOOL);
	COleMessageFilter* m_pMessageFilter;

protected:
	CPoint m_ptCursorLast;      // last mouse position
	UINT m_nMsgLast;            // last mouse message
	BOOL DispatchThreadMessageEx(MSG* msg);  // helper
	void DispatchThreadMessage(MSG* msg);  // obsolete
};


ASSERT_SIZE(CWinThread, 0x68);


// global helpers for threads

CWinThread* AFXAPI AfxBeginThread(AFX_THREADPROC pfnThreadProc, LPVOID pParam,
	int nPriority = THREAD_PRIORITY_NORMAL, UINT nStackSize = 0,
	DWORD dwCreateFlags = 0, LPSECURITY_ATTRIBUTES lpSecurityAttrs = NULL);
CWinThread* AFXAPI AfxBeginThread(CRuntimeClass* pThreadClass,
	int nPriority = THREAD_PRIORITY_NORMAL, UINT nStackSize = 0,
	DWORD dwCreateFlags = 0, LPSECURITY_ATTRIBUTES lpSecurityAttrs = NULL);

CWinThread* AFXAPI AfxGetThread();
void AFXAPI AfxEndThread(UINT nExitCode, BOOL bDelete = TRUE);

void AFXAPI AfxInitThread();
void AFXAPI AfxTermThread(HINSTANCE hInstTerm = NULL);



class CCommandLineInfo;
class CDC;
class CRecentFileList;
class CDocTemplate;
class CPrintDialog;
class CCmdUI;

class CWinApp : public CWinThread
{
	DECLARE_DYNAMIC(CWinApp)
public:
	virtual ~CWinApp();
	virtual const AFX_MSGMAP* GetMessageMap() const override;

	virtual BOOL InitInstance();
	virtual int Run();

	virtual BOOL OnIdle(LONG lCount); // return TRUE if more idle processing
	virtual int ExitInstance(); // return app exit code
	virtual LRESULT ProcessWndProcException(CException* e, const MSG* pMsg);

	virtual CDocument* OpenDocumentFile(LPCTSTR lpszFileName); // open named file
	virtual void AddToRecentFileList(LPCTSTR lpszPathName);  // add to MRU
	virtual BOOL InitApplication();
	virtual BOOL SaveAllModified(); // save before exit
	virtual int DoMessageBox(LPCTSTR lpszPrompt, UINT nType, UINT nIDPrompt);
	virtual void DoWaitCursor(int nCode); // 0 => restore, 1=> begin, -1=> end
	virtual BOOL OnDDECommand(LPTSTR lpszCommand);
	virtual void WinHelp(DWORD dwData, UINT nCmd = HELP_CONTEXT);

public:

	// Constructor
	CWinApp(LPCTSTR lpszAppName = NULL);     // app name defaults to EXE name

	// Attributes
		// Startup args (do not change)
	HINSTANCE m_hInstance;
	HINSTANCE m_hPrevInstance;
	LPTSTR m_lpCmdLine;
	int m_nCmdShow;

	LPCTSTR m_pszAppName;  // human readable name
	LPCTSTR m_pszRegistryKey;   // used for registry entries
	void* m_pDocManager;

	BOOL m_bHelpMode;           // are we in Shift+F1 mode?


public:  // set in constructor to override default
	LPCTSTR m_pszExeName;       // executable name (no spaces)
	LPCTSTR m_pszHelpFilePath;  // default based on module path
	LPCTSTR m_pszProfileName;   // default based on app name

protected:
	void LoadStdProfileSettings(UINT nMaxMRU = 4); // load MRU file list and last preview state
	void EnableShellOpen();

	void SetDialogBkColor(COLORREF clrCtlBk = RGB(192, 192, 192), COLORREF clrCtlText = RGB(0, 0, 0));

	void SetRegistryKey(LPCTSTR lpszRegistryKey);
	void SetRegistryKey(UINT nIDRegistryKey);


	BOOL Enable3dControls(); // use CTL3D32.DLL for 3D controls in dialogs
	BOOL Enable3dControlsStatic();  // statically link CTL3D.LIB instead
	void RegisterShellFileTypes(BOOL bCompat = FALSE);
	void RegisterShellFileTypesCompat();
	void UnregisterShellFileTypes();

public:
	// Cursors
	HCURSOR LoadCursor(LPCTSTR lpszResourceName) const;
	HCURSOR LoadCursor(UINT nIDResource) const;
	HCURSOR LoadStandardCursor(LPCTSTR lpszCursorName) const; // for IDC_ values
	HCURSOR LoadOEMCursor(UINT nIDCursor) const;             // for OCR_ values

	// Icons
	HICON LoadIcon(LPCTSTR lpszResourceName) const;
	HICON LoadIcon(UINT nIDResource) const;
	HICON LoadStandardIcon(LPCTSTR lpszIconName) const;       // for IDI_ values
	HICON LoadOEMIcon(UINT nIDIcon) const;                   // for OIC_ values

	// Profile settings (to the app specific .INI file, or registry)
	UINT GetProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault);
	BOOL WriteProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue);
	CString GetProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault = NULL);
	BOOL WriteProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry,	LPCTSTR lpszValue);
	BOOL GetProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPBYTE* ppData, UINT* pBytes);
	BOOL WriteProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPBYTE pData, UINT nBytes);

	BOOL Unregister();
	LONG DelRegTree(HKEY hParentKey, const CString& strKeyName);

	void AddDocTemplate(void* pTemplate);
	POSITION GetFirstDocTemplatePosition() const;
	void* GetNextDocTemplate(POSITION& pos) const;


	void SelectPrinter(HANDLE hDevNames, HANDLE hDevMode, BOOL bFreeOld = TRUE);
	BOOL CreatePrinterDC(CDC& dc);
	BOOL GetPrinterDeviceDefaults(struct tagPDA* pPrintDlg);

	BOOL RunEmbedded();
	BOOL RunAutomated();
	void ParseCommandLine(CCommandLineInfo& rCmdInfo);
	BOOL ProcessShellCommand(CCommandLineInfo& rCmdInfo);

	void HideApplication();
	void CloseAllDocuments(BOOL bEndSession); // close documents before exiting

protected:
	// map to the following for file new/open
	void OnFileNew();
	void OnFileOpen();

	void OnFilePrintSetup();

	void OnContextHelp();   // shift-F1
	void OnHelp();          // F1 (uses current context)
	void OnHelpIndex();     // ID_HELP_INDEX
	void OnHelpFinder();    // ID_HELP_FINDER, ID_DEFAULT_HELP
	void OnHelpUsing();     // ID_HELP_USING

	// Implementation
protected:
	HGLOBAL m_hDevMode;             // printer Dev Mode
	HGLOBAL m_hDevNames;            // printer Device Names
	DWORD m_dwPromptContext;        // help context override for message box

	int m_nWaitCursorCount;         // for wait cursor (>0 => waiting)
	HCURSOR m_hcurWaitCursorRestore; // old cursor to restore after wait cursor

	CRecentFileList* m_pRecentFileList;

	void UpdatePrinterSelection(BOOL bForceDefaults);
	void SaveStdProfileSettings();  // save options to .INI file

public: // public for implementation access
	CCommandLineInfo* m_pCmdInfo;

	ATOM m_atomApp, m_atomSystemTopic;   // for DDE open
	UINT m_nNumPreviewPages;        // number of default printed pages

	size_t  m_nSafetyPoolSize;      // ideal size

	void (AFXAPI* m_lpfnDaoTerm)();

	void DevModeChange(LPTSTR lpDeviceName);
	void SetCurrentHandles();
	int GetOpenDocumentCount();

	// helpers for standard commdlg dialogs
	BOOL DoPromptFileName(CString& fileName, UINT nIDSTitle, DWORD lFlags, BOOL bOpenFileDialog, CDocTemplate* pTemplate);
	int DoPrintDialog(CPrintDialog* pPD);

	void EnableModeless(BOOL bEnable); // to disable OLE in-place dialogs

	// overrides for implementation

public:

	HKEY GetSectionKey(LPCTSTR lpszSection);
	HKEY GetAppRegistryKey();

protected:
	void OnAppExit();
	void OnUpdateRecentFileMenu(CCmdUI* pCmdUI);
	BOOL OnOpenRecentFile(UINT nID);
};


ASSERT_SIZE(CWinApp, 0xC0);


int AFXAPI AfxLoadString(UINT nID, LPTSTR lpszBuf, UINT nMaxBuf = 256);
void AFXAPI AfxFormatStrings(CString& rString, UINT nIDS, LPCTSTR const* rglpsz, int nString);
void AFXAPI AfxFormatStrings(CString& rString, LPCTSTR lpszFormat, LPCTSTR const* rglpsz, int nString);
void AFXAPI AfxFormatString1(CString& rString, UINT nIDS, LPCTSTR lpsz1);
void AFXAPI AfxFormatString2(CString& rString, UINT nIDS, LPCTSTR lpsz1, LPCTSTR lpsz2);


struct CriticalSection;

struct CriticalSectionDebugInfo {
    uint16_t type;
    uint16_t creator_back_trace_index;
    CriticalSection* critical_section;
    LIST_ENTRY process_locks_list;
    uint32_t entry_count;
    uint32_t contention_count;
    uint32_t spare[2];
};

struct CriticalSection {
    CriticalSectionDebugInfo* debug_info;
    uint32_t lock_count;
    uint32_t recursion_count;
    HANDLE owning_thread;
    HANDLE lock_semaphore;
    uint32_t spin_count;
};





















#endif
