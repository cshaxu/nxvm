; Copyright 2012-2014 Neko.
; Recovered guest assembly; source provenance is recorded in README.md.
test dl, 80
jnz $(label_int_13_hdd)
int 40
jmp near $(label_int_13_end)
$(label_int_13_hdd):
$(label_int_13_cmp_00):
cmp ah, 00
jnz $(label_int_13_cmp_01)
jmp near $(label_int_13_00)
$(label_int_13_cmp_01):
cmp ah, 01
jnz $(label_int_13_cmp_02)
jmp near $(label_int_13_01)
$(label_int_13_cmp_02):
cmp ah, 02
jnz $(label_int_13_cmp_03)
jmp near $(label_int_13_02)
$(label_int_13_cmp_03):
cmp ah, 03
jnz $(label_int_13_cmp_08)
jmp near $(label_int_13_03)
$(label_int_13_cmp_08):
cmp ah, 08
jnz $(label_int_13_cmp_15)
jmp near $(label_int_13_08)
$(label_int_13_cmp_15):
cmp ah, 15
jnz $(label_int_13_cmp_def)
jmp near $(label_int_13_15)
$(label_int_13_cmp_def):
mov ah, 01
stc
jmp near $(label_int_13_end)
$(label_int_13_00):
; reset drive
cmp dl, 80
jnz $(label_int_13_00_x)
mov ah, 00
clc
jmp near $(label_int_13_end)
$(label_int_13_00_x):
mov ah, 0c
stc
jmp near $(label_int_13_end)
$(label_int_13_01):
; get hdd status byte
push bx
push ds
mov bx, 0040
mov ds, bx
mov ah, ds:[0074]
pop ds
pop bx
or ah, ah
jnz $(label_int_13_01_fail)
clc
jmp near $(label_int_13_end)
$(label_int_13_01_fail):
stc
jmp near $(label_int_13_end)
$(label_int_13_02):
; read AL CHS sectors through the primary fixed-disk PIO channel
or al, al
jnz $(label_int_13_02_nonzero)
jmp near $(label_int_13_02_fail)
$(label_int_13_02_nonzero):
push ax
push bx
push cx
push dx
push si
push di
push es
mov si, ax
mov di, bx
mov bx, dx
mov dx, 01f2
mov ax, si
out dx, al
inc dx
mov al, cl
and al, 3f
out dx, al
inc dx
mov al, ch
out dx, al
inc dx
mov al, cl
shr al, 01
shr al, 01
shr al, 01
shr al, 01
shr al, 01
shr al, 01
out dx, al
inc dx
mov ax, bx
mov al, ah
or al, a0
out dx, al
inc dx
mov al, 20
out dx, al
$(label_int_13_02_wait):
in al, dx
test al, 80
jnz $(label_int_13_02_wait)
test al, 01
jnz $(label_int_13_02_pop_fail)
test al, 08
jz $(label_int_13_02_pop_fail)
mov cx, si
and cx, 00ff
shl cx, 01
shl cx, 01
shl cx, 01
shl cx, 01
shl cx, 01
shl cx, 01
shl cx, 01
shl cx, 01
mov dx, 01f0
$(label_int_13_02_data):
in ax, dx
mov es:[di], ax
add di, 02
dec cx
jz $(label_int_13_02_done)
test cl, ff
jnz $(label_int_13_02_data)
mov dx, 01f7
$(label_int_13_02_sector_wait):
in al, dx
test al, 80
jnz $(label_int_13_02_sector_wait)
test al, 01
jnz $(label_int_13_02_pop_fail)
test al, 08
jz $(label_int_13_02_pop_fail)
mov dx, 01f0
jmp near $(label_int_13_02_data)
$(label_int_13_02_done):
mov dx, 01f7
in al, dx
pop es
pop di
pop si
pop dx
pop cx
pop bx
pop ax
mov ah, 00
clc
jmp near $(label_int_13_end)
$(label_int_13_02_pop_fail):
pop es
pop di
pop si
pop dx
pop cx
pop bx
pop ax
$(label_int_13_02_fail):
mov ah, 04
stc
jmp near $(label_int_13_end)
$(label_int_13_03):
; write AL CHS sectors through the primary fixed-disk PIO channel
or al, al
jnz $(label_int_13_03_nonzero)
jmp near $(label_int_13_03_fail)
$(label_int_13_03_nonzero):
push ax
push bx
push cx
push dx
push si
push di
push ds
push es
mov di, ax
mov si, bx
mov bx, dx
mov ax, es
mov ds, ax
mov dx, 01f2
mov ax, di
out dx, al
inc dx
mov al, cl
and al, 3f
out dx, al
inc dx
mov al, ch
out dx, al
inc dx
mov al, cl
shr al, 01
shr al, 01
shr al, 01
shr al, 01
shr al, 01
shr al, 01
out dx, al
inc dx
mov ax, bx
mov al, ah
or al, a0
out dx, al
inc dx
mov al, 30
out dx, al
$(label_int_13_03_wait):
in al, dx
test al, 80
jnz $(label_int_13_03_wait)
test al, 01
jnz $(label_int_13_03_pop_fail)
test al, 08
jz $(label_int_13_03_pop_fail)
mov cx, di
and cx, 00ff
shl cx, 01
shl cx, 01
shl cx, 01
shl cx, 01
shl cx, 01
shl cx, 01
shl cx, 01
shl cx, 01
mov dx, 01f0
$(label_int_13_03_data):
mov ax, ds:[si]
add si, 02
out dx, ax
dec cx
jz $(label_int_13_03_done)
test cl, ff
jnz $(label_int_13_03_data)
mov dx, 01f7
$(label_int_13_03_sector_wait):
in al, dx
test al, 80
jnz $(label_int_13_03_sector_wait)
test al, 01
jnz $(label_int_13_03_pop_fail)
test al, 08
jz $(label_int_13_03_pop_fail)
mov dx, 01f0
jmp near $(label_int_13_03_data)
$(label_int_13_03_done):
mov dx, 01f7
in al, dx
pop es
pop ds
pop di
pop si
pop dx
pop cx
pop bx
pop ax
mov ah, 00
clc
jmp near $(label_int_13_end)
$(label_int_13_03_pop_fail):
pop es
pop ds
pop di
pop si
pop dx
pop cx
pop bx
pop ax
$(label_int_13_03_fail):
mov ah, 04
stc
jmp near $(label_int_13_end)
$(label_int_13_08):
; get hdd parameters
push ax
push bx
push ds
mov ax, 0000
mov ds, ax
mov bx, ds:[0104]
mov ax, ds:[0106]
mov ds, ax
mov cx, ds:[bx+00]
dec cx          ; ncyl - 1
xchg ch, cl
shl cl, 01
shl cl, 01
shl cl, 01
shl cl, 01
shl cl, 01
shl cl, 01
mov al, ds:[bx+0e] ; nsector
or  cl, al ; (ncyl>>2)&0xc0
           ; | nsector
