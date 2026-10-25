# T580 S47 — Cold initialization and port-width contract

## Baseline and bounded observation

Baseline: `98889bc39`, unchanged TFT firmware and normal Core CPU execution.
The temporary grouped sampler is an observation only; it injects no SMI,
register value, memory flag or firmware return. Its containment is 200M
retirements, 600 seconds and 64 KiB output.

The x64 run reaches the first post-timer software-service write after
59,993,605 retirements and three processor resets. Timer calibration remains
successful with delta 1668. All 80 selected port observations fit in the
128-entry buffer, so that selected stream is not truncated. This is not a
complete instruction or I/O trace, a boot result or a product qualification.
The task-owned process exits normally.

The grouped checkpoints establish:

- The F3C31 ROM-copy routine executes once, with EAX=E000, CX=0100,
  SI=9000, DS=0040 and ES=0000 at the sampled retirement.
- F3F4A executes once. F4CF2 executes once with AX=0E82 and FLAGS=0083,
  taking the branch past the F341B initializer.
- The sampled F332A/F341B/F3452/F3457, F3CA9/F3CCE, C9006/E9006,
  C907B/E907B, C912F/E912F and 38000 checkpoints are absent.
- The first five bytes at 38000 remain zero at the stopping boundary.

Fixed physical checkpoints do not cover relocated aliases. In particular,
EC/ED writes are observed from physical 2DBF8, proving that a RAM-relocated
firmware body executes. Therefore absence of a selected ROM checkpoint alone
does not prove that no equivalent initialization code ran elsewhere.

Relevant selected I/O, in execution order:

| Retired instruction | Access/value | Fact |
| --- | --- | --- |
| FDB72 | read 22 = 00 | No existing read receiver is installed. |
| FDB83, FDB87 | write 22 = 80, 0080 | The BIOS contains a byte write followed by a word write; the callback values alone cannot distinguish them. |
| F3605/F360E | read 22 = 00; write 22 = 000A | Firmware selects a configuration unit. |
| F3617 | write 302 = 0000 | The selected prefix never writes 302 bit 3 set. |
| F363E/F3647 | read 22 = 00; write 22 = 0002 | A different unit is selected. |
| F3661/F366A | read 22 = 00; write 22 = 000E | Another unit is selected. |
| F367A/FDB9A | write 22 = 0000, 0100 | Existing receiver only uses bit 8 to relock Block 2. |
| FDBAB | read 23 = FF | Existing status receiver always returns FF, even after the unlock sequence. |
| FDC64 | SCAMP 01 = 02, then 82 | Firmware, not merely the cold image, changes the initializer's predicate. |
| FDD10/FDD57 | write 15EE = 80 | A real firmware software-service request exists; no fabricated trigger is used in this run. |

Ports 98 and 704 have no observation in this selected prefix. This does not
establish that their hardware semantics are unnecessary.

## Source reconciliation

Original publications acquired on 2026-10-25 into the owner-managed external
manual archive:

| Publication | Source | SHA-256 |
| --- | --- | --- |
| Intel386 SL SuperSet Data Book, July 1992, 240814-005 | <https://bitsavers.trailing-edge.com/components/intel/80386/240814-005_386SL_Data_Book_Jul92.pdf> | `d0be283edfd0eca99979c597b2df48670b3ab9b98e9606751a7ffc36fb0e8d63` |
| Intel's SL Architecture: Designing Portable Applications, 1993 | <https://bitsavers.trailing-edge.com/components/intel/80486/Intels_SL_Architecture_Designing_Portable_Applications_1993/Intels_SL_Architecture_Designing_Portable_Applications_1993.pdf> | `3bfea5a3c1b3bdc6a03bf2e622f24a4522123e3d0ab810386e02e3c72a94755b` |

The book's printed pages 216--219 describe CPUPWRMODE at 22h as a 16-bit
register and explicitly distinguish the final word write in its unlock
sequence. Bit 0 reports unlocked status; bits 1--3 select the internal
configuration unit. CPU configuration unlocking is distinct from the four-read
82360SL indexed-configuration unlock. The latter reports 7F through CFGSTAT
after unlocking. Printed pages 168--170 describe the 30000--3FFFF CPU SMRAM
window, opened by SMI or OMDCR bit 3 at 302h in the IBU configuration space.

These are exact statements about Intel's SL architecture, not proof that every
VL82C420 register is identical. The pinned Open-Source-PC110 component record
describes licensed SL-compatible power management; the actual PC110 BIOS uses
the same ordered accesses and initializer window. Applying that relationship
to PC110 is an explicitly L2 compatibility model. It must not be presented as
a recovered VLSI programming manual or pin-level proof. No publication,
third-party implementation or firmware bytes enter tracked source.

