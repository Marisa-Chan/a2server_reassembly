

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


; WorldEquip
externdef ??0WorldEquip@@QAE@XZ :proc


; MagicItem
externdef ??0MagicItem@@QAE@XZ :proc


; MatShape
externdef ??0MatShape@@QAE@XZ :proc


; MaginInfo
externdef ??0MagicInfo@@QAE@XZ :proc


; MonsterInfo
externdef ??0MonsterInfo@@QAE@XZ :proc


; HumanInfo
externdef ??0HumanInfo@@QAE@XZ :proc


; BuildingInfo
externdef ??0BuildingInfo@@QAE@XZ :proc


; SpellInfo
externdef ??0SpellInfo@@QAE@XZ :proc


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

extern  ?AppGetWorkingDir@@YAHHPAD@Z  : proc
extern  ?AppHasAnySaveFile@@YAHXZ  : proc
extern ?AppFindSavesList@@YAXPAV?$CArray@U_WIN32_FIND_DATAA@@ABU1@@@H@Z  : proc

extern ?g_WorkingDir@@3PADA : byte





externdef ?LogMessage@@YAXVCString@@@Z:proc     ;  LogMessage
externdef _sub_43A857:proc                       ;  void __cdecl sub_43A857(const char* source)


;GameDataRes
externdef ?g_GameDataRes@@3UGameDataRes@@A:byte    ; g_GameDataRes

externdef ?ParseWorldIn@GameDataRes@@QAEHABVCString@@@Z : proc
externdef ?sub_510502@GameDataRes@@QAEPAVItem@@PAVCString@@@Z :proc
externdef ?sub_5126A2@GameDataRes@@QAEHII@Z :proc
externdef ?sub_50DDAE@GameDataRes@@QAEHII@Z :proc                              ; int GameDataRes::sub_50DDAE(uint32_t, uint32_t)
extern ?FUN_00512625@GameDataRes@@QAEHH@Z : proc
extern ?FUN_005125a8@GameDataRes@@QAEHH@Z : proc
externdef ?sub_50D8BA@GameDataRes@@QAEEPAVCString@@0@Z :proc              ; uint8_t GameDataRes::sub_50D8BA(CString*, CString*)
externdef ?sub_50DA04@GameDataRes@@QAEEPAVCString@@0@Z :proc              ; uint8_t GameDataRes::sub_50DA04(CString*, CString*)
externdef ?sub_50DB4E@GameDataRes@@QAEPAVCString@@PAV2@PBV2@0@Z :proc     ; CString* GameDataRes::sub_50DB4E(CString*, CString*, CString*)
externdef ?sub_50DC69@GameDataRes@@QAEXHPAVCString@@@Z :proc              ; void GameDataRes::sub_50DC69(int, CString*)

; BuildingsList
externdef ?sub_557DB2@BuildingsList@@QAEPAVBuilding@@G@Z :proc
externdef ?sub_558128@BuildingsList@@QAEPAVBuilding@@PAUTokenPos@@@Z :proc   ; BuildingsList::sub_558128(TokenPos*)
externdef ?sub_557EA5@BuildingsList@@QAEPAVBuilding@@PAUTokenPos@@@Z :proc   ; BuildingsList::sub_557EA5(TokenPos*)
externdef ?sub_557F69@BuildingsList@@QAEPAVBuilding@@PAUTokenPos@@@Z :proc   ; BuildingsList::sub_557F69(TokenPos*)
externdef ?sub_558055@BuildingsList@@QAEPAVBuilding@@PAVPlayer@@@Z :proc     ; BuildingsList::sub_558055(Player*)
externdef ?sub_558228@BuildingsList@@QAEXPAVBuilding@@@Z :proc                ; void BuildingsList::sub_558228(Building*)
externdef ?sub_558908@BuildingsList@@QAEXAAVCArchive@@@Z :proc

; Building
extern ?classBuilding@Building@@2UCRuntimeClass@@A  : dword
extern ??5@YGAAVCArchive@@AAV0@AAPAVBuilding@@@Z : proc
externdef ??0Building@@QAE@EPAUTokenPos@@EE@Z :proc                             ; Building::Building(uint8_t, TokenPos*, uint8_t, uint8_t)
externdef ??0Building@@QAE@PAUTokenPos@@@Z :proc                                ; Building::Building(TokenPos*)
externdef ??0Building@@QAE@ABVCString@@@Z :proc                                 ; Building::Building(const CString&)
externdef ??0Building@@QAE@XZ :proc                                             ; Building::Building()
externdef ??1Building@@UAE@XZ :proc                                             ; Building::~Building()
externdef ?CreateObject@Building@@SGPAVCObject@@XZ :proc  ; Building::CreateObject()
externdef ?VMethod1@Building@@UAEXXZ :proc
externdef ?Serialize@Building@@UAEXAAVCArchive@@@Z :proc                        ; void Building::Serialize(CArchive&)
externdef ?sub_5427AC@Building@@QAEXEE@Z :proc                                  ; void Building::sub_5427AC(uint8_t, uint8_t)
externdef ?sub_542624@Building@@QAEXABVCString@@@Z :proc                        ; void Building::sub_542624(const CString&)


; Pointer
externdef ??0Pointer@@QAE@EPAUTokenPos@@EE@Z :proc                              ; Pointer::Pointer(uint8_t, TokenPos*, uint8_t, uint8_t)
externdef ??0Pointer@@QAE@XZ :proc                                              ; Pointer::Pointer()
externdef ??1Pointer@@UAE@XZ :proc                                              ; Pointer::~Pointer()
externdef ?Serialize@Pointer@@UAEXAAVCArchive@@@Z :proc                         ; void Pointer::Serialize(CArchive&)
externdef ?CreateObject@Pointer@@SGPAVCObject@@XZ :proc                         ; Pointer::CreateObject()


; Outpost
externdef ??_7Outpost@@6B@ :dword
externdef ?classOutpost@Outpost@@2UCRuntimeClass@@A :dword
externdef ??0Outpost@@QAE@XZ :proc                                               ; Outpost::Outpost()
externdef ??0Outpost@@QAE@PAUTokenPos@@@Z :proc                                  ; Outpost::Outpost(TokenPos*)
externdef ??1Outpost@@UAE@XZ :proc                                               ; Outpost::~Outpost()
externdef ?VMethod1@Outpost@@UAEXXZ : proc
externdef ?sub_543EC3@Outpost@@QAEXPAVGroup@@@Z :proc


; Inn
externdef ??_7Inn@@6B@ :dword
externdef ?classInn@Inn@@2UCRuntimeClass@@A :dword
externdef ??0Inn@@QAE@XZ :proc                                                   ; Inn::Inn()
externdef ??0Inn@@QAE@EPAUTokenPos@@@Z :proc                                     ; Inn::Inn(uint8_t, TokenPos*)
externdef ??1Inn@@UAE@XZ :proc                                                   ; Inn::~Inn()

externdef ?VMethod1@Inn@@UAEXXZ :proc
externdef ?sub_560C67@Inn@@QAEXPAVUnit@@@Z :proc
externdef ?InnCreateQuests@Inn@@QAEXPAVPlayer@@@Z :proc
externdef ?InnReward@Inn@@QAEXPAVPlayer@@@Z :proc                               ; void Inn::InnReward(Player*)
externdef ?sub_567B4D@Inn@@QAE_NPAVPlayer@@@Z :proc
externdef ?sub_567A25@Inn@@QAEPAVQuest@@PAVPlayer@@@Z :proc                     ; Quest* Inn::sub_567A25(Player*)


; Shop
externdef ??0Shop@@QAE@XZ :proc                                                  ; Shop::Shop()
externdef ??0Shop@@QAE@ABVCString@@@Z :proc                                      ; Shop::Shop(const CString&)
externdef ??0Shop@@QAE@EPAUTokenPos@@@Z :proc                                    ; Shop::Shop(uint8_t, TokenPos*)
externdef ??1Shop@@UAE@XZ :proc                                                  ; Shop::~Shop()
externdef ??_GShop@@UAEPAXI@Z :proc                                              ; Shop scalar deleting destructor
externdef ?Serialize@Shop@@UAEXAAVCArchive@@@Z :proc                             ; void Shop::Serialize(CArchive&)
externdef ?VMethod1@Shop@@UAEXXZ :proc                                           ; void Shop::VMethod1()

externdef ?sub_544655@Shop@@QAEXPAVUnit@@@Z :proc
externdef ?sub_544793@Shop@@QAEXPAVHumanoid@@HPAVItem@@@Z :proc
externdef ?sub_54463F@Shop@@QAEXXZ :proc
externdef ?sub_5446EB@Shop@@QAEXPAVHumanoid@@EFEFH@Z :proc
externdef ?sub_547C5A@CMultiShopTemplate@@QAEPAVItem@@PAVHumanoid@@FH@Z :proc

externdef ?sub_54471B@Shop@@QAEXPAVUnit@@@Z :proc
externdef ?sub_544737@Shop@@QAEXPAVUnit@@@Z :proc
externdef ?sub_544777@Shop@@QAEXPAVUnit@@@Z :proc
externdef ?sub_544685@Shop@@QAEXPAVUnit@@@Z :proc


; MultiShopShelf
externdef ?ClearItems@CMultiShopShelf@@QAEXXZ : proc

; MultiShopTemplate
externdef ?sub_546F18@CMultiShopTemplate@@QAEXPAVCMultiShopInstance@@H@Z : proc
externdef ?sub_547DD4@CMultiShopTemplate@@QAEXXZ :proc                           ; void MultiShopTemplate::sub_547DD4()

; MultiShopInstance
externdef ??0CMultiShopInstance@@QAE@XZ :proc                                     ; MultiShopInstance::MultiShopInstance()
externdef ?Sell@CMultiShopInstance@@QAEXXZ : proc
externdef ?Buy@CMultiShopInstance@@QAEXXZ : proc
externdef ?sub_5462C8@CMultiShopInstance@@QAEHPAVItem@@H@Z : proc
externdef ?sub_5464B6@CMultiShopInstance@@QAEXH@Z : proc


; ShopAssortment
externdef ??0ShopAssortment@@QAE@XZ :proc
externdef ??1ShopAssortment@@QAE@XZ :proc
externdef ?GenerateAssortment@ShopAssortment@@QAEXPAUAssortGenParams@@@Z :proc
externdef ?ArrangeShelfs@ShopAssortment@@QAEPAUInventory@@HHHHPAV?$CArray@PAVItem@@ABQAV1@@@@Z :proc
externdef ?sub_54EA76@ShopAssortment@@QAEHPAVItem@@@Z :proc  ; int32_t ShopAssortment::sub_54EA76(Item*)
externdef _sub_54C6DD :proc  ; void __cdecl sub_54C6DD(CArray<Item*>*, Item*)
externdef _sub_54C556 :proc  ; void __cdecl sub_54C556(CArray<Item*>*)
externdef ?sub_548FAA@Item@@QAEXPAVEffect@@@Z :proc  ; void Item::sub_548FAA(Effect*)
externdef _sub_54F04F :proc  ; Effect* __cdecl sub_54F04F(int, int, int, int)
externdef _sub_54F176 :proc  ; int32_t __cdecl sub_54F176(Item*)
externdef ?sub_541FD7@Effect@@QAEHHH@Z :proc  ; int32_t Effect::sub_541FD7(int, int)


;Scenario
extern ?g_scenario_dll@@3PAUHINSTANCE__@@A  : dword
externdef _ScenarioGetVar  :dword
externdef _ScenarioSetVar  :dword
externdef _ScenarioTalkTo  :dword
externdef _ScenarioEnterLocation  :dword
externdef _ScenarioLeaveLocation  :dword
externdef _ScenarioEnterShop  :dword
externdef _ScenarioLeaveShop  :dword
externdef _ScenarioEnterInn  :dword
externdef _ScenarioLeaveInn  :dword
externdef _ScenarioNewGame  :dword
externdef _ScenarioSave  :dword
externdef _ScenarioLoad  :dword
externdef _ScenarioGetAvailableLocations  :dword
externdef _ScenarioGetShopAssortment  :dword
externdef _ScenarioIsTownAvailable  :dword
externdef _ScenarioIsMissionAvailable  :dword
externdef _ScenarioGetCurrentLocation  :dword
externdef _ScenarioGetAllLocations  :dword




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
extern ?CompatGetYX@TokenPos@@QBEGXZ:proc   ; TokenPos::GetYX
extern ?GetXx@TokenPos@@QBEGXZ:proc   ; TokenPos::GetXx
extern ?GetYy@TokenPos@@QBEGXZ:proc   ; TokenPos::GetYy
extern ?SetCoords@TokenPos@@QAEXEE@Z:proc   ; TokenPos::SetCoords
extern ?SetCoords2@TokenPos@@QAEXGG@Z:proc   ; TokenPos::SetCoords2
extern ?Distance@TokenPos@@QBEEPBU1@@Z:proc   ; TokenPos::Distance
extern ?IsSameYX@TokenPos@@QBEHPBU1@@Z:proc   ; TokenPos::IsSameYX
extern ?sub_58bec3@TokenPos@@QAEHXZ :proc  ; TokenPos::sub_58bec3() – position mode check
extern ?FUN_0058a7e8@TokenPos@@QAEHEE@Z : proc   ;  TokenPos::FUN_0058a7e8



; MapStuff
externdef ??0MapStuff@@QAE@PAUMapAlm@@PAVUnitList@@@Z :proc  ; MapStuff::MapStuff(MapAlm*, UnitList*)
externdef ??1MapStuff@@QAE@XZ :proc

externdef ?sub_5913BD@MapStuff@@QAEFPAVUnit@@EE@Z :proc
externdef ?sub_5948B0@MapStuff@@QAEXPAVCWordArray@@@Z :proc
externdef ?sub_59166C@MapStuff@@QAEEPAVUnit@@UPosYX@@@Z  :proc
externdef ?sub_59190D@MapStuff@@QAEHPAVUnit@@0@Z :proc          ; int MapStuff::sub_59190D(Unit* target, Unit* observer)
externdef ?sub_591424@MapStuff@@QAEEPAVUnit@@0@Z :proc          ; uint8_t MapStuff::sub_591424(Unit*, Unit*)
externdef ?sub_58FE6D@MapStuff@@QAEHPAVUnit@@0E@Z :proc         ; int MapStuff::sub_58FE6D(Unit*, Unit*, uint8_t)
externdef ?sub_58FEDA@MapStuff@@QAEHPAVUnit@@UPosYX@@E@Z :proc         ; int MapStuff::sub_58FEDA(Unit*, uint16_t, uint8_t)
externdef ?sub_58FF51@MapStuff@@QAEXPAVUnit@@UPosYX@@E@Z :proc         ; void MapStuff::sub_58FF51(Unit*, uint16_t, uint8_t)
externdef ?sub_590678@MapStuff@@QAEXPAVUnit@@@Z :proc           ; void MapStuff::sub_590678(Unit*)
externdef ?sub_5907BE@MapStuff@@QAEXPAVUnit@@@Z :proc           ; void MapStuff::sub_5907BE(Unit*)
externdef ?sub_590902@MapStuff@@QAEXPAVUnit@@0@Z :proc          ; void MapStuff::sub_590902(Unit*, Unit*)
externdef ?sub_597140@MapStuff@@QAEHPAVUnit@@UPosYX@@H@Z :proc         ; int MapStuff::sub_597140(Unit*, uint16_t, int32_t)
externdef ?sub_59028D@MapStuff@@QAEXPAVUnit@@0E@Z :proc         ; void MapStuff::sub_59028D(Unit*, Unit*, uint8_t)
externdef ?sub_58AD4A@MapStuff@@QAEHPAVUnit@@@Z :proc

