
extern _pathStrCpy:proc


;CObject
extern ??_7CObject@@6B@:dword ; CObject::vftable 60FE84
extern ?classCObject@CObject@@2UCRuntimeClass@@B:dword ; CObject::classObject 0060FEA0
extern ??3CObject@@SGXPAX@Z:proc ; CObject::operator delete(void *) 401860
extern ??2CObject@@SGPAXI@Z:proc ; CObject::operator new
extern ??0CObject@@IAE@XZ:proc ; CObject::CObject 401900
extern ??1CObject@@UAE@XZ:proc ; CObject::~CObject
extern ?Serialize@CObject@@UAEXAAVCArchive@@@Z:proc   ;CObject::Serialize(class CArchive &) 00401970
extern ?AssertValid@CObject@@UBEXXZ:proc ; CObject::AssertValid 00401980
extern ?Dump@CObject@@UBEXAAVCDumpContext@@@Z:proc ; CObject::Dump  00401990
extern ?GetRuntimeClass@CObject@@UBEPAUCRuntimeClass@@XZ:proc ; CObject::GetRuntimeClass 005DDF9B
extern ?IsKindOf@CObject@@QBEHPBUCRuntimeClass@@@Z:proc  ; CObject::IsKindOf
extern ?IsSerializable@CObject@@QBEHXZ:proc  ; CObject::IsSerializable

;CRuntimeClass
extern ?CreateObject@CRuntimeClass@@QAEPAVCObject@@XZ:proc   ;   CRuntimeClass::CreateObject
extern ?IsDerivedFrom@CRuntimeClass@@QBEHPBU1@@Z:proc   ;  CRuntimeClass::IsDerivedFrom



;CNoTrackObject
externdef ??2CNoTrackObject@@SGPAXI@Z :proc   ; (public: static void * __stdcall CNoTrackObject::operator new(unsigned int))
externdef ??3CNoTrackObject@@SGXPAX@Z :proc   ; (public: static void __stdcall CNoTrackObject::operator delete(void *))


;CThreadLocalObject
externdef ?GetData@CThreadLocalObject@@QAEPAVCNoTrackObject@@P6GPAV2@XZ@Z :proc   ; (public: class CNoTrackObject * __thiscall CThreadLocalObject::GetData(class CNoTrackObject * (__stdcall*)(void)))

externdef ?_afxThreadState@@3V?$CThreadLocal@V_AFX_THREAD_STATE@@@@A :byte    ;CThreadLocal<class _AFX_THREAD_STATE>

externdef ??0_AFX_THREAD_STATE@@QAE@XZ  : proc  



;CArchive
public ?Write@CArchive@@QAEXPBXI@Z ; CArchive::Write  005E6A26
public ?Read@CArchive@@QAEIPAXI@Z ; CArchive::Read  005E6918 
public ?WriteCount@CArchive@@QAEXK@Z  ; CArchive::WriteCount 005E6BF2
public ?ReadCount@CArchive@@QAEKXZ   ; CArchive::ReadCount 005E6C20
public ?ReadObject@CArchive@@QAEPAVCObject@@PBUCRuntimeClass@@@Z     ;  CArchive::ReadObject  005e848c
public ?WriteObject@CArchive@@QAEXPBVCObject@@@Z    ; CArchive::WriteObject   5e840f
public ?Flush@CArchive@@QAEXXZ 
public ?FillBuffer@CArchive@@QAEXI@Z

public ??5@YGAAVCArchive@@AAV0@AAVCString@@@Z
public ??6@YGAAVCArchive@@AAV0@ABVCString@@@Z





public ?AfxGetModuleThreadState@@YGPAVAFX_MODULE_THREAD_STATE@@XZ  ; AfxGetModuleThreadState(); 005F4DB8 
public ??0AFX_CLASSINIT@@QAE@PAUCRuntimeClass@@@Z  ; AFX_CLASSINIT::AFX_CLASSINIT(CRuntimeClass *)
public ??0AFX_EXCEPTION_LINK@@QAE@XZ  ;AFX_EXCEPTION_LINK::AFX_EXCEPTION_LINK
public ?AfxTryCleanup@@YGXXZ ; AfxTryCleanup
public ?AfxGetModuleState@@YGPAVAFX_MODULE_STATE@@XZ
public ?AfxGetThread@@YGPAVCWinThread@@XZ

extern ?AfxLoadString@@YGHIPADI@Z:proc   ; AfxLoadString

public ?AfxGetFileName@@YGIPBDPADI@Z
public ?AfxGetFileTitle@@YGIPBDPADI@Z


public ?classCException@CException@@2UCRuntimeClass@@B
public ??0CException@@QAE@XZ     ;CException()
public ??0CException@@QAE@H@Z    ;CException(BOOL)
public ??1CException@@UAE@XZ     ;~CException
public ?ReportError@CException@@UAEHII@Z   ; ReportError

public ?AfxThrowNotSupportedException@@YGXXZ

extern ?AfxGetMainWnd@@YGPAVCWnd@@XZ:proc