## Established current gaps and proposed boundary

### Complete initializer call-site reconciliation

A subsequent read-only scan of the immutable F-bank for near-call encodings
targeting offset 341B finds two candidates. Both are confirmed as instruction
boundaries by their enclosing routines: calls at F000:4301 and F000:4CF4.
The earlier caller is important and must not be hidden by focusing only on
the later observed branch:

- At F000:424D the firmware calls 432D, then uses **ZF** at 4250 to skip to
  430B if SCAMP 01 bit 7 is zero. If it takes the other path, keyboard/video
  preparation leads to the unconditional initializer call at 4301.
- At F000:4C63 it uses **CF** from the same helper to bypass later video
  initialization while bit 7 is set. At 4CEF/4CF2 it again uses CF to bypass
  the initializer at 4CF4 while that bit is set.
- Helper 432D reads SCAMP 01, tests bit 7 (thereby setting ZF and clearing CF),
  and sets CF if bit 7 is set. Its two flag consumers are intentionally
  different. Existing CPU flag semantics must not be replaced with a forced
  branch result.
- The video-init routine writes `old | 80h` through the ordinary SCAMP setter.
  This does not by itself establish whether bit 7 is ordinary storage, a
  status bit, or another write-qualified hardware latch. A write-one-to-clear
  interpretation is only a hypothesis; no source-qualified implementation
  of it is admitted.
- The reset vector immediately far-jumps to F000:E05B, which near-jumps to
  44CA. Thus a reset-hidden-CS-base alias does not explain away the absent
  low F-bank initializer checkpoint after that far jump. RAM-relocated aliases
  remain a separate observation limit stated above.

The pinned QEMU reference stores SCAMP as an ordinary byte array but also
uses firmware interception; that array is not independent proof of the
original latch semantics. Open-Source-PC110 distinguishes separate SCAMP and
Block 2 banks with some observed overlapping DRAM fields. Those partial
read-back matches do not justify aliasing all registers or merging the banks.
The newly identified early call site therefore refines the remaining cold
initialization investigation; it is not proof that the width change alone
will produce a bootable machine.

1. Core internally records access width, but its public port read/write
   callbacks discard it. Byte and word writes of 80 to 22h are observationally
   identical at the App. An exact unlock protocol cannot be implemented by
   guessing width from value, inspecting Core private layout, or observing the
   BIOS PC.
2. The App has no CPUPWRMODE read receiver or configuration-unit state. This
   cannot be repaired by merely changing one cold SCAMP value.
3. Block 2's existing unlock state is real, but its App status port always
   returns FF. CPU configuration state and Block 2 unlock state must not be
   conflated into a fabricated common flag.
4. The existing memory map has C/F writable shadow mechanisms but no isolated
   SMM view. Board profile does not forward Core's existing SMIACT output.
   Reusing that output is a separate, concrete binding requirement; it is not
   a reason to add a second CPU executor or guest clock.

The proposed first public change adds `lib_u8 access_bytes` only to the
existing port-provider write signature. Direct unsplit dispatch passes 1, 2 or
4; existing eight-bit bank splitting passes 1 to each lane. Read callbacks and
wired-OR read dispatch remain unchanged: this receiver has not demonstrated a
need to extend them. Existing direct bus operations retain
their current one-byte dispatch semantics. No new public operation, optional
parallel callback, route registry, timing source or machine-specific dispatch
is proposed. Callbacks that do not need width explicitly ignore it.

A repository C-definition inventory initially finds 111 read/write candidate
callbacks in 57 C files. Restricting the change to writes reduces it to 55
candidate functions (21 production, 34 test) in 50 C files. This is an
estimate, not a claimed final caller audit. Header declarations and the
dispatcher also need migration. Estimated source/test delta for this width
contract, caller adaptation and focused existing-test extensions is
+100--180/-60--110 lines, mostly mechanical. Manifest and
documentation deltas are separate. Lib and Emulator remain unchanged.

The owner explicitly approved the access-width extension after reviewing its
scope. Implementation uses the narrower write-only contract described above.
Approval of the width contract does not silently approve guessed PC110 register semantics. The
subsequent receiver design must define configuration and SMRAM ownership,
reset persistence, decode/cache invalidation, actual SMI entry and RSM return
before claiming the complete batch repaired. Existing lane, byte/word/dword,
wired-OR and old-board behavior must remain covered. A Shared implementation
also rebuilds every affected admitted product in both host widths.

