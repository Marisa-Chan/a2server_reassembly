; ---------------------------------------------------------------------------


IShellLinkAVtbl struc ; (sizeof=0x54, align=0x4, copyof_340)
QueryInterface  dd ?                    ; offset
AddRef          dd ?                    ; offset
Release         dd ?                    ; offset
_GetPath         dd ?                    ; offset
GetIDList       dd ?                    ; offset
SetIDList       dd ?                    ; offset
GetDescription  dd ?                    ; offset
SetDescription  dd ?                    ; offset
GetWorkingDirectory dd ?                ; offset
SetWorkingDirectory dd ?                ; offset
GetArguments    dd ?                    ; offset
SetArguments    dd ?                    ; offset
GetHotkey       dd ?                    ; offset
SetHotkey       dd ?                    ; offset
GetShowCmd      dd ?                    ; offset
SetShowCmd      dd ?                    ; offset
GetIconLocation dd ?                    ; offset
SetIconLocation dd ?                    ; offset
SetRelativePath dd ?                    ; offset
Resolve         dd ?                    ; offset
SetPath         dd ?                    ; offset
IShellLinkAVtbl ends

; ---------------------------------------------------------------------------

IClassFactoryVtbl struc ; (sizeof=0x14, align=0x4, copyof_335)
QueryInterface  dd ?                    ; offset
AddRef          dd ?                    ; offset
Release         dd ?                    ; offset
CreateInstance  dd ?                    ; offset
LockServer      dd ?                    ; offset
IClassFactoryVtbl ends

; ---------------------------------------------------------------------------

IDispatchVtbl   struc ; (sizeof=0x1C, align=0x4, copyof_246)
QueryInterface  dd ?                    ; offset
AddRef          dd ?                    ; offset
Release         dd ?                    ; offset
GetTypeInfoCount dd ?                   ; offset
GetTypeInfo     dd ?                    ; offset
GetIDsOfNames   dd ?                    ; offset
InvokeMethod          dd ?                    ; offset
IDispatchVtbl   ends

; ---------------------------------------------------------------------------

_SCOPETABLE_ENTRY struc ; (sizeof=0xC, align=0x4, copyof_3)
                                        ; XREF: .rdata:stru_60C0F0/r
                                        ; .rdata:stru_6143F8/r ...
EnclosingLevel  dd ?
FilterFunc      dd ?                    ; offset
HandlerFunc     dd ?                    ; offset
_SCOPETABLE_ENTRY ends

; ---------------------------------------------------------------------------

_EH3_EXCEPTION_REGISTRATION struc ; (sizeof=0x10, align=0x4, copyof_5)
                                        ; XREF: CPPEH_RECORD/r
