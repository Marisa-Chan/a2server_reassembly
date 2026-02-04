
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



;CArchive
public ?Write@CArchive@@QAEXPBXI@Z ; CArchive::Write  005E6A26
public ?Read@CArchive@@QAEIPAXI@Z ; CArchive::Read  005E6918 
public ?WriteCount@CArchive@@QAEXK@Z  ; CArchive::WriteCount 005E6BF2
public ?ReadCount@CArchive@@QAEKXZ   ; CArchive::ReadCount 005E6C20
public ?ReadObject@CArchive@@QAEPAVCObject@@PBUCRuntimeClass@@@Z     ;  CArchive::ReadObject  005e848c



public ?AfxGetModuleThreadState@@YGPAVAFX_MODULE_THREAD_STATE@@XZ  ; AfxGetModuleThreadState(); 005F4DB8 
public ??0AFX_CLASSINIT@@QAE@PAUCRuntimeClass@@@Z  ; AFX_CLASSINIT::AFX_CLASSINIT(CRuntimeClass *)
public ??0AFX_EXCEPTION_LINK@@QAE@XZ  ;AFX_EXCEPTION_LINK::AFX_EXCEPTION_LINK
public ?AfxTryCleanup@@YGXXZ ; AfxTryCleanup


;CString
public ?AfxLoadString@@YGHIPADI@Z   ; AfxLoadString
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



;Unit
extern ??1Unit@@UAE@XZ:proc   ; (public: virtual __thiscall Unit::~Unit(void))
externdef ?Serialize@Unit@@UAEXAAVCArchive@@@Z:proc   ; (public: virtual void __thiscall Unit::Serialize(class CArchive &))
externdef ?VMethod1@Unit@@UAEXXZ:proc   ; (public: virtual void __thiscall Unit::VMethod1(void))
externdef ?VMethod2@Unit@@UAEXXZ:proc   ; (public: virtual void __thiscall Unit::VMethod2(void))
extern ?VMethod3@Unit@@UAEHXZ:proc   ; (public: virtual int __thiscall Unit::VMethod3(void))
extern ?VMethod4@Unit@@UAEHXZ:proc   ; (public: virtual int __thiscall Unit::VMethod4(void))
externdef ?VMethod5@Unit@@UAEXXZ:proc   ; (public: virtual void __thiscall Unit::VMethod5(void))

externdef ?VMethod10@Unit@@UAEXXZ:proc   ; (public: virtual void __thiscall Unit::VMethod10(void))
externdef ?VMethod11@Unit@@UAEXPAV1@@Z:proc   ; (public: virtual void __thiscall Unit::VMethod11(class Unit *))

externdef ?VMethod17@Unit@@UAEIPAUUnitToHit@@PAV1@@Z:proc   ; (public: virtual unsigned int __thiscall Unit::VMethod17(struct UnitToHit *,class Unit *))

externdef ?VMethod25@Unit@@UAEHXZ:proc   ; (public: virtual int __thiscall Unit::VMethod25(void))

extern ??0Unit@@QAE@XZ:proc    ; (public: __thiscall Unit::Unit(void))
extern ??0Unit@@QAE@ABVCString@@@Z:proc   ; (public: __thiscall Unit::Unit(class CString const &))
extern ??0Unit@@QAE@PBUTokenPos@@@Z:proc   ; (public: __thiscall Unit::Unit(struct TokenPos const *))
extern ??5@YGAAVCArchive@@AAV0@AAPAVUnit@@@Z:proc     ;  CArchive & __stdcall operator>>(class CArchive &,class Unit * &)

externdef ?FUN_0052931b@Unit@@QAEXABVCString@@@Z:proc   ; FUN_0052931b



;Humanoid
extern ?classHumanoid@Humanoid@@2UCRuntimeClass@@A:dword    ;  classHumanoid

extern ??1Humanoid@@UAE@XZ:proc   ; (public: virtual __thiscall Humanoid::~Humanoid(void))
externdef ?Serialize@Humanoid@@UAEXAAVCArchive@@@Z:proc   ; (public: virtual void __thiscall Humanoid::Serialize(class CArchive &))
extern ?VMethod5@Humanoid@@UAEXXZ:proc   ; (public: virtual void __thiscall Humanoid::VMethod5(void))
extern ?VMethod7@Humanoid@@UAEHXZ:proc   ; (public: virtual int __thiscall Humanoid::VMethod7(void))
extern ?VMethod8@Humanoid@@UAEHXZ:proc   ; (public: virtual int __thiscall Humanoid::VMethod8(void))
extern ?VMethod10@Humanoid@@UAEXXZ:proc   ; (public: virtual void __thiscall Humanoid::VMethod10(void))
externdef ?VMethod12@Humanoid@@UAEPAVItem@@PAV2@@Z:proc   ; (public: virtual void __thiscall Humanoid::VMethod12(class Item *))
extern ?VMethod13@Humanoid@@UAEXPAVItem@@@Z:proc   ; (public: virtual void __thiscall Humanoid::VMethod13(class Item *))
extern ?Unequip@Humanoid@@UAEPAVItem@@PAV2@@Z:proc   ; (public: virtual void __thiscall Humanoid::Unequip(class Item *))
extern ?VMethod15@Humanoid@@UAEXXZ:proc   ; (public: virtual void __thiscall Humanoid::VMethod15(void))
externdef ?VMethod16@Humanoid@@UAEXPAVItem@@@Z:proc   ; (public: virtual void __thiscall Humanoid::VMethod16(class Item *))
externdef ?VMethod18@Humanoid@@UAEXXZ:proc   ; (public: virtual void __thiscall Humanoid::VMethod18(void))
extern ?VMethod19@Humanoid@@UAEIXZ:proc   ; (public: virtual unsigned int __thiscall Humanoid::VMethod19(void))
externdef ?VMethod21@Humanoid@@UAEXIPAVUnit@@H@Z:proc   ; (public: virtual void __thiscall Humanoid::VMethod21(unsigned int,class Unit *,int))
externdef ?VMethod22@Humanoid@@UAEXPAVUnit@@H@Z:proc   ; (public: virtual void __thiscall Humanoid::VMethod22(class Unit *,int))
externdef ?VMethod23@Humanoid@@UAEXPAVUnit@@IHH@Z:proc   ; (public: virtual void __thiscall Humanoid::VMethod23(class Unit *,unsigned int,int,int))
externdef ?VMethod24@Humanoid@@UAEXPAVUnit@@IHH@Z:proc   ; (public: virtual void __thiscall Humanoid::VMethod24(class Unit *,unsigned int,int,int))
externdef ?VMethod25@Humanoid@@UAEHXZ:proc   ; (public: virtual int __thiscall Humanoid::VMethod25(void))

extern ??0Humanoid@@QAE@XZ:proc   ; (public: __thiscall Humanoid::Humanoid(void))
extern ??0Humanoid@@QAE@PBUTokenPos@@@Z:proc  ;  (public: __thiscall Humanoid::Humanoid(struct TokenPos const *))
externdef ?InitStats@Humanoid@@QAEXXZ:proc   ;    Humanoid::InitStats




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