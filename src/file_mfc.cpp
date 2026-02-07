#include "asm_mfc.h"

BOOL AFXAPI AfxFullPath(LPTSTR lpszPathOut, LPCTSTR lpszFileIn);
BOOL AFXAPI AfxComparePath(LPCTSTR lpszPath1, LPCTSTR lpszPath2);

UINT AFXAPI AfxGetFileTitle(LPCTSTR lpszPathName, LPTSTR lpszTitle, UINT nMax);
UINT AFXAPI AfxGetFileName(LPCTSTR lpszPathName, LPTSTR lpszTitle, UINT nMax);
void AFX_CDECL AfxTimeToFileTime(const CTime& time, LPFILETIME pFileTime);
void AFXAPI AfxGetRoot(LPCTSTR lpszPath, CString& strRoot);


IMPLEMENT_DYNAMIC(CFile, CObject)

CFile::CFile()
{
	m_hFile = (UINT)hFileNull;
	m_bCloseOnDelete = FALSE;
}

CFile::CFile(int hFile)
{
	m_hFile = hFile;
	m_bCloseOnDelete = FALSE;
}

CFile::CFile(LPCTSTR lpszFileName, UINT nOpenFlags)
{
	ASSERT(AfxIsValidString(lpszFileName));

	CFileException e;
	if (!Open(lpszFileName, nOpenFlags, &e))
		AfxThrowFileException(e.m_cause, e.m_lOsError, e.m_strFileName);
}

CFile::~CFile()
{
	if (m_hFile != (UINT)hFileNull && m_bCloseOnDelete)
		Close();
}

CFile* CFile::Duplicate() const
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != (UINT)hFileNull);

	CFile* pFile = new CFile(hFileNull);
	HANDLE hFile;
	if (!::DuplicateHandle(::GetCurrentProcess(), (HANDLE)m_hFile,
		::GetCurrentProcess(), &hFile, 0, FALSE, DUPLICATE_SAME_ACCESS))
	{
		delete pFile;
		CFileException::ThrowOsError((LONG)::GetLastError());
	}
	pFile->m_hFile = (UINT)hFile;
	ASSERT(pFile->m_hFile != (UINT)hFileNull);
	pFile->m_bCloseOnDelete = m_bCloseOnDelete;
	return pFile;
}

BOOL CFile::Open(LPCTSTR lpszFileName, UINT nOpenFlags,
	CFileException* pException)
{
	ASSERT_VALID(this);
	ASSERT(AfxIsValidString(lpszFileName));
	ASSERT(pException == NULL ||
		AfxIsValidAddress(pException, sizeof(CFileException)));
	ASSERT((nOpenFlags & typeText) == 0);   // text mode not supported

	// CFile objects are always binary and CreateFile does not need flag
	nOpenFlags &= ~(UINT)typeBinary;

	m_bCloseOnDelete = FALSE;
	m_hFile = (UINT)hFileNull;
	m_strFileName.Empty();

	TCHAR szTemp[_MAX_PATH];
	AfxFullPath(szTemp, lpszFileName);
	m_strFileName = szTemp;

	ASSERT(sizeof(HANDLE) == sizeof(UINT));
	ASSERT(shareCompat == 0);

	// map read/write mode
	ASSERT((modeRead | modeWrite | modeReadWrite) == 3);
	DWORD dwAccess;
	switch (nOpenFlags & 3)
	{
	case modeRead:
		dwAccess = GENERIC_READ;
		break;
	case modeWrite:
		dwAccess = GENERIC_WRITE;
		break;
	case modeReadWrite:
		dwAccess = GENERIC_READ | GENERIC_WRITE;
		break;
	default:
		ASSERT(FALSE);  // invalid share mode
	}

	// map share mode
	DWORD dwShareMode;
	switch (nOpenFlags & 0x70)
	{
	case shareCompat:       // map compatibility mode to exclusive
	case shareExclusive:
		dwShareMode = 0;
		break;
	case shareDenyWrite:
		dwShareMode = FILE_SHARE_READ;
		break;
	case shareDenyRead:
		dwShareMode = FILE_SHARE_WRITE;
		break;
	case shareDenyNone:
		dwShareMode = FILE_SHARE_WRITE | FILE_SHARE_READ;
		break;
	default:
		ASSERT(FALSE);  // invalid share mode?
	}

	// Note: typeText and typeBinary are used in derived classes only.

	// map modeNoInherit flag
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = (nOpenFlags & modeNoInherit) == 0;

	// map creation flags
	DWORD dwCreateFlag;
	if (nOpenFlags & modeCreate)
	{
		if (nOpenFlags & modeNoTruncate)
			dwCreateFlag = OPEN_ALWAYS;
		else
			dwCreateFlag = CREATE_ALWAYS;
	}
	else
		dwCreateFlag = OPEN_EXISTING;

	// attempt file creation
	HANDLE hFile = ::CreateFile(lpszFileName, dwAccess, dwShareMode, &sa,
		dwCreateFlag, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		if (pException != NULL)
		{
			pException->m_lOsError = ::GetLastError();
			pException->m_cause =
				CFileException::OsErrorToException(pException->m_lOsError);

			// use passed file name (not expanded vesion) when reporting
			// an error while opening

			pException->m_strFileName = lpszFileName;
		}
		return FALSE;
	}
	m_hFile = (HFILE)hFile;
	m_bCloseOnDelete = TRUE;

	return TRUE;
}

