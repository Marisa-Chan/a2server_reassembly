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
public ?AfxLockGlobals@@YGXH@Z
public ?AfxUnlockGlobals@@YGXH@Z
extern ?ReadObject@CArchive@@QAEPAVCObject@@PBUCRuntimeClass@@@Z : proc     ;  CArchive::ReadObject  005e848c
extern ?WriteObject@CArchive@@QAEXPBVCObject@@@Z : proc    ; CArchive::WriteObject   5e840f
public ?Flush@CArchive@@QAEXXZ
public ?FillBuffer@CArchive@@QAEXI@Z
externdef ??0CArchive@@QAE@PAVCFile@@IHPAX@Z :proc  ; CArchive::CArchive(CFile*, uint, int, void*)
externdef ??1CArchive@@QAE@XZ :proc                 ; CArchive::~CArchive()
externdef ?Close@CArchive@@QAEXXZ :proc             ; CArchive::Close()

public ??5@YGAAVCArchive@@AAV0@AAVCString@@@Z
public ??6@YGAAVCArchive@@AAV0@ABVCString@@@Z




externdef ?AfxRegisterWndClass@@YGPBDIPAUHICON__@@PAUHBRUSH__@@0@Z : proc
public ?AfxGetModuleThreadState@@YGPAVAFX_MODULE_THREAD_STATE@@XZ  ; AfxGetModuleThreadState(); 005F4DB8
public ??0AFX_CLASSINIT@@QAE@PAUCRuntimeClass@@@Z  ; AFX_CLASSINIT::AFX_CLASSINIT(CRuntimeClass *)
public ??0AFX_EXCEPTION_LINK@@QAE@XZ  ;AFX_EXCEPTION_LINK::AFX_EXCEPTION_LINK
public ?AfxTryCleanup@@YGXXZ ; AfxTryCleanup
public ?AfxGetModuleState@@YGPAVAFX_MODULE_STATE@@XZ
public ?AfxGetThread@@YGPAVCWinThread@@XZ
public ?AfxWinInit@@YGHPAUHINSTANCE__@@0PADH@Z  ; BOOL AfxWinInit(HINSTANCE, HINSTANCE, LPTSTR, int); 005F658E
public ?AfxWinTerm@@YGXXZ  ; void AfxWinTerm(); 005F6E92

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


extern ?AfxMessageBox@@YAHPBDII@Z  :proc


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
externdef ??1CWinThread@@UAE@XZ : proc

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
externdef ?ExitInstance@CWinApp@@UAEHXZ:proc   ; (public: virtual int __thiscall GameApp::ExitInstance(void))


