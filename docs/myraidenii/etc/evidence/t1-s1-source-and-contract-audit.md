# T1 S1: Source, Set And Contract Audit

## Request, Baseline And Role Cycle

The owner requests full MyRaidenII implementation in one-session dual-role
mode. Coordinator admits T1 S1 at clean f21a22997 after M0 acceptance;
executor reads the packet and authorities, questions reuse assumptions and
delivers this complete source/contract audit. Coordinator reviews actual diff,
source/asset observations and ledger before the separate acceptance P.
This batch changes product documentation only, not a runnable path or ABI.

## Actual Asset Audit

After an unsuccessful filename search of the existing external archives and
public authorized-distribution research, the owner supplied a local
`raiden2.zip`. No game was downloaded, copied, extracted to tracked source,
modified or embedded. The audit streams ZIP members read-only and compares
length/SHA-1 to the pinned original parent reference, also computing SHA-256.
Machine-local paths are deliberately absent from committed records.

The 10654480-byte ZIP has SHA-256
`9fedc8ddac2208408c3b05201316e3f2db43bdfb104551ba17337bb55db13d71`.
All 13 runtime images match original V30 `raiden2`, US set 1. Program identities
exclude the later V33/DX board and PC ports. This proves data identity, not
emulator execution or third-party redistribution permission.

Both background images use different socket-name spellings from the pinned
reference: actual bg-1.u0714 matches reference bg-1.u075, and bg-2.u075 matches
reference bg-2.u0714. Role assignment follows the verified contents, not a
blind renaming/sorting rule. The original archive stays unchanged.

The archive's one extra JED is not either binary PAL dump listed by current
MAME. The selected emulator runtime needs 13 images; it is incorrect to claim
all 15 reference ROM records match or to treat a JED as the missing binary.
Audio `snd.u1110` is 65536 bytes although its banked/copy-populated region is
131072 bytes. Sample regions likewise exceed their actual 262144-byte files.
Full metadata is in [inventory](t1-s1-rom-inventory.json).

## Sources Actually Inspected

The fixed MAME source/hash from M0 was fetched in memory and reverified, not
imported. Its board configuration, runtime ROM list, main/sound maps, IRQ
vector and palette setup were inspected. It establishes research leads: V30
16 MHz, sound-domain crystal divisions, 320x240 native raster/ROT270, 2048
palette entries and main interrupt vector 0x30. The tuned raster is L2, not
proof of silicon timing. Source remains LGPL-2.1+; no code/table import is
authorized by this audit.

Original NEC October 1986 and Zilog/Yamaha manuals were acquired to ignored
research output and pinned in the source register. NEC PDF pages 24--36 and
46--60 include interrupt, reset, mode and repeat-prefix evidence. The reset
CS/IP model is FFFF:0000, native mode is selected, and undocumented register
reset values must not be claimed as hardware facts. MD is architectural PSW
state, not a parallel board-mode Boolean. Existing EFLAGS=2 reset is not enough.

The operator manual's three scanned pages were rendered and visually read:
vertical JAMMA output, two players with directional controls and two buttons,
coin/start/service lines and DIP documentation. It supplies no complete board
schematic, CPU execution corpus or game-stage enumeration. Do not promote its
battery warning into a protection algorithm or silently import modern PCB lore.
OKI datasheet mirrors were located, but attempted acquisition returned 404 /
an interactive challenge; no byte hash or full inspection is claimed.

## CPU Extension Audit

The mechanical profile-order query was:
`rg -n '(cpu_profile|profile)\s*(>=|<=|<|>)\s*(X86_CORE_CPU_PROFILE_|metadata.minimum_cpu)' src/x86-core/cpu src/x86-core/machine --glob '*.c' --glob '*.h'`.
It reports 306 matching lines: 297 instruction-unit lines, six timing-model
lines, two Machine lines and one instruction-header line. This is an inventory
of candidate assumptions, not 306 proven bugs or a count of necessary edits.
Later CPU qualification must classify every reachable candidate, not blindly
rewrite comparisons or assign V30 an Intel generation rank.

Confirmed mechanisms needing V30 work:

1. Public profile enumeration and private validity/default/reset selection.
   Preserve existing enum values; explicit membership must replace range-based
   admission. Existing 8086 helpers cannot simply be redefined to mean V30.
