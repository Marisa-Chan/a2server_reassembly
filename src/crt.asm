                .686p
                .mmx
                .model flat
                assume fs:nothing
                option casemap :none

include kernel32.inc

include crt.inc ; to not define it as public


.data

_aexit_rtn      dd offset __exit        ; DATA XREF: __amsg_exit+20↑r

_exitflag     db ?                    ; DATA XREF: _doexit+38↑w
_C_Termination_Done    dd ?                    ; DATA XREF: _doexit+2E↑w
_C_Exit_Done    dd ?                    ; DATA XREF: _doexit+8↑r
__onexitend    dd ?                    ; DATA XREF: __onexit+11↑r
__onexitbegin    dd ?                    ; DATA XREF: __onexit+6↑r


_adbgmsg    dd ?                    ; DATA XREF: __FF_MSGBANNER+21↑r

aProgramNameUnk db '<program name unknown>',0
aMicrosoftVisua db 'Microsoft Visual C++ Runtime Library',0
aRuntimeErrorPr db 'Runtime Error!',0Ah
                db 0Ah
                db 'Program: ',0
asc_614A8C      db 0Ah                  ; DATA XREF: __NMSG_WRITE+11A↑o
                db 0Ah,0
asc_614AAC      db '...',0              ; DATA XREF: __NMSG_WRITE+C4↑o

; int dword_70D4E0
dword_70D4E0    dd ?

__sbh_threshold    dd 1E0h                 ; DATA XREF: __heap_alloc+9↑r

_crtheap           dd ?                    ; DATA XREF: __heap_alloc:loc_5BF062↑r



sigcritsect     db 18h dup(?)
ltlcritsect     db 18h dup(?)
hlcritsect      db 18h dup(?)
xlcritsect      db 18h dup(?)

_locktable      dd 0                    ; 0
                dd offset sigcritsect   ; 1
                dd 0                    ; 2
                dd 0                    ; 3
                dd 0                    ; 4
                dd 0                    ; 5
                dd 0                    ; 6
                dd 0                    ; 7
                dd 0                    ; 8
                dd offset hlcritsect    ; 9
                dd 0                    ; 10
                dd 0                    ; 11
                dd 0                    ; 12
                dd offset xlcritsect    ; 13
                dd 0                    ; 14
                dd 0                    ; 15
                dd 0                    ; 16
                dd offset ltlcritsect   ; 17
                dd 0                    ; 18
                dd 0                    ; 19
                dd 0                    ; 20
                dd 0                    ; 21
                dd 0                    ; 22
                dd 0                    ; 23
                dd 0                    ; 24
                dd 0                    ; 25
                dd 0                    ; 26
                dd 0                    ; 27
                dd 0                    ; 28
                dd 0                    ; 29
                dd 0                    ; 30
                dd 0                    ; 31
                dd 0                    ; 32
                dd 0                    ; 33
                dd 0                    ; 34
                dd 0                    ; 35
                dd 0                    ; 36
                dd 0                    ; 37
                dd 0                    ; 38
                dd 0                    ; 39
                dd 0                    ; 40
                dd 0                    ; 41
                dd 0                    ; 42
                dd 0                    ; 43
                dd 0                    ; 44
                dd 0                    ; 45
                dd 0                    ; 46
                dd 0                    ; 47


__small_block_heap      dd offset __small_block_heap    ; p_next_region
                        dd offset __small_block_heap    ; p_prev_region +4
                        dd offset __small_block_heap+18h            ; p_starting_region_map +8
                        dd offset __small_block_heap+18h            ; p_first_uncommitted +C
                        dd 0FFFFFFFFh           ; p_pages_begin  +10
                        dd 0FFFFFFFFh           ; p_pages_end  +14
                        dd 0F0h                 ; region_map[0]  +18
                        dd 0F1h                 ; region_map[1]  +1C
                        db 2000h dup(0)         ; region_map[...]

__sbh_p_starting_region      dd offset __small_block_heap



aRuntimeError   db 'runtime error ',0
asc_6147E8      db 0Dh,0Ah,0            ; DATA XREF: .data:006416D4\u2193o
aTlossError     db 'TLOSS error',0Dh,0Ah,0
aSingError      db 'SING error',0Dh,0Ah,0
aDomainError    db 'DOMAIN error',0Dh,0Ah,0
aR6028UnableToI db 'R6028',0Dh,0Ah
                db '- unable to initialize heap',0Dh,0Ah,0
aR6027NotEnough db 'R6027',0Dh,0Ah
                db '- not enough space for lowio initialization',0Dh,0Ah,0
aR6026NotEnough db 'R6026',0Dh,0Ah
                db '- not enough space for stdio initialization',0Dh,0Ah,0
aR6025PureVirtu db 'R6025',0Dh,0Ah
                db '- pure virtual function call',0Dh,0Ah,0
aR6024NotEnough db 'R6024',0Dh,0Ah
                db '- not enough space for _onexit/atexit table',0Dh,0Ah,0
aR6019UnableToO db 'R6019',0Dh,0Ah
                db '- unable to open console device',0Dh,0Ah,0
aR6018Unexpecte db 'R6018',0Dh,0Ah
                db '- unexpected heap error',0Dh,0Ah,0
aR6017Unexpecte db 'R6017',0Dh,0Ah
                db '- unexpected multithread lock error',0Dh,0Ah,0
aR6016NotEnough db 'R6016',0Dh,0Ah
                db '- not enough space for thread data',0Dh,0Ah,0
aAbnormalProgra db 0Dh,0Ah
                db 'abnormal program termination',0Dh,0Ah,0
aR6009NotEnough db 'R6009',0Dh,0Ah
                db '- not enough space for environment',0Dh,0Ah,0
aR6008NotEnough db 'R6008',0Dh,0Ah
                db '- not enough space for arguments',0Dh,0Ah,0
aR6002FloatingP db 'R6002',0Dh,0Ah      ; DATA XREF: .data:lpBuffer↓o
                db '- floating point not loaded',0Dh,0Ah,0


rterrs          dd 2                      ; DATA XREF: __NMSG_WRITE+A↑o
                dd offset aR6002FloatingP ; "R6002\r\n- floating point not loaded\r"...
                dd 8
                dd offset aR6008NotEnough ; "R6008\r\n- not enough space for argumen"...
                dd 9
                dd offset aR6009NotEnough ; "R6009\r\n- not enough space for environ"...
                dd 0Ah
                dd offset aAbnormalProgra ; "\r\nabnormal program termination\r\n"
                dd 10h
                dd offset aR6016NotEnough ; "R6016\r\n- not enough space for thread "...
                dd 11h
                dd offset aR6017Unexpecte ; "R6017\r\n- unexpected multithread lock "...
                dd 12h
                dd offset aR6018Unexpecte ; "R6018\r\n- unexpected heap error\r\n"
                dd 13h
                dd offset aR6019UnableToO ; "R6019\r\n- unable to open console devic"...
                dd 18h
                dd offset aR6024NotEnough ; "R6024\r\n- not enough space for _onexit"...
                dd 19h
                dd offset aR6025PureVirtu ; "R6025\r\n- pure virtual function call\r"...
                dd 1Ah
                dd offset aR6026NotEnough ; "R6026\r\n- not enough space for stdio i"...
                dd 1Bh
                dd offset aR6027NotEnough ; "R6027\r\n- not enough space for lowio i"...
                dd 1Ch
                dd offset aR6028UnableToI ; "R6028\r\n- unable to initialize heap\r"...
                dd 78h
                dd offset aDomainError  ; "DOMAIN error\r\n"
                dd 79h
                dd offset aSingError    ; "SING error\r\n"
                dd 7Ah
                dd offset aTlossError   ; "TLOSS error\r\n"
                dd 0FCh
                dd offset asc_6147E8    ; "\r\n"
                dd 0FFh
                dd offset aRuntimeError ; "runtime error "


aGetlastactivep db 'GetLastActivePopup',0
aGetactivewindo db 'GetActiveWindow',0  ; DATA XREF: ___crtMessageBoxA+36↑o
aMessageboxa    db 'MessageBoxA',0      ; DATA XREF: ___crtMessageBoxA+25↑o
LibFileName     db 'user32.dll',0       ; DATA XREF: ___crtMessageBoxA+E↑o



__sbh_decommitable_pages    dd ?                    ; DATA XREF: ___sbh_decommit_pages

_pnhHeap    dd ?                    ; DATA XREF: _set_new_handler

__error_mode    dd ?                    ; DATA XREF: __amsg_exit

__app_type    dd 2                   

pfnMessageBoxA    dd ?                    ; DATA XREF: ___crtMessageBoxA↑r
pfnGetActiveWindow    dd ?                    ; DATA XREF: ___crtMessageBoxA+44↑w
pfnGetLastActivePopup    dd ?                    ; DATA XREF: ___crtMessageBoxA+4B↑w


_XcptActTabSize     dd 78h   
_XcptActTabCount    dd 0Ah 

_XcptActTab     dd 0C0000005h
                dd 0Bh
                dd 0
                dd 0C000001Dh
                dd 4
                dd 0
                dd 0C0000096h
                dd 4
                dd 0
                dd 0C000008Dh
                dd 8
                dd 0
                dd 0C000008Eh
                dd 8
                dd 0
                dd 0C000008Fh
                dd 8
                dd 0
                dd 0C0000090h
                dd 8
                dd 0
                dd 0C0000091h
                dd 8
                dd 0
                dd 0C0000092h
                dd 8
                dd 0
                dd 0C0000093h
                dd 8
                dd 0

__tlsindex      dd 0FFFFFFFFh 

dword_70D584    dd ?                    ; DATA XREF: _signal:loc_5CC20C↑r
dword_70D588    dd ?                    ; DATA XREF: _signal:loc_5CC229↑r
dword_70D58C    dd ?                    ; DATA XREF: _signal:loc_5CC246↑r
dword_70D590    dd ?                    ; DATA XREF: _signal:loc_5CC263↑r

ConsoleCtrlHandler_Installed    dd ?                    ; DATA XREF: _signal:loc_5CC1CD↑r

.code


; =============== S U B R O U T I N E =======================================

_malloc         proc near               ; CODE XREF: sub_402FB8+11C↑p

varSize            = dword ptr  4
                mov     eax, dword_70D4E0
                mov     ecx, dword ptr [esp+varSize]
                push    eax             ; int
                push    ecx             ; varSize
                call    __nh_malloc
                add     esp, 8
                retn
_malloc         endp


; =============== S U B R O U T I N E =======================================

; Attributes: library function

; void *__cdecl realloc(void *Block, size_t varSize)
_realloc        proc near               ; CODE XREF: sub_4E819C+F2↑p

var_8           = dword ptr -8
var_4           = dword ptr -4
lpMem           = dword ptr  4
dwBytes         = dword ptr  8

                sub     esp, 8
                push    ebx
                push    ebp
                push    esi
                mov     esi, [esp+14h+lpMem]
                test    esi, esi
                push    edi
                jnz     short loc_5C1E54
                mov     eax, dword ptr [esp+18h+dwBytes]
                push    eax             ; varSize
                call    _malloc
                add     esp, 4
                pop     edi
                pop     esi
                pop     ebp
                pop     ebx
                add     esp, 8
                retn
; ---------------------------------------------------------------------------

loc_5C1E54:                             ; CODE XREF: _realloc+D↑j
                mov     edi, [esp+18h+dwBytes]
                test    edi, edi
                jnz     short loc_5C1E6F
                push    esi             ; Block
                call    _free
                add     esp, 4
                xor     eax, eax
                pop     edi
                pop     esi
                pop     ebp
                pop     ebx
                add     esp, 8
                retn
; ---------------------------------------------------------------------------

loc_5C1E6F:                             ; CODE XREF: _realloc+2A↑j
                cmp     edi, 0FFFFFFE0h
                ja      short loc_5C1E90
                test    edi, edi
                jbe     short loc_5C1E84
                add     edi, 0Fh
                and     edi, 0FFFFFFF0h
                mov     [esp+18h+dwBytes], edi
                jmp     short loc_5C1E8C
; ---------------------------------------------------------------------------

loc_5C1E84:                             ; CODE XREF: _realloc+46↑j
                mov     [esp+18h+dwBytes], 10h

loc_5C1E8C:                             ; CODE XREF: _realloc+52↑j
                mov     edi, [esp+18h+dwBytes]

loc_5C1E90:                             ; CODE XREF: _realloc+42↑j
                xor     ebx, ebx
                cmp     edi, 0FFFFFFE0h
                ja      loc_5C1FB0
                push    9
                call    __lock
                add     esp, 4
                lea     ecx, [esp+18h+var_8]
                lea     edx, [esp+18h+var_4]
                push    ecx
                push    edx
                push    esi
                call    ___sbh_find_block
                mov     ebp, eax
                add     esp, 0Ch
                test    ebp, ebp
                jz      loc_5C1F94
                cmp     edi, __sbh_threshold
                jnb     short loc_5C1F35
                mov     eax, dword ptr [esp+18h+var_8]
                mov     ecx, dword ptr [esp+18h+var_4]
                mov     ebx, edi
                shr     ebx, 4
                push    ebx
                push    ebp
                push    eax
                push    ecx
                call    ___sbh_resize_block
                add     esp, 10h
                test    eax, eax
                jz      short loc_5C1EEB
                mov     ebx, esi
                jmp     short loc_5C1F35
; ---------------------------------------------------------------------------

loc_5C1EEB:                             ; CODE XREF: _realloc+B5↑j
                push    ebx
                call    ___sbh_alloc_block
                mov     ebx, eax
                add     esp, 4
                test    ebx, ebx
                jz      short loc_5C1F39
                xor     eax, eax
                mov     al, [ebp+0]
                shl     eax, 4
                cmp     eax, edi
                jb      short loc_5C1F08
                mov     eax, edi

loc_5C1F08:                             ; CODE XREF: _realloc+D4↑j
                mov     ecx, eax
                mov     edi, ebx
                mov     edx, ecx
                push    ebp
                shr     ecx, 2
                rep movsd
                mov     ecx, edx
                and     ecx, 3
                rep movsb
                mov     eax, dword ptr [esp+1Ch+var_8]
                mov     ecx, dword ptr [esp+1Ch+var_4]
                push    eax
                push    ecx
                call    ___sbh_free_block
                mov     edi, [esp+24h+dwBytes]
                mov     esi, [esp+24h+lpMem]
                add     esp, 0Ch