;CString
public ?AfxThrowMemoryException@@YGXXZ    ; AfxThrowMemoryException
extern ?AfxGetEmptyString@@YGABVCString@@XZ:proc   ;AfxGetEmptyString in strcore.cpp
extern ??0CString@@QAE@XZ:proc ; CString::CString
extern ??0CString@@QAE@ABV0@@Z:proc   ; (public: __thiscall CString::CString(class CString const &))
extern ?Empty@CString@@QAEXXZ:proc   ; (public: void __thiscall CString::Empty(void))
extern ??1CString@@QAE@XZ:proc   ; (public: __thiscall CString::~CString(void))
extern ??0CString@@QAE@PBD@Z:proc   ; (public: __thiscall CString::CString(char const *))
;extern ??0CString@@QAE@PB_W@Z:proc   ; (public: __thiscall CString::CString(wchar_t const *))
extern ??4CString@@QAEABV0@ABV0@@Z:proc   ; (public: class CString const & __thiscall CString::operator=(class CString const &))
extern ??4CString@@QAEABV0@PBD@Z:proc   ; (public: class CString const & __thiscall CString::operator=(char const *))
extern ??4CString@@QAEABV0@PB_W@Z:proc   ; (public: class CString const & __thiscall CString::operator=(wchar_t const *))
extern ??H@YG?AVCString@@ABV0@0@Z:proc   ; (class CString __stdcall operator+(class CString const &,class CString const &))
extern ??H@YG?AVCString@@ABV0@PBD@Z:proc   ; (class CString __stdcall operator+(class CString const &,char const *))
extern ??H@YG?AVCString@@PBDABV0@@Z:proc   ; (class CString __stdcall operator+(char const *,class CString const &))
extern ??YCString@@QAEABV0@PBD@Z:proc   ; (public: class CString const & __thiscall CString::operator+=(char const *))
extern ??YCString@@QAEABV0@D@Z:proc   ; (public: class CString const & __thiscall CString::operator+=(char))
extern ??YCString@@QAEABV0@ABV0@@Z:proc   ; (public: class CString const & __thiscall CString::operator+=(class CString const &))
extern ?GetBuffer@CString@@QAEPADH@Z:proc   ; (public: char * __thiscall CString::GetBuffer(int))
extern ?ReleaseBuffer@CString@@QAEXH@Z:proc   ; (public: void __thiscall CString::ReleaseBuffer(int))
extern ?GetBufferSetLength@CString@@QAEPADH@Z:proc   ; (public: char * __thiscall CString::GetBufferSetLength(int))
extern ?Find@CString@@QBEHD@Z:proc   ; (public: int __thiscall CString::Find(char)const )
extern ?FindOneOf@CString@@QBEHPBD@Z:proc   ; (public: int __thiscall CString::FindOneOf(char const *)const )
;extern ?MakeUpper@CString@@QAEXXZ:proc   ; (public: void __thiscall CString::MakeUpper(void))
extern ?MakeLower@CString@@QAEXXZ:proc   ; (public: void __thiscall CString::MakeLower(void))
extern ?SetAt@CString@@QAEXHD@Z:proc   ; (public: void __thiscall CString::SetAt(int,char))
;extern ?OemToCharA@CString@@QAEXXZ:proc   ; (public: void __thiscall CString::OemToCharA(void))
extern ?_wcstombsz@@YAHPADPB_WI@Z:proc   ; (int __cdecl _wcstombsz(char *,wchar_t const *,unsigned int))
extern ?_mbstowcsz@@YAHPA_WPBDI@Z:proc   ; (int __cdecl _mbstowcsz(wchar_t *,char const *,unsigned int))
extern ?AfxA2WHelper@@YGPA_WPA_WPBDH@Z:proc    ;  (wchar_t * __stdcall AfxA2WHelper(wchar_t *,char const *,int))
extern ?LoadStringA@CString@@QAEHI@Z:proc   ; (public: int __thiscall CString::LoadStringA(unsigned int))

;extern ??4CString@@QAEABV0@D@Z:proc   ; (public: class CString const & __thiscall CString::operator=(char))
extern ?Mid@CString@@QBE?AV1@H@Z:proc   ; (public: class CString __thiscall CString::Mid(int)const )
extern ?Mid@CString@@QBE?AV1@HH@Z:proc   ; (public: class CString __thiscall CString::Mid(int,int)const )
extern ?Left@CString@@QBE?AV1@H@Z:proc   ; (public: class CString __thiscall CString::Left(int)const )
;extern ?FormatMessageA@CString@@QAAXIZZ:proc   ; (public: void __cdecl CString::FormatMessageA(unsigned int,...))
extern ?TrimLeft@CString@@QAEXXZ:proc   ; (public: void __thiscall CString::TrimLeft(void))

; CStringArray
extern ?GetSize@CStringArray@@QBEHXZ :proc  ; CStringArray::GetSize()

;CPlex
extern ?Create@CPlex@@SGPAU1@AAPAU1@II@Z:proc   ; CPlex::Create 005dbbb6
extern ?FreeDataChain@CPlex@@QAEXXZ:proc ; CPlex::FreeDataChain  005DBBD6

