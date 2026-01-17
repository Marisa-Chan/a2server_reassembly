.686p
.mmx
.model flat
assume fs:nothing
option casemap :none

extern _testvar:dword
externdef ?TestField@TestClass@@QAEHXZ:proc

.code

testInit proc
	mov _testvar, 77h
	xor eax, eax
	retn
testInit endp

?TestField@TestClass@@QAEHXZ proc
	mov eax, 0AAh
	retn
?TestField@TestClass@@QAEHXZ endp


crtxia SEGMENT alias(".CRT$XIU") DWORD PUBLIC 'DATA'
dd offset testInit
crtxia ends


end