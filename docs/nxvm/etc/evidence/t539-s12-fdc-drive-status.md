# T539 S12: FDC Drive And Status Qualification

Work in progress against 15dd1f3cd; not S acceptance or extraction evidence.
The complete batch is the command/input inventory and four mandatory repair
gates in the [S10 decision](../architecture/t539-s10-fdc-boundary.md).
S11 accepted only the pending-operation identity/capacity member.

## Primary-Source Check

Intel 8272A 1982 preliminary, order 210606-001, archived original SHA-256
`C03A1FABE42FE43BB47FBD84042840F7B78B1DDD03E8A117FC7059C3453CC398`:
rendered PDF pages 10 and 12 (printed 6-233 and 6-235) were inspected, not
merely OCR. Table 12 defines ST2 SH at bit 3 and SN at bit 2. Table 10
distinguishes equality (SH=1) from a successful strict inequality (SH=SN=0).
Low/High compares disk data against processor data, not the reverse.
The prose's general Scan Hit statement does not override its explicit table
or the equality definition in Table 12.

The read-only 86Box `fdd_86f.c:d86f_compare_byte` at the S10 checkout uses
the reverse operand order. Its result constructor uses the correct 04h/08h
bit positions but does not establish Intel's equality/inequality distinction.
PCjs `FDC.REG_DATA.RES` also assigns SN=04h, SH=08h. Reference code was not
copied. Agreement with an emulator is not an oracle when Intel is explicit.

## SCAN Correction In Progress

The existing masks were reversed. All existing tests used those same macros,
and Low/High fixtures followed the same reversed comparison. A new nine-row
wire-result matrix uses literal Table-10 expectations for all three commands
and all three disk/host orderings. Seven rows fail before the repair.

Production now uses the correct masks and operand order. The existing pending
ST2 SH bit carries per-sector equality, so no second comparison result field
or side path is added. A strict inequality may satisfy the command without
setting SH. Old Low/High fixtures now expect that result; their original
coverage is retained. Host FFh no-care remains the previously admitted
reference-model behavior, not a new Manual-L3 claim. SK-exhaustion clears SH
before reporting SN. The original DMA and non-DMA command path remains sole.

The x64 and x86 FDC tests pass after this correction, including all original cases,
S10's readiness characterization, S11's pending-operation cases and the new
nine-row matrix. This is a local proof, not full-S verification. No EXE,
Shared source, INI, external master or sibling repository has been changed.

The historical [T376 S4 record](t376-s4-8272a-scan.md) describes the old accepted
reference model; its reversed bit/direction claims are superseded by these
specific Intel findings, not rewritten as if the old evidence were correct.

## Board-Level READY Evidence

The existing IBM Options and Adapters Volume 2 archive has SHA-256
`B5BF24EA3E63082D5C637DB8B08469C6D4929B4B9F6B7B24C7A211338B42A15F`.
Its filename says April 1984, but the AT adapter sheets are dated August 31,
1984; that page-level edition is retained rather than inferred from the name.
Visual inspection of PDF page 397, AT adapter sheet 4 of 9, shows U30 pin 35
RDY directly tied to +5V. PDF page 296, 5-1/4-inch diskette adapter sheet 3
of 4, likewise shows U6 pin 35 RDY connected high. Thus absent media cannot
by itself deassert the chip READY input on these boards. Sector availability,
drive-select/motor gating and Track0 are separate facts; a single combined
`drive_ready` predicate would encode the wrong hardware boundary.

The AT drawing also shows active-low connector Track0 through board logic
to the chip's FLT/TK0 input. It is not evidence for a caller-controlled
per-command status override. The same archive's DOR description and XT
selection drawing distinguish motor/select routing from controller US outputs.
Board integration must own that routing; a chip's unit number is not itself
proof that a physical drive receives a step.

