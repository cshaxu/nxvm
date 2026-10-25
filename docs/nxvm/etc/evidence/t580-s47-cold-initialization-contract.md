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

## Register-qualified shadow decoder receiver

This continuation starts at `24bfb9ce9`. The earlier observations above describe
their respective baselines; they are not claims about the decoder below.

### Retained mechanism and source limits

One App-owned, cold-zero 256 KiB shadow replaces the prefilled C and permanently
writable F allocations. The chip remains the sole indexed-register owner. A
bounded observational `x86_vl82c420_read_register()` reads the existing bank
without changing its selected index, unlock protocol or bank permissions.
Selected I/O reads use that same operation. App memory queries no longer mirror
the EC/ED index or derive a shadow-write Boolean from ROMSET.

Each 16 KiB C--F field selects external/internal reads and writes independently.
C external reads return FF. D external accesses decline to the existing font
route or explicit unpopulated-memory fallback. E external reads expose Flash
bank 2 when the admitted ROMSET location bits are clear; F external reads expose
bank 3. External writes never change immutable Flash. The high reset alias
always selects the immutable F bank, independently of mutable low shadow.
Instruction fetch and data read use the same provider; the former forwarding
instruction callback and setter are removed. Full reset clears shadow and
restores the cold access fields; processor-only reset preserves them through
the existing Core reset distinction.

The additional read-only Open-Source-PC110 `Discovery/BIOS-Flash/readme.md`,
SHA-256 `a4a1885b2036fca5ac6062583b74b45a3350f8ec65dadb8d535606dd14c7e123`,
records hardware probes of runtime E/F Flash banks 2/3, C shadow and absent D.
It also explicitly distinguishes the boot-only lower bank 0/1 view. That
boot-only selection is **not yet implemented**; runtime mapping evidence is not
claimed as a full cold-ROM truth table or Easy-Setup qualification.
The related VLSI 480 access fields, PC110 register anchors and actual BIOS copy
path support this receiver as L2, not a recovered VL82C420 silicon specification.

The first decoder replay also disproves S30's assumed F9 hard gate: F3BE9
writes F9 and returns to F3B43, whose F3C52 call uses EC/ED without another FB.
The hard gate returns FF and discards required register writes, yielding
ROMSET 8F/AF and CAXS EA rather than the programmed copy/lock sequence. The
diagnostic was stopped once that conflict was established. The App now accepts
F9/FB as L2 compatibility strobes without inventing inaccessible-bank semantics;
its gate Boolean is deleted. This explicitly supersedes that S30 model, not
its historical evidence. Read-only reference models agree on accessible EC/ED:

- PC110-EMU `81235b5fdf7d17ae180fc5cb6148e44bcbb87b4c`,
  `Sources/PC110Core/pc110_core.c`, SHA-256
  `d484454beb37a9d3eef5cf716fcef80e8131772c82532df3a411704c57bf898a`.
- PC110-QEMU `bae390ebf21665ec97860d28e26ec46f1fe8f4e9`,
  `qemu/hw-misc/pc110-chipset.c`, SHA-256
  `950985a7c280f10d9e028317f853a0b0d53d4bd771fb8283ef3f85a95682028b`.

These implementations are behavioral references only; no source is imported.

### Actual unchanged-firmware result

Both host widths execute the guest's E/F relocation, C copy/patch and F patch
sequences. CAXS ends at 2A; the guest changes the Option ROM header from
55-AA-47 to 55-AA-60. The early SCAMP predicate now takes the initializer path
without a host-forced flag. The BIOS reaches E900:0006, E907B and the E912F
copy, opens 302 bit 3, prepares the handler at 38000, closes 302 and returns
through F3457. The bounded temporary sampler reports on both widths:

```text
COLD:done service=1 initializer-return=1 executed=6381568 resets=0
status=0 reason=1 pc=000f47c6 handler-status=0 handler=ea005b0038
```

The temporary sampler was removed in full. The existing timer integration now
also requires the actual completed-copy and initializer-return observations,
not merely a final successful timer count. It passes in 37.70 s on x64 and
44.73 s on x86, with both reporting:

```text
PC110:TIMER:passed=1 failed=0 delta=166e executed=11844085
pc=fffffff0 status=0 copied=1 initialized=1
```

