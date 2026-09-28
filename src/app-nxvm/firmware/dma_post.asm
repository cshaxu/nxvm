; Copyright 2012-2026 Neko.
; Recovered guest assembly; source provenance is recorded in README.md.
; init vdma
mov al, 00
out 08, al ;
out d0, al ;
mov al, c0
out d6, al ;
