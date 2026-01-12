
; Imports from ADVAPI32.dll
;
; Section 4. (virtual address 0030F000)
; Virtual size                  : 000031F2 (  12786.)
; Section size in file          : 00003200 (  12800.)
; varOffset to raw data for section: 00241200
; Flags C0000040: Data Readable Writable
; Alignment     : default
; ===========================================================================


;
; Imports from smackw32.dll
;
; __stdcall SmackUseMMX(x)
;                extrn _SmackUseMMX@4:dword ; CODE XREF: sub_4C6E49+F1↑p
; __stdcall SmackWait(x)
;                extrn _SmackWait@4:dword ; CODE XREF: sub_4C714D+D↑p
; __stdcall SmackNextFrame(x)
;                extrn _SmackNextFrame@4:dword
; __stdcall SmackBlit(x, x, x, x, x, x, x, x, x, x, x)
;                extrn _SmackBlit@44:dword ; CODE XREF: sub_4C716A+167↑p
; __stdcall SmackClose(x)
;                extrn _SmackClose@4:dword ; CODE XREF: sub_4C7556+35↑p
; __stdcall SmackSoundUseDirectSound(x)
;                extrn _SmackSoundUseDirectSound@4:dword
; __stdcall SmackBlitClose(x)
;                extrn _SmackBlitClose@4:dword
; __stdcall SmackBlitSetPalette(x, x, x)
;                extrn _SmackBlitSetPalette@12:dword
; __stdcall SmackBlitOpen(x)
;                extrn _SmackBlitOpen@4:dword
; __stdcall SmackDoFrame(x)
;                extrn _SmackDoFrame@4:dword ; CODE XREF: sub_4C716A+C6↑p
; __stdcall SmackBufferClose(x)
;                extrn _SmackBufferClose@4:dword
; __stdcall SmackToBuffer(x, x, x, x, x, x, x)
;                extrn _SmackToBuffer@28:dword
; __stdcall SmackBufferOpen(x, x, x, x, x, x)
;                extrn _SmackBufferOpen@24:dword
; __stdcall SmackOpen(x, x, x)
;                extrn _SmackOpen@12:dword ; CODE XREF: sub_4C733E+80↑p
; __stdcall SmackBufferFocused(x)
;                extrn _SmackBufferFocused@4:dword
; __stdcall SmackSoundOnOff(x, x)
;                extrn _SmackSoundOnOff@8:dword