UINT CFile::Read(void* lpBuf, UINT nCount)
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != (UINT)hFileNull);

	if (nCount == 0)
		return 0;   // avoid Win32 "null-read"

	ASSERT(lpBuf != NULL);
	ASSERT(AfxIsValidAddress(lpBuf, nCount));

	DWORD dwRead;
	if (!::ReadFile((HANDLE)m_hFile, lpBuf, nCount, &dwRead, NULL))
		CFileException::ThrowOsError((LONG)::GetLastError());

	return (UINT)dwRead;
}

void CFile::Write(const void* lpBuf, UINT nCount)
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != (UINT)hFileNull);

	if (nCount == 0)
		return;     // avoid Win32 "null-write" option

	ASSERT(lpBuf != NULL);
	ASSERT(AfxIsValidAddress(lpBuf, nCount, FALSE));

	DWORD nWritten;
	if (!::WriteFile((HANDLE)m_hFile, lpBuf, nCount, &nWritten, NULL))
		CFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);

	// Win32s will not return an error all the time (usually DISK_FULL)
	if (nWritten != nCount)
		AfxThrowFileException(CFileException::diskFull, -1, m_strFileName);
}

LONG CFile::Seek(LONG lOff, UINT nFrom)
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != (UINT)hFileNull);
	ASSERT(nFrom == begin || nFrom == end || nFrom == current);
	ASSERT(begin == FILE_BEGIN && end == FILE_END && current == FILE_CURRENT);

	DWORD dwNew = ::SetFilePointer((HANDLE)m_hFile, lOff, NULL, (DWORD)nFrom);
	if (dwNew == (DWORD)-1)
		CFileException::ThrowOsError((LONG)::GetLastError());

	return dwNew;
}

DWORD CFile::GetPosition() const
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != (UINT)hFileNull);

	DWORD dwPos = ::SetFilePointer((HANDLE)m_hFile, 0, NULL, FILE_CURRENT);
	if (dwPos == (DWORD)-1)
		CFileException::ThrowOsError((LONG)::GetLastError());

	return dwPos;
}

void CFile::Flush()
{
	ASSERT_VALID(this);

	if (m_hFile == (UINT)hFileNull)
		return;

	if (!::FlushFileBuffers((HANDLE)m_hFile))
		CFileException::ThrowOsError((LONG)::GetLastError());
}

void CFile::Close()
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != (UINT)hFileNull);

	BOOL bError = FALSE;
	if (m_hFile != (UINT)hFileNull)
		bError = !::CloseHandle((HANDLE)m_hFile);

	m_hFile = (UINT)hFileNull;
	m_bCloseOnDelete = FALSE;
	m_strFileName.Empty();

	if (bError)
		CFileException::ThrowOsError((LONG)::GetLastError());
}

void CFile::Abort()
{
	ASSERT_VALID(this);
	if (m_hFile != (UINT)hFileNull)
	{
		// close but ignore errors
		::CloseHandle((HANDLE)m_hFile);
		m_hFile = (UINT)hFileNull;
	}
	m_strFileName.Empty();
}