It records one processor reset and no delivered exception. The earlier path
needed about 59.9M retirements and three resets; eliminating the incorrect
decode path is not instruction skipping or a host-speed timing model.
Neither result proves an SMI/RSM transaction, isolated SMRAM, complete POST,
Setup, guest boot, display, desktop or RDP behavior.

### Regression, review and retained boundaries

The existing firmware fixture uses a table over three windows, four fields and
four access choices, plus boundary writes, immutable Flash, non-mutating index
read-back, high alias and full reset. Existing configuration and construction
fixtures retain actual CPU fetch/OUT and font/open-bus checks. The changed chip
fixture checks null output, index masking, lock rejection and preservation of
the selected index. No test target or framework is added.

Both stable Ninja graphs build the affected App units and both TFT/DSTN
previews. All 711 ordinary unit cases pass per width (229.13/230.78 s), with
the compiler-spawning Types self-test scheduled separately to avoid competing
build subprocesses. Their isolated runs pass in 104.41 s (x64) and 113.86 s
(x86), completing all 712 current cases on each width. Source/test Core manifests,
dependency/platform gate and whitespace checks pass.

Actual C/H delta against this continuation baseline, excluding manifests,
documentation and artifacts: 12 paths, +313/-216, net +97. Shared is +26/-1;
NXVM is +287/-215. The increase is primarily table-driven regression coverage;
production alone is +173/-170, net +3. The shadow allocation is one owner, not
an additional handler cache. Separate read and write chunk loops retain their
different effects; both consume the same decoder. Both existing App constructor
paths receive the D fallback; neither adds a new construction path.

The source change in Shared affects only the VL82C420 library. No currently
published PC, MyNES or MyRaidenII executable links that library; PC-family
product-target incremental checking reports no work. Their accepted earlier
S47 artifacts remain current, so no timestamp-only binary refresh is made.
The affected PC110 previews rebuild, but remain unpublished pending real product
qualification. Lib, Emulator, other App sources, owner INIs and external input
masters are unchanged.

The complete S47 receiver remains open for CPU-only SMRAM decode, SMIACT
forwarding, qualified software source/acknowledge/rearm and actual RSM return.
The Core already emits SMIACT; Board's profile contract does not forward it.
The shared memory query currently cannot distinguish CPU and DMA initiators.
These are genuine remaining contract boundaries, not reasons to inject the
handler, unconditionally trigger SMI or declare this decoder a full boot fix.

## Active-view and ASMI receiver continuation

Continuation baseline: `af5e1ff04`. The existing CPU output already brackets
state save and RSM restore; Board now forwards that physical SMIACT level to
the App's firmware binding. One additional CPU-only overlay mode uses the
existing memory resolver. CPU instruction/data/inspection accesses see the
selected SMRAM view; DMA resolves its ordinary RAM route, including aliases.
The App extends its one shadow allocation by 64 KiB, not by a second execution
path or handler cache. IBU-selected 302 bit 3 opens the view during preparation;
SMIACT keeps it selected during SMM. Full reset clears transient contents and
controls; processor-only reset does not call the firmware full-reset provider.

One bounded decode-change notification invalidates CPU cache/prefetch contents
from the old view. It reuses the existing range invalidation algorithm but is
not subject to the NW bit that suppresses ordinary write snooping. It neither
executes an instruction nor advances time. Public device callbacks and the
existing four-lane access-width contract remain unchanged. Lib and Emulator
are not modified.

### Source reconciliation and rejected draft

- Original *Intel's SL Architecture: Designing Portable Applications* (1993),
  printed pp. 169--170 (PDF 188--189), distinguishes CPU SMRAM from ordinary
  memory used by DMA. Its pp. 59/84 establish ASMI's indexed address at 84/85
  and B5 bit 2 enable. The archived PDF hash remains
  `3BFEA5A3C1B3BDC6A03BF2E622F24A4522123E3D0AB810386E02E3C72A94755B`.
- The actual CPU-lock sequence must not relock the independently unlocked
  Block 2 bank. Removing that coupling allows the initializer's real writes
  to reach B6=90 rather than disappear behind FF read-back.