Next            dd ?                    ; XREF: sub_48A756:loc_48AC9B/r
                                        ; `eh vector destructor iterator'(void *,uint,int,void (*)(void *)):loc_5C0100/r ... ; offset
ExceptionHandler dd ?                   ; offset
ScopeTable      dd ?                    ; offset
TryLevel        dd ?                    ; XREF: sub_48A756+29/w
                                        ; sub_48A756+295/w ...
_EH3_EXCEPTION_REGISTRATION ends


CPPEH_RECORD    struc ; (sizeof=0x18, align=0x4, copyof_8)
                                        ; XREF: ??_M@YGXPAXIHP6EX0@Z@Z/r
                                        ; ?__ArrayUnwind@@YGXPAXIHP6EX0@Z@Z/r ...
old_esp         dd ?                    ; XREF: sub_48A756+23/w
                                        ; sub_48A756:loc_48AC91/r ...
exc_ptr         dd ?                    ; XREF: __ArrayUnwind(void *,uint,int,void (*)(void *)):loc_5C0190/r
                                        ; start:loc_5C1CFD/r ... ; offset
registration    _EH3_EXCEPTION_REGISTRATION <> ; XREF: sub_48A756+29/w
                                        ; sub_48A756+295/w ...
CPPEH_RECORD    ends

; ---------------------------------------------------------------------------

POINT           struc ; (sizeof=0x8, align=0x4, copyof_18)
                                        ; XREF: sub_40B314/r
                                        ; sub_40B314/r ...
x               dd ?                    ; XREF: sub_40B314+99/w
                                        ; sub_40B314+A6/r ...
y               dd ?                    ; XREF: sub_40B314+9F/w
                                        ; sub_40B314+A2/r ...
POINT           ends

; ---------------------------------------------------------------------------

tagMSG          struc ; (sizeof=0x1C, align=0x4, copyof_9)
                                        ; XREF: sub_40D7F3/r
                                        ; sub_447105/r ...
hwnd            dd ?                    ; offset
message         dd ?                    ; XREF: sub_40D7F3+18E/r
                                        ; sub_447105+1C/r ...
wParam          dd ?                    ; XREF: CDockContext::Track(void)+B3/r
                                        ; CDockContext::Track(void)+C9/r ...
lParam          dd ?
time            dd ?
pt              POINT <>                 ; XREF: CDockContext::Track(void)+89/r
                                        ; CDockContext::Track(void)+90/r
tagMSG          ends

; ---------------------------------------------------------------------------



; ---------------------------------------------------------------------------

tagPOINT        struc ; (sizeof=0x8, align=0x4) ; XREF: sub_4271E6/r
                                        ; sub_4D8F7E/r ...
x               dd ?                    ; XREF: sub_4271E6+63/r
                                        ; sub_4271E6+82/r ...
y               dd ?                    ; XREF: sub_4271E6+70/r
                                        ; sub_4271E6+7E/r ...
tagPOINT        ends

; ---------------------------------------------------------------------------

tagRECT         struc ; (sizeof=0x10, align=0x4, copyof_21)
                                        ; XREF: .data:stru_70D010/r
                                        ; sub_438520/r ...
left            dd ?                    ; XREF: sub_438520+E/w
                                        ; sub_438520+38/r ...
top             dd ?                    ; XREF: sub_438520+14/w
                                        ; sub_438520+3D/r ...
right           dd ?                    ; XREF: sub_438520+1A/w
                                        ; sub_438520+43/r ...
bottom          dd ?                    ; XREF: sub_438520+20/w
                                        ; sub_438520+49/r ...
tagRECT         ends

; ---------------------------------------------------------------------------

FILETIME        struc ; (sizeof=0x8, align=0x4, copyof_22)
                                        ; XREF: CompareFunction/r
                                        ; CompareFunction/r ...
dwLowDateTime   dd ?
dwHighDateTime  dd ?
FILETIME        ends

; ---------------------------------------------------------------------------

_WIN32_FIND_DATAA struc ; (sizeof=0x140, align=0x4, copyof_24)
                                        ; XREF: sub_43EAA0/r
                                        ; sub_440D96/r ...
dwFileAttributes dd ?                   ; XREF: sub_43EAA0:loc_43EAFA/r
                                        ; sub_44AAE5:loc_44AB58/r ...
ftCreationTime  FILETIME <>              ; XREF: CFile::GetStatus(char const *,CFileStatus &)+5C/o
ftLastAccessTime FILETIME <>             ; XREF: CFile::GetStatus(char const *,CFileStatus &)+6D/o
ftLastWriteTime FILETIME <>              ; XREF: CFile::GetStatus(char const *,CFileStatus &)+82/o
nFileSizeHigh   dd ?
nFileSizeLow    dd ?                    ; XREF: CFile::GetStatus(char const *,CFileStatus &)+53/r
dwReserved0     dd ?
dwReserved1     dd ?
cFileName       db 260 dup(?)           ; XREF: sub_43EAA0+74/o
                                        ; sub_43EAA0+8C/o ...
cAlternateFileName db 14 dup(?)
                db ? ; undefined
                db ? ; undefined
_WIN32_FIND_DATAA ends

; ---------------------------------------------------------------------------

_MEMORYSTATUS   struc ; (sizeof=0x20, align=0x4, copyof_26)
                                        ; XREF: sub_4538EB/r
dwLength        dd ?
dwMemoryLoad    dd ?
dwTotalPhys     dd ?                    ; XREF: sub_4538EB+11/r
dwAvailPhys     dd ?
dwTotalPageFile dd ?
dwAvailPageFile dd ?
dwTotalVirtual  dd ?
dwAvailVirtual  dd ?
_MEMORYSTATUS   ends

; ---------------------------------------------------------------------------

tWAVEFORMATEX   struc ; (sizeof=0x12, copyof_29) ; XREF: .data:pwfxDst/r
wFormatTag      dw ?                    ; XREF: sub_45C489+1B/w
nChannels       dw ?                    ; XREF: sub_45C489+24/w
nSamplesPerSec  dd ?                    ; XREF: sub_45C489+2D/w
                                        ; sub_45C489+48/r
nAvgBytesPerSec dd ?                    ; XREF: sub_45C489+4F/w
nBlockAlign     dw ?                    ; XREF: sub_45C489+37/w
                                        ; sub_45C489+42/r
wBitsPerSample  dw ?                    ; XREF: sub_45C489+54/w
cbSize          dw ?                    ; XREF: sub_45C489+5D/w
tWAVEFORMATEX   ends

; ---------------------------------------------------------------------------

tACMSTREAMHEADER struc ; (sizeof=0x54, copyof_31) ; XREF: .data:pash/r
cbStruct        dd ?                    ; XREF: sub_45BA94+30B/w
fdwStatus       dd ?
dwUser          dd ?
pbSrc           dd ?                    ; XREF: sub_45BA94+322/w
                                        ; sub_45BA94+399/r ... ; offset
cbSrcLength     dd ?                    ; XREF: sub_45BA94+327/w
                                        ; sub_45BA94+338/r ...
cbSrcLengthUsed dd ?
dwSrcUser       dd ?
pbDst           dd ?                    ; XREF: sub_45BA94+359/w
                                        ; sub_45BA94+3CE/r ... ; offset
cbDstLength     dd ?                    ; XREF: sub_45BA94+333/o
                                        ; sub_45BA94+34A/r
cbDstLengthUsed dd ?                    ; XREF: sub_45BA94+3C7/r
                                        ; sub_45BA94+3E3/r ...
dwDstUser       dd ?
dwReservedDriver dd 10 dup(?)
tACMSTREAMHEADER ends

; ---------------------------------------------------------------------------

MSG             struc ; (sizeof=0x1C, align=0x4, copyof_35)
                                        ; XREF: sub_49057B/r
hwnd            dd ?                    ; offset
message         dd ?                    ; XREF: sub_49057B+68A/r
wParam          dd ?
lParam          dd ?
time            dd ?
pt              POINT <>
MSG             ends

; ---------------------------------------------------------------------------

IID             struc ; (sizeof=0x10, align=0x4, copyof_36)
                                        ; XREF: .rdata:stru_60BA00/r
                                        ; .rdata:stru_60BA20/r ...
Data1           dd ?
Data2           dw ?
Data3           dw ?
Data4           db 8 dup(?)
IID             ends

; ---------------------------------------------------------------------------

_OSVERSIONINFOA struc ; (sizeof=0x94, align=0x4, copyof_39)
                                        ; XREF: sub_5214C5/r
                                        ; ?_AfxGetMouseScrollLines@@YAIH@Z/r
dwOSVersionInfoSize dd ?                ; XREF: sub_5214C5+37/w
                                        ; _AfxGetMouseScrollLines(int)+A6/w
dwMajorVersion  dd ?                    ; XREF: _AfxGetMouseScrollLines(int):loc_5E8D82/r
                                        ; _AfxGetMouseScrollLines(int)+157/r
dwMinorVersion  dd ?
dwBuildNumber   dd ?
dwPlatformId    dd ?                    ; XREF: sub_5214C5+50/r
                                        ; _AfxGetMouseScrollLines(int)+C9/r ...
szCSDVersion    db 128 dup(?)
_OSVERSIONINFOA ends

; ---------------------------------------------------------------------------

fd_set          struc ; (sizeof=0x104, align=0x4, copyof_40)
                                        ; XREF: sub_522E8F/r
fd_count        dd ?                    ; XREF: sub_522E8F+40/w
                                        ; sub_522E8F:loc_522ED9/r ...
fd_array        dd 64 dup(?)            ; XREF: sub_522E8F+5F/w
                                        ; sub_522E8F+E4/w
fd_set          ends

; ---------------------------------------------------------------------------

timeval         struc ; (sizeof=0x8, align=0x4, copyof_43)
                                        ; XREF: sub_522E8F/r
tv_sec          dd ?                    ; XREF: sub_522E8F+32/w
tv_usec         dd ?                    ; XREF: sub_522E8F+39/w
timeval         ends

; ---------------------------------------------------------------------------

sockaddr        struc ; (sizeof=0x10, align=0x2, copyof_44)
                                        ; XREF: sub_522E8F/r
                                        ; sub_523621/r ...
sa_family       dw ?
sa_data         db 14 dup(?)            ; XREF: sub_522E8F+34D/r
                                        ; sub_522E8F+360/r ...
sockaddr        ends

; ---------------------------------------------------------------------------

WSAData         struc ; (sizeof=0x190, align=0x4, copyof_47)
                                        ; XREF: sub_52359B/r
wVersion        dw ?                    ; XREF: sub_52359B:loc_5235D1/r
                                        ; sub_52359B+47/r
wHighVersion    dw ?
szDescription   db 257 dup(?)
szSystemStatus  db 129 dup(?)
iMaxSockets     dw ?
iMaxUdpDg       dw ?
                db ? ; undefined
                db ? ; undefined
lpVendorInfo    dd ?                    ; offset
WSAData         ends

; ---------------------------------------------------------------------------

_STARTUPINFOA   struc ; (sizeof=0x44, align=0x4, copyof_48)
                                        ; XREF: start/r __ioinit/r ...
cb              dd ?                    ; XREF: __dospawn+62/w
lpReserved      dd ?                    ; offset
lpDesktop       dd ?                    ; offset
lpTitle         dd ?                    ; offset
dwX             dd ?
dwY             dd ?
dwXSize         dd ?
dwYSize         dd ?
dwXCountChars   dd ?
dwYCountChars   dd ?
dwFillAttribute dd ?
dwFlags         dd ?                    ; XREF: start:loc_5C1CBA/w
                                        ; start+12B/r
wShowWindow     dw ?                    ; XREF: start+131/r
cbReserved2     dw ?                    ; XREF: __ioinit+70/r
                                        ; __dospawn+96/w ...
lpReserved2     dd ?                    ; XREF: __ioinit+7B/r
                                        ; __dospawn+AB/w ... ; offset
hStdInput       dd ?                    ; offset
hStdOutput      dd ?                    ; offset
hStdError       dd ?                    ; offset
_STARTUPINFOA   ends

; ---------------------------------------------------------------------------

_SYSTEMTIME     struc ; (sizeof=0x10, align=0x2, copyof_51)
                                        ; XREF: _time/r _time/r ...
wYear           dw ?                    ; XREF: _time+54/r
                                        ; _time:loc_5C29C2/r ...
wMonth          dw ?                    ; XREF: _time+46/r _time+F3/r
wDayOfWeek      dw ?
wDay            dw ?                    ; XREF: _time+38/r
wHour           dw ?                    ; XREF: _time+2A/r _time+B0/r ...
wMinute         dw ?                    ; XREF: _time+1C/r _time+D8/r
wSecond         dw ?                    ; XREF: _time+BA/r
                                        ; _time:loc_5C29EF/r
wMilliseconds   dw ?
_SYSTEMTIME     ends

; ---------------------------------------------------------------------------


SYSTEMTIME      struc ; (sizeof=0x10, align=0x2, copyof_55)
                                        ; XREF: ?AfxTimeToFileTime@@YAXABVCTime@@PAU_FILETIME@@@Z/r
                                        ; _TIME_ZONE_INFORMATION/r ...
wYear           dw ?                    ; XREF: sub_5CF160+3B/r
                                        ; sub_5CF160+EE/r ...
wMonth          dw ?                    ; XREF: _time+7E/r
                                        ; __tzset_lk+84/r ...
wDayOfWeek      dw ?                    ; XREF: sub_5CF160+6C/r
                                        ; sub_5CF160+11C/r
wDay            dw ?                    ; XREF: sub_5CF160+86/r
                                        ; sub_5CF160+CA/r ...
wHour           dw ?                    ; XREF: sub_5CF160+65/r
                                        ; sub_5CF160+BB/r ...
wMinute         dw ?                    ; XREF: sub_5CF160+5D/r
                                        ; sub_5CF160+B3/r ...
wSecond         dw ?                    ; XREF: sub_5CF160+45/r
                                        ; sub_5CF160:loc_5CF1FB/r ...
wMilliseconds   dw ?                    ; XREF: sub_5CF160+4D/r
                                        ; sub_5CF160+A3/r ...
SYSTEMTIME      ends

_TIME_ZONE_INFORMATION struc ; (sizeof=0xAC, align=0x4, copyof_52)
                                        ; XREF: .data:TimeZoneInformation/r
                                        ; _time/r
Bias            dd ?                    ; XREF: __tzset_lk+63/r
StandardName    dw 32 dup(?)            ; XREF: __tzset_lk+DF/o
StandardDate    SYSTEMTIME <>            ; XREF: __tzset_lk+84/r
                                        ; sub_5CF160+EE/r ...
StandardBias    dd ?                    ; XREF: __tzset_lk+68/r
DaylightName    dw 32 dup(?)            ; XREF: __tzset_lk+F4/o
DaylightDate    SYSTEMTIME <>            ; XREF: _time+7E/r
                                        ; __tzset_lk:loc_5CEEF0/r ...
DaylightBias    dd ?                    ; XREF: _time+89/r
                                        ; __tzset_lk+A9/r
_TIME_ZONE_INFORMATION ends

; ---------------------------------------------------------------------------


; ---------------------------------------------------------------------------

_cpinfo         struc ; (sizeof=0x14, align=0x4, copyof_56)
                                        ; XREF: __setmbcp/r
                                        ; ___crtCompareStringA/r ...
MaxCharSize     dd ?                    ; XREF: __setmbcp+89/r
                                        ; sub_5D2570+D0/r ...
DefaultChar     db 2 dup(?)
LeadByte        db 12 dup(?)            ; XREF: __setmbcp+90/r
                                        ; __setmbcp+98/o ...
                db ? ; undefined
                db ? ; undefined
_cpinfo         ends

; ---------------------------------------------------------------------------

_SECURITY_ATTRIBUTES struc ; (sizeof=0xC, align=0x4, copyof_57)
                                        ; XREF: __sopen/r
                                        ; ?Open@CFile@@UAEHPBDIPAVCFileException@@@Z/r
nLength         dd ?                    ; XREF: __sopen+11/w
                                        ; CFile::Open(char const *,uint,CFileException *)+A4/w
lpSecurityDescriptor dd ?               ; XREF: __sopen+19/w
                                        ; CFile::Open(char const *,uint,CFileException *)+B2/w ; offset
bInheritHandle  dd ?                    ; XREF: __sopen+1F/w
                                        ; __sopen:loc_5D12FA/w ...
_SECURITY_ATTRIBUTES ends

; ---------------------------------------------------------------------------

_PROCESS_INFORMATION struc ; (sizeof=0x10, align=0x4, copyof_60)
                                        ; XREF: __dospawn/r
hProcess        dd ?                    ; XREF: __dospawn+1D7/r
                                        ; __dospawn+1E4/r ... ; offset
hThread         dd ?                    ; XREF: __dospawn+1BD/r
                                        ; __dospawn+1FB/r ... ; offset
dwProcessId     dd ?
dwThreadId      dd ?
_PROCESS_INFORMATION ends

; ---------------------------------------------------------------------------

_FILETIME       struc ; (sizeof=0x8, align=0x4)
                                        ; XREF: ??0CTime@@QAE@ABU_FILETIME@@H@Z/r
                                        ; ?GetStatus@CFile@@QBEHAAUCFileStatus@@@Z/r ...
dwLowDateTime   dd ?
dwHighDateTime  dd ?                    ; XREF: CMirrorFile::Open(char const *,uint,CFileException *)+59/o
                                        ; CMirrorFile::Open(char const *,uint,CFileException *)+65/o ...
_FILETIME       ends

; ---------------------------------------------------------------------------

tagWNDCLASSA    struc ; (sizeof=0x28, align=0x4, copyof_61)
                                        ; XREF: ?AfxRegisterWndClass@@YGPBDIPAUHICON__@@PAUHBRUSH__@@0@Z/r
                                        ; ?GetIconWndClass@CFrameWnd@@IAEPBDKI@Z/r ...
style           dd ?                    ; XREF: AfxRegisterWndClass(uint,HICON__ *,HBRUSH__ *,HICON__ *)+71/w
                                        ; CFrameWnd::GetIconWndClass(ulong,uint)+71/r
lpfnWndProc     dd ?                    ; XREF: AfxRegisterWndClass(uint,HICON__ *,HBRUSH__ *,HICON__ *)+79/w ; offset
cbClsExtra      dd ?                    ; XREF: AfxRegisterWndClass(uint,HICON__ *,HBRUSH__ *,HICON__ *)+81/w
cbWndExtra      dd ?                    ; XREF: AfxRegisterWndClass(uint,HICON__ *,HBRUSH__ *,HICON__ *)+7E/w
hInstance       dd ?                    ; XREF: AfxRegisterWndClass(uint,HICON__ *,HBRUSH__ *,HICON__ *)+91/w ; offset
hIcon           dd ?                    ; XREF: AfxRegisterWndClass(uint,HICON__ *,HBRUSH__ *,HICON__ *)+87/w
                                        ; CFrameWnd::GetIconWndClass(ulong,uint)+65/r ; offset
hCursor         dd ?                    ; XREF: AfxRegisterWndClass(uint,HICON__ *,HBRUSH__ *,HICON__ *)+94/w
                                        ; CFrameWnd::GetIconWndClass(ulong,uint)+6E/r ; offset
hbrBackground   dd ?                    ; XREF: AfxRegisterWndClass(uint,HICON__ *,HBRUSH__ *,HICON__ *)+97/w
                                        ; CFrameWnd::GetIconWndClass(ulong,uint)+6B/r ; offset
lpszMenuName    dd ?                    ; XREF: AfxRegisterWndClass(uint,HICON__ *,HBRUSH__ *,HICON__ *)+84/w ; offset
lpszClassName   dd ?                    ; XREF: AfxRegisterWndClass(uint,HICON__ *,HBRUSH__ *,HICON__ *)+9A/w ; offset
tagWNDCLASSA    ends

; ---------------------------------------------------------------------------

tagSIZE         struc ; (sizeof=0x8, align=0x4, copyof_72)
                                        ; XREF: ?LPtoDP@CDC@@QBEXPAUtagSIZE@@@Z/r
                                        ; ?LPtoDP@CDC@@QBEXPAUtagSIZE@@@Z/r ...
_cx             dd ?                    ; XREF: CDialogTemplate::GetSizeInPixels(tagSIZE *)+79/r
                                        ; ConvertDialogUnitsToPixels(char const *,ushort,int,int,tagSIZE *)+9E/r ...
cy              dd ?                    ; XREF: CDialogTemplate::GetSizeInPixels(tagSIZE *)+76/r
                                        ; CDC::SetViewportExt(int,int)+42/r ...
tagSIZE         ends

; ---------------------------------------------------------------------------

struc_1         struc ; (sizeof=0x10, align=0x4, mappedto_353)
left            dd ?
top             dd ?
right           dd ?
bottom          dd ?
struc_1         ends

; ---------------------------------------------------------------------------

RECT            struc ; (sizeof=0x10, align=0x4, copyof_73)
                                        ; XREF: .data:rcSrc/r
                                        ; .data:rcSrc2/r ...
left            dd ?                    ; XREF: sub_406F7B+1CC/r
                                        ; sub_406F7B+1FD/r ...
top             dd ?                    ; XREF: sub_406F7B+1D1/r
                                        ; sub_406F7B+202/r ...
right           dd ?                    ; XREF: sub_406F7B+1D7/r
                                        ; sub_406F7B+208/r ...
bottom          dd ?                    ; XREF: sub_40212B+47/r
                                        ; sub_406F7B+1DD/r ...
RECT            ends

; ---------------------------------------------------------------------------

LOGFONTA        struc ; (sizeof=0x3C, align=0x4, copyof_74)
                                        ; XREF: ?CreatePointFontIndirect@CFont@@QAEHPBUtagLOGFONTA@@PAVCDC@@@Z/r
                                        ; ?Initialize@CMiniFrameWnd@@SAXXZ/r ...
lfHeight        dd ?                    ; XREF: ConvertDialogUnitsToPixels(char const *,ushort,int,int,tagSIZE *)+49/w
                                        ; CFont::CreatePointFontIndirect(tagLOGFONTA const *,CDC *)+36/r ...
lfWidth         dd ?                    ; XREF: CMiniFrameWnd::Initialize(void):loc_5F2D6F/o
                                        ; CMiniFrameWnd::Initialize(void)+121/r
lfEscapement    dd ?
lfOrientation   dd ?
lfWeight        dd ?                    ; XREF: ConvertDialogUnitsToPixels(char const *,ushort,int,int,tagSIZE *)+40/w
                                        ; CMiniFrameWnd::Initialize(void)+83/w ...
lfItalic        db ?
lfUnderline     db ?
lfStrikeOut     db ?
lfCharSet       db ?                    ; XREF: ConvertDialogUnitsToPixels(char const *,ushort,int,int,tagSIZE *)+50/w
                                        ; CMiniFrameWnd::Initialize(void)+7F/w
lfOutPrecision  db ?
lfClipPrecision db ?
lfQuality       db ?
lfPitchAndFamily db ?                   ; XREF: CMiniFrameWnd::Initialize(void)+CA/w
lfFaceName      db 32 dup(?)            ; XREF: ConvertDialogUnitsToPixels(char const *,ushort,int,int,tagSIZE *)+4C/o
                                        ; CMiniFrameWnd::Initialize(void):loc_5F2D19/o
LOGFONTA        ends

; ---------------------------------------------------------------------------

_RTL_CRITICAL_SECTION struc ; (sizeof=0x18, align=0x4, copyof_76)
                                        ; XREF: .data:stru_70D040/r
                                        ; .data:CriticalSection/r ...
DebugInfo       dd ?                    ; offset
LockCount       dd ?
RecursionCount  dd ?
OwningThread    dd ?                    ; offset
LockSemaphore   dd ?                    ; offset
SpinCount       dd ?
_RTL_CRITICAL_SECTION ends

; ---------------------------------------------------------------------------

_ThrowInfo      struc ; (sizeof=0x10, align=0x4, copyof_81)
                                        ; XREF: .rdata:__TI1?AVCRegException@@/r
                                        ; .rdata:__TI5PAVCMemoryException@@/r ...
attributes      dd ?
pmfnUnwind      dd ?                    ; offset
pForwardCompat  dd ?                    ; offset
pCatchableTypeArray dd ?                ; offset
_ThrowInfo      ends

; ---------------------------------------------------------------------------

_complex        struc ; (sizeof=0x10, align=0x8, copyof_91)
                                        ; XREF: __cabs/r
x               dq ?                    ; XREF: __cabs+8/r __cabs+F/r
y               dq ?                    ; XREF: __cabs/r __cabs+4/r
_complex        ends

; ---------------------------------------------------------------------------

FILE            struc ; (sizeof=0x20, align=0x4, copyof_92)
                                        ; XREF: .data:File/r
                                        ; .data:stru_641940/r ...
_ptr            dd ?                    ; XREF: _sprintf+F/w
                                        ; _sprintf+44/r ... ; offset
_cnt            dd ?                    ; XREF: _sprintf+27/w
                                        ; _sprintf+36/r ...
_base           dd ?                    ; XREF: _sprintf+B/w
                                        ; _sscanf+D/w ... ; offset
_flag           dd ?                    ; XREF: _sprintf+1F/w
                                        ; _sscanf+2D/w ...
_file           dd ?
_charbuf        dd ?
_bufsiz         dd ?
_tmpfname       dd ?                    ; offset
FILE            ends

; ---------------------------------------------------------------------------

tm              struc ; (sizeof=0x24, align=0x4, copyof_94)
                                        ; XREF: ??0CTime@@QAE@HHHHHHH@Z/r
                                        ; ??0CTime@@QAE@GGH@Z/r
tm_sec          dd ?                    ; XREF: CTime::CTime(int,int,int,int,int,int,int)+A/w
                                        ; CTime::CTime(ushort,ushort,int)+18/w
tm_min          dd ?                    ; XREF: CTime::CTime(int,int,int,int,int,int,int)+10/w
                                        ; CTime::CTime(ushort,ushort,int)+2F/w
tm_hour         dd ?                    ; XREF: CTime::CTime(int,int,int,int,int,int,int)+16/w
                                        ; CTime::CTime(ushort,ushort,int)+23/w
tm_mday         dd ?                    ; XREF: CTime::CTime(int,int,int,int,int,int,int)+1C/w
                                        ; CTime::CTime(ushort,ushort,int)+3E/w
tm_mon          dd ?                    ; XREF: CTime::CTime(int,int,int,int,int,int,int)+25/w
                                        ; CTime::CTime(ushort,ushort,int)+57/w
tm_year         dd ?                    ; XREF: CTime::CTime(int,int,int,int,int,int,int)+30/w
                                        ; CTime::CTime(ushort,ushort,int)+41/w
tm_wday         dd ?
tm_yday         dd ?
tm_isdst        dd ?                    ; XREF: CTime::CTime(int,int,int,int,int,int,int)+36/w
                                        ; CTime::CTime(ushort,ushort,int)+4F/w
tm              ends

; ---------------------------------------------------------------------------

IStream         struc ; (sizeof=0x4, align=0x4, copyof_95)
                                        ; XREF: ?Read@CImageList@@QAEHPAVCArchive@@@Z/r
                                        ; ?Write@CImageList@@QAEHPAVCArchive@@@Z/r
lpVtbl          dd ?                    ; offset
IStream         ends

; ---------------------------------------------------------------------------

SHFILEINFOA     struc ; (sizeof=0x160, copyof_112)
                                        ; XREF: ?AfxResolveShortcut@@YGHPAVCWnd@@PBDPADH@Z/r
hIcon           dd ?                    ; offset
iIcon           dd ?
dwAttributes    dd ?                    ; XREF: AfxResolveShortcut(CWnd *,char const *,char *,int)+44/r
szDisplayName   db 260 dup(?)
szTypeName      db 80 dup(?)
SHFILEINFOA     ends

; ---------------------------------------------------------------------------

tagTEXTMETRICA  struc ; (sizeof=0x38, align=0x4, copyof_114)
                                        ; XREF: ?ConvertDialogUnitsToPixels@@YAXPBDGHHPAUtagSIZE@@@Z/r
                                        ; ?CalcFixedLayout@CStatusBar@@UAE?AVCSize@@HH@Z/r ...
tmHeight        dd ?                    ; XREF: ConvertDialogUnitsToPixels(char const *,ushort,int,int,tagSIZE *)+86/r
                                        ; CStatusBar::CalcFixedLayout(int,int)+AB/r
tmAscent        dd ?                    ; XREF: CMiniFrameWnd::OnNcPaint(void)+388/r
tmDescent       dd ?                    ; XREF: CMiniFrameWnd::OnNcPaint(void)+38E/r
tmInternalLeading dd ?                  ; XREF: CStatusBar::CalcFixedLayout(int,int)+B5/r
                                        ; CMiniFrameWnd::OnNcPaint(void)+39E/r
tmExternalLeading dd ?                  ; XREF: ConvertDialogUnitsToPixels(char const *,ushort,int,int,tagSIZE *)+83/r
tmAveCharWidth  dd ?
tmMaxCharWidth  dd ?
tmWeight        dd ?
tmOverhang      dd ?
tmDigitizedAspectX dd ?
tmDigitizedAspectY dd ?
tmFirstChar     db ?
tmLastChar      db ?
tmDefaultChar   db ?
tmBreakChar     db ?
tmItalic        db ?
tmUnderlined    db ?
tmStruckOut     db ?
tmPitchAndFamily db ?
tmCharSet       db ?
                db ? ; undefined
                db ? ; undefined
                db ? ; undefined
tagTEXTMETRICA  ends

; ---------------------------------------------------------------------------

DOCINFOA        struc ; (sizeof=0x14, align=0x4, copyof_115)
                                        ; XREF: ?StartDocA@CDC@@QAEHPBD@Z/r
cbSize          dd ?                    ; XREF: CDC::StartDocA(char const *)+23/w
lpszDocName     dd ?                    ; XREF: CDC::StartDocA(char const *)+1C/w ; offset
lpszOutput      dd ?                    ; offset
lpszDatatype    dd ?                    ; offset
fwType          dd ?
DOCINFOA        ends

; ---------------------------------------------------------------------------

FuncInfoV1      struc ; (sizeof=0x1C, mappedto_117)
                                        ; XREF: .rdata:stru_6172C0/r
                                        ; .rdata:stru_6172F0/r ...
magicNumber     dd ?                    ; base 16
maxState        dd ?                    ; base 10
pUnwindMap      dd ?                    ; offset
nTryBlocks      dd ?                    ; base 10
pTryBlockMap    dd ?                    ; offset
nIPMapEntries   dd ?                    ; base 10
pIPtoStateMap   dd ?                    ; offset
FuncInfoV1      ends

; ---------------------------------------------------------------------------

UnwindMapEntry  struc ; (sizeof=0x8, mappedto_118)
                                        ; XREF: .rdata:stru_6172E0/r
                                        ; .rdata:006172E8/r ...
toState         dd ?                    ; base 10
action          dd ?                    ; offset
UnwindMapEntry  ends

; ---------------------------------------------------------------------------

TryBlockMapEntry struc ; (sizeof=0x14, mappedto_119)
                                        ; XREF: .rdata:stru_617A38/r
                                        ; .rdata:00617A4C/r ...
tryLow          dd ?                    ; base 10
tryHigh         dd ?                    ; base 10
catchHigh       dd ?                    ; base 10
nCatches        dd ?                    ; base 10
pHandlerArray   dd ?                    ; offset
TryBlockMapEntry ends

; ---------------------------------------------------------------------------

HandlerType     struc ; (sizeof=0x10, mappedto_120)
                                        ; XREF: .rdata:stru_617A78/r
                                        ; .rdata:stru_617A88/r ...
adjectives      dd ?                    ; base 16
pType           dd ?                    ; offset
dispCatchObj    dd ?                    ; base 10
addressOfHandler dd ?                   ; offset
HandlerType     ends

; ---------------------------------------------------------------------------

TypeDescriptor  struc ; (sizeof=0x8, mappedto_89, variable size)
                                        ; XREF: .data:CFileException * `RTTI Type Descriptor'/r
                                        ; .data:CException * `RTTI Type Descriptor'/r ...
pVFTable        dd ?                    ; offset
spare           dd ?                    ; offset
name            db 0 dup(?)             ; string(C)
TypeDescriptor  ends

; ---------------------------------------------------------------------------

SCROLLINFO      struc ; (sizeof=0x1C, align=0x4, copyof_176)
                                        ; XREF: ?UpdateBars@CScrollView@@IAEXXZ/r
cbSize          dd ?
fMask           dd ?                    ; XREF: CScrollView::UpdateBars(void)+F3/w
nMin            dd ?                    ; XREF: CScrollView::UpdateBars(void)+FA/w
nMax            dd ?                    ; XREF: CScrollView::UpdateBars(void)+115/w
                                        ; CScrollView::UpdateBars(void)+150/w
nPage           dd ?                    ; XREF: CScrollView::UpdateBars(void)+10C/w
                                        ; CScrollView::UpdateBars(void)+147/w
nPos            dd ?
nTrackPos       dd ?
SCROLLINFO      ends

; ---------------------------------------------------------------------------

tagSCROLLINFO   struc ; (sizeof=0x1C, align=0x4)
                                        ; XREF: ?GetScrollLimit@CWnd@@QAEHH@Z/r
cbSize          dd ?
fMask           dd ?
nMin            dd ?
nMax            dd ?
nPage           dd ?                    ; XREF: CWnd::GetScrollLimit(int)+2D/r
nPos            dd ?
nTrackPos       dd ?
tagSCROLLINFO   ends

; ---------------------------------------------------------------------------

HWND__          struc ; (sizeof=0x4, align=0x4, copyof_11)
                                        ; XREF: .data:hWndInsertAfter/r
unused          dd ?
HWND__          ends

; ---------------------------------------------------------------------------

HBRUSH__        struc ; (sizeof=0x4, align=0x4, copyof_70)
                                        ; XREF: ?OnNcPaint@CMiniFrameWnd@@QAEXXZ/r
                                        ; ?OnNcPaint@CMiniFrameWnd@@QAEXXZ/r
unused          dd ?                    ; XREF: CMiniFrameWnd::OnNcPaint(void)+6D/w
                                        ; CMiniFrameWnd::OnNcPaint(void)+94/w ...
HBRUSH__        ends

; ---------------------------------------------------------------------------

_LARGE_INTEGER_837407842DC9087486FDFA5FEB63B74E struc ; (sizeof=0x8, align=0x4, copyof_101)
                                        ; XREF: _LARGE_INTEGER/r
                                        ; _LARGE_INTEGER/r
LowPart         dd ?
HighPart        dd ?
_LARGE_INTEGER_837407842DC9087486FDFA5FEB63B74E ends



_LARGE_INTEGER  union ; (sizeof=0x8, align=0x8, copyof_100)
                                        ; XREF: CArchiveStream::Seek(_LARGE_INTEGER,ulong,_ULARGE_INTEGER *)+B/r
                                        ; CArchiveStream::Seek(_LARGE_INTEGER,ulong,_ULARGE_INTEGER *)+17/r ...
l_anon     _LARGE_INTEGER_837407842DC9087486FDFA5FEB63B74E <>
u               _LARGE_INTEGER_837407842DC9087486FDFA5FEB63B74E <>
QuadPart        dq ?
_LARGE_INTEGER  ends

; ---------------------------------------------------------------------------


; ---------------------------------------------------------------------------

WNDCLASSA       struc ; (sizeof=0x28, align=0x4, copyof_230)
                                        ; XREF: ?AfxEndDeferRegisterClass@@YGHF@Z/r
style           dd ?                    ; XREF: AfxEndDeferRegisterClass(short)+45/w
                                        ; AfxEndDeferRegisterClass(short)+70/w ...
lpfnWndProc     dd ?                    ; XREF: AfxEndDeferRegisterClass(short)+1E/w ; offset
cbClsExtra      dd ?
cbWndExtra      dd ?
hInstance       dd ?                    ; XREF: AfxEndDeferRegisterClass(short)+29/w ; offset
hIcon           dd ?                    ; offset
hCursor         dd ?                    ; XREF: AfxEndDeferRegisterClass(short)+31/w ; offset
hbrBackground   dd ?                    ; XREF: AfxEndDeferRegisterClass(short)+AA/w
                                        ; AfxEndDeferRegisterClass(short)+C6/w ... ; offset
lpszMenuName    dd ?                    ; offset
lpszClassName   dd ?                    ; XREF: AfxEndDeferRegisterClass(short)+4D/w
                                        ; AfxEndDeferRegisterClass(short)+78/w ... ; offset
WNDCLASSA       ends

; ---------------------------------------------------------------------------

tagLOGFONTA     struc ; (sizeof=0x3C, align=0x4, copyof_75)
                                        ; XREF: ?CreatePointFont@CFont@@QAEHHPBDPAVCDC@@@Z/r
lfHeight        dd ?                    ; XREF: CFont::CreatePointFont(int,char const *,CDC *)+1C/w
lfWidth         dd ?
lfEscapement    dd ?
lfOrientation   dd ?
lfWeight        dd ?
lfItalic        db ?
lfUnderline     db ?
lfStrikeOut     db ?
lfCharSet       db ?                    ; XREF: CFont::CreatePointFont(int,char const *,CDC *)+24/w
lfOutPrecision  db ?
lfClipPrecision db ?
lfQuality       db ?
lfPitchAndFamily db ?
lfFaceName      db 32 dup(?)            ; XREF: CFont::CreatePointFont(int,char const *,CDC *)+1F/o
tagLOGFONTA     ends

; ---------------------------------------------------------------------------

_GUID           struc ; (sizeof=0x10, align=0x4, copyof_38)
                                        ; XREF: .rdata:CLSID_IClassFactory/r
                                        ; .rdata:stru_610168/r ...
Data1           dd ?
Data2           dw ?
Data3           dw ?
Data4           db 8 dup(?)
_GUID           ends

; ---------------------------------------------------------------------------

HMENU__         struc ; (sizeof=0x4, align=0x4, copyof_349)
                                        ; XREF: ?ReflectChildNotify@CWnd@@IAEHIIJPAJ@Z/r
unused          dd ?
HMENU__         ends

; ---------------------------------------------------------------------------

; enum __TI_flags, copyof_1, bitfield
TI_IsConst       = 1
TI_IsVolatile    = 2
TI_IsUnaligned   = 4
TI_IsPure        = 8
TI_IsWinRT       = 10h

; ---------------------------------------------------------------------------

; enum __CT_flags, copyof_2, bitfield
CT_IsSimpleType  = 1                    ; XREF: .rdata:__CT??_R0PAX@8/s
                                        ; .rdata:__CT??_R0PAVCObject@@@8/s ...
CT_ByReferenceOnly  = 2
CT_HasVirtualBase  = 4
CT_IsWinRTHandle  = 8
CT_IsStdBadAlloc  = 10h



sCRuntimeClass struct
	m_lpszClassName dword ?
	m_nObjectSize dword ?
	m_wSchema dword ?
	m_pfnCreateObject dword ?
	m_pBaseClass dword ?
    m_pNextClass dword ?
sCRuntimeClass ends