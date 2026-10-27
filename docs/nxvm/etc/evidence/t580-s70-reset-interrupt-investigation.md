# M5 T580 S70 — Reset, timer and board-route investigation

## Baseline and candidate 4Fh route

S69 establishes that the selected firmware repairs the CMOS checksum in the
same machine, then reaches a distinct `102` path after CPU reset. The observed
IRQ0 edge occurs while the BIOS is counting refresh transitions; its PIT
period agrees with the existing source-labelled nominal clock model. It is
not evidence that a previously latched PIC request survived initialization.

The local Open-Source-PC110 commit `02ccf2e8e19b298a2cf04c5d430278eb087cc368`,
`Discovery/Service-Manual/readme.md` section 8.4, describes port `4Fh` as an
RTC-related index or shadow and discusses an extended bank. Current NXVM
PC110 construction has no `4Fh` receiver. This is a candidate route gap,
not yet evidence for the `102` failure: the firmware's ordinary RTC helper
writes the same NMI-masked value to `70h` and `4Fh`, then uses `71h`; those
accesses produced the actual expected 128-byte CMOS configuration and checksum
observations in S69.

The reference descriptions are not interchangeable. PC110-EMU commit
`81235b5fdf7d17ae180fc5cb6148e44bcbb87b4c` routes `71h` through a distinct
`4Fh` latch only under its particular Setup mode; its ordinary RTC path masks
the `70h` index to seven bits. PC110-QEMU commit
`bae390ebf21665ec97860d28e26ec46f1fe8f4e9` retains a `4Fh` latch, but its
ordinary `71h` implementation also selects the seven-bit `70h` index; nearby
comments describe different hypotheses. Neither implementation demonstrates
that changing this route fixes the timer test. Do not add a 256-byte bank or
replace the RTC contract on the strength of this ambiguity alone. The route is
retained for a later consumer-specific probe, including Setup access patterns.

`F000:603E` in the captured post-reset trace is the guest's `REP STOSD` memory
test, not an idle or timer callback. The later `F000:EA86` loop samples port
`61h` refresh bit 4; a PIT0 edge there is ordinary in the current model. The
specific PC110 reset/IRQ interaction is still under investigation.

The x64 real-firmware replay traced the first refresh wait following the
relevant PIC initialization to `F000:6C8C`. The BIOS checks KBC status port
`64h` bit 0, and when no output byte is present it loads `CX=19D6h`, calls
the `EA80`/`EA86` refresh delay, then checks again. The wait is guest-directed,
not the PIT self-test itself. Its earlier path selects EC block register
`04h` through `35EAh/35EBh`; the current App returns `84h` from the pinned L2
live capture, which makes that path eligible. `F000:6CF0` would read the
KBC data byte if one arrives. Neither a missing external keyboard byte nor a
static captured EC value alone proves a defect: absence may legitimately
take the timeout. Forcing a byte or bypassing the delay would manufacture
guest input and is not an acceptable repair.

The BIOS configures the keyboard-controller command byte as `6Dh` before the
wait; if an output byte appears, the subsequent code reads it and checks for
`10h`. The PC110-EMU reference's generic command-byte write does not itself
enqueue that value. NXVM's existing controller can start keyboard BAT on
specific documented enable/inhibit transitions, but the captured execution
does not establish that the PC110 keyboard should produce `10h` in this
state. The pinned Pluto research identifies EC index `04h=84h` as a repeatable
live reading, but does not establish the semantics of these two bits. Both
the EC status and any keyboard startup behavior therefore require a real
consumer or source-backed model, not a BIOS-address special case.

## Candidate PIC correction and regression impact

Intel 8259A data sheet 231468-003, pages 17–18, Figure 9 and the edge/level
triggering text describe a transparent request latch while acknowledgement
is not frozen and require the IR input to remain high until the first INTA;
an input removed prematurely yields default IRQ7 on acknowledgement. The
current X86 NXVM PIC cleared a falling input only in level mode. The PC110
trace shows a masked IRQ0 followed by a PIT mode write that makes its input
low before unmask/acknowledgement. That combination is a source-backed
candidate for the `102` path; firmware passage after correction remains the
required proof.

