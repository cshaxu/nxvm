; Copyright 2012-2026 Neko.
; Recovered guest assembly; source provenance is recorded in README.md.
; init pic master
mov al, 11 ; icw1 0001 0001
out 20, al
mov al, 08 ; icw2 0000 1000
out 21, al
mov al, 04 ; icw3 0000 0100
out 21, al
mov al, 11 ; icw4 0001 0001
out 21, al
; init pic slave
mov al, 11 ; icw1 0001 0001
out a0, al
mov al, 70 ; icw2 0111 0000
out a1, al
mov al, 02 ; icw3 0000 0010
out a1, al
mov al, 01 ; icw4 0000 0001
out a1, al