loc_5C1F35:                             ; CODE XREF: _realloc+98↑j
                test    ebx, ebx
                jnz     short loc_5C1F88

loc_5C1F39:                             ; CODE XREF: _realloc+C8↑j
                mov     edx, _crtheap
                push    edi             ; dwBytes
                push    0               ; dwFlags
                push    edx             ; _crtheap
                call    HeapAlloc
                mov     ebx, eax
                test    ebx, ebx
                jz      short loc_5C1F88
                xor     eax, eax
                mov     al, [ebp+0]
                shl     eax, 4
                cmp     eax, edi
                jb      short loc_5C1F5D
                mov     eax, edi

loc_5C1F5D:                             ; CODE XREF: _realloc+129↑j
                mov     ecx, eax
                mov     edi, ebx
                shr     ecx, 2
                rep movsd
                mov     ecx, eax
                push    ebp
                and     ecx, 3
                rep movsb
                mov     ecx, dword ptr [esp+1Ch+var_8]
                mov     edx, dword ptr [esp+1Ch+var_4]
                push    ecx
                push    edx
                call    ___sbh_free_block
                mov     edi, [esp+24h+dwBytes]
                mov     esi, [esp+24h+lpMem]
                add     esp, 0Ch

loc_5C1F88:                             ; CODE XREF: _realloc+107↑j
                push    9
                call    __unlock
                add     esp, 4
                jmp     short loc_5C1FB0
; ---------------------------------------------------------------------------

loc_5C1F94:                             ; CODE XREF: _realloc+8C↑j
                push    9
                call    __unlock
                mov     eax, _crtheap
                add     esp, 4
                push    edi             ; dwBytes
                push    esi             ; lpMem
                push    0               ; dwFlags
                push    eax             ; _crtheap
                call    HeapReAlloc
                mov     ebx, eax

loc_5C1FB0:                             ; CODE XREF: _realloc+65↑j
                test    ebx, ebx
                jnz     short loc_5C1FD6
                mov     eax, dword_70D4E0
                test    eax, eax
                jz      short loc_5C1FD6
                push    edi             ; varSize
                call    __callnewh
                add     esp, 4
                test    eax, eax
                jnz     loc_5C1E90
                pop     edi
                pop     esi
                pop     ebp
                pop     ebx
                add     esp, 8
                retn
; ---------------------------------------------------------------------------

loc_5C1FD6:                             ; CODE XREF: _realloc+182↑j
                pop     edi
                pop     esi
                mov     eax, ebx
                pop     ebp
                pop     ebx
                add     esp, 8
                retn
_realloc        endp


; =============== S U B R O U T I N E =======================================

; Attributes: library function

; void *__cdecl calloc(size_t Count, size_t varSize)
_calloc         proc near               ; CODE XREF: __beginthread+8↑p

Count           = dword ptr  4
varSize            = dword ptr  8

                push    ebx
                mov     ebx, dword ptr [esp+4+varSize]
                imul    ebx, [esp+4+Count]
                push    ebp
                push    esi
                cmp     ebx, 0FFFFFFE0h
                push    edi
                ja      short loc_5C61E3
                test    ebx, ebx
                jbe     short loc_5C61DE
                add     ebx, 0Fh
                and     ebx, 0FFFFFFF0h
                jmp     short loc_5C61E3
; ---------------------------------------------------------------------------

loc_5C61DE:                             ; CODE XREF: _calloc+14↑j
                mov     ebx, 10h

loc_5C61E3:                             ; CODE XREF: _calloc+10↑j
                mov     ebp, ds:HeapAlloc

loc_5C61E9:                             ; CODE XREF: _calloc+9D↓j
                xor     esi, esi
                cmp     ebx, 0FFFFFFE0h
                ja      short loc_5C6245
                cmp     ebx, __sbh_threshold
                ja      short loc_5C6234
                push    9
                call    __lock
                mov     eax, ebx
                add     esp, 4
                shr     eax, 4
                push    eax
                call    ___sbh_alloc_block
                add     esp, 4
                mov     esi, eax
                push    9
                call    __unlock
                add     esp, 4
                test    esi, esi
                jz      short loc_5C6238
                mov     ecx, ebx
                xor     eax, eax
                mov     edx, ecx
                mov     edi, esi
                shr     ecx, 2
                rep stosd
                mov     ecx, edx
                and     ecx, 3
                rep stosb

loc_5C6234:                             ; CODE XREF: _calloc+36↑j
                test    esi, esi
                jnz     short loc_5C6264

loc_5C6238:                             ; CODE XREF: _calloc+5E↑j
                mov     eax, _crtheap
                push    ebx             ; dwBytes
                push    8               ; dwFlags
                push    eax             ; _crtheap
                call    ebp ; HeapAlloc
                mov     esi, eax

loc_5C6245:                             ; CODE XREF: _calloc+2E↑j
                test    esi, esi
                jnz     short loc_5C6264
                mov     eax, dword_70D4E0
                test    eax, eax
                jz      short loc_5C6264
                push    ebx             ; varSize
                call    __callnewh
                add     esp, 4
                test    eax, eax
                jnz     short loc_5C61E9
                pop     edi
                pop     esi
                pop     ebp
                pop     ebx
                retn
; ---------------------------------------------------------------------------

loc_5C6264:                             ; CODE XREF: _calloc+76↑j
                mov     eax, esi
                pop     edi
                pop     esi
                pop     ebp
                pop     ebx
                retn
_calloc         endp


; =============== S U B R O U T I N E =======================================

; Attributes: library function

; void __cdecl free(void *Block)
_free           proc near               ; CODE XREF: sub_40328E+11↑p

var_4           = dword ptr -4
lpMem           = dword ptr  4

                push    ecx
                push    esi
                mov     esi, [esp+8+lpMem]
                test    esi, esi
                jz      short loc_5BF0E5
                push    9
                call    __lock
                add     esp, 4
                lea     eax, [esp+8+lpMem]
                lea     ecx, [esp+8+var_4]
                push    eax
                push    ecx
                push    esi
                call    ___sbh_find_block
                add     esp, 0Ch
                test    eax, eax
                jz      short loc_5BF0CB
                mov     edx, dword ptr [esp+8+lpMem]
                push    eax
                mov     eax, dword ptr [esp+0Ch+var_4]
                push    edx
                push    eax
                call    ___sbh_free_block
                add     esp, 0Ch
                push    9
                call    __unlock
                add     esp, 4
                pop     esi
                pop     ecx
                retn
; ---------------------------------------------------------------------------

loc_5BF0CB:                             ; CODE XREF: _free+29↑j
                push    9
                call    __unlock
                mov     ecx, _crtheap
                add     esp, 4
                push    esi             ; lpMem
                push    0               ; dwFlags
                push    ecx             ; _crtheap
                call    HeapFree

loc_5BF0E5:                             ; CODE XREF: _free+8↑j
                pop     esi
                pop     ecx
                retn
_free           endp

; ---------------------------------------------------------------------------


; =============== S U B R O U T I N E =======================================

; Attributes: library function

; int __cdecl _nh_malloc(size_t varSize, int)
__nh_malloc     proc near               ; CODE XREF: _malloc+B↑p

varSize            = dword ptr  4
arg_4           = dword ptr  8

                push    esi
                mov     esi, [esp+4+varSize]
                cmp     esi, 0FFFFFFE0h
                push    edi
                ja      short loc_5BF00F
                test    esi, esi
                jnz     short loc_5BEFE4
                mov     esi, 1

loc_5BEFE4:                             ; CODE XREF: __nh_malloc+D↑j
                mov     edi, [esp+8+arg_4]

loc_5BEFE8:                             ; CODE XREF: __nh_malloc+3D↓j
                cmp     esi, 0FFFFFFE0h
                ja      short loc_5BEFF8
                push    esi
                call    __heap_alloc
                add     esp, 4
                jmp     short loc_5BEFFA
; ---------------------------------------------------------------------------

loc_5BEFF8:                             ; CODE XREF: __nh_malloc+1B↑j
                xor     eax, eax

loc_5BEFFA:                             ; CODE XREF: __nh_malloc+26↑j
                test    eax, eax
                jnz     short loc_5BF011
                test    edi, edi
                jz      short loc_5BF011
                push    esi             ; varSize
                call    __callnewh
                add     esp, 4
                test    eax, eax
                jnz     short loc_5BEFE8

loc_5BF00F:                             ; CODE XREF: __nh_malloc+9↑j
                xor     eax, eax

loc_5BF011:                             ; CODE XREF: __nh_malloc+2C↑j
                pop     edi
                pop     esi
                retn
__nh_malloc     endp




__lock          proc near               ; CODE XREF: __heap_alloc+17↑p

arg_0           = dword ptr  8

                push    ebp
                mov     ebp, esp
                push    edi
                mov     edi, [ebp+arg_0]
                mov     eax, _locktable[edi*4]
                test    eax, eax
                jnz     short loc_5C7815
                push    esi
                push    18h             ; varSize
                call    _malloc
                mov     esi, eax
                add     esp, 4
                test    esi, esi
                jnz     short loc_5C77DD
                push    11h
                call    __amsg_exit
; ---------------------------------------------------------------------------
                add     esp, 4

loc_5C77DD:                             ; CODE XREF: __lock+21↑j
                push    11h
                call    __lock
                mov     eax, _locktable[edi*4]
                add     esp, 4
                test    eax, eax
                push    esi             ; Block
                jnz     short loc_5C7802
                call    InitializeCriticalSection
                mov     _locktable[edi*4], esi
                jmp     short loc_5C780A
; ---------------------------------------------------------------------------

loc_5C7802:                             ; CODE XREF: __lock+41↑j
                call    _free
                add     esp, 4

loc_5C780A:                             ; CODE XREF: __lock+50↑j
                push    11h
                call    __unlock
                add     esp, 4
                pop     esi

loc_5C7815:                             ; CODE XREF: __lock+10↑j
                mov     eax, _locktable[edi*4]
                push    eax             ; lpCriticalSection
                call    EnterCriticalSection
                pop     edi
                pop     ebp
                retn
__lock          endp

; ---------------------------------------------------------------------------


; =============== S U B R O U T I N E =======================================

; Attributes: library function bp-based frame

__unlock        proc near               ; CODE XREF: __heap_alloc+31↑p

arg_0           = dword ptr  8

                push    ebp
                mov     ebp, esp
                mov     eax, dword ptr [ebp+arg_0]
                mov     ecx, _locktable[eax*4]
                push    ecx             ; lpCriticalSection
                call    LeaveCriticalSection
                pop     ebp
                retn
__unlock        endp

; ---------------------------------------------------------------------------



; =============== S U B R O U T I N E =======================================

; Attributes: library function

___sbh_find_block proc near             ; CODE XREF: _free+1F↑p

arg_0           = dword ptr  4
arg_4           = dword ptr  8
arg_8           = dword ptr  0Ch

                mov     ecx, dword ptr [esp+arg_0]
                mov     eax, offset __small_block_heap

loc_5C7C29:                             ; CODE XREF: ___sbh_find_block+1C↓j
                cmp     ecx, [eax+10h]
                jbe     short loc_5C7C33
                cmp     ecx, [eax+14h]
                jb      short loc_5C7C3E

loc_5C7C33:                             ; CODE XREF: ___sbh_find_block+C↑j
                mov     eax, dword ptr [eax]
                cmp     eax, offset __small_block_heap
                jz      short loc_5C7C76
                jmp     short loc_5C7C29
; ---------------------------------------------------------------------------

loc_5C7C3E:                             ; CODE XREF: ___sbh_find_block+11↑j
                test    cl, 0Fh
                jnz     short loc_5C7C76
                mov     edx, ecx
                and     edx, 0FFFh
                cmp     edx, 100h
                jb      short loc_5C7C76
                mov     edx, dword ptr [esp+arg_4]
                mov     [edx], eax
                mov     edx, dword ptr [esp+arg_8]
                mov     eax, ecx
                and     eax, 0FFFFF000h
                sub     ecx, eax
                mov     [edx], eax
                sub     ecx, 100h
                sar     ecx, 4
                lea     eax, [ecx+eax+8]
                retn
; ---------------------------------------------------------------------------

loc_5C7C76:                             ; CODE XREF: ___sbh_find_block+1A↑j
                xor     eax, eax
                retn
___sbh_find_block endp

; ---------------------------------------------------------------------------

; =============== S U B R O U T I N E =======================================

; Attributes: library function

___sbh_free_block proc near             ; CODE XREF: _free+36↑p

arg_0           = dword ptr  4
arg_4           = dword ptr  8
arg_8           = dword ptr  0Ch

                mov     eax, dword ptr [esp+arg_0]
                mov     ecx, dword ptr [esp+arg_4]
                push    esi
                xor     edx, edx
                sub     ecx, [eax+10h]
                sar     ecx, 0Ch
                mov     esi, [eax+ecx*8+18h]
                lea     eax, [eax+ecx*8+18h]
                mov     ecx, dword ptr [esp+4+arg_8]
                mov     dl, [ecx]
                add     esi, edx
                mov     [eax], esi
                mov     byte ptr [ecx], 0
                mov     ecx, dword ptr [eax]
                mov     dword ptr [eax+4], 0F1h
                cmp     ecx, 0F0h
                jnz     short loc_5C7CD1
                mov     eax, __sbh_decommitable_pages
                inc     eax
                cmp     eax, 20h ; ' '
                mov     __sbh_decommitable_pages, eax
                jnz     short loc_5C7CD1
                push    10h
                call    ___sbh_decommit_pages
                add     esp, 4

loc_5C7CD1:                             ; CODE XREF: ___sbh_free_block+35↑j
                pop     esi
                retn
___sbh_free_block endp

; ---------------------------------------------------------------------------



; =============== S U B R O U T I N E =======================================

; Attributes: library function

; int __cdecl _set_sbh_threshold(size_t NewValue)
__set_sbh_threshold proc near

