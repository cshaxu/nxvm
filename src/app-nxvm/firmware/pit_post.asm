; Copyright 2012-2026 Neko.
; Recovered guest assembly; source provenance is recorded in README.md.
; init pit
mov al, 36 ; 0011 0110 mode = 3, counter = 0, 16b
out 43, al
mov al, 00
out 40, al ; initial count (0x10000)
out 40, al
mov al, 54 ; 0101 0100 mode = 2, counter = 1, LSB
out 43, al
mov al, 12
out 41, al ; initial count (0x12)