The previously selected Compaq September 1986 Technical Reference Volume II
original is now archived with SHA-256
`E523F1ACF81B7223D94835FB9BEDDEF0738BA6B1C5E1C740028FDCF3DA79357F`
(87,066,278 bytes). Acquisition uses the existing capability record's
[archive original](https://archive.org/download/compaq-desk-pro-386-technical-reference-guide-vol-2-1986-09/COMPAQ_DeskPro_386_Technical_Reference_Guide_Vol2_1986-09.pdf),
not new firmware or imported implementation. The original stays outside Git
in the authorized manual archive.

Rendered PDF page 102, printed 7-32, Figure 7-10 sheet 4 of 6, shows the
765A FDC (U13) RDY pin 35 connected directly to +5V. A higher-resolution
inspection follows the complete wire from pin to supply; OCR alone was not
used to infer the connection. Chapter 7's J501 description separately lists
Track00, write protection, index and disk-change signals. The processor-board
D3PE scan is not used as evidence for this separate multipurpose controller.
Page 7-29 qualifies these schematics as general understanding rather than a
guarantee of circuit accuracy; preserve that source limitation.

Thus the selected Compaq drawing independently supports the same fixed-high
READY input as the IBM drawings. It does not justify a special read-only
Not-Ready result for absent media. Removing `DESKPRO_REFERENCE` is now
source-supported, but remains unaccepted production work until all affected
commands and Model-40 boot pass. It must not be replaced with another status
override to accommodate firmware.

## READY/Data Repair In Progress

The enum, config field, topology check and profile selections of
`DESKPRO_REFERENCE` are removed from source and tests. Data commands now use
one input-failure constructor: a low READY pin produces NR, whereas absent
media, an unavailable mechanism or motor/selection failure retains the
existing record-unavailable result. This is not a claim that the flat-sector
backend now models every address-mark search timeout precisely.

The ten-command S10 matrix retains all six distinct input variants, but its
two compatibility-policy runs collapse to one after deleting that policy.
Its 60 cases now require READY and installation to be honored; they do not
preserve the old contradictory admission results. The Model-40 receiver test
now expects the same absent-input behavior, with terminal observation retained.
The common input-failure path also covers FORMAT's previously unchecked
mid-transfer input loss and SCAN/read/write admission and service.

READY baseline/polling now samples the same pin as ST3, rather than media
presence. A new fixture proves removal/insertion does not assert a READY IRQ
on fixed-high boards, and independently injected low/high pin transitions
report C8/C0. The fixture drains the initial reset causes first. Multi-cause
arbitration and no-pending SIS length remain open, not covered by this proof.

Inspection found AT/default's old descriptor encoded READY only for fitted
unit zero. AT now follows its tied-high schematic for every unit selection;
the generic default adopts that existing PC/AT board model. No installed
drive or medium is invented. Their installation masks remain unchanged.

Local verification: `unit.core-machine-fdc-smoke`,
`unit.vm-model40-fdc-s24-smoke` and `unit.vm-default-pc-at-apply-smoke` pass on
both x64 and x86. An initial CTest expression matched no tests; it is not
counted as verification. The corrected names above were executed. The doc
structural gate and `git diff --check` pass. No full-unit/integration result,
new artifact or acceptance is claimed for S12.

## ST0 Correction In Progress

Reinspection of rendered Intel Table 12 confirms normal completion is IC=00;
bit 5 is Seek End, not a generic success bit. The old normal constant was 20h,
so every successful data command falsely asserted SE. Data result construction
also omitted HD/US. The single result constructor now supplies the selected
head/unit bits, and terminal observation checks IC rather than requiring the
entire status byte to be zero. SEEK results retain SE explicitly.

The SCAN wire matrix additionally requires literal normal ST0=00h. A new
eight-identity result matrix checks HD/US on missing-record completion; the
existing two-drive transfer test now requires each unit's literal identity.
Two old SEEK expectations now explicitly assert SE. The DOS READ TRACK test's
literal 20h success expectation is corrected to 00h; its other data/result
checks remain intact. Integration has not yet been rerun for this change.

The complete x64 unit aggregate then ran all 347 tests: 345 passed, including
the eight-row identity matrix. Two old assertions contradicted the corrected
READY contract: media-change expected insertion/removal to produce a READY
interrupt; controller-authority configured READY low but expected no-record.
The former now checks DIR changes without READY IRQ, and the latter expects
NR=1 with ST1=0. Both targeted reruns pass. Their original cases remain;
electrical READY transitions are independently tested by the new fixture.
This is not yet a new complete-aggregate pass.

Five affected x86 tests subsequently pass: FDC command/status, topology ports,
media-change ports, controller authority and Model-40 FDC binding. The current
code's next repair boundary was physical head position versus PCN: Intel
6-234 explicitly rejects implied seeks in read/write commands and defines
Track0/77-step termination for recalibration. Controller reset currently clears
the combined position field, and SEEK clamps PCN to mechanics; neither should
be preserved as chip behavior during extraction.

## PCN And Physical Position Repair In Progress

Rendered Intel page 11 (6-234) defines per-step SEEK, READY termination and
RECALIBRATE's Track0/77-pulse limit. Controller PCN is now separate from the
board's physical drive position. Controller reset clears PCN but does not
move a head. DOR routes each STEP to the physical mechanism; the command's
unit selects its PCN. Each due pulse advances at SRT, and READY loss ends
the operation at the current PCN rather than its requested destination.
The existing time conversion is retained; this is not a timing-grade upgrade.

New wire-result tests cover PCN beyond the mechanical limit, reset preserving
head position, 77-pulse failure followed by a second recalibration, READY
loss during SEEK, low READY at admission, and differing DOR/command units.
They pass locally on x64. No-motion recalibration no longer clears the
board's media-change latch: the port regression now issues a real STEP.

Data admission previously addressed a flat image using command C without
checking the physical head. Read/write/deleted/SCAN now share one check:
the flat provider's physical track ID must match C, otherwise ND/WC is
reported without reading or writing payload. READ ID and FORMAT take their
track from the physical mechanism, not stale command C. A seven-command
matrix and a stale-C READ ID case pass with the existing x64 FDC suite.
Arbitrary on-disk ID layouts are not inferred from flat geometry, and READ
TRACK's different comparison semantics remain part of the pending review.

The x86 complete aggregate ran 347 tests in 36.32 seconds: 346 passed. Its
only failure, `unit.vm-fdc-port-smoke`, also expected no-motion SEEK to clear
disk-change. It now steps out and back through the ports, and its targeted
rerun passes. The original x64 aggregate completed in 203.38 seconds with
the same 346/347 result and same failure; its corrected port test also passes.
Neither targeted pass is a new complete-aggregate pass. Integration and
artifacts remain outstanding.

## No-Pending SIS Result

The rendered Intel 6-234 page distinguishes SIS causes from normal data-result
interrupts and defines invalid-command ST0=80h, but does not explicitly give
the no-pending SIS result length. The inspected 86Box `fdc_sis` returns one
byte in that case; PCjs's two-byte path disagrees. This repair selects the
explicit 86Box model as external-reference L2, not Manual-L3.

NXVM now returns exactly one byte when SIS has no pending cause. Both command
and port tests check MSR before each result byte; stale reads after the result
phase cannot count as successful receipt. The exhausted-reset case and a
SIS after the normal data result independently check 80h and phase completion.
Temporarily restoring the old two-byte branch makes both x64 tests fail;
restoring the fix makes both pass on x64 and x86. The source and local test
binaries are restored to the fixed implementation, not the negative control.
The topology and media-change port tests also contained the old two-byte
expectation; both now verify one-byte completion rather than a stale second
read. No pending-cause conclusion follows from length alone.

## Pending Causes And Input Loss

READY reporting previously borrowed global ST0 and returned transfer C as
PCN; ordinary ST3 result consumption could erase the IRQ before SIS. A
four-bit READY mask now retains one sampled cause per unit independently of
the data result and seek-result array. SIS uses that unit's PCN. Reading ST3
does not acknowledge unrelated causes, and consuming a data result does not
erase queued causes. Tests interleave four READY changes with READ ID, ST3
and SEEK and verify that each result survives with the correct identity/PCN.
Intel defines the reason codes and PCN meaning. The retained polling cadence,
per-unit bounded latch and cause priority are model-level choices, not a new
claim of exact silicon polling arbitration or waveform timing.

The same command/input matrix now covers 60 admission and 54 execution-stage
cases. Input loss ends through the common error-result path and clears DRQ
and byte deadlines; it no longer silently discards the command on DOR motor
or selection changes. Reset still cancels without producing a data result.
Read/write/SCAN/FORMAT share the same distinction between low READY and
unavailable records. A failed SCAN clears its provisional equality bit.
This preserves the existing flat-media service model; it does not pretend
to reproduce a physical motor's coast-down or index-search latency.

The new matrix initially caught 15 SCAN cases leaking provisional SH; the
common failure completion correction makes them pass. Receiver reruns found
two obsolete SIS length expectations and the old silent motor-off-cancel
expectation. The latter now checks a seven-byte ND result and separately
retains the reset cancellation case. The five affected receiver tests pass
on x64 and x86 after these corrections; full aggregates need a final rerun.

## Remaining Batch Members

Rendered Intel 6-233 was rechecked for SCAN's deleted-mark and STP prose.
SK=1 still reports CM; SK=0 treats the deleted sector as the final sector.
STP=2 compares alternating sectors, and exhausting a sequence without reaching
EOT is abnormal. Production now uses those rules through the existing byte
path and one sector-step calculation. A nine-row, three-sector in-code
fixture checks data consumption, SH/SN/CM, EOT parity and termination after
a deleted sector, including a later matching sector which must not be read.
Both widths pass. The old all-skipped case now expects CM as Intel requires.
Existing non-1/non-2 STP encodings retain their prior one-step compatibility
fallback; that behavior is not promoted to Manual-L3. Result-CHRN and TC
qualification are not implied by this byte-selection test.

READ TRACK retains the S10-authorized finite subset: drive zero, DMA, 42h,
N=2, sector one and exact track geometry. It additionally requires command C
to match the physical head; unsupported ID-mismatch streaming returns the
existing ND error instead of silently reading another track. The two-cylinder
no-implied-seek fixture verifies no payload is read. Intel's broader continuing
ID/CRC-error stream is not claimed, and no new on-disk encoding model is added.

- READY/Track0 must have one input contract across sense, seek/recalibrate,
  data and input-loss paths. Existing status READY, media readiness and
  mechanical readiness are different predicates; no new board wiring is
  inferred from boot success.
- Complete receiver verification of the separated physical position and PCN,
  including no-implied-seek behavior and READ TRACK's distinct semantics.
- Validate the removed Model-40 unready-read policy through the complete
  packet, including its real boot. Do not reintroduce a status override or
  use PCjs's admitted BIOS workaround as electrical proof.
- Finish the partial TC/result review. The explicit READ TRACK subset and source/model
  distinctions above must survive extraction; focused cases alone do not
  close the complete command semantics.
- Finish full-consumer verification of corrected ST0 IC/SE and HD/US identity,
  including the DOS READ TRACK result bytes and terminal observation.
- Complete negative controls, both full unit suites, receiver integrations,
  artifacts, manifests/gates and actual-diff review are still required before
  any S12 implementation commit or acceptance.

## READ Mark Selection And DMA Sector Boundary

READ, READ DELETED and SCAN now share the existing sector-mark preparation
path instead of querying marks in a second byte-transfer branch. Intel
6-231/6-232 requires SK=0 to deliver the mismatched sector and terminate with
CM; SK=1 skips its payload. Unlike the explicit SCAN prose, those READ rules
do not set CM for skipped sectors. READ TRACK retains its stated non-SK subset.
Twelve in-code cases cover both READ commands, mismatches in the first and
second sectors, SK on/off and all-skipped input; x64 local verification passes.
The input-loss matrix now supplies a matching deleted mark for READ DELETED
so it isolates input loss rather than incidentally testing CM as well.

The initial unification exposed an ordering regression in the existing DMA
terminal test: 512 bytes were transferred, but the next sector was inspected
before DMA delivered TC, yielding 40/80/00 rather than success. The next-sector
preparation for DMA now occurs before the next DRQ, not inside the preceding
byte callback. TC can complete the old command first. READ and SCAN share this
boundary; NDMA retains immediate preparation before another CPU data access.
The original TC test and new mark-selection cases pass. No additional state,
API, DMA loop or compatibility status override was introduced.

The first complete aggregates after this change were 346/347 on both widths
(x64 194.84 s, x86 30.26 s). The sole failing Model-40 composition test still
read a nonexistent second byte for no-pending SIS. It now verifies the 80h
byte followed by command-ready MSR. x86 then passed 347/347 in 19.82 s.
The accompanying S24 result helper was also found to accept stale extra reads;
it now checks RQM/DIO before every byte and its two no-pending cases request
one byte. Complete aggregates then passed 347/347 on each width after that
final test correction (x64 22.00 s, x86 21.11 s). Those results precede the
WRITE-tail production change below and are not its full-aggregate acceptance.

## Partial WRITE Terminal Count

Intel 6-232 explicitly requires finishing a partially written data field with
zeros after TC. Previously the DMA callback completed immediately and left
the old bytes untouched. One internal WRITE-tail phase now consumes the
remaining sector through the original write-byte path and byte deadline.
It is busy without requesting DRQ/RQM. No new public interface, sector cache,
independent write loop or duplicated completion state is introduced. Reset,
input loss and media errors use the same existing cancellation/error paths.
The existing configured byte-service cadence or logical fallback is retained;
this does not promote that cadence to exact physical disk timing.

Twenty-four in-code cases combine ordinary/deleted writes with both cadence
choices: TC after 1/511/512 bytes, plus reset, motor loss and provider failure
during the tail. They verify deadline gating, no extra DMA, the zero tail,
untouched later sectors, error/reset results and no surviving byte deadline.
Both x64 and x86 local suites pass. Temporarily bypassing the new TC branch
fails the x64 test; restoring it passes again. Fixed source and the x64 local
binary are restored. The negative run is not a final artifact.

## Processor-Terminated CHRN

Rendered Intel PDF page 8, printed 6-231, Table 8 defines the ID following the
last sector transferred when the processor terminates. Before EOT, R advances
and C/H remain unchanged. At EOT, R becomes one; without MT, C advances and H
is unchanged. With MT, H toggles, and C advances only after side one.

One completion helper now applies this rule to the four read/write commands,
including partial-sector READ TC and the end of WRITE's zero tail. TC on the
same DMA service as the final sector byte is also recognized while completion
is pending. Error completion, SCAN, FORMAT and the finite READ TRACK path do
not borrow this READ/WRITE table. This changes returned ID only; physical
drive position and PCN are unchanged.

A 64-case matrix crosses four commands, two heads, MT on/off, EOT/not-EOT,
and partial/full-sector TC. It checks literal C/H/R/N, status and unchanged
mechanical position/PCN. The x64 local suite passes; the complete x86 unit
aggregate passes 347/347 in 34.21 s with this implementation. The x64 complete
aggregate passes 347/347 in 211.92 s. This table coverage
does not claim a newly implemented continuous multi-track transfer engine or
arbitrary on-disk ID/CRC stream.

## Service-Time Qualification Retained For Extraction

Intel's 13/27-us read and 15/31-us write text is a maximum host-service bound,
not an exact inter-byte arrival formula. The existing byte gate uses the
write bound as a deterministic macro model at 500 kbps and scales it for
300 kbps; that remains a model-level cadence, not Manual-L3 arrival timing.
The 250-kbps branch retains its existing logical next-progression fallback.
Likewise, a profile's macro tick conversion does not prove the physical clock
of an 8272A. Future extraction must accept board-qualified service inputs and
preserve these qualifications, not relabel the old numeric values as L3.
READ TRACK's finite restrictions remain those explicitly retained above.
This S repairs the demonstrated semantic contradictions; it does not claim
all 8272A encoding, flux, CRC, motor or physical timing capabilities.

## External Receiver Gate Remains Open

The specialized x64 static aggregate passes. Each XT/AT boot was run once:
XT x86 23.89 s, XT x64 22.40 s, AT x64 39.35 s and AT x86 47.27 s pass.
Model 40 fails in both widths at 180 s with READ E6 returning 40/04/10
(Wrong Cylinder), physical/result C=2. The cause is not yet established;
bounded existing terminal diagnostics now also capture PCN and physical C.

One 60-second Model-40 x64 diagnostic using the existing bounded 64-terminal
ring observes four successful E6 reads at C=0, then E6 requests C=1 while
PCN=2 and physical C=2; the latter correctly returns WC. All five commands
use CCR=01 (300 kbps) on the 80x2x15 image. Production `rate_supported()`
currently only admits three CCR values; it does not validate a drive/medium
rate combination. A false density/double-step detection is therefore a
specific lead, not yet a proven BIOS branch or an accepted repair. No implicit
seek, double-step special case or machine-specific status was restored.
The build's optional generic CPU trace emitted no events; only the existing
FDC terminal callback supplied this record. This diagnostic is not a repeated
acceptance boot or a successful receiver gate.

A second, explicitly diagnostic 60-second run temporarily rejects non-500-kbps
rates for the 15-sector image in READ/READ-ID admission. Without changing ROM,
INI, media or seek logic, BIOS changes CCR to 00, PCN/physical C agree with
requested C throughout the retained terminal ring, and successful reads reach
C=17. This establishes the false-positive media-rate admission as a causal
part of the original C=1/physical=2 failure. It does not establish a final
boot pass. The temporary geometry-based production condition was removed
immediately after the diagnostic and is not the proposed production contract.

Compaq Technical Reference vol. 2 PDF page 80 (printed 7-10), visually checked,
Table 7-4 lists both 500 and 300 kbps against the same "1.2-MB diskette drive
data" wording. It therefore does not alone distinguish inserted formats.
Read-only 86Box `fdd_img.c` at the recorded checkout separates encoded media
rate from drive RPM and uses format-density tables; PCjs READ-ID comments
explain the 40-track/80-track double-step detection. No external code is copied.
The proper repair needs a neutral drive/media read-channel contract, shared
by READ-ID, data, SCAN and FORMAT, rather than a special Model-40/15-sector
condition inside the chip. Drive pitch/physical position and media track IDs
also need explicit mapping before admitting 360K media in an 80-track drive.

Default x86 integration passes 6/20; the other 14 scenarios fail, including
boot and Windows checkpoints. Its READ result is 44/04/10. Actual external
default ROM SHA-256 is
`DF7A7FE8172001739DE7DD4228B4EAB4F11F7BE97412F277E858DA3AA61D7FF0`.
Read-only disassembly shows READ E6 issued at 0DE4 without SEEK; 0E23-0E29
discard all seven result bytes, then 0E2A clears AH and 0E2C clears carry
unconditionally. The historical project-owned FDC firmware source at
`d9dc15ad1^:src/vm/profile/default_profile/firmware/fdc_firmware.h` confirms
the same missing SEEK and unconditional success in read/write services.
Restoring controller implied seek would hide that firmware defect and is
not an acceptable repair. Owner approval to repair/rebuild the external
default firmware master has been requested; no external master or INI has
changed. Default x64 integration is not claimed as run or passing.

The integration/static build dependencies regenerated the default x86/x64
0539 EXEs in the worktree. They are unaccepted candidates, not a release.
No S12 commit or closure is claimed while these receiver failures remain.

## Neutral Drive-Channel Implementation

The frozen FDC drive binding now borrows one stateless channel sampler. It
receives the selected physical unit, copied media geometry, physical cylinder,
data rate and encoding; it returns a track ID or no usable track. It cannot
mutate chip state or construct status. All four live profiles bind the one
NXVM flat-format implementation; synthetic logical controller fixtures without
recording metadata retain identity mapping explicitly, not a firmware fallback.
No media bytes, PCN, physical position or command phase are duplicated.

The board model admits the four existing flat formats. Its 1.2MB mechanism
reads 360K at 300 kbps with two physical steps per track, or 1.2MB at 500 kbps
with one step. XT uses its fixed 250-kbps channel, not a nonexistent CCR.
The default logical mechanism retains its four-format compatibility without
claiming that one physical 3.5-inch drive accepts 5.25-inch media. These are
qualified flat-media models, not new flux/PLL fidelity or physical-time grades.
READ ID, READ TRACK, four read/write forms, three SCAN forms and FORMAT use
the same admission helper; physical motion remains SEEK/RECALIBRATE only.

The existing Model-40 fixture now includes 384 drive-format/rate/position
combinations, their FM rejection counterparts, and 30 wrong-rate command
admissions across all ten forms, followed by a valid READ ID. Complete unit
aggregates are running. A real Model-40 x64 boot with the implementation
passes once in 60.97 s; this supersedes its earlier failing receiver result,
but does not replace x86 or the other profiles' post-change verification.

The first x86 complete aggregate with this channel passes 345/347 (41.79 s).
Only `vm-fdc-port-smoke` and `vm-fdc-t242-corpus-port-smoke` fail: both use a
1.44MB in-code medium without selecting its 500-kbps rate; the former also
restores 250 kbps after its reserved-rate negative check. The fixtures now
explicitly select CCR=00 and both local tests pass. No expected status or
production rate rule is weakened. Complete aggregates and Model-40 x86 are
pending at this record update.

## Post-Channel Receiver Verification

The x64 aggregate that began before the two fixture fixes also failed only
those two cases (345/347, 197.38 s). After rebuilding those fixtures, complete
repository-only units pass 347/347 on x64 (21.93 s) and x86 (19.93 s).
No production condition or expected result was weakened to obtain these passes.

Each unchanged external INI/ROM/media receiver was booted once per width
after the channel implementation:

| Receiver | x64 | x86 |
| --- | --- | --- |
| XT 5160 | Pass, 21.59 s | Pass, 21.11 s |
| AT 5170 | Pass, 32.80 s | Pass, 45.51 s |
| DeskPro Model 40 | Pass, 60.97 s | Pass, 72.30 s |

These replace the pre-channel receiver observations, not the unresolved
default integration result. No repeated boot round was used as an acceptance
requirement. Six unchanged Shared manifests have matching hashes for all
listed entries (108/22/37 source and 50/19/23 test entries); no Shared or MyNES
file changed. The post-channel specialized static aggregate completes all 108
build/check steps on x86; both reusable trees are restored to the default
profile. S12 remains open: external default-firmware repair is outside the
packet's existing no-master-mutation authority and has been requested, not
performed. Current worktree artifacts are not an accepted release.

## Default Firmware Repair Boundary Review

Read-only review reconfirms the same external ROM hash above. The project-owned
historical `fdc_firmware.h` and actual 16-bit ROM instructions agree on these
additional coupled members; this is a proposed repair boundary, not approval
to regenerate or replace the external image:

| Member | Observed behavior | Required cohesive repair |
| --- | --- | --- |
| POST, ROM 0780-079D | DOR reset and SPECIFY, no CCR selection in this routine. Controller initialization currently defaults to 250 kbps. | Firmware selects the supported medium's rate; do not change a chip reset default to conceal a firmware omission. |
| Read/write services | Both issue a data command without SEEK. Read 0E23-0E2C and write 0EFC-0F05 discard seven result bytes, then AH=0/CF=0. | One seek/completion and result-decoding mechanism shared by read/write, with BIOS error return and saved status. |
| IRQ6, ROM 0191-01AE | When command-ready, sends SIS and discards both bytes. Otherwise sends EOI only. | Choose one firmware owner of seek completion; an added polling routine must not race an IRQ handler that consumes its result. |
| Read/write waits, 0E19 and 0EF2 | Unbounded MSR polling; the seven result reads have no individual RQM/DIO check. | Bounded command/result handshakes and explicit timeout/error cleanup. |
| Reset/status dispatch, historical source | AH=00 returns success without resetting; AH=01 returns saved status but always clears CF; unsupported dispatch clears CF. | Review these together with the new error and completion owner, against the chosen BIOS service contract. |

The old `bios.c` also writes BDA/IVT through a host firmware context. Restoring
that file as a runtime provider would reintroduce the retired parallel firmware
route. If authorized, recover only a reproducible build-time ROM source/tool;
keep the existing immutable external-ROM loader as the sole runtime route.
The firmware request must cover initialization, read/write, reset, IRQ/result
ownership, errors and timeouts as one batch, with no commercial BIOS edits,
INI changes or controller compatibility branch. Existing external integration
must then be rerun; the current unit and other-profile passes cannot validate
an image that has not yet been built.

## Owner-Admitted Firmware Packaging Cutover

The owner subsequently admitted project-authored BIOS source recovery into
`src/app-nxvm/firmware`, build-time embedding of all four selected machines'
firmware, and committing their eight EXEs. Commercial originals remain external.
This supersedes the proposed external-loader boundary above; it does not admit
host BIOS services or changes to vendor ROM instructions.

The working implementation now generates one immutable firmware byte object
under ignored `build/`, using the selected CMake asset table. Original byte-size
and SHA checks remain at configuration; changed inputs trigger reconfiguration.
Only product executables and external integration link this object. Main supplies
its copied byte views to App composition; integration supplies the same object
to the existing machine-from-assets constructor. Unit fixtures provide in-code
bytes without linking that object. Core ROM mapping and reset are unchanged.

Removed paths include the file-backed machine constructor, temporary file-asset
owner, ROM/CMOS/font path fields, unused XT/Model-40 manifest loaders and private
runtime SHA implementation. The live option-ROM signature/length/checksum check
remains. The retired file-helper negative test now tests that retained byte
validation boundary. A source sweep of profiles/config finds no `bios_path`,
`font_path`, `create_file_backed`, `byob_blob_load`, `byob_manifest_load`, or
`lib_storage_file` route. This is not evidence of boot correctness of the still
unrepaired default BIOS.

Post-cutover validation:

- x64 existing complete unit aggregate: 347/347, 61.93 s; the subsequently
  registered synthetic embedding case passes separately in 0.37 s.
- x86 complete aggregate including that case: 348/348, 24.82 s.
- Synthetic embedding checks exact 01/7F/FF bytes, extents, absent slots,
  missing/empty input rejection, and no build-input path in generated source.
- Default x64 product and integration support compile/link against the same
  embedded object; x86 embedded-object compilation passes. These are build
  results, not new boot acceptance or final eight-artifact release evidence.
- The first static aggregate rejected the unlisted auxiliary CMake test.
  Registering it in the existing auxiliary inventory fixes that omission;
  the complete specialized aggregate then exits zero. No gate is weakened.
- Shared/MyNES diffs are empty. INI contents and external asset masters remain
  unchanged. No S12 implementation is committed or accepted yet.

Next work remains source-based default BIOS reconstruction and the complete
FDC firmware repair, followed by post-cutover receiver/integration acceptance.
The six successful XT/AT/Model-40 boots above precede embedding and cannot serve
as the new packaging acceptance. Historical source inspection located the
guest assembly macros at `1cf34c1f452f4e76b7a6e88eccb0953c70ba9cd9`; the old
host BDA/IVT builder must not be restored as runtime code.

## Source-Based BIOS Reconstruction

The recovered guest assembly now lives in `src/app-nxvm/firmware/`, with the
exact historical source mapping and MIT authorization recorded in its README
and the provenance index. `nxvm-firmware-build` links only the shared assembler
and Storage; it does not link Core, a machine or a host BIOS provider.
The default profile's build now assembles these sources into ignored
`generated/default-pc-at.rom` and passes that output to the same byte-embedding
command as the vendor profiles. It no longer selects the old external default
ROM. External CMOS/font inputs and vendor firmware are unchanged.

The pre-repair recovery candidate has identical SHA-256 on x64 and x86:
`EDAD5ECC24E7FAE41BC32DE1F858FFB43FEEF9191389F20C8657CC2AA8445563`.
Read-only comparison against the previously identified owner-built ROM proves:

- Interrupt service region 0001-06CD, video 4000-446E, floppy 0C00-0F69,
  keyboard tables E000-E0D8, IVT F800-FBFF and BDA FC00-FDFF match byte-for-byte.
- Entry 06CE-06F1 differs at two bytes: 31 C0 / 31 FF become 33 C0 / 33 FF.
  Disassembly confirms both pairs are the same self-XOR operations on AX/DI.
- The redundant E9 0000 jump at 06F2 is omitted. Subsequent POST/boot code
  matches byte-for-byte at a three-byte-earlier address, including disk fallback.
  This is source recovery proof, not new hardware or boot acceptance.

New repository-only build regression checks deterministic 64 KiB output,
reset/interrupt vector shape, missing/invalid/empty/oversized assembly rejection
and preservation of an existing output on assembly failure. Both widths pass
the build and embedding cases. The x64 complete unit aggregate passes 349/349
in 20.73 s; x86 passes 349/349 in 20.25 s. Unit registration verification passes
on both widths, and the specialized static aggregate completes all 87 steps.
Documentation governance and whitespace checks pass. Shared/MyNES/INI diffs
remain empty. No external ROM or media is a unit input.

The recovered FDC code intentionally remains the known-bad comparison baseline
until the cohesive protocol repair is implemented: missing SEEK/rate setup,
discarded results, unbounded waits and competing SIS consumption are not fixed
by moving source or embedding bytes. S12 stays open and no partial P is made.

## Guest FDC Protocol And IRQ Repair

The cohesive replacement now uses SEEK/SIS/READ ID before data commands,
CMOS-selected rate/geometry, one sector-sized DMA bounce route for both read
and write, bounded MSR/result waits, and explicit BIOS status/carry results.
IRQ6 notifies only; INT 40h owns SIS and all result consumption. POST initializes
PIT/PIC before the FDC reset service. INT 1Eh names a real ROM parameter table.
Intel printed 6-234/235 and IBM AT March 1984 printed 5-28/29 and 5-89 were
visually inspected for these contracts; no vendor BIOS code was imported.

The first rebuilt default x64 integration run passed 14/20. Five failures used
an obsolete 800,000-instruction boot limit. The unchanged CGA semantic test
reached the DOS prompt at 1,043,840 budgeted instructions and then validated its
original graphics pattern. Those four source files (EGA has two receivers)
now use the existing 6,000,000 boot containment budget; program assertions and
wall-clock ceilings remain unchanged. A budget is not a success criterion.

The remaining direct READ TRACK program assumed DOS left the head at cylinder
zero, but the observed head was at 42. Guest BIOS reset/recalibration was added
before its IRQ6 takeover, exposing a real recovered-firmware defect rather
than changing Core position from the host: reset waited for IRQ6 indefinitely
until the BIOS timeout. Bounded diagnostics showed master IRR=40h, ISR=04h,
slave ISR=40h and master mask=00h. IRQ14 had retained the cascade because its
old default vector only returned IRET. The new guest IRQ14 handler reads ATA
status and sends EOI to slave then master; Core PIC/FDC are not special-cased.
The same replay then reached the real IRQ6 handler and completed calibration.

READ TRACK now compares all 9,216 bytes with the source track and compares
single-instruction versus 128-instruction execution. Its last old assertion
expected a second byte after no-pending SIS=80h. That invalid FIFO read and
assertion were removed to match the already qualified one-byte SIS contract;
all seven command-result bytes and the one SIS byte remain checked. Temporary
IRQ/port/register trace probes were removed. Failure-only PC/status diagnostics
remain, with no additional production API.

The repaired candidate ROM has matching x64/x86 SHA-256:
`ED31CA5DAC547697F312E3E1E76ADB9F530E80129371332E876D92725DAC1D8F`.
Complete default integrations pass 20/20 x64 (11.38 s) and 20/20 x86 (12.95 s).
These are current embedded-firmware results, unlike the earlier pre-cutover
vendor boots. Repository-only firmware build checks now include IRQ14's vector;
the full unit suites pass 349/349 on x64 (20.11 s) and x86 (22.46 s). Shared/MyNES and INI content
diffs remain empty. No implementation P or accepted artifact release exists yet.

Remaining S12 work: isolated guest-BIOS boundary regressions for error/reset,
write and segment-crossing behavior, final full unit/static checks, post-cutover
vendor boot receivers and all eight rebuilt/deployed artifacts, followed by
actual-diff review. Passing DOS integration alone does not discharge these items.

## Guest-Only Boundary Regression And Post-Embedding Receivers

`test/app-nxvm/unit/firmware/floppy_smoke.c` boots repository-authored BIOS
and an in-code synthetic boot sector/media provider through public machine
construction. It does not read external ROMs, fonts, CMOS, media or INI files.
The guest instructions exercise two-sector read/write across both a head
boundary and ES:BX=1000:FFF0, reset/recalibration after cylinder 79, invalid
count/sector/cylinder/drive/command, readonly write and absent-medium read.
Tests check data, AH/carry, status-query retention and caller registers.
A guest IRQ6 mask forces reset timeout 80h; after unmasking, the next read
must reinitialize and return the correct sector. No host-side head/reset
patch or direct register access substitutes for the BIOS path.

Full units with these eleven scenarios pass 350/350 on x64 (22.96 s) and
x86 (22.87 s). The specialized gate then exposed missing strict compilation
for the generated-source fixture: it was classified as deferred production.
The fixture now receives the same target-local strict warning options as the
BIOS builder, rather than a new exemption or an added residual-debt entry.
The subsequent gate found a verifier bug: matching the first line containing
the generated C filename selects its generator, not its compiler. The verifier
now requires the actual `-c` invocation; all four warning flags remain mandatory.
The final specialized aggregate passes all 66 actions, including 379 inspected
direct-compilation rows (345 strict, 34 existing deferred). The x86 strict-built
fixture passes its eleven guest cases in 6.53 s. Documentation governance and
`git diff --check` pass; no shared verifier or shared source was changed.

Each post-embedding vendor boot was executed once, through the same compiled
firmware object linked into its product. Existing boot assertions are unchanged:

| Receiver | x64 seconds | x86 seconds |
| --- | ---: | ---: |
| IBM 5160 Model 268 | 20.34 | 23.57 |
| IBM 5170 Model 339 | 31.89 | 44.27 |
| DeskPro 386 Model 40 | 59.84 | 77.77 |

Both build trees are restored to default configuration. All eight optimized
0539 EXEs have been rebuilt/deployed with PE architecture checks. Current
worktree hashes (not yet a committed release):

| Machine/width | SHA-256 |
| --- | --- |
| default x64 | BB835F44D12990F5985B751DB2615B759BBBF37CE7C39FD33A545A7AFB0FEF1F |
| default x86 | 0051B7133BF78E2627F76F881167127E6153073244B919A0D53F8DF2B4ACBCB5 |
| XT x64 | 852DDF43452227555533AD51C2733AF6C9B5E01101D5BE1F4F374EBE9C55D91B |
| XT x86 | D428CD5288671E9DA98205F206F8D77EA20C782207592003E7CD3829326ACDB5 |
| AT x64 | 897E9C962E719217231201DE1089E277B886C9EF6A5EC4715F8CC7A965D7D7D5 |
| AT x86 | 0DD2EA77B7BDDBE577DE707F614308EB85A6CE0B456F2F3DB9A66FBD66CC82D1 |
| Model 40 x64 | 12929103061B6524CD74E3E6DF0A3CE8DDC697C825F5458D9182365A9C149219 |
| Model 40 x86 | 9EAA0E47B242F043CC1FA12ED08BAAEB9BEBA09421D26F5262005DA0BC115022 |

Shared/MyNES source/test/docs/artifact diffs and NXVM INI content diffs are
empty. Vendor ROM originals remain unchanged; own ROM hash remains ED31CA5D
(full value above). Actual-diff closure remained required; the runtime packaging
proof is recorded below before the single S12 delivery.

## Runtime Input Independence And Final Corpus Checks

The ignored diagnostic `build/t539-s3/verify-embedded-startup.ps1` parses each
configured product's embedding command and opens every selected BIOS, video,
CMOS and font input read-only with `FileShare.None`. A second open must fail
for each input before the lifecycle receiver starts. This leaves asset bytes,
names and ACLs unchanged while denying runtime file reads. It is an equivalent
input-unavailability probe, not a claim that the external archive was moved.

All four profiles pass on x64 and x86 with `CONTEXT-LIFECYCLE:OK`: default holds
three inputs, XT two, AT four and Model 40 five. The receiver uses the product's
compiled firmware object and real INI, then exercises creation/start/pause/
reset/resume/stop. These short packaging probes do not repeat the DOS boot
matrix or substitute for its recorded checkpoints. Handles are closed on every
exit and both reusable trees are restored to default configuration.

The immutable-ROM closure gate rejects runtime file-open/read/path routes in
Profile sources. A separate ignored synthetic source tree containing the
required immutable-map anchors plus `lib_storage_file_read_all` fails with
`Runtime firmware file route in profile`; the real tree passes. This proves
the new rejection branch, not just the gate's happy path.

A fresh complete hash/coverage check passes all six manifests: src Lib 108,
Common 22, x86 37; test Lib 50, Common 19, x86 23 files. Every file except the
manifest itself is listed and every recorded hash matches. All eight deployed
EXE hashes still match the table above. Shared/MyNES and INI content diffs are
empty, and `git diff --check` passes. These checks do not replace the remaining
coordinator review of the actual S12 code, tests and documentation changes.

The subsequent complete specialized aggregate passes 89 build/check actions,
including the 379-row strict-compilation audit. Restoring the default profile
had invalidated main.c's generated-profile dependency, so this invocation also
rebuilt/deployed default x64. Its timestamp-bearing banner explains a new
artifact identity (999E430A, superseded by the cancellation repair below).
No guest firmware or production source changed in
this verification step.

A concurrent full-unit rerun then passes x86 350/350 (18.11 s), but x64 passes
349/350 (20.51 s): `unit.vm-console-pause-resume-smoke` fails after 2.08 s with
no diagnostic output. Its isolated rerun passes in 0.04 s. The fixture waits
for lifecycle states with 2,000 ms deadlines; this observation does not yet
establish scheduling contention as the cause or exonerate lifecycle ordering.
The failure remains an explicit closure investigation, not a silently discarded
run. External DOS boot cases were not rerun.

## HLT Cancellation Boundary Follow-Up

Failure-stage diagnostics were added to the original lifecycle test without
changing its 2,000 ms limits. A bounded diagnostic replay did not reproduce the
original failure; the original uninstrumented run cannot prove its exact stage.
Review nevertheless exposed a concrete race: stop may clear the adapter's
active flag after the runner checks it but before HLT service. The old service
returned INVALID_STATE, which the runner upgraded to failure and Common ERROR.

The deterministic speed-policy regression now sets stop before HLT service in
both Standard and Turbo. Before the fix, both cases fail with exit 1. The
NXVM-owned waiting function now treats that cancellation as OK with no progress;
bad arguments and a non-HLT result still fail as before. The test asserts no
guest tick advance and clears the output progress flag. No Shared state, API,
guest algorithm, timeout or unconditional error suppression was added.
The similar-path review covers runner's fault conversion and both pacing exits;
the pacing helper already treats stopped work as cancellation.

Full rebuilt unit suites pass 350/350 x64 (64.74 s) and x86 (25.86 s), including
the original lifecycle regression. The complete specialized aggregate passes
again with 379 strict/deferred compilation rows. All four products on both
architectures are rebuilt/deployed with PE checks; the table above records
these final candidate hashes. Existing once-per-receiver DOS boot evidence is
retained: this follow-up changes only the already-stopped HLT cancellation
branch, not firmware/device execution. It does not claim the uninstrumented
intermittent failure has been causally proven or that all future races are absent.

## Complete S12 Delivery Review

Executor review inspected actual production, assembly, build, test and document
diffs, rather than treating passing gates as the architecture audit. The bounded
batch has these dispositions:

- Controller command/status ownership: READY is separate from media availability;
  seek/recalibrate retain their operation identity and distinguish PCN from
  physical position; all affected data/scan/format/read-ID routes use the same
  drive-channel qualification. The old DeskPro unready selector is deleted.
- Original tests: old scenarios remain; changed expected wire values are tied
  to the inspected source tables. No-pending SIS checks one byte rather than
  reading a nonexistent second result. Added tests cover status identity,
  transitions, cancellation, TC, scan ordering and drive rate/pitch combinations.
- Firmware: recovered project-owned source is committed with provenance;
  guest instructions own IVT/BDA setup and disk services. One build/embedding
  route supplies immutable bytes to the existing ROM mapper. File-backed
  constructors, path fields, runtime SHA/file helpers and vendor file loaders
  are removed, not retained as fallback APIs. Unit fixtures remain code-owned;
  only product/integration targets link selected external firmware.
- Publication and failure: missing/invalid/oversized assembly cannot overwrite
  an earlier ROM; missing/empty embedding inputs fail. Runtime read denial and
  the static negative control prove the packaging boundary. Build-time I/O
  failure is reported; no broader filesystem-transaction guarantee is claimed.
- Retained limits: restricted READ TRACK, logical flat-media and explicitly
  qualified timing models remain documented rather than promoted to L3.
  The recovered BIOS's fixed INT 15h memory report is retained, not represented
  as new dynamic memory support. FDC is still product-local until extraction.
- Receiving scope: all eight EXEs are current candidates; Shared, MyNES,
  protected originals and owner INI contents are unchanged. All final unit,
  static, manifest, local-link and documentation checks are recorded above.

Code-size method: `git diff --numstat` plus full line counts for new untracked
source/test/build files, excluding Markdown, generated files and artifacts.
Production `src/app-nxvm`: 47 files, +3,030/-578, net +2,452 lines; tests
`test/app-nxvm`: 25 files, +1,284/-198, net +1,086; build `cmake/nxvm`: six
files, +119/-21, net +98. The positive source delta includes the recovered
guest BIOS assembly and its offline builder; these replace an external,
unrebuildable project-owned image, not another host firmware service. Test
growth provides finite behavior matrices and real guest BIOS calls, not a
parallel emulator. Changed-document local links pass (59 before this final
review record), as do whitespace and product documentation governance.

Before/after ledger disposition: S12's input/status prerequisite changes from
unresolved to implemented and verified, pending coordinator acceptance of the
complete P. The FDC extraction row remains pending. T539 also still requires
the HDC/video/CPU/FPU and remaining file dispositions; S12 cannot close T539.
