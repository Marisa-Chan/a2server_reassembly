


;functions
externdef _malloc:proc
externdef _calloc:proc
externdef _free:proc
externdef _realloc:proc
externdef __exit:proc
externdef _exit:proc
externdef __onexit:proc
externdef _doexit:proc
externdef __amsg_exit:proc
externdef __lock:proc
externdef __unlock:proc
externdef __initterm:proc
externdef __heap_init:proc
externdef __getptd:proc
externdef __errno:proc
externdef ___doserrno:proc
externdef __NMSG_WRITE:proc
externdef __mtinit:proc
externdef __initptd:proc
externdef __freeptd:proc
externdef _siglookup:proc
externdef __msize:proc
externdef __expand:proc
externdef ___onexitinit:proc

;data

externdef __app_type:dword
externdef __tlsindex:dword
externdef dword_70D584:dword    ; DATA XREF: _signal
externdef dword_70D588:dword    ; DATA XREF: _signal
externdef dword_70D58C:dword    ; DATA XREF: _signal
externdef dword_70D590:dword    ; DATA XREF: _signal
externdef rterrs:dword
externdef _exitflag:byte
externdef _XcptActTabCount:dword





;In main.asm
externdef __pioinfo:dword
externdef __xp_a:dword
externdef __xp_z:dword
externdef __xt_a:dword
externdef __xt_z:dword

externdef _strncpy:proc
externdef _ctrlevent_capture@4:proc