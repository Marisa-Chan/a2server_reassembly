
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




public ?Write@CArchive@@QAEXPBXI@Z ; CArchive::Write  005E6A26
public ?Read@CArchive@@QAEIPAXI@Z ; CArchive::Read  005E6918 
public ?WriteCount@CArchive@@QAEXK@Z  ; CArchive::WriteCount 005E6BF2
public ?ReadCount@CArchive@@QAEKXZ   ; CArchive::ReadCount 005E6C20



public ?AfxGetModuleThreadState@@YGPAVAFX_MODULE_THREAD_STATE@@XZ  ; AfxGetModuleThreadState(); 005F4DB8 
public ??0AFX_CLASSINIT@@QAE@PAUCRuntimeClass@@@Z  ; AFX_CLASSINIT::AFX_CLASSINIT(CRuntimeClass *)
public ??0AFX_EXCEPTION_LINK@@QAE@XZ  ;AFX_EXCEPTION_LINK::AFX_EXCEPTION_LINK
public ?AfxTryCleanup@@YGXXZ ; AfxTryCleanup




;CPlex
extern ?Create@CPlex@@SGPAU1@AAPAU1@II@Z:proc   ; CPlex::Create 005dbbb6
extern ?FreeDataChain@CPlex@@QAEXXZ:proc ; CPlex::FreeDataChain  005DBBD6

;C++
extern ??2@YAPAXI@Z:proc ; operator new         5ddf54
extern ??3@YAXPAX@Z:proc ; operator delete      5ddf90


;TokenPos 
extern ??0TokenPos@@QAE@EEPAUastruct_5@@@Z:proc  ;  TokenPos::TokenPos(uint8_t _x, uint8_t _y, astruct_5* _inst)
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




extern ?astruct_5_Instance@@3PAUastruct_5@@A:dword   ;  astruct_5_Instance

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
