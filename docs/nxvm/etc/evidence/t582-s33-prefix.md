# T582 S33 — single prefix classifier

**S34 setup correction:** The referenced S32 fixed-work comparison used empty
CMOS. The corrected product-path game probes below used the configured seed.
See [the correction](t582-s34-cmos-probe-correction.md).

Shared `c1b6f95b4` moves the existing CPU-private prefix classifier unchanged
from `cpu_instructions.c` to the existing private header as `static inline`.
Execution and timing keep one source definition, with no new state, public API,
decoder or timing behavior. The production source diff is 22 added and 24
removed lines; the X86 Core manifest updates two hashes. Repository source,
tests and CMake references were checked before removing the old external
symbol; no caller exists outside the owning CPU source.

The S32 fixed-work comparison is recorded in
[the investigation](t582-s32-execution-cost.md#candidate-one-inlinable-prefix-classifier).
Its matched 10-million-instruction startup samples favor the candidate on
both widths, but do not establish complete guest-state equivalence or
interactive improvement. The deployed PC110 x64/x86 EXEs grew by 15,289 and
15,800 bytes respectively. The full
[fourteen-artifact inventory](t582-s33-prefix-artifacts.json) records PE width and hashes; MyNES binaries
were unchanged and match the built artifacts. MyPC110 and MyRaidenII deployed
EXEs also match their respective builds.

## Qualification

The existing 33 targeted prefix, V30, timing, preview and retirement tests
passed on each architecture. The first x64 full qualification process exited
with `0x40010004` after 40 completed unit tests; this was an interrupted run,
not a completed suite or a recorded test failure. Its log was preserved.
The resumed run excluded precisely those 40 completed tests and passed the
remaining 672 x64 unit tests; x86 passed 712/712. Default integration passed
48/48 on each width, and selected 5160/5170/DeskPro integration passed 3/5/6
per width. Cached builds and the MyRaidenII selected-set startup and deployed
Console probes passed on both widths. The latter do not prove arcade gameplay.
X86 Core manifest and NXVM documentation governance checks passed.

## PC110 product-path sampling

The first baseline experiment incorrectly placed its EXE in the parent of the
copied CMOS files. MyPC110 resolves CMOS next to the EXE, so this run used an
unconfigured seed, displayed firmware errors 00173/00163 and did not enter
the menu. It is an invalid performance comparison; the script was corrected.

The corrected baseline and candidate both reached the game, but the initial
two-pixel game check accepted a baseline frame before the score label was
drawn. The candidate frame did show the game grid and score label. A later
baseline run required two pixels in that label and took 245.88 seconds; the
candidate run took 265.54 seconds, and a separate candidate diagnostic took
about 238 seconds. Their sampled game images also had portions of the top UI
still drawing. These asynchronous, differently timed runs do not prove a
performance gain or regression, a fully completed frame, complete-state
equivalence, or responsive native/RDP gameplay. The PC110 usability objective
remains open.

Task-owned log SHA-256: interrupted x64 qualification
`486B924A86F436058F1C11C10802F107F2A06DAED096058B6D93A8C25663F079`;
resumed dual-width qualification
`345DE6767A40E96C5979CFC3D3C10216C3F69CB71729A5272D3AFA6DC08BD0C7`;
targeted x64/x86
`FE086B7306A6C0D22D2488B4C7810CAF0BD293F388C064A4DEF16D1D9B785613` /
`0FC0D55EEECCF33848CA150BE9DCBCF2154BE4D0FB4A03D6A25EB72FB415A12A`;
external receiver
`D829B6B9159B4FC435B77EA9664536C7FBF0141BD92E96C5ABA8C6E63833C585`;
corrected game pair
`3CC4B179812B98B84A52A4BEA91A85D7C5E30C58AC0BFE5E7489EC722C6A0BC9`;
stronger baseline
`CBC4CF51207FE7B3424238DA48058D523097184ED7895FC8DDC1203E51E6A212`;
candidate frame diagnostic
`EBBB6470BDD011ACE3BA71848D7DBACC4AABD72163F87453E692B79933599A48`.