Independent L2 corroboration exists in local PCjs commit
`c7f21b4fa2bdedac3d5c73094a6402fdc8b24c70`,
`machines/pcx86/modules/v2/chipset.js` near `outTimerCtrl`. Its maintainer
describes the same failure pattern in 5150/5160 firmware: PIT0 is active
before the timer self-test, and a pending IRQ0 is incorrectly delivered after
unmask. That implementation clears its timer request when PIT0 output is
driven low by reprogramming. PCjs itself marks broader mode behavior as a
question; the candidate here follows the Intel request-latch input transition
generally rather than copying its unconditional timer-control shortcut.

The candidate clears the request on a high-to-low input transition in both
trigger modes, retaining the existing edge assertion and default-IRQ7 paths.
Focused PIC contract and command tests pass on x64 and x86. A broader x86
X86-NXVM unit run currently fails 52 of 233 cases, many of which inject an
immediate asserted/deasserted pulse and expect subsequent IRQ delivery. This
is a real regression against existing tests, not a passing qualification.
Whether each test models an invalid short pulse or identifies a production
source that must hold an interrupt through acknowledgement must be resolved
before accepting the generic change. A real TFT BIOS replay with the candidate
on x64 directly observed the prior `F000:68FF` error branch fall through,
then reached processor resets 7 and 8. It did not reach the requested INT 19h
checkpoint: it later stopped at the SMM/power wait around `00039333` after
180,852,109 retired instructions and the diagnostic exited 1. This proves
the prior `102` path was bypassed by the chip behavior in this run, not that
PC110 setup, resume or boot is complete. A separately sourced L2 PCjs comment
also identifies the same PIT/PIC interaction for PC/XT BIOSes.
Static search found many Board tests calling assert immediately followed by
deassert on the same source before execution or acknowledgement. No directly
adjacent assert/deassert pair was found in the X86 NXVM or MyPC110 production
sources; this search does not prove that separate callbacks cannot form a
short pulse and does not replace end-to-end device tests.

After moving the deassertion after execution in 27 existing single-run test
cases, the x86 X86-NXVM unit suite reports 207 passed and 26 failed out of
233. The PIC IRQ lifecycle case was subsequently corrected to hold each
source until acknowledgement and passes independently on x86. Three more
single-run cases were adjusted after that suite and await rebuilding and
verification. These results do not yet qualify the final tree. Remaining
failures included multi-stage interrupt tests, PIC priority and the keyboard
controller. Follow-up targeted x86 runs passed the prior failures after
keeping the test IRQ asserted until acknowledge or until the pending-state
observation. In particular, a KBC test read the output byte, dropping IRQ1,
before expecting the PIC to acknowledge it; the test now acknowledges while
the line is asserted. This is test-path evidence and does not replace the
complete final-tree suite or device/firmware integration.

The complete X86-NXVM unit selection subsequently passed 233/233 on each
host width, using the stable release Ninja trees. The x64 real-firmware FDC
reset integration passed in 49.03 seconds; the real-firmware boot handoff
passed in 238.94 seconds. On x86 the real-firmware FDC reset passed in
142.88 seconds. The initial x86 boot-handoff CTest exhausted its 360-second
budget and remains a failed run. A subsequent direct run of the exact same
test executable and `--resume-boot` argument exited zero after genuine F12,
RSM and INT 19h at 64,823,720 retired instructions. Progress timestamps
show approximately 719 seconds elapsed. The x86 test registration therefore
uses a 1080-second containment budget; the x64 budget remains 360 seconds.
The direct success does not retroactively turn the earlier CTest timeout into
a pass. The registered x86 CTest was subsequently rerun with the revised
budget and passed in 745.24 seconds. Both handoff results establish the
existing checkpoint, not the later post-Setup guest boot.

