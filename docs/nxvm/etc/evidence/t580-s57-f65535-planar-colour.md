# T580 S57 - F65535 planar access and programmed colour

Baseline: clean pushed 16aad0e66, accepted S56. The owner reports that real
ERROR and Easy-Setup backgrounds are pink, while the current output is blue.
The observed defect is not a verified whole-screen inversion. Both F65535
capture paths use fixed RGBI despite retaining Attribute, DAC and PEL values.
CPU aperture reads/writes also bypass the documented VGA latch pipeline,
and the App currently drops Core's observe-only fact.

## Before-change boundary and estimate

Repair these responsibilities in the existing F65535 owner. App forwards
Core's read-versus-inspect fact through the existing read signature. The chip
owns four read latches, register decoding, rotation/ROP/set-reset/masks and
one palette resolver consumed by both existing capture paths. No Lib or
Emulator change, public function/enum, renderer, clock, thread or queue.
Expected production +180--260/-35--60; existing tests +140--220/-10--25.
The packet retains separate panel, PCIC, storage and whole-product gates.

## Source review and mechanism

Original C&T 65535 Register Reference revision 2.0, 1994, external archived
`ct-f65535-register-reference-1994.pdf`, SHA-256
`ACB95E9BD56786227B6B9201B3BFF317ADBD224D1A587FBF8DB515FB5A648A91`,
is read directly. Printed pp. 65--71 define GR00--GR08, four read latches,
read compare and write modes; PDF p. 72/printed 68 is rendered and visually
checked. Printed pp. 74--78 define Attribute palette/mode/plane enable/Color
Select, pixel mask and the single DAC address/sequence; pp. 49--50 define
font-map selection and sequencer addressing. Exact register constants and
formulas come from this manual. Normalized six-to-eight-bit RGB expansion
is a presentation mapping, not a claim of analogue panel/DAC measurement.
The whole compatibility receiver remains functional L2 with no physical
panel or timing qualification. No external implementation is copied.

CPU aperture reads load four real latches at the addressed offset, then
select a plane or compare the cared-for colour bits. Inspection uses local
read values and leaves the latches unchanged. Writes apply map/address
selection, mode-specific CPU rotation/set-reset/ROP and bit mask; mode 1
copies the old read latches, and mode 3 uses rotated CPU bits as a mask.
The initial candidate omitted mode 3's logical operation; the final review
correction below supersedes that unsupported exception. Odd/even reads preserve
Read Map's high bit; writes can select both even or odd plane pairs.
Chain-4 selection is intersected with the sequencer Map Mask.

Both existing snapshot paths call one resolver: plane enable, Attribute
palette, Color Select/P54S, PEL mask, then actual six-bit DAC RGB. The fixed
RGBI table is deleted. Palette register changes use the existing generation
owner. DAC's two incompatible read/write indices become its documented
shared address and readable index, with one component sequence, read-mode
preincrement and wrapping; reserved data bits remain readable while RGB
uses the defined low six bits.

The same text-addressing audit finds packed active-height font addressing;
hardware glyph slots are 32 bytes. The existing snapshot now copies the
selected 8 KiB A/B maps using fixed slots and its existing secondary-font
contract. No new font storage or output interface is introduced. The old
fixture's 0410h write was an expectation of that wrong packed layout; it is
corrected to the actual A-glyph slot 0820h, with a separate bank selection
assertion rather than merely changing a display-height expectation.

## Focused verification and failed attempt

Both existing chip and App-route tests pass on x64 and x86. A small table in
the existing fixture covers all four writes with known nonzero read latches,
rotation, logical operations and bit masks. Additional existing-fixture
assertions cover compare/don't-care, map-mask/chain-4, odd/even high-plane
selection, DAC sequencing/wrap/mixed data accesses, plane/palette/P54S/PEL
colour mapping, generation, text slots and secondary font selection.
No new test executable/framework is added.

The first App inspection regression incorrectly used Core memory_read as
though it were observational. Phase assertions and source inspection show
Core already has a separate memory_inspect contract; memory_read deliberately
loads hardware. Correct the fixture to exercise a real Core read, Core
inspection of different bytes, then mode-1 write. The App's forwarded
observe_only flag now proves that the write retains the original latch.
The production fix is unchanged, no Core modification or weakened assertion.
The first partial failure is not counted as a passing App result.