NewValue        = dword ptr  4

                mov     eax, dword ptr [esp+NewValue]
                add     eax, 0Fh
                and     al, 0F0h
                cmp     eax, 780h
                ja      short loc_5C797B
                mov     __sbh_threshold, eax
                mov     eax, 1
                retn
; ---------------------------------------------------------------------------

loc_5C797B:                             ; CODE XREF: __set_sbh_threshold+E↑j
                xor     eax, eax
                retn
__set_sbh_threshold endp

; ---------------------------------------------------------------------------
                align 10h

; =============== S U B R O U T I N E =======================================

; Attributes: library function

___sbh_new_region proc near             ; CODE XREF: __heap_init:loc_5C7699↑p
                mov     eax, __small_block_heap+10h
                push    ebp
                push    esi
                cmp     eax, 0FFFFFFFFh
                push    edi
                jnz     short loc_5C7994
                mov     ebp, offset __small_block_heap
                jmp     short loc_5C79B1
; ---------------------------------------------------------------------------

loc_5C7994:                             ; CODE XREF: ___sbh_new_region+B↑j
                mov     eax, _crtheap
                push    2020h           ; dwBytes
                push    0               ; dwFlags
                push    eax             ; _crtheap
                call    HeapAlloc
                mov     ebp, eax
                test    ebp, ebp
                jz      loc_5C7ADC

loc_5C79B1:                             ; CODE XREF: ___sbh_new_region+12↑j
                mov     edi, ds:VirtualAlloc
                push    4               ; flProtect
                push    2000h           ; flAllocationType
                push    400000h         ; dwSize
                push    0               ; lpAddress
                call    edi ; VirtualAlloc
                mov     esi, eax
                test    esi, esi
                jz      loc_5C7AC5
                push    4               ; flProtect
                push    1000h           ; flAllocationType
                push    10000h          ; dwSize
                push    esi             ; lpAddress
                call    edi ; VirtualAlloc
                test    eax, eax
                jz      loc_5C7AB7
                cmp     ebp, offset __small_block_heap
                jnz     short loc_5C7A18
                mov     eax, __small_block_heap
                test    eax, eax
                jnz     short loc_5C7A03
                mov     __small_block_heap, offset __small_block_heap

loc_5C7A03:                             ; CODE XREF: ___sbh_new_region+77↑j
                mov     eax, [__small_block_heap+4h]
                test    eax, eax
                jnz     short loc_5C7A33
                mov     [__small_block_heap+4h], offset __small_block_heap
                jmp     short loc_5C7A33
; ---------------------------------------------------------------------------

loc_5C7A18:                             ; CODE XREF: ___sbh_new_region+6E↑j
                mov     dword ptr [ebp+0], offset __small_block_heap
                mov     ecx, [__small_block_heap+4h]
                mov     [ebp+4], ecx
                mov     [__small_block_heap+4h], ebp
                mov     edx, dword ptr [ebp+4]
                mov     [edx], ebp

loc_5C7A33:                             ; CODE XREF: ___sbh_new_region+8A↑j
                lea     eax, [esi+400000h]
                lea     ecx, [ebp+18h]
                lea     edx, [ebp+98h]
                mov     [ebp+14h], eax
                mov     [ebp+10h], esi
                mov     [ebp+8], ecx
                mov     [ebp+0Ch], edx
                xor     eax, eax
                mov     edi, 0F1h

loc_5C7A55:                             ; CODE XREF: ___sbh_new_region+F0↓j
                xor     edx, edx
                cmp     eax, 10h
                setnl   dl
                dec     edx
                add     ecx, 8
                and     edx, edi
                dec     edx
                inc     eax
                mov     [ecx-8], edx
                mov     [ecx-4], edi
                cmp     eax, 400h
                jl      short loc_5C7A55
                mov     ecx, 4000h
                xor     eax, eax
                mov     edi, esi
                rep stosd
                mov     eax, dword ptr [ebp+10h]
                add     eax, 10000h
                cmp     esi, eax
                jnb     short loc_5C7AB1
                mov     ecx, 0F0h
                mov     al, 0FFh

loc_5C7A90:                             ; CODE XREF: ___sbh_new_region+12F↓j
                lea     edx, [esi+8]
                mov     [esi+4], ecx
                mov     [esi], edx
                mov     [esi+0F8h], al
                mov     edx, dword ptr [ebp+10h]
                add     esi, 1000h
                add     edx, 10000h
                cmp     esi, edx
                jb      short loc_5C7A90

loc_5C7AB1:                             ; CODE XREF: ___sbh_new_region+107↑j
                mov     eax, ebp
                pop     edi
                pop     esi
                pop     ebp
                retn
; ---------------------------------------------------------------------------

loc_5C7AB7:                             ; CODE XREF: ___sbh_new_region+62↑j
                push    8000h           ; dwFreeType
                push    0               ; dwSize
                push    esi             ; lpAddress
                call    VirtualFree

loc_5C7AC5:                             ; CODE XREF: ___sbh_new_region+4B↑j
                cmp     ebp, offset __small_block_heap
                jz      short loc_5C7ADC
                mov     eax, _crtheap
                push    ebp             ; lpMem
                push    0               ; dwFlags
                push    eax             ; _crtheap
                call    HeapFree

loc_5C7ADC:                             ; CODE XREF: ___sbh_new_region+2B↑j
                pop     edi
                pop     esi
                xor     eax, eax
                pop     ebp
                retn
___sbh_new_region endp

; ---------------------------------------------------------------------------
                align 10h

; =============== S U B R O U T I N E =======================================

; Attributes: library function

; int __cdecl __sbh_release_region(LPVOID lpMem)
___sbh_release_region proc near         ; CODE XREF: ___sbh_decommit_pages+AC↓p

lpMem           = dword ptr  4

                push    esi
                mov     esi, [esp+4+lpMem]
                push    8000h           ; dwFreeType
                push    0               ; dwSize
                mov     eax, dword ptr [esi+10h]
                push    eax             ; lpAddress
                call    VirtualFree
                cmp     __sbh_p_starting_region, esi
                jnz     short loc_5C7B17
                mov     ecx, dword ptr [esi+4]
                mov     __sbh_p_starting_region, ecx

loc_5C7B17:                             ; CODE XREF: ___sbh_release_region+1C↑j
                cmp     esi, offset __small_block_heap
                jz      short loc_5C7B3F
                mov     edx, dword ptr [esi+4]
                mov     eax, dword ptr [esi]
                push    esi             ; lpMem
                push    0               ; dwFlags
                mov     [edx], eax
                mov     ecx, dword ptr [esi]
                mov     edx, dword ptr [esi+4]
                mov     [ecx+4], edx
                mov     eax, _crtheap
                push    eax             ; _crtheap
                call    HeapFree
                pop     esi
                retn
; ---------------------------------------------------------------------------

loc_5C7B3F:                             ; CODE XREF: ___sbh_release_region+2D↑j
                mov     __small_block_heap+10h, 0FFFFFFFFh
                pop     esi
                retn
___sbh_release_region endp

; ---------------------------------------------------------------------------
                align 10h

; =============== S U B R O U T I N E =======================================

; Attributes: library function

___sbh_decommit_pages proc near         ; CODE XREF: ___sbh_free_block+49↓p

arg_0           = dword ptr  4

                push    ebx
                push    ebp
                push    esi
                push    edi
                mov     edi, [__small_block_heap+4h]

loc_5C7B5A:                             ; CODE XREF: ___sbh_decommit_pages+C2↓j
                cmp     dword ptr [edi+10h], 0FFFFFFFFh
                jz      loc_5C7C04
                xor     ebp, ebp
                lea     esi, [edi+2010h]
                mov     ebx, 3FF000h

loc_5C7B71:                             ; CODE XREF: ___sbh_decommit_pages+7B↓j
                cmp     dword ptr [esi], 0F0h
                jnz     short loc_5C7BC0
                mov     eax, dword ptr [edi+10h]
                push    4000h           ; dwFreeType
                add     eax, ebx
                push    1000h           ; dwSize
                push    eax             ; lpAddress
                call    VirtualFree
                test    eax, eax
                jz      short loc_5C7BC0
                mov     dword ptr [esi], 0FFFFFFFFh
                mov     edx, __sbh_decommitable_pages
                dec     edx
                mov     __sbh_decommitable_pages, edx
                mov     eax, dword ptr [edi+0Ch]
                test    eax, eax
                jz      short loc_5C7BB1
                cmp     eax, esi
                jbe     short loc_5C7BB4

loc_5C7BB1:                             ; CODE XREF: ___sbh_decommit_pages+5B↑j
                mov     [edi+0Ch], esi

loc_5C7BB4:                             ; CODE XREF: ___sbh_decommit_pages+5F↑j
                mov     eax, dword ptr [esp+10h+arg_0]
                inc     ebp
                dec     eax
                mov     [esp+10h+arg_0], eax
                jz      short loc_5C7BCD

loc_5C7BC0:                             ; CODE XREF: ___sbh_decommit_pages+27↑j
                sub     ebx, 1000h
                sub     esi, 8
                test    ebx, ebx
                jge     short loc_5C7B71

loc_5C7BCD:                             ; CODE XREF: ___sbh_decommit_pages+6E↑j
                mov     edx, edi
                mov     edi, [edi+4]
                test    ebp, ebp
                jz      short loc_5C7C04
                cmp     dword ptr [edx+18h], 0FFFFFFFFh
                jnz     short loc_5C7C04
                mov     eax, 1
                lea     ecx, [edx+20h]

loc_5C7BE4:                             ; CODE XREF: ___sbh_decommit_pages+A2↓j
                cmp     dword ptr [ecx], 0FFFFFFFFh
                jnz     short loc_5C7BF4
                inc     eax
                add     ecx, 8
                cmp     eax, 400h
                jl      short loc_5C7BE4

loc_5C7BF4:                             ; CODE XREF: ___sbh_decommit_pages+97↑j
                cmp     eax, 400h
                jnz     short loc_5C7C04
                push    edx             ; lpMem
                call    ___sbh_release_region
                add     esp, 4

loc_5C7C04:                             ; CODE XREF: ___sbh_decommit_pages+E↑j
                cmp     edi, [__small_block_heap+4h]
                jz      short loc_5C7C18
                mov     eax, dword ptr [esp+10h+arg_0]
                test    eax, eax
                jg      loc_5C7B5A

loc_5C7C18:                             ; CODE XREF: ___sbh_decommit_pages+BA↑j
                pop     edi
                pop     esi
                pop     ebp
                pop     ebx
                retn
___sbh_decommit_pages endp

; ---------------------------------------------------------------------------
                align 10h



; =============== S U B R O U T I N E =======================================

; Attributes: library function

___sbh_alloc_block proc near            ; CODE XREF: __heap_alloc+25↑p

var_4           = dword ptr -4
arg_0           = dword ptr  4

                push    ecx
                mov     ecx, __sbh_p_starting_region
                push    ebx
                mov     ebx, dword ptr [esp+8+arg_0]
                push    ebp
                push    esi
                push    edi
                mov     [esp+14h+var_4], ecx

loc_5C7CF3:                             ; CODE XREF: ___sbh_alloc_block+B3↓j
                mov     eax, dword ptr [ecx+10h]
                cmp     eax, 0FFFFFFFFh
                jz      loc_5C7D84
                mov     edi, [ecx+8]
                lea     ebp, [ecx+2018h]
                mov     esi, edi
                sub     esi, ecx
                sub     esi, 18h
                sar     esi, 3
                shl     esi, 0Ch
                add     esi, eax
                cmp     edi, ebp
                jnb     short loc_5C7D49

loc_5C7D1B:                             ; CODE XREF: ___sbh_alloc_block+67↓j
                mov     eax, dword ptr [edi]
                cmp     eax, ebx
                jl      short loc_5C7D3C
                cmp     [edi+4], ebx
                jbe     short loc_5C7D3C
                push    ebx
                push    eax
                push    esi
                call    ___sbh_alloc_block_from_page
                add     esp, 0Ch
                test    eax, eax
                jnz     short loc_5C7D98
                mov     ecx, dword ptr [esp+14h+var_4]
                mov     [edi+4], ebx

loc_5C7D3C:                             ; CODE XREF: ___sbh_alloc_block+3F↑j
                add     edi, 8
                add     esi, 1000h
                cmp     edi, ebp
                jb      short loc_5C7D1B

loc_5C7D49:                             ; CODE XREF: ___sbh_alloc_block+39↑j
                mov     ebp, [ecx+8]
                mov     edi, [ecx+10h]
                lea     esi, [ecx+18h]
                cmp     esi, ebp
                jnb     short loc_5C7D84

loc_5C7D56:                             ; CODE XREF: ___sbh_alloc_block+A2↓j
                mov     eax, dword ptr [esi]
                cmp     eax, ebx
                jl      short loc_5C7D77
                cmp     [esi+4], ebx
                jbe     short loc_5C7D77
                push    ebx
                push    eax
                push    edi
                call    ___sbh_alloc_block_from_page
                add     esp, 0Ch
                test    eax, eax
                jnz     short loc_5C7DB1
                mov     ecx, dword ptr [esp+14h+var_4]
                mov     [esi+4], ebx

loc_5C7D77:                             ; CODE XREF: ___sbh_alloc_block+7A↑j
                add     esi, 8
                add     edi, 1000h
                cmp     esi, ebp
                jb      short loc_5C7D56

loc_5C7D84:                             ; CODE XREF: ___sbh_alloc_block+19↑j
                mov     ecx, dword ptr [ecx]
                mov     eax, __sbh_p_starting_region
                cmp     ecx, eax
                mov     [esp+14h+var_4], ecx
                jz      short loc_5C7DCA
                jmp     loc_5C7CF3
; ---------------------------------------------------------------------------

loc_5C7D98:                             ; CODE XREF: ___sbh_alloc_block+53↑j
                mov     ecx, dword ptr [esp+14h+var_4]
                mov     __sbh_p_starting_region, ecx
                mov     edx, dword ptr [edi]
                sub     edx, ebx
                mov     [edi], edx
                mov     [ecx+8], edi
                pop     edi
                pop     esi
                pop     ebp
                pop     ebx
                pop     ecx
                retn
; ---------------------------------------------------------------------------

