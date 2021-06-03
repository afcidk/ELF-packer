global unpack_asm:function
global payload_size:data
global data_size:data

payload_size    dq end - unpack_asm 
data_size       dq end - data

unpack_asm:
    nop
    nop
    mov rcx, [rel encode_size]
    mov r11, [rel text_start]
    xor r12, r12

L1:
    sub byte [r11], r12b
    inc r11 
    inc r12
    loop L1

    mov rax, [rel text_start]
    jmp rax

data:
    text_start  dq 0xffffffffffffffff
    encode_size dq 0xffffffffffffffff
    MAGIC       dq 0x00000000efbeadde

end:
