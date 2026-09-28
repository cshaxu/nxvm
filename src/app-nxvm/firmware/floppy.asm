; Copyright 2012-2026 Neko.
; Guest BIOS INT 40h. Protocol/return authorities are recorded in README.md.
; One selected drive; every read/write uses the reserved 9FC0:0000 DMA sector.
push bp
mov bp, sp
sub sp, 000c
mov ss:[bp-02], ax
mov ss:[bp-04], cx
mov ss:[bp-06], dx
mov ss:[bp-08], es
mov ss:[bp-0a], bx
mov word ss:[bp-0c], 0000
push bx
push cx
push dx
push si
push di
push ds
push es
mov bx, 0040
mov ds, bx
sti
cmp ah, 01
jnz $(fdc_not_status)
mov ah, ds:[0041]
cmp ah, 01
cmc
jmp near $(fdc_return)
$(fdc_not_status):
cmp dl, 00
jz $(fdc_valid_drive)
cmp ah, 15
jnz $(fdc_bad_request)
xor ax, ax
clc
jmp near $(fdc_return)
$(fdc_bad_request):
mov ah, 01
stc
jmp near $(fdc_complete)
$(fdc_valid_drive):
cmp ah, 00
jnz $(fdc_not_reset)
call $(fdc_initialize)
jmp near $(fdc_complete)
$(fdc_not_reset):
cmp ah, 08
jnz $(fdc_not_parameters)
mov bx, ds:[00ac]
mov ss:[bp-10], bx
mov word ss:[bp-12], 0101
mov word ss:[bp-16], f7c0
mov word ss:[bp-1a], f000
mov al, 10
out 70, al
in al, 71
shr al, 01
shr al, 01
shr al, 01
shr al, 01
xor ah, ah
mov ss:[bp-0e], ax
xor ax, ax
clc
jmp near $(fdc_complete)
$(fdc_not_parameters):
cmp ah, 15
jnz $(fdc_not_type)
mov byte ds:[0041], 00
mov ah, 01
clc
jmp near $(fdc_return)
$(fdc_not_type):
cmp ah, 02
jz $(fdc_transfer)
cmp ah, 03
jz $(fdc_transfer)
jmp near $(fdc_bad_request)
$(fdc_transfer):
test byte ds:[003e], 01
jnz $(fdc_sector)
call $(fdc_initialize)
jnc $(fdc_sector)
jmp near $(fdc_complete)
$(fdc_sector):
mov cx, ss:[bp-04]
mov dx, ss:[bp-06]
cmp byte ss:[bp-02], 00
je $(fdc_bad_address)
cmp dh, 01
ja $(fdc_bad_address)
cmp cl, 01
jb $(fdc_bad_address)
cmp cl, ds:[00ac]
ja $(fdc_bad_address)
cmp ch, ds:[00ad]
jbe $(fdc_address_valid)
$(fdc_bad_address):
mov ah, 04
stc
jmp near $(fdc_complete)
$(fdc_address_valid):
; Normalize caller address so a sector copy never wraps a segment offset.
mov ax, ss:[bp-0a]
mov bx, ax
shr bx, 01
shr bx, 01
shr bx, 01
shr bx, 01
add ss:[bp-08], bx
and ax, 000f
mov ss:[bp-0a], ax
; SEEK, SIS and READ ID have one synchronous consumer, not the IRQ handler.
sub sp, 0004
mov si, sp
mov byte ss:[si], 0f
mov byte ss:[si+01], 00
mov ss:[si+02], ch
push cx
mov cx, 0003
call $(fdc_send)
pop cx
lea sp, ss:[si+04]
jnc $(fdc_seek_wait)
jmp near $(fdc_timeout)
$(fdc_seek_wait):
call $(fdc_finish_seek)
jnc $(fdc_read_id)
jmp near $(fdc_complete)
$(fdc_read_id):
mov al, 4a
call $(fdc_put)
jc $(fdc_seek_io_error)
mov ax, dx
mov al, ah
shl al, 01
shl al, 01
call $(fdc_put)
jc $(fdc_seek_io_error)
call $(fdc_result)
jnc $(fdc_check_id)
jmp near $(fdc_complete)
$(fdc_seek_io_error):
jmp near $(fdc_timeout)
$(fdc_check_id):
cmp ch, ds:[0045]
jne $(fdc_id_error)
cmp dh, ds:[0046]
jne $(fdc_id_error)
cmp byte ds:[0048], 02
je $(fdc_prepare_dma)
$(fdc_id_error):
mov ah, 40
stc
jmp near $(fdc_complete)
$(fdc_prepare_dma):
cmp byte ss:[bp-01], 03
jne $(fdc_dma_setup)
; Write: copy caller bytes into the same bounded DMA sector used by reads.
push ds
mov ax, ss:[bp-08]
mov ds, ax
mov si, ss:[bp-0a]
mov ax, 9fc0
mov es, ax
xor di, di
mov cx, 0100
cld
rep:
movsw
pop ds
$(fdc_dma_setup):
; Mask the channel while programming its one shared address/count flip-flop.
cli
mov al, 06
out 0a, al
xor al, al
out 0c, al
out 04, al
mov al, fc
out 04, al
mov al, 09
out 81, al
xor al, al
out 0c, al
mov al, ff
out 05, al
mov al, 01
out 05, al
mov al, 86
cmp byte ss:[bp-01], 02
je $(fdc_dma_mode)
mov al, 8a
$(fdc_dma_mode):
out 0b, al
mov al, 02
out 0a, al
xor al, al
out d4, al
sti
; One sector per command: software alone advances the caller CHS and buffer.
sub sp, 000a
mov si, sp
mov byte ss:[si], 46
cmp byte ss:[bp-01], 02
je $(fdc_command)
mov byte ss:[si], 45
$(fdc_command):
mov cx, ss:[bp-04]
mov dx, ss:[bp-06]
mov al, dh
shl al, 01
shl al, 01
mov ss:[si+01], al
mov ss:[si+02], ch
mov ss:[si+03], dh
mov ss:[si+04], cl
mov byte ss:[si+05], 02
mov ss:[si+06], cl
mov byte ss:[si+07], 1b
mov byte ss:[si+08], ff
mov cx, 0009
call $(fdc_send)
lea sp, ss:[si+0a]
jnc $(fdc_data_result)
jmp near $(fdc_timeout)
$(fdc_data_result):
call $(fdc_result)
jnc $(fdc_copy_read)
jmp near $(fdc_complete)
$(fdc_copy_read):
cmp byte ss:[bp-01], 02
jne $(fdc_next_sector)
push ds
mov ax, 9fc0
mov ds, ax
xor si, si
mov ax, ss:[bp-08]
mov es, ax
mov di, ss:[bp-0a]
mov cx, 0100
cld
rep:
movsw
pop ds
$(fdc_next_sector):
inc byte ss:[bp-0c]
mov al, ss:[bp-0c]
cmp al, ss:[bp-02]
je $(fdc_success)
add word ss:[bp-0a], 0200
mov cx, ss:[bp-04]
mov dx, ss:[bp-06]
inc cl
cmp cl, ds:[00ac]
jbe $(fdc_store_chs)
mov cl, 01
inc dh
cmp dh, 02
jb $(fdc_store_chs)
xor dh, dh
inc ch
$(fdc_store_chs):
mov ss:[bp-04], cx
mov ss:[bp-06], dx
jmp near $(fdc_sector)
$(fdc_success):
xor ah, ah
clc
jmp near $(fdc_complete)
$(fdc_timeout):
mov ah, 80
stc
$(fdc_complete):
mov ds:[0041], ah
jnc $(fdc_return)
; Abort any incomplete command/DMA. A later operation must initialize again.
push ax
cli
mov al, 06
out 0a, al
mov dx, 03f2
xor al, al
out dx, al
mov al, 1c
out dx, al
mov byte ds:[003e], 00
sti
pop ax
stc
$(fdc_return):
pushf
pop dx
and dx, 0001
and word ss:[bp+06], fffe
or ss:[bp+06], dx
mov al, ss:[bp-0c]
pop es
pop ds
pop di
pop si
pop dx
pop cx
pop bx
mov sp, bp
pop bp
iret
; DS=0040 throughout the helpers. BX holds mask/expected; DX=0 selects
; the IRQ flag, otherwise it is the MSR port. Two BIOS seconds and a finite
; poll budget bound even a stopped timer; this is a BIOS policy, not chip timing.
$(fdc_wait):
push cx
push si
push di
mov si, ds:[006c]
mov cx, ffff
mov di, 0040
$(fdc_wait_poll):
or dx, dx
jnz $(fdc_wait_port)
mov al, ds:[003e]
jmp short $(fdc_wait_sample)
$(fdc_wait_port):
in al, dx
$(fdc_wait_sample):
and al, bh
cmp al, bl
je $(fdc_wait_ready)
mov ax, ds:[006c]
sub ax, si
cmp ax, 0024
jae $(fdc_wait_timeout)
loop $(fdc_wait_poll)
dec di
jnz $(fdc_wait_poll)
$(fdc_wait_timeout):
stc
jmp short $(fdc_wait_done)
$(fdc_wait_ready):
clc
$(fdc_wait_done):
pop di
pop si
pop cx
ret
$(fdc_put):
push ax
push bx
push dx
mov bx, c080
mov dx, 03f4
call $(fdc_wait)
pop dx
pop bx
pop ax
jc $(fdc_put_done)
push dx
mov dx, 03f5
out dx, al
pop dx
$(fdc_put_done):
ret
$(fdc_get):
push bx
push dx
mov bx, c0c0
mov dx, 03f4
call $(fdc_wait)
jc $(fdc_get_done)
inc dx
in al, dx
$(fdc_get_done):
pop dx
pop bx
ret
$(fdc_send):
push si
push cx
$(fdc_send_byte):
mov al, ss:[si]
call $(fdc_put)
jc $(fdc_send_done)
inc si
loop $(fdc_send_byte)
$(fdc_send_done):
pop cx
pop si
ret
$(fdc_initialize):
; CMOS drive kind selects the configured logical format, not a chip exception.
mov al, 10
out 70, al
in al, 71
and al, f0
mov word ds:[00ac], 4f12
mov bx, 0000
cmp al, 40
je $(fdc_reset)
mov word ds:[00ac], 4f0f
cmp al, 20
je $(fdc_reset)
mov word ds:[00ac], 4f09
mov bx, 0002
cmp al, 30
je $(fdc_reset)
mov word ds:[00ac], 2709
cmp al, 10
je $(fdc_reset)
mov ah, 01
stc
ret
$(fdc_reset):
push bx
cli
mov byte ds:[003e], 00
mov al, 06
out 0a, al
mov dx, 03f2
xor al, al
out dx, al
mov al, 1c
out dx, al
sti
mov bx, 8080
xor dx, dx
call $(fdc_wait)
pop bx
jc $(fdc_initialize_timeout)
mov dx, 03f7
mov al, bl
out dx, al
mov di, 0004
$(fdc_reset_sense):
mov al, 08
call $(fdc_put)
jc $(fdc_initialize_timeout)
call $(fdc_get)
jc $(fdc_initialize_timeout)
and al, fc
cmp al, c0
jne $(fdc_initialize_controller)
call $(fdc_get)
jc $(fdc_initialize_timeout)
dec di
jnz $(fdc_reset_sense)
mov al, 03
call $(fdc_put)
jc $(fdc_initialize_timeout)
mov al, af
call $(fdc_put)
jc $(fdc_initialize_timeout)
mov al, 02
call $(fdc_put)
jc $(fdc_initialize_timeout)
; 77 pulses may not reach Track 0 from cylinder 79: permit a second attempt.
mov di, 0002
$(fdc_recalibrate):
mov al, 07
call $(fdc_put)
jc $(fdc_initialize_timeout)
xor al, al
call $(fdc_put)
jc $(fdc_initialize_timeout)
xor cx, cx
call $(fdc_finish_seek)
jnc $(fdc_initialized)
dec di
jnz $(fdc_recalibrate)
ret
$(fdc_initialized):
or byte ds:[003e], 01
xor ah, ah
clc
ret
$(fdc_initialize_controller):
mov ah, 20
stc
ret
$(fdc_initialize_timeout):
mov ah, 80
stc
ret
$(fdc_finish_seek):
push bx
push dx
mov bx, 0100
mov dx, 03f4
call $(fdc_wait)
pop dx
pop bx
jc $(fdc_seek_timeout)
mov al, 08
call $(fdc_put)
jc $(fdc_seek_timeout)
call $(fdc_get)
jc $(fdc_seek_timeout)
mov ah, al
cmp al, 80
je $(fdc_seek_failed)
push ax
call $(fdc_get)
pop bx
jc $(fdc_seek_timeout)
cmp al, ch
jne $(fdc_seek_failed)
and bl, fb
cmp bl, 20
jne $(fdc_seek_failed)
xor ah, ah
clc
ret
$(fdc_seek_failed):
mov ah, 40
stc
ret
$(fdc_seek_timeout):
mov ah, 80
stc
ret
$(fdc_result):
push cx
push di
mov cx, 0007
mov di, 0042
$(fdc_result_byte):
call $(fdc_get)
jc $(fdc_result_timeout)
mov ds:[di], al
inc di
loop $(fdc_result_byte)
; Intel ST0/ST1/ST2 -> BIOS status (IBM AT Technical Reference 5-28/29).
mov ah, 80
test byte ds:[0042], 08
jnz $(fdc_result_error)
mov ah, 03
test byte ds:[0043], 02
jnz $(fdc_result_error)
mov ah, 10
test byte ds:[0043], 20
jnz $(fdc_result_error)
test byte ds:[0044], 20
jnz $(fdc_result_error)
mov ah, 08
test byte ds:[0043], 10
jnz $(fdc_result_error)
mov ah, 40
test byte ds:[0044], 12
jnz $(fdc_result_error)
mov ah, 04
test byte ds:[0043], 84
jnz $(fdc_result_error)
test byte ds:[0044], 40
jnz $(fdc_result_error)
mov ah, 02
test byte ds:[0043], 01
jnz $(fdc_result_error)
test byte ds:[0044], 01
jnz $(fdc_result_error)
mov ah, 20
test byte ds:[0042], d0
jnz $(fdc_result_error)
xor ah, ah
clc
jmp short $(fdc_result_done)
$(fdc_result_timeout):
mov ah, 80
$(fdc_result_error):
stc
$(fdc_result_done):
pop di
pop cx
ret