loc_5C7DB1:                             ; CODE XREF: ___sbh_alloc_block+8E↑j
                mov     ecx, dword ptr [esp+14h+var_4]
                mov     __sbh_p_starting_region, ecx
                mov     edx, dword ptr [esi]
                sub     edx, ebx
                mov     [esi], edx
                mov     [ecx+8], esi
                pop     edi
                pop     esi
                pop     ebp
                pop     ebx
                pop     ecx
                retn
; ---------------------------------------------------------------------------

loc_5C7DCA:                             ; CODE XREF: ___sbh_alloc_block+B1↑j
                mov     ebp, offset __small_block_heap
                or      ecx, 0FFFFFFFFh

loc_5C7DD2:                             ; CODE XREF: ___sbh_alloc_block+10D↓j
                cmp     [ebp+10h], ecx
                jz      short loc_5C7DDE
                mov     eax, dword ptr [ebp+0Ch]
                test    eax, eax
                jnz     short loc_5C7DEF

loc_5C7DDE:                             ; CODE XREF: ___sbh_alloc_block+F5↑j
                mov     ebp, [ebp+0]
                cmp     ebp, offset __small_block_heap
                jz      loc_5C7ECD
                jmp     short loc_5C7DD2
; ---------------------------------------------------------------------------

loc_5C7DEF:                             ; CODE XREF: ___sbh_alloc_block+FC↑j
                mov     eax, dword ptr [ebp+0Ch]
                mov     esi, [ebp+10h]
                mov     edi, eax
                mov     [esp+14h+arg_0], eax
                sub     edi, ebp
                mov     edx, dword ptr [eax]
                sub     edi, 18h
                sar     edi, 3
                shl     edi, 0Ch
                add     edi, esi
                xor     esi, esi
                cmp     edx, ecx
                jnz     short loc_5C7E20

loc_5C7E10:                             ; CODE XREF: ___sbh_alloc_block+13E↓j
                cmp     esi, 10h
                jge     short loc_5C7E20
                mov     edx, dword ptr [eax+8]
                add     eax, 8
                inc     esi
                cmp     edx, ecx
                jz      short loc_5C7E10

loc_5C7E20:                             ; CODE XREF: ___sbh_alloc_block+12E↑j
                mov     eax, esi
                push    4               ; flProtect
                shl     eax, 0Ch
                push    1000h           ; flAllocationType
                push    eax             ; dwSize
                push    edi             ; lpAddress
                mov     [esp+24h+var_4], eax
                call    VirtualAlloc
                cmp     eax, edi
                jnz     loc_5C7F0B
                mov     edx, dword ptr [esp+14h+arg_0]
                mov     eax, dword ptr [esp+14h+var_4]
                xor     ecx, ecx
                test    esi, esi
                mov     ecx, edx
                jle     short loc_5C7E82
                lea     eax, [edi+4]

loc_5C7E53:                             ; CODE XREF: ___sbh_alloc_block+19C↓j
                lea     edx, [eax+4]
                mov     dword ptr [eax], 0F0h
                mov     [eax-4], edx
                mov     byte ptr [eax+0F4h], 0FFh
                mov     dword ptr [ecx], 0F0h
                mov     dword ptr [ecx+4], 0F1h
                add     eax, 1000h
                add     ecx, 8
                dec     esi
                jnz     short loc_5C7E53
                mov     edx, dword ptr [esp+14h+arg_0]

loc_5C7E82:                             ; CODE XREF: ___sbh_alloc_block+16E↑j
                lea     eax, [ebp+2018h]
                mov     __sbh_p_starting_region, ebp
                cmp     ecx, eax
                jnb     short loc_5C7EA0

loc_5C7E92:                             ; CODE XREF: ___sbh_alloc_block+1BC↓j
                cmp     dword ptr [ecx], 0FFFFFFFFh
                jz      short loc_5C7E9E
                add     ecx, 8
                cmp     ecx, eax
                jb      short loc_5C7E92

loc_5C7E9E:                             ; CODE XREF: ___sbh_alloc_block+1B5↑j
                cmp     ecx, eax

loc_5C7EA0:                             ; CODE XREF: ___sbh_alloc_block+1B0↑j
                sbb     eax, eax
                and     eax, ecx
                mov     [ebp+0Ch], eax
                mov     [edi+8], bl
                mov     [ebp+8], edx
                mov     ecx, dword ptr [edx]
                sub     ecx, ebx
                mov     [edx], ecx
                mov     eax, dword ptr [edi+4]
                sub     eax, ebx
                lea     ecx, [edi+ebx+8]
                mov     [edi+4], eax
                mov     [edi], ecx
                lea     eax, [edi+100h]
                pop     edi
                pop     esi
                pop     ebp
                pop     ebx
                pop     ecx
                retn
; ---------------------------------------------------------------------------

loc_5C7ECD:                             ; CODE XREF: ___sbh_alloc_block+107↑j
                call    ___sbh_new_region
                test    eax, eax
                jz      short loc_5C7F0B
                mov     ecx, dword ptr [eax+10h]
                mov     [ecx+8], bl
                lea     edx, [ecx+ebx+8]
                mov     __sbh_p_starting_region, eax
                mov     [ecx], edx
                mov     edx, 0F0h
                sub     edx, ebx
                and     ebx, 0FFh
                mov     [ecx+4], edx
                mov     edx, dword ptr [eax+18h]
                sub     edx, ebx
                mov     [eax+18h], edx
                lea     eax, [ecx+100h]
                pop     edi
                pop     esi
                pop     ebp
                pop     ebx
                pop     ecx
                retn
; ---------------------------------------------------------------------------

loc_5C7F0B:                             ; CODE XREF: ___sbh_alloc_block+15A↑j
                pop     edi
                pop     esi
                pop     ebp
                xor     eax, eax
                pop     ebx
                pop     ecx
                retn
___sbh_alloc_block endp

; ---------------------------------------------------------------------------
                align 10h

; =============== S U B R O U T I N E =======================================

; Attributes: library function

___sbh_alloc_block_from_page proc near  ; CODE XREF: ___sbh_alloc_block+49↑p

arg_0           = dword ptr  4
arg_4           = dword ptr  8
arg_8           = dword ptr  0Ch

                mov     edx, dword ptr [esp+arg_8]
                push    ebx
                push    ebp
                push    esi
                push    edi
                mov     edi, [esp+10h+arg_0]
                mov     eax, dword ptr [edi+4]
                mov     ecx, dword ptr [edi]
                cmp     eax, edx
                mov     [esp+10h+arg_0], ecx
                mov     esi, ecx
                lea     ebx, [edi+0F8h]
                jb      short loc_5C7F7B
                lea     eax, [ecx+edx]
                mov     [ecx], dl
                cmp     eax, ebx
                jnb     short loc_5C7F5A
                mov     esi, [edi]
                mov     eax, dword ptr [edi+4]
                add     esi, edx
                sub     eax, edx
                mov     [edi], esi
                mov     [edi+4], eax
                jmp     short loc_5C7F66
; ---------------------------------------------------------------------------

loc_5C7F5A:                             ; CODE XREF: ___sbh_alloc_block_from_page+28↑j
                lea     edx, [edi+8]
                mov     dword ptr [edi+4], 0
                mov     [edi], edx

loc_5C7F66:                             ; CODE XREF: ___sbh_alloc_block_from_page+38↑j
                lea     eax, [edi+edi*2]
                lea     eax, [eax+eax*4]
                mov     edx, eax
                lea     eax, [ecx+8]
                shl     eax, 4
                sub     eax, edx
                pop     edi
                pop     esi
                pop     ebp
                pop     ebx
                retn
; ---------------------------------------------------------------------------

loc_5C7F7B:                             ; CODE XREF: ___sbh_alloc_block_from_page+1F↑j
                add     eax, ecx
                cmp     byte ptr [eax], 0
                jz      short loc_5C7F84
                mov     esi, eax

loc_5C7F84:                             ; CODE XREF: ___sbh_alloc_block_from_page+60↑j
                lea     eax, [esi+edx]
                cmp     eax, ebx
                mov     ebx, dword ptr [esp+10h+arg_4]
                jnb     short loc_5C8004

loc_5C7F8F:                             ; CODE XREF: ___sbh_alloc_block_from_page+C3↓j
                mov     al, [esi]
                test    al, al
                jnz     short loc_5C7FD1
                cmp     byte ptr [esi+1], 0
                lea     eax, [esi+1]
                mov     ecx, 1
                jnz     short loc_5C7FAA

loc_5C7FA3:                             ; CODE XREF: ___sbh_alloc_block_from_page+88↓j
                inc     eax
                inc     ecx
                cmp     byte ptr [eax], 0
                jz      short loc_5C7FA3

loc_5C7FAA:                             ; CODE XREF: ___sbh_alloc_block_from_page+81↑j
                cmp     ecx, edx
                jnb     short loc_5C7FE7
                mov     ebp, [esp+10h+arg_0]
                cmp     esi, ebp
                jnz     short loc_5C7FBF
                mov     [edi+4], ecx
                mov     esi, eax
                mov     ecx, ebp
                jmp     short loc_5C7FD8
; ---------------------------------------------------------------------------

loc_5C7FBF:                             ; CODE XREF: ___sbh_alloc_block_from_page+94↑j
                sub     ebx, ecx
                cmp     ebx, edx
                jb      loc_5C808B
                mov     ecx, dword ptr [esp+10h+arg_0]
                mov     esi, eax
                jmp     short loc_5C7FD8
; ---------------------------------------------------------------------------

loc_5C7FD1:                             ; CODE XREF: ___sbh_alloc_block_from_page+73↑j
                and     eax, 0FFh
                add     esi, eax

loc_5C7FD8:                             ; CODE XREF: ___sbh_alloc_block_from_page+9D↑j
                lea     ebp, [esi+edx]
                lea     eax, [edi+0F8h]
                cmp     ebp, eax
                jb      short loc_5C7F8F
                jmp     short loc_5C8004
; ---------------------------------------------------------------------------

loc_5C7FE7:                             ; CODE XREF: ___sbh_alloc_block_from_page+8C↑j
                lea     eax, [esi+edx]
                lea     ebx, [edi+0F8h]
                cmp     eax, ebx
                jnb     short loc_5C7FFD
                sub     ecx, edx
                mov     [edi], eax
                mov     [edi+4], ecx
                jmp     short loc_5C8076
; ---------------------------------------------------------------------------

loc_5C7FFD:                             ; CODE XREF: ___sbh_alloc_block_from_page+D2↑j
                lea     ecx, [edi+8]
                mov     [edi], ecx
                jmp     short loc_5C806F
; ---------------------------------------------------------------------------

loc_5C8004:                             ; CODE XREF: ___sbh_alloc_block_from_page+6D↑j
                lea     ebp, [edi+8]
                mov     esi, ebp
                cmp     esi, ecx
                jnb     short loc_5C808B

loc_5C800D:                             ; CODE XREF: ___sbh_alloc_block_from_page+12E↓j
                lea     ecx, [esi+edx]
                lea     eax, [edi+0F8h]
                cmp     ecx, eax
                jnb     short loc_5C808B
                mov     al, [esi]
                test    al, al
                jnz     short loc_5C8043
                cmp     byte ptr [esi+1], 0
                lea     eax, [esi+1]
                mov     ecx, 1
                jnz     short loc_5C8035

loc_5C802E:                             ; CODE XREF: ___sbh_alloc_block_from_page+113↓j
                inc     eax
                inc     ecx
                cmp     byte ptr [eax], 0
                jz      short loc_5C802E

loc_5C8035:                             ; CODE XREF: ___sbh_alloc_block_from_page+10C↑j
                cmp     ecx, edx
                jnb     short loc_5C8057
                sub     ebx, ecx
                cmp     ebx, edx
                jb      short loc_5C808B
                mov     esi, eax
                jmp     short loc_5C804A
; ---------------------------------------------------------------------------

loc_5C8043:                             ; CODE XREF: ___sbh_alloc_block_from_page+FE↑j
                and     eax, 0FFh
                add     esi, eax

loc_5C804A:                             ; CODE XREF: ___sbh_alloc_block_from_page+121↑j
                cmp     esi, [esp+10h+arg_0]
                jb      short loc_5C800D
                xor     eax, eax
                pop     edi
                pop     esi
                pop     ebp
                pop     ebx
                retn
; ---------------------------------------------------------------------------

loc_5C8057:                             ; CODE XREF: ___sbh_alloc_block_from_page+117↑j
                lea     eax, [esi+edx]
                lea     ebx, [edi+0F8h]
                cmp     eax, ebx
                jnb     short loc_5C806D
                sub     ecx, edx
                mov     [edi], eax
                mov     [edi+4], ecx
                jmp     short loc_5C8076
; ---------------------------------------------------------------------------

loc_5C806D:                             ; CODE XREF: ___sbh_alloc_block_from_page+142↑j
                mov     [edi], ebp

loc_5C806F:                             ; CODE XREF: ___sbh_alloc_block_from_page+E2↑j
                mov     dword ptr [edi+4], 0

loc_5C8076:                             ; CODE XREF: ___sbh_alloc_block_from_page+DB↑j
                lea     eax, [edi+edi*2]
                mov     [esi], dl
                lea     edx, [eax+eax*4]
                lea     eax, [esi+8]
                shl     eax, 4
                sub     eax, edx
                pop     edi
                pop     esi
                pop     ebp
                pop     ebx
                retn
; ---------------------------------------------------------------------------

loc_5C808B:                             ; CODE XREF: ___sbh_alloc_block_from_page+A3↑j
                pop     edi
                pop     esi
                pop     ebp
                xor     eax, eax
                pop     ebx
                retn
___sbh_alloc_block_from_page endp

; ---------------------------------------------------------------------------
                align 10h

; =============== S U B R O U T I N E =======================================

; Attributes: library function

___sbh_resize_block proc near           ; CODE XREF: _realloc+AB↑p

