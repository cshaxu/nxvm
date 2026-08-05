# M5 T223: Keyboard Set-1 Delivery

## S1: Alt Chord Encoding

**Status:** complete. The previous candidate treated a chord as a raw Set-1
make/break stream, but that changed the established T209 `EDIT.COM` menu
semantics: Alt+F selected a menu rather than immediately opening it. The
legacy, profile-owned host modifier snapshot is retained for compatibility.

S1 preserves the existing host -> modifier snapshot + make-code -> KBC -> IRQ1
-> ROM -> BDA route, but changes ingress consumption to one event per
`core_machine` execution boundary. The old all-at-once drain collapsed
Alt-down, F, and Alt-up into one guest instant; now guest IRQ and application
work can observe the modifier state between host events. T210's ROM `INT 09h`
also regains the exact T209 translation rule: while the snapshot reports Alt,
a make code becomes BIOS extended `AL=00h, AH=scan` rather than ordinary ASCII.
Crucially, a zero ASCII byte is a valid extended BIOS key, not an absent key:
the ROM queues every delivered nonzero scan code, including arrows and function
keys (`AX=scan:00`) as well as Alt chords. Window handling consumes `WM_SYSCHAR`
so the host menu cannot reclaim an Alt chord.
The candidate keeps T209-compatible immediate menu behavior as the required
manual acceptance result; no host shortcut, second input route, or KBC/PIC
ownership change is introduced.

Verification: the real-FDD `EDIT.COM` launch/keyboard smoke,
host-cancellation smoke, transport ordering smoke, and the complete 56-test
current CTest matrix pass. The owner manually confirmed `Alt+F`, `Alt+H`, and
menu-arrow behavior using `fdd.img`. The task artifact is
`build/output/nxvm_0_5_0223.exe` with SHA-256
`7A7288B02A2B9C476710BB9B9F8E0B172F19C900FBEDDBD365BE932444672EB0`.

This task changes input delivery timing only: it does not alter the
core-owned elapsed tick scheduler or PIT/DMA/VADP advancement order. Break
codes, E0/E1 prefixes, and typematic remain later scoped work.