externdef ?sub_5882AE@MapStuff@@QAEXPAVUnit@@EEEEH0@Z :proc         ; void MapStuff::sub_5882AE(Unit*, uint8_t, uint8_t, uint8_t, uint8_t, int32_t, Unit*)
externdef ?sub_58B1D7@MapStuff@@QAEHPAVUnit@@@Z :proc                ; int MapStuff::sub_58B1D7(Unit*)
externdef ?sub_58B593@MapStuff@@QAEXUPosYX@@@Z :proc                 ; void MapStuff::sub_58B593(PosYX)
externdef ?sub_58E611@MapStuff@@QAEPAVSack@@G@Z :proc                ; Sack* MapStuff::sub_58E611(uint16_t)
externdef ?sub_590F94@MapStuff@@QAEXPAVUnit@@E@Z :proc               ; void MapStuff::sub_590F94(Unit*, uint8_t)
externdef ?sub_59449A@MapStuff@@QAEHPAVBuilding@@UPosYX@@@Z :proc    ; int MapStuff::sub_59449A(Building*, PosYX)
externdef ?sub_595468@MapStuff@@QAEHPAVUnit@@UPosYX@@@Z :proc       ; int MapStuff::sub_595468(Unit*, PosYX)

; Migrated to C++ (map_stuff.cpp), still called directly from remaining ASM code.
externdef ?sub_58CA1B@MapStuff@@QAEPAVUnit@@UPosYX@@@Z :proc
externdef ?sub_58CB5A@MapStuff@@QAEPAVUnit@@G@Z :proc
externdef ?sub_58CBB9@MapStuff@@QAEPAVUnit@@G@Z :proc
externdef ?sub_58826D@MapStuff@@QAEXPAVUnit@@EEH0@Z :proc
externdef ?sub_58E3D1@MapStuff@@QAEHPAVUnit@@@Z :proc
externdef ?sub_58E525@MapStuff@@QAEXPAVSack@@@Z :proc
externdef ?sub_58E5C7@MapStuff@@QAEPAVSack@@GG@Z :proc
externdef ?sub_58E5F3@MapStuff@@QAEPAVSack@@PAUTokenPos@@@Z :proc
externdef ?sub_590F0A@MapStuff@@QAEHPAVUnit@@@Z :proc
externdef ?sub_5918B8@MapStuff@@QAEXPAVUnit@@0@Z :proc
externdef ?sub_5918E2@MapStuff@@QAEXPAVUnit@@UPosYX@@@Z :proc
externdef ?sub_593AA4@MapStuff@@QAEGPAVUnit@@@Z :proc
externdef ?sub_593B29@MapStuff@@QAEEUPosYX@@0@Z :proc
externdef ?sub_594125@MapStuff@@QAEXXZ :proc
externdef ?sub_5945EF@MapStuff@@QAEHPAVBuilding@@@Z :proc
externdef ?sub_59536C@MapStuff@@QAEPAPAVAreaEffect@@I@Z :proc
externdef ?sub_5953CB@MapStuff@@QAEPAVAreaEffect@@PAV2@G@Z :proc
externdef ?sub_595438@MapStuff@@QAEHPAVAreaEffect@@EE@Z :proc
externdef ?sub_5954AC@MapStuff@@QAEXPAVUnit@@EE@Z :proc

extern ?FUN_0058b5d6@MapStuff@@QAEXAAUCellState@@@Z : proc
extern ?FUN_0058b4a6@MapStuff@@QAEHUPosYX@@@Z : proc
extern ?FUN_0058b3e0@MapStuff@@QAEXUPosYX@@@Z : proc
extern ?FUN_005969c6@MapStuff@@QAEXPAVUnit@@UPosYX@@E@Z : proc
externdef ?sub_58AB48@MapStuff@@QAEXAAVCArchive@@@Z :proc

extern ?IsEmpty@CellState@@QBEHXZ : proc
externdef ?sub_59423F@CellState@@QAEXXZ :proc          ; void CellState::sub_59423F()
externdef ?sub_538897@AreaEffect@@QBEIXZ :proc         ; uint32_t AreaEffect::sub_538897() const


extern ?MapStuff_Instance@@3PAUMapStuff@@A:dword   ;  MapStuff_Instance

extern ?g_MissionText@@3VCString@@A : dword
extern ?g_MissionBriefing@@3VCString@@A : dword
extern ?g_MissionFailures@@3VCStringArray@@A : dword
extern ?g_MissionSubjs@@3VCStringArray@@A : dword

extern ?MissionGetTips@@YAXHPAVCString@@@Z  :proc
extern ?MissionGetLocName@@YAXHHPAVCString@@@Z  :proc
extern ?MissionGetDescription@@YAXHHPAVCString@@@Z  :proc


; ScanPresenceGrid
externdef ?sub_596047@ScanPresenceGrid@@QAEXPAVAreaEffect@@@Z :proc
externdef ?sub_596131@ScanPresenceGrid@@QAEXXZ :proc


;Token
;extern ??_7Token@@6B@:dword ; Token::`vftable`
extern ?classToken@Token@@2UCRuntimeClass@@A:dword ; Token::classToken

extern ??0Token@@QAE@XZ:proc ; Token::Token
extern ??0Token@@QAE@ABV0@@Z:proc ; Token::Token(const Token&)
extern ??0Token@@QAE@PBUTokenPos@@@Z:proc ; Token::Token(const TokenPos*)
extern ??0Token@@QAE@PBUTokenPos@@PAVPlayer@@@Z:proc ; Token::Token(const TokenPos*, Player*)

extern ??1Token@@UAE@XZ:proc ; Token::~Token 005283d6

extern ?Serialize@Token@@UAEXAAVCArchive@@@Z :proc ; Token::Serialize 0055AB39

extern ?VMethod1@Token@@UAEXXZ:proc ;  Token::VMethod1
extern ?VMethod2@Token@@UAEXXZ:proc ;  Token::VMethod2
extern ?VMethod3@Token@@UAEHXZ:proc ;  Token::VMethod3
extern ?VMethod4@Token@@UAEHXZ:proc ;  Token::VMethod4
extern ?VMethod5@Token@@UAEXXZ:proc ;  Token::VMethod5
extern ?VMethod6@Token@@UAEHXZ:proc ;  Token::VMethod6
extern ?VMethod7@Token@@UAEHXZ:proc ;  Token::VMethod7
extern ?VMethod8@Token@@UAEHXZ:proc ;  Token::VMethod8
extern ?VMethod9@Token@@UAEHXZ:proc ;  Token::VMethod9


; VirtualCaster
externdef ??_7VirtualCaster@@6B@ :dword                                         ; VirtualCaster vftable (C++ generated)
extern ??5@YGAAVCArchive@@AAV0@AAPAVVirtualCaster@@@Z : proc                    ; operator>>(CArchive&, VirtualCaster*&)
externdef ??0VirtualCaster@@QAE@XZ :proc                                        ; VirtualCaster::VirtualCaster()
externdef ??1VirtualCaster@@UAE@XZ :proc                                        ; VirtualCaster::~VirtualCaster()
externdef ?Serialize@VirtualCaster@@UAEXAAVCArchive@@@Z :proc                   ; void VirtualCaster::Serialize(CArchive&)
externdef ?CreateObject@VirtualCaster@@SGPAVCObject@@XZ :proc                   ; VirtualCaster::CreateObject()
externdef ?classVirtualCaster@VirtualCaster@@2UCRuntimeClass@@A :dword



;BitSet g_buildingIdSet     in token.cpp
extern _BldIdSet_Set:proc
extern _BldIdSet_Unset:proc
extern _BldIdSet_Clear:proc
extern _BldIdSet_AllocBit:proc



; Inventory
externdef ??0Inventory@@QAE@XZ :proc             ; Inventory::Inventory()
externdef ??0Inventory@@QAE@PBU0@@Z :proc        ; Inventory::Inventory(const Inventory*)
externdef ??1Inventory@@QAE@XZ :proc             ; Inventory::~Inventory()
externdef ?PutItemIntoBagAtDefault@Inventory@@QAEXPAVItem@@@Z :proc ; void Inventory::PutItemIntoBagAtDefault(Item*)
externdef ?PutItemIntoBag@Inventory@@QAEXHPAVItem@@@Z :proc        ; void Inventory::PutItemIntoBag(int32_t, Item*)
externdef ?sub_552B6B@Inventory@@QAEPAVItem@@G@Z :proc             ; Item* Inventory::sub_552B6B(uint16_t)
externdef ?sub_552E42@Inventory@@QAEPAVItem@@HH@Z :proc            ; Item* Inventory::sub_552E42(int32_t, int32_t)

;Unit
externdef ?Serialize@Unit@@UAEXAAVCArchive@@@Z:proc   ; (public: virtual void __thiscall Unit::Serialize(class CArchive &))
externdef ?VMethod1@Unit@@UAEXXZ:proc   ; (public: virtual void __thiscall Unit::VMethod1(void))
externdef ?VMethod2@Unit@@UAEXXZ:proc   ; (public: virtual void __thiscall Unit::VMethod2(void))
externdef ?VMethod5@Unit@@UAEXXZ:proc   ; (public: virtual void __thiscall Unit::VMethod5(void))

externdef ?VMethod10@Unit@@UAEXXZ:proc   ; (public: virtual void __thiscall Unit::VMethod10(void))
externdef ?VMethod11@Unit@@UAEXPAV1@@Z:proc   ; (public: virtual void __thiscall Unit::VMethod11(class Unit *))

externdef ?VMethod17@Unit@@UAEIPAUUnitToHit@@PAV1@@Z:proc   ; (public: virtual unsigned int __thiscall Unit::VMethod17(struct UnitToHit *,class Unit *))

externdef ?VMethod25@Unit@@UAEHXZ:proc   ; (public: virtual int __thiscall Unit::VMethod25(void))



extern ??0Unit@@QAE@XZ:proc    ; (public: __thiscall Unit::Unit(void))
extern ??0Unit@@QAE@ABVCString@@@Z:proc   ; (public: __thiscall Unit::Unit(class CString const &))
extern ??5@YGAAVCArchive@@AAV0@AAPAVUnit@@@Z:proc     ;  CArchive & __stdcall operator>>(class CArchive &,class Unit * &)
externdef ?sub_52C813@Unit@@QAEXXZ :proc
externdef ?sub_52BF3D@Unit@@QAEHEEE@Z :proc
externdef ?sub_52C163@Unit@@QAEXXZ :proc
externdef ?sub_542A31@Building@@QAEHPAUUnitToHit@@PAVUnit@@@Z :proc       ; int32_t Building::sub_542A31(UnitToHit*, Unit*)
externdef ?FUN_0052ec7a@Unit@@QAEXABV?$CArray@UMonsterInfoData@@ABU1@@@@Z :proc

externdef ?sub_52C409@Unit@@QAEHXZ :proc
externdef ??0UnitEye@@QAE@XZ :proc                                        ; UnitEye::UnitEye()
externdef ??0UnitEye2@@QAE@XZ :proc                                       ; UnitEye2::UnitEye2()
externdef ??1UnitEye2@@QAE@XZ :proc                                       ; UnitEye2::~UnitEye2()

externdef ?sub_52BABD@Unit@@QAEHXZ :proc                                  ; Unit::sub_52BABD()
externdef ?sub_59367D@UnitEye@@QAEHPAVUnit@@PAEEE@Z :proc                 ; int UnitEye::sub_59367D(Unit*, uint8_t*, uint8_t, uint8_t)
externdef ?sub_51B99E@NetStru1@@QAEXPAVUnit@@EEE@Z :proc                  ; void NetStru1::sub_51B99E(Unit*, uint8_t, uint8_t, uint8_t)

externdef ?sub_528763@Unit@@QAEGXZ :proc
externdef ?sub_528725@Unit@@QAEGXZ :proc
externdef ?sub_52A790@Unit@@QAEXH@Z :proc

; UnitEye

externdef ?sub_58c00e@UnitEye@@QAEEXZ :proc     ; UnitEye::sub_58c00e() – get position byte
externdef ?sub_5925C9@UnitEye@@QAEXPAVUnit@@@Z :proc  ; void UnitEye::sub_5925C9(Unit*)

; Group
extern ??0Group@@QAE@XZ :proc
extern ??1Group@@QAE@XZ :proc
extern ?RemoveUnit@Group@@QAEXPAVUnit@@@Z :proc
extern ?AddUnit@Group@@QAEXPAVUnit@@@Z :proc
extern ?Serialize@Group@@QAEXAAVCArchive@@@Z  :proc

; GroupList
extern ??0GroupList@@QAE@XZ   :proc
extern ??1GroupList@@QAE@XZ   :proc
extern ?AllocGroup@GroupList@@QAEPAVGroup@@XZ   :proc
externdef ?Serialize@GroupList@@QAEXAAVCArchive@@@Z :proc




;Humanoid

externdef ?InitStats@Humanoid@@QAEXXZ:proc   ;    Humanoid::InitStats
externdef ?sub_532dde@Humanoid@@QAEXPAV?$CArray@UHumanInfoData@@ABU1@@@@Z :proc


;Human

extern ??0Human@@QAE@XZ:proc   ; Human::Human
extern ??0Human@@QAE@ABVCString@@HPBD@Z:proc    ; Human::Human(CString&, int, const char *)
externdef ?FUN_00532587@Human@@QAEXVCString@@HPBD@Z:proc   ;  Human::FUN_00532587(class CString,int,char const *)


;UnitList
externdef _dword_6CDB3C :dword

extern ??0UnitList@@QAE@XZ:proc                         ; UnitList::UnitList
extern ??1UnitList@@QAE@XZ:proc                         ; UnitList::~UnitList
extern ?AddTail@UnitList@@QAEXPAVUnit@@@Z:proc          ; UnitList::AddTail
extern ?AddTailAllocId@UnitList@@QAEXPAVUnit@@@Z:proc   ; UnitList::AddTailAllocId
extern ?AddTailId6xxx@UnitList@@QAEXPAVUnit@@@Z:proc    ; UnitList::AddTailId6xxx
externdef ?sub_5579D8@UnitList@@QAEXPAVPlayer@@@Z :proc
externdef ?sub_5560D2@UnitList@@QAEPAVUnit@@G@Z :proc
externdef ?ProcessTick@UnitList@@QAEXXZ :proc
externdef ?sub_557AB0@UnitList@@QAEHXZ :proc                                   ; UnitList::sub_557AB0()



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


; Effect


extern ??5@YGAAVCArchive@@AAV0@AAPAVEffect@@@Z : proc   ; CArchive >> Effect

extern ?classEffect@Effect@@2UCRuntimeClass@@A  : byte

extern ?GetRuntimeClass@Effect@@UBEPAUCRuntimeClass@@XZ  : proc
extern ??1Effect@@UAE@XZ  : proc
extern ?Serialize@Effect@@UAEXAAVCArchive@@@Z  : proc

externdef ?VMethod10@Effect@@UAEXPAVUnit@@@Z  : proc
externdef ?VMethod11@Effect@@UAEXPAVUnit@@@Z  : proc
externdef ?VMethod12@Effect@@UAEXPAVUnit@@@Z  : proc
externdef ?VMethod13@Effect@@UAEXPAVUnit@@@Z  : proc
externdef ?VMethod14@Effect@@UAEXPAVUnit@@H@Z  : proc
externdef ?VMethod15@Effect@@UAEHXZ  : proc
externdef ?VMethod16@Effect@@UAEHN@Z  : proc
externdef ?VMethod17@Effect@@UAEXH@Z  : proc

extern ??0Effect@@QAE@XZ  : proc
extern ??0Effect@@QAE@ABVCString@@@Z  : proc
extern ??0Effect@@QAE@PBV0@@Z  : proc

externdef ?EffectPrice@Effect@@QAEHXZ :proc                                   ; int32_t Effect::EffectPrice()
externdef ?MagicPriceBonus@Effect@@SA_JH@Z :proc                              ; int64_t Effect::MagicPriceBonus(int32_t)
externdef ?CreateFromString@Effect@@SAPAV1@ABVCString@@@Z  : proc