mov dh, ds:[bx+02]
dec dh          ; nhead - 1
mov ax, 0040
mov ds, ax
mov dl, ds:[0075]
pop ds
pop bx
pop ax
mov bl, 2f
mov al, cl
and al, 3f
mov ah, 00
clc
jmp near $(label_int_13_end)
$(label_int_13_15):
; get drive type
; count=(ncyl-1)*nhead*nsec
push bx
push ds
mov ax, 0000
mov ds, ax
mov bx, ds:[0104]
mov ax, ds:[0106]
mov ds, ax
mov cx, ds:[bx+00]
mov al, ds:[bx+0e] ; nsector
mov dh, ds:[bx+02] ; nhead
mov ah, 00
mul dh ; nhead * nsector
mul cx ; total size
mov cx, dx ; size high 16
mov dx, ax ; size low  16
pop ds
pop bx
mov ah, 03
clc
jmp near $(label_int_13_end)
$(label_int_13_end):
; set hdd status byte
push ax
push bx
push cx
push dx
push ds
mov cl, ah
pushf
pop dx
mov bx, 0040
mov ds, bx
test dl, 01
jnz $(label_int_13_status_error)
mov byte ds:[0074], 00
jmp near $(label_int_13_status_done)
$(label_int_13_status_error):
mov ds:[0074], cl
$(label_int_13_status_done):
push dx
popf
pop ds
pop dx
pop cx
pop bx
pop ax
; set/clear cf
push ax
push bx
pushf
pop ax
and ax, 0001
mov bx, sp
and word ss:[bx+08], fffe
or  word ss:[bx+08], ax
pop bx
pop ax
iret