arg_0           = dword ptr  4
arg_4           = dword ptr  8
arg_8           = dword ptr  0Ch
arg_C           = dword ptr  10h

                mov     ecx, dword ptr [esp+arg_0]
                push    ebx
                push    ebp
                mov     ebp, [esp+8+arg_4]
                push    esi
                push    edi
                mov     edi, [ecx+10h]
                mov     edx, ebp
                sub     edx, edi
                mov     edi, [esp+10h+arg_8]
                sar     edx, 0Ch
                mov     ebx, dword ptr [esp+10h+arg_C]
                xor     eax, eax
                lea     ecx, [ecx+edx*8+18h]
                xor     edx, edx
                mov     dl, [edi]
                mov     [esp+10h+arg_4], ecx
                mov     esi, edx
                cmp     esi, ebx
                jbe     short loc_5C80ED
                mov     [edi], bl
                mov     eax, dword ptr [ecx]
                sub     esi, ebx
                mov     dword ptr [ecx+4], 0F1h
                add     eax, esi
                mov     [ecx], eax
                mov     eax, 1
                pop     edi
                pop     esi
                pop     ebp
                pop     ebx
                retn
; ---------------------------------------------------------------------------

loc_5C80ED:                             ; CODE XREF: ___sbh_resize_block+30↑j
                jnb     short loc_5C815F
                lea     ecx, [ebx+edi]
                lea     edx, [ebp+0F8h]
                cmp     ecx, edx
                ja      short loc_5C815F
                lea     edx, [esi+edi]
                cmp     edx, ecx
                jnb     short loc_5C810F

loc_5C8103:                             ; CODE XREF: ___sbh_resize_block+6B↓j
                cmp     byte ptr [edx], 0
                jnz     short loc_5C810D
                inc     edx
                cmp     edx, ecx
                jb      short loc_5C8103

loc_5C810D:                             ; CODE XREF: ___sbh_resize_block+66↑j
                cmp     edx, ecx

loc_5C810F:                             ; CODE XREF: ___sbh_resize_block+61↑j
                jnz     short loc_5C815F
                mov     [edi], bl
                mov     eax, dword ptr [ebp+0]
                cmp     edi, eax
                ja      short loc_5C814E
                cmp     ecx, eax
                jbe     short loc_5C814E
                lea     eax, [ebp+0F8h]
                cmp     ecx, eax
                jnb     short loc_5C8141
                mov     [ebp+0], ecx
                mov     dl, [ecx]
                xor     eax, eax
                test    dl, dl
                jnz     short loc_5C813C

loc_5C8133:                             ; CODE XREF: ___sbh_resize_block+9A↓j
                mov     dl, [eax+ecx+1]
                inc     eax
                test    dl, dl
                jz      short loc_5C8133

loc_5C813C:                             ; CODE XREF: ___sbh_resize_block+91↑j
                mov     [ebp+4], eax
                jmp     short loc_5C814E
; ---------------------------------------------------------------------------

loc_5C8141:                             ; CODE XREF: ___sbh_resize_block+86↑j
                lea     eax, [ebp+8]
                mov     dword ptr [ebp+4], 0
                mov     [ebp+0], eax

loc_5C814E:                             ; CODE XREF: ___sbh_resize_block+78↑j
                mov     eax, dword ptr [esp+10h+arg_4]
                sub     esi, ebx
                mov     ecx, dword ptr [eax]
                add     ecx, esi
                mov     [eax], ecx
                mov     eax, 1

loc_5C815F:                             ; CODE XREF: ___sbh_resize_block:loc_5C80ED↑j
                pop     edi
                pop     esi
                pop     ebp
                pop     ebx
                retn
___sbh_resize_block endp

; ---------------------------------------------------------------------------
                align 10h

; =============== S U B R O U T I N E =======================================

; Attributes: library function

___sbh_heap_check proc near

var_1C          = dword ptr -1Ch
var_18          = dword ptr -18h
var_14          = dword ptr -14h
var_10          = dword ptr -10h
var_C           = dword ptr -0Ch
var_8           = dword ptr -8
var_4           = dword ptr -4

                sub     esp, 1Ch
                push    ebx
                push    ebp
                push    esi
                push    edi
                xor     edi, edi
                mov     edx, offset __small_block_heap
                mov     [esp+2Ch+var_C], edi
                mov     [esp+2Ch+var_10], edx

loc_5C8186:                             ; CODE XREF: ___sbh_heap_check+14D↓j
                cmp     __sbh_p_starting_region, edx
                jnz     short loc_5C8192
                inc     [esp+2Ch+var_C]

loc_5C8192:                             ; CODE XREF: ___sbh_heap_check+1C↑j
                mov     ecx, dword ptr [edx+10h]
                cmp     ecx, 0FFFFFFFFh
                jz      loc_5C82AF
                mov     ebx, ecx
                lea     eax, [edx+18h]
                mov     [esp+2Ch+var_8], edi
                mov     [esp+2Ch+var_1C], edi
                lea     ebp, [ebx+8]
                mov     [esp+2Ch+var_4], eax

loc_5C81B2:                             ; CODE XREF: ___sbh_heap_check+139↓j
                cmp     dword ptr [eax], 0FFFFFFFFh
                jnz     short loc_5C81CF
                cmp     [esp+2Ch+var_1C], edi
                jnz     short loc_5C81C6
                cmp     [edx+0Ch], eax
                jnz     loc_5C82C2

loc_5C81C6:                             ; CODE XREF: ___sbh_heap_check+4B↑j
                inc     [esp+2Ch+var_1C]
                jmp     loc_5C8287
; ---------------------------------------------------------------------------

loc_5C81CF:                             ; CODE XREF: ___sbh_heap_check+45↑j
                mov     edx, dword ptr [ebx]
                lea     ecx, [ebp+0F0h]
                cmp     edx, ecx
                jnb     loc_5C82CD
                cmp     byte ptr [ecx], 0FFh
                jnz     loc_5C82DA
                xor     edx, edx
                xor     ecx, ecx
                mov     [esp+2Ch+var_18], ecx
                mov     [esp+2Ch+var_14], edi

loc_5C81F4:                             ; CODE XREF: ___sbh_heap_check+F9↓j
                lea     esi, [edx+ebp]
                cmp     esi, [ebx]
                jnz     short loc_5C8200
                inc     ecx
                mov     [esp+2Ch+var_18], ecx

loc_5C8200:                             ; CODE XREF: ___sbh_heap_check+89↑j
                cmp     byte ptr [esi], 0
                jnz     short loc_5C8212
                mov     esi, [esp+2Ch+var_14]
                inc     esi
                inc     edi
                mov     [esp+2Ch+var_14], esi
                inc     edx
                jmp     short loc_5C8263
; ---------------------------------------------------------------------------

loc_5C8212:                             ; CODE XREF: ___sbh_heap_check+93↑j
                cmp     edi, [eax+4]
                jge     loc_5C82E7
                cmp     ecx, 1
                jnz     short loc_5C8231
                cmp     edi, [ebp-4]
                jl      loc_5C82F4
                mov     [esp+2Ch+var_18], 2

loc_5C8231:                             ; CODE XREF: ___sbh_heap_check+AE↑j
                xor     eax, eax
                lea     ecx, [edx+1]
                mov     al, [esi]
                mov     esi, edx
                add     esi, eax
                xor     edi, edi
                cmp     ecx, esi
                jge     short loc_5C8259

loc_5C8242:                             ; CODE XREF: ___sbh_heap_check+E7↓j
                cmp     byte ptr [ecx+ebp], 0
                jnz     loc_5C8301
                xor     eax, eax
                inc     ecx
                mov     al, [edx+ebx+8]
                add     eax, edx
                cmp     ecx, eax
                jl      short loc_5C8242

loc_5C8259:                             ; CODE XREF: ___sbh_heap_check+D0↑j
                mov     eax, dword ptr [esp+2Ch+var_4]
                mov     edx, ecx
                mov     ecx, dword ptr [esp+2Ch+var_18]

loc_5C8263:                             ; CODE XREF: ___sbh_heap_check+A0↑j
                cmp     edx, 0F0h
                jl      short loc_5C81F4
                mov     edx, dword ptr [esp+2Ch+var_14]
                mov     esi, [eax]
                cmp     edx, esi
                jnz     loc_5C830E
                test    ecx, ecx
                jz      loc_5C831B
                mov     edx, dword ptr [esp+2Ch+var_10]
                xor     edi, edi

loc_5C8287:                             ; CODE XREF: ___sbh_heap_check+5A↑j
                mov     ecx, dword ptr [esp+2Ch+var_8]
                add     eax, 8
                inc     ecx
                add     ebx, 1000h
                add     ebp, 1000h
                cmp     ecx, 400h
                mov     [esp+2Ch+var_8], ecx
                mov     [esp+2Ch+var_4], eax
                jl      loc_5C81B2

loc_5C82AF:                             ; CODE XREF: ___sbh_heap_check+28↑j
                mov     edx, dword ptr [edx]
                cmp     edx, offset __small_block_heap
                mov     [esp+2Ch+var_10], edx
                jz      short loc_5C8328
                jmp     loc_5C8186
; ---------------------------------------------------------------------------

loc_5C82C2:                             ; CODE XREF: ___sbh_heap_check+50↑j
                or      eax, 0FFFFFFFFh
                pop     edi
                pop     esi
                pop     ebp
                pop     ebx
                add     esp, 1Ch
                retn
; ---------------------------------------------------------------------------

loc_5C82CD:                             ; CODE XREF: ___sbh_heap_check+69↑j
                mov     eax, 0FFFFFFFEh
                pop     edi
                pop     esi
                pop     ebp
                pop     ebx
                add     esp, 1Ch
                retn
; ---------------------------------------------------------------------------

loc_5C82DA:                             ; CODE XREF: ___sbh_heap_check+72↑j
                mov     eax, 0FFFFFFFDh
                pop     edi
                pop     esi
                pop     ebp
                pop     ebx
                add     esp, 1Ch
                retn
; ---------------------------------------------------------------------------

loc_5C82E7:                             ; CODE XREF: ___sbh_heap_check+A5↑j
                mov     eax, 0FFFFFFFCh
                pop     edi
                pop     esi
                pop     ebp
                pop     ebx
                add     esp, 1Ch
                retn
; ---------------------------------------------------------------------------

loc_5C82F4:                             ; CODE XREF: ___sbh_heap_check+B3↑j
                mov     eax, 0FFFFFFFBh
                pop     edi
                pop     esi
                pop     ebp
                pop     ebx
                add     esp, 1Ch
                retn
; ---------------------------------------------------------------------------

loc_5C8301:                             ; CODE XREF: ___sbh_heap_check+D6↑j
                mov     eax, 0FFFFFFFAh
                pop     edi
                pop     esi
                pop     ebp
                pop     ebx
                add     esp, 1Ch
                retn
; ---------------------------------------------------------------------------

loc_5C830E:                             ; CODE XREF: ___sbh_heap_check+103↑j
                mov     eax, 0FFFFFFF9h
                pop     edi
                pop     esi
                pop     ebp
                pop     ebx
                add     esp, 1Ch
                retn
; ---------------------------------------------------------------------------

loc_5C831B:                             ; CODE XREF: ___sbh_heap_check+10B↑j
                mov     eax, 0FFFFFFF8h
                pop     edi
                pop     esi
                pop     ebp
                pop     ebx
                add     esp, 1Ch
                retn
; ---------------------------------------------------------------------------

loc_5C8328:                             ; CODE XREF: ___sbh_heap_check+14B↑j
                mov     eax, dword ptr [esp+2Ch+var_C]
                pop     edi
                neg     eax
                sbb     eax, eax
                pop     esi
                and     eax, 9
                pop     ebp
                add     eax, 0FFFFFFF7h
                pop     ebx
                add     esp, 1Ch
                retn
___sbh_heap_check endp

; ---------------------------------------------------------------------------

; =============== S U B R O U T I N E =======================================

; Attributes: library function

; int __cdecl _callnewh(size_t varSize)
__callnewh      proc near               ; CODE XREF: __nh_malloc+33↑p

varSize            = dword ptr  4

                mov     eax, _pnhHeap
                test    eax, eax
                jz      short loc_5C767D
                mov     ecx, dword ptr [esp+varSize]
                push    ecx
                call    eax ; _pnhHeap
                add     esp, 4
                test    eax, eax
                jz      short loc_5C767D
                mov     eax, 1
                retn
; ---------------------------------------------------------------------------

loc_5C767D:                             ; CODE XREF: __callnewh+7↑j
                xor     eax, eax
                retn
__callnewh      endp


; =============== S U B R O U T I N E =======================================

; Attributes: library function

__heap_alloc    proc near               ; CODE XREF: __nh_malloc+1E↑p

arg_0           = dword ptr  4

                mov     eax, dword ptr [esp+arg_0]
                push    esi
                push    edi
                lea     esi, [eax+0Fh]
                mov     eax, __sbh_threshold
                and     esi, 0FFFFFFF0h
                cmp     esi, eax
                ja      short loc_5BF062
                push    9
                call    __lock
                mov     ecx, esi
                add     esp, 4
                shr     ecx, 4
                push    ecx
                call    ___sbh_alloc_block
                add     esp, 4
                mov     edi, eax
                push    9
                call    __unlock
                add     esp, 4
                test    edi, edi
                jz      short loc_5BF062
                mov     eax, edi
                pop     edi
                pop     esi
                retn
; ---------------------------------------------------------------------------

loc_5BF062:                             ; CODE XREF: __heap_alloc+13↑j
                mov     edx, _crtheap
                push    esi             ; dwBytes
                push    0               ; dwFlags
                push    edx             ; _crtheap
                call    HeapAlloc
                pop     edi
                pop     esi
                retn
__heap_alloc    endp

; ---------------------------------------------------------------------------
                align 10h


; =============== S U B R O U T I N E =======================================

; Attributes: library function

__heap_term     proc near
                push    esi
                push    edi
                mov     edi, ds:VirtualFree
                mov     esi, offset __small_block_heap

loc_5C76CD:                             ; CODE XREF: __heap_term+26↓j
                mov     eax, dword ptr [esi+10h]
                test    eax, eax
                jz      short loc_5C76DE
                push    8000h           ; dwFreeType
                push    0               ; dwSize
                push    eax             ; lpAddress
                call    edi ; VirtualFree

loc_5C76DE:                             ; CODE XREF: __heap_term+12↑j
                mov     esi, [esi]
                cmp     esi, offset __small_block_heap
                jnz     short loc_5C76CD
                mov     eax, _crtheap
                push    eax             ; _crtheap
                call    HeapDestroy
                pop     edi
                pop     esi
                retn