The actual source dependency check finds F65535 is consumed by PC110, not the
four admitted PC boards. Rebuild all relevant targets in the stable graphs
and verify the eight published receiving identities; unchanged EXEs need no
gratuitous timestamp refresh. Four PC110 previews are the actual affected
artifacts and stay unpublished pending the original product gates.

## Initial candidate implementation and first real-frame result

The final C/H delta is production +122/-63 (net +59) in four paths and
existing tests +176/-1 (net +175) in two paths: total +298/-64, net +234.
Two private helpers own real palette and per-plane write computation; the
old shared read/write address helper is removed because read-map and write
map-mask have distinct documented selection rules. Four hardware latch bytes
are added; DAC's existing two index bytes are replaced, not duplicated.
No public function/enum, snapshot field, renderer, clock or thread is added.

All 712 units per width have final passing results: Types alone 30.25 s x64
and 27.52 s x86, remaining 711 at 52.03 s x64 / 55.23 s x86. Both specialized
gate runs pass (83 executed steps); strict matrix 498 retained rows/zero
deferred, global Types vocabulary 1,501 files. Both manifest/ownership gates
are included. The existing two focused tests also pass both widths.

Actual unchanged x64 TFT BIOS replay passes, ending at the fifth real
processor reset after 144,110,532 instructions, four known exceptions and no
Core fault. The changed planar read results affect guest drawing instruction
flow; do not require the previous run's exact instruction count as a UI
predicate. Loader comparison and actual input/reset predicates remain.
The ERROR frame and subsequent date/time frame are exported from the normal
Machine copied frame and visually inspected. The background is programmed
RGB BE9E96, not fixed cyan/blue. Text/icons and controls remain visible.
This proves ERROR/date-time colour consumption, not the normal main Setup
menu or post-save boot. ERROR still reports 08081/00173/00163.

| Actual x64 frame | Principal colours and counts | SHA-256 |
| --- | --- | --- |
| ERROR, 640x480 | 000000:137810, BE9E96:113730, FFFFFF:50030, 710820:4579, 140051:781, 555555:270 | D9B110C0CA4F495127B22FF64E80F38AE075ECEF22D4BDBD478802D09C49AFB5 |
| Date/time, 640x480 | BE9E96:177692, FFFFFF:106114, 710820:16954, 000000:4832, 140051:1374, 555555:234 | 0644016D2EE985038EFC43E069A260768DE91AC98231A7891A5463EBD45F8EDD |

The x64 real log SHA-256 is
`CE1B64A8846E6276CCFFA88F9A82B754522AE5CB302FFA0B96AEB14EE736A863`.
No screen-specific colour constant is added to production or to its generic
unit test. The existing integration's actual frame is inspected; no duplicate
integration framework or synthetic replacement of the BIOS is introduced.

Eight receiving profile builds and real adjacent-INI identity tests pass.
Reconfiguration relinks the eight admitted EXEs: a complete byte comparison
against S56 verifies that only COFF timestamp and PE checksum fields differ;
all section bytes and file lengths are unchanged. Include these authorized
relink identities in delivery rather than reverting rebuilt artifacts.
Four affected previews are rebuilt. The current
[artifact inventory](t580-s57-artifacts.json) verifies PE width, hash and
embedded runtime version 0.5.0580, without claiming a native version resource.

At this checkpoint the x86 original-BIOS replay is still running in the
existing bounded job, followed by both widths' registered FDC-reset and
boot-handoff regressions. S57 is not submitted, accepted or closed yet.
Owned background parent is integration PID 26980; its x86 native child is
PID 6316. Inspect their actual current identity/results on continuation, do
not infer terminal state from an empty buffered log. Scratch remains within
the 16 MiB budget; cleanup and final artifact/input review follow terminal
qualification. Desktop/RDP/physical-panel and whole-T gates remain unproved.

## Actual-change review correction: shared logical operation

The final mechanism review finds that excluding mode 3 from the GR03
function-select stage is not established by the manual's mode-3 prose;
the generic GR03 function description has no such restriction. The pinned
86Box `4fef696a4eead1d55a28d6ac0e5bd2864e5454da` independently applies
the same logical stage to modes 0, 2 and 3. Reference file
`src/video/vid_svga.c` SHA-256 is
`7FF93624B4BA10833DBB51B2B2E1795D02718B1EA234FAABAA86D3F59B6B0A54`.
This is a read-only L2 behavior reference, not imported source or a claim
that ambiguous manual prose proves silicon details.

