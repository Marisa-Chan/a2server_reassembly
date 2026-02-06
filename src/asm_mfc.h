#ifndef ASM_MFC_H
#define ASM_MFC_H

#include <windows.h>
#include <tchar.h>
#include <new>

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



class CFile : public CObject {
public: // VTable at 006101ac.
    // virtual CRuntimeClass* GetRuntimeClass() const override;
    // virtual ~CFile() override;
    // virtual void Serialize(CArchive& ar) override;
    // virtual void AssertValid() const override;
    // virtual void Dump(CDumpContext& dc) const override;
    // virtual uint32_t GetPosition() override;
    // virtual const char* GetFileName(const char* buffer) override;
    // virtual const char* GetFileTitle(const char* buffer) override;
    // virtual const char* GetFilePath(const char* buffer) override;
    // virtual void SetFilePath(const char* path) override;
    // virtual BOOL Open(const char* lpszFileName, uint32_t nOpenFlags, CFileException* pError = nullptr) override;
    // virtual CFile* Duplicate() override;
    // virtual uint32_t Seek(int32_t lOff, uint32_t nFrom) override;
    // virtual void SetLength(uint32_t dwNewLen) override;
	// virtual uint32_t GetLength() override;
	// virtual UINT Read(void* lpBuf, UINT nCount) override;
	// virtual UINT Write(const void* lpBuf, UINT nCount) override;
	// virtual void LockRange(uint32_t dwPos, uint32_t dwCount) override;
	// virtual void UnlockRange(uint32_t dwPos, uint32_t dwCount) override;
	// virtual void Abort() override;
	// virtual void Flush() override;
	// virtual void Close() override;
	// virtual BYTE* GetBufferPtr(UINT nMin, UINT nMax) override;

public:
	HANDLE m_hFile;
	BOOL m_bCloseOnDelete;
	const char* m_strFileName;
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
/*
#ifndef _AFX_NO_SOCKET_SUPPORT
	// WinSock specific thread state
	HWND m_hSocketWindow;
	CMapPtrToPtr m_mapSocketHandle;
	CMapPtrToPtr m_mapDeadSockets;
	CPtrList m_listSocketNotifications;
#endif
*/
};

class CStringArray: public CObject {
public:
	const char** data;
	int32_t size;
	int32_t capacity;
	int32_t grow_by;
};

AFX_MODULE_THREAD_STATE* AFXAPI AfxGetModuleThreadState();




void AFXAPI AfxThrowMemoryException();
void AFXAPI AfxThrowNotSupportedException();
void AFXAPI AfxThrowArchiveException(int cause,
	LPCTSTR lpszArchiveName = NULL);
void AFXAPI AfxThrowFileException(int cause, LONG lOsError = -1,
	LPCTSTR lpszFileName = NULL);


#endif