; DirectDamage

externdef ??0DirectDamage@@QAE@XZ :proc
externdef ??0DirectDamage@@QAE@PAV0@@Z :proc             ; DirectDamage::DirectDamage(DirectDamage*)
externdef ?VMethod11@DirectDamage@@UAEXPAVUnit@@@Z :proc ; void DirectDamage::VMethod11(Unit*)
externdef ?classDirectDamage@DirectDamage@@2UCRuntimeClass@@A :dword


; SpellEffect
externdef ??0SpellEffect@@QAE@XZ :proc                              ; SpellEffect::SpellEffect()
externdef ??0SpellEffect@@QAE@PBUTokenPos@@@Z :proc                 ; SpellEffect::SpellEffect(const TokenPos*)
externdef ??1SpellEffect@@UAE@XZ :proc                              ; SpellEffect::~SpellEffect()
externdef ?Serialize@SpellEffect@@UAEXAAVCArchive@@@Z :proc         ; void SpellEffect::Serialize(CArchive&)
externdef ?VMethod2@SpellEffect@@UAEXXZ :proc                       ; void SpellEffect::VMethod2()
externdef ?VMethod10@SpellEffect@@UAEXPAVUnit@@@Z :proc             ; void SpellEffect::VMethod10(Unit*)
externdef ?classSpellEffect@SpellEffect@@2UCRuntimeClass@@A :byte   ; static CRuntimeClass SpellEffect::classSpellEffect

; AreaEffect
externdef ??0AreaEffect@@QAE@XZ :proc                                          ; AreaEffect::AreaEffect()
externdef ??0AreaEffect@@QAE@PAVEffect@@PAUTokenPos@@E@Z :proc                 ; AreaEffect::AreaEffect(Effect*, TokenPos*, uint8_t)
externdef ??1AreaEffect@@UAE@XZ :proc                                          ; AreaEffect::~AreaEffect()
externdef ?Serialize@AreaEffect@@UAEXAAVCArchive@@@Z :proc                     ; void AreaEffect::Serialize(CArchive&)
externdef ?VMethod2@AreaEffect@@UAEXXZ :proc                                   ; void AreaEffect::VMethod2()
externdef ?VMethod5@AreaEffect@@UAEXXZ :proc                                   ; void AreaEffect::VMethod5()
externdef ?VMethod10@AreaEffect@@UAEXPAVUnit@@@Z :proc                         ; void AreaEffect::VMethod10(Unit*)
externdef ?classAreaEffect@AreaEffect@@2UCRuntimeClass@@A :byte                ; static CRuntimeClass AreaEffect::classAreaEffect
externdef ?sub_53831D@AreaEffect@@QAEXPAVUnit@@@Z :proc                        ; void AreaEffect::sub_53831D(Unit*)
externdef ?sub_537CD6@AreaEffect@@QAEXXZ :proc                                 ; void AreaEffect::sub_537CD6()
externdef ?sub_537C8C@AreaEffect@@QAEXXZ :proc                                 ; void AreaEffect::sub_537C8C()
externdef ?sub_53822C@AreaEffect@@QAEXXZ :proc                                 ; void AreaEffect::sub_53822C()
externdef ?sub_5384FF@AreaEffect@@QAEXXZ :proc                                 ; void AreaEffect::sub_5384FF()

; PointEffect
externdef ??0PointEffect@@QAE@XZ :proc                                         ; PointEffect::PointEffect()
externdef ??0PointEffect@@QAE@PAVEffect@@PAVUnit@@@Z :proc
externdef ??1PointEffect@@UAE@XZ :proc                                         ; PointEffect::~PointEffect()
externdef ?Serialize@PointEffect@@UAEXAAVCArchive@@@Z :proc                    ; void PointEffect::Serialize(CArchive&)
externdef ?VMethod2@PointEffect@@UAEXXZ :proc                                  ; void PointEffect::VMethod2()
externdef ?VMethod5@PointEffect@@UAEXXZ :proc                                  ; void PointEffect::VMethod5()
externdef ?classPointEffect@PointEffect@@2UCRuntimeClass@@A :byte              ; static CRuntimeClass PointEffect::classPointEffect

; SpellTransport
externdef ??0SpellTransport@@QAE@XZ :proc                                        ; SpellTransport::SpellTransport()
externdef ??0SpellTransport@@QAE@PAVSpellEffect@@PAUTokenPos@@F@Z :proc          ; SpellTransport::SpellTransport(SpellEffect*, TokenPos*, int16_t)
externdef ??0SpellTransport@@QAE@PAVAreaEffect@@PAUTokenPos@@F@Z :proc           ; SpellTransport::SpellTransport(AreaEffect*, TokenPos*, int16_t)
externdef ??1SpellTransport@@UAE@XZ :proc                                        ; SpellTransport::~SpellTransport()
externdef ?Serialize@SpellTransport@@UAEXAAVCArchive@@@Z :proc                   ; void SpellTransport::Serialize(CArchive&)
externdef ?VMethod2@SpellTransport@@UAEXXZ :proc                                 ; void SpellTransport::VMethod2()
externdef ?VMethod5@SpellTransport@@UAEXXZ :proc                                 ; void SpellTransport::VMethod5()
externdef ?classSpellTransport@SpellTransport@@2UCRuntimeClass@@A :byte          ; static CRuntimeClass SpellTransport::classSpellTransport


; SpellEffectList
externdef ??0SpellEffectList@@QAE@XZ :proc                                       ; SpellEffectList::SpellEffectList()
externdef ??1SpellEffectList@@QAE@XZ :proc                                       ; SpellEffectList::~SpellEffectList()
externdef ?sub_55BF31@SpellEffectList@@QAEXAAVCArchive@@@Z :proc


;EquipmentExtra
extern ??0EquipmentExtra@@QAE@XZ:proc   ; (public: __thiscall EquipmentExtra::EquipmentExtra(void))
extern ??4EquipmentExtra@@QAEAAU0@ABU0@@Z:proc   ; (public: struct EquipmentExtra & __thiscall EquipmentExtra::operator=(struct EquipmentExtra const &))
extern ?Serialize@EquipmentExtra@@QAEXAAVCArchive@@@Z:proc   ; (public: void __thiscall EquipmentExtra::Serialize(class CArchive &))
extern ?AddToUnit@EquipmentExtra@@QAEXPAVUnit@@@Z:proc   ; (public: void __thiscall EquipmentExtra::AddToUnit(class Unit *))


; Item
externdef ??0Item@@QAE@XZ :proc                                               ; Item::Item()
externdef ?classItem@Item@@2UCRuntimeClass@@A :byte                           ; static CRuntimeClass Item::classItem
externdef ??1Item@@UAE@XZ :proc                                               ; Item::~Item()
externdef ?GetRuntimeClass@Item@@UBEPAUCRuntimeClass@@XZ :proc                ; Item::GetRuntimeClass
externdef ?Serialize@Item@@UAEXAAVCArchive@@@Z :proc                          ; Item::Serialize
externdef ?VMethod1@Item@@UAEXXZ :proc
externdef ?VMethod2@Item@@UAEXXZ :proc
externdef ?VMethod6@Item@@UAEHXZ :proc
externdef ?VMethod10@Item@@UAEPAV1@PAVUnit@@@Z :proc
externdef ?VMethod11@Item@@UAEXPAVUnit@@@Z :proc
externdef ?TakeOne@Item@@UAEPAV1@XZ :proc
externdef ?VMethod13@Item@@UAEPAV1@XZ :proc
externdef ?VMethod14@Item@@UAEHHH@Z :proc
externdef ?VMethod15@Item@@UAEHXZ :proc
externdef ?VMethod16@Item@@UAEHXZ :proc
externdef ?VMethod17@Item@@UAEXPAVPacketUnitStateVec@@PAE@Z :proc
externdef ?WriteEffects@Item@@QAEXPAVPacketUnitStateVec@@PAE@Z :proc          ; sub_549F2C
externdef ?ApplyEffects@Item@@QAEXPAVUnit@@@Z :proc                           ; void Item::ApplyEffects(Unit*)
externdef ?RemoveEffects@Item@@QAEXPAVUnit@@@Z :proc                          ; void Item::RemoveEffects(Unit*)
externdef ??0Item@@QAE@ABVCString@@@Z :proc                                   ; Item::Item(const CString&)
externdef ??0Item@@QAE@EE@Z :proc                                             ; Item::Item(uint8_t, uint8_t)
externdef ??0Item@@QAE@PBV0@@Z :proc                                          ; Item::Item(const Item*)
externdef ?sub_548F6A@Item@@QAEHXZ :proc
externdef ?sub_548F07@Item@@QAEHXZ :proc
externdef ?sub_548E4E@Item@@QAEPAVEffect@@XZ :proc
externdef ?IsSimilar@Item@@QAEHPAV1@@Z :proc
externdef ?sub_548F3F@Item@@QAEXABVCString@@@Z :proc                           ; Item::sub_548F3F(const CString&)
externdef ?sub_54A0BE@Item@@QAEXXZ :proc                                       ; Item::sub_54A0BE()
extern ?StoreToPacket@Item@@QAEXPAVPacketUnitStateVec@@H@Z : proc


; Armor
externdef ?classArmor@Armor@@2UCRuntimeClass@@A :byte  ; static CRuntimeClass Armor::classArmor

externdef ??1Armor@@UAE@XZ :proc                                               ; Armor::~Armor()
externdef ??_GArmor@@UAEPAXI@Z :proc                                           ; Armor scalar deleting destructor
externdef ?LoadInfo@Armor@@QAEXXZ :proc
externdef ??0Armor@@QAE@XZ :proc                                               ; Armor::Armor()
externdef ??0Armor@@QAE@ABVCString@@@Z :proc                                   ; Armor::Armor(const CString&)
externdef ??0Armor@@QAE@EEE@Z :proc                                            ; Armor::Armor(uint8_t, uint8_t, uint8_t)
externdef ??0Armor@@QAE@PBV0@@Z :proc                                          ; Armor::Armor(const Armor*)
externdef ?Serialize@Armor@@UAEXAAVCArchive@@@Z :proc                          ; void Armor::Serialize(CArchive&)
externdef ?VMethod10@Armor@@UAEPAVItem@@PAVUnit@@@Z :proc                      ; Item* Armor::VMethod10(Unit*)
externdef ?VMethod11@Armor@@UAEXPAVUnit@@@Z :proc                              ; void Armor::VMethod11(Unit*)
externdef ?TakeOne@Armor@@UAEPAVItem@@XZ :proc                                 ; Item* Armor::TakeOne()
externdef ?VMethod13@Armor@@UAEPAVItem@@XZ :proc                               ; Item* Armor::VMethod13()
externdef ?VMethod15@Armor@@UAEHXZ :proc                                       ; int32_t Armor::VMethod15()
externdef ?VMethod17@Armor@@UAEXPAVPacketUnitStateVec@@PAE@Z :proc             ; void Armor::VMethod17(PacketUnitStateVec*, uint8_t*)


; Shield
externdef ?classShield@Shield@@2UCRuntimeClass@@A :byte  ; static CRuntimeClass Shield::classShield

externdef ??1Shield@@UAE@XZ :proc                                               ; Shield::~Shield()
externdef ??_GShield@@UAEPAXI@Z :proc                                           ; Shield scalar deleting destructor
externdef ?GetRuntimeClass@Shield@@UBEPAUCRuntimeClass@@XZ :proc               ; CRuntimeClass* Shield::GetRuntimeClass() const
externdef ?LoadInfo@Shield@@QAEXXZ :proc
externdef ??0Shield@@QAE@XZ :proc                                               ; Shield::Shield()
externdef ??0Shield@@QAE@ABVCString@@@Z :proc                                   ; Shield::Shield(const CString&)
externdef ??0Shield@@QAE@EEE@Z :proc                                            ; Shield::Shield(uint8_t, uint8_t, uint8_t)
externdef ??0Shield@@QAE@PBV0@@Z :proc                                          ; Shield::Shield(const Shield*)
externdef ?Serialize@Shield@@UAEXAAVCArchive@@@Z :proc                          ; void Shield::Serialize(CArchive&)
externdef ?VMethod10@Shield@@UAEPAVItem@@PAVUnit@@@Z :proc                      ; Item* Shield::VMethod10(Unit*)
externdef ?VMethod11@Shield@@UAEXPAVUnit@@@Z :proc                              ; void Shield::VMethod11(Unit*)
externdef ?TakeOne@Shield@@UAEPAVItem@@XZ :proc                                 ; Item* Shield::TakeOne()
externdef ?VMethod13@Shield@@UAEPAVItem@@XZ :proc                               ; Item* Shield::VMethod13()
externdef ?VMethod15@Shield@@UAEHXZ :proc                                       ; int32_t Shield::VMethod15()
externdef ?VMethod17@Shield@@UAEXPAVPacketUnitStateVec@@PAE@Z :proc             ; void Shield::VMethod17(PacketUnitStateVec*, uint8_t*)


; Weapon
externdef ?classWeapon@Weapon@@2UCRuntimeClass@@A :byte  ; static CRuntimeClass Weapon::classWeapon

externdef ??1Weapon@@UAE@XZ :proc                                               ; Weapon::~Weapon()
externdef ??_GWeapon@@UAEPAXI@Z :proc                                           ; Weapon scalar deleting destructor
externdef ?GetRuntimeClass@Weapon@@UBEPAUCRuntimeClass@@XZ :proc               ; CRuntimeClass* Weapon::GetRuntimeClass() const
externdef ?LoadEquipInfo@Weapon@@QAEXPAVWorldEquip@@@Z :proc
externdef ?LoadInfo@Weapon@@QAEXXZ :proc
externdef ??0Weapon@@QAE@XZ :proc                                               ; Weapon::Weapon()
externdef ??0Weapon@@QAE@ABVCString@@@Z :proc                                  ; Weapon::Weapon(const CString&)
externdef ??0Weapon@@QAE@EEE@Z :proc                                           ; Weapon::Weapon(uint8_t, uint8_t, uint8_t)
externdef ??0Weapon@@QAE@PBV0@@Z :proc                                         ; Weapon::Weapon(const Weapon*)
externdef ?Serialize@Weapon@@UAEXAAVCArchive@@@Z :proc                          ; void Weapon::Serialize(CArchive&)
externdef ?VMethod10@Weapon@@UAEPAVItem@@PAVUnit@@@Z :proc                      ; Item* Weapon::VMethod10(Unit*)
externdef ?VMethod11@Weapon@@UAEXPAVUnit@@@Z :proc                              ; void Weapon::VMethod11(Unit*)
externdef ?TakeOne@Weapon@@UAEPAVItem@@XZ :proc                                 ; Item* Weapon::TakeOne()
externdef ?VMethod13@Weapon@@UAEPAVItem@@XZ :proc                               ; Item* Weapon::VMethod13()
externdef ?VMethod15@Weapon@@UAEHXZ :proc                                       ; int32_t Weapon::VMethod15()
externdef ?VMethod17@Weapon@@UAEXPAVPacketUnitStateVec@@PAE@Z :proc             ; void Weapon::VMethod17(PacketUnitStateVec*, uint8_t*)


; Inventory
extern ?PutItemIntoBagAtDefault@Inventory@@QAEXPAVItem@@@Z:proc   ;   (public: void __thiscall Inventory::PutItemIntoBagAtDefault(class Item *))
externdef ?PutItemIntoBag@Inventory@@QAEXHPAVItem@@@Z:proc   ; (public: void __thiscall Inventory::PutItemIntoBag(int,class Item *))
externdef ??0Inventory@@QAE@XZ :proc
externdef ?sub_552E42@Inventory@@QAEPAVItem@@HH@Z :proc




;Spell