- The unchanged selected BIOS's complete initializer at Flash offsets
  2B3DF--2B47B registers the vendor handler at 3800:2774 for source mask 20,
  then configures 84/85 and B5 bit 2. Its dispatcher at 2EB4B acknowledges
  B7 sources with `~mask & 7F`. This is the direct evidence for the retained
  L2 bit-5 ASMI model, B7 W0C acknowledgement and FA=1 rearm. It is not a
  claim of a complete VL82C420 silicon register specification.
- An uncommitted bit-7 draft was rejected. Mask 80 invokes power management,
  eventually writing FF=82 and halting at 3800:1333. This is a wrong-source
  experiment, not proof of a BIOS hang or authority to bypass HLT. Neither
  the failed trial nor the test fixture's missing required attachment finalizer
  is retained as a product defect.
- The wrong-source trial also exercised an existing missing F65535 DAC status
  read. The archived C&T 65535 Revision 2.0 register reference, printed
  pp. 77--78, specifies 3C7 read status 00 after a 3C8 index write and 11 after
  a 3C7 index write. SHA-256:
  `ACB95E9BD56786227B6B9201B3BFF317ADBD224D1A587FBF8DB515FB5A648A91`.
  The five-line chip correction and existing DAC regression retain this real
  operation, without claiming it caused the rejected ASMI branch.