void CFile::LockRange(DWORD dwPos, DWORD dwCount)
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != (UINT)hFileNull);

	if (!::LockFile((HANDLE)m_hFile, dwPos, 0, dwCount, 0))
		CFileException::ThrowOsError((LONG)::GetLastError());
}

void CFile::UnlockRange(DWORD dwPos, DWORD dwCount)
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != (UINT)hFileNull);

	if (!::UnlockFile((HANDLE)m_hFile, dwPos, 0, dwCount, 0))
		CFileException::ThrowOsError((LONG)::GetLastError());
}

void CFile::SetLength(DWORD dwNewLen)
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != (UINT)hFileNull);

	Seek((LONG)dwNewLen, (UINT)begin);

	if (!::SetEndOfFile((HANDLE)m_hFile))
		CFileException::ThrowOsError((LONG)::GetLastError());
}

DWORD CFile::GetLength() const
{
	ASSERT_VALID(this);

	DWORD dwLen, dwCur;

	// Seek is a non const operation
	CFile* pFile = (CFile*)this;
	dwCur = pFile->Seek(0L, current);
	dwLen = pFile->SeekToEnd();
	VERIFY(dwCur == (DWORD)pFile->Seek(dwCur, begin));

	return dwLen;
}

// CFile does not support direct buffering (CMemFile does)
UINT CFile::GetBufferPtr(UINT nCommand, UINT /*nCount*/,
	void** /*ppBufStart*/, void** /*ppBufMax*/)
{
	ASSERT(nCommand == bufferCheck);
	UNUSED(nCommand);    // not used in retail build

	return 0;   // no support
}

void PASCAL CFile::Rename(LPCTSTR lpszOldName, LPCTSTR lpszNewName)
{
	if (!::MoveFile((LPTSTR)lpszOldName, (LPTSTR)lpszNewName))
		CFileException::ThrowOsError((LONG)::GetLastError());
}

void PASCAL CFile::Remove(LPCTSTR lpszFileName)
{
	if (!::DeleteFile((LPTSTR)lpszFileName))
		CFileException::ThrowOsError((LONG)::GetLastError());
}






static inline BOOL IsDirSep(TCHAR ch)
{
	return (ch == '\\' || ch == '/');
}



BOOL AFXAPI AfxFullPath(LPTSTR lpszPathOut, LPCTSTR lpszFileIn)
// lpszPathOut = buffer of _MAX_PATH
// lpszFileIn = file, relative path or absolute path
// (both in ANSI character set)
{
	ASSERT(AfxIsValidAddress(lpszPathOut, _MAX_PATH));

	// first, fully qualify the path name
	LPTSTR lpszFilePart;
	if (!GetFullPathName(lpszFileIn, _MAX_PATH, lpszPathOut, &lpszFilePart))
	{
		lstrcpyn(lpszPathOut, lpszFileIn, _MAX_PATH); // take it literally
		return FALSE;
	}

	CString strRoot;
	// determine the root name of the volume
	AfxGetRoot(lpszPathOut, strRoot);

	// get file system information for the volume
	DWORD dwFlags, dwDummy;
	if (!GetVolumeInformation(strRoot, NULL, 0, NULL, &dwDummy, &dwFlags,
		NULL, 0))
	{
		TRACE1("Warning: could not get volume information '%s'.\n",
			(LPCTSTR)strRoot);
		return FALSE;   // preserving case may not be correct
	}

	// not all characters have complete uppercase/lowercase
	if (!(dwFlags & FS_CASE_IS_PRESERVED))
		CharUpper(lpszPathOut);

	// assume non-UNICODE file systems, use OEM character set
	if (!(dwFlags & FS_UNICODE_STORED_ON_DISK))
	{
		WIN32_FIND_DATA data;
		HANDLE h = FindFirstFile(lpszFileIn, &data);
		if (h != INVALID_HANDLE_VALUE)
		{
			FindClose(h);
			lstrcpy(lpszFilePart, data.cFileName);
		}
	}

	return TRUE;
}