extern ?classSpell@Spell@@2UCRuntimeClass@@A :byte                          ; static CRuntimeClass Spell::classSpell
externdef ?sub_539F21@Spell@@QAEXPAVUnit@@0@Z :proc                           ; Spell::sub_539F21(Unit*,Unit*)
externdef ?sub_539F5A@Spell@@QAEXPAVUnit@@0CC@Z :proc                         ; Spell::sub_539F5A(Unit*,Unit*,int8_t,int8_t)
extern ?RefreshForHumanoid@SpellBook@@QAEXPAVHumanoid@@@Z:proc  ; SpellBook::RefreshForHumanoid()
externdef ??0Spell@@QAE@XZ:proc   ;  Spell::Spell(void)
externdef ??0Spell@@QAE@E@Z:proc  ;  Spell::Spell(uint8_t)
externdef ??0Spell@@QAE@ABVCString@@@Z:proc    ;  Spell::Spell(const CString&)
externdef ?sub_5393C7@Spell@@QAEHXZ :proc
externdef ?sub_53940D@Spell@@QAEXPAVUnit@@@Z:proc     ;  Spell::sub_53940D
externdef ?sub_539541@Spell@@QAEXI@Z :proc
externdef ?sub_539C49@Spell@@QAEXPAVUnit@@0@Z :proc

; SpellBook
externdef ??_7SpellBook@@6B@ :dword                                         ; SpellBook vtable
externdef ?sub_53D7F0@SpellBook@@QAEXHPAVSpell@@@Z :proc
externdef ?sub_53DD3D@SpellBook@@QAEHXZ :proc                               ; SpellBook::sub_53DD3D() – get current spell ID
extern ?sub_53DB79@SpellBook@@QAEPAVSpell@@H@Z :proc



;Packets
externdef ?Inst@Packet@@2V1@A : byte
externdef ??_7Packet@@6B@ : dword
externdef ?Duplicate@Packet@@UAEPAV1@XZ : proc

externdef ?Inst@PacketJoin@@2V1@A : byte
externdef ??0PacketJoin@@QAE@XZ : proc
externdef ??0PacketJoin@@QAE@VCString@@@Z : proc
externdef ??1PacketJoin@@UAE@XZ : proc

externdef ?Inst@PacketInfo@@2V1@A : byte

externdef ?Inst@PacketWord@@2V1@A : byte                              ; unk_6E9B40

externdef ?Inst@PacketItemOperation@@2V1@A : byte                     ; unk_6D1670
externdef ?AppendWord@PacketItemOperation@@QAEXG@Z : proc             ; sub_52695A
externdef ?AppendByteByte@PacketUnitStateVec@@QAEXEEPAE@Z : proc      ; sub_5275F8
externdef ?AppendByteInt@PacketUnitStateVec@@QAEXEHPAE@Z : proc       ; sub_5276E0


externdef ?Inst@PacketCmd@@2V1@A : byte                               ; unk_6B1650

externdef ?Inst@Packet3Dwords@@2V1@A : byte                           ; unk_6B1670

externdef ?Inst@PacketDword@@2V1@A : byte                             ; unk_6CDB78

externdef ?Inst@PacketAbility@@2V1@A : byte                           ; unk_70A7D0

externdef ?Inst@PacketEight@@2V1@A : byte                             ; unk_6CE970

externdef ?Inst@PacketMoveCmd@@2V1@A : byte                           ; unk_6A6A70

externdef ?Inst@PacketEffect@@2V1@A : byte                            ; unk_6CDB28

externdef ?Inst@PacketAoeZone@@2V1@A : byte                           ; unk_6B37A8

externdef ?Inst@PacketMount@@2V1@A : byte                             ; unk_6B37C8

externdef ?Inst@PacketPlayerInfo@@2V1@A : byte                        ; unk_6D1888

externdef ?Inst@PacketData@@2V1@A : byte                              ; unk_6B37D8

externdef ?Inst@PacketTerrain@@2V1@A : byte                           ; unk_6e9db0

externdef ?Inst@PacketUnitStateVec@@2V1@A : byte                      ; unk_6C5AF8
externdef ??0PacketUnitStateVec@@QAE@XZ : proc                        ; sub_527415
externdef ??1PacketUnitStateVec@@UAE@XZ : proc                        ; sub_527456

externdef ?Inst@PacketSync@@2V1@A : byte                              ; unk_6A6A88

externdef ?Inst@PacketUnitProperties@@2V1@A : byte                    ; unk_6D0628

externdef ?Inst@PacketPing@@2V1@A : byte                              ; unk_6CDB88

externdef ?Inst@PacketUnitUpdate@@2V1@A : byte                        ; _unk_6D1180



;PackerDat
extern ?DeleteTail@PackerDat@@QAEXPAUPackerTail@@@Z  : proc
extern ?Clear@PackerDat@@QAEXXZ : proc
extern ??0PackerDat@@QAE@XZ : proc
extern ??1PackerDat@@QAE@XZ : proc

;NetStru1

externdef ??_7NetStru1@@6B@ : dword
extern ??0NetStru1@@QAE@H@Z : proc
extern ??1NetStru1@@QAE@XZ : proc

extern ?HatConnector@NetStru1@@2V1@A : dword
extern ?g_NetStru1_main@@3VNetStru1@@A:byte
extern ?g_NetStru1_local@@3VNetStru1@@A:byte

extern ?AddTailNet2@NetStru1@@QAEXAAPAUNetStru2@@@Z : proc
extern ?AddTailFreeNet3@NetStru1@@QAEXPAUNetStru3@@@Z : proc
extern ?AllocClientBufManager@NetStru1@@QAEPAUNetStru2@@I@Z : proc
extern ?GetFreeNet3@NetStru1@@QAEPAUNetStru3@@XZ :proc

extern ?ProcessConnections@NetStru1@@QAEXXZ : proc
extern ?DisconnectClient@NetStru1@@QAEXPAUNetStru2@@@Z :proc



extern ?GetClientByPlayerID@NetStru1@@QAEPAUNetStru2@@G@Z : proc

externdef ?FUN_0051ce86@NetStru1@@QAEXIIPAVPlayer@@@Z :proc   ; NetStru1::FUN_0051ce86(unsigned int,unsigned int,class Player *)
externdef ?FUN_0051c748@NetStru1@@QAEXPAVPlayer@@@Z :proc   ; NetStru1::FUN_0051c748(class Player *)
externdef ?sub_51E63F@NetStru1@@QAEXHHH@Z :proc             ; void NetStru1::sub_51E63F(int, int, int)
externdef ?sub_51E794@NetStru1@@QAEXHHEF@Z :proc            ; void NetStru1::sub_51E794(int, int, uint8_t, short)

externdef ?FUN_0051d6b4@NetStru1@@QAEXG@Z :proc
externdef ?FUN_0051cefb@NetStru1@@QAEXEHHPAVPlayer@@@Z :proc
externdef ?sub_519221@NetStru1@@QAEXPAVUnit@@PAVPlayer@@IHHH@Z :proc
externdef ?sub_51BE8F@NetStru1@@QAEXPAVAreaEffect@@H@Z :proc
externdef ?sub_51CB21@NetStru1@@QAEXPAVPlayer@@@Z :proc
externdef ?sub_51CF5C@NetStru1@@QAEXPAVUnit@@HPAVPlayer@@@Z :proc
externdef ?sub_51D4F6@NetStru1@@QAEXPAVQuestMap@@PAVPlayer@@H@Z :proc
externdef ?sub_51B370@NetStru1@@QAEXPAVPacketMoveCmd@@PAUTokenPos@@@Z :proc
externdef ?sub_51CDFB@NetStru1@@QAEXAAVCString@@IDPAVPlayer@@@Z :proc  ; void NetStru1::sub_51CDFB(CString&, uint32_t, char, Player*)


externdef ?sub_51AC77@NetStru1@@QAEXPAVToken@@PAVPlayer@@E@Z :proc  ; NetStru1::sub_51AC77(Token*,Player*,uint8_t)

externdef ?sub_51C601@NetStru1@@QAEXPAVUnit@@H@Z :proc
externdef _dword_665CFC :dword
externdef ?sub_51D837@NetStru1@@QAEXHPAVPlayer@@@Z :proc

extern ?SendPacket_64@NetStru1@@QAEXIG@Z : proc

extern ?AddStat1@NetStru1@@QAEXPAUNetStru2@@H@Z : proc
extern ?AddPacketStatistic@NetStru1@@QAEXPAVPacket@@@Z : proc

extern ?SendAllData@NetStru1@@QAEXXZ : proc
extern ?QueuePacketSend@NetStru1@@QAEXPAVPacket@@@Z : proc
extern ?SetLLDriver@NetStru1@@QAEXPAUCLlDriver@@@Z : proc
extern ?SetLinkedHLDriver@NetStru1@@QAEXPAV1@@Z  : proc
extern ?GetClientByLowUid@NetStru1@@QAEPAUNetStru2@@I@Z : proc
extern ?ReceiveAnyPacket@NetStru1@@QAEPAVPacket@@XZ : proc
extern ?GetClientsPktNum@NetStru1@@QAEIXZ : proc
extern ?IsActive@NetStru1@@QAEHXZ : proc
externdef ?sub_51D4F6@NetStru1@@QAEXPAVQuestMap@@PAVPlayer@@H@Z :proc
externdef ?sub_51EEB7@NetStru1@@QAEXXZ :proc

externdef ?DAT_00666a00@@3VCStringArray@@A  : byte

;NetStru2

extern ?WriteData@NetStru2@@QAEHPAXI@Z : proc
extern ?ReadData@NetStru2@@QAEHPAXI@Z : proc

extern ?ReceiveData@NetStru2@@QAEHPAUNetStru3@@@Z : proc
extern ??1NetStru2@@QAE@XZ :proc

;NetStru3
extern ??0NetStru3@@QAE@XZ :proc
extern ??1NetStru3@@QAE@XZ :proc
extern ?Clear@NetStru3@@QAEXXZ :proc
extern ?WriteToBuffer@NetStru3@@QAEHPAXI@Z : proc
extern ?ReadFromBuffer@NetStru3@@QAEHPAXI@Z : proc


; Server
externdef ?g_Server@@3PAUServer@@A : dword
externdef ?g_IsServer@@3HA : dword
externdef ?g_ShutdownIn@@3HA : dword
externdef ?dword_6B37C4@@3PAVUnitList@@A :dword

externdef ?ServerTic@Server@@QAEXXZ :proc
externdef ?sub_4F1E2A@Server@@QAEXXZ :proc
externdef ?sub_4F465E@Server@@QAEXXZ :proc                                       ; void Server::sub_4F465E()
externdef ?sub_4F9E55@Server@@QAEXXZ :proc                                       ; void Server::sub_4F9E55()
externdef ?sub_4FAEB4@SrvStru1@@QAEXXZ :proc                                     ; void SrvStru1::sub_4FAEB4()
externdef ?sub_534A74@Player@@QAEXPAVUnit@@@Z :proc                              ; void Player::sub_534A74(Unit*)
externdef ?sub_52C641@Unit@@QAEXXZ :proc                                         ; void Unit::sub_52C641()
externdef ?sub_547468@CMultiShopTemplate@@QAEPAVCMultiShopInstance@@PAVHumanoid@@@Z :proc ; CMultiShopInstance* CMultiShopTemplate::sub_547468(Humanoid*)
externdef ?sub_5479C6@CMultiShopTemplate@@QAEXPAVHumanoid@@@Z :proc              ; void CMultiShopTemplate::sub_5479C6(Humanoid*)

extern ??1Server@@QAE@XZ :proc

extern ?Deinit@SrvStru1@@QAEXXZ :proc
externdef ?sub_4F1471@Server@@QAEHVCString@@@Z :proc    ; int Server::sub_4F1471(CString)
externdef ?sub_4F8831@Server@@QAEXXZ :proc
externdef _sub_4FA768 :proc                                                     ; void sub_4FA768(Player*)
externdef ?CheatCommand@Server@@QAEXPAVPlayer@@VCString@@@Z :proc
externdef ?sub_4EE028@Server@@QAEXPAVHumanoid@@@Z :proc
externdef ?sub_5013D4@Server@@QAEXPAVPlayer@@@Z :proc
externdef ?sub_4F950F@Server@@QAEXXZ :proc              ; void Server::sub_4F950F()
externdef ?sub_5090FA@Server@@QAEXPAVPlayer@@@Z :proc   ; void Server::sub_5090FA(Player*)
externdef ?sub_5585F2@SpellEffectList@@QAEXXZ :proc     ; void SpellEffectList::sub_5585F2()
externdef ?sub_5561A1@UnitList@@QAEXXZ :proc            ; void UnitList::sub_5561A1()
externdef ?sub_500907@Server@@QAEPAVHuman@@PAVPlayer@@EEEEEE@Z :proc


extern ?sub_504a96@Server@@QAEXPAVPacket@@@Z : proc
extern ?Start@Server@@QAEHH@Z : proc

extern ?FUN_00501b9e@Server@@QAEXHAAVCDWordArray@@@Z : proc


extern ?Instance@SrvStru1@@2U1@A : dword


; SrvStru1


externdef ?sub_51E5FB@NetStru1@@QAEXHH@Z :proc          ; void NetStru1::sub_51E5FB(int, int)
externdef ?sub_51E63F@NetStru1@@QAEXHHH@Z :proc         ; void NetStru1::sub_51E63F(int, int, int)
externdef ?sub_51E794@NetStru1@@QAEXHHEF@Z :proc        ; void NetStru1::sub_51E794(int, int, uint8_t, int16_t)
externdef ?sub_51E289@NetStru1@@QAEXHVCString@@HHH@Z :proc  ; void NetStru1::sub_51E289(int, CString, int, int, int)
externdef ?sub_51E40C@NetStru1@@QAEXPBDH@Z :proc   ; void NetStru1::sub_51E40C(const char*, int)


; CowardActivation



; World
externdef _g_World :dword

externdef ??0World@@QAE@PAUMapStuff@@PAVPlayersList@@@Z :proc  ; World::World(MapStuff*, PlayersList*)
externdef ??1World@@QAE@XZ :proc
externdef ?sub_5A9B6B@World@@QAEXPAVUnit@@@Z :proc                               ; World::sub_5A9B6B(Unit*)
extern ?sub_5A9AC4@World@@QAEXPAVUnit@@@Z :proc                               ; World::sub_5A9AC4(Unit*)
externdef ?sub_5AA7CF@World@@QAEXPAVUnit@@@Z :proc                               ; World::sub_5AA7CF(Unit*)
externdef ?sub_5AA91B@World@@QAEXPAVUnit@@0@Z :proc                              ; World::sub_5AA91B(Unit*, Unit*)
externdef ?sub_5AAA89@World@@QAEXPAVUnit@@@Z :proc                               ; World::sub_5AAA89(Unit*)
externdef ?DoScriptInstID@World@@QAEXH@Z :proc                                   ; World::DoScriptInstID(int)
externdef ?sub_5ABB32@World@@QAEEPAVUnit@@UPosYX@@@Z :proc                               ; uint8_t World::sub_5ABB32(Unit*, uint16_t)



