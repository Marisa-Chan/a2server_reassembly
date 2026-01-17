.686p
.mmx
.model flat
assume fs:nothing
option casemap :none

extern _testvar:dword

.code

testInit proc
	mov _testvar, 55h
	xor eax, eax
	retn
testInit endp

crtxia SEGMENT alias(".CRT$XIA") DWORD PUBLIC 'DATA'
dd offset testInit
crtxia ends


end