## Current disposition

The grouped observation is complete and its temporary sampler has been removed.
The approved write-width contract and its caller migration are implemented.
The existing registration fixture now asserts unsplit 1/2/4 widths with
unchanged values and exactly one callback; the existing neutral-bank and
locked CPU-bus fixtures require each split lane to have width 1. No new test
target, framework, callback path or production state is added to Core.

The App's existing configuration receiver now implements the source-labelled
L2 SL-compatible unlock sequence, read-only unlock-status bit, byte/word mode
writes and lock behavior. Its mode and two-step protocol progress belong to
the existing firmware/configuration binding, not to CPU architectural state or
a second copy of Block 2 data. CFGSTAT obtains Block 2 availability from the
existing side-effect-free selected-register read status; no mirrored unlock
Boolean or new chip getter is introduced. Full firmware reset clears these
transient controls and resets the chip's existing indexed-access protocol,
without fabricating or replacing its bank contents.

The existing App route fixture executes real CPU instructions to distinguish
an invalid all-byte sequence from the correct byte/word unlock, checks the
mode/status reads, uses actual word OUT instructions for relocking, and checks
reset while Block 2 is unlocked. Its first x64 focused pass succeeds; final
reset assertions also pass. Full regression/replay results follow below.

The source/test delta for these App controls is separately estimated at
+80--120 lines; the earlier width estimate excludes their real device protocol.
Lib, Emulator, their tests, owner INI, immutable firmware/font and MyNES remain
unchanged. The four receiving PC products are refreshed at current T580;
MyRaidenII's Core-linked receiving artifacts retain their existing 0007 identity.
The stale common PC version header is synchronized from 0575 to 0580 rather
than publishing files whose banner disagrees with the task version.

T580 and S47 remain open. The remaining SMRAM decode, SMIACT binding,
initializer predicates and software-trigger/acknowledge mechanism are not
silently accepted by the port fix. No POST, DOS, desktop or RDP success is
claimed, and no PC110 product artifact is published before qualification.

## Completed verification and actual-change review

Both stable `gcc-x64-release` and `gcc-x86-release` Ninja graphs build with the
repository ccache routing; subsequent unchanged builds report no work to do.
No alternative task-specific build tree is introduced.

| Verification | x64 | x86 |
| --- | --- | --- |
| Initial repository unit run, 712 cases | 710 pass; Types self-test timeout and stale HDC object failure | 711 pass; Types self-test timeout |
| Unchanged Types layout self-test, isolated without concurrent builds | Pass, 176.77 s | Pass, 96.75 s |
| Affected Core/NXVM/PC110 regression after dependency repair | 440/440 pass, 108.07 s | Included in the 711 passing initial cases |
| Existing PC110 units | 11/11 pass | 11/11 pass |
| Actual TFT BIOS timer integration | Pass, 156.58 s | Pass, 176.77 s |
| Deployed PC INI identity integration | 4/4 pass | 4/4 pass |
| MyRaidenII receiving unit cases | 13/13 pass | 13/13 pass |

Thus every one of the 712 registered unit cases has passing evidence on each
width, but neither initial full run is described as a single clean full pass.
Both timer runs retain `delta=1668`, `executed=59918870` and status 0; this is
the existing timer checkpoint, not a later POST or guest boot assertion.
The four changed manifests, Core/NXVM inward-dependency/platform gates,
documentation governance, artifact-root verification and whitespace check pass.
No new desktop/RDP or complete guest boot validation is claimed.

The HDC failure was traced to an old x64 object with an empty Ninja dependency
record: the test includes production HDC code, which still used the old callback
ABI in that object. Inventory found 94 such Core/NXVM test objects. Their exact
regular-file paths were checked inside the original build tree before removing
only those invalid objects and rebuilding. No source workaround, test weakening
or cache-root deletion was used. Native dependency compaction/rebuild also
restored the x86 tree. Both trees now have zero empty dependency records; the
unchanged HDC regression passes and subsequent builds are incremental.

Actual C/H delta against `98889bc39`, excluding manifest hashes: 59 files,
289 added / 82 deleted / net +207 lines. Shared accounts for +157/-60;
App source/test accounts for +132/-22. Of the latter, DeskPro is signature-only
(+4/-2), PC110 source is +74/-17 and its existing route test is +54/-3.
The width migration itself introduces no Core state, helper, alternate callback,
queue, inference from value or changed error handling. Two App-owned fields
represent the actual configuration register and its unlock progress. The one
new test helper executes real CPU instructions through existing public debug
operations; it is not a production entry or independent I/O interpreter.