externdef ?sub_510EE6@GameDataRes@@QAEPAVItem@@G@Z :proc
extern ?sub_4FBB24@SrvStru1@@QAEXEEEEII@Z :proc
extern ?sub_4FBAE3@SrvStru1@@QAEXEEPAVUnit@@II@Z :proc
externdef ?sub_596576@MapStuff@@QAEHGPAX@Z :proc
externdef ?sub_5A9383@World@@QAEXPAVUnit@@@Z :proc
externdef ?sub_5AC983@World@@QAEXPAVGroup@@H@Z :proc
externdef ?sub_5AC785@World@@QAEXPAVGroup@@EE@Z :proc
externdef ?sub_5AC507@World@@QAEXPAVGroup@@EE@Z :proc
externdef ?sub_5ACB9E@World@@QAEXPAVGroup@@PAVToken@@E@Z :proc
externdef ?sub_5AC7C8@World@@QAEXPAVGroup@@@Z :proc
externdef ?sub_5AFBFD@World@@QAEXXZ :proc
externdef ?sub_5ACAA7@World@@QAEXPAVGroup@@@Z :proc
externdef ?sub_5AC80F@World@@QAEXPAVGroup@@PAVToken@@@Z :proc
externdef ?sub_5ACB4D@World@@QAEXPAVGroup@@PAVToken@@E@Z :proc
externdef ?sub_5AC289@World@@QAEXPAVGroup@@EE@Z :proc
externdef ?sub_5ACBEF@World@@QAEXPAVGroup@@EE@Z :proc
externdef ?sub_5AC187@World@@QAEXPAVGroup@@PAVToken@@PAVSpell@@@Z :proc
externdef ?sub_5AC206@World@@QAEXPAVGroup@@EEPAVSpell@@@Z :proc
externdef ?sub_5A90F4@World@@QAEXPAVUnit@@PAVBuilding@@@Z :proc
externdef ?sub_5AC881@World@@QAEXPAVGroup@@EE@Z :proc
externdef ?sub_5AC8A2@World@@QAEXPAVGroup@@E@Z :proc
externdef ?sub_5ACAFA@World@@QAEXPAVGroup@@@Z :proc
externdef ?sub_5AF805@World@@QAEXHHHPAVPlayer@@@Z :proc
externdef ?sub_5AFA01@World@@QAEXHHHPAVPlayer@@@Z :proc
externdef ?sub_5AF683@World@@QAEXPAVPlayer@@H@Z :proc
externdef ?sub_5AA581@World@@QAEXPAVUnit@@0H@Z :proc
externdef ?sub_5B5643@Diplomacy@@QAEXPAVUnit@@0H@Z :proc  ; Diplomacy::sub_5B5643(Unit*, Unit*, int)
extern ?sub_5A9A6A@World@@QAEXPAVUnit@@@Z :proc
extern ?sub_5ACDF4@World@@QAEXPAVGroup@@@Z :proc
externdef ?sub_5A79C9@World@@QAEXPAVUnit@@@Z :proc              ; void World::sub_5A79C9(Unit*)
externdef ?sub_5A3AD6@World@@QAEXPAVUnit@@PAVUnitList@@@Z :proc  ; void World::sub_5A3AD6(Unit*, UnitList*)
externdef ?sub_5A3808@World@@QAEPAVUnitList@@PAVUnit@@UPosYX@@@Z :proc  ; UnitList* World::sub_5A3808(Unit*, uint16_t)
externdef ?sub_5A384F@World@@QAEPAVUnitList@@PAVUnit@@UPosYX@@@Z :proc  ; UnitList* World::sub_5A384F(Unit*, uint16_t)
extern ?UnitMaxRange@World@@QAEEPAVUnit@@@Z :proc               ; uint8_t World::sub_5A6ADB(Unit*)
externdef ?sub_5A6E2C@World@@QAEXPAVUnit@@H@Z :proc              ; void World::sub_5A6E2C(Unit*, int)
externdef ?sub_5A79D6@World@@QAEPAVSpell@@PAVUnit@@HH@Z :proc   ; Spell* World::sub_5A79D6(Unit*, int, int)
externdef ?sub_5A7A1C@World@@QAEHPAVUnit@@0@Z :proc             ; int World::sub_5A7A1C(Unit*, Unit*)
externdef ?sub_5A7AF7@World@@QAEHPAVUnit@@0PAVSpell@@@Z :proc   ; int World::sub_5A7AF7(Unit*, Unit*, Spell*)
externdef ?sub_5A7B44@World@@QAEXPAVUnit@@@Z :proc              ; void World::sub_5A7B44(Unit*)
externdef ?sub_5A85F4@World@@QAEXPAVUnit@@0PAVSpell@@@Z :proc    ; void World::sub_5A85F4(Unit*, Unit*, Spell*)
externdef ?sub_5ADD64@World@@QAEXPAVGroup@@@Z :proc              ; void World::sub_5ADD64(Group*)
externdef ?sub_5B5816@World@@QAE_NPAVUnit@@0@Z :proc             ; bool World::sub_5B5816(Unit*, Unit*)
externdef ?sub_5ABD16@World@@QAEXPAVPlayersList@@@Z :proc
extern ?FUN_005a9832@World@@QAEXPAVUnit@@@Z : proc
extern ?FUN_005a93f4@World@@QAEXPAVUnit@@@Z : proc
extern ?FUN_005acd4c@World@@QAEXPAVGroup@@@Z : proc
externdef ?sub_5AF6F5@World@@QAEXPAVUnit@@@Z :proc              ; void World::sub_5AF6F5(Unit*)

externdef ?sub_5B0556@World@@QAEXAAVCArchive@@@Z :proc
externdef ?sub_5B2E7A@World@@QAEXXZ :proc


; TriggerAction
externdef ??0TriggerAction@@QAE@XZ :proc                                         ; TriggerAction::TriggerAction()


; TriggerCheck
externdef ??0TriggerCheck@@QAE@XZ :proc                                          ; TriggerCheck::TriggerCheck()


; Player
externdef ?sub_534AC1@Player@@QAEXHI@Z :proc
extern ??0Player@@QAE@XZ :proc
extern ??1Player@@UAE@XZ :proc
extern ??5@YGAAVCArchive@@AAV0@AAPAVPlayer@@@Z : proc   ;  CArchive >> Player *&
externdef ?sub_5346AC@Player@@QAEXXZ :proc

externdef ?sub_534B17@Player@@QAEXXZ :proc

externdef ?sub_5347A1@Player@@QAEXPAVUnit@@@Z :proc


; PlayersList
externdef ?g_PlayersList@@3PAVPlayersList@@A :dword

externdef ??1World@@QAE@XZ :proc
externdef ??1MapStuff@@QAE@XZ :proc
extern ?CountHumanPlayers@PlayersList@@QAEHXZ :proc
externdef ?sub_535D39@PlayersList@@QAEPAVPlayer@@VCString@@@Z :proc
externdef ?sub_53636E@PlayersList@@QAEHXZ :proc
externdef ?sub_5357C6@PlayersList@@QAEXPAVPlayer@@@Z :proc
externdef ?sub_535B50@PlayersList@@QAEPAVPlayer@@G@Z :proc
externdef ?sub_535E94@PlayersList@@QAEPAVPlayer@@HH@Z :proc
externdef ?sub_535C46@PlayersList@@QAEPAVPlayer@@G@Z :proc                     ; Player* PlayersList::sub_535C46(uint16_t)
externdef ?CountCD@PlayersList@@QAEHXZ :proc
externdef ?sub_536286@PlayersList@@QAEXXZ :proc                                ; void PlayersList::sub_536286()
externdef ?sub_535F97@PlayersList@@QAEHXZ :proc                  ; int32_t PlayersList::sub_535F97()
externdef ?sub_55AFB3@PlayersList@@QAEXAAVCArchive@@@Z :proc

; Sack
externdef ??0Sack@@QAE@PBUTokenPos@@@Z :proc                                 ; Sack::Sack(const TokenPos*)
externdef ??0Sack@@QAE@PBUTokenPos@@PAUInventory@@@Z :proc                   ; Sack::Sack(const TokenPos*, Inventory*)
externdef ?classSack@Sack@@2UCRuntimeClass@@A :byte                          ; static CRuntimeClass Sack::classSack

; SackList method
externdef ?sub_554460@SackList@@QAEHPAUTokenPos@@PAUInventory@@HH@Z :proc
externdef ??1SackList@@QAE@XZ :proc   ; SackList::~SackList()
externdef ?sub_55BC58@SackList@@QAEXAAVCArchive@@@Z :proc

; Sack methods
externdef ?sub_55401E@Sack@@QAEXXZ :proc   ; void Sack::sub_55401E()

; Server methods

; MapStuff methods
externdef ?sub_58E407@MapStuff@@QAEHPAVSack@@@Z :proc   ; int MapStuff::sub_58E407(Sack*)

; Srv1
externdef ??1Srv1@@UAE@XZ :proc
externdef ?sub_59D891@Srv1@@QAEXPAUMapAlm@@H@Z :proc ; void Srv1::sub_59D891(MapAlm*, int)
externdef ?sub_59F1BE@Srv1@@QAEXPAUMapAlm@@@Z :proc  ; void Srv1::sub_59F1BE(MapAlm*)
externdef ?sub_59BEDB@Srv1@@QAEXPAVGroup@@PAUMapAlm@@@Z :proc  ; void Srv1::sub_59BEDB(Group*, MapAlm*)

; MapAlm
externdef ??0MapAlm@@QAE@PBD@Z :proc  ; MapAlm::MapAlm(const char*)
externdef ??1MapAlm@@QAE@XZ :proc      ; MapAlm::~MapAlm()

; Settings
externdef ??0Settings@@QAE@XZ :proc
externdef ?Serialize@Settings@@QAEXAAVCArchive@@@Z :proc


; ScriptSettings
externdef ?sub_5B681A@ScriptSettings@@QAEXAAVCArchive@@@Z :proc


