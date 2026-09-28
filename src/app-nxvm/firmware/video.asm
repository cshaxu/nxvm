; Copyright 2012-2014 Neko.
; Recovered guest assembly; source provenance is recorded in README.md.
cmp ah, 00
jnz $(label_int_10_cmp_02)
jmp near $(label_int_10_set_mode)
$(label_int_10_cmp_02):
cmp ah, 02
jnz $(label_int_10_cmp_05)
jmp near $(label_int_10_cursor)
$(label_int_10_cmp_05):
cmp ah, 05
jnz $(label_int_10_cmp_06)
jmp near $(label_int_10_page)
$(label_int_10_cmp_06):
cmp ah, 06
jnz $(label_int_10_cmp_08)
jmp near $(label_int_10_clear)
$(label_int_10_cmp_08):
cmp ah, 08
jnz $(label_int_10_cmp_09)
jmp near $(label_int_10_read_char)
$(label_int_10_cmp_09):
cmp ah, 09
jnz $(label_int_10_cmp_0b)
jmp near $(label_int_10_write_char)
$(label_int_10_cmp_0b):
cmp ah, 0b
jnz $(label_int_10_cmp_0e)
iret
$(label_int_10_cmp_0e):
cmp ah, 0e
jnz $(label_int_10_cmp_0f)
jmp near $(label_int_10_tty)
$(label_int_10_cmp_0f):
cmp ah, 0f
jnz $(label_int_10_ret)
jmp near $(label_int_10_mode)
$(label_int_10_ret):
iret
$(label_int_10_set_mode):
cmp al, 06
jz $(label_int_10_set_cga_06_jump)
cmp al, 0d
jz $(label_int_10_set_ega_0d_jump)
cmp al, 0e
jz $(label_int_10_set_ega_0e_jump)
cmp al, 10
jnz $(label_int_10_cmp_03)
jmp near $(label_int_10_set_ega_10_jump)
$(label_int_10_cmp_03):
cmp al, 03
jnz $(label_int_10_set_mode_ret)
jmp near $(label_int_10_set_text_03)
$(label_int_10_set_cga_06_jump):
jmp near $(label_int_10_set_cga_06)
$(label_int_10_set_ega_0d_jump):
jmp near $(label_int_10_set_ega_0d)
$(label_int_10_set_ega_0e_jump):
jmp near $(label_int_10_set_ega_0e)
$(label_int_10_set_ega_10_jump):
jmp near $(label_int_10_set_ega_10)
$(label_int_10_set_mode_ret):
iret
$(label_int_10_set_ega_0d):
push ax
push bx
push dx
push ds
mov bx, 0040
mov ds, bx
mov byte ds:[0049], 0d
pop ds
mov dx, 03c4
mov al, 02
out dx, al
inc dx
mov al, 0f
out dx, al
mov dx, 03ce
mov al, 05
out dx, al
inc dx
xor al, al
out dx, al
dec dx
mov al, 06
out dx, al
inc dx
mov al, 05
out dx, al
mov dx, 03d4
mov al, 01
out dx, al
inc dx
mov al, 27
out dx, al
dec dx
mov al, 07
out dx, al
inc dx
xor al, al
out dx, al
dec dx
mov al, 12
out dx, al
inc dx
mov ax, 00c7
out dx, al
dec dx
mov al, 13
out dx, al
inc dx
mov al, 14
out dx, al
mov dx, 03da
in al, dx
mov dx, 03c0
mov al, 30
out dx, al
mov al, 01
out dx, al
pop dx
pop bx
pop ax
iret
$(label_int_10_set_ega_0e):
push ax
push bx
push dx
push ds
mov bx, 0040
mov ds, bx
mov byte ds:[0049], 0e
pop ds
mov dx, 03c4
mov al, 02
out dx, al
inc dx
mov al, 0f
out dx, al
mov dx, 03ce
mov al, 05
out dx, al
inc dx
xor al, al
out dx, al
dec dx
mov al, 06
out dx, al
inc dx
mov al, 05
out dx, al
mov dx, 03d4
mov al, 01
out dx, al
inc dx
mov al, 4f
out dx, al
dec dx
mov al, 07
out dx, al
inc dx
xor al, al
out dx, al
dec dx
mov al, 12
out dx, al
inc dx
mov ax, 00c7
out dx, al
dec dx
mov al, 13
out dx, al
inc dx
mov al, 28
out dx, al
mov dx, 03da
in al, dx
mov dx, 03c0
mov al, 30
out dx, al
mov al, 01
out dx, al
pop dx
pop bx
pop ax
iret
$(label_int_10_set_ega_10):
push ax
push bx
push dx
push ds
mov bx, 0040
mov ds, bx
mov byte ds:[0049], 10
pop ds
mov dx, 03c4
mov al, 02
out dx, al
inc dx
mov al, 0f
out dx, al
mov dx, 03ce
mov al, 05
out dx, al
inc dx
xor al, al
out dx, al
dec dx
mov al, 06
out dx, al
inc dx
mov al, 05
out dx, al
mov dx, 03d4
mov al, 01
out dx, al
inc dx
mov al, 4f
out dx, al
dec dx
mov al, 07
out dx, al
inc dx
mov al, 02
out dx, al
dec dx
mov al, 12
out dx, al
inc dx
mov al, 5d
out dx, al
dec dx
mov al, 0c
out dx, al
inc dx
xor al, al
out dx, al
dec dx
mov al, 0d
out dx, al
inc dx
xor al, al
out dx, al
dec dx
mov al, 13
out dx, al
inc dx
mov al, 28
out dx, al
mov dx, 03da
in al, dx
mov dx, 03c0
mov al, 30
out dx, al
mov al, 01
out dx, al
pop dx
pop bx
pop ax
iret

