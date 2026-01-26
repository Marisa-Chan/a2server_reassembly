                .686p
                .mmx
                .model flat
                assume fs:nothing
                option casemap :none


; Token class
.code

extern ??_7Token@@6B@:dword

public _GetVtblToken
_GetVtblToken proc
    mov eax, offset ??_7Token@@6B@
    ret
_GetVtblToken endp

end