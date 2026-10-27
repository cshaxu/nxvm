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
integration tests passed in 46.78 s and 60.84 s respectively.

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

The original failing log SHA-256 is
`dd392e4dfede9cdc0f1c6a1746339c01501f3cff55bb30efdeba3dfc6a418dd4`;
its raw guest frame SHA-256 is
`a6087ec5178c7619d8136de2aa159dde7161d56f9e4c3b899b7165935d0353d8`.
The source disk and oracle provenance remain as documented in S74/S75.
