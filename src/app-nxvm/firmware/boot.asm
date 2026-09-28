; Copyright 2012-2014 Neko.
; Recovered guest assembly; source provenance is recorded in README.md.
$(label_post_boot_start):
xor dl, dl
jmp near $(label_post_boot_read)
$(label_post_boot_read):
mov dh, 00     ; select head 0
mov ch, 00     ; select cylender 0
mov cl, 01     ; select sector 1
mov bx, 0000
mov es, bx     ; target es = 0000
mov bx, 7c00   ; target bx = 7c00
mov al, 01     ; read 1 sector
mov ah, 02     ; command read
int 13
pushf
pop ax
test al, 01
jz $(label_post_boot_signature)
jmp near $(label_post_boot_fail)
$(label_post_boot_signature):
mov bx, 0000
mov ds, bx
mov ax, ds:[7dfe]
cmp ax, aa55
jz $(label_post_boot_valid)
jmp near $(label_post_boot_fail)
$(label_post_boot_valid):
jmp near $(label_post_boot_succ)
$(label_post_boot_fail):
cmp dl, 80
jz $(label_post_boot_message)
mov dl, 80
jmp near $(label_post_boot_read)
$(label_post_boot_message):
mov ah, 02
mov dh, 05
mov dl, 00
int 10  ; set cursor position
mov ah, 0e
mov bl, 0f
mov bh, 00
mov al, 49
int 10  ; display char 'I'
mov al, 6e
int 10  ; display char 'n'
mov al, 76
int 10  ; display char 'v'
mov al, 61
int 10  ; display char 'a'
mov al, 6c
int 10  ; display char 'l'
mov al, 69
int 10  ; display char 'i'
mov al, 64
int 10  ; display char 'd'
mov al, 20
int 10  ; display char ' '
mov al, 62
int 10  ; display char 'b'
mov al, 6f
int 10  ; display char 'o'
mov al, 6f
int 10  ; display char 'o'
mov al, 74
int 10  ; display char 't'
mov al, 20
int 10  ; display char ' '
mov al, 64
int 10  ; display char 'd'
mov al, 69
int 10  ; display char 'i'
mov al, 73
int 10  ; display char 's'
mov al, 6b
int 10  ; display char 'k'
mov al, 0d
int 10  ; display new line
mov al, 0a
int 10  ; display new line
$(label_post_boot_fail_loop):
mov ah, 11
int 16  ; get key press
pushf   ; if any key pressed,
pop ax  ; then stop nxvm
test ax, 0040
jnz $(label_post_boot_fail_loop)
 mov ah, 00
 int 16
 mov bx, 0040
 mov ds, bx
 mov byte ds:[0505], 01
 jmp near $(label_post_boot_read)
$(label_post_boot_succ):
; start operating system
xor bx, bx
mov cx, 0001
xor dx, dx
mov dx, 03c2
mov al, 01
out dx, al
xor dx, dx
mov sp, fffe
 jmp 0000:7c00
