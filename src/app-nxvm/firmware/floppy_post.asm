; Copyright 2012-2026 Neko.
; POST uses the same guest reset/initialization service as INT 13h clients.
xor ax, ax
xor dx, dx
int 40