__heap_term     endp

; ---------------------------------------------------------------------------
                align 10h


; =============== S U B R O U T I N E =======================================

; Attributes: library function noreturn

__amsg_exit     proc near               ; CODE XREF: ___onexitinit+18↑p

arg_0           = dword ptr  4

                cmp     __error_mode, 1
                jnz     short loc_5C1D5E
                call    __FF_MSGBANNER

loc_5C1D5E:                             ; CODE XREF: __amsg_exit+7↑j
                mov     eax, dword ptr [esp+arg_0]
                push    eax
                call    __NMSG_WRITE
                add     esp, 4
                push    0FFh            ; Code
                call    _aexit_rtn
                add     esp, 4
                retn
__amsg_exit     endp
; ---------------------------------------------------------------------------
                align 10h


; =============== S U B R O U T I N E =======================================

; Attributes: library function

__FF_MSGBANNER  proc near               ; CODE XREF: __amsg_exit+9↑p
                mov     eax, __error_mode
                cmp     eax, 1
                jz      short loc_5CC617
                test    eax, eax
                jnz     short locret_5CC63C
                cmp     __app_type, 1
                jnz     short locret_5CC63C

loc_5CC617:                             ; CODE XREF: __FF_MSGBANNER+8↑j
                push    0FCh
                call    __NMSG_WRITE
                mov     eax, _adbgmsg
                add     esp, 4
                test    eax, eax
                jz      short loc_5CC62F
                call    eax ; _adbgmsg

loc_5CC62F:                             ; CODE XREF: __FF_MSGBANNER+2B↑j
                push    0FFh
                call    __NMSG_WRITE
                add     esp, 4

locret_5CC63C:                          ; CODE XREF: __FF_MSGBANNER+C↑j
                retn
__FF_MSGBANNER  endp

; ---------------------------------------------------------------------------
                align 10h

; =============== S U B R O U T I N E =======================================

; Attributes: library function

__NMSG_WRITE    proc near               ; CODE XREF: _abort+2↑p

NumberOfBytesWritten= dword ptr -1A8h
var_1A4         = byte ptr -1A4h
Filename        = byte ptr -104h
arg_0           = dword ptr  4

                mov     ecx, dword ptr [esp+arg_0]
                sub     esp, 1A8h
                mov     eax, offset rterrs
                push    ebx
                push    ebp
                push    esi
                push    edi
                xor     ebp, ebp

loc_5CC655:                             ; CODE XREF: __NMSG_WRITE+22↓j
                cmp     ecx, [eax]
                jz      short loc_5CC664
                add     eax, 8
                inc     ebp
                cmp     eax, offset rterrs+90h ;end of rterrs
                jb      short loc_5CC655

loc_5CC664:                             ; CODE XREF: __NMSG_WRITE+17↑j
                cmp     ecx, rterrs[ebp*8]
                jnz     loc_5CC80B
                mov     eax, __error_mode
                cmp     eax, 1
                jz      loc_5CC7CD
                test    eax, eax
                jnz     short loc_5CC690
                cmp     __app_type, 1
                jz      loc_5CC7CD

loc_5CC690:                             ; CODE XREF: __NMSG_WRITE+41↑j
                cmp     ecx, 0FCh
                jz      loc_5CC80B
                lea     eax, [esp+1B8h+Filename]
                push    104h            ; nSize
                push    eax             ; lpFilename
                push    0               ; hModule
                call    GetModuleFileNameA
                test    eax, eax
                jnz     short loc_5CC6CB
                mov     ecx, 5
                mov     esi, offset aProgramNameUnk ; "<program name unknown>"
                lea     edi, [esp+1B8h+Filename]
                rep movsd
                movsw
                movsb

loc_5CC6CB:                             ; CODE XREF: __NMSG_WRITE+73↑j
                lea     edi, [esp+1B8h+Filename]
                or      ecx, 0FFFFFFFFh
                xor     eax, eax
                lea     ebx, [esp+1B8h+Filename]
                repne scasb
                not     ecx
                cmp     ecx, 3Ch ; '<'
                jbe     short loc_5CC714
                lea     edi, [esp+1B8h+Filename]
                or      ecx, 0FFFFFFFFh
                repne scasb
                not     ecx
                dec     ecx
                push    3               ; Count
                mov     ebx, ecx
                lea     ecx, [esp+1BCh+Filename]
                sub     ecx, 3Bh ; ';'
                push    offset asc_614AAC ; "..."
                add     ebx, ecx
                push    ebx             ; Destination
                call    _strncpy
                add     esp, 0Ch

loc_5CC714:                             ; CODE XREF: __NMSG_WRITE+A5↑j
                mov     ecx, 6
                mov     esi, offset aRuntimeErrorPr ; "Runtime Error!\n\nProgram: "
                lea     edi, [esp+1B8h+var_1A4]
                xor     eax, eax
                rep movsd
                movsw
                or      ecx, 0FFFFFFFFh
                mov     edi, ebx
                repne scasb
                not     ecx
                sub     edi, ecx
                lea     edx, [esp+1B8h+var_1A4]
                mov     ebx, ecx
                mov     esi, edi
                or      ecx, 0FFFFFFFFh
                mov     edi, edx
                repne scasb
                mov     ecx, ebx
                dec     edi
                shr     ecx, 2
                rep movsd
                mov     ecx, ebx
                lea     edx, [esp+1B8h+var_1A4]
                and     ecx, 3
                push    12010h
                rep movsb
                mov     edi, offset asc_614A8C ; "\n\n"
                or      ecx, 0FFFFFFFFh
                repne scasb
                not     ecx
                sub     edi, ecx
                push    offset aMicrosoftVisua ; "Microsoft Visual C++ Runtime Library"
                mov     esi, edi
                mov     ebx, ecx
                mov     edi, edx
                or      ecx, 0FFFFFFFFh
                repne scasb
                mov     ecx, ebx
                dec     edi
                shr     ecx, 2
                rep movsd
                mov     ecx, ebx
                lea     edx, [esp+1C0h+var_1A4]
                and     ecx, 3
                rep movsb
                mov     edi, rterrs+4h[ebp*8]
                or      ecx, 0FFFFFFFFh
                repne scasb
                not     ecx
                sub     edi, ecx
                mov     esi, edi
                mov     ebx, ecx
                mov     edi, edx
                or      ecx, 0FFFFFFFFh
                repne scasb
                mov     ecx, ebx
                dec     edi
                shr     ecx, 2
                rep movsd
                mov     ecx, ebx
                lea     eax, [esp+1C0h+var_1A4]
                and     ecx, 3
                push    eax
                rep movsb
                call    ___crtMessageBoxA
                add     esp, 0Ch
                pop     edi
                pop     esi
                pop     ebp
                pop     ebx
                add     esp, 1A8h
                retn
; ---------------------------------------------------------------------------

loc_5CC7CD:                             ; CODE XREF: __NMSG_WRITE+39↑j
                mov     eax, __pioinfo
                test    eax, eax
                jz      short loc_5CC7DE
                mov     esi, [eax+48h]
                cmp     esi, 0FFFFFFFFh
                jnz     short loc_5CC7E8

loc_5CC7DE:                             ; CODE XREF: __NMSG_WRITE+194↑j
                push    0FFFFFFF4h      ; nStdHandle
                call    GetStdHandle
                mov     esi, eax

loc_5CC7E8:                             ; CODE XREF: __NMSG_WRITE+19C↑j
                mov     edx, rterrs+4h[ebp*8]
                lea     ecx, [esp+1B8h+NumberOfBytesWritten]
                push    0               ; lpOverlapped
                push    ecx             ; lpNumberOfBytesWritten
                mov     edi, edx
                or      ecx, 0FFFFFFFFh
                xor     eax, eax
                repne scasb
                not     ecx
                dec     ecx
                push    ecx             ; nNumberOfBytesToWrite
                push    edx             ; lpBuffer
                push    esi             ; hFile
                call    WriteFile

loc_5CC80B:                             ; CODE XREF: __NMSG_WRITE+2B↑j
                pop     edi
                pop     esi
                pop     ebp
                pop     ebx
                add     esp, 1A8h
                retn
__NMSG_WRITE    endp

; ---------------------------------------------------------------------------
                align 10h



; =============== S U B R O U T I N E =======================================

; Attributes: library function

___crtMessageBoxA proc near             ; CODE XREF: __NMSG_WRITE+17A↑p

arg_0           = dword ptr  4
arg_4           = dword ptr  8
arg_8           = dword ptr  0Ch

                mov     eax, pfnMessageBoxA
                push    ebx
                xor     ebx, ebx
                push    esi
                test    eax, eax
                push    edi
                jnz     short loc_5D3380
                push    offset LibFileName ; "user32.dll"
                call    LoadLibraryA
                mov     esi, eax
                test    esi, esi
                jz      short loc_5D33B9
                mov     edi, ds:GetProcAddress
                push    offset aMessageboxa ; "MessageBoxA"
                push    esi             ; hModule
                call    edi ; GetProcAddress
                test    eax, eax
                mov     pfnMessageBoxA, eax
                jz      short loc_5D33B9
                push    offset aGetactivewindo ; "GetActiveWindow"
                push    esi             ; hModule
                call    edi ; GetProcAddress
                push    offset aGetlastactivep ; "GetLastActivePopup"
                push    esi             ; hModule
                mov     pfnGetActiveWindow, eax
                call    edi ; GetProcAddress
                mov     pfnGetLastActivePopup, eax

loc_5D3380:                             ; CODE XREF: ___crtMessageBoxA+C↑j
                mov     eax, pfnGetActiveWindow
                test    eax, eax
                jz      short loc_5D338D
                call    eax ; pfnGetActiveWindow
                mov     ebx, eax

loc_5D338D:                             ; CODE XREF: ___crtMessageBoxA+57↑j
                test    ebx, ebx
                jz      short loc_5D339F
                mov     eax, pfnGetLastActivePopup
                test    eax, eax
                jz      short loc_5D339F
                push    ebx
                call    eax ; pfnGetLastActivePopup
                mov     ebx, eax

loc_5D339F:                             ; CODE XREF: ___crtMessageBoxA+5F↑j
                mov     eax, dword ptr [esp+0Ch+arg_8]
                mov     ecx, dword ptr [esp+0Ch+arg_4]
                mov     edx, dword ptr [esp+0Ch+arg_0]
                push    eax
                push    ecx
                push    edx
                push    ebx
                call    pfnMessageBoxA
                pop     edi
                pop     esi
                pop     ebx
                retn
; ---------------------------------------------------------------------------

loc_5D33B9:                             ; CODE XREF: ___crtMessageBoxA+1D↑j
                pop     edi
                pop     esi
                xor     eax, eax
                pop     ebx
                retn
___crtMessageBoxA endp

; ---------------------------------------------------------------------------
                align 10h


; =============== S U B R O U T I N E =======================================

; Attributes: library function

; int (__cdecl *__cdecl _set_new_handler(int (__cdecl *)(unsigned int)))(unsigned int)
?_set_new_handler@@YAP6AHI@ZP6AHI@Z@Z proc near

arg_0           = dword ptr  4

                push    esi
                push    9
                call    __lock
                mov     eax, dword ptr [esp+8+arg_0]
                mov     esi, _pnhHeap
                add     esp, 4
                mov     _pnhHeap, eax
                push    9
                call    __unlock
                add     esp, 4
                mov     eax, esi
                pop     esi
                retn
?_set_new_handler@@YAP6AHI@ZP6AHI@Z@Z endp



; =============== S U B R O U T I N E =======================================

; Attributes: library function noreturn

; void __cdecl __noreturn _exit(int Code)
__exit          proc near               ; CODE XREF: sub_4823BD+997↑p

Code            = dword ptr  4

                mov     eax, dword ptr [esp+Code]
                push    0               ; int
                push    1               ; int
                push    eax             ; uExitCode
                call    _doexit
                add     esp, 0Ch
                retn
__exit          endp


; =============== S U B R O U T I N E =======================================

; Attributes: library function static

; int __cdecl doexit(UINT uExitCode, int, int)
_doexit         proc near               ; CODE XREF: _exit+9↑p

uExitCode       = dword ptr  4
arg_4           = dword ptr  8
arg_8           = dword ptr  0Ch

                push    ebx
                push    ebp
                push    esi
                call    __lockexit
                mov     eax, _C_Exit_Done
                mov     ebp, [esp+0Ch+uExitCode]
                cmp     eax, 1
                jnz     short loc_5C10D4
                push    ebp             ; uExitCode
                call    GetCurrentProcess
                push    eax             ; hProcess
                call    TerminateProcess

loc_5C10D4:                             ; CODE XREF: _doexit+14↑j
                mov     eax, dword ptr [esp+0Ch+arg_4]
                mov     ebx, dword ptr [esp+0Ch+arg_8]
                test    eax, eax
                mov     _C_Termination_Done, 1
                mov     _exitflag, bl
                jnz     short loc_5C112E
                mov     ecx, __onexitbegin
                test    ecx, ecx
                jz      short loc_5C111C
                mov     esi, __onexitend
                sub     esi, 4
                cmp     esi, ecx
                jb      short loc_5C111C

loc_5C1107:                             ; CODE XREF: _doexit+6A↓j
                mov     eax, dword ptr [esi]
                test    eax, eax
                jz      short loc_5C1115
                call    eax
                mov     ecx, __onexitbegin

loc_5C1115:                             ; CODE XREF: _doexit+5B↑j
                sub     esi, 4
                cmp     esi, ecx
                jnb     short loc_5C1107

loc_5C111C:                             ; CODE XREF: _doexit+48↑j
                push    offset __xp_z ; Last
                push    offset __xp_a ; First
                call    __initterm
                add     esp, 8

loc_5C112E:                             ; CODE XREF: _doexit+3E↑j
                push    offset __xt_z ; Last
                push    offset __xt_a ; First
                call    __initterm
                add     esp, 8
                test    ebx, ebx
                jz      short loc_5C114D
                call    __unlockexit
                pop     esi
                pop     ebp
                pop     ebx
                retn
; ---------------------------------------------------------------------------