Remove the extra branch and share that stage. Correct the mode-3 vector
to exercise actual XOR, and add one OR row to the same small table; together
with the existing direct-copy/AND cases, all four functions are covered
without a new test or framework. Both initial real replays did pass and
showed programmed colours, but they are old-candidate evidence, not final
source qualification. The initial follow-on regression is stopped while
running x64 boot-handoff; this deliberate interruption is not a test failure
or a claimed pass. Owned parent 26980 and native descendants are terminated
before relinking. Initial real logs/results/frames are renamed with
`initial-` under the same owned scratch directory.

Final corrected-source verification runs the two complete unit suites and
gates, then both real Setup replays and both registered firmware regressions
per width, sequentially to avoid contention against native deadlines. It
uses the same graphs, budgets and untouched external inputs. S57 remains
active until those final results, artifact identities and cleanup are proved.

## Final source review and qualification in progress

The corrected final C/H diff is production +122/-63 (net +59), existing
tests +177/-1 (net +176): total +299/-64, net +235. The added OR vector is
one table row, not a separate test. All public read callers use the changed
signature; the old fixed RGBI resolver and separate DAC read/write indices
are removed. Raw logical-VRAM access and CPU aperture access remain distinct
documented operations, not competing CPU paths. Both text and graphics use
the same colour resolver; App forwards the one Core inspection fact.

Actual source review confirms complete range validation precedes CPU
aperture reads/writes. Unsupported copied-capture geometry still returns
before target/cache mutation. Latch changes do not dirty visible frames;
palette/memory changes retain the existing generation owner. No ABI change
to Lib/Emulator, Core/CPU, other chips or snapshot layout. Four real latch
bytes are the only new persistent state. The two new private computations
replace one misleading shared address selector; no wrapper/manager, alternate
renderer, platform call or product-name branch is added. Existing compatible
PC input/control, source policy and owner inputs are untouched.

This is the named standard planar/colour/inspection batch, not every F65535
extension or physical panel behavior. The original display/panel row still
owns scanout enable/blanking, panel-specific paths and independently qualified
DSTN output; no broad all-VGA-complete claim follows from the six colours.
Other graphics modes retain their explicit unsupported boundary. The three
remaining firmware error codes are not assigned to devices by guesswork.

Corrected-source full units pass both widths: Types 28.90/29.62 s and the
remaining 711 cases 49.56/58.48 s (x64/x86). Both final specialized runs pass,
including 498 strict rows/zero deferred and the 1,501-file Types audit.
Final x64 actual BIOS replay passes with the same 144,110,532 instructions,
five resets/four known exceptions and no Core fault. Its ERROR/date-time
frame hashes equal the initial candidate's recorded hashes exactly; no
visible result was fabricated by the removed logical-operation exception.
Final x64 real log SHA-256 is
`A2BEF0A046E7C7ED4A512F5897CA804AB5F311AAC2EFFD7CE03D807168F9ED40`.

The one final background parent is PID 10628, command
`build/t580-s57-video/run-final.ps1`. At this checkpoint it runs x86 Setup
under the unchanged 900 s bound, then the two registered firmware cases per
width. Earlier PID references above are historical, not handles to restart.
No S57 acceptance, cleanup or submission is claimed before those jobs end.

## Final range correction and completed qualification

One safe query in the existing test reproduces a real x64 boundary defect:
offset 1 plus LIB_SIZE_MAX wraps the old unsigned sum to zero and incorrectly
reports OK. No unsafe read/write is used to reproduce the old code. Replace
the addition check with offset/capacity subtraction at the existing owner.
The four-map table then checks complete legal ranges, their endpoints,
outside starts and overflowing read/write lengths; invalid reads preserve
the caller's byte. Raw logical-VRAM access already uses a safe subtraction
check and needs no second change. Types' existing LIB_SIZE_MAX is used.
The first failing query assertion is retained here as a failed attempt;
the focused raw log is superseded by its passing rerun.

This final two-line guard edit changes only invalid overflowing ranges.
For every non-overflowing positive range, the old sum comparison and the
new remaining-capacity comparison are mathematically equivalent. All real
BIOS/CPU/DMA and diagnostic accesses in the recorded normal replay are
bounded within that domain. Therefore the long colour/boot replays below
are not repeated a third time for this guard edit. Instead the exact final
tree rebuilds both complete unit suites, both previews and the specialized
checks, then relinks the real firmware receiver and reruns its FDC-reset
integration on each width. This is a scoped verification decision, not a
claim that the long replay's executable bytes include the later guard.

