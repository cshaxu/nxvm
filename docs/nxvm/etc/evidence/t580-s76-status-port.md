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

A longer bounded x64 continuation executed 400 million instructions after
the loader. It passed the earlier timer sample but then repeatedly entered
0048:023E and 3000:8000, with the delivered-exception count rising above
290,000; the Core reported no terminal fault. At the final sample the BIOS
tick had advanced to 06B0h and no new display frame was produced. This is a
reproducible later startup failure, not merely a too-short timer budget; it
does not identify the responsible exception or owner. The long-run log
SHA-256 is `7cf7015f0c462ae2388890b5a6deb638b9b447f6f274e55af6310b85aa6e3e64`.

A bounded storm probe stopped at 1,000 delivered exceptions. Its most recent
exception is #GP with IDT error B2h at the VM86 instruction `INT 16h`, with
AH=01h, followed by a conditional branch. This is the BIOS nonblocking key
availability query. It supports the hypothesis that guest software is waiting
for input, but does not prove which key or whether a display is visible. The
display provider reported no new frame at that stop; that is not evidence of
the current contents of the last visible frame. The exact probe log SHA-256 is
`df14bc5d7fba12dc2e4f01ba9bcd3cafd35b6a2962b25cb5f9ff8475e03caa13`.

The F65535 graphics snapshot implementation currently recognizes only VGA
mode 12; other modes return unsupported. The lack of a produced frame is
consistent with a possible unsupported mode but does not identify the active
register configuration. That remains a separate scanout investigation.

An active follow-up will send one Enter make/break through the existing
Machine input driver when the guest first executes its BIOS key-availability
query, then collect the outcome and the F65535 register banks after guest
execution stops. A just-started earlier version that waited for 1,000
exceptions was terminated before completion so this equivalent observation
need not wait through the extra guest polling; it is not counted as a pass.
Register inspection occurs only after guest execution ends.

The x64 input probe sent Enter at an earlier `INT 16h/AH=01h` query at
B3AC:0A8B, not the later 0FF2:0735 query. It continued into the timer
path, so this is not proof that the later wait consumed the key. At the
stop the actual graphics registers included CRTC horizontal display end
4Fh, vertical display end DAh with overflow 3Eh, and offset 40h; graphics
register 6 was 05h. The current capture therefore rejects the selected
planar configuration: the programmed dimensions are 640 by 475 with
128-byte stride, not the hard-coded 640 by 480 with 80-byte stride. This
is source and register evidence for an incomplete scanout receiver; it is
not yet a verified display fix. The probe log SHA-256 is
`bef1522368d8e6a73152b1d12d82292da825bd262a4edf77d31028a4e47900c7`.

The F65535 planar receiver now accepts bounded four-plane VGA geometries
using the CRTC's visible dimensions and pitch, while rejecting chained,
packed, oversized and out-of-plane configurations. The existing snapshot
kind was renamed to describe planar 16-colour content; its enum value is
unchanged. Existing mode-12 output remains covered, and the observed
640-by-475, 128-byte-stride case is tested with distinct second-row pixels.
The F65535, machine frame and PC110 display-route tests pass on both widths;
both affected manifests pass. Incremental full configured-tree builds pass
on both widths, as do all 219 tests labelled unit under X86-NXVM on each.
The 12 MyPC110 units also pass on each width. These counts do not claim all
repository tests or product integrations.

A real-firmware x64 replay using the same disk and the same early Enter
input now produces a 640-by-475 guest graphics frame containing the PC DOS
startup and Power Manager messages. Its previous corresponding sample
produced no new frame. This establishes a real scanout improvement, but not
completion of DOS boot or guest input consumption. `POWER:result=0` still
reflects the diagnostic's pre-loader exception-count invariant and must not
be presented as a full boot pass. The log SHA-256 is
`8cbb55fcfced91f8041f5d94ddca98bcb6341592fd622864603acb7fc7388a62`, and
the raw frame SHA-256 is
`0d9c178fa01458d66b9b6ae0c76218315977ba487e6b72399d43a1ea31bd84a8`.
The x86 real-firmware replay produced the same graphics frame byte for byte;
its log SHA-256 is
`24e8d5b165718445b19d75dab83225958e3650b26c9f8d963b7ee00a89d9e61c`.
Native Window and physical display colours have not been verified.

A further x64 probe observed the later `INT 16h/AH=01h` followed by a zero
branch at 0FF2:0735 and sent one Enter make/break through the Machine driver
at instruction 438,482,679. At the 50-million-instruction continuation
boundary, a real graphics frame shows PC DOS startup, mouse-driver failure
and the inking driver's first calibration request. This later screenshot
differs from the earlier frame but, because both time and an input changed,
it does not establish that Enter was consumed or caused the progress.
The diagnostic's exception-baseline result remains false; it is not a full
DOS or product pass. Log SHA-256:
`d58c9a52ee9bb5759b7b9c230049fe5955eacb59dcc65667a2a447cdfef27085`;
raw frame SHA-256:
`3a14f0e064ce15ff87a5df205c46ba7938d024f1d71663c94fb47ae45b3a5df4`.

The supplied disk's startup configuration invokes `INKDRV.COM`. The local
Open-Source-PC110 inking research describes a separate resistive digitizer,
distinct from the PS/2 pointing device, with an IRQ-driven 3-byte packet
interface. Current App code has no inking owner. Its implementation and the
mouse-driver finding require their own machine-device investigation; neither
was emulated or bypassed in this S.

All four existing PC product profiles were rebuilt for x64 and x86 after
the Shared frame-kind change. Their eight product/INI identity checks pass;
the PE machine types, lengths and hashes are in
[`t580-s76-pc-artifacts.json`](t580-s76-pc-artifacts.json). The rebuild did
not alter user INIs or external media. This is receiver validation, not a
full PC110 product release or desktop qualification.

The original failing log SHA-256 is
`dd392e4dfede9cdc0f1c6a1746339c01501f3cff55bb30efdeba3dfc6a418dd4`;
its raw guest frame SHA-256 is
`a6087ec5178c7619d8136de2aa159dde7161d56f9e4c3b899b7165935d0353d8`.
The source disk and oracle provenance remain as documented in S74/S75.