void AFXAPI AfxGetRoot(LPCTSTR lpszPath, CString& strRoot)
{
	ASSERT(lpszPath != NULL);
	// determine the root name of the volume
	LPTSTR lpszRoot = strRoot.GetBuffer(_MAX_PATH);
	memset(lpszRoot, 0, _MAX_PATH);
	lstrcpyn(lpszRoot, lpszPath, _MAX_PATH);
	LPTSTR lpsz = 0;
	for (lpsz = lpszRoot; *lpsz != '\0'; lpsz = _tcsinc(lpsz))
	{
		// find first double slash and stop
		if (IsDirSep(lpsz[0]) && IsDirSep(lpsz[1]))
			break;
	}
	if (*lpsz != '\0')
	{
		// it is a UNC name, find second slash past '\\'
		ASSERT(IsDirSep(lpsz[0]));
		ASSERT(IsDirSep(lpsz[1]));
		lpsz += 2;
		while (*lpsz != '\0' && (!IsDirSep(*lpsz)))
			lpsz = _tcsinc(lpsz);
		if (*lpsz != '\0')
			lpsz = _tcsinc(lpsz);
		while (*lpsz != '\0' && (!IsDirSep(*lpsz)))
			lpsz = _tcsinc(lpsz);
		// terminate it just after the UNC root (ie. '\\server\share\')
		if (*lpsz != '\0')
			lpsz[1] = '\0';
	}
	else
	{
		// not a UNC, look for just the first slash
		lpsz = lpszRoot;
		while (*lpsz != '\0' && (!IsDirSep(*lpsz)))
			lpsz = _tcsinc(lpsz);
		// terminate it just after root (ie. 'x:\')
		if (*lpsz != '\0')
			lpsz[1] = '\0';
	}
	strRoot.ReleaseBuffer();
}




CFile::operator HFILE() const
{
	return m_hFile;
}
DWORD CFile::ReadHuge(void* lpBuffer, DWORD dwCount)
{
	return (DWORD)Read(lpBuffer, (UINT)dwCount);
}
void CFile::WriteHuge(const void* lpBuffer, DWORD dwCount)
{
	Write(lpBuffer, (UINT)dwCount);
}
DWORD CFile::SeekToEnd()
{
	return Seek(0, CFile::end);
}
void CFile::SeekToBegin()
{
	Seek(0, CFile::begin);
}
void CFile::SetFilePath(LPCTSTR lpszNewName)
{
	ASSERT_VALID(this);
	ASSERT(AfxIsValidString(lpszNewName, FALSE));
	m_strFileName = lpszNewName;
}




CString CFile::GetFileName() const
{
	ASSERT_VALID(this);

	CFileStatus status;
	GetStatus(status);
	CString strResult;
	AfxGetFileName(status.m_szFullName, strResult.GetBuffer(_MAX_FNAME),
		_MAX_FNAME);
	strResult.ReleaseBuffer();
	return strResult;
}

CString CFile::GetFileTitle() const
{
	ASSERT_VALID(this);

	CFileStatus status;
	GetStatus(status);
	CString strResult;
	AfxGetFileTitle(status.m_szFullName, strResult.GetBuffer(_MAX_FNAME),
		_MAX_FNAME);
	strResult.ReleaseBuffer();
	return strResult;
}

CString CFile::GetFilePath() const
{
	ASSERT_VALID(this);

	CFileStatus status;
	GetStatus(status);
	return status.m_szFullName;
}

/////////////////////////////////////////////////////////////////////////////
// CFile Status implementation