;CCmdTarget
externdef ?OnCmdMsg@CCmdTarget@@UAEHIHPAXPAUAFX_CMDHANDLERINFO@@@Z:proc   ; (public: virtual int __thiscall CCmdTarget::OnCmdMsg(unsigned int,int,void *,struct AFX_CMDHANDLERINFO *))
externdef ?OnFinalRelease@CCmdTarget@@UAEXXZ:proc   ; (public: virtual void __thiscall CCmdTarget::OnFinalRelease(void))
externdef ?IsInvokeAllowed@CCmdTarget@@UAEHJ@Z:proc   ; (public: virtual int __thiscall CCmdTarget::IsInvokeAllowed(long))
externdef ?GetDispatchIID@CCmdTarget@@UAEHPAU_GUID@@@Z:proc   ; (public: virtual int __thiscall CCmdTarget::GetDispatchIID(struct _GUID *))
externdef ?GetTypeInfoCount@CCmdTarget@@UAEIXZ:proc   ; (public: virtual unsigned int __thiscall CCmdTarget::GetTypeInfoCount(void))
externdef ?GetTypeLibCache@CCmdTarget@@UAEPAVCTypeLibCache@@XZ:proc   ; (public: virtual class CTypeLibCache * __thiscall CCmdTarget::GetTypeLibCache(void))
externdef ?GetTypeLib@CCmdTarget@@UAEJKPAPAUITypeLib@@@Z:proc   ; (public: virtual long __thiscall CCmdTarget::GetTypeLib(unsigned long,struct ITypeLib * *))
externdef ?GetCommandMap@CCmdTarget@@UBEPBXXZ:proc   ; (public: virtual void const * __thiscall CCmdTarget::GetCommandMap(void)const )
externdef ?GetDispatchMap@CCmdTarget@@UBEPBXXZ:proc   ; (public: virtual void const * __thiscall CCmdTarget::GetDispatchMap(void)const )
externdef ?GetConnectionMap@CCmdTarget@@UBEPBXXZ:proc   ; (public: virtual void const * __thiscall CCmdTarget::GetConnectionMap(void)const )
externdef ?GetInterfaceMap@CCmdTarget@@UBEPBXXZ:proc   ; (public: virtual void const * __thiscall CCmdTarget::GetInterfaceMap(void)const )
externdef ?GetEventSinkMap@CCmdTarget@@UBEPBXXZ:proc   ; (public: virtual void const * __thiscall CCmdTarget::GetEventSinkMap(void)const )
externdef ?OnCreateAggregates@CCmdTarget@@UAEHXZ:proc   ; (public: virtual int __thiscall CCmdTarget::OnCreateAggregates(void))
externdef ?GetInterfaceHook@CCmdTarget@@UAEPAUIUnknown@@PBX@Z:proc   ; (public: virtual struct IUnknown * __thiscall CCmdTarget::GetInterfaceHook(void const *))
externdef ?GetExtraConnectionPoints@CCmdTarget@@UAEHPAVCPtrArray@@@Z:proc   ; (public: virtual int __thiscall CCmdTarget::GetExtraConnectionPoints(class CPtrArray *))
externdef ?GetConnectionHook@CCmdTarget@@UAEPAUIConnectionPoint@@ABU_GUID@@@Z:proc   ; (public: virtual struct IConnectionPoint * __thiscall CCmdTarget::GetConnectionHook(struct _GUID const

;CWinThread
externdef ?PreTranslateMessage@CWinThread@@UAEHPAUtagMSG@@@Z:proc   ; (public: virtual int __thiscall CWinThread::PreTranslateMessage(struct tagMSG *))
externdef ?PumpMessage@CWinThread@@UAEHXZ:proc   ; (public: virtual int __thiscall CWinThread::PumpMessage(void))
externdef ?IsIdleMessage@CWinThread@@UAEHPAUtagMSG@@@Z:proc   ; (public: virtual int __thiscall CWinThread::IsIdleMessage(struct tagMSG *))
externdef ?ProcessMessageFilter@CWinThread@@UAEHHPAUtagMSG@@@Z:proc   ; (public: virtual int __thiscall CWinThread::ProcessMessageFilter(int,struct tagMSG *))
externdef ?GetMainWnd@CWinThread@@UAEPAVCWnd@@XZ:proc   ; (public: virtual class CWnd * __thiscall CWinThread::GetMainWnd(void))
externdef ?Delete@CWinThread@@UAEXXZ:proc   ; (public: virtual void __thiscall CWinThread::Delete(void))

;CWinApp
externdef ?GetRuntimeClass@CWinApp@@UBEPAUCRuntimeClass@@XZ:proc   ; (public: virtual struct CRuntimeClass * __thiscall CWinApp::GetRuntimeClass(void)const )
externdef ??1CWinApp@@UAE@XZ:proc   ; (public: virtual __thiscall CWinApp::~CWinApp(void))
externdef ?GetMessageMap@CWinApp@@UBEPBUAFX_MSGMAP@@XZ:proc   ; (public: virtual struct AFX_MSGMAP const * __thiscall CWinApp::GetMessageMap(void)const )
externdef ?Run@CWinApp@@UAEHXZ:proc   ; (public: virtual int __thiscall CWinApp::Run(void))
externdef ?ProcessWndProcException@CWinApp@@UAEJPAVCException@@PBUtagMSG@@@Z:proc   ; (public: virtual long __thiscall CWinApp::ProcessWndProcException(class CException *,struct tagMSG c
externdef ?OpenDocumentFile@CWinApp@@UAEPAVCDocument@@PBD@Z:proc   ; (public: virtual class CDocument * __thiscall CWinApp::OpenDocumentFile(char const *))
externdef ?AddToRecentFileList@CWinApp@@UAEXPBD@Z:proc   ; (public: virtual void __thiscall CWinApp::AddToRecentFileList(char const *))
externdef ?InitApplication@CWinApp@@UAEHXZ:proc   ; (public: virtual int __thiscall CWinApp::InitApplication(void))
externdef ?SaveAllModified@CWinApp@@UAEHXZ:proc   ; (public: virtual int __thiscall CWinApp::SaveAllModified(void))
externdef ?DoMessageBox@CWinApp@@UAEHPBDII@Z:proc   ; (public: virtual int __thiscall CWinApp::DoMessageBox(char const *,unsigned int,unsigned int))
externdef ?DoWaitCursor@CWinApp@@UAEXH@Z:proc   ; (public: virtual void __thiscall CWinApp::DoWaitCursor(int))
externdef ?OnDDECommand@CWinApp@@UAEHPAD@Z:proc   ; (public: virtual int __thiscall CWinApp::OnDDECommand(char *))
externdef ?WinHelpA@CWinApp@@UAEXKI@Z:proc   ; (public: virtual void __thiscall CWinApp::WinHelpA(unsigned long,unsigned int))
externdef ??0CWinApp@@QAE@PBD@Z:proc   ; (public: __thiscall CWinApp::CWinApp(char const *))

;GameApp
externdef ?InitInstance@GameApp@@UAEHXZ:proc   ; (public: virtual int __thiscall GameApp::InitInstance(void))
externdef ?OnIdle@GameApp@@UAEHJ@Z:proc   ; (public: virtual int __thiscall GameApp::OnIdle(long))
externdef ?ExitInstance@GameApp@@UAEHXZ:proc   ; (public: virtual int __thiscall GameApp::ExitInstance(void))





externdef ?LogMessage@@YAXVCString@@@Z:proc     ;  LogMessage
externdef ?g_GameDataRes@@3UGameDataRes@@A:byte    ; g_GameDataRes





;CString
public ?AfxThrowMemoryException@@YGXXZ    ; AfxThrowMemoryException
extern ?AfxGetEmptyString@@YGABVCString@@XZ:proc   ;AfxGetEmptyString in strcore.cpp
extern ??0CString@@QAE@XZ:proc ; CString::CString
extern ??0CString@@QAE@ABV0@@Z:proc   ; (public: __thiscall CString::CString(class CString const &))
extern ?AllocBuffer@CString@@IAEXH@Z:proc   ; (protected: void __thiscall CString::AllocBuffer(int))
extern ?Release@CString@@IAEXXZ:proc   ; (protected: void __thiscall CString::Release(void))
extern ?Release@CString@@KGXPAUCStringData@@@Z:proc   ; (protected: static void __stdcall CString::Release(struct CStringData *))
extern ?Empty@CString@@QAEXXZ:proc   ; (public: void __thiscall CString::Empty(void))
extern ?CopyBeforeWrite@CString@@IAEXXZ:proc   ; (protected: void __thiscall CString::CopyBeforeWrite(void))
extern ?AllocBeforeWrite@CString@@IAEXH@Z:proc   ; (protected: void __thiscall CString::AllocBeforeWrite(int))
extern ??1CString@@QAE@XZ:proc   ; (public: __thiscall CString::~CString(void))
extern ?AllocCopy@CString@@IBEXAAV1@HHH@Z:proc   ; (protected: void __thiscall CString::AllocCopy(class CString &,int,int,int)const )
extern ??0CString@@QAE@PBD@Z:proc   ; (public: __thiscall CString::CString(char const *))
;extern ??0CString@@QAE@PB_W@Z:proc   ; (public: __thiscall CString::CString(wchar_t const *))
extern ?AssignCopy@CString@@IAEXHPBD@Z:proc   ; (protected: void __thiscall CString::AssignCopy(int,char const *))
extern ??4CString@@QAEABV0@ABV0@@Z:proc   ; (public: class CString const & __thiscall CString::operator=(class CString const &))
extern ??4CString@@QAEABV0@PBD@Z:proc   ; (public: class CString const & __thiscall CString::operator=(char const *))
extern ??4CString@@QAEABV0@PB_W@Z:proc   ; (public: class CString const & __thiscall CString::operator=(wchar_t const *))
extern ?ConcatCopy@CString@@IAEXHPBDH0@Z:proc   ; (protected: void __thiscall CString::ConcatCopy(int,char const *,int,char const *))
extern ??H@YG?AVCString@@ABV0@0@Z:proc   ; (class CString __stdcall operator+(class CString const &,class CString const &))
extern ??H@YG?AVCString@@ABV0@PBD@Z:proc   ; (class CString __stdcall operator+(class CString const &,char const *))
extern ??H@YG?AVCString@@PBDABV0@@Z:proc   ; (class CString __stdcall operator+(char const *,class CString const &))
extern ?ConcatInPlace@CString@@IAEXHPBD@Z:proc   ; (protected: void __thiscall CString::ConcatInPlace(int,char const *))
extern ??YCString@@QAEABV0@PBD@Z:proc   ; (public: class CString const & __thiscall CString::operator+=(char const *))
extern ??YCString@@QAEABV0@D@Z:proc   ; (public: class CString const & __thiscall CString::operator+=(char))
extern ??YCString@@QAEABV0@ABV0@@Z:proc   ; (public: class CString const & __thiscall CString::operator+=(class CString const &))
extern ?GetBuffer@CString@@QAEPADH@Z:proc   ; (public: char * __thiscall CString::GetBuffer(int))
extern ?ReleaseBuffer@CString@@QAEXH@Z:proc   ; (public: void __thiscall CString::ReleaseBuffer(int))
extern ?GetBufferSetLength@CString@@QAEPADH@Z:proc   ; (public: char * __thiscall CString::GetBufferSetLength(int))
extern ?FreeExtra@CString@@QAEXXZ:proc   ; (public: void __thiscall CString::FreeExtra(void))
extern ?LockBuffer@CString@@QAEPADXZ:proc   ; (public: char * __thiscall CString::LockBuffer(void))
extern ?UnlockBuffer@CString@@QAEXXZ:proc   ; (public: void __thiscall CString::UnlockBuffer(void))
extern ?Find@CString@@QBEHD@Z:proc   ; (public: int __thiscall CString::Find(char)const )
extern ?FindOneOf@CString@@QBEHPBD@Z:proc   ; (public: int __thiscall CString::FindOneOf(char const *)const )
;extern ?MakeUpper@CString@@QAEXXZ:proc   ; (public: void __thiscall CString::MakeUpper(void))
extern ?MakeLower@CString@@QAEXXZ:proc   ; (public: void __thiscall CString::MakeLower(void))
extern ?MakeReverse@CString@@QAEXXZ:proc   ; (public: void __thiscall CString::MakeReverse(void))
extern ?SetAt@CString@@QAEXHD@Z:proc   ; (public: void __thiscall CString::SetAt(int,char))
extern ?CharToOemA@CString@@QAEXXZ:proc   ; (public: void __thiscall CString::CharToOemA(void))
;extern ?OemToCharA@CString@@QAEXXZ:proc   ; (public: void __thiscall CString::OemToCharA(void))
extern ?_wcstombsz@@YAHPADPB_WI@Z:proc   ; (int __cdecl _wcstombsz(char *,wchar_t const *,unsigned int))
extern ?_mbstowcsz@@YAHPA_WPBDI@Z:proc   ; (int __cdecl _mbstowcsz(wchar_t *,char const *,unsigned int))
extern ?AfxA2WHelper@@YGPA_WPA_WPBDH@Z:proc    ;  (wchar_t * __stdcall AfxA2WHelper(wchar_t *,char const *,int))
extern ?AfxW2AHelper@@YGPADPADPB_WH@Z:proc
extern ?LoadStringA@CString@@QAEHI@Z:proc   ; (public: int __thiscall CString::LoadStringA(unsigned int))

extern ??0CString@@QAE@DH@Z:proc   ; (public: __thiscall CString::CString(char,int))
extern ??0CString@@QAE@PBDH@Z:proc   ; (public: __thiscall CString::CString(char const *,int))
;extern ??4CString@@QAEABV0@D@Z:proc   ; (public: class CString const & __thiscall CString::operator=(char))
extern ??H@YG?AVCString@@ABV0@D@Z:proc   ; (class CString __stdcall operator+(class CString const &,char))
extern ??H@YG?AVCString@@DABV0@@Z:proc   ; (class CString __stdcall operator+(char,class CString const &))
extern ?Mid@CString@@QBE?AV1@H@Z:proc   ; (public: class CString __thiscall CString::Mid(int)const )
extern ?Mid@CString@@QBE?AV1@HH@Z:proc   ; (public: class CString __thiscall CString::Mid(int,int)const )
extern ?Right@CString@@QBE?AV1@H@Z:proc   ; (public: class CString __thiscall CString::Right(int)const )
extern ?Left@CString@@QBE?AV1@H@Z:proc   ; (public: class CString __thiscall CString::Left(int)const )
extern ?SpanIncluding@CString@@QBE?AV1@PBD@Z:proc   ; (public: class CString __thiscall CString::SpanIncluding(char const *)const )
extern ?SpanExcluding@CString@@QBE?AV1@PBD@Z:proc   ; (public: class CString __thiscall CString::SpanExcluding(char const *)const )
extern ?ReverseFind@CString@@QBEHD@Z:proc   ; (public: int __thiscall CString::ReverseFind(char)const )
extern ?Find@CString@@QBEHPBD@Z:proc   ; (public: int __thiscall CString::Find(char const *)const )
extern ?FormatV@CString@@IAEXPBDPAD@Z:proc   ; (protected: void __thiscall CString::FormatV(char const *,char *))
extern ?Format@CString@@QAAXPBDZZ:proc   ; (public: void __cdecl CString::Format(char const *,...))
extern ?Format@CString@@QAAXIZZ:proc   ; (public: void __cdecl CString::Format(unsigned int,...))
extern ?FormatMessageA@CString@@QAAXPBDZZ:proc   ; (public: void __cdecl CString::FormatMessageA(char const *,...))
;extern ?FormatMessageA@CString@@QAAXIZZ:proc   ; (public: void __cdecl CString::FormatMessageA(unsigned int,...))
extern ?TrimRight@CString@@QAEXXZ:proc   ; (public: void __thiscall CString::TrimRight(void))
extern ?TrimLeft@CString@@QAEXXZ:proc   ; (public: void __thiscall CString::TrimLeft(void))

; CStringArray
extern ?GetSize@CStringArray@@QBEHXZ :proc  ; CStringArray::GetSize()

;CPlex
extern ?Create@CPlex@@SGPAU1@AAPAU1@II@Z:proc   ; CPlex::Create 005dbbb6
extern ?FreeDataChain@CPlex@@QAEXXZ:proc ; CPlex::FreeDataChain  005DBBD6

;C++
extern ??2@YAPAXI@Z:proc ; operator new         5ddf54
extern ??3@YAXPAX@Z:proc ; operator delete      5ddf90



;File2
extern ??1File2@@UAE@XZ:proc   ; (public: virtual __thiscall File2::~File2(void))
extern ??0File2@@QAE@XZ:proc   ; (public: __thiscall File2::File2(void))
extern ??0File2@@QAE@PBDI@Z:proc   ; (public: __thiscall File2::File2(char const *,unsigned int))
extern ?Seek@File2@@UAEJJI@Z:proc   ; (public: virtual long __thiscall File2::Seek(long,unsigned int))
extern ?GetLength@File2@@UBEKXZ:proc   ; (public: virtual unsigned long __thiscall File2::GetLength(void)const )
extern ?Read@File2@@UAEIPAXI@Z:proc   ; (public: virtual unsigned int __thiscall File2::Read(void *,unsigned int))
extern ?Close@File2@@UAEXXZ:proc   ; (public: virtual void __thiscall File2::Close(void))
extern ?Open@File2@@UAEHPBDIPAVCFileException@@@Z:proc   ; (public: virtual int __thiscall File2::Open(char const *,unsigned int,class CFileException *))
extern ?Rewind@File2@@QAEXXZ:proc    ; (public: void __thiscall File2::Rewind(void))


;Resources
extern ?StaticAddResFile@Resources@@SAXPBD@Z:proc   ; (public: static void __cdecl Resources::StaticAddResFile(char const *))
extern ?StaticAddPath@Resources@@SAXPBD@Z:proc   ; (public: static void __cdecl Resources::StaticAddPath(char const *))
extern ?FUN_004e632d@Resources@@SAXPBD@Z:proc   ; (public: static void __cdecl Resources::FUN_004e632d(char const *))



;ResFile
extern ?ReadFromFile@ResBase@@QAEXPAVCFile@@H@Z:proc   ; (public: void __thiscall ResBase::ReadFromFile(class CFile *,int))
extern ?WriteToFile@ResBase@@QAEXPAVCFile@@H@Z:proc   ; (public: void __thiscall ResBase::WriteToFile(class CFile *,int))
extern ??0RegFile@@QAE@XZ:proc      ; (public: __thiscall RegFile::RegFile(void))
extern ??0RegFile@@QAE@PBD@Z:proc      ;  (public: __thiscall RegFile::RegFile(char const *))
extern ??1RegFile@@QAE@XZ:proc       ;   RegFile::~RegFile
extern ?GetSizedString@ResBase@@QAEHPBD00PADI@Z:proc   ; (public: int __thiscall ResBase::GetSizedString(char const *,char const *,char const *,char *,unsigned int))
extern ?GetValueString@ResBase@@QAE?AVCString@@PBD00@Z:proc   ; (public: class CString __thiscall ResBase::GetValueString(char const *,char const *,char const *))
extern ?SetString@ResBase@@QAEXPBD00@Z:proc   ; (public: void __thiscall ResBase::SetString(char const *,char const *,char const *))
extern ?GetInt@ResBase@@QAEHPBD0H@Z:proc   ; (public: int __thiscall ResBase::GetInt(char const *,char const *,int))
extern ?SetInt@ResBase@@QAEXPBD0H@Z:proc   ; (public: void __thiscall ResBase::SetInt(char const *,char const *,int))
extern ?GetDouble@ResBase@@QAENPBD0N@Z:proc   ; (public: double __thiscall ResBase::GetDouble(char const *,char const *,double))
extern ?GetInt8Array@ResBase@@QAEHPBD0PAV?$CArray@EABE@@@Z:proc   ; (public: int __thiscall ResBase::GetInt8Array(char const *,char const *,class CArray<uint8_t>
extern ?GetInt16Array@ResBase@@QAEHPBD0PAV?$CArray@GABG@@@Z:proc   ; (public: int __thiscall ResBase::GetInt16Array(char const *,char const *,class CArray<uint16_t>
extern ?GetInt32Array@ResBase@@QAEHPBD0PAV?$CArray@IABI@@@Z:proc   ; (public: int __thiscall ResBase::GetInt32Array(char const *,char const *,class CArray<uint32_t>
extern ?GetStringArray@ResBase@@QAEHPBD0PAVCStringArray@@@Z:proc   ; (public: int __thiscall ResBase::GetStringArray(char const *,char const *,class CStringArray *))
extern ?SetInt16Array@ResBase@@QAEXPBD0AAV?$CArray@GABG@@@Z:proc   ; (public: void __thiscall ResBase::SetInt16Array(char const *,char const *,class CArray<uint16_t>
extern ?SetInt32Array@ResBase@@QAEXPBD0AAV?$CArray@IABI@@@Z:proc   ; (public: void __thiscall ResBase::SetInt32Array(char const *,char const *,class CArray<uint32_t>






;TokenPos 
extern ??0TokenPos@@QAE@EEPAUMapStuff@@@Z:proc  ;  TokenPos::TokenPos(uint8_t _x, uint8_t _y, MapStuff* _inst)
extern ??0TokenPos@@QAE@XZ:proc  ; TokenPos::TokenPos()
extern ??0TokenPos@@QAE@ABU0@@Z:proc  ; TokenPos::TokenPos(struct TokenPos const &)
extern ??4TokenPos@@QAEAAU0@ABU0@@Z:proc   ; TokenPos::operator=(struct TokenPos const &)
extern ?Serialize@TokenPos@@QAEXAAVCArchive@@@Z:proc    ; TokenPos::Serialize(CArchive&)
extern ?GetX@TokenPos@@QBEEXZ:proc  ; TokenPos::GetX
extern ?GetY@TokenPos@@QBEEXZ:proc  ; TokenPos::GetY
extern ?GetYX@TokenPos@@QBEGXZ:proc   ; TokenPos::GetYX
extern ?GetXx@TokenPos@@QBEGXZ:proc   ; TokenPos::GetXx
extern ?GetYy@TokenPos@@QBEGXZ:proc   ; TokenPos::GetYy
extern ?SetCoords@TokenPos@@QAEXEE@Z:proc   ; TokenPos::SetCoords
extern ?SetCoords2@TokenPos@@QAEXGG@Z:proc   ; TokenPos::SetCoords2
extern ?Distance@TokenPos@@QBEEPBU1@@Z:proc   ; TokenPos::Distance
extern ?IsSameYX@TokenPos@@QBE_NPBU1@@Z:proc   ; TokenPos::IsSameYX

public ?FUN_0058a7e8@TokenPos@@QAE_NEE@Z    ;  TokenPos::FUN_0058a7e8
public ?FUN_00594200@TokenPos@@QAEXXZ   ;  TokenPos::FUN_00594200




extern ?MapStuff_Instance@@3PAUMapStuff@@A:dword   ;  MapStuff_Instance

;Token
;extern ??_7Token@@6B@:dword ; Token::`vftable`
extern ?classToken@Token@@2UCRuntimeClass@@A:dword ; Token::classToken

extern ??0Token@@QAE@XZ:proc ; Token::Token
extern ??0Token@@QAE@ABV0@@Z:proc ; Token::Token(const Token&)
extern ??0Token@@QAE@PBUTokenPos@@@Z:proc ; Token::Token(const TokenPos*)
extern ??0Token@@QAE@PBUTokenPos@@PAVPlayer@@@Z:proc ; Token::Token(const TokenPos*, Player*)

extern ??1Token@@UAE@XZ:proc ; Token::~Token 005283d6

public ?Serialize@Token@@UAEXAAVCArchive@@@Z ; Token::Serialize 0055AB39

extern ?VMethod1@Token@@UAEXXZ:proc ;  Token::VMethod1
extern ?VMethod2@Token@@UAEXXZ:proc ;  Token::VMethod2
extern ?VMethod3@Token@@UAEHXZ:proc ;  Token::VMethod3
extern ?VMethod4@Token@@UAEHXZ:proc ;  Token::VMethod4
extern ?VMethod5@Token@@UAEXXZ:proc ;  Token::VMethod5
extern ?VMethod6@Token@@UAEHXZ:proc ;  Token::VMethod6
extern ?VMethod7@Token@@UAEHXZ:proc ;  Token::VMethod7
extern ?VMethod8@Token@@UAEHXZ:proc ;  Token::VMethod8
extern ?VMethod9@Token@@UAEHXZ:proc ;  Token::VMethod9



;BitSet g_buildingIdSet     in token.cpp
extern _BldIdSet_Set:proc
extern _BldIdSet_Unset:proc
extern _BldIdSet_Clear:proc
extern _BldIdSet_AllocBit:proc



;Unit
externdef ?Serialize@Unit@@UAEXAAVCArchive@@@Z:proc   ; (public: virtual void __thiscall Unit::Serialize(class CArchive &))
externdef ?VMethod1@Unit@@UAEXXZ:proc   ; (public: virtual void __thiscall Unit::VMethod1(void))
externdef ?VMethod2@Unit@@UAEXXZ:proc   ; (public: virtual void __thiscall Unit::VMethod2(void))
externdef ?VMethod5@Unit@@UAEXXZ:proc   ; (public: virtual void __thiscall Unit::VMethod5(void))

externdef ?VMethod10@Unit@@UAEXXZ:proc   ; (public: virtual void __thiscall Unit::VMethod10(void))
externdef ?VMethod11@Unit@@UAEXPAV1@@Z:proc   ; (public: virtual void __thiscall Unit::VMethod11(class Unit *))

externdef ?VMethod17@Unit@@UAEIPAUUnitToHit@@PAV1@@Z:proc   ; (public: virtual unsigned int __thiscall Unit::VMethod17(struct UnitToHit *,class Unit *))

externdef ?VMethod25@Unit@@UAEHXZ:proc   ; (public: virtual int __thiscall Unit::VMethod25(void))

externdef ?FUN_0052931b@Unit@@QAEXABVCString@@@Z:proc   ; FUN_0052931b


extern ??0Unit@@QAE@XZ:proc    ; (public: __thiscall Unit::Unit(void))
extern ??0Unit@@QAE@ABVCString@@@Z:proc   ; (public: __thiscall Unit::Unit(class CString const &))
extern ??5@YGAAVCArchive@@AAV0@AAPAVUnit@@@Z:proc     ;  CArchive & __stdcall operator>>(class CArchive &,class Unit * &)



;Humanoid

externdef ?Serialize@Humanoid@@UAEXAAVCArchive@@@Z:proc   ; (public: virtual void __thiscall Humanoid::Serialize(class CArchive &))
externdef ?VMethod12@Humanoid@@UAEPAVItem@@PAV2@@Z:proc   ; (public: virtual void __thiscall Humanoid::VMethod12(class Item *))
externdef ?VMethod16@Humanoid@@UAEXPAVItem@@@Z:proc   ; (public: virtual void __thiscall Humanoid::VMethod16(class Item *))
extern ?VMethod18@Humanoid@@UAEXXZ:proc   ; (public: virtual void __thiscall Humanoid::VMethod18(void))
externdef ?VMethod21@Humanoid@@UAEXIPAVUnit@@H@Z:proc   ; (public: virtual void __thiscall Humanoid::VMethod21(unsigned int,class Unit *,int))
externdef ?VMethod22@Humanoid@@UAEXPAVUnit@@H@Z:proc   ; (public: virtual void __thiscall Humanoid::VMethod22(class Unit *,int))
externdef ?VMethod23@Humanoid@@UAEXPAVUnit@@IHH@Z:proc   ; (public: virtual void __thiscall Humanoid::VMethod23(class Unit *,unsigned int,int,int))
externdef ?VMethod24@Humanoid@@UAEXPAVUnit@@IHH@Z:proc   ; (public: virtual void __thiscall Humanoid::VMethod24(class Unit *,unsigned int,int,int))
externdef ?VMethod25@Humanoid@@UAEHXZ:proc   ; (public: virtual int __thiscall Humanoid::VMethod25(void))

externdef ?InitStats@Humanoid@@QAEXXZ:proc   ;    Humanoid::InitStats


;Human

extern ??0Human@@QAE@XZ:proc   ; Human::Human
extern ??0Human@@QAE@ABVCString@@HPBD@Z:proc    ; Human::Human(CString&, int, const char *)

externdef ?Serialize@Human@@UAEXAAVCArchive@@@Z:proc    ; Human::Serialize
externdef ?VMethod20@Human@@UAEXXZ:proc    ; Human::Serialize
externdef ?FUN_00532587@Human@@QAEXVCString@@HPBD@Z:proc   ;  Human::FUN_00532587(class CString,int,char const *)


;UnitList
extern ??0UnitList@@QAE@XZ:proc                         ; UnitList::UnitList
extern ??1UnitList@@QAE@XZ:proc                         ; UnitList::~UnitList
extern ?AddTail@UnitList@@QAEXPAVUnit@@@Z:proc          ; UnitList::AddTail
extern ?AddTailAllocId@UnitList@@QAEXPAVUnit@@@Z:proc   ; UnitList::AddTailAllocId
extern ?AddTailId6xxx@UnitList@@QAEXPAVUnit@@@Z:proc    ; UnitList::AddTailId6xxx



;UnitToHit
extern ??0UnitToHit@@QAE@XZ:proc   ; (public: __thiscall UnitToHit::UnitToHit(void))
extern ??0UnitToHit@@QAE@ABU0@@Z:proc   ; (public: __thiscall UnitToHit::UnitToHit(struct UnitToHit const &))
extern ??4UnitToHit@@QAEAAU0@ABU0@@Z:proc   ; (public: struct UnitToHit & __thiscall UnitToHit::operator=(struct UnitToHit const &))
extern ?AddHits@UnitToHit@@QAEXABU1@@Z:proc   ; (public: void __thiscall UnitToHit::AddHits(struct UnitToHit const &))
extern ?Serialize@UnitToHit@@QAEXAAVCArchive@@@Z:proc   ; (public: void __thiscall UnitToHit::Serialize(class CArchive &))

;Protections
extern ??0Protections@@QAE@XZ:proc   ; (public: __thiscall Protections::Protections(void))
extern ??0Protections@@QAE@ABU0@@Z:proc   ; (public: __thiscall Protections::Protections(struct Protections const &))
extern ??0Protections@@QAE@$$QAU0@@Z:proc   ; (public: __thiscall Protections::Protections(struct Protections &&))
extern ??4Protections@@QAEAAU0@ABU0@@Z:proc   ; (public: struct Protections & __thiscall Protections::operator=(struct Protections const &))
extern ??YProtections@@QAEAAU0@ABU0@@Z:proc   ; (public: struct Protections & __thiscall Protections::operator+=(struct Protections const &))
extern ??ZProtections@@QAEAAU0@ABU0@@Z:proc   ; (public: struct Protections & __thiscall Protections::operator-=(struct Protections const &))
extern ?Clear@Protections@@QAEXXZ:proc   ; (public: void __thiscall Protections::Clear(void))
extern ?Serialize@Protections@@QAEXAAVCArchive@@@Z:proc   ; (public: void __thiscall Protections::Serialize(class CArchive &))

;EquipmentExtra
extern ??0EquipmentExtra@@QAE@XZ:proc   ; (public: __thiscall EquipmentExtra::EquipmentExtra(void))
extern ??4EquipmentExtra@@QAEAAU0@ABU0@@Z:proc   ; (public: struct EquipmentExtra & __thiscall EquipmentExtra::operator=(struct EquipmentExtra const &))
extern ?Serialize@EquipmentExtra@@QAEXAAVCArchive@@@Z:proc   ; (public: void __thiscall EquipmentExtra::Serialize(class CArchive &))
extern ?AddToUnit@EquipmentExtra@@QAEXPAVUnit@@@Z:proc   ; (public: void __thiscall EquipmentExtra::AddToUnit(class Unit *))


;inventory
extern ?PutItemIntoBagAtDefault@Inventory@@QAEXPAVItem@@@Z:proc   ;   (public: void __thiscall Inventory::PutItemIntoBagAtDefault(class Item *))
externdef ?PutItemIntoBag@Inventory@@QAEXHPAVItem@@@Z:proc   ; (public: void __thiscall Inventory::PutItemIntoBag(int,class Item *))




;Spell

extern ?RefreshForHumanoid@SpellBook@@QAEXPAVHumanoid@@@Z:proc  ; SpellBook::RefreshForHumanoid()
externdef ??0Spell@@QAE@XZ:proc   ;  Spell::Spell(void)
externdef ??0Spell@@QAE@E@Z:proc  ;  Spell::Spell(uint8_t)
externdef ??0Spell@@QAE@ABVCString@@@Z:proc    ;  Spell::Spell(const CString&)
externdef ?sub_53940D@Spell@@QAEXPAVUnit@@@Z:proc     ;  Spell::sub_53940D




;Packets
externdef ?Inst@PacketJoin@@2V1@A:byte    ; PacketJoin::Inst



;NetStru1
externdef ?g_NetStru1_main@@3VNetStru1@@A:byte  

externdef ?FUN_0051d49b@NetStru1@@QAEXPAVPlayer@@@Z :proc   ;  NetStru1::FUN_0051d49b
externdef ?FUN_00518544@NetStru1@@QAEPAUNetStru2@@G@Z :proc   ; NetStru1::FUN_00518544(unsigned short)
externdef ?FUN_005170b6@NetStru1@@QAEXPAUNetStru2@@@Z :proc   ; NetStru1::FUN_005170b6(struct NetStru2 *)
externdef ?FUN_0051800f@NetStru1@@QAEXXZ :proc   ;  NetStru1::FUN_0051800f(void)
externdef ?FUN_005186cd@NetStru1@@QAEXPAVPacket@@@Z :proc   ; NetStru1::FUN_005186cd(class Packet *)
externdef ?FUN_0051ce86@NetStru1@@QAEXIIPAVPlayer@@@Z :proc   ; NetStru1::FUN_0051ce86(unsigned int,unsigned int,class Player *)
externdef ?FUN_0051ceac@NetStru1@@QAEXIPAVPlayer@@@Z :proc   ;  NetStru1::FUN_0051ceac(unsigned int,class Player *)
externdef ?FUN_0051c748@NetStru1@@QAEXPAVPacket@@@Z :proc   ; NetStru1::FUN_0051c748(class Packet *)
externdef ?FUN_005188db@NetStru1@@QAEXXZ :proc   ; NetStru1::FUN_005188db(void)


;NetStru2
externdef ?FUN_00518544@NetStru1@@QAEPAUNetStru2@@G@Z:proc

;Server
externdef ?g_Server@@3PAUServer@@A : dword
externdef ?g_IsServer@@3HA : dword
externdef ?g_ShutdownIn@@3HA : dword
externdef ?g_CurrentMapIndex@@3HA : dword
externdef _g_MapNames : dword  ; CStringArray
externdef ?g_MapDurations@@3V?$CArray@HABH@@A : dword  ; CList<int32_t>
externdef ?g_GameType@@3HA : dword

externdef ?sub_4FBB79@Server@@QAEXXZ :proc
externdef ?sub_4F1E2A@Server@@QAEXXZ :proc
externdef ?sub_4FA5C4@Server@@QAEXXZ :proc
externdef ?sub_4F94C0@Server@@QAEXH@Z :proc
externdef ?sub_4FA78E@Server@@QAEXH@Z :proc
externdef ?sub_4F8831@Server@@QAEXXZ :proc


EXTERN ?FUN_004ff439@Server@@QAEXPAVPlayer@@H@Z :proc  ; Server::FUN_004ff439() in game.cpp


;Player 
externdef ?FUN_00534AC1@Player@@QAEXHI@Z :proc

;PlayersList
externdef ?g_PlayersList@@3PAVPlayersList@@A :dword
externdef ?FUN_00534ddd@PlayersList@@QAEXXZ  : proc
extern ?CountHumanPlayers@PlayersList@@QAEHXZ :proc







;CListBox2  in C++
extern  ??0CListBox2@@QAE@XZ  :proc
extern  ??1CListBox2@@UAE@XZ  :proc



;CListBox
externdef ?GetCurSel@CListBox@@QBEHXZ:proc
externdef ??0CListBox@@QAE@XZ : proc    ;  CListBox::CListBox(void)
externdef ??1CListBox@@UAE@XZ : proc    ;  CListBox::~CListBox(void)
externdef ?OnChildNotify@CListBox@@UAEHIIJPAJ@Z :proc   ; (public: virtual int __thiscall CListBox::OnChildNotify(unsigned int,unsigned int,long,long *))
externdef ?DrawItem@CListBox@@UAEXPAUtagDRAWITEMSTRUCT@@@Z :proc   ; (public: virtual void __thiscall CListBox::DrawItem(struct tagDRAWITEMSTRUCT *))
externdef ?MeasureItem@CListBox@@UAEXPAUtagMEASUREITEMSTRUCT@@@Z :proc   ; (public: virtual void __thiscall CListBox::MeasureItem(struct tagMEASUREITEMSTRUCT *))
externdef ?CompareItem@CListBox@@UAEHPAUtagCOMPAREITEMSTRUCT@@@Z :proc   ; (public: virtual int __thiscall CListBox::CompareItem(struct tagCOMPAREITEMSTRUCT *))
externdef ?DeleteItem@CListBox@@UAEXPAUtagDELETEITEMSTRUCT@@@Z :proc   ; (public: virtual void __thiscall CListBox::DeleteItem(struct tagDELETEITEMSTRUCT *))
externdef ?VKeyToItem@CListBox@@UAEHII@Z :proc   ; (public: virtual int __thiscall CListBox::VKeyToItem(unsigned int,unsigned int))
externdef ?CharToItem@CListBox@@UAEHII@Z :proc   ; (public: virtual int __thiscall CListBox::CharToItem(unsigned int,unsigned int))
externdef ?GetRuntimeClass@CListBox@@UBEPAUCRuntimeClass@@XZ  :proc


;CEdit
externdef ?GetRuntimeClass@CEdit@@UBEPAUCRuntimeClass@@XZ :proc   ; (public: virtual struct CRuntimeClass * __thiscall CEdit::GetRuntimeClass(void)const )
externdef ??0CEdit@@QAE@XZ :proc   ; (public: __thiscall CEdit::CEdit(void))
externdef ??1CEdit@@UAE@XZ :proc   ; (public: virtual __thiscall CEdit::~CEdit(void))
externdef ?SetSel@CEdit@@QAEXKH@Z :proc  ;  CEdit::SetSel


;CWnd
externdef ?messageMap@CWnd@@2UAFX_MSGMAP@@B :dword   ; (public: static struct AFX_MSGMAP const CWnd::messageMap)
externdef ?CheckAutoCenter@CWnd@@UAEHXZ :proc   ; (public: virtual int __thiscall CWnd::CheckAutoCenter(void))
externdef ?ContinueModal@CWnd@@UAEHXZ :proc   ; (public: virtual int __thiscall CWnd::ContinueModal(void))
externdef ?GetRuntimeClass@CWnd@@UBEPAUCRuntimeClass@@XZ :proc   ; (public: virtual struct CRuntimeClass * __thiscall CWnd::GetRuntimeClass(void)const )
externdef ?OnFinalRelease@CWnd@@UAEXXZ :proc   ; (public: virtual void __thiscall CWnd::OnFinalRelease(void))
externdef ?PreSubclassWindow@CWnd@@UAEXXZ :proc   ; (public: virtual void __thiscall CWnd::PreSubclassWindow(void))
externdef ?Create@CWnd@@UAEHPBD0KABUtagRECT@@PAV1@IPAX@Z :proc   ; (public: virtual int __thiscall CWnd::Create(char const *,char const *,unsigned long,struct tagRECT const &,class CWnd *,unsigned int,v
externdef ?DestroyWindow@CWnd@@UAEHXZ :proc   ; (public: virtual int __thiscall CWnd::DestroyWindow(void))
externdef ?PreCreateWindow@CWnd@@UAEHAAUtagCREATESTRUCTA@@@Z :proc   ; (public: virtual int __thiscall CWnd::PreCreateWindow(struct tagCREATESTRUCTA &))
externdef ?CalcWindowRect@CWnd@@UAEXPAUtagRECT@@I@Z :proc   ; (public: virtual void __thiscall CWnd::CalcWindowRect(struct tagRECT *,unsigned int))
externdef ?OnToolHitTest@CWnd@@UBEHVCPoint@@PAUtagTOOLINFOA@@@Z :proc   ; (public: virtual int __thiscall CWnd::OnToolHitTest(class CPoint,struct tagTOOLINFOA *)const )
externdef ?GetScrollBarCtrl@CWnd@@UBEPAXH@Z :proc   ; (public: virtual void * __thiscall CWnd::GetScrollBarCtrl(int)const )
externdef ?WinHelpA@CWnd@@UAEXKI@Z :proc   ; (public: virtual void __thiscall CWnd::WinHelpA(unsigned long,unsigned int))
externdef ?EndModalLoop@CWnd@@UAEXH@Z :proc   ; (public: virtual void __thiscall CWnd::EndModalLoop(int))
externdef ?OnCommand@CWnd@@UAEHIJ@Z :proc   ; (public: virtual int __thiscall CWnd::OnCommand(unsigned int,long))
externdef ?OnNotify@CWnd@@UAEHIJPAJ@Z :proc   ; (public: virtual int __thiscall CWnd::OnNotify(unsigned int,long,long *))
externdef ?GetSuperWndProcAddr@CWnd@@UAEPAP6GJPAUHWND__@@IIJ@ZXZ :proc   ; (public: virtual long (__stdcall** __thiscall CWnd::GetSuperWndProcAddr(void))(struct HWND__ *,unsigned int,unsigned int,long))
externdef ?DoDataExchange@CWnd@@UAEXPAX@Z :proc   ; (public: virtual void __thiscall CWnd::DoDataExchange(void *))
externdef ?BeginModalState@CWnd@@UAEXXZ :proc   ; (public: virtual void __thiscall CWnd::BeginModalState(void))
externdef ?EndModalState@CWnd@@UAEXXZ :proc   ; (public: virtual void __thiscall CWnd::EndModalState(void))
externdef ?PreTranslateMessage@CWnd@@UAEHPAUtagMSG@@@Z :proc   ; (public: virtual int __thiscall CWnd::PreTranslateMessage(struct tagMSG *))
externdef ?OnAmbientProperty@CWnd@@UAEHPAXJPAUtagVARIANT@@@Z :proc   ; (public: virtual int __thiscall CWnd::OnAmbientProperty(void *,long,struct tagVARIANT *))
externdef ?WindowProc@CWnd@@UAEJIIJ@Z :proc   ; (public: virtual long __thiscall CWnd::WindowProc(unsigned int,unsigned int,long))
externdef ?OnWndMsg@CWnd@@UAEHIIJPAJ@Z :proc   ; (public: virtual int __thiscall CWnd::OnWndMsg(unsigned int,unsigned int,long,long *))
externdef ?DefWindowProcA@CWnd@@UAEJIIJ@Z :proc   ; (public: virtual long __thiscall CWnd::DefWindowProcA(unsigned int,unsigned int,long))
externdef ?PostNcDestroy@CWnd@@UAEXXZ :proc   ; (public: virtual void __thiscall CWnd::PostNcDestroy(void))
externdef ?OnChildNotify@CWnd@@UAEHIIJPAJ@Z :proc   ; (public: virtual int __thiscall CWnd::OnChildNotify(unsigned int,unsigned int,long,long *))
externdef ?IsFrameWnd@CWnd@@UBEHXZ :proc   ; (public: virtual int __thiscall CWnd::IsFrameWnd(void)const )
externdef ?SetOccDialogInfo@CWnd@@UAEHPAU_AFX_OCC_DIALOG_INFO@@@Z :proc   ; (public: virtual int __thiscall CWnd::SetOccDialogInfo(struct _AFX_OCC_DIALOG_INFO *))
externdef ?OnChildNotify@CWnd@@UAEHIIJPAJ@Z  :proc    ;   CWnd:: OnChildNotify
externdef ?GetWindowTextA@CWnd@@QBEXAAVCString@@@Z  :proc   ; CWnd::GetWindowTextA
externdef ?PostMessageA@CWnd@@QAEHIIJ@Z  :proc  ; CWnd::PostMessage



;CDialog
externdef ??0CDialog@@QAE@XZ :proc   ; (public: __thiscall CDialog::CDialog(void))
externdef ??0CDialog@@QAE@PBDPAVCWnd@@@Z :proc   ; (public: __thiscall CDialog::CDialog(char const *,class CWnd *))
externdef ??0CDialog@@QAE@IPAVCWnd@@@Z :proc   ; (public: __thiscall CDialog::CDialog(unsigned int,class CWnd *))
externdef ??1CDialog@@UAE@XZ :proc   ; (public: virtual __thiscall CDialog::~CDialog(void))


;CStatic
externdef ??0CStatic@@QAE@XZ :proc   ; (public: __thiscall CStatic::CStatic(void))
externdef ??1CStatic@@UAE@XZ :proc   ; (public: virtual __thiscall CStatic::~CStatic(void))


;CStatusBar
externdef ??0CStatusBar@@QAE@XZ :proc   ; (public: __thiscall CStatusBar::CStatusBar(void))
externdef ??1CStatusBar@@UAE@XZ :proc   ; (public: virtual __thiscall CStatusBar::~CStatusBar(void))

;CEdit2
extern ??0CEdit2@@QAE@XZ :proc   ; (public: __thiscall CEdit2::CEdit2(void))
extern ??1CEdit2@@UAE@XZ :proc   ; (public: virtual __thiscall CEdit2::~CEdit2(void))



; MainWindow
externdef ?sub_484259@MainWindow@@QAEXXZ :proc

externdef ?sub_48A756@MainWindow@@QAEXXZ :proc
extern ?SetSpeed@MainWindow@@QAE_NH@Z :proc ; MainWindow::SetSpeed


externdef @sub_4271E6@4 :proc
externdef @sub_49585B@4 :proc
externdef @sub_48A747@0 :proc

externdef _dword_6D1660 :dword
externdef _byte_642C68 :byte



;TxtFile

extern ?AllLines@TxtFile@@2V?$CArray@PADABQAD@@A  : byte  ; TxtFile::AllLines

extern ??0TxtFile@@QAE@XZ :proc   ; (public: __thiscall TxtFile::TxtFile(void))
extern ??1TxtFile@@UAE@XZ :proc   ; (public: virtual __thiscall TxtFile::~TxtFile(void))
extern ?LoadChunkFile@TxtFile@@QAEXPBD@Z :proc   ; (public: void __thiscall TxtFile::LoadChunkFile(char const *))
extern ?Free@TxtFile@@QAEXXZ :proc   ; (public: void __thiscall TxtFile::Free(void))
extern ?GetLine@TxtFile@@QAEPADH@Z :proc   ; (public: char * __thiscall TxtFile::GetLine(int))
extern ?GetCount@TxtFile@@QAEHXZ   :proc   ; TxtFile::GetCount