## Receiving release artifacts

Shared implementation is P1 `d603e81e7`; the App receiver and this evidence
are delivered as NXVM P2. All eight PC artifacts use 0.5.0580 and their
expected PE width. All ten refreshed PC/MyRaidenII artifacts have no debug
sections. Six obsolete 0575 files are removed only after replacement
verification and remain recoverable in Git. INI contents are untouched.

| Artifact | SHA-256 |
| --- | --- |
| `assets/my5160/my5160_0_5_0580_x64.exe` | `CF40FC937479B698FE7EBFD422418A64E791604F6169D075EE21718E62FD46EC` |
| `assets/my5160/my5160_0_5_0580_x86.exe` | `4D4BEE9021308955DB220249122DD73DC3E7AC4280CE8189CB7595287007CF91` |
| `assets/my5170/my5170_0_5_0580_x64.exe` | `7B96DFB65D6C7024098C1E210945E5013775929EB7D45A9FBBC12583D7B28993` |
| `assets/my5170/my5170_0_5_0580_x86.exe` | `AB6B3AF976B1497EC0C356FFB56730CADA7AEDEB3734B00908D2A392C117E9AA` |
| `assets/mydeskpro386/mydeskpro386_0_5_0580_x64.exe` | `972960FF8F31F60E2BC3DF06898D586DBFC56AC7C4596BDC2D070FC039D8FE59` |
| `assets/mydeskpro386/mydeskpro386_0_5_0580_x86.exe` | `3A4ADCA6E3BBEA8EB2977A3BDCAD3469326F00E977B9387F71ED7D889CE02347` |
| `assets/nxvm/nxvm_0_5_0580_x64.exe` | `302DAF76C142FE0B05663475313704951E03C599A7D43F29A6417FF36E34E3FC` |
| `assets/nxvm/nxvm_0_5_0580_x86.exe` | `F96D7E7F2AB6BABA3FE7AA9BB5387D229886CEB4A81CADBCDE89A276B9ED46E8` |

MyRaidenII receives the same rebuilt Core in its unchanged 0.0.0007 pair,
recorded separately in the receiving product's evidence and P commit. MyNES
does not link this Core and its existing pair is intentionally unchanged.
PC110 TFT/DSTN previews compile on both widths, but no unqualified PC110
release is added to assets.

## Remaining complete receiver boundary

This prerequisite does not finish cold SMRAM initialization. Existing Core
attachments expose SMIACT, but Board's profile binding does not forward that
output to the App. Existing memory-provider queries expose address, size and
access, but not CPU-versus-DMA initiator scope. A declined query falls through
to RAM; no CPU-only SMRAM property exists. The paused machine observation
interface rejects RUNNING state, so it cannot be borrowed inside a memory
callback to guess the initiator from private transaction state.

The remaining batch must design the CPU-only SMRAM view, SMIACT forwarding,
initialization, software trigger/acknowledge and reset together, with source
grades and actual BIOS proof. It must not implement an unconditional second
RAM view, a BIOS hook or a second execution path. Any further Shared contract
extension needs its concrete design and review; the approved write-width
change does not implicitly approve a speculative memory API. The SCAMP
predicate remains unresolved, not silently given guessed W1C semantics.
T580 and its product qualification remain open.

## Post-delivery replay and shadow-decode reconciliation

Baseline `54dfcea04`. One temporary mode in the existing integration harness
replays the unchanged TFT BIOS through the first post-timer service write.
It observes real retirements and never injects SMI, changes guest registers,
patches firmware or fabricates a service response. Its source delta was
+80/-5, removed in full afterward. The native x64 process exits normally
within approximately three minutes; no duplicate x86 diagnostic was needed
to decide the next receiver boundary.

### New actual-path facts

- The first word read of 22h is zero; after the real byte/word unlock, the
  BIOS reads 0081, selects 008B/0083/008F and finally relocks with 0101.
- CFGSTAT now reads 7F. The BIOS proceeds into the Block 2 programming path,
  writing, among others, 60=00, 61=09, 84=EE, 85=15, B8=80 and FA=01.
  These are newly observed firmware writes, not host-seeded success values.
- At F424D the first SCAMP predicate reads 02; the F4250 result has ZF set
  and skips the early initializer. F3F4A executes later. At F4CF2 the same
  register reads 82 with CF set, skipping the later initializer.