$(label_int_10_set_cga_06):
push ax
push bx
push dx
push ds
mov bx, 0040
mov ds, bx
mov byte ds:[0049], 06
pop ds
mov dx, 03d8
mov al, 1a
out dx, al
inc dx
mov al, 0f
out dx, al
pop dx
pop bx
pop ax
iret
$(label_int_10_set_text_03):
push ax
push bx
push dx
push ds
mov bx, 0040
mov ds, bx
mov byte ds:[0049], 03
pop ds
mov dx, 03c2
mov al, 01
out dx, al
mov dx, 03d8
mov al, 0d
out dx, al
mov dx, 03ce
mov al, 06
out dx, al
inc dx
mov al, 09
out dx, al
mov dx, 03d4
mov al, 13
out dx, al
inc dx
xor al, al
out dx, al
pop dx
pop bx
pop ax
iret
$(label_int_10_cursor):
push ax
push bx
push cx
push dx
push ds
mov bx, 0040
mov ds, bx
mov ax, dx
mov ds:[0050], ax
mov al, dh
xor ah, ah
mov cl, 50
mul cl
xor dh, dh
add ax, dx
mov cx, ax
mov dx, 03d4
mov al, 0e
out dx, al
inc dx
mov al, ch
out dx, al
dec dx
mov al, 0f
out dx, al
inc dx
mov al, cl
out dx, al
pop ds
pop dx
pop cx
pop bx
pop ax
iret
$(label_int_10_page):
push ax
push bx
push cx
push dx
push ds
cmp bh, 00
jnz $(label_int_10_page_done)
mov bx, 0040
mov ds, bx
mov byte ds:[0062], 00
mov word ds:[004e], 0000
mov dx, ds:[0050]
mov al, dh
xor ah, ah
mov cl, 50
mul cl
xor dh, dh
add ax, dx
mov cx, ax
mov dx, 03d4
mov al, 0e
out dx, al
inc dx
mov al, ch
out dx, al
dec dx
mov al, 0f
out dx, al
inc dx
mov al, cl
out dx, al
$(label_int_10_page_done):
pop ds
pop dx
pop cx
pop bx
pop ax
iret
$(label_int_10_read_char):
push bx
push cx
push dx
push si
push di
push ds
push es
mov si, bx
mov bx, 0040
mov ds, bx
mov ax, si
cmp ah, ds:[0062]
jnz $(label_int_10_read_char_done)
mov dx, ds:[0050]
mov al, dh
xor ah, ah
mov cl, 50
mul cl
xor dh, dh
add ax, dx
shl ax, 01
add ax, ds:[004e]
mov di, ax
mov ax, b800
mov es, ax
mov ax, es:[di]
$(label_int_10_read_char_done):
pop es
pop ds
pop di
pop si
pop dx
pop cx
pop bx
iret
$(label_int_10_write_char):
push ax
push bx
push cx
push dx
push si
push di
push bp
push ds
push es
mov ah, bl
mov bp, ax
mov si, bx
mov bx, 0040
mov ds, bx
mov ax, si
cmp ah, ds:[0062]
jnz $(label_int_10_write_char_done)
push cx
mov dx, ds:[0050]
mov al, dh
xor ah, ah
mov cl, 50
mul cl
xor dh, dh
add ax, dx
shl ax, 01
add ax, ds:[004e]
mov di, ax
pop cx
mov ax, b800
mov es, ax
mov ax, bp
rep:
stosw
$(label_int_10_write_char_done):
pop es
pop ds
pop bp
pop di
pop si
pop dx
pop cx
pop bx
pop ax
iret
$(label_int_10_clear):
push ax
push bx
push cx
push dx
push si
push di
push es
or al, al
jnz $(label_int_10_clear_legacy)
cmp dh, ch
jb $(label_int_10_clear_done)
cmp dl, cl
jb $(label_int_10_clear_done)
cmp dh, 18
ja $(label_int_10_clear_done)
cmp dl, 4f
ja $(label_int_10_clear_done)
push dx
push cx
mov dl, cl
mov al, ch
xor ah, ah
mov cl, 50
mul cl
xor dh, dh
add ax, dx
shl ax, 01
mov di, ax
pop cx
pop dx
mov bl, dl
sub bl, cl
inc bl
mov ah, bh
xor al, al
mov bh, dh
sub bh, ch
inc bh
mov dx, b800
mov es, dx
cld
$(label_int_10_clear_row):
xor ch, ch
mov cl, bl
rep:
stosw
dec bh
jz $(label_int_10_clear_done)
mov cx, 0050
sub cl, bl
shl cx, 01
add di, cx
jmp near $(label_int_10_clear_row)
$(label_int_10_clear_legacy):
mov ax, b800
mov es, ax
xor di, di
xor ax, ax
mov cx, 07d0
cld
rep:
stosw
$(label_int_10_clear_done):
pop es
pop di
pop si
pop dx
pop cx
pop bx
pop ax
iret
$(label_int_10_tty):
push ax
push bx
push cx
push dx
push si
push di
push bp
push ds
push es
mov si, ax
mov bp, bx
mov bx, 0040
mov ds, bx
mov dh, ds:[0051]
mov dl, ds:[0050]
mov ax, si
cmp al, 0d
jnz $(label_int_10_tty_cmp_lf)
mov dl, 00
jmp near $(label_int_10_cursor_store)
$(label_int_10_tty_cmp_lf):
cmp al, 0a
jnz $(label_int_10_tty_cmp_bs)
inc dh
jmp near $(label_int_10_tty_row)
$(label_int_10_tty_cmp_bs):
cmp al, 08
jnz $(label_int_10_tty_put)
or dl, dl
jnz $(label_int_10_tty_back_col)
or dh, dh
jz $(label_int_10_cursor_store)
dec dh
mov dl, 4f
jmp near $(label_int_10_cursor_store)
$(label_int_10_tty_back_col):
dec dl
jmp near $(label_int_10_cursor_store)
$(label_int_10_tty_put):
mov al, dh
xor ah, ah
mov cl, 50
mul cl
mov cx, dx
xor ch, ch
add ax, cx
shl ax, 01
mov di, ax
mov ax, b800
mov es, ax
mov ax, si
mov es:[di], al
mov bx, bp
mov es:[di+01], bl
inc dl
cmp dl, 50
jnb $(label_int_10_tty_wrap)
jmp near $(label_int_10_cursor_store)
$(label_int_10_tty_wrap):
mov dl, 00
inc dh
$(label_int_10_tty_row):
cmp dh, 19
jnb $(label_int_10_tty_scroll)
jmp near $(label_int_10_cursor_store)
$(label_int_10_tty_scroll):
mov bx, b800
mov ds, bx
mov es, bx
mov si, 00a0
xor di, di
mov cx, 0780
rep:
movsw
mov ax, 0720
mov cx, 0050
rep:
stosw
mov bx, 0040
mov ds, bx
mov dh, 18
mov dl, 00
$(label_int_10_cursor_store):
mov ax, dx
mov ds:[0050], ax
$(label_int_10_cursor_crtc):
mov al, dh
xor ah, ah
mov cl, 50
mul cl
xor dh, dh
add ax, dx
mov cx, ax
mov dx, 03d4
mov al, 0e
out dx, al
inc dx
mov al, ch
out dx, al
dec dx
mov al, 0f
out dx, al
inc dx
mov al, cl
out dx, al
pop es
pop ds
pop bp
pop di
pop si
pop dx
pop cx
pop bx
pop ax
iret
$(label_int_10_mode):
push ds
push bx
mov bx, 0040
mov ds, bx
mov al, ds:[0049]
mov ah, ds:[004a]
mov bh, ds:[0062]
pop bx
pop ds
iret