Final actual C/H: production +124/-64 (net +60) in four paths; existing
tests +195/-1 (net +194) in two paths. Total +319/-65, net +254. All names,
public callers, current manifests and source contracts are reviewed again.
No Lib/Emulator, CPU/Core/Debug/IBMPC or unrelated App implementation changes.

| Verification | x64 | x86 |
| --- | --- | --- |
| Final exact-tree Types self-test | 1 pass, 30.04 s | 1 pass, 29.06 s |
| Final exact-tree remaining repository units | 711 pass, 48.76 s | 711 pass, 60.18 s |
| Specialized gates / strict compilation | pass, 498 retained rows, zero deferred | pass, 498 retained rows, zero deferred |
| Main-feature unchanged BIOS Setup/input/reset replay | pass, 144,110,532 instructions | pass, 144,110,532 instructions |
| Main-feature FDC reset / boot handoff | 40.54 / 215.85 s, pass | 53.38 / 321.83 s, pass |
| Final guard-tree real FDC reset | 43.99 s, pass | 63.34 s, pass |
| Receiving profile build / actual INI identity | 4 pass | 4 pass |

Both final real Setup replays retain five resets, four known exceptions and
no Core fault. Their ERROR frames match SHA-256
`D9B110C0CA4F495127B22FF64E80F38AE075ECEF22D4BDBD478802D09C49AFB5`;
their date/time frames match
`0644016D2EE985038EFC43E069A260768DE91AC98231A7891A5463EBD45F8EDD`.
The RGB counts above match byte for byte on both widths, including BE9E96.
No special pink constant exists in production. The actual normal main Setup
menu, configuration persistence and selected guest remain unqualified.

Final log SHA-256 index, recorded before cleanup:

| Log | SHA-256 |
| --- | --- |
| real-x64 | A2BEF0A046E7C7ED4A512F5897CA804AB5F311AAC2EFFD7CE03D807168F9ED40 |
| real-x86 | 4527EDC5D547D175D07C2F9DB24539439CF25CBE350AF5D8F3040E7343B15AB0 |
| units-x64 | F5A87E80CAD93B98C7E80F8FA0193DA89E43A55DC538235780A9778991F4F2D4 |
| units-x86 | D27FE1F62C494F5241AB3887672E9AF2810A0A53F4485A5C42B8879F18917B19 |
| integration-x64 | 18B31849F0537BEAA9B6A21AB0EE3763257CBEA3AF0E811E8536A059CC65D287 |
| integration-x86 | 70F8BB8F453120D28BA7658F1F31857B8C01A64C98C230289728D0C46FBE3107 |
| range-firmware-x64 | A5EE6052E67873B09C9DF2522C5419B6FD1A185A7656B0CC0D3D3D89AE27AA33 |
| range-firmware-x86 | 58DD0A9BD60E99BB1B0B26E11D4F5C553843C48B252C48B2B9C24D661557362B |
| gates-x64 | 27F67A2E16D9D07439FBE834E44DB41ED3AF85DCE48BC4AA3D9D1ADF3C617270 |
| gates-x86 | 90C8452B63CE691191EE0367792DD588944C3A4240E4D611494B260A111BD1A9 |

Both final flows are terminal PASS and native process inspection confirms
their complete exit. The final twelve-artifact inventory is regenerated
after the range fix: eight published PC receivers and four unpublished
PC110 previews, with actual hashes, PE widths and runtime 0.5.0580 evidence.
The eight published receivers still differ from S56 only in relink timestamp
and checksum; MyNES/MyRaidenII and all owner INI are unchanged. BIOS/font
and all five selected media hashes still match their external manifests.

The official cleaner previews 90 owned files / 13,302,526 bytes, below the
16 MiB bound, including hash-reviewed reference-easysetup.bin. After archiving
these facts, Apply removes only that exact scratch directory and retains
both stable caches. No desktop, RDP, physical-panel or independent DSTN guest
run is counted as passing. Existing normal Setup/save/restart, two-slot
SunDisk/CF, PCIC/card, panel/live lifecycle, selected guests and four-product
publication gates remain in T580; this batch cannot close the whole task.