; QuestMap
externdef ??0QuestMap@@QAE@XZ :proc                                              ; QuestMap::QuestMap()
externdef ??1QuestMap@@UAE@XZ :proc                                              ; QuestMap::~QuestMap()
externdef ??_GQuestMap@@UAEPAXI@Z :proc                                          ; QuestMap::`scalar deleting destructor'
externdef ?sub_55DD10@QuestMap@@QAEXHHH@Z :proc
externdef ?sub_55E24A@QuestMap@@QAEXPAVQuest@@@Z :proc
externdef ?sub_55F441@QuestMap@@QAEHH@Z :proc
externdef ?sub_55ECFE@QuestMap@@QAEXH@Z :proc
externdef ?sub_55E5FB@QuestMap@@QAEXPAVQuest@@@Z :proc                           ; void QuestMap::sub_55E5FB(Quest*)
externdef ?sub_55E00C@QuestMap@@QAEXXZ :proc
externdef ?sub_55D6F7@QuestMap@@QAEXXZ :proc                                    ; void QuestMap::sub_55D6F7()
externdef ?sub_55EA81@QuestMap@@QAEXPAVQuestInnGlue@@@Z :proc                  ; void QuestMap::sub_55EA81(QuestInnGlue*)

externdef ?g_QuestMap@@3VQuestMap@@A :byte


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
externdef ?GetText@CListBox@@QBEXHAAVCString@@@Z : proc
externdef ?AddString@CListBox@@QAEHPBD@Z  : proc
externdef ?ResetContent@CListBox@@QAEXXZ : proc
externdef ?SelectString@CListBox@@QAEHHPBD@Z : proc


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

externdef ?ShowWindow@CWnd@@QAEHH@Z  :proc  ;CWnd::ShowWindow
externdef ?SetWindowTextA@CWnd@@QAEXPBD@Z   :proc
externdef ?SetDlgItemTextA@CWnd@@QAEXHPBD@Z : proc


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


;SomeMainStructure
externdef ?FUN_004946e0@SomeMainStructure@@QAEXXZ  : proc
externdef ?FUN_004946ae@SomeMainStructure@@QAEXXZ  : proc
externdef ?FUN_00420070@SomeMainStructure@@QAE?AVCString@@XZ : proc
externdef ?FUN_00494982@SomeMainStructure@@QAEXXZ : proc



; MainWindow
externdef ?FUN_0048cb3c@MainWindow@@QAEXXZ : proc
extern ?Proc_421@MainWindow@@QAEXXZ : proc

extern ?FUN_0048f905@MainWindow@@QAEXXZ : proc
extern ?Proc_44c@MainWindow@@QAEXPAVCVisualObject@@@Z : proc
extern ?FUN_0048ca7e@MainWindow@@QAEHH@Z  : proc
extern ?FUN_0048cff7@MainWindow@@QAEXXZ : proc
extern ?FUN_00491822@MainWindow@@QAEXXZ : proc
extern ?FUN_004918ae@MainWindow@@QAEXXZ : proc

extern ?FUN_0041f6b0@BigStruct2@@QAEHXZ  : proc
extern ?FUN_0041b6e0@BigStruct2@@QAEXXZ  : proc
extern ?FUN_0048e502@MainWindow@@QAEHH@Z : proc
extern ?FUN_0048f79d@MainWindow@@QAEXXZ : proc

externdef ??0MainWindow@@QAE@XZ : proc
externdef ?CreateUI@MainWindow@@QAEXXZ  : proc
externdef ?LoadData@MainWindow@@QAEXXZ  : proc
externdef ?LoadSettings@MainWindow@@QAEXXZ  : proc
externdef ?sub_48A756@MainWindow@@QAEXXZ :proc
externdef ?FUN_00494a9e@MainWindow@@QAEXXZ : proc

externdef ?FUN_00491b3e@MainWindow@@QAEXXZ : proc
externdef ?FUN_00494c91@MainWindow@@QAEXXZ : proc
externdef ?FUN_00478130@MainWindow@@QAEXXZ : proc

extern ?SetSpeed@MainWindow@@QAEHH@Z :proc ; MainWindow::SetSpeed
extern ?UpdateClipCursor@MainWindow@@QAEHXZ  : proc
extern ?PopUpScreen@MainWindow@@QAEXPAVVisScreen@@@Z : proc
extern ?FUN_0048f6f7@MainWindow@@QAEXXZ : proc




extern ??0MWin_5e8@@QAE@XZ  :proc   ; (public: __thiscall MWin_5e8::MWin_5e8(void))
extern ??1MWin_5e8@@QAE@XZ  :proc   ; (public: __thiscall MWin_5e8::~MWin_5e8(void))
extern ?FUN_004ac3af@MWin_5e8@@QAEXXZ  :proc   ; (public: void __thiscall MWin_5e8::FUN_004ac3af(void))
extern ?FUN_00497270@MWin_5e8@@QAEXI@Z  :proc   ; (public: void __thiscall MWin_5e8::FUN_00497270(unsigned int))
extern ?FUN_004ac3ce@MWin_5e8@@QAEXABUFame1@@@Z  :proc   ; (public: void __thiscall MWin_5e8::FUN_004ac3ce(struct Fame1 const &))

extern ??0Fame2@@QAE@XZ  :proc   ; (public: __thiscall Fame2::Fame2(void))
extern ??4Fame2@@QAEXABU0@@Z  :proc   ; (public: void __thiscall Fame2::operator=(struct Fame2 const &))
extern ?Clear@Fame2@@QAEXXZ  :proc   ; (public: void __thiscall Fame2::Clear(void))
extern ?FUN_004abf0f@Fame2@@QAEXHI@Z  :proc   ; (public: void __thiscall Fame2::FUN_004abf0f(int,unsigned int))
extern ?FUN_004ad880@Fame2@@QAEHXZ  :proc   ; (public: int __thiscall Fame2::FUN_004ad880(void))
extern ?FUN_004ad890@Fame2@@QAEIXZ  :proc   ; (public: unsigned int __thiscall Fame2::FUN_004ad890(void))
extern ??1Fame2@@QAE@XZ  :proc   ; (public: __thiscall Fame2::~Fame2(void))

externdef @sub_49585B@4 :proc
externdef @sub_48A747@0 :proc

externdef ?FUN_00475e7a@@YAXXZ  :proc

extern ?ReadFileToString@@YAXPBDPAVCString@@@Z  : proc
extern ?GetRandS16@@YAHH@Z   : proc


extern ?g_CdSpeed@@3PADA : byte  ; char * g_CdSpeed
extern ?g_resolution@@3PADA : byte  ; char * g_resolution
extern ?g_InitialWorkingDir@@3PADA : byte  ; char * g_InitialWorkingDir
extern ?g_CmdLatency@@3IA : dword  ; unsigned int g_CmdLatency
extern ?g_CmdTimeout@@3IA : dword  ; unsigned int g_CmdTimeout
extern ?g_MainWndHWND@@3PAUHWND__@@A  : dword  ; HWND g_MainWndHWND
extern ?g_IsCdPresent@@3IA  : dword
extern ?g_EnableMusic@@3IA  : dword
extern ?g_EnableTrace@@3IA  : dword
extern ?g_DWORD_00659d18@@3IA  : dword
extern ?g_CmdSafeVideo@@3IA  : dword
extern ?INT_00660f74@@3IA  : dword

extern ?g_HelpStr@@3VCString@@A : byte


extern ?txt_main@@3VTxtFile@@A :byte   ; (class TxtFile txt_main)
extern ?txt_heropicture@@3VTxtFile@@A :byte   ; (class TxtFile txt_heropicture)
extern ?txt_stats@@3VTxtFile@@A :byte   ; (class TxtFile txt_stats)
extern ?txt_spells@@3VTxtFile@@A :byte   ; (class TxtFile txt_spells)
extern ?txt_spell@@3VTxtFile@@A :byte   ; (class TxtFile txt_spell)
extern ?txt_dialogs@@3VTxtFile@@A :byte   ; (class TxtFile txt_dialogs)
extern ?txt_unitname@@3VTxtFile@@A :byte   ; (class TxtFile txt_unitname)
extern ?txt_building@@3VTxtFile@@A :byte   ; (class TxtFile txt_building)
extern ?txt_itemname@@3VTxtFile@@A :byte   ; (class TxtFile txt_itemname)
extern ?txt_npcnames@@3VTxtFile@@A :byte   ; (class TxtFile txt_npcnames)
extern ?txt_cutscene@@3VTxtFile@@A :byte   ; (class TxtFile txt_cutscene)
extern ?txt_cutpaths@@3VTxtFile@@A :byte   ; (class TxtFile txt_cutpaths)
extern ?txt_tunes@@3VTxtFile@@A :byte   ; (class TxtFile txt_tunes)
extern ?txt_patch@@3VTxtFile@@A :byte   ; (class TxtFile txt_patch)

extern ?g_MessageColors@@3IA  : dword

extern ?g_ServerConfig@@3UServerConfig@@A   : dword


externdef ?ParseConfig@@YAHPBD@Z :proc
externdef ?LoadFonts@@YAXXZ  : proc
externdef ?LoadItemNames@@YAXXZ  : proc
extern ?unk_660D28@@3VCMapWordToPtr@@A :byte
extern ?unk_660DA8@@3VCMapWordToPtr@@A :byte
externdef _dword_65FBB8 :dword
externdef ?ParseTunes@@YAXXZ : proc
externdef ?PrecomputeExperienceLevelsTable@@YAXXZ : proc
externdef ?SetMessageColors@@YAXI@Z  : proc

externdef ?g_kbControlState@@3HA : dword
externdef ?g_kbShiftState@@3HA : dword
externdef ?g_kbMenuState@@3HA : dword

externdef ?FUN_00493cd8@SomeMainStructure@@QAEXXZ : proc
externdef ?FUN_00492c66@SomeMainStructure@@QAEXH@Z : proc
externdef ?ModalScreen@MainWindow@@QAEXPAVVisScreen@@@Z  : proc
extern ?FUN_00493ffe@SomeMainStructure@@QAEHXZ : proc
extern ?FUN_004941c0@SomeMainStructure@@QAEXXZ : proc

externdef ?g_LButtonDown@@3HA : dword

extern ?g_mapmusicinfos@@3V?$CArray@PAUMapMusicInfo@@ABQAU1@@@A : byte
extern ?g_settings@@3UGameSettings@@A : dword

extern ?DAT_00660f88@@3HA : dword
extern ?DAT_006658d8@@3VCDWordArray@@A : dword



;A2NetSock
extern ??0A2NetSock@@QAE@XZ :proc
extern ??1A2NetSock@@QAE@XZ :proc


; CLlDriver
extern ??0CLlDriver@@QAE@U_GUID@@PAVNetStru1@@@Z  : proc
extern ??1CLlDriver@@QAE@XZ  : proc

extern ?Close@CLlDriver@@QAEXXZ :proc

extern ?EnumAddresses@CLlDriver@@QAEHPAPAUCLlAddress@@PAH@Z : proc

extern ?GetClientBySocketId@CLlDriver@@QAEPAUA2NetSock@@I@Z : proc

extern ?ResetProvider@CLlDriver@@QAEHH@Z : proc
extern ?RecreateDp@CLlDriver@@QAEXXZ : proc

extern ?EnumSessions@CLlDriver@@QAEHPAPAUCLlNetSession@@PAHP6AHXZI@Z :proc
extern ?cbEnumSessions@CLlDriver@@SGHPBUtagDPSESSIONDESC2@@PAKKPAX@Z :proc

extern ?EnumConnections@CLlDriver@@QAEXPAPAUCLlConn@@PAH@Z : proc

extern ?EnumPlayers@CLlDriver@@QAEXPAUCLlNetSession@@PAPAUCLlName@@PAH@Z  : proc

extern ?StartServer@CLlDriver@@QAEHHPBDPAUCLlAddress@@@Z : proc

extern ?PrepareForConnect@CLlDriver@@QAEHPBDPAUCLlAddress@@@Z : proc
extern ?Connect@CLlDriver@@QAEHPBDPAUCLlNetSession@@@Z : proc

extern ?SendData@CLlDriver@@QAEHIPAUNetStru3@@@Z :proc

extern ?DisconnectClient@CLlDriver@@QAEXI@Z :proc
extern ?CleanupInvalidTcpClient@CLlDriver@@QAEXI@Z :proc
extern ?CleanupAllInvalid@CLlDriver@@QAEXXZ : proc
extern ?RestartModemServerDp@CLlDriver@@QAEHXZ : proc

extern ?SetLatency@CLlDriver@@QAEXIH@Z :proc

extern ?g_CLlDriver@@3UCLlDriver@@A  : dword
extern ?g_HatLLDriver@@3UCLlDriver@@A : dword


;TxtFile

extern ?AllLines@TxtFile@@2V?$CArray@PADABQAD@@A  : byte  ; TxtFile::AllLines

extern ??0TxtFile@@QAE@XZ :proc   ; (public: __thiscall TxtFile::TxtFile(void))
extern ??1TxtFile@@UAE@XZ :proc   ; (public: virtual __thiscall TxtFile::~TxtFile(void))
extern ?LoadChunkFile@TxtFile@@QAEXPBD@Z :proc   ; (public: void __thiscall TxtFile::LoadChunkFile(char const *))
extern ?Free@TxtFile@@QAEXXZ :proc   ; (public: void __thiscall TxtFile::Free(void))
extern ?GetLine@TxtFile@@QAEPADH@Z :proc   ; (public: char * __thiscall TxtFile::GetLine(int))
extern ?GetCount@TxtFile@@QAEHXZ   :proc   ; TxtFile::GetCount








;CVisualObject
extern ??0CVisualObject@@QAE@XZ :proc   ; (public: __thiscall CVisualObject::CVisualObject(void))
extern ??0CVisualObject@@QAE@HABUtagRECT@@PBD@Z :proc   ; (public: __thiscall CVisualObject::CVisualObject(int,struct tagRECT const &,char const *))
extern ??0CVisualObject@@QAE@HHHHHPBD@Z :proc   ; (public: __thiscall CVisualObject::CVisualObject(int,int,int,int,int,char const *))

extern ??1CVisualObject@@UAE@XZ  :proc    ;   CVisualObject::~CVisualObject
extern ?Dump@CVisualObject@@UBEXAAVCDumpContext@@@Z :proc   ; (public: virtual void __thiscall CVisualObject::Dump(class CDumpContext &)const )
extern ?GetHint@CVisualObject@@UAEPBDXZ :proc   ; (public: virtual char const * __thiscall CVisualObject::GetHint(void))
extern ?SetHint@CVisualObject@@UAEXPBD@Z :proc   ; (public: virtual void __thiscall CVisualObject::SetName(char const *))
extern ?ChangeFlags@CVisualObject@@UAEXI_N@Z :proc   ; (public: virtual void __thiscall CVisualObject::ChangeFlags(unsigned int,bool))
extern ?TestFlags@CVisualObject@@UAEII@Z :proc   ; (public: virtual unsigned int __thiscall CVisualObject::TestFlags(unsigned int))
extern ?SetCursorOver@CVisualObject@@UAEX_N@Z :proc   ; (public: virtual void __thiscall CVisualObject::SetCursorOver(bool))
extern ?SetFocus@CVisualObject@@UAEX_N@Z :proc   ; (public: virtual void __thiscall CVisualObject::SetFocus(bool))
extern ?VMethod7@CVisualObject@@UAEXXZ :proc   ; (public: virtual void __thiscall CVisualObject::VMethod7(void))
extern ?VMethod8@CVisualObject@@UAEXPAVCRect@@@Z :proc   ; (public: virtual void __thiscall CVisualObject::VMethod8(struct tagRECT *))
extern ?VMethod9@CVisualObject@@UAEXXZ :proc   ; (public: virtual void __thiscall CVisualObject::VMethod9(void))
extern ?VMethod10@CVisualObject@@UAEXXZ :proc   ; (public: virtual void __thiscall CVisualObject::VMethod10(void))
extern ?WriteData@CVisualObject@@UAEXPAX@Z :proc   ; (public: virtual void __thiscall CVisualObject::WriteData(void *))
extern ?DataSize@CVisualObject@@UAEIXZ :proc   ; (public: virtual unsigned int __thiscall CVisualObject::DataSize(void))
extern ?ReadData@CVisualObject@@UAEXPBX@Z :proc   ; (public: virtual void __thiscall CVisualObject::ReadData(void *))
extern ?MsgProc@CVisualObject@@UAEHIII@Z :proc   ; (public: virtual int __thiscall CVisualObject::MsgProc(unsigned int,unsigned int,unsigned int))
extern ?OnMouseMove@CVisualObject@@UAEHIVCPoint@@@Z :proc   ; (public: virtual int __thiscall CVisualObject::OnMouseMove(unsigned int,class CPoint))
extern ?OnWmUser@CVisualObject@@UAEHIVCPoint@@@Z :proc   ; (public: virtual int __thiscall CVisualObject::OnWmUser(unsigned int,class CPoint))
extern ?OnLButtonDown@CVisualObject@@UAEHIVCPoint@@@Z :proc   ; (public: virtual int __thiscall CVisualObject::OnLButtonDown(unsigned int,class CPoint))
extern ?OnLButtonUp@CVisualObject@@UAEHIVCPoint@@@Z :proc   ; (public: virtual int __thiscall CVisualObject::OnLButtonUp(unsigned int,class CPoint))
extern ?OnLButtonDblClk@CVisualObject@@UAEHIVCPoint@@@Z :proc   ; (public: virtual int __thiscall CVisualObject::OnLButtonDblClk(unsigned int,class CPoint))
extern ?OnRButtonDown@CVisualObject@@UAEHIVCPoint@@@Z :proc   ; (public: virtual int __thiscall CVisualObject::OnRButtonDown(unsigned int,class CPoint))
extern ?OnRButtonUp@CVisualObject@@UAEHIVCPoint@@@Z :proc   ; (public: virtual int __thiscall CVisualObject::OnRButtonUp(unsigned int,class CPoint))
extern ?OnRButtonDblClk@CVisualObject@@UAEHIVCPoint@@@Z :proc   ; (public: virtual int __thiscall CVisualObject::OnRButtonDblClk(unsigned int,class CPoint))
extern ?OnKeyDown@CVisualObject@@UAEHI@Z :proc   ; (public: virtual int __thiscall CVisualObject::OnKeyDown(unsigned int))
extern ?OnKeyUp@CVisualObject@@UAEHI@Z :proc   ; (public: virtual int __thiscall CVisualObject::OnKeyUp(unsigned int))
extern ?OnChar@CVisualObject@@UAEHI@Z :proc   ; (public: virtual int __thiscall CVisualObject::OnChar(unsigned int))

extern ?MsgProcOnChilds@CVisualObject@@QAEHIII@Z  :proc    ;   CVisualObject::MsgProcOnChilds
extern ?AddChild@CVisualObject@@QAEXPAV1@@Z :proc   ; (public: void __thiscall CVisualObject::AddChild(class CVisualObject *))
extern ?RemoveChild@CVisualObject@@QAEXPAV1@@Z :proc   ; (public: void __thiscall CVisualObject::RemoveChild(class CVisualObject *))
extern ?RemoveChildById@CVisualObject@@QAEXH@Z :proc   ; (public: void __thiscall CVisualObject::RemoveChildById(int))
extern ?RemoveAllChilds@CVisualObject@@QAEXXZ :proc   ; (public: void __thiscall CVisualObject::RemoveAllChilds(void))
extern ?DestroyChild@CVisualObject@@QAEXPAV1@@Z :proc   ; (public: void __thiscall CVisualObject::DestroyChild(class CVisualObject *))
extern ?DestroyAllChilds@CVisualObject@@QAEXXZ :proc   ; (public: void __thiscall CVisualObject::DestroyAllChilds(void))
extern ?FindChild@CVisualObject@@QAEPAV1@H@Z :proc   ; (public: class CVisualObject * __thiscall CVisualObject::FindChild(int))
extern ?GetChildAt@CVisualObject@@QAEPAV1@UtagPOINT@@@Z :proc   ; (public: class CVisualObject * __thiscall CVisualObject::GetChildAt(struct tagPOINT))
extern ?ClientPtToScreen@CVisualObject@@QAEXPAVCPoint@@ABV2@@Z :proc   ; (public: void __thiscall CVisualObject::ClientPtToScreen(class CPoint *,class CPoint const &))
extern ?ClientRectToScreen@CVisualObject@@QAEXPAVCRect@@ABV2@@Z :proc   ; (public: void __thiscall CVisualObject::ClientRectToScreen(class CRect *,class CRect const &))
extern ?SetLeftObj@CVisualObject@@QAEXPAV1@@Z :proc   ; (public: void __thiscall CVisualObject::SetLeftObj(class CVisualObject *))
extern ?SetRightObj@CVisualObject@@QAEXPAV1@@Z :proc   ; (public: void __thiscall CVisualObject::SetRightObj(class CVisualObject *))
extern ?SetUpObj@CVisualObject@@QAEXPAV1@@Z :proc   ; (public: void __thiscall CVisualObject::SetUpObj(class CVisualObject *))
extern ?SetDownObj@CVisualObject@@QAEXPAV1@@Z :proc   ; (public: void __thiscall CVisualObject::SetDownObj(class CVisualObject *))
extern ?SetCaptionLabel@CVisualObject@@QAEXPAVVisLabel@@@Z :proc   ; (public: void __thiscall CVisualObject::SetCaptionLabel(class VisLabel *))
extern ?FocusTo@CVisualObject@@QAEXPAV1@_N@Z :proc   ; (public: void __thiscall CVisualObject::FocusTo(class CVisualObject *,bool))
extern ?TabFocus@CVisualObject@@QAEX_N0@Z :proc   ; (public: void __thiscall CVisualObject::TabFocus(bool,bool))
extern ?SetRect@CVisualObject@@QAEXUtagRECT@@@Z :proc   ; (public: void __thiscall CVisualObject::SetRect(struct tagRECT))
extern ?SetRect@CVisualObject@@QAEXPBUtagRECT@@@Z :proc   ; (public: void __thiscall CVisualObject::SetRect(struct tagRECT const *))

extern ?GetId@CVisualObject@@QAEHXZ : proc
extern ?GetRect@CVisualObject@@QAEAAVCRect@@XZ : proc


;VizLabel
extern ??0VisLabel@@QAE@HHHHHPBDPAVCGameFont@@PAGI@Z :proc   ; (public: __thiscall VisLabel::VisLabel(int,int,int,int,int,char const *,class CGameFont *,unsigned short *,unsigned int))
extern ??0VisLabel@@QAE@HABUtagRECT@@PBDPAVCGameFont@@PAGI@Z :proc   ; (public: __thiscall VisLabel::VisLabel(int,struct tagRECT const &,char const *,class CGameFont *,unsigned short *,unsigned i
extern ?VMethod7@VisLabel@@UAEXXZ :proc   ; (public: virtual void __thiscall VisLabel::VMethod7(void))
extern ?SetActiveColor@VisLabel@@QAEX_N@Z :proc   ; (public: void __thiscall VisLabel::SetActiveColor(bool))
extern ??1VisLabel@@UAE@XZ :proc   ; (public: virtual __thiscall VisLabel::~VisLabel(void))


;VisButton
extern ??1VisButton@@UAE@XZ  :proc
extern ?VMethod7@VisButton@@UAEXXZ  :proc
extern ?OnMouseMove@VisButton@@UAEHIVCPoint@@@Z  :proc
extern ?OnLButtonDown@VisButton@@UAEHIVCPoint@@@Z  :proc
extern ?OnLButtonUp@VisButton@@UAEHIVCPoint@@@Z  :proc
extern ?OnKeyDown@VisButton@@UAEHI@Z  :proc
extern ?OnChar@VisButton@@UAEHI@Z  :proc
extern ??0VisButton@@QAE@HABUtagRECT@@PBDPAVCGameFont@@PAGHH1@Z  :proc
extern ??0VisButton@@QAE@HHHHHPBDPAVCGameFont@@PAGHH0@Z  :proc
extern ?SetDowned@VisButton@@QAEX_N@Z  :proc

;VisScrollBar
extern ?SetPos@VisScrollBar@@QAEXHH@Z  :proc
extern ??0VisScrollBar@@QAE@HHHHHPBD@Z  :proc

;VisListBox
extern ??1VisListBox@@UAE@XZ  :proc
extern ?VMethod7@VisListBox@@UAEXXZ  :proc
extern ?WriteData@VisListBox@@UAEXPAX@Z  :proc
extern ?DataSize@VisListBox@@UAEIXZ  :proc
extern ?ReadData@VisListBox@@UAEXPBX@Z  :proc
extern ?MsgProc@VisListBox@@UAEHIII@Z  :proc
extern ?OnMouseMove@VisListBox@@UAEHIVCPoint@@@Z  :proc
extern ?OnLButtonDown@VisListBox@@UAEHIVCPoint@@@Z  :proc
extern ?OnLButtonUp@VisListBox@@UAEHIVCPoint@@@Z  :proc
extern ?OnLButtonDblClk@VisListBox@@UAEHIVCPoint@@@Z  :proc
extern ?OnKeyDown@VisListBox@@UAEHI@Z  :proc
extern ?IsValidIndex@VisListBox@@UAEHH@Z  :proc
extern ?DrawItem@VisListBox@@UAEXHVCPoint@@PAG@Z  :proc
extern ?SelectItem@VisListBox@@UAEXH@Z  :proc
extern ?GetItemCount@VisListBox@@UAEHXZ  :proc
extern ?VMethod30@VisListBox@@UAEXVCPoint@@ABVCRect@@@Z  :proc
extern ??0VisListBox@@QAE@HABUtagRECT@@PAVCGameFont@@PAG2HPBD@Z  :proc
extern ??0VisListBox@@QAE@HHHHHPAVCGameFont@@PAG1HPBD@Z  :proc
extern ?Down@VisListBox@@QAEXXZ  :proc
extern ?Up@VisListBox@@QAEXXZ  :proc
extern ?PageDown@VisListBox@@QAEXXZ  :proc
extern ?PageUp@VisListBox@@QAEXXZ  :proc
extern ?YToIndex@VisListBox@@QAEHH@Z  :proc
extern ?GetItem@VisListBox@@QAEAAVCString@@H@Z : proc
extern ?SetSelectedIndex@VisListBox@@QAEXH@Z : proc
extern ?GetSelectedIndex@VisListBox@@QAEHXZ : proc

;VisTextBox
extern ??1VisTextBox@@UAE@XZ  :proc
extern ?VMethod7@VisTextBox@@UAEXXZ  :proc
extern ?WriteData@VisTextBox@@UAEXPAX@Z  :proc
extern ?DataSize@VisTextBox@@UAEIXZ  :proc
extern ?ReadData@VisTextBox@@UAEXPBX@Z  :proc
extern ?MsgProc@VisTextBox@@UAEHIII@Z  :proc
extern ?OnMouseMove@VisTextBox@@UAEHIVCPoint@@@Z  :proc
extern ?OnLButtonDown@VisTextBox@@UAEHIVCPoint@@@Z  :proc
extern ?OnKeyDown@VisTextBox@@UAEHI@Z  :proc
extern ?OnChar@VisTextBox@@UAEHI@Z  :proc
extern ??0VisTextBox@@QAE@HHHHHPAVCGameFont@@PAGPBD@Z  :proc
extern ??0VisTextBox@@QAE@HABUtagRECT@@PAVCGameFont@@PAGPBD@Z  :proc

;VisRadioBase
extern ??1VisRadioBase@@UAE@XZ  :proc
extern ?SetCursorOver@VisRadioBase@@UAEX_N@Z  :proc
extern ?WriteData@VisRadioBase@@UAEXPAX@Z  :proc
extern ?DataSize@VisRadioBase@@UAEIXZ  :proc
extern ?ReadData@VisRadioBase@@UAEXPBX@Z  :proc
extern ?OnMouseMove@VisRadioBase@@UAEHIVCPoint@@@Z  :proc
extern ?GetIndex@VisRadioBase@@UAEHH@Z  :proc
extern ??0VisRadioBase@@QAE@HHHHHPAVCGameFont@@PAGPBD@Z  :proc
extern ?AddEntry@VisRadioBase@@QAEXPBD@Z  :proc

;VisRadioType1
extern ??1VisRadioType1@@UAE@XZ  :proc
extern ?ReadData@VisRadioType1@@UAEXPBX@Z  :proc
extern ?OnMouseMove@VisRadioType1@@UAEHIVCPoint@@@Z  :proc
extern ?OnLButtonDown@VisRadioType1@@UAEHIVCPoint@@@Z  :proc
extern ?OnLButtonDblClk@VisRadioType1@@UAEHIVCPoint@@@Z  :proc
extern ?OnKeyDown@VisRadioType1@@UAEHI@Z  :proc
extern ?OnChar@VisRadioType1@@UAEHI@Z  :proc
extern ??0VisRadioType1@@QAE@HHHHHPAVCGameFont@@PAGPBD@Z  :proc

;VisRadioType2
extern ??1VisRadioType2@@UAE@XZ  :proc
extern ?OnMouseMove@VisRadioType2@@UAEHIVCPoint@@@Z  :proc
extern ?OnLButtonDown@VisRadioType2@@UAEHIVCPoint@@@Z  :proc
extern ?OnKeyDown@VisRadioType2@@UAEHI@Z  :proc
extern ?OnChar@VisRadioType2@@UAEHI@Z  :proc
extern ??0VisRadioType2@@QAE@HHHHHPAVCGameFont@@PAGPBD@Z  :proc

;VisBitmap
extern ??0VisBitmap@@QAE@HHHHHPAVCGameBitmap@@H@Z  :proc
extern ??0VisBitmap@@QAE@HABUtagRECT@@PAVCGameBitmap@@H@Z  :proc
extern ?GetBitmap@VisBitmap@@QAEPAVCGameBitmap@@XZ  :proc
extern ?SetBitmap@VisBitmap@@QAEXPAVCGameBitmap@@@Z  :proc


;VisComboBox
extern ??0VisComboBox@@QAE@HVCRect@@PBD@Z  :proc
extern ?AddItem@VisComboBox@@QAEXPBD@Z  :proc
extern ?ToggleList@VisComboBox@@QAEXXZ  :proc
extern ?HideList@VisComboBox@@QAEXXZ  :proc
extern ?ProcSelectList@VisComboBox@@QAEXXZ  :proc
extern ?SelectItem@VisComboBox@@QAEXH@Z  :proc


;VisMultiText
extern ??0VisMultiText@@QAE@HHHHHPBDPAVCGameFont@@PAGH@Z  :proc
extern ??0VisMultiText@@QAE@HABUtagRECT@@PBDPAVCGameFont@@PAGH@Z  :proc
extern ?SetText@VisMultiText@@QAEXPBD@Z  : proc
extern ?SizesCheck@VisMultiText@@QAEXXZ : proc


;VisScreen
extern ??1VisScreen@@UAE@XZ  :proc
extern ?VMethod7@VisScreen@@UAEXXZ  :proc
extern ?VMethod8@VisScreen@@UAEXPAVCRect@@@Z  :proc
extern ?MsgProc@VisScreen@@UAEHIII@Z  :proc
extern ?OnLButtonDown@VisScreen@@UAEHIVCPoint@@@Z  :proc
extern ?OnKeyDown@VisScreen@@UAEHI@Z  :proc
extern ?VMethod26@VisScreen@@UAEXXZ  :proc
extern ?VMethod27@VisScreen@@UAEXXZ  :proc
extern ?VMethod28@VisScreen@@UAEXXZ  :proc
extern ?DoClose@VisScreen@@UAEXI@Z  :proc
extern ??0VisScreen@@QAE@XZ  :proc
extern ??0VisScreen@@QAE@HABUtagRECT@@PAVCGameBitmap@@@Z  :proc
extern ??0VisScreen@@QAE@HHHHHPAVCGameBitmap@@@Z  :proc
extern ?GetCloseCode@VisScreen@@QAEHXZ  :proc
extern ?CloseOk@VisScreen@@QAEXXZ  :proc
extern ?CloseCancel@VisScreen@@QAEXXZ  :proc


;VisWindow
extern ??1VisWindow@@UAE@XZ  :proc
extern ?OnKeyDown@VisWindow@@UAEHI@Z  :proc
extern ??0VisWindow@@QAE@HHHHHPAVCGameBitmap@@@Z  :proc
extern ?UpdateWinRect@VisWindow@@QAEXXZ  :proc


;VisCharSelect
externdef ?OpenRenameWindow@VisCharSelect@@QAEXXZ : proc
externdef ?FUN_00432655@VisCharSelect@@QAEXPAVCUnit@@@Z  : proc

;VisCharSellectButtons
extern ?OnLButtonUp@VisCharSellectButtons@@UAEHIVCPoint@@@Z : proc


;VisMessageBox
extern ??1VisMessageBox@@UAE@XZ  :proc
extern ?MsgProc@VisMessageBox@@UAEHIII@Z  :proc
extern ?VMethod26@VisMessageBox@@UAEXXZ  :proc
extern ?VMethod31@VisMessageBox@@UAEXH@Z  :proc
extern ??0VisMessageBox@@QAE@HHHHHPBD0H0@Z  :proc

;VisMessageBoxWithList
extern ??1VisMessageBoxWithList@@UAE@XZ  :proc
extern ?VMethod30@VisMessageBoxWithList@@UAEPAVCVisualObject@@PBDABUtagRECT@@@Z  :proc
extern ??0VisMessageBoxWithList@@QAE@HHHHHPBD0H@Z  :proc

;VisCharSellectStats
externdef ?FUN_0042f6f3@VisCharSellectStats@@QAEXXZ  : proc

;VisMenuWnd
extern ??0VisMenuWnd@@QAE@HHHHHPAVCGameBitmap@@IABVCRect@@@Z : proc
extern ??1VisMenuWnd@@UAE@XZ : proc
extern ?MsgProc@VisMenuWnd@@UAEHIII@Z : proc
extern ?OnKeyDown@VisMenuWnd@@UAEHI@Z : proc
extern ?AddElement@VisMenuWnd@@QAEXPAVCVisualObject@@H@Z : proc

;MenuButton
extern ??0MenuButton@@QAE@HPBDPAVCGameFont@@PAGHH0@Z : proc
extern ??0MenuButton@@QAE@HABUtagRECT@@PBDPAVCGameFont@@PAGHH1@Z : proc

;IngameMenu
extern ??0IngameMenu@@QAE@HHHHHPAVCGameBitmap@@IABVCRect@@@Z : proc

;HintedListBox
extern ??0HintedListBox@@QAE@HHHHHPAVCGameFont@@PAG1HPBD@Z  :proc
extern ?SetHints@HintedListBox@@QAEXPAVCStringArray@@@Z  :proc

;LoadGameWindow SaveGameWindow
extern ??0LoadGameWindow@@QAE@HHHHHPAVCGameBitmap@@PAUMWin_Unk1@@@Z : proc
extern ??0SaveGameWindow@@QAE@HHHHHPAVCGameBitmap@@PAUMWin_Unk1@@@Z : proc


;GM_a28
extern ??1GM_a28@@UAE@XZ  :proc
extern ??0GM_a28@@QAE@XZ  :proc
extern ?Add@GM_a28@@QAEXPBDPAGI@Z  :proc
extern ?AddWithCheck@GM_a28@@QAEXPBDPAGI@Z  :proc
extern ?Clear@GM_a28@@QAEXXZ  :proc
extern ?Draw@GM_a28@@QAEXXZ  :proc
extern ?SetRect@GM_a28@@QAEXABVCRect@@@Z  :proc
extern ?Update@GM_a28@@QAEXXZ  :proc

;TakeDamage
extern ?AddDamage@TakeDamage@@SAXPAV?$CArray@UTakeDamage@@ABU1@@@ABU1@@Z  :proc
extern ?UpdateDamages@TakeDamage@@SAXPAV?$CArray@UTakeDamage@@ABU1@@@@Z : proc
extern ?DrawDamages@TakeDamage@@SAXPAV?$CArray@UTakeDamage@@ABU1@@@@Z  : proc
extern ??1TakeDamage@@QAE@XZ : proc
extern ??0TakeDamage@@QAE@HPAGHHHPAVCUnit@@@Z : proc
extern ??0TakeDamage@@QAE@XZ : proc

;BigStruct2
extern ?ProcessPackets@BigStruct2@@QAEHE@Z : proc
externdef ?FUN_00416cf7@BigStruct2@@QAEXXZ  : proc
externdef ?GetUnit_3f6c@BigStruct2@@QAEPAVCUnit@@XZ : proc
externdef ?FUN_0041c4a1@BigStruct2@@QAEXPBD@Z  : proc
externdef ?FUN_0041b7b7@BigStruct2@@QAEXHHPAH0@Z : proc
externdef ?FUN_0041c74b@BigStruct2@@QAEXHHHHHPAEH@Z : proc
externdef ?FUN_0041df23@BigStruct2@@QAEPAVCUnit@@H@Z : proc
externdef ?FUN_0041c630@BigStruct2@@QAEXPAVCWordArray@@@Z : proc
externdef ?FUN_0041cb67@BigStruct2@@QAEXH@Z : proc
externdef ?FUN_0041cb21@BigStruct2@@QAEXXZ  : proc
externdef ?FUN_00403ca0@BigStruct2@@QAEXPAVCGameObject@@@Z  : proc
externdef ?FUN_0041d97e@BigStruct2@@QAEXH@Z : proc

extern ?FUN_0041b2a4@BigStruct2@@QAEXPBDHH@Z : proc
extern ?FUN_0040d7f3@BigStruct2@@QAEHXZ : proc
extern ?FUN_0041d2da@BigStruct2@@QAEXH@Z : proc

extern ?FUN_0041aaaa@BigStruct2@@QAEXH@Z : proc
extern ?FUN_0041abd2@BigStruct2@@QAEXH@Z : proc
extern ?FUN_0041ab74@BigStruct2@@QAEXXZ : proc
extern ?FUN_0041b636@BigStruct2@@QAEXXZ : proc
extern ?FUN_0041b509@BigStruct2@@QAEXXZ : proc
extern ?FUN_0041b40e@BigStruct2@@QAEXXZ : proc

externdef ?INT_00660f8c@@3HA  :dword
externdef ?INT_00660f90@@3HA  :dword


;GO_d0
extern ??1GO_d0@@UAE@XZ  :proc   ; (public: virtual __thiscall GO_d0::~GO_d0(void))
extern ??0GO_d0@@QAE@XZ  :proc   ; (public: __thiscall GO_d0::GO_d0(void))
extern ??0GO_d0@@QAE@H@Z : proc ; (int)
extern ??0GO_d0@@QAE@PBV0@@Z  :proc   ; (public: __thiscall GO_d0::GO_d0(class GO_d0 const *))
extern ??0GO_d0@@QAE@PAPAEH@Z  :proc   ; (public: __thiscall GO_d0::GO_d0(unsigned char * *,int))
extern ??4GO_d0@@QAEXABV0@@Z  :proc   ; (public: void __thiscall 

externdef ?FUN_0041f0d0@GO_d0@@QAEHXZ   : proc
externdef ?FUN_004394f3@GO_d0@@QAE?AVCString@@XZ  :proc
externdef ?FUN_004396d6@GO_d0@@QAEHXZ  :proc



externdef _sub_5008CA :proc
externdef _sub_43A820 :proc                                     ; CString* sub_43A820(CString*, uint32_t) — itoa
externdef _dword_6CDB38 :dword                                  ; file checksum global
externdef _sub_4F499B :proc                                                     ; sub_4F499B — parse item from packet data


;VisSpellBook
externdef ?FUN_004caa69@VisSpellBook@@QAEXXZ : proc

;VisTav
externdef ?FUN_0049edec@VisTav@@QAEXXZ  : proc

;VisShop
externdef ?FUN_004bcd02@VisShop@@QAEXXZ  : proc

;Vis1200
externdef ?FUN_004972d0@Vis1200@@QAEHXZ  : proc
externdef ?FUN_00497310@Vis1200@@QAEHXZ  : proc
externdef  ?FUN_0043be9f@Vis1200@@QAEXXZ : proc

;VisGlobalMap
externdef ?FUN_0047024a@VisGlobalMap@@QAEXXZ : proc

;VisServerScreen
externdef ??0VisServerScreen@@QAE@HHHHHPAVGM_a28@@@Z : proc

;GameOptionsWindow
extern ??0GameOptionsWindow@@QAE@HHHHH@Z : proc

;MPGameMenu
extern ??0EndGameMenu@@QAE@HHHHHPAVCGameBitmap@@IABVCRect@@H@Z :proc

;WinGameMenu
extern ??0ExitGameMenu@@QAE@HHHHHABVCRect@@@Z :proc


;GFX things
externdef ?gfxFlushRect@@YAXABVCRect@@@Z :proc
externdef ?LockSurface2@@YAXXZ  :proc
externdef ?UnlockSurface2@@YAXXZ  :proc
externdef ?LockSurface1@@YAXXZ  :proc
externdef ?UnlockSurface1@@YAXXZ :proc
externdef ?InitVideo@@YAHXZ :proc
externdef ?SetVideoMode@@YAHXZ :proc
externdef ?GetLockCountSurf2@@YAHXZ : proc
externdef ?CopyScreenRectToBmp64@@YAXHHHHHHPAX@Z : proc

externdef ?gfx_ball@@3PAVCBmp64@@A  : dword
externdef ?gfx_interface_lm@@3PAVCSprite256@@A : dword
externdef ?gfx_scrollbars@@3PAVCSprite256@@A : dword
externdef ?gfx_radiob@@3PAVCSprite256@@A : dword
externdef ?g_font1@@3PAVCGameFont@@A : dword
externdef ?g_font2@@3PAVCGameFont@@A : dword


; Free functions

externdef ?sub_4954EA@@YAXXZ :proc
externdef _sub_5049D1 :proc
externdef _sub_536630 :proc   ; start attack?
externdef _sub_53678F :proc   ; execute attack?
externdef _Random0N :proc     ; 542216
externdef _Random1N :proc     ; 54223F
externdef _sub_530726 :proc   ; ExperienceToSkillLevel
externdef _sub_5306EA :proc   ; SkillLevelFromExperience
externdef _sub_530DCB :proc   ; ClampExperienceGain
externdef _sub_5499A6 :proc   ; CreateRandomItem

externdef ?sub_5365AB@@YAIPAUTokenPos@@0@Z :proc

externdef _WritePlayerFile_4F53EA :proc                  ; sub_4F53EA — save character data to file
externdef _ParsePlayerFile_4F62E6 :proc                  ; sub_4F62E6 — parse character file sections
externdef _CheckSum_4F5308 :proc                         ; sub_4F5308 — checksum calculation
externdef _LoadDecrypt_4F535E :proc                      ; sub_4F535E — XOR decrypt/encrypt


;gfx
externdef ?g_clipRect@@3VCRect@@A : RECT
externdef ?g_selDrawBitmap@@3U_DDSURFACEDESC@@A : dword
externdef ?g_ScreenSize@@3VCRect@@A : dword

extern ?FUN_004559d4@@YAXHHHHPAEPAG@Z : proc

extern ?FillRectColor@@YAXHHHHI@Z : proc
extern ?FillRectColorSimple@@YAXHHHHI@Z : proc
extern ?SetPixelColor@@YAXHHI@Z : proc
extern ?ShadowRect@@YAXVCRect@@H@Z : proc

extern ?DrawRectangleFrame@@YAXHHHHI@Z : proc

extern ?g_brightnessLookup@@3PAGA  :dword
extern ?g_brightnessLookupCount@@3IA  :dword

extern ?g_Shadows@@3HA : dword
extern ?g_Animation@@3HA : dword
extern ?g_Lightning@@3HA : dword

extern ?g_RBits@@3HA  :byte
extern ?g_GBits@@3HA  :byte
extern ?g_BBits@@3HA  :byte
extern ?g_RBitShift@@3HA  :byte
extern ?g_GBitShift@@3HA  :byte
extern ?g_BBitShift@@3HA  :byte
extern ?g_RBitMask@@3HA  :word
extern ?g_GBitMask@@3HA  :word
extern ?g_BBitMask@@3HA  :word


extern ?p_clrsh_Black@@3PAGA  :dword
extern ?p_clrsh_ShockingBlack@@3PAGA  :dword
extern ?p_clrsh_Gold@@3PAGA  :dword
extern ?clrsh_DullGold@@3PAGA  :word
extern ?clrsh_TechBlack@@3PAGA  :word
extern ?clrsh_ShockingBlack@@3PAGA  :word
extern ?clrsh_CharlieBrown@@3PAGA  :word
extern ?clrsh_LuxorGold@@3PAGA : word

extern ?clr_log_tblack@@3PAGA  : dword
extern ?clr_log_sblack@@3PAGA  : dword
extern ?clr_log_tok0@@3PAGA  : dword
extern ?clr_log_tok1@@3PAGA  : dword
extern ?clr_log_tok2@@3PAGA  : dword
extern ?clr_log_tok3@@3PAGA  : dword

extern ?g_isLowMemory@@3IA : dword

extern ?g_colors_human_pals@@3PAY0BA@GA : word

extern ?g_pal_projectiles@@3PAVCGamePalette@@A   : dword
extern ?g_pal_projectile_@@3PAVCGamePalette@@A   : dword
extern ?g_spr_smoke@@3PAPAVCA16@@A : dword

extern ?g_DeltaCLR@@3UColorThing@@A : byte


;CBmp64
externdef ??0CBmp64@@QAE@II@Z : proc

;CGameBitmap
externdef ?ResetPalette@CGameBitmap@@QAEXIHH@Z  : proc

;CSprite256
externdef ??0CSprite256@@QAE@PBD@Z : proc

;CA16
externdef ??0CA16@@QAE@PBD@Z  :proc

;CGamePalette
extern ?Free@CGamePalette@@QAEXXZ  : proc
extern ?SetPalette@CGamePalette@@QAEXPAUtagRGBQUAD@@IHH@Z : proc
extern ??0CGamePalette@@QAE@XZ : proc
extern ??1CGamePalette@@UAE@XZ : proc
extern ?GetPalette@CGamePalette@@QAEPAGH@Z : proc


;CGameFont
extern ?GetStrWidth@CGameFont@@QAEHPBD@Z : proc
extern ?StringArrayForRect@CGameFont@@QAEAAVCStringArray@@ABVCRect@@PBD@Z  : proc
externdef ?DrawTextLines@CGameFont@@QAEXABVCRect@@HHABVCStringArray@@PAGH@Z : proc


;sound
externdef ?Play@CSound@@SAXAAU1@@Z :proc
externdef ?Play@SfxSample@@QAEXHHHEH@Z : proc

extern ?Release@SfxSample@@QAEXXZ : proc
extern ?FindPlayingChannel@SfxSample@@QAEPAVSoundChannel@@XZ : proc

extern ?Clear@SoundChannel@@QAEXXZ : proc
extern ?Stop@SoundChannel@@QAEXXZ : proc

extern ?g_dsound@@3PAUIDirectSound@@A  :dword
extern ?g_dsound_channel_num@@3HA  :dword
extern ?g_dsound_channels@@3PAVSoundChannel@@A  :dword
extern ?g_SfxArray@@3V?$CArray@PAVSfxSample@@ABQAV1@@@A  :dword
extern ?g_SoundSettings@@3USoundSettings@@A : dword


externdef ?StartPlayTrack@MusicPlayer@@QAEXH@Z : proc


; util.h

extern _ReportWarning : proc
extern _XorData : proc


extern ?g_isDosCP@@3HA : dword
extern ?DecodeChar@@YAEE@Z  :proc
extern ?EncodeChar@@YAEE@Z  :proc
extern ?ToLowerChar@@YAEE@Z  :proc



; CMousePointer
extern ?g_mousept@@3VCMousePointer@@A : byte 

extern ?SetCursor@CMousePointer@@QAEXPAVCSprite256@@HHHH@Z  :proc
extern ?Unpaint@CMousePointer@@QAEXXZ  :proc
extern ?Paint@CMousePointer@@QAEXXZ  :proc
extern ?ResetNext400msg@CMousePointer@@QAEXXZ  :proc
extern ?Update@CMousePointer@@QAEXXZ  :proc
extern ?SaveScreenParts2@CMousePointer@@QAEXABVCRect@@@Z  :proc
extern ?SaveScreenParts@CMousePointer@@QAEXABVCRect@@@Z  :proc
extern ?StartSelect@CMousePointer@@QAEXABVCPoint@@@Z  :proc
extern ?ResetStates@CMousePointer@@QAEXXZ  : proc
extern ?ForceDisableHint@CMousePointer@@QAEXXZ : proc
extern ?GetSelectFrame@CMousePointer@@QAEAAVCRect@@XZ : proc
extern ?GetSelectState@CMousePointer@@QAEHXZ : proc
extern ?GetY@CMousePointer@@QAEHXZ : proc
extern ?GetX@CMousePointer@@QAEHXZ : proc
extern ?GetCursorSprite@CMousePointer@@QAEPAVCSprite256@@XZ : proc
extern ?DisableHint@CMousePointer@@QAEXXZ : proc
extern ?EnableHint@CMousePointer@@QAEXXZ : proc

extern ?LoadCursors@@YAXXZ  : proc
extern ?DeleteCursors@@YAXXZ : proc
extern ?ApplyCursor@@YAHPAVCCursor@@@Z : proc
extern ?g_Cursors@@3PAPAVCCursor@@A : dword


extern ??0CCursor@@QAE@PBDHHH@Z :proc
extern ?Use@CCursor@@QAEXXZ : proc
extern ?GetSprite@CCursor@@QBEPAVCSprite256@@XZ : proc



;CGameObject
extern ?FUN_0041f110@CGameObject@@QAEHXZ : proc


extern ?classCGameObject@CGameObject@@2UCRuntimeClass@@B  :dword
extern ??1CGameObject@@UAE@XZ  :proc

extern ?AssertValid@CGameObject@@UBEXXZ : proc

extern ?VMethod1@CGameObject@@UAEXH@Z  :proc
extern ?VMethod2@CGameObject@@UAEHXZ  :proc
extern ?VMethod3@CGameObject@@UAEHXZ  :proc
extern ?VMethod4@CGameObject@@UAEHXZ  :proc
extern ?VMethod5@CGameObject@@UAEHXZ  :proc
;extern ?VMethod6@CGameObject@@UAEXHHH@Z  :proc
extern ?VMethod7@CGameObject@@UAEXHHH@Z  :proc
extern ?VMethod8@CGameObject@@UAEXHHH@Z  :proc
extern ?VMethod9@CGameObject@@UAEXHHH@Z  :proc
extern ?VMethod10@CGameObject@@UAEXXZ  :proc
extern ?VMethod11@CGameObject@@UAEHXZ  :proc
extern ?VMethod12@CGameObject@@UAEXXZ  :proc
extern ?VMethod13@CGameObject@@UAEXXZ  :proc
extern ?VMethod14@CGameObject@@UAEXXZ  :proc
extern ?VMethod15@CGameObject@@UAEHXZ  :proc
extern ?VMethod16@CGameObject@@UAEXXZ  :proc
extern ??0CGameObject@@QAE@XZ  :proc
extern ??0CGameObject@@QAE@PBV0@@Z  :proc
extern ?FUN_0041f180@CGameObject@@QAEXH@Z  :proc

extern ?SetVals@CGameObject@@QAEXGHHHHHHHH@Z : proc

externdef ?FUN_0046190d@CGameObject@@QAEXXZ : proc
externdef ?FUN_00462405@CGameObject@@QAEHH@Z : proc

;CUnit
externdef ?g_CUnitStatic@@3VCUnit@@A : byte
externdef ??0CUnit@@QAE@XZ : proc
externdef ?FUN_0046b0d7@CUnit@@QAEXABVUnit@@@Z  :proc
externdef ?FUN_0046b7d2@CUnit@@QAEXH@Z :proc
externdef ?FUN_0046b91c@CUnit@@QAEXXZ : proc

;CAirUnit
externdef ??0CAirUnit@@QAE@XZ : proc

;CBackPack
externdef ??0CBackPack@@QAE@XZ : proc

;CProjectile
externdef ??0CProjectile@@QAE@XZ : proc


;MapPlayerData
externdef ??0MapPlayerData@@QAE@XZ :proc  ; MapPlayerData::MapPlayerData()
externdef ??0MapPlayerData@@QAE@HH@Z : proc
externdef  ?FUN_0041ee20@MapPlayerData@@QAEIH@Z  : proc
externdef ?FUN_0041edf0@MapPlayerData@@QAEHH@Z : proc

;UnitVFXInfo
externdef ?g_VFX_info@@3V?$CArray@PAVUnitVFXUnfo@@ABQAV1@@@A : byte

;GfxObject
extern ?Init@GfxFile@@QAEXXZ : proc
extern ?g_GfxFiles@@3V?$CArray@PAVGfxFile@@ABQAV1@@@A : dword
extern ?g_GfxObjects@@3V?$CArray@PAVGfxObject@@ABQAV1@@@A  : dword

;UnitGfxObject
extern ?g_UnitGfxFiles@@3V?$CArray@PAVUnitGfxFile@@ABQAV1@@@A : dword
extern ?Init@UnitGfxFile@@QAEXXZ : proc
extern ??0UnitGfxFile@@QAE@PBD@Z : proc

;StructureInfo
extern ?Init@StructureInfo@@QAEXXZ : proc
extern ?g_StructuresInfo@@3V?$CArray@PAVStructureInfo@@ABQAV1@@@A  : dword

;ProjectileInfo
extern  ?g_ProjectileInfos@@3V?$CArray@PAVProjectileInfo@@ABQAV1@@@A  : byte
extern ?Init@ProjectileInfo@@QAEXXZ : proc