- The selected initializer and handler checkpoints do not execute. The final
  observational read at 38000 returns five zero bytes.
- The run reaches its first post-timer 15EE write with timer pass recorded,
  59,993,605 retirements, three processor resets and status 0.

The diagnostic counted 609 selected I/O events but printed only the first
128. Therefore its printed I/O sequence is explicitly incomplete, not a
complete control trace. The separately observed branch, timer and terminal
facts above are retained independently of that output cap. Absence of a fixed
physical checkpoint still does not exclude relocated aliases.

### Additional original source

The VLSI VL82C480 Product Specification, scan stamped August 12, 1993, was
acquired on 2026-10-25 into the external manual archive from
<https://bitsavers.trailing-edge.com/components/vti/pc/VL82C480.pdf>.
SHA-256: `dc7205bc3d05d327b7f33e7ba2edc83846149dff623a84414a8f426ce1a2e7be`.
The relevant original pages were rendered and visually inspected, rather than
trusting the image-only PDF's empty text extraction:

- Printed page 3 distinguishes four ROM/slot-bus versus system-board-DRAM
  read/write choices on 16 KiB boundaries.
- Printed page 20 defines ROMSET[6:4] as ROMMOV location control, not a
  standalone shadow-write lock. It separately identifies MBIOS and flash
  programming control.
- Printed page 21 defines each AAXS--FAXS two-bit field: 00 external read/write,
  01 external read/internal write, 10 internal read/external write and 11
  internal read/write.

Those are exact VL82C480 facts. Applying them to VL82C420 remains L2 and needs
the actual PC110 BIOS and live register-map anchors; no full chip equivalence
is claimed. The read-only Open-Source-PC110 `Discovery/Chipset/readme.md`
SHA-256 `70e05e07fdfcf5786a5de6ab5787b6726032016151e0a6a19b1b61e65794f806`
records EC/ED CAXS=0F, EAXS=11 and FAXS=12 from firmware and live read-back.
Its informal ROMSET open/relock shorthand cannot substitute for the original
location-field definition. The 86Box VL82C480 reference at commit
`4fef696a4eead1d55a28d6ac0e5bd2864e5454da`, file
`src/chipset/vl82c480.c`, SHA-256
`b4bd4f1b4d3e13707f51772ad0806f0b9ce0a026b279bbaafac6022ee56f8a5e`,
independently models separate access fields and ROM select. It is a behavioral
cross-check, not imported source or proof of PC110-specific ROM wiring.

### Concrete discrepancy and next complete design batch

The current App has only `video_shadow_writable`, derived from ROMSET bit 5.
It always reads the prefilled C shadow, always reads/writes the F shadow and
does not consume CAXS/EAXS/FAXS access selections. Thus the complete four-way,
16 KiB read-source/write-target mechanism is absent, not merely an incorrect
initial byte. The current all-C/F behavior must not be supplemented with another
special-case SMI buffer or a guessed SCAMP bit-7 rule.

The next receiver design must replace that mechanism coherently:

1. Keep immutable Flash, internal shadow storage and ROM location selection
   distinct. Derive each access from the chip-owned segment field plus the
   source-qualified board ROM decode; do not mirror the indexed registers in
   another App array or alter the selected-index latch during a memory query.
2. Define cold state, CPU-only reset and full reset for all C/E/F windows,
   including the existing high reset alias. A register write changes decode,
   not pre-copied guest instructions. Guest firmware performs shadow copies.
3. Reconcile both initializer callers, the C/E copy and patch sequence, actual
   SMRAM preparation, SMIACT view and source acknowledgement as one batch.
   Do not publish a partial shadow change as a complete cold-boot repair.
4. Reuse existing route and CPU execution owners. Only after the complete
   decode table is specified should any minimal chip observation, memory
   initiator or Board SMIACT contract extension be admitted and counted.
5. Extend existing App fixtures with table-driven 00/01/10/11 checks, 16 KiB
   boundaries, instruction/data consistency, reset and immutable-input proof;
   use actual unchanged-BIOS initialization/copy/SMI/RSM as the final receiver
   proof. A synthetic mode check alone cannot qualify firmware boot.

This research changes the next action from adding a software-SMI trigger to
completing the shadow/ROM decoder design first. It establishes no final ROMMOV
truth table for PC110 and therefore retains no speculative production edit.
Lib, Emulator, all other Apps, owner INIs and release artifacts are unchanged
in this diagnostic continuation. Temporary diagnostic code and rendered manual
pages are removed; the original manual remains in the external archive.
