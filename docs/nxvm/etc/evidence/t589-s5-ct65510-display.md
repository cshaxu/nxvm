# T589 S5 C&T display hardware evidence

The HP System ROM 1.0 option image and I/O trace are recorded in the
[S4 display handoff](t589-s4-am386sxlv-management.md#display-handoff).
The option image contains the 65510 identifier and programs the associated
extension registers. HP service publication 5965-0244 describes a 640 × 480
LCD with 16 gray levels. This supports implementing the 65510 firmware
contract for OB300. The exact physical marking and stepping remain unknown.

## Device boundary

The [C&T 65510 preliminary data sheet, revision 0.7](https://www.nrjfwax.vgamuseum.info/images/doc/chips/f65510.pdf)
describes XR00 revision encoding on p. 74, CPU paging on pp. 77 and 79,
palette conversion on pp. 8 and 76, and Panel Off on p. 93.

The shared C&T controller reuses the existing VGA memory, four latches,
indexed registers and copied frame capture. Construction selects the 65510
or 65535. The 65510 exposes XR00 revision zero and the 65535 keeps the
MyPC110-observed C1h. MyPC110 selects the 65535 explicitly.

XR0B enables the XR10/XR11 window mapping. The chip applies a separate bank
base on each side of a dual-window boundary, including within a single
multi-byte access. The planar bank step is 1 KiB per plane, equivalent to
4 KiB in the packed CPU address space. Wrapping bank addresses to the
implemented 512 KiB store is a bounded model. Linear selection disables the
legacy aperture. External linear decode is part of board wiring.

The panel capture supports the documented RGB weighting choices. It maps the
result to 16 steady gray levels for host presentation, an L2 model of the
visible result. XR52 Panel Off blacks the palette and hides the text cursor
while CPU memory and register access can continue. Pin-level power sequencing,
frame-rate control and panel compensation remain open. The 65510 cursor uses
an L2 nominal 16-VSync half-period and honors the XR0E blink-disable bit. The
65535 retains its XR60 cadence. XR0C supplies the upper text display-start
bit when XR04 enables the extended CRTC counter. The text test writes
distinct data to the two possible starts and verifies both selections.

The 65510 also captures standard 256-color doubleword scanout. CR01 and
AR10 determine width, CR09 reduces repeated scan lines, and CR13/XR0D
determine row stride. The test drives chain-4 CPU writes at a nonzero
display start and verifies plane order, row stride and the full DAC index.
The data sheet describes the 256-color assembler on p. 69 and the CRTC
addressing on pp. 54–57. These are logical guest frames. LCD placement
and physical compensation are separate from pixel acquisition.

The revision 0.7 XR5B register overview shows a reset value of 71h, while
the detailed description on p. 99 states 81h. The current model follows
the register overview for readback. Firmware-relevant sequencing cannot
be derived from this conflicting reset value alone.

## Working-tree verification

The cached x64 and x86 release trees each passed the C&T chip contract and
the MyPC110 machine construction test. The chip test checks read-only
identity, both dual-window boundaries, packed CPU lane selection, legacy
decode disable, and a 640 × 480 planar frame with palette changes and Panel
Off recovery. Both X86-NXVM manifests and the production component-boundary
check passed. Before the packed-scanout change, the MyPC110 real-firmware
FDC-reset, resume-boot handoff and cold-boot handoff tests all passed on
both host widths. The packed capture is enabled only for the 65510.

The final cached unit aggregates passed 716 of 716 registrations on
each host width, in 219.71 seconds on x64 and 220.34 seconds on x86.
Receiver publication is separate. The display firmware's use of panel
compensation and board straps remains a later board-level verification
requirement.

| Verification log | SHA-256 |
| --- | --- |
| x64 unit aggregate | `7b12af8083c59a2beb1002420eb14b4b228eee2bcdb674612813e81835a491c7` |
| x86 unit aggregate | `86c1c2b75e952309b267727fb04a06f01770c0fa16b2360a6dce24f014b68f8e` |
| x64 MyPC110 real-firmware integration | `9b02d99313355d5206943ee8afb8066355d5aaa3735221acdd1a0d90afb80792` |
| x86 MyPC110 real-firmware integration | `3834402c6722809a0500b17ad8ea3599c994e059ec75dc150d7ea83feca8cbc1` |

The repository's specialized gate batch also exposed two existing NXVM
timing-verifier failures. The inventory still searches for the old
`insTable` tables removed by the T582 switch-dispatch consolidation.
The seam checker looks for `x86_core_cpu_timing_select(context,`, but
the current selector takes only `context` and still precedes
`if (dispatch_ready) ExecInt(context)` in the retirement body.
Neither script nor its CPU input changes in S5. Carry these two checks to the
T589 hardware qualification batch without changing CPU timing to satisfy
the stale probes.

## Input handoff

HP service publication 5965-0244, pp. 69–70, lists 8042 auxiliary-interface
POST operations, including command-byte writes, write-aux-device and
auxiliary reset. The table covers several models, so this alone does not
prove the exact keyboard controller silicon in the OB300.
The System 1.0 card's `OBMOUSE.COM` calls BIOS INT 15h AX=C200h with BH=0
and BH=1, at file offsets 26BDh and 2932h. The existing 8042/PS2 contracts
are candidates to qualify in S6. The original guest BIOS must still be
exercised through those functions when the HP board is wired. The separate
`OBMOUSE.DRV` includes HP-specific entry names, which do not establish a
different wire protocol by themselves.

## Published receivers

The cached release script published all seven products for both widths.
An independent PE-header check confirmed each machine type and matched the
file hash to the publisher output. Existing product versions were retained.

Publication and PE-check log SHA-256: a91b179e8cc0f7194d2a448577657d5294765b85fcf6877ed413842258c38bd6.

| Product | Width | Filename | SHA-256 |
| --- | --- | --- | --- |
| nxvm | x64 | nxvm_0_5_0582_x64.exe | 5e8bf12bfd810160de1177091017968453be54307e2e5842a82c0a21f7cb064d |
| nxvm | x86 | nxvm_0_5_0582_x86.exe | 1b9f8115d35f4ceb1bf2fac06e07d5696521be1285b8f176761f6af0a47a6e5c |
| my5160 | x64 | my5160_0_5_0582_x64.exe | 321523f1ce308e198603eaf3aec47c6a3ea1a0c49b9b8e631feb68d7ba4019df |
| my5160 | x86 | my5160_0_5_0582_x86.exe | 7ba29947c94c9bb035ad46b931e210ad3a5cff34c99c772a691c47033c3bc0f9 |
| my5170 | x64 | my5170_0_5_0582_x64.exe | a7f6cc693393ba25633c80cc5e3701502b58b0b9eca4e619e4ccf848c9cf9161 |
| my5170 | x86 | my5170_0_5_0582_x86.exe | 8ed6f6c810242b14c0a8c16de7eb4bf0a8946b331e9af60d714491b2cc0d1ad7 |
| mydeskpro386 | x64 | mydeskpro386_0_5_0582_x64.exe | ad5aec441cb3fb57370d076364df3a930e10a081494c8a798431ef3aa184d062 |
| mydeskpro386 | x86 | mydeskpro386_0_5_0582_x86.exe | 73d7617d65e8e0f7eaad8d61e771d29cb7d835a1b8e4df03f90e71b0976bd097 |
| mypc110 | x64 | mypc110_x64.exe | af61e773b76b1046375e24999cade5143d09cfccb2eeb64d81e37f1639a86a31 |
| mypc110 | x86 | mypc110_x86.exe | e24383e9b515a995e3aab74e305b16b3b79b780f74e0920ae5e81d8489a72cde |
| mynes | x64 | mynes_0_0_0044_x64.exe | c0caea48f5a86b7552bc50c173cf1f70ed6ce38c7419c553a917f92aab075546 |
| mynes | x86 | mynes_0_0_0044_x86.exe | 21cea936b54b2589a2f318a2263f5c59e2125e221ffb4a375a4fff6d1f027204 |
| myraidenii | x64 | myraidenii_0_0_0007_x64.exe | 73acc9108752740753b3189f6a01f74068681c6a34479d1837c5a73f2f38eb7f |
| myraidenii | x86 | myraidenii_0_0_0007_x86.exe | 60d2350086174ba7e2ce3776c037ba4cc192c4b824f973196f6edbc79daf71db |