The extra original Intel 241325-001 November 1992 data book was acquired from
the [Bitsavers mirror](https://bitsavers.trailing-edge.com/components/intel/80486/241325-001_Intel486_SL_82360__Microprocessor_SuperSet_Data_Book_199211.pdf)
and archived externally as `manuals-nxvm/cpu/intel-486sl-82360-241325-001-1992.pdf`,
SHA-256 `7E6A936BD8BA057ED642F7E3777C9A20A9944F30300F8A65F4B3E83AC75BE16F`.
It supplies electrical/background material, not an ASMI status-bit definition;
no register claim or implementation is derived from its absence of that table.
No external source, ROM, font or manual is imported into tracked repository code.

### Actual unchanged-firmware proof

The bounded x64 grouped replay confirms the prepared handler, naturally
triggered after initialization by 15EE with AX=5380/BX=8301. The dispatcher
reads B7=20, writes 5F, invokes 3800:2774 and executes RSM at 3800:5BC8.
RSM returns to F000:DD11 with the original AX/BX; no fault or delivered
exception occurs. The run exits normally after 6,385,235 retirements with
zero resets. These are observed guest operations, not host callbacks that
implement the firmware service.

The temporary `--smi-contract` mode and selected-port print sampler are removed.
The existing timer integration instead requires initialized copy, actual ASMI
request, vendor handler entry, RSM and timer pass in one execution. The existing
App configuration fixture proves CPU-only open/close, DMA read/write isolation,
the actual CPU port-unlock sequence, SMI entry, acknowledgement/rearm, RSM
register restoration and full reset. Core memory and CPU-cache fixtures cover
the neutral route and NW-independent decode change; no new test target,
framework, alternate clock or firmware-address production exception is added.

### Continuation review and regression

Both final host-width executions report:

```text
PC110:TIMER:passed=1 failed=0 delta=166c executed=11860908
pc=fffffff0 status=0 copied=1 initialized=1 asmi=1/1 rsm=1
```

The initial enhanced integrations pass in 34.40 s (x64) and 40.49 s (x86).
After the last App construction precondition cleanup, the x86 replay passes
again in 39.88 s. The source delta does not change the successful route:
configuration installation now requires this firmware's memory routes on the
same Core, and only publishes the borrowed chip after port installation
succeeds. Both App unit sets pass 11/11, including rejection before memory
installation and full-reset SMRAM clearing.

The 711 ordinary repository units pass per width: x64 381.20 s, x86 137.84 s.
The compiler-spawning Types layout self-test is excluded from those aggregates
and scheduled separately; its qualification is recorded separately rather than
silently included in 711. All four source/test manifests changed here validate;
both Core/NXVM dependency/platform verification targets and NXVM documentation
governance pass. Whitespace checks pass. No warning suppression is added or
removed, and existing CPU instruction-file exceptions are not requalified as
strict-warning-free by this delivery.

Actual C/H change against `af5e1ff04`, from `git diff --numstat`, excludes
manifests, README, task documents and generated/artifact files:

| Owner | Paths | Added | Deleted | Net |
| --- | ---: | ---: | ---: | ---: |
| src/x86-core | 11 | 139 | 42 | +97 |
| test/x86-core | 11 | 121 | 32 | +89 |
| src/x86-nxvm | 4 | 15 | 0 | +15 |
| test/x86-nxvm | 1 | 5 | 0 | +5 |
| src/app-mypc110 | 6 | 146 | 67 | +79 |
| test/app-mypc110 | 2 | 107 | 10 | +97 |
| Total | 35 | 533 | 151 | +382 |

Production is +300/-109, net +191; tests +233/-42, net +191. The positive
production delta implements the previously absent initiator-qualified memory
view, cache transition and chip source protocol. Most App profile callbacks
are moved from the old plan owner, not added parallel callbacks. Test growth
extends five existing behavior fixtures and the one actual firmware replay;
the other RAM-test edits are mechanical private-signature migration. No new
test executable or test framework is introduced.

Actual-change review checks all added/removed C/H hunks and their call sites:
the sole resolver filters CPU-only providers in every applicable lookup pass;
DMA preflight and data use the same initiator qualification; provider declines
and errors retain existing behavior. Ordinary DMA snooping still honors NW,
whereas a changed memory view does not. Board forwards one existing CPU output;
the App owns the resulting decode. Initial ASMI is disabled until the guest
enables it. Pending source, acknowledgement and rearm stay chip-owned, including
a request between acknowledgement and rearm. Neither App constructor owns a
second CPU, SMI queue, clock or firmware copy service. Lib, Emulator,
X86 Debug, X86 IBMPC, MyNES and external ROM/font/media/configuration masters
remain unchanged.

This qualifies the cold-memory/SMRAM/ASMI receiver, not whole POST, Setup,
DOS/Windows, live PC110 panel/input, desktop, RDP or a PC110 release. The
boot-only lower Flash view and remaining product/guest ledger rows are not
accepted by reaching RSM or the timer checkpoint.

### Final qualification and receiving artifacts

The isolated Types layout self-tests pass in 104.24 s (x64) and 128.85 s (x86),
completing all 712 current unit cases per width. The final x64 BIOS replay
passes in 34.50 s, retaining the identical 166C/ASMI/RSM result; the final x86
replay above passes in 39.88 s. These final runs include the App construction
precondition correction. Both complete App unit sets pass after the final
assertions. Subsequent `build-unit-tests` builds in both stable trees report
`ninja: no work to do`; no second build graph or cache reset is used.

Four real deployed PC products pass the executable-adjacent INI identity
integration on each width, including wrong-name rejection and INI-relative
media paths. The four PC pairs retain 0580 and the Core-linked MyRaidenII pair
retains 0007. PE machine widths are checked for all ten affected artifacts;
the MyRaidenII receiving files are stripped of compiler debug information.
MyNES does not link Core and its deployed pair remains unchanged.

| Artifact | SHA-256 |
| --- | --- |
| `assets/my5160/my5160_0_5_0580_x64.exe` | `56F9E50F0E2DA03AF39DB966AE7861E91159645FAD4189ED90EF8B409AB396D6` |
| `assets/my5160/my5160_0_5_0580_x86.exe` | `A83DE76FC89BB294093280495B9C439F26333813D4C655BA3CD94C2A6F6F02C4` |
| `assets/my5170/my5170_0_5_0580_x64.exe` | `C7600E739D9FF4A201F8D6D36D11E87E96AAA8108B7CE6A884238816A388752A` |
| `assets/my5170/my5170_0_5_0580_x86.exe` | `75160E358A8D4D6833E73A035251D5EAFC0F502650737AEF1323E27F9DF4097D` |
| `assets/mydeskpro386/mydeskpro386_0_5_0580_x64.exe` | `B70046EB266142C99E1B3883AF77C2F9C399615A68C8FB5D062F59487B39EADB` |
| `assets/mydeskpro386/mydeskpro386_0_5_0580_x86.exe` | `ADA8C3313EE18E05647132C8BF09E1CA99CA20A06067C97E4EFE925CF216BD89` |
| `assets/nxvm/nxvm_0_5_0580_x64.exe` | `1AED78193A79733F73BF71A39821B3EB3E4C3EF97474BC51A6482209AE51C332` |
| `assets/nxvm/nxvm_0_5_0580_x86.exe` | `7EE66D812544C6DE2EBB36C97D1C6A37B931C0DC6B66BD17BBA310DA09FF253E` |
| `assets/myraidenii/myraidenii_0_0_0007_x64.exe` | `31C09F8E8674182720126952F4D7B775E9CA0E1C3B4B1CDC944152AE42CBD76E` |
| `assets/myraidenii/myraidenii_0_0_0007_x86.exe` | `E24D9FB666AE005C94459D26539B8D2A72DEC54FCF20A87898F10553F331977A` |

No PC110 preview is deployed or relabelled as a qualified product. The existing
TFT and separately labelled derived-DSTN previews rebuild only under the two
stable Ninja trees. Owner INIs, snapshots, disk media, ROM and font masters
are unchanged. Native automatic unit/INI checks are not owner desktop, RDP,
PC110 guest or MyRaidenII game-play acceptance. Other guest integrations are
not rerun by this bounded S47 continuation.

For the whole S47, against original baseline `98889bc39`, tracked C/H changes
are 39 production paths +577/-296 (net +281) and 49 test paths +533/-128
(net +405): 88 paths, +1110/-424, net +686. This includes the earlier approved
write-width and shadow deliveries, not merely the final continuation table.

### Temporary-output disposition

The x64 ordinary-unit aggregate is retained as its terminal result above;
CTest's rotating native log is not a promised durable raw transcript. The
following S-owned verification logs have their results copied above and hashes
recorded before removal after process exit. Raw logs are not fixtures or
permanent cache content.

| Temporary log | Bytes | SHA-256 |
| --- | ---: | --- |
| `x64-app-unit.log` | 1650 | `A06E81F96773131614637586B66563B32F80DBAEBD0D9B22E57356D72E1FF8B7` |
| `x64-final-build.log` | 3168 | `33B1DBE447A43A715F45D6F93FF68416A2BAB153BE3A1B85BAD032CFE075BDD8` |
| `x64-final-timer.log` | 394 | `81D23C46E104F9649C83C069C42ABECD83B8F195A412A45971C324402F2BB4CA` |
| `x64-types-layout.log` | 357 | `9E75B6B1923971B1B4E6F2B5AC3E182A71E9965822DC7C23C21D661B35F170B9` |
| `x86-app-unit.log` | 1650 | `D4B5D9077EEE892B4A84A8A6CDA2C37296712F7EB5A861B4B9360ABE94C1A2B2` |
| `x86-final-build.log` | 2219 | `5940A87A7BE2CF8CE5945468500B55CC830534361BD2E282F09905D06C10283B` |
| `x86-final-timer.log` | 394 | `44C2B4E084EC6E44761F1EBDE50F32F72D0235F40C6ED59C7CA194021FAC6FBE` |
| `x86-types-layout.log` | 357 | `C5B6ABA824BF27FD23E4F176D853DB22C65EAF859370E1514C40EDAD931A848F` |
| `x86-unit.log` | 121556 | `787D6544CCE3243DA4256A1BC35BB4347645B0BC520B171A9F244D81AF190606` |

After terminal completion, the real cleanup tool previewed and removed those
nine logs (131,745 bytes). Four PDF scratch files (4,841,153 bytes) were removed
after preserving the original externally and recording source hashes. Eleven
owned, no-longer-registered diagnostic EXE copies from earlier S47 probes
(25,716,665 bytes) were removed from the two existing test output directories
after checking current Ninja targets and absence of matching live processes.
Total disposable cleanup is 30,689,563 bytes. Current integration/previews,
valid Ninja/ccache products and all owner inputs are preserved.

Shared receiver implementation is pushed as `794380f69`. The companion NXVM
delivery carries App source, this complete evidence and the eight PC release
files; MyRaidenII's two receiving artifacts use the next separate-target P.

Coordinator actual-change review accepts Shared `794380f69`, NXVM `782f09ebb`
and MyRaidenII `5ed407b4b`. Review maps every changed route, callback, state and
test to the admitted cold-receiver batch and preserves the separate remaining
product rows. S47 is closed; T580 and the PC110 product goal remain open.
There is no outstanding owned native process or unsubmitted implementation,
and no PC110 release is inferred from this receiver acceptance.