BOOL CFile::GetStatus(CFileStatus& rStatus) const
{
	ASSERT_VALID(this);

	memset(&rStatus, 0, sizeof(CFileStatus));

	// copy file name from cached m_strFileName
	lstrcpyn(rStatus.m_szFullName, m_strFileName,
		_countof(rStatus.m_szFullName));

	if (m_hFile != hFileNull)
	{
		// get time current file size
		FILETIME ftCreate, ftAccess, ftModify;
		if (!::GetFileTime((HANDLE)m_hFile, &ftCreate, &ftAccess, &ftModify))
			return FALSE;

		if ((rStatus.m_size = ::GetFileSize((HANDLE)m_hFile, NULL)) == (DWORD)-1L)
			return FALSE;

#ifdef _MAC
		rStatus.m_attribute = 0;
#else
		if (m_strFileName.IsEmpty())
			rStatus.m_attribute = 0;
		else
		{
			DWORD dwAttribute = ::GetFileAttributes(m_strFileName);

			// don't return an error for this because previous versions of MFC didn't
			if (dwAttribute == 0xFFFFFFFF)
				rStatus.m_attribute = 0;
			else
			{
				rStatus.m_attribute = (BYTE)dwAttribute;
#ifdef _DEBUG
				// MFC BUG: m_attribute is only a BYTE wide
				if (dwAttribute & ~0xFF)
					TRACE0("Warning: CFile::GetStatus() returns m_attribute without high-order flags.\n");
#endif
			}
		}
#endif // _MAC

		// convert times as appropriate
		rStatus.m_ctime = CTime(ftCreate);
		rStatus.m_atime = CTime(ftAccess);
		rStatus.m_mtime = CTime(ftModify);

		if (rStatus.m_ctime.GetTime() == 0)
			rStatus.m_ctime = rStatus.m_mtime;

		if (rStatus.m_atime.GetTime() == 0)
			rStatus.m_atime = rStatus.m_mtime;
	}
	return TRUE;
}

BOOL PASCAL CFile::GetStatus(LPCTSTR lpszFileName, CFileStatus& rStatus)
{
	// attempt to fully qualify path first
	if (!AfxFullPath(rStatus.m_szFullName, lpszFileName))
	{
		rStatus.m_szFullName[0] = '\0';
		return FALSE;
	}

	WIN32_FIND_DATA findFileData;
	HANDLE hFind = FindFirstFile((LPTSTR)lpszFileName, &findFileData);
	if (hFind == INVALID_HANDLE_VALUE)
		return FALSE;
	VERIFY(FindClose(hFind));

	// strip attribute of NORMAL bit, our API doesn't have a "normal" bit.
	rStatus.m_attribute = (BYTE)
		(findFileData.dwFileAttributes & ~FILE_ATTRIBUTE_NORMAL);

	// get just the low DWORD of the file size
	ASSERT(findFileData.nFileSizeHigh == 0);
	rStatus.m_size = (LONG)findFileData.nFileSizeLow;

	// convert times as appropriate
	rStatus.m_ctime = CTime(findFileData.ftCreationTime);
	rStatus.m_atime = CTime(findFileData.ftLastAccessTime);
	rStatus.m_mtime = CTime(findFileData.ftLastWriteTime);

	if (rStatus.m_ctime.GetTime() == 0)
		rStatus.m_ctime = rStatus.m_mtime;

	if (rStatus.m_atime.GetTime() == 0)
		rStatus.m_atime = rStatus.m_mtime;

	return TRUE;
}

void AFX_CDECL AfxTimeToFileTime(const CTime& time, LPFILETIME pFileTime)
{
	SYSTEMTIME sysTime;
	sysTime.wYear = (WORD)time.GetYear();
	sysTime.wMonth = (WORD)time.GetMonth();
	sysTime.wDay = (WORD)time.GetDay();
	sysTime.wHour = (WORD)time.GetHour();
	sysTime.wMinute = (WORD)time.GetMinute();
	sysTime.wSecond = (WORD)time.GetSecond();
	sysTime.wMilliseconds = 0;

	// convert system time to local file time
	FILETIME localTime;
	if (!SystemTimeToFileTime((LPSYSTEMTIME)&sysTime, &localTime))
		CFileException::ThrowOsError((LONG)::GetLastError());

	// convert local file time to UTC file time
	if (!LocalFileTimeToFileTime(&localTime, pFileTime))
		CFileException::ThrowOsError((LONG)::GetLastError());
}

