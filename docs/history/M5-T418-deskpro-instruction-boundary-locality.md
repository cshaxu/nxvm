# M5 T418: DeskPro Instruction-Boundary Locality

T418 consumes the next bounded CPU locality receiver. Original D3PE source says
an inserted CPU idle state ends a page-hit cycle. Since Core does not expose a
physical overlap/idle phase, T418 uses an explicitly generic-AT conservative
boundary: each new instruction round clears the prior locality key. It must not
claim that every instruction boundary was an original D4 idle state.