;C++
extern ??2@YAPAXI@Z:proc ; operator new         5ddf54
extern ??3@YAXPAX@Z:proc ; operator delete      5ddf90


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
externdef ?GetText@CListBox@@QBEXHAAVCString@@@Z : proc
externdef ?AddString@CListBox@@QAEHPBD@Z  : proc
externdef ?ResetContent@CListBox@@QAEXXZ : proc
externdef ?SelectString@CListBox@@QAEHHPBD@Z : proc
externdef ?SetCurSel@CListBox@@QAEXH@Z : proc


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
externdef ?OnDestroy@CWnd@@IAEXXZ :proc   ; CWnd::OnDestroy(void) - 5e130c
externdef ?OnDestroy@CFrameWnd@@IAEXXZ : proc  ; CFrameWnd::OnDestroy(void) - 5f1251
externdef ?OnSetFocus@CFrameWnd@@IAEXPAVCWnd@@@Z : proc  ; CFrameWnd::OnSetFocus(CWnd*) - 5f183f
externdef ?OnClose@CFrameWnd@@IAEXXZ : proc  ; CFrameWnd::OnClose(void) - 5f1148
externdef ?OnSize@CFrameWnd@@IAEXIHH@Z : proc  ; CFrameWnd::OnSize(UINT,int,int) - 5f2550
externdef ??0CPaintDC@@QAE@PAVCWnd@@@Z : proc  ; CPaintDC::CPaintDC(CWnd*) - 5e8049
externdef ??1CPaintDC@@UAE@XZ : proc  ; CPaintDC::~CPaintDC(void) - 5e80bb
externdef ?OnCreate@CFrameWnd@@IAEHPAUtagCREATESTRUCTA@@@Z : proc  ; CFrameWnd::OnCreate(LPCREATESTRUCT) - 5f0e80
externdef ?Create@CListBox@@QAEHKABUtagRECT@@PAVCWnd@@I@Z : proc  ; CListBox::Create(DWORD,const RECT&,CWnd*,UINT) - 5f27ed (non-virtual MFC inline wrapper)
externdef ?Create@CStatic@@QAEHPBDKABUtagRECT@@PAVCWnd@@I@Z : proc  ; CStatic::Create(LPCTSTR,DWORD,const RECT&,CWnd*,UINT) - 5f26e0 (non-virtual MFC inline wrapper)
externdef ?Create@CEdit@@QAEHKABUtagRECT@@PAVCWnd@@I@Z : proc  ; CEdit::Create(DWORD,const RECT&,CWnd*,UINT) - 5f2a89 (non-virtual MFC inline wrapper)
externdef ?Create@CStatusBar@@QAEHPAVCWnd@@KI@Z : proc  ; CStatusBar::Create(CWnd*,DWORD,UINT) - 5f5696
externdef ?SetActiveWindow@CWnd@@QAEPAV1@XZ : proc  ; CWnd::SetActiveWindow(void) - 496830
externdef ?Create@CDialog@@QAEHPBDPAVCWnd@@@Z : proc  ; CDialog::Create(LPCTSTR,CWnd*) - 5dfe8a
externdef ?PreCreateWindow@CWnd@@UAEHAAUtagCREATESTRUCTA@@@Z :proc   ; (public: virtual int __thiscall CWnd::PreCreateWindow(struct tagCREATESTRUCTA &))
externdef ?CalcWindowRect@CWnd@@UAEXPAUtagRECT@@I@Z :proc   ; (public: virtual void __thiscall CWnd::CalcWindowRect(struct tagRECT *,unsigned int))
externdef ?OnToolHitTest@CWnd@@UBEHVCPoint@@PAUtagTOOLINFOA@@@Z :proc   ; (public: virtual int __thiscall CWnd::OnToolHitTest(class CPoint,struct tagTOOLINFOA *)const )
externdef ?GetScrollBarCtrl@CWnd@@UBEPAXH@Z :proc   ; (public: virtual void * __thiscall CWnd::GetScrollBarCtrl(int)const )
externdef ?WinHelpA@CWnd@@UAEXKI@Z :proc   ; (public: virtual void __thiscall CWnd::WinHelpA(unsigned long,unsigned int))
externdef ?EndModalLoop@CWnd@@UAEXH@Z :proc   ; (public: virtual void __thiscall CWnd::EndModalLoop(int))
externdef ?OnCommand@CWnd@@UAEHIJ@Z :proc   ; (public: virtual int __thiscall CWnd::OnCommand(unsigned int,long))
externdef ?OnNotify@CWnd@@UAEHIJPAJ@Z :proc   ; (public: virtual int __thiscall CWnd::OnNotify(unsigned int,long,long *))
externdef ?GetSuperWndProcAddr@CWnd@@UAEPAP6GJPAUHWND__@@IIJ@ZXZ :proc   ; (public: virtual long (__stdcall** __thiscall CWnd::GetSuperWndProcAddr(void))(struct HWND__ *,unsigned int,unsigned int,long))
externdef ?DoDataExchange@CWnd@@MAEXPAVCDataExchange@@@Z :proc   ; (public: virtual void __thiscall CWnd::DoDataExchange(void *))
externdef ?DDX_Control@@YGXPAVCDataExchange@@HAAVCWnd@@@Z :proc   ; DDX_Control(CDataExchange*, int, CWnd&)
externdef ?DDX_Text@@YGXPAVCDataExchange@@HAAVCString@@@Z :proc   ; DDX_Text(CDataExchange*, int, CString&)
externdef ?DDV_MaxChars@@YGXPAVCDataExchange@@ABVCString@@H@Z :proc   ; DDV_MaxChars(CDataExchange*, const CString&, int)
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
externdef ?SendMessageA@CWnd@@QAEJIIJ@Z : proc  ; CWnd::SendMessage

