; Copyright 2012-2014 Neko.
; Recovered guest assembly; source provenance is recorded in README.md.
; init cmos
mov al, 0b ; select reg b
out 70, al
 mov al, 02 ; 24 hour mode
out 71, al
; init vrtc
mov ah, 02 ; ch,cl,dh
int 1a     ; get cmos STD_TIME
mov bh, ch ; convert ch
and bh, 0f
shr ch, 01
shr ch, 01
shr ch, 01
shr ch, 01
mov al, ch
mov ch, 0a
mul ch
add bh, al
mov ch, bh ; ch is hex now
mov bh, cl ; convert cl
and bh, 0f
shr cl, 01
shr cl, 01
shr cl, 01
shr cl, 01
mov al, cl
mov cl, 0a
mul cl
add bh, al
mov cl, bh ; cl is hex now
mov bh, dh ; convert dh
and bh, 0f
shr dh, 01
shr dh, 01
shr dh, 01
shr dh, 01
mov al, dh
mov dh, 0a
mul dh
add bh, al
mov dh, bh ; dh is hex now
mov al, ch ; x = hour
mov bl, 3c
mul bl     ; x *= 60
mov ch, 00
add ax, cx ; x += min
xor cx, cx
mov cl, dh
mov bx, 003c
mul bx     ; x *= 60
add ax, cx ; x += second
mov bx, 0040
mov ds, bx
mov cx, dx
mov bx, 0012
mul bx      ; x *= 18
mov ds:[006c], ax
mov ds:[006e], dx
mov ax, cx
mul bx
add ds:[006e], ax