The existing versioned PC build, INI identity and floppy-boot receiver
checks passed for all four x64 profiles and, on x86, default NXVM, My5160
and My5170. The x86 DeskPro identity check passed, but its initial boot
receiver reached its own 300-second terminal with FDC E6 still transferring;
that initial run is not counted as a pass. An extended direct execution with the same candidate,
INI and media reached `installer-running` and exited zero in approximately
357 seconds; its output SHA-256 is
`CE6DC6E6BDDFA1D898C94DC04B330A9B6420A5C3DA9F0432A39148A85EA5A896`.
The x86-only registered budget is revised to 480 seconds, with a 490-second
CTest containment; the formal rerun passed in 370.32 seconds, with output
SHA-256 `1EA06CD3F6617C08C982AAFFE76ECD5EF26901C60465A81DDA2890CF811FDADE`.
Thus all eight product/profile receiving rows passed once within their final
registered bounds. The x64 budget and guest timing remain unchanged. Earlier T533 and T538 evidence also records
this profile timing out while transferring; that historical similarity alone
does not prove the current result is unrelated to the PIC change. No desktop
or full PC110 product qualification is implied by these integration checks.

For a same-host control, the existing x86 receiver was rebuilt once with the
pre-S70 PIC source from HEAD; the candidate source and build were restored
before executing the copied control. The same INI, media, executable test
and 300-second budget also ended at `TERMINAL-TIMEOUT`: the control reached
336,921,566 guest ticks, while the candidate reached 327,278,178; both were
still executing the BIOS/FDC path. This confirms that the timeout also occurs
without the candidate, but does not establish precise performance equivalence
or prove that every DeskPro behavior is unaffected. The control binary and
output SHA-256 values are
`C8832F0ECEBF720FE20A7A01614E1A99C34078BABE760C37E89F42AD7685D6E8` and
`FBA6172CC192A1FA2997C77233BB40C764D1EA64325F06DB790648FF237D118E`;
the candidate matrix log SHA-256 is
`75A21D32453FDABA489E78C2386140C150A5F520ED22A629A01E553A68E6DCF0`.

The existing PC110 diagnostic's progress output is now enabled before power
restoration as well as afterwards. At approximately 355 seconds the follow-up
x86 run was still executing the guest's finite F000:6440 keyboard-response
wait, which repeatedly reads the port-61 refresh bit. It later reached the
real power-restoration path after approximately 36 million instructions and
completed its handoff. The extra output changes no machine or firmware
behavior.

The x86 direct-run output SHA-256 is
`AEA9B9B8E7D1F4456DF1EAC71235FECFF35B174074945FA54F4A3C33E859A6C3`;
the x64 CTest output SHA-256 is
`5D8C11F1188C98A3770819F661BF3B6D5BA7C23B3D620CBB1F5FBFC1726A32BD`.
The original x86 timeout output SHA-256 is
`F7209F11EE1C285EBE122B1B342DF5BCAB4A05B9E06689051DE9FE7A69940DED`.
The subsequently passing x86 CTest output SHA-256 is
`86767D5EDD54C2913F536C50CBAA7AC258613DCA48C45E6CC2D33536520168B2`.
The x64 and x86 full X86-NXVM unit log hashes are respectively
`BE0B1C617D523D5F773A6517EA6D75BDCA1B7F3655299D75E97BFD0C60F0A534`
and `ED79031DA6582B15FD168F2E3478E0CFAE46C9FD8925CE17A9DD1BDD9CFB3C5E`.

After all owned processes exited, the existing safe cleanup tool previewed and
removed 78 task scratch files (5,835,228 bytes). The extracted comparison
oracle was reviewed by SHA-256
`7128F7C5D4907A8C7F1D7B8C0D64F8617CDA43464F36599D21ED7BBCCAB8FE8B`;
stable Ninja and ccache trees and user media were retained. Documentation
governance and `git diff --check` passed. This batch did not run the full
repository, native desktop, DSTN, or post-Setup PC110 guest qualifications.