externdef ?ShowWindow@CWnd@@QAEHH@Z  :proc  ;CWnd::ShowWindow
externdef ?SetWindowTextA@CWnd@@QAEXPBD@Z   :proc
externdef ?SetDlgItemTextA@CWnd@@QAEXHPBD@Z : proc

externdef ?GetClientRect@CWnd@@QBEXPAUtagRECT@@@Z : proc
externdef ?ModifyStyle@CWnd@@QAEHKKI@Z : proc
externdef ?SetWindowPos@CWnd@@QAEHPBV1@HHHHI@Z : proc

externdef ?EnableWindow@CWnd@@QAEHH@Z :proc   ; CWnd::EnableWindow(int) - 5e4721
externdef ?UpdateData@CWnd@@QAEHH@Z :proc     ; CWnd::UpdateData(int) - 5e31fc

;CFrameWnd
externdef ??0CFrameWnd@@QAE@XZ : proc
externdef ??1CFrameWnd@@UAE@XZ : proc

externdef ?messageMap@CFrameWnd@@2UAFX_MSGMAP@@B : dword

externdef ?RecalcLayout@CFrameWnd@@UAEXH@Z  : proc
externdef ?PostNcDestroy@CFrameWnd@@UAEXXZ  : proc
externdef ?OnUpdateFrameTitle@CFrameWnd@@UAEXH@Z  : proc
externdef ?OnUpdateFrameMenu@CFrameWnd@@UAEXPAUHMENU__@@@Z  : proc
externdef ?OnSetPreviewMode@CFrameWnd@@UAEXHPAX@Z  : proc
externdef ?GetMessageString@CFrameWnd@@UBEXIAAVCString@@@Z  : proc
externdef ?ExitHelpMode@CFrameWnd@@UAEXXZ  : proc
externdef ?EndModalState@CFrameWnd@@UAEXXZ  : proc
externdef ?DelayUpdateFrameMenu@CFrameWnd@@UAEXPAUHMENU__@@@Z  : proc
externdef ?BeginModalState@CFrameWnd@@UAEXXZ  : proc
externdef ?ActivateFrame@CFrameWnd@@UAEXH@Z  : proc
externdef ?GetDefaultAccelerator@CFrameWnd@@UAEPAUHACCEL__@@XZ  : proc
externdef ?GetRuntimeClass@CFrameWnd@@UBEPAUCRuntimeClass@@XZ  : proc
externdef ?PreTranslateMessage@CFrameWnd@@UAEHPAUtagMSG@@@Z  : proc
externdef ?PreCreateWindow@CFrameWnd@@UAEHAAUtagCREATESTRUCTA@@@Z  : proc
externdef ?OnCreateClient@CFrameWnd@@UAEHPAUtagCREATESTRUCTA@@PAX@Z  : proc
externdef ?OnCommand@CFrameWnd@@UAEHIJ@Z  : proc
externdef ?OnCmdMsg@CFrameWnd@@UAEHIHPAXPAUAFX_CMDHANDLERINFO@@@Z  : proc
externdef ?NegotiateBorderSpace@CFrameWnd@@UAEHIPAUtagRECT@@@Z  : proc
externdef ?LoadFrame@CFrameWnd@@UAEHIKPAVCWnd@@PAX@Z  : proc
externdef ?IsFrameWnd@CFrameWnd@@UBEHXZ  : proc
externdef ?GetMessageBar@CFrameWnd@@UAEPAVCWnd@@XZ  : proc
externdef ?GetActiveFrame@CFrameWnd@@UAEPAV1@XZ  : proc
externdef ?GetActiveDocument@CFrameWnd@@UAEPAVCDocument@@XZ  : proc
externdef ?Create@CFrameWnd@@QAEHPBD0KABUtagRECT@@PAVCWnd@@0KPAX@Z : proc

externdef ?Create@CFrameWnd@@QAEHPBD0KABUtagRECT@@PAVCWnd@@0KPAX@Z : proc