loc_5C114D:                             ; CODE XREF: _doexit+92↑j
                push    ebp             ; uExitCode
                mov     _C_Exit_Done, 1
                call    ExitProcess
                pop     esi
                pop     ebp
                pop     ebx
                retn

_doexit         endp

; =============== S U B R O U T I N E =======================================

; Attributes: library function static

; void __cdecl _initterm(_PVFV *First, _PVFV *Last)
__initterm      proc near               ; CODE XREF: __cinit+15↑p

First           = dword ptr  4
Last            = dword ptr  8

                push    esi
                mov     esi, [esp+4+First]
                push    edi
                mov     edi, [esp+8+Last]
                cmp     esi, edi
                jnb     short loc_5C11AD

loc_5C119E:                             ; CODE XREF: __initterm+1B↓j
                mov     eax, dword ptr [esi]
                test    eax, eax
                jz      short loc_5C11A6
                call    eax

loc_5C11A6:                             ; CODE XREF: __initterm+12↑j
                add     esi, 4
                cmp     esi, edi
                jb      short loc_5C119E

loc_5C11AD:                             ; CODE XREF: __initterm+C↑j
                pop     edi
                pop     esi
                retn
__initterm      endp

; =============== S U B R O U T I N E =======================================

; Attributes: library function

__lockexit      proc near               ; CODE XREF: __onexit+1↑p
                push    0Dh
                call    __lock
                add     esp, 4
                retn
__lockexit      endp


; =============== S U B R O U T I N E =======================================

; Attributes: library function

__unlockexit    proc near               ; CODE XREF: __onexit+4B↑p
                push    0Dh
                call    __unlock
                add     esp, 4
                retn
__unlockexit    endp

; ----------------------------


; =============== S U B R O U T I N E =======================================

; Attributes: library function

; _onexit_t __cdecl _onexit(_onexit_t Func)
__onexit        proc near               ; CODE XREF: _atexit+5↓p

Func            = dword ptr  4

                push    esi
                call    __lockexit
                mov     eax, __onexitbegin
                push    eax             ; Block
                call    __msize
                mov     ecx, __onexitend
                mov     edx, __onexitbegin
                mov     esi, ecx
                add     esp, 4
                sub     esi, edx
                add     esi, 4
                cmp     eax, esi
                jnb     short loc_5BEEB3
                push    edx             ; Block
                call    __msize
                mov     ecx, __onexitbegin
                add     esp, 4
                add     eax, 10h
                push    eax             ; varSize
                push    ecx             ; Block
                call    _realloc
                add     esp, 8
                test    eax, eax
                jnz     short loc_5BEE94
                call    __unlockexit
                xor     eax, eax
                pop     esi
                retn
; ---------------------------------------------------------------------------

loc_5BEE94:                             ; CODE XREF: __onexit+49↑j
                mov     edx, __onexitend
                mov     esi, __onexitbegin
                sub     edx, esi
                mov     __onexitbegin, eax
                sar     edx, 2
                lea     ecx, [eax+edx*4]
                mov     __onexitend, ecx

loc_5BEEB3:                             ; CODE XREF: __onexit+29↑j
                mov     esi, [esp+4+Func]
                mov     [ecx], esi
                mov     eax, __onexitend
                add     eax, 4
                mov     __onexitend, eax
                call    __unlockexit
                mov     eax, esi
                pop     esi
                retn
__onexit        endp

; ---------------------------------------------------------------------------

; =============== S U B R O U T I N E =======================================

; Attributes: library function

___onexitinit   proc near               ; DATA XREF: .data:0062C240↓o
                push    80h             ; varSize
                call    _malloc
                add     esp, 4
                mov     __onexitbegin, eax
                test    eax, eax
                jnz     short loc_5BEF15
                push    18h
                call    __amsg_exit
; ---------------------------------------------------------------------------
                mov     eax, __onexitbegin
                add     esp, 4

loc_5BEF15:                             ; CODE XREF: ___onexitinit+14↑j
                mov     dword ptr [eax], 0
                mov     eax, __onexitbegin
                mov     __onexitend, eax
                retn
___onexitinit   endp

; ---------------------------------------------------------------------------


; =============== S U B R O U T I N E =======================================

; Attributes: library function

; size_t __cdecl _msize(void *Block)
__msize         proc near               ; CODE XREF: __onexit+C↑p

var_8           = byte ptr -8
var_4           = byte ptr -4
lpMem           = dword ptr  4

                sub     esp, 8
                push    esi
                push    9
                call    __lock
                mov     esi, [esp+10h+lpMem]
                add     esp, 4
                lea     eax, [esp+0Ch+var_8]
                lea     ecx, [esp+0Ch+var_4]
                push    eax
                push    ecx
                push    esi
                call    ___sbh_find_block
                add     esp, 0Ch
                test    eax, eax
                jz      short loc_5C60E3
                xor     edx, edx
                push    9
                mov     dl, [eax]
                mov     esi, edx
                shl     esi, 4
                call    __unlock
                add     esp, 4
                mov     eax, esi
                pop     esi
                add     esp, 8
                retn
; ---------------------------------------------------------------------------

loc_5C60E3:                             ; CODE XREF: __msize+27↑j
                push    9
                call    __unlock
                mov     eax, _crtheap
                add     esp, 4
                push    esi             ; lpMem
                push    0               ; dwFlags
                push    eax             ; _crtheap
                call    HeapSize
                mov     esi, eax
                pop     esi
                add     esp, 8
                retn
__msize         endp

; ---------------------------------------------------------------------------


; =============== S U B R O U T I N E =======================================

; Attributes: library function

__heap_init     proc near               ; CODE XREF: start+57↑p
                push    0               ; dwMaximumSize
                push    1000h           ; dwInitialSize
                push    0               ; flOptions
                call    HeapCreate
                test    eax, eax
                mov     _crtheap, eax
                jnz     short loc_5C7699
                retn
; ---------------------------------------------------------------------------

loc_5C7699:                             ; CODE XREF: __heap_init+16↑j
                call    ___sbh_new_region
                test    eax, eax
                jnz     short loc_5C76B1
                mov     eax, _crtheap
                push    eax             ; _crtheap
                call    HeapDestroy
                xor     eax, eax
                retn
; ---------------------------------------------------------------------------

loc_5C76B1:                             ; CODE XREF: __heap_init+20↑j
                mov     eax, 1
                retn
__heap_init     endp

; ---------------------------------------------------------------------------



; =============== S U B R O U T I N E =======================================

; Attributes: library function

__mtinitlocks   proc near               ; CODE XREF: __mtinit+1↑p
                mov     eax, [_locktable+44h]
                push    esi
                mov     esi, ds:InitializeCriticalSection
                push    eax             ; lpCriticalSection
                call    esi ; InitializeCriticalSection
                mov     ecx, _locktable+34h
                push    ecx             ; lpCriticalSection
                call    esi ; InitializeCriticalSection
                mov     edx, _locktable+24h
                push    edx             ; lpCriticalSection
                call    esi ; InitializeCriticalSection
                mov     eax, _locktable+4h
                push    eax             ; lpCriticalSection
                call    esi ; InitializeCriticalSection
                pop     esi
                retn
__mtinitlocks   endp

; ---------------------------------------------------------------------------


; =============== S U B R O U T I N E =======================================

; Attributes: library function

__mtdeletelocks proc near               ; CODE XREF: .text:__mtterm↑p
                push    esi
                push    edi
                mov     edi, ds:DeleteCriticalSection
                mov     esi, offset _locktable

loc_5C773D:                             ; CODE XREF: __mtdeletelocks+4A↓j
                mov     eax, dword ptr [esi]
                test    eax, eax
                jz      short loc_5C7771
                cmp     esi, offset _locktable+44h
                jz      short loc_5C7771
                cmp     esi, offset _locktable+34h
                jz      short loc_5C7771
                cmp     esi, offset _locktable+24h
                jz      short loc_5C7771
                cmp     esi, offset _locktable+4h
                jz      short loc_5C7771
                push    eax             ; lpCriticalSection
                call    edi ; DeleteCriticalSection
                mov     eax, dword ptr [esi]
                push    eax             ; Block
                call    _free
                add     esp, 4

loc_5C7771:                             ; CODE XREF: __mtdeletelocks+11↑j
                add     esi, 4
                cmp     esi, offset __small_block_heap
                jl      short loc_5C773D
                mov     ecx, _locktable+24h
                push    ecx             ; lpCriticalSection
                call    edi ; DeleteCriticalSection
                mov     edx, _locktable+34h
                push    edx             ; lpCriticalSection
                call    edi ; DeleteCriticalSection
                mov     eax, [_locktable+44h]
                push    eax             ; lpCriticalSection
                call    edi ; DeleteCriticalSection
                mov     ecx, _locktable+4h
                push    ecx             ; lpCriticalSection
                call    edi ; DeleteCriticalSection
                pop     edi
                pop     esi
                retn
__mtdeletelocks endp

; ---------------------------------------------------------------------------


; =============== S U B R O U T I N E =======================================

; Attributes: library function noreturn

; void __cdecl __noreturn exit(int Code)
_exit           proc near               ; CODE XREF: start+B2↓p

Code            = dword ptr  4

                mov     eax, dword ptr [esp+Code]
                push    0               ; int
                push    0               ; int
                push    eax             ; uExitCode
                call    _doexit
                add     esp, 0Ch
                retn
_exit           endp

; ---------------------------------------------------------------------------



; =============== S U B R O U T I N E =======================================

; Attributes: library function

; void __cdecl _cexit()
__cexit         proc near
                push    1               ; int
                push    0               ; int
                push    0               ; uExitCode
                call    _doexit
                add     esp, 0Ch
                retn
__cexit         endp

; ---------------------------------------------------------------------------


; =============== S U B R O U T I N E =======================================

; Attributes: library function

; void __cdecl _c_exit()
__c_exit        proc near
                push    1               ; int
                push    1               ; int
                push    0               ; uExitCode
                call    _doexit
                add     esp, 0Ch
                retn
__c_exit        endp

; ---------------------------------------------------------------------------



; =============== S U B R O U T I N E =======================================

; Attributes: library function

; void *__cdecl _expand(void *Block, size_t varSize)
__expand        proc near               ; CODE XREF: AfxCriticalNewHandler(uint)+3D↓p

var_4           = dword ptr -4
lpMem           = dword ptr  4
varSize            = dword ptr  8

                push    ecx
                mov     eax, dword ptr [esp+4+varSize]
                push    ebx
                push    esi
                cmp     eax, 0FFFFFFE0h
                push    edi
                jbe     short loc_5C6124
                xor     eax, eax
                pop     edi
                pop     esi
                pop     ebx
                pop     ecx
                retn
; ---------------------------------------------------------------------------

loc_5C6124:                             ; CODE XREF: __expand+B↑j
                test    eax, eax
                jbe     short loc_5C6130
                lea     esi, [eax+0Fh]
                and     esi, 0FFFFFFF0h
                jmp     short loc_5C6135
; ---------------------------------------------------------------------------

loc_5C6130:                             ; CODE XREF: __expand+16↑j
                mov     esi, 10h

loc_5C6135:                             ; CODE XREF: __expand+1E↑j
                push    9
                call    __lock
                mov     edi, [esp+14h+lpMem]
                add     esp, 4
                lea     eax, [esp+10h+varSize]
                lea     ecx, [esp+10h+var_4]
                push    eax
                push    ecx
                push    edi
                call    ___sbh_find_block
                add     esp, 0Ch
                test    eax, eax
                jz      short loc_5C6194
                mov     ecx, __sbh_threshold
                xor     ebx, ebx
                cmp     esi, ecx
                ja      short loc_5C6183
                mov     edx, dword ptr [esp+10h+varSize]
                shr     esi, 4
                push    esi
                push    eax
                mov     eax, dword ptr [esp+18h+var_4]
                push    edx
                push    eax
                call    ___sbh_resize_block
                add     esp, 10h
                test    eax, eax
                jz      short loc_5C6183
                mov     ebx, edi

loc_5C6183:                             ; CODE XREF: __expand+54↑j
                push    9
                call    __unlock
                add     esp, 4
                mov     eax, ebx
                pop     edi
                pop     esi
                pop     ebx
                pop     ecx
                retn
; ---------------------------------------------------------------------------

loc_5C6194:                             ; CODE XREF: __expand+48↑j
                push    9
                call    __unlock
                mov     ecx, _crtheap
                add     esp, 4
                push    esi             ; dwBytes
                push    edi             ; lpMem
                push    10h             ; dwFlags
                push    ecx             ; _crtheap
                call    HeapReAlloc
                pop     edi
                pop     esi
                pop     ebx
                pop     ecx
                retn
__expand        endp

; ---------------------------------------------------------------------------


; =============== S U B R O U T I N E =======================================

; Attributes: library function

__mtinit        proc near               ; CODE XREF: start:loc_5C1C0A↑p
                push    esi
                call    __mtinitlocks
                call    TlsAlloc
                cmp     eax, 0FFFFFFFFh
                mov     __tlsindex, eax
                jz      short loc_5C6D48
                push    74h ; 't'       ; varSize
                push    1               ; Count
                call    _calloc
                mov     esi, eax
                add     esp, 8
                test    esi, esi
                jz      short loc_5C6D48
                mov     eax, __tlsindex
                push    esi             ; lpTlsValue
                push    eax             ; __tlsindex
                call    TlsSetValue
                test    eax, eax
                jz      short loc_5C6D48
                push    esi
                call    __initptd
                add     esp, 4
                call    GetCurrentThreadId
                mov     [esi], eax
                mov     dword ptr [esi+4], 0FFFFFFFFh
                mov     eax, 1
                pop     esi
                retn
; ---------------------------------------------------------------------------

loc_5C6D48:                             ; CODE XREF: __mtinit+14↑j
                xor     eax, eax
                pop     esi
                retn
__mtinit        endp

; ---------------------------------------------------------------------------


__mtterm:
                call    __mtdeletelocks
                mov     eax, __tlsindex
                cmp     eax, 0FFFFFFFFh
                jz      short locret_5C6D70
                push    eax
                call    TlsFree
                mov     __tlsindex, 0FFFFFFFFh

