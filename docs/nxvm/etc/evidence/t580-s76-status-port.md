# M5 T580 S76 — PC110 loader display-status wait

## Observation and cause

The unchanged TFT firmware and task-local reviewed disk reached and executed
the matching loader. A bounded x64 run then continued 50 million guest
instructions. Its final frame was black, status was 0 and the Core reported
no terminal fault; the diagnostic's separate `POWER:result=0` reflects its
earlier invariant that exception counts stay at the pre-loader baseline, so
it is not a claim that a Core fault was observed. The last loop was at
035C:664C–664F, with DX=03DAh and bytes including `test al,09h; jnz` back
to the port input. The F65535 status implementation always returned bit 0
set (`01h` or `09h`), so this observed guest wait for bits 0 and 3 both
clear could never finish.

The independent PC110-EMU reference uses the same read-driven `01h`/`09h`
approximation; it does not disprove the observed wait. This is an existing
L2 compatibility model, not physical F65535 scan timing.

## Repair and validation in progress

The controller now represents its two existing read-driven phases as
active display (`00h`) and vertical retrace (`09h`). Attribute-controller
flip-flop reset remains in the same read path. No function, state, timebase,
host clock, firmware patch or guest-specific test is added to production.
The existing controller contract test asserts both values. Both host widths
built and passed that test, and their existing real-firmware FDC reset
integration tests passed in 46.78 s and 60.84 s respectively. Both affected
source and test manifests also pass their actual verifiers.

The x64 real-firmware replay reached the matching system loader and moved
beyond the former 03DAh loop. Afterward the guest delivered many exceptions
and entered another observed loop near 0FF2:6504, with DX=02F8h; the
50-million-instruction continuation ended at its budget with 566 delivered
exceptions and no Core terminal fault. Its final display copy produced no
new frame. This shows the original wait was bypassed by the fix, but it
does not establish a DOS boot or identify the next root cause. The new
failure is being traced separately. The approximation still does not model
raster timing.

The fixed replay log SHA-256 is
`098926113c2e87dab744493d2597c27c6a62063506588e67f7ae1839d75f540a`.

A subsequent x64 exception probe stopped at the first newly observed
exception: #GP (`mask=00002000`, error `00E2h`) at the firmware's `INT 1Ch`,
with VM set in EFLAGS. At that earlier point the guest text display reads
`Starting PC DOS...`. A VM86 monitor can deliberately use such faults for
interrupt emulation; this observation alone does not prove an x86 defect.
The log SHA-256 is
`eb94b66b50f0db185a4422afd97fe1e7fc1155f1abff5e72405dd44953e014d4`.

The subsequent context probe read the actual 1Ch IDT entry as eight zero
bytes, so the observed IDT-related #GP is consistent with the CPU's gate
checks. Its later loop is not repeated I/O at 02F8h: disassembly of the
captured bytes shows `mov al,[bx]; sub al,cl; cmp ah,al; jnc` repeating.
At the sample DS:BX addresses BIOS timer byte 0000:046Ch, CL is 32h, AH is
13h, and the BIOS timer is 0335h. Thus the elapsed timer value is three
ticks while the guest waits for more than 19; timer progress is present.
The previous 50-million-instruction continuation is too short to classify
that wait as a hang. This remains a bounded observation, not proof of a
completed DOS boot. The context log SHA-256 is
`307ad138841457d1c1ba9caab699f122cfa0a15a4ac096d92a327e7a1404eb56`.

The original failing log SHA-256 is
`dd392e4dfede9cdc0f1c6a1746339c01501f3cff55bb30efdeba3dfc6a418dd4`;
its raw guest frame SHA-256 is
`a6087ec5178c7619d8136de2aa159dde7161d56f9e4c3b899b7165935d0353d8`.
The source disk and oracle provenance remain as documented in S74/S75.