void PASCAL CFile::SetStatus(LPCTSTR lpszFileName, const CFileStatus& status)
{
	DWORD wAttr;
	FILETIME creationTime;
	FILETIME lastAccessTime;
	FILETIME lastWriteTime;
	LPFILETIME lpCreationTime = NULL;
	LPFILETIME lpLastAccessTime = NULL;
	LPFILETIME lpLastWriteTime = NULL;

	if ((wAttr = GetFileAttributes((LPTSTR)lpszFileName)) == (DWORD)-1L)
		CFileException::ThrowOsError((LONG)GetLastError());

	if ((DWORD)status.m_attribute != wAttr && (wAttr & readOnly))
	{
		// Set file attribute, only if currently readonly.
		// This way we will be able to modify the time assuming the
		// caller changed the file from readonly.

		if (!SetFileAttributes((LPTSTR)lpszFileName, (DWORD)status.m_attribute))
			CFileException::ThrowOsError((LONG)GetLastError());
	}

	// last modification time
	if (status.m_mtime.GetTime() != 0)
	{
		AfxTimeToFileTime(status.m_mtime, &lastWriteTime);
		lpLastWriteTime = &lastWriteTime;

		// last access time
		if (status.m_atime.GetTime() != 0)
		{
			AfxTimeToFileTime(status.m_atime, &lastAccessTime);
			lpLastAccessTime = &lastAccessTime;
		}

		// create time
		if (status.m_ctime.GetTime() != 0)
		{
			AfxTimeToFileTime(status.m_ctime, &creationTime);
			lpCreationTime = &creationTime;
		}

		HANDLE hFile = ::CreateFile(lpszFileName, GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (hFile == INVALID_HANDLE_VALUE)
			CFileException::ThrowOsError((LONG)::GetLastError());

		if (!SetFileTime((HANDLE)hFile, lpCreationTime, lpLastAccessTime, lpLastWriteTime))
			CFileException::ThrowOsError((LONG)::GetLastError());

		if (!::CloseHandle(hFile))
			CFileException::ThrowOsError((LONG)::GetLastError());
	}

	if ((DWORD)status.m_attribute != wAttr && !(wAttr & readOnly))
	{
		if (!SetFileAttributes((LPTSTR)lpszFileName, (DWORD)status.m_attribute))
			CFileException::ThrowOsError((LONG)GetLastError());
	}
}

///////////////////////////////////////////////////////////////////////////////
// CMemFile::GetStatus implementation

BOOL CMemFile::GetStatus(CFileStatus& rStatus) const
{
	ASSERT_VALID(this);

	rStatus.m_ctime = 0;
	rStatus.m_mtime = 0;
	rStatus.m_atime = 0;
	rStatus.m_size = m_nFileSize;
	rStatus.m_attribute = normal;
	rStatus.m_szFullName[0] = '\0';
	return TRUE;
}














// CFileFind
BOOL CFileFind::IsReadOnly() const
{
	return MatchesMask(FILE_ATTRIBUTE_READONLY);
}
BOOL CFileFind::IsDirectory() const
{
	return MatchesMask(FILE_ATTRIBUTE_DIRECTORY);
}
BOOL CFileFind::IsCompressed() const
{
	return MatchesMask(FILE_ATTRIBUTE_COMPRESSED);
}
BOOL CFileFind::IsSystem() const
{
	return MatchesMask(FILE_ATTRIBUTE_SYSTEM);
}
BOOL CFileFind::IsHidden() const
{
	return MatchesMask(FILE_ATTRIBUTE_HIDDEN);
}
BOOL CFileFind::IsTemporary() const
{
	return MatchesMask(FILE_ATTRIBUTE_TEMPORARY);
}
BOOL CFileFind::IsNormal() const
{
	return MatchesMask(FILE_ATTRIBUTE_NORMAL);
}
BOOL CFileFind::IsArchived() const
{
	return MatchesMask(FILE_ATTRIBUTE_ARCHIVE);
}









/////////////////////////////////////////////////////////////////////////////
// CFileException

CFileException::CFileException(int cause, LONG lOsError, LPCTSTR pstrFileName /* = NULL */)
{
	m_cause = cause; m_lOsError = lOsError; m_strFileName = pstrFileName;
}
CFileException::~CFileException()
{}


void PASCAL CFileException::ThrowOsError(LONG lOsError,
	LPCTSTR lpszFileName /* = NULL */)
{
	if (lOsError != 0)
		AfxThrowFileException(CFileException::OsErrorToException(lOsError),
			lOsError, lpszFileName);
}

void PASCAL CFileException::ThrowErrno(int nErrno,
	LPCTSTR lpszFileName /* = NULL */)
{
	if (nErrno != 0)
		AfxThrowFileException(CFileException::ErrnoToException(nErrno),
			_doserrno, lpszFileName);
}




BOOL CFileException::GetErrorMessage(LPTSTR lpszError, UINT nMaxError,
	PUINT pnHelpContext)
{
	ASSERT(lpszError != NULL && AfxIsValidString(lpszError, nMaxError));

	if (pnHelpContext != NULL)
		*pnHelpContext = m_cause + AFX_IDP_FILE_NONE;

	CString strMessage;
	CString strFileName = m_strFileName;
	if (strFileName.IsEmpty())
		strFileName.LoadString(AFX_IDS_UNNAMED_FILE);
	AfxFormatString1(strMessage,
		m_cause + AFX_IDP_FILE_NONE, strFileName);
	lstrcpyn(lpszError, strMessage, nMaxError);

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CFileException helpers

void AFXAPI AfxThrowFileException(int cause, LONG lOsError,
	LPCTSTR lpszFileName /* == NULL */)
{
	THROW(new CFileException(cause, lOsError, lpszFileName));
}

int PASCAL CFileException::ErrnoToException(int nErrno)
{
	switch (nErrno)
	{
	case EPERM:
	case EACCES:
		return CFileException::accessDenied;
	case EBADF:
		return CFileException::invalidFile;
	case EDEADLOCK:
		return CFileException::sharingViolation;
	case EMFILE:
		return CFileException::tooManyOpenFiles;
	case ENOENT:
	case ENFILE:
		return CFileException::fileNotFound;
	case ENOSPC:
		return CFileException::diskFull;
	case EINVAL:
	case EIO:
		return CFileException::hardIO;
	default:
		return CFileException::generic;
	}
}

int PASCAL CFileException::OsErrorToException(LONG lOsErr)
{
	// NT Error codes
	switch ((UINT)lOsErr)
	{
	case NO_ERROR:
		return CFileException::none;
	case ERROR_FILE_NOT_FOUND:
		return CFileException::fileNotFound;
	case ERROR_PATH_NOT_FOUND:
		return CFileException::badPath;
	case ERROR_TOO_MANY_OPEN_FILES:
		return CFileException::tooManyOpenFiles;
	case ERROR_ACCESS_DENIED:
		return CFileException::accessDenied;
	case ERROR_INVALID_HANDLE:
		return CFileException::fileNotFound;
	case ERROR_BAD_FORMAT:
		return CFileException::invalidFile;
	case ERROR_INVALID_ACCESS:
		return CFileException::accessDenied;
	case ERROR_INVALID_DRIVE:
		return CFileException::badPath;
	case ERROR_CURRENT_DIRECTORY:
		return CFileException::removeCurrentDir;
	case ERROR_NOT_SAME_DEVICE:
		return CFileException::badPath;
	case ERROR_NO_MORE_FILES:
		return CFileException::fileNotFound;
	case ERROR_WRITE_PROTECT:
		return CFileException::accessDenied;
	case ERROR_BAD_UNIT:
		return CFileException::hardIO;
	case ERROR_NOT_READY:
		return CFileException::hardIO;
	case ERROR_BAD_COMMAND:
		return CFileException::hardIO;
	case ERROR_CRC:
		return CFileException::hardIO;
	case ERROR_BAD_LENGTH:
		return CFileException::badSeek;
	case ERROR_SEEK:
		return CFileException::badSeek;
	case ERROR_NOT_DOS_DISK:
		return CFileException::invalidFile;
	case ERROR_SECTOR_NOT_FOUND:
		return CFileException::badSeek;
	case ERROR_WRITE_FAULT:
		return CFileException::accessDenied;
	case ERROR_READ_FAULT:
		return CFileException::badSeek;
	case ERROR_SHARING_VIOLATION:
		return CFileException::sharingViolation;
	case ERROR_LOCK_VIOLATION:
		return CFileException::lockViolation;
	case ERROR_WRONG_DISK:
		return CFileException::badPath;
	case ERROR_SHARING_BUFFER_EXCEEDED:
		return CFileException::tooManyOpenFiles;
	case ERROR_HANDLE_EOF:
		return CFileException::endOfFile;
	case ERROR_HANDLE_DISK_FULL:
		return CFileException::diskFull;
	case ERROR_DUP_NAME:
		return CFileException::badPath;
	case ERROR_BAD_NETPATH:
		return CFileException::badPath;
	case ERROR_NETWORK_BUSY:
		return CFileException::accessDenied;
	case ERROR_DEV_NOT_EXIST:
		return CFileException::badPath;
	case ERROR_ADAP_HDW_ERR:
		return CFileException::hardIO;
	case ERROR_BAD_NET_RESP:
		return CFileException::accessDenied;
	case ERROR_UNEXP_NET_ERR:
		return CFileException::hardIO;
	case ERROR_BAD_REM_ADAP:
		return CFileException::invalidFile;
	case ERROR_NO_SPOOL_SPACE:
		return CFileException::directoryFull;
	case ERROR_NETNAME_DELETED:
		return CFileException::accessDenied;
	case ERROR_NETWORK_ACCESS_DENIED:
		return CFileException::accessDenied;
	case ERROR_BAD_DEV_TYPE:
		return CFileException::invalidFile;
	case ERROR_BAD_NET_NAME:
		return CFileException::badPath;
	case ERROR_TOO_MANY_NAMES:
		return CFileException::tooManyOpenFiles;
	case ERROR_SHARING_PAUSED:
		return CFileException::badPath;
	case ERROR_REQ_NOT_ACCEP:
		return CFileException::accessDenied;
	case ERROR_FILE_EXISTS:
		return CFileException::accessDenied;
	case ERROR_CANNOT_MAKE:
		return CFileException::accessDenied;
	case ERROR_ALREADY_ASSIGNED:
		return CFileException::badPath;
	case ERROR_INVALID_PASSWORD:
		return CFileException::accessDenied;
	case ERROR_NET_WRITE_FAULT:
		return CFileException::hardIO;
	case ERROR_DISK_CHANGE:
		return CFileException::fileNotFound;
	case ERROR_DRIVE_LOCKED:
		return CFileException::lockViolation;
	case ERROR_BUFFER_OVERFLOW:
		return CFileException::badPath;
	case ERROR_DISK_FULL:
		return CFileException::diskFull;
	case ERROR_NO_MORE_SEARCH_HANDLES:
		return CFileException::tooManyOpenFiles;
	case ERROR_INVALID_TARGET_HANDLE:
		return CFileException::invalidFile;
	case ERROR_INVALID_CATEGORY:
		return CFileException::hardIO;
	case ERROR_INVALID_NAME:
		return CFileException::badPath;
	case ERROR_INVALID_LEVEL:
		return CFileException::badPath;
	case ERROR_NO_VOLUME_LABEL:
		return CFileException::badPath;
	case ERROR_NEGATIVE_SEEK:
		return CFileException::badSeek;
	case ERROR_SEEK_ON_DEVICE:
		return CFileException::badSeek;
	case ERROR_DIR_NOT_ROOT:
		return CFileException::badPath;
	case ERROR_DIR_NOT_EMPTY:
		return CFileException::removeCurrentDir;
	case ERROR_LABEL_TOO_LONG:
		return CFileException::badPath;
	case ERROR_BAD_PATHNAME:
		return CFileException::badPath;
	case ERROR_LOCK_FAILED:
		return CFileException::lockViolation;
	case ERROR_BUSY:
		return CFileException::accessDenied;
	case ERROR_INVALID_ORDINAL:
		return CFileException::invalidFile;
	case ERROR_ALREADY_EXISTS:
		return CFileException::accessDenied;
	case ERROR_INVALID_EXE_SIGNATURE:
		return CFileException::invalidFile;
	case ERROR_BAD_EXE_FORMAT:
		return CFileException::invalidFile;
	case ERROR_FILENAME_EXCED_RANGE:
		return CFileException::badPath;
	case ERROR_META_EXPANSION_TOO_LONG:
		return CFileException::badPath;
	case ERROR_DIRECTORY:
		return CFileException::badPath;
	case ERROR_OPERATION_ABORTED:
		return CFileException::hardIO;
	case ERROR_IO_INCOMPLETE:
		return CFileException::hardIO;
	case ERROR_IO_PENDING:
		return CFileException::hardIO;
	case ERROR_SWAPERROR:
		return CFileException::accessDenied;
	default:
		return CFileException::generic;
	}
}


IMPLEMENT_DYNAMIC(CFileException, CException)