2. Primary/NEC opcode selection, prefixes, form admission and operand decoding.
   `INS_0F` currently distinguishes Intel early POP-CS and later Intel maps;
   V30 needs its own map. Reuse proven ALU/addressing handlers after semantics
   review; do not clone an interpreter or alter the original table style.
3. Lexeme scanner and live preview currently reject unknown profiles and
   interpret 0F through Intel metadata. NEC instruction lengths, prefix effects
   and actual execution must agree through one owned NEC description.
4. FLAGS normalization, reset, interrupts, block restart and documented mode
   transitions need NEC evidence. Unsupported forms must not become NOPs or
   an invented exception. Full V30 includes 8080 emulation, not the Z80 core.
5. `cpu_timing.c` needs a V30 selector/model and source-grade identity, including
   prefix/branch/repeat/entry/wait terms. No generic Intel fallback qualifies it.
   Existing bounded maximum-time queries and preview/retirement observations
   must cover the same family. No second clock or host-time instruction cost.

Proposed later Shared surface is a V30 profile and, where required, NEC decode
space / timing origin values in existing enums. No new caller-visible raw CPU
layout, board callback, manager, plugin registry or public family facade is
justified here. Exact additions need their own reviewed CPU S. CPU/FPU remains
an inward dependency; no new FPU implementation is planned for this board.

## Existing Machine And Host Contracts

Initial reuse choice is neutral X86 Core Machine, not App-private CPU execution:

- Public memory device routes can represent RAM, ROM and mapped board devices;
  inspection is observational, and failures retain their existing contracts.
- Interrupt pending/acknowledge callbacks return the board's vector without
  introducing an IBM PIC. For this reference board the vector is 0x30.
- Attachment deadline and before/after-advance callbacks allow board/COP/video
  and sound-domain progress on Core's single elapsed-tick axis. Null PC-specific
  refresh/DMA/cache hooks do not require fake PC devices.
- Retirement-time qualification is explicit and currently has no V30 rows.
  Adding the profile without source-graded costs/admission is not enough.
  Direct CPU creation is public, but choosing it solely to duplicate the existing
  Core timeline/runner would be a second owner, not simplification.
- Emulator already owns the sole host worker, lifecycle rendezvous and input/
  frame publications. Board callbacks must service bounded safe points;
  no sound CPU thread or second Session provider is needed.
- Lib PCM accepts mixed signed-16 PCM; it does not synthesize YM2151/OKI/Z80.
  Existing graphical dimensions admit the native and rotated geometry.

One contrary M0 assumption is now corrected: the KVM graphics format is
indexed 8-bit with 256 palette entries. A 2048-entry board palette cannot be
promised lossless output through that contract. Source/test reference scan
found four direct production users and 14 test files across Lib, Emulator,
X86 NXVM and MyNES (including an NXVM firmware consumer test). This is not
a complete future change manifest, and no production file was modified.

The owner explicitly declined the proposed RGB change and requires no Lib
edit. The [High TODO](../../states/TODO.md) retains the proposal, affected
owners, risk and re-admission condition. No colour quantization, second output
path or weakened full-game exit is authorized. CPU/board research can proceed;
full-colour release remains a genuine gate.

## Verification And Remaining Boundaries

The [freeze ledger](../t1-freeze-ledger.md) maps sources/assets/contracts to
their actual implementation/test owners. All runtime features remain
unimplemented. T1 stays open for the remaining source/contract/checkpoint
freeze; the declined Lib work is retained TODO, not silently admitted.

Independent ZIP re-read verified all 14 members against recorded lengths,
SHA-1 and SHA-256. A second fixed-reference parse verified all 13 runtime
records, summing ROM_CONTINUE into sound-file length and excluding ROM_COPY
from file size. Both checks passed; neither is a test of an emulator loader.
The documentation gate and whitespace check passed, and all ten deployed EXE
hashes match their unchanged T563 S10 baseline. Final link, 16-field packet,
ordered-queue and target-only diff review accompanies the complete P.
The final sweep passed 46 relative links in 25 product Markdown files, all 16
unique nonempty packet fields and six ordered remaining Queue entries. Actual
source/test line change is +0/-0/net 0: only product documentation/metadata
changed. No Shared/package manifest update or product rebuild is applicable.
No runtime tests, RDP, physical audio, gameplay, new EXE or all-profile
correctness is claimed by this research batch.
The relevant manual files remain ignored research inputs for immediately
following source work; rendered operator-page images are disposable.