;CButton
externdef ?GetRuntimeClass@CButton@@UBEPAUCRuntimeClass@@XZ :proc   ; (public: virtual struct CRuntimeClass * __thiscall CButton::GetRuntimeClass(void)const )
externdef ??0CButton@@QAE@XZ :proc   ; (public: __thiscall CButton::CButton(void))
externdef ??1CButton@@UAE@XZ :proc   ; (public: virtual __thiscall CButton::~CButton(void))
externdef ??_GCButton@@UAEPAXI@Z :proc   ; (public: virtual void * __thiscall CButton::`scalar deleting destructor'(unsigned int))


;CDialog
externdef ??0CDialog@@QAE@XZ :proc   ; (public: __thiscall CDialog::CDialog(void))
externdef ??0CDialog@@QAE@PBDPAVCWnd@@@Z :proc   ; (public: __thiscall CDialog::CDialog(char const *,class CWnd *))
externdef ??0CDialog@@QAE@IPAVCWnd@@@Z :proc   ; (public: __thiscall CDialog::CDialog(unsigned int,class CWnd *))
externdef ??1CDialog@@UAE@XZ :proc   ; (public: virtual __thiscall CDialog::~CDialog(void))
externdef ?GetRuntimeClass@CDialog@@UBEPAUCRuntimeClass@@XZ :proc   ; (public: virtual struct CRuntimeClass * __thiscall CDialog::GetRuntimeClass(void)const )
externdef ?OnCmdMsg@CDialog@@UAEHIHPAXPAUAFX_CMDHANDLERINFO@@@Z :proc   ; (public: virtual int __thiscall CDialog::OnCmdMsg(unsigned int,int,void *,struct AFX_CMDHANDLERINFO *))
externdef ?PreTranslateMessage@CDialog@@UAEHPAUtagMSG@@@Z :proc   ; (public: virtual int __thiscall CDialog::PreTranslateMessage(struct tagMSG *))
externdef ?CheckAutoCenter@CDialog@@UAEHXZ :proc   ; (public: virtual int __thiscall CDialog::CheckAutoCenter(void))
externdef ?SetOccDialogInfo@CDialog@@UAEHPAU_AFX_OCC_DIALOG_INFO@@@Z :proc   ; (public: virtual int __thiscall CDialog::SetOccDialogInfo(struct _AFX_OCC_DIALOG_INFO *))
externdef ?DoModal@CDialog@@UAEHXZ :proc   ; (public: virtual int __thiscall CDialog::DoModal(void))
externdef ?OnInitDialog@CDialog@@UAEHXZ :proc   ; (public: virtual int __thiscall CDialog::OnInitDialog(void))
externdef ?OnSetFont@CDialog@@UAEXPAVCFont@@@Z :proc   ; (public: virtual void __thiscall CDialog::OnSetFont(class CFont *))
externdef ?OnOK@CDialog@@UAEXXZ :proc   ; (public: virtual void __thiscall CDialog::OnOK(void))
externdef ?OnCancel@CDialog@@UAEXXZ :proc   ; (public: virtual void __thiscall CDialog::OnCancel(void))
externdef ?PreInitDialog@CDialog@@UAEXXZ :proc   ; (public: virtual void __thiscall CDialog::PreInitDialog(void))

externdef ?messageMap@CDialog@@2UAFX_MSGMAP@@B :dword ; CDialog message map - 610878


;CStatic
externdef ??0CStatic@@QAE@XZ :proc   ; (public: __thiscall CStatic::CStatic(void))
externdef ??1CStatic@@UAE@XZ :proc   ; (public: virtual __thiscall CStatic::~CStatic(void))


;CStatusBar
externdef ??0CStatusBar@@QAE@XZ :proc   ; (public: __thiscall CStatusBar::CStatusBar(void))
externdef ??1CStatusBar@@UAE@XZ :proc   ; (public: virtual __thiscall CStatusBar::~CStatusBar(void))




; VisListBoxDiplomacy
externdef ??0CWinThread@@QAE@P6AIPAX@Z0@Z : proc
extern _byte_659A5C : byte  ; 659a5c zeroed buffer used to clear the net chat text box
externdef ?CreateThread@CWinThread@@QAEHKIPAU_SECURITY_ATTRIBUTES@@@Z : proc
extern ?XorRegString@@YA?AVCString@@ABV1@@Z : proc  ; CString XorRegString(const CString&) - 43d1a9
extern ?Null@CellState@@QAEXXZ : proc  ; void CellState::Null() - 58a3da