locret_5C6D70:                          ; CODE XREF: .text:005C6D5D↑j
                retn
; ---------------------------------------------------------------------------


; =============== S U B R O U T I N E =======================================

; Attributes: library function

__initptd       proc near               ; CODE XREF: __beginthread+17↑p

arg_0           = dword ptr  4

                mov     eax, dword ptr [esp+arg_0]
                mov     dword ptr [eax+50h], offset _XcptActTab
                mov     dword ptr [eax+14h], 1
                retn
__initptd       endp

; ---------------------------------------------------------------------------
                align 10h

; =============== S U B R O U T I N E =======================================

; Attributes: library function

__getptd        proc near               ; CODE XREF: _CallSETranslator(EHExceptionRecord *,EHRegistrationNode *,void *,void *,_s_FuncInfo const *,int,EHRegistrationNode *)+8A↑p
                push    esi
                push    edi
                call    GetLastError
                mov     edi, eax
                mov     eax, __tlsindex
                push    eax             ; __tlsindex
                call    TlsGetValue
                mov     esi, eax
                test    esi, esi
                jnz     short loc_5C6E0E
                push    74h ; 't'       ; varSize
                push    1               ; Count
                call    _calloc
                mov     esi, eax
                add     esp, 8
                test    esi, esi
                jz      short loc_5C6E04
                mov     ecx, __tlsindex
                push    esi             ; lpTlsValue
                push    ecx             ; __tlsindex
                call    TlsSetValue
                test    eax, eax
                jz      short loc_5C6E04
                push    esi
                call    __initptd
                add     esp, 4
                call    GetCurrentThreadId
                push    edi             ; dwErrCode
                mov     [esi], eax
                mov     dword ptr [esi+4], 0FFFFFFFFh
                call    SetLastError
                mov     eax, esi
                pop     edi
                pop     esi
                retn
; ---------------------------------------------------------------------------

loc_5C6E04:                             ; CODE XREF: __getptd+2C↑j
                push    10h
                call    __amsg_exit
; ---------------------------------------------------------------------------
                add     esp, 4

loc_5C6E0E:                             ; CODE XREF: __getptd+1A↑j
                push    edi             ; dwErrCode
                call    SetLastError
                mov     eax, esi
                pop     edi
                pop     esi
                retn
__getptd        endp

; ---------------------------------------------------------------------------
                align 10h

; =============== S U B R O U T I N E =======================================

; Attributes: library function

; int __cdecl _freeptd(void *Block)
__freeptd       proc near               ; CODE XREF: __endthread+31↑p

Block           = dword ptr  4

                mov     eax, __tlsindex
                cmp     eax, 0FFFFFFFFh
                jz      locret_5C6EBC
                push    esi
                mov     esi, [esp+4+Block]
                test    esi, esi
                jnz     short loc_5C6E44
                push    eax             ; __tlsindex
                call    TlsGetValue
                mov     esi, eax
                test    esi, esi
                jz      short loc_5C6EAD

loc_5C6E44:                             ; CODE XREF: __freeptd+15↑j
                mov     eax, dword ptr [esi+24h]
                test    eax, eax
                jz      short loc_5C6E54
                push    eax             ; Block
                call    _free
                add     esp, 4

loc_5C6E54:                             ; CODE XREF: __freeptd+29↑j
                mov     eax, dword ptr [esi+28h]
                test    eax, eax
                jz      short loc_5C6E64
                push    eax             ; Block
                call    _free
                add     esp, 4

loc_5C6E64:                             ; CODE XREF: __freeptd+39↑j
                mov     eax, dword ptr [esi+30h]
                test    eax, eax
                jz      short loc_5C6E74
                push    eax             ; Block
                call    _free
                add     esp, 4

loc_5C6E74:                             ; CODE XREF: __freeptd+49↑j
                mov     eax, dword ptr [esi+38h]
                test    eax, eax
                jz      short loc_5C6E84
                push    eax             ; Block
                call    _free
                add     esp, 4

loc_5C6E84:                             ; CODE XREF: __freeptd+59↑j
                mov     eax, dword ptr [esi+40h]
                test    eax, eax
                jz      short loc_5C6E94
                push    eax             ; Block
                call    _free
                add     esp, 4

loc_5C6E94:                             ; CODE XREF: __freeptd+69↑j
                mov     eax, dword ptr [esi+44h]
                test    eax, eax
                jz      short loc_5C6EA4
                push    eax             ; Block
                call    _free
                add     esp, 4

loc_5C6EA4:                             ; CODE XREF: __freeptd+79↑j
                push    esi             ; Block
                call    _free
                add     esp, 4

loc_5C6EAD:                             ; CODE XREF: __freeptd+22↑j
                mov     eax, __tlsindex
                push    0               ; lpTlsValue
                push    eax             ; __tlsindex
                call    TlsSetValue
                pop     esi

locret_5C6EBC:                          ; CODE XREF: __freeptd+8↑j
                retn
__freeptd       endp

; =============== S U B R O U T I N E =======================================

; Attributes: library function

; _crt_signal_t __cdecl signal(int Signal, _crt_signal_t Function)
_signal         proc near

Signal          = dword ptr  4
Function        = dword ptr  8

                push    ebx
                push    ebp
                push    esi
                mov     esi, [esp+0Ch+Function]
                cmp     esi, 4
                push    edi
                jz      loc_5CC295
                cmp     esi, 3
                jz      loc_5CC295
                mov     ebp, [esp+10h+Signal]
                cmp     ebp, 2
                jz      loc_5CC1B9
                cmp     ebp, 15h
                jz      loc_5CC1B9
                cmp     ebp, 16h
                jz      loc_5CC1B9
                cmp     ebp, 0Fh
                jz      loc_5CC1B9
                cmp     ebp, 8
                jz      short loc_5CC125
                cmp     ebp, 4
                jz      short loc_5CC125
                cmp     ebp, 0Bh
                jnz     loc_5CC295

loc_5CC125:                             ; CODE XREF: _signal+45↑j
                call    __getptd
                mov     ebx, eax
                cmp     dword ptr [ebx+50h], offset _XcptActTab
                jnz     short loc_5CC16D
                mov     eax, [_XcptActTabSize]
                push    eax             ; varSize
                call    _malloc
                add     esp, 4
                mov     [ebx+50h], eax
                test    eax, eax
                jz      loc_5CC295
                mov     ecx, [_XcptActTabSize]
                mov     esi, offset _XcptActTab
                mov     edx, ecx
                mov     edi, eax
                shr     ecx, 2
                rep movsd
                mov     ecx, edx
                and     ecx, 3
                rep movsb
                mov     esi, [esp+10h+Function]

loc_5CC16D:                             ; CODE XREF: _signal+63↑j
                mov     eax, dword ptr [ebx+50h]
                push    eax
                push    ebp
                call    _siglookup
                add     esp, 8
                test    eax, eax
                jz      loc_5CC295
                mov     ecx, dword ptr [eax+4]
                mov     edi, [eax+8]
                cmp     ecx, ebp
                jnz     loc_5CC28E

loc_5CC190:                             ; CODE XREF: _signal+E0↓j
                mov     [eax+8], esi
                mov     ecx, _XcptActTabCount
                mov     edx, dword ptr [ebx+50h]
                add     eax, 0Ch
                lea     ecx, [ecx+ecx*2]
                lea     ecx, [edx+ecx*4]
                cmp     eax, ecx
                jnb     loc_5CC28E
                cmp     [eax+4], ebp
                jz      short loc_5CC190
                mov     eax, edi
                pop     edi
                pop     esi
                pop     ebp
                pop     ebx
                retn
; ---------------------------------------------------------------------------

loc_5CC1B9:                             ; CODE XREF: _signal+21↑j
                push    1
                call    __lock
                add     esp, 4
                cmp     ebp, 2
                jz      short loc_5CC1CD
                cmp     ebp, 15h
                jnz     short loc_5CC1F1

loc_5CC1CD:                             ; CODE XREF: _signal+F6↑j
                mov     eax, ConsoleCtrlHandler_Installed
                test    eax, eax
                jnz     short loc_5CC1F1
                push    1               ; Add
                push    offset _ctrlevent_capture@4 ; HandlerRoutine
                call    SetConsoleCtrlHandler
                cmp     eax, 1
                jnz     loc_5CC2A8
                mov     ConsoleCtrlHandler_Installed, eax

loc_5CC1F1:                             ; CODE XREF: _signal+FB↑j
                add     ebp, 0FFFFFFFEh
                cmp     ebp, 14h
                ja      loc_5CC280      ; jumptable 005CC205 case 4
                xor     edx, edx
                mov     dl, ss:byte_5CC2E8[ebp]
                jmp     ds:jpt_5CC205[edx*4] ; switch 5 cases
; ---------------------------------------------------------------------------

loc_5CC20C::                             ; CODE XREF: _signal+135↑j
                mov     edi, dword_70D584 ; jumptable 005CC205 case 0
                push    1
                mov     dword_70D584, esi
                call    __unlock
                add     esp, 4
                mov     eax, edi
                pop     edi
                pop     esi
                pop     ebp
                pop     ebx
                retn
; ---------------------------------------------------------------------------

loc_5CC229::                             ; CODE XREF: _signal+135↑j
                mov     edi, dword_70D588 ; jumptable 005CC205 case 2
                push    1
                mov     dword_70D588, esi
                call    __unlock
                add     esp, 4
                mov     eax, edi
                pop     edi
                pop     esi
                pop     ebp
                pop     ebx
                retn
; ---------------------------------------------------------------------------

loc_5CC246::                             ; CODE XREF: _signal+135↑j
                mov     edi, dword_70D58C ; jumptable 005CC205 case 3
                push    1
                mov     dword_70D58C, esi
                call    __unlock
                add     esp, 4
                mov     eax, edi
                pop     edi
                pop     esi
                pop     ebp
                pop     ebx
                retn
; ---------------------------------------------------------------------------

loc_5CC263::                             ; CODE XREF: _signal+135↑j
                mov     edi, dword_70D590 ; jumptable 005CC205 case 1
                push    1
                mov     dword_70D590, esi
                call    __unlock
                add     esp, 4
                mov     eax, edi
                pop     edi
                pop     esi
                pop     ebp
                pop     ebx
                retn
; ---------------------------------------------------------------------------

loc_5CC280::                             ; CODE XREF: _signal+127↑j
                mov     edi, [esp+10h+Function] ; jumptable 005CC205 case 4
                push    1
                call    __unlock
                add     esp, 4

loc_5CC28E:                             ; CODE XREF: _signal+BA↑j
                mov     eax, edi
                pop     edi
                pop     esi
                pop     ebp
                pop     ebx
                retn
; ---------------------------------------------------------------------------

loc_5CC295:                             ; CODE XREF: _signal+B↑j
                call    __errno
                mov     dword ptr [eax], 16h
                or      eax, 0FFFFFFFFh
                pop     edi
                pop     esi
                pop     ebp
                pop     ebx
                retn
; ---------------------------------------------------------------------------

loc_5CC2A8:                             ; CODE XREF: _signal+116↑j
                call    GetLastError
                mov     esi, eax
                call    ___doserrno
                push    1
                mov     [eax], esi
                call    __unlock
                add     esp, 4
                call    __errno
                pop     edi
                pop     esi
                mov     dword ptr [eax], 16h
                pop     ebp
                or      eax, 0FFFFFFFFh
                pop     ebx
                retn
_signal         endp

; ---------------------------------------------------------------------------
jpt_5CC205      dd offset loc_5CC20C    ; DATA XREF: _signal+135↑r
                dd offset loc_5CC263    ; jump table for switch statement
                dd offset loc_5CC229
                dd offset loc_5CC246
                dd offset loc_5CC280
byte_5CC2E8     db 0                    ; DATA XREF: _signal+12F↑r
                db 3 dup(4)
                dd 2 dup(4040404h), 4040104h, 2040404h, 90909003h



; =============== S U B R O U T I N E =======================================

; Attributes: library function

; int *__cdecl _errno()
__errno         proc near               ; CODE XREF: __fseek_lk:loc_5C0856↑p
                call    __getptd
                add     eax, 8
                retn
__errno         endp

; ---------------------------------------------------------------------------


; =============== S U B R O U T I N E =======================================

; Attributes: library function

; unsigned int *__cdecl __doserrno()
___doserrno     proc near               ; CODE XREF: __set_osfhnd+8F↑p
                call    __getptd
                add     eax, 0Ch
                retn
___doserrno     endp

; ---------------------------------------------------------------------------

; =============== S U B R O U T I N E =======================================

; Attributes: library function static

_siglookup      proc near               ; CODE XREF: _signal+A2↑p

arg_0           = dword ptr  4
arg_4           = dword ptr  8

                mov     edx, dword ptr [esp+arg_0]
                push    esi
                mov     esi, [esp+4+arg_4]
                push    edi
                mov     edi, _XcptActTabCount
                mov     ecx, dword ptr [esi+4]
                mov     eax, esi
                cmp     ecx, edx
                jz      short loc_5CC5CB
                lea     ecx, [edi+edi*2]
                lea     ecx, [esi+ecx*4]

loc_5CC5BF:                             ; CODE XREF: _siglookup+29↓j
                add     eax, 0Ch
                cmp     eax, ecx
                jnb     short loc_5CC5CB
                cmp     [eax+4], edx
                jnz     short loc_5CC5BF

loc_5CC5CB:                             ; CODE XREF: _siglookup+17↑j
                lea     ecx, [edi+edi*2]
                lea     ecx, [esi+ecx*4]
                cmp     eax, ecx
                jnb     short loc_5CC5DA
                cmp     [eax+4], edx
                jz      short loc_5CC5DC

loc_5CC5DA:                             ; CODE XREF: _siglookup+33↑j
                xor     eax, eax

loc_5CC5DC:                             ; CODE XREF: _siglookup+38↑j
                pop     edi
                pop     esi
                retn
_siglookup      endp

; ---------------------------------------------------------------------------

; ---------------------------------------------------------------------------
;005C793D
                push    0FFh
                call    ExitProcess
; ---------------------------------------------------------------------------
                retn
; ---------------------------------------------------------------------------

;005C793D
                mov     eax, __sbh_threshold
                retn
; ---------------------------------------------------------------------------




end
