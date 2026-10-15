# T560 S7 SMM And System Signal Investigation

## Baseline And Status

Runtime baseline: accepted S6 implementation `926bfe185` and governance
closure `3de2f4596`. CURRENT owns S7 admission. This record consumes C11/C12
from the [complete CPU ledger](t560-s1-cpu-convergence.md); it does not accept
those rows or open production 486 construction.

The initial investigation changed no production, test, public contract or
artifact. Its result was a primary-source save-field inventory, event matrix
and actual-owner audit. Unresolved items below must be decided explicitly
before the affected implementation, not guessed from a modern emulator.
The complete S7 delivery still requires actual CPU/Core proof, dual-width
repository units, specialized gates, eight integrations and eight PC EXEs.

## Sources And Applicability

Originals remain in the owner-managed external manual archive. Text and
renderings are ignored research inputs under `build/t560-s7-research/`;
they are not new firmware, source imports or release inputs.

| Source | Identity and inspected authority |
| --- | --- |
| Intel Embedded Intel486 Processor Family Developer's Manual | Order 273021-001, 1997; SHA-256 `BE6242FD0A0B0663C27AA578C6BB7A3994DB21F1EC31887C2EA440D011B82BD3`. Chapter 8, standard-bus reset/interrupt sections 9.2.9 and clock-control sections 9.3.4/9.6.4. |
| Intel486 SX Processor datasheet | Order 272769-004, 2004; SHA-256 `A35A0A428168ED45428DD886876238424A228B8DB0583EE74A1815A88BF421CD`. Table 7, printed pages 18--19, confirms SRESET, SMI/REP/LOCK, SMIACT and STPCLK for the selected SL-enhanced SX candidate. |

The selected SX is not the Write-Back Enhanced DX4. Sections 7.1.2 and
9.2.18.4 describe a distinct enhanced-bus SRESET/cache contract and must not
override the SX pin description. The candidate sources do not prove the exact
package/stepping fitted to the future PC110.

Complete save-map pages were visually inspected in the preceding admission
investigation: physical PDF pages 206--207, printed 8-8--8-9. Physical pages
208 and 210 were rendered and inspected in this continuation: the invalid-RSM
shutdown note and complete initial-register table are present, not OCR guesses.
The render command warns about missing Symbol/ArialUnicode display fonts;
the relevant ASCII register names, addresses, values and notes remain legible.
Other references below are text/source inspection, not claimed visual review
of every manual page or figure.

### Reference Models Are Not The Selected Hardware Contract

- Bochs 2.6 `cpu/smm.h` explicitly identifies its 32-bit layout as Intel P6.
  Its segment slots differ from the original 486 public table. It cannot
  supply the 486 layout merely because the build is 32-bit.
- Read-only 86Box source revision
  `4fef696a4eead1d55a28d6ac0e5bd2864e5454da`, `src/cpu/386_common.c`,
  SHA-256 `610FC8C7760A39DF32E5C054706A351FFF270AB0EC6E5CE1E6ADB6C0160272A7`,
  contains a 386-to-P5 layout, later P6/AMD layouts and AMD-specific saves.
  Its revision constant and AMD debug-register saves are not Intel SX proof.
  No code is imported, translated or used as an independent implementation.
- Robert R. Collins' [January 1997 investigation](https://www.rcollins.org/ddj/Jan97/Jan97.html)
  and [March 1997 measured Pentium investigation](https://www.rcollins.org/ddj/Mar97/Mar97.html)
  explain reserved descriptor and restart fields in a compatible older public
  map. They explicitly concern Pentium and implementation-dependent details.
  They are leads for a declared L2 model, not measured Intel486 SX results.
  In particular they cannot override the SX manual's NMI/IRET rule.
- The local PC110-QEMU repository contains integration/patch files rather
  than a complete QEMU CPU source tree. The first scoped search in the
  PC110-EMU `PC110Core` source did not provide an SMM implementation. Neither
  observation establishes absence across every external file or a usable
  reference implementation for the selected state image.

## Original Public State Image

The register-offset column below is relative to `SMBASE + 8000h`, not to
SMBASE alone. The 512-byte image occupies `SMBASE + FE00h..FFFFh`. Saving
proceeds down from the top of the area. Multi-byte values are little-endian.
The default SMBASE is 30000h and first handler instruction is at 38000h.

| Register offset | Saved field | Handler modification contract |
| --- | --- | --- |
| 7FFC | CR0, dword | Read-only; invalid RSM combinations have the separate shutdown rule. |
| 7FF8 | CR3, dword | Read-only. |
| 7FF4 | EFLAGS, dword | Writable. |
| 7FF0 | EIP, dword | Writable. |
| 7FEC | EDI, dword | Writable. |
| 7FE8 | ESI, dword | Writable. |
| 7FE4 | EBP, dword | Writable. |
| 7FE0 | ESP, dword | Writable. |
| 7FDC | EBX, dword | Writable. |
| 7FD8 | EDX, dword | Writable. |
| 7FD4 | ECX, dword | Writable. |
| 7FD0 | EAX, dword | Writable. |
| 7FCC | DR6, dword | Read-only. |
| 7FC8 | DR7, dword | Read-only. |
| 7FC4 | TR selector, low word | Read-only; upper two bytes reserved. |
| 7FC0 | LDTR selector, low word | Read-only; upper two bytes reserved. |
| 7FBC | GS selector, low word | Read-only; upper two bytes reserved. |
| 7FB8 | FS selector, low word | Read-only; upper two bytes reserved. |
| 7FB4 | DS selector, low word | Read-only; upper two bytes reserved. |
| 7FB0 | SS selector, low word | Read-only; upper two bytes reserved. |
| 7FAC | CS selector, low word | Read-only; upper two bytes reserved. |
| 7FA8 | ES selector, low word | Read-only; upper two bytes reserved. |
| 7FA7--7F98 | Reserved | Not a software ABI. |
| 7F94 | IDT base, dword | Read-only. |
| 7F93--7F8C | Reserved | Not a software ABI. |
| 7F88 | GDT base, dword | Read-only. |
| 7F87--7F04 | Reserved | Not a software ABI. |
| 7F02 | Auto-HALT restart, word | Bit 0 initially indicates interrupted HALT. |
| 7F00 | I/O restart, word | Initially 00h; 00FFh requests trapped-I/O restart. |
| 7EFC | SMM revision, dword | Read-only; extension bits 16 and 17 defined. |
| 7EF8 | SMBASE, dword | Writable; applies to the next SMI after RSM. |
| 7EF7--7E00 | Reserved | Not a software ABI. |

Read-only corruption is generally unpredictable, not automatically a guest
GP. Section 8.3.4 separately specifies shutdown for unaligned SMBASE, reserved
CR4 bits and invalid CR0 PG/PE or NW/CD combinations. Do not introduce broader
validation merely to make a test or corrupted image easier to recover.

### Hidden State And Unresolved Encoding

The original manual explicitly saves/restores CR1, CR2, CR4 and the six hidden
segment descriptors in reserved image space. It requires all reserved bytes
to survive power-down suspend/resume. A host-only `t_cpu` checkpoint would
violate that requirement, even if a trivial handler round trip passed.
Conversely, serializing the native C structure would introduce ABI padding,
host width and an invented firmware layout.

TR/LDTR descriptor state and GDTR/IDTR limits also have to survive a real
protected-mode return. Their encoding, CR1/CR2 positions, revision low word
and any transient interrupt/restart image still need an explicit source/model
disposition. The public table establishes their required effects, but does
not publish the complete reserved encoding. A later P6 layout is not a valid
substitute. A bounded older-map reference model, if selected, must identify
every modeled slot and keep its L2 grade separate from the public map.

DR0--5, TR3--7 and FPU registers are not automatically saved/restored according
to printed 8-9--8-10. Do not accidentally restore them from a blanket CPU copy.
Existing selected-SX DR4/5 alias behavior must be reconciled explicitly rather
than treating those names as independent new registers.

## Entry, Exit And Memory Ownership

The following are source-backed requirements, not claims of implementation:

- SMI is recognized at an instruction/REP-iteration boundary, never midway
  through a locked transfer. Previously accepted stores complete before
  SMIACT assertion. Assertion precedes the first save write; deassertion
  follows the last restore read.
- Actual memory cycles use the existing CPU physical bus and Core transaction
  owner. A board observes SMIACT to select SMRAM. Non-CPU masters do not
  automatically acquire the CPU's SMRAM mapping.
- Entry uses EFLAGS=2, EIP=8000h, CS base=SMBASE, other segment bases/selectors
  zero, 4-GiB limits and 16-bit default operand/address/stack sizes. CR0 clears
  PE, EM, TS and PG while preserving the other bits. The table names CS
  selector 3000h; relocation prose changes the base. The nondefault-selector
  value must not be inferred from a modern implementation without review.
- The initial DR7 table says zero, while the adjacent detailed feature list
  preserves bits 12 and 13. This is a real original-source ambiguity requiring
  explicit reconciliation, particularly because S6 implements GD. No entry
  mask has been guessed in production.
- SMM starts with IF/TF disabled and NMI blocked. Handler-enabled INTR/debug
  use its prepared vector environment; original 8.4.3.1 says IRET can unblock
  NMI. The Pentium-specific reference reports different observations and is
  not authority to change this SX contract.
- Nested SMI is not recursive: one pending request is retained. Pending SMI
  and NMI are eligible before the resumed application's next instruction.
  Existing pending-debug and interrupt-shadow state must not leak into the
  handler or be silently lost on return.
- An SMM transition does not imply a cache flush. Section 8.6.2 gives the
  system designer responsibility for overlay/cache coherence. The selected
  CPU still recognizes A20M in SMM; section 8.6.3 places any blocking of A20M
  at the board. Neither belongs in an unconditional CPU bypass.
- RSM outside SMM remains UD. RSM inside SMM reads the real save image, applies
  next-entry SMBASE only after using the current image, and reconciles the
  existing translation/prefetch owners. Guest changes to writable GPR/EIP/
  flags/control slots must survive; excluded register changes are not undone.
- Auto-HALT 1->1 requires a real HLT refetch/execution and its bus behavior;
  1->0 resumes after it. Setting restart after a non-HALT is unpredictable.
- I/O restart must retain the actual trapped opcode/prefix and string
  state boundary, not subtract a constant from EIP. Ordinary IN/OUT, INS/OUTS,
  REP iteration, zero count, failed port admission and back-to-back SMI need
  separate actual-path proof. Undefined non-I/O restart is not a recovery API.
- Invalid-state RSM shutdown has an explicit original note permitting INTR
  and SMI wake in addition to NMI/reset. Existing ordinary shutdown behavior
  cannot be reused unchanged or globally loosened for older processors.

## Finite System/Event Matrix

Each row requires both transition and no-transition proof where applicable.
All instruction-affecting rows include running, REP, memory LOCK, HLT/WAIT,
ordinary shutdown, invalid-RSM shutdown and SMM contexts as applicable.

| ID | Input/event | Required disposition and current receiver |
| --- | --- | --- |
| E01 | RESET | Highest-priority reset; default SMBASE, CPU/cache/TLB/reset-vector owner. Preserve existing whole-machine versus processor-reset distinction. |
| E02 | SRESET | Selected SX duplicates RESET except retained SMBASE; resets cache. No enhanced-DX4 preserve-cache rule. Board pulse qualification remains external. |
| E03 | FLUSH | Higher priority than SMI in ordinary-bus source order; real cache invalidation, including deferred low-power processing. Not an INVD external-flush output loop. |
| E04 | SMI outside SMM | Complete instruction/REP boundary, stores then SMIACT then physical state save then handler. No synthetic retired instruction for entry alone. |
| E05 | SMI inside SMM | Single pending latch; no recursive state save; service after RSM before interrupted code. |
| E06 | SMI with debug/NMI/INTR | SMI priority is above those events; preserve the displaced event's correct pending/return disposition. |
| E07 | NMI in SMM | Initially blocked, one pending request, IRET unblock and post-RSM delivery per selected source. Board NMI mask remains a distinct input. |
| E08 | INTR in SMM | IF starts clear; handler may enable after vector preparation. Existing PIC acknowledgement remains authoritative. |
| E09 | RSM valid | Read current physical image, restore and publish once; next SMI uses edited SMBASE. Retain handler changes to registers not autosaved. |
| E10 | RSM invalid | Source-specific shutdown and special-cycle indication, not GP or automatic cold reset; qualify wake inputs separately. |
| E11 | HALT restart | Real refetch/HLT or next instruction according to slot; pending interrupts checked at the proper boundary. |
| E12 | Trapped I/O restart | Correct prefix/string restart state and no accidental replay of an unaccepted or unrelated transfer. |
| E13 | STPCLK from running | Complete current work and stores; Stop Grant indication precedes stopped execution. HOLD/HLDA may delay grant. |
| E14 | STPCLK from HALT | Stop Grant, then return to HALT with its indication on STPCLK release, not to ordinary execution. |
| E15 | Interrupt during Stop Grant | SMI/NMI transitions are not recognized as normal running edges; a signal held through release can qualify after release. INTR remains level-sensitive. |
| E16 | Cache invalidation during low power | Existing snoop mechanism works without guest instruction retirement; deferred FLUSH serviced on the defined normal-state boundary. |
| E17 | CLK stopped/restarted | Board clock/PLL preconditions and Core timeline remain distinct from CPU execution enable. No host sleep, second clock or fabricated instruction. |
| E18 | Save/restore/native provider failure | Report through the existing Core fault path; preserve accepted external write prefix, no fictitious storage rollback or new retry thread. |
| E19 | Preview/diagnostic/reset observation | Never enter SMM, consume an edge, issue a special cycle or change image/cache/TLB from lexical observation. |
| E20 | Retained CPU families | Unsupported SMM input/encoding remains explicitly rejected/inapplicable without changing 8086/188/286/386 event rules. |

The ordinary-bus source order is RESET/SRESET, FLUSH, SMI, NMI, INTR, STPCLK
(developer manual 3.15.6); synchronous exceptions and instruction completion
still need their proper boundary, not a blanket asynchronous preemption rule.
Original 9.6.4 excludes ordinary interrupt recognition during Stop Grant and
specifies signal-held-through-release behavior. Replaying every edge queued
while stopped would therefore be wrong. Pin pulse lengths, synchronizer/PLL
time and the semantic sampling boundary must be graded separately in S9;
that receiver does not permit successful no-op Stop Grant in S7.

## Current Code Owners And Required Work

| Current owner | Inspection and required change |
| --- | --- |
| `core/chips/cpu/cpu_instructions.c` | RSM at the 0F-AA dispatcher is still an explicit UD TODO. Existing `ExecInt` performs debug then NMI/INTR; refresh handles ordinary shutdown separately. Add SMM to these boundaries, not a second executor. |
| `core/chips/cpu/cpu_instructions.h` | Execution context owns resident shutdown, NMI, interrupt shadows, WAIT, pending debug, cache/TLB and preview. New SMM/low-power state belongs here, with reset and all state writers audited. No board mirror of this state. |
| `core/chips/cpu/cpu.c` | Creation, reset, public signals and debug mutation share the existing opaque CPU. Reset currently clears architectural storage and calls execution reset; distinguish qualified SRESET while retaining existing-family reset. |
| `core/chips/cpu/cpu_interface.h` | Existing memory/port/IRQ/cache/LOCK provider has no SMIACT or system-special-cycle output and no SMI/stop-clock input contract. Add only required bounded serialized signals, not raw CPU storage or a generic device framework. |
| `core/x86/cpu_bus.c` | Physical access, A20/cache lookup, special cache cycles and LOCK already use one Core transaction owner. SMRAM selection must reach real memory routes with signal ordering, without mutating inspect-only paths. |
| `core/x86/attachment_interface.h` | Optional board callbacks already run on the serialized owner. A required SMIACT/special-cycle binding needs an explicit failure/order contract; null must not silently deliver a missing required physical effect. |
| `core/x86/machine.c` | Existing runner consumes reset before work and recognizes CPU shutdown/HLT/WAIT outcomes. Extend this same waiting/entry boundary so SMM entry is not counted as an opcode or exposed as a product Stop. |
| `core/x86/machine_scheduler.c` | HOLD/DMA/deadline and device advancement remain the sole timeline. Stop Grant cannot suppress external peripherals or invent a host-side clock. |
| Existing Core CPU/Executor tests | Use real handler bytes and physical bus observation; reserve private CPU-model binding for legitimate internal owner tests. Public FPU fixture must not again acquire raw executor state. |

### Additional Direct-Path Findings

- `_ksa_load_real_sreg` currently resets limit to FFFFh and replaces access/
  default-size attributes. Its three real-mode segment call sites must retain
  the SMM limit/attribute rule from original 8.4.3 while preserving all current
  non-SMM behavior. An entry/RSM-only test would miss this live handler defect.
- Lexical scanning and instruction metadata live inside `cpu_instructions.c`,
  not a `cpu_lexeme.c` or `cpu_metadata.c` file. RSM profile, LOCK, prefixes,
  unavailable fetch and non-mutating preview belong to those existing owners.
- `_debug_complete_instruction` clears RF after ordinary instructions and
  schedules post-instruction traps. RSM must not accidentally consume restored
  RF or reapply the handler's outgoing TF to resumed state. A pending debug
  event displaced by SMI must follow an explicit save/return disposition.
- CPU reset requests currently collapse to a boolean and Core consumes that
  boolean before `core_machine_processor_reset`. The latter deliberately
  preserves RAM, board state and scheduled work. SRESET needs a bounded reset
  cause at that same boundary; it is not a second whole-machine reset route.
- NMI currently accepts a qualified event rather than a raw voltage level.
  Stop-Grant held-through-release behavior cannot be proved by blindly adding
  another pending bit to that existing request call. Define which layer owns
  signal sampling before changing its admitted-edge contract.

There is no current `machine_execute.c`, `cpu_memory.c` or separate control
service to extend. File lists must be taken from the real tree. New files are
not justified merely by the size of the matrix.

## Next Implementation Review

Before production edits, finish the reserved-state/revision and DR7/relocated
selector dispositions, then freeze the exact bounded signal/output API and
refined source/test diff. The initial S1/S7 estimate is not treated as a budget
that can omit required behavior. Do not use a private snapshot or all-zero
reserved area to force the implementation into that estimate.

Independent remaining review can proceed on C12 standard-bus reset, signal
sampling and Stop Grant while the unpublished image slots are reconciled.
No required CPU capability has been declared inapplicable, deferred outside
T560 or implemented by a successful no-op. No dynamic verification was run
for that initial source-only checkpoint; the implementation checkpoint below
records the subsequent code and focused validation separately.

### Independent C12 Reset Implementation Review

Coordinator review separates the unambiguous E01/E02 reset mechanism from
the unresolved SMM entry image. Implementation may proceed on this bounded
part without claiming that C11's image is frozen: both the selected SX
datasheet and standard-bus developer manual require SRESET to retain SMBASE
while otherwise applying RESET, including internal cache invalidation.
The enhanced write-back DX4 exception is explicitly inapplicable.

The existing CPU reset implementation remains the sole register/cache/TLB
reset owner. Add a typed NONE/HARD/SOFT pending cause in place of the boolean,
one CPU-owned SMBASE value, and a bounded selected-SX soft-reset request at
CPU/Core signal boundaries. Hard reset dominates an already queued soft
reset. Requests do not execute reset or alter board/RAM state immediately.
The existing Core processor-reset branch consumes the cause; whole-machine
cold reset and 8042 hard-reset wiring keep their original semantics.
No App model switch, raw CPU pointer or second reset route is admitted.

Expected files: CPU interface/private execution header/reset implementation,
Core machine interface/implementation, and existing CPU lifecycle/Core Executor
tests. Estimate: production +65--100/-10--20; tests +140--220/-0--15.
No new test executable/framework is necessary. Prove actual register reset,
cache/TLB/prefetch invalidation, source-repeat/debug/wait/shutdown cleanup,
SMBASE retention versus hard-reset default, request precedence/consumption,
retained-family rejection and real Core consumption with RAM/board work
preserved. This is internal implementation progress, not a partial S7 P or
subtask closure; all remaining E/C11/C12 obligations still apply.

### C12 Reset Implementation Checkpoint

The reviewed reset portion is now implemented at the existing owners:

- `cpu.c` applies both resets through one local reset body. The public HARD
  entry and selected-SX SOFT entry have real distinct contracts, not duplicate
  implementations. CPU creation/initialization and HARD establish SMBASE
  30000h; SOFT retains its prior value. CPU remains its only owner.
- The existing pending-reset field now has a typed NONE/HARD/SOFT cause. A
  SOFT request does not execute reset; HARD dominates either arrival order.
  Consumption returns the actual cause once, not a boolean that loses SOFT.
  Earlier profiles reject SOFT without mutating CPU state.
- Core's existing processor-reset helper consumes that cause, resets only
  CPU/incomplete-retirement bookkeeping and retains RAM, board work and the
  timeline. Existing whole-machine cold reset and hard-reset wiring remain.
- Existing CPU lifecycle tests prove register reset, SMBASE distinction,
  complete cache/TLB arrays and buffers, prefetch/repeat/debug/interrupt/WAIT/
  shutdown cleanup, request ordering and older-profile rejection. Existing
  internal Core Executor tests prove actual request consumption, unchanged
  time/RAM and delivery of a preexisting scheduled event after the resets.
  No new test executable or public raw state API was introduced.

Focused terminal results in the original Release Ninja/ccache graphs:

| Host | Build/test evidence | Result |
| --- | --- | --- |
| x64 | `build/t560-s7-x64-reset-build-final.log`, `build/t560-s7-x64-reset-test.log` | Both affected targets built; 2/2 passed, 1.53 s CTest total. |
| x86 | `build/t560-s7-x86-reset-build-final.log`, `build/t560-s7-x86-reset-test.log` | Both affected targets built; 2/2 passed, 0.62 s CTest total. |
| x64 specialized aggregate | `build/t560-s7-x64-reset-gates.log` | Terminal exit 0. This target also rebuilds dependent targets and the selected PC artifact; it is not a static-only command. Expected negative self-test diagnostics are followed by their passing self-test results. |

The first two builds failed because the added CPU test referred to a macro
private to `cpu_instructions.c`. The test now seeds the literal BS cause
without extending an interface; both were rebuilt successfully. Initial logs
remain `build/t560-s7-{x64,x86}-reset-build.log`. An initial test-manifest
command incorrectly looked for a verifier below `test/core`; rerunning the
actual source-owned verifier with `CORE_ROOT=test/core` passes. Neither is
reported as a runtime or manifest-semantic defect.

Actual reset-only code diff, excluding manifests/docs/artifacts: production
+94/-22 (net +72), tests +165/-2 (net +163), across six existing source/header
and two existing test files. The necessary additions are one reset-cause enum,
one CPU SMBASE value, the real SOFT reset/request boundaries and one shared
reset body; there is no second queue, board CPU mirror or recovery path.

The aggregate rebuilt/deployed the selected default x64 0560 EXE, so that
worktree artifact is now an intermediate S7 build, not the accepted S6 binary.
It is not a complete release or published P. All eight final PC EXEs and full
dual-width regression/integration remain required before S7 delivery. No
Shared-six, MyNES, INI, ROM master, media or snapshot input changed.

The same-class sweep found two DMA/HOLD conditions in
`core/x86/machine_scheduler.c` that still enumerate only 286/386. They require
selected-SX qualification in the remaining C12 work; the production admission
gate stays closed, and the internal reset test deliberately does not pretend
to qualify SX HOLD by constructing a public SX machine. This remains an open
S7 member, not a deferral outside T560.

## SMM Implementation Model Review

One-session coordinator review selects the following explicit functional L2
model under the owner's standing authority for Core accuracy upgrades. This
upgrades an unimplemented mechanism; it does not assert an exact fitted-SX
reserved encoding or silently substitute a later processor's documented map.
Public slots, 512-byte capacity, physical address arithmetic and defined
architectural effects remain governed by the original chapter 8 contract.

- Hidden descriptor triples follow the older map's descending TR, IDTR,
  GDTR, LDTR, GS, FS, DS, SS, CS, ES ordering, from attribute offset 7FA4h
  through 7F38h. Each has base at attribute-4 and limit at attribute-8; this
  preserves the original public IDTR/GDTR base locations. The model's compact
  attributes encode valid in bit 7, DPL in bits 6:5, normal descriptor type
  bits 3:0 and default-size in bit 14. No native structure/padding is serialized.
- CR4 uses the older reference's 7F28h slot. CR2/CR1 use model-only reserved
  slots 7EF4h/7EF0h. Pending debug cause and interrupt shadow/pending flag use
  7EE8h/7EECh. These extra positions are internal suspend-image encoding,
  explicitly not a firmware-supported hardware ABI.
- I/O restart EIP/ESI/ECX/EDI use older-reference slots 7F10h/7F0Ch/7F08h/
  7F04h. The model captures the actual last completed I/O iteration's incoming
  state, including prefixes, not an arbitrary EIP decrement or a host replay
  queue. No defined result is promised for restart requested at a non-I/O
  boundary or a second consecutive SMI; those are original undefined inputs.
- Revision advertises only original IO-restart and relocation bits, 00030000h.
  The unpublished low revision word is modeled as zero, not fitted-stepping
  evidence. All reserved bytes have one deterministic physical encoding so
  copying/restoring the whole image works without a host shadow checkpoint.
- DR7 entry follows the original detailed feature list's preserve-12/13 mask;
  DR6 is modeled as zero where the original table says unpredictable. The
  contradictory short DR7 table is retained as an applicability limit, not
  silently claimed resolved by a modern implementation. Relocated entry keeps
  the original table's CS selector 3000h with the actual relocated CS base.
  Unlisted control registers remain unchanged until handler writes them.
- RSM restores writable public state and saved hidden controls, but does not
  restore excluded debug/test/FPU registers wholesale. It preserves restored
  RF and displaced pending debug state. NMI blocking ends on RSM; the external
  board mask remains separate. Invalid-state shutdown preserves the handler
  CPU checkpoint as the model's unspecified register result and has its own
  source-defined wake class; it is not a GP or automatic reset.

SMI input is a serialized qualified level; inactive-to-active latches at most
one request. Pulse/synchronizer timing remains explicit S9 work. The CPU's
SMIACT output is a non-failing physical-level notification, before the first
save and after the last restore. Core forwards it to the existing optional
board attachment. The output itself does not rewrite global memory routes,
mask A20 or invalidate cache; a future real board owns overlay protection.
Current direct Core proof uses non-overlay physical RAM, not a pretend PC110
chipset. Other masters retain their existing routes.

Affected owners: CPU interface/context/reset/instructions; Core bus/attachment/
signal interfaces and machine runner; existing Core tests/registration and a
cohesive dedicated CPU SMM test where needed. Increment estimate, excluding
the preceding reset part: source +450--700/-15--45, tests +650--1000/-0--30.
Only Core contracts change. Tests must exercise real handler instructions and
the physical save image, including non-mutating lexical observation; source
grades and undefined-input limits cannot be replaced by a passing fixture.

## Live SMM Implementation Checkpoint

The selected model is now implemented, not merely a proposed state table:

- SMI is admitted through one qualified-level CPU signal and forwarded by
  the opaque Core signal API. An unconnected CPU SMIACT output or an earlier
  CPU rejects it explicitly. One asserted level does not create repeated
  requests; a new edge during SMM occupies only the one pending latch.
- The existing dispatcher now recognizes 0F AA for the selected SX. Its RSM
  handler reads the actual physical 512-byte image, validates the original
  specified invalid-state conditions, restores saved fields and returns.
  There is no persistent host CPU checkpoint used for return.
- Save writes occur top-down with SMIACT active before their first transfer.
  The board callback receives SMIACT deassertion only after the final restore
  read. Failed reads/writes use existing CE/Core fault handling and retain
  externally accepted write prefixes; the CPU does not invent storage rollback.
- Entry/return use existing physical-memory/cache/TLB/prefetch owners. They
  invalidate translation/prefetch but do not invalidate the data cache or
  override A20. Handler segment loads preserve SMM's limit/attributes while
  real/protected/VM86 behavior outside SMM retains its existing paths.
- Successful port completion records the incoming I/O iteration state. Failed
  port admission does not create restart state. HALT restart re-executes the
  real opcode rather than simply setting a host-side halted bit.
- RSM bypasses ordinary outgoing instruction RF/TF completion, preserving the
  restored flags and displaced pending debug event. Original NMI blocking,
  handler IRET unblocking and post-RSM interrupt delivery use existing owners.
- Invalid-RSM shutdown adds only its necessary resident shutdown class; INTR,
  NMI and SMI wake it through actual delivery/entry. Ordinary shutdown keeps
  its prior wake rule. Common boundary bookkeeping was separated from opcode
  prefetch at `ExecBoundary`, so asynchronous entry does not depend on fetching
  the interrupted application's next byte.
- Existing CPU finalization now quiesces SMIACT. Its existing operation was
  promoted to the public CPU contract because Core must invoke it before the
  borrowed board context is finalized. Isolated CPU destruction invokes the
  same idempotent operation. This is a real lifetime boundary, not a second
  destroy manager or a new raw-state interface.

### Direct CPU And Core Proof

The dedicated CPU-owned `cpu_smm_smoke.c` uses real instruction execution,
physical byte storage and existing diagnostic contracts. Its finite groups
currently prove:

- Real mode, protected CPL0--3 and VM86 round trips, including all saved GPRs,
  selectors/hidden caches, controls, descriptor-table state and flags.
- A guest handler reloads DS without losing the 4-GiB limit, edits saved EAX
  through a real memory instruction, modifies excluded DR0 and returns with
  the original RF intact. This is not only a standalone image codec test.
- SMBASE relocation and exactly one coalesced nested SMI are taken before an
  interrupted application instruction. The new physical image/handler base is
  used, not the old image with a changed label.
- All 128 save-write and 128 restore-read failure positions are injected.
  Save failure accepts one byte of the failing word, then proves that byte and
  earlier accepted words survive, later bytes do not change, and architectural
  preparation does not publish a candidate. Reset/finalize releases SMIACT.
- Auto-HALT keep/clear behavior uses the actual HLT instruction. Scalar byte/
  dword and REP string I/O restart return to the incoming iteration state;
  failed port transfer produces no successful-I/O restart or synthetic SMI.
- Unaligned SMBASE, unsupported CR4 bits and both prohibited CR0 combinations
  produce shutdown, not GP. Each is recovered separately by real NMI, INTR
  acknowledgement and a new SMI; no-signal refresh does not execute code.
- Lexical preview, with pending SMI and inside SMM, leaves CPU/execution state
  and the save image untouched. RSM outside SMM delivers UD; missing output
  and earlier-family signal admission do not silently succeed.
- SMI displaces pending debug/NMI/INTR; each is delivered after return. SMI
  entry/return also succeeds when the interrupted next fetch is unmapped.
- A real SMM INT/IRET sequence unmasks a pending NMI, whose own IRET returns
  to the SMM handler. A resident cache line survives entry/return while stale
  translation state does not.
- Explicit LOCK XADD and implicit memory XCHG finish their bus lock, memory
  write and register result before SMIACT/state save. Signal admission alone
  does not execute the handler recursively from a memory callback.
- A saved physical image survives RESET and a second SMI that overwrites the
  original live CPU/decoder and RAM image. Restoring those bytes and executing
  RSM recovers the original saved state; excluded DR0/test state remains reset.

The existing internal Core Executor test additionally constructs its real
Core memory/attachment/run path, executes the handler's saved-EAX write and
RSM, and verifies no synthetic entry retirement. Destruction while inside SMM
deasserts the physical output before the board finalizer. The test uses the
same legitimate private CPU qualification boundary as S6; the public
production constructor still rejects unqualified SX.

The non-overlay Core test has no configured physical SMM-duration model;
its zero entry retirement/tick assertion is not a silicon latency claim.
Nominal microcode clocks, pin sampling and full physical timing remain C16/S9.
Future board SMRAM overlay protection is not simulated by a global route
change in this CPU implementation.

### Verification At This Checkpoint

All work uses the original two Release Ninja/ccache graphs and one persistent
cache. No build directory or toolchain was replaced. The full CPU-owned
selection was derived transiently from current CTest registration: 98 tests,
92 executable targets and six existing static/decoder checks.

| Host/scope | Terminal evidence | Result |
| --- | --- | --- |
| x64 full CPU-owned group | `build/t560-s7-x64-cpu-owner-{build,test}.log` | 98/98 pass; 12.48 s runtime. |
| x86 full CPU-owned group | `build/t560-s7-x86-cpu-owner-{build,test}.log` | 98/98 pass; 6.42 s runtime. |
| x64 actual Core SMM connection | `build/t560-s7-x64-core-smm-{build,test}.log` | 1/1 pass, 1.33 s. |
| x86 actual Core SMM connection | `build/t560-s7-x86-core-smm-{build,test}.log` | 1/1 pass, 0.23 s. |
| x64/x86 subsequently extended SMM cases | `build/t560-s7-{x64,x86}-smm-{nmi,lock,image}-{build,test}.log` | Each updated target built and case passed on both widths. Other production/tests did not change after the 98-case snapshot. Latest physical-image case: 1.23 s / 0.12 s. |

The preceding seven-test CPU/Core boundary cohort also passed on both widths
(`build/t560-s7-{x64,x86}-smm-boundary-{build,test}.log`). It is not a claim
that the complete repository unit/integration suite was rerun for this partial
S7 snapshot. Source/test manifests, Core dependency gate, actual lifecycle
ownership checker and whitespace checks pass; full specialized aggregates and
whole-repository qualification remain final S7 duties.

Current code diff against accepted S6, excluding manifests/docs/artifacts:
production +503/-75 (net +428), across nine existing files; tests +950/-2
(net +948), including the 681-line new CPU SMM case and three existing files.
The new helper responsibilities are physical image encoding, segment state,
SMM entry and common boundary initialization; no handler/framework split or
parallel dispatch was introduced. The one CPU SMBASE and SMI/restart fields
have actual consumers; there is no second board-owned CPU snapshot.

### Remaining S7 Work, Not Accepted By These Results

E13--E17 stop-clock/grant/clock/snoop/FLUSH and physical special-cycle outputs
still require implementation and direct Core proof. E01/E02 need final SMM/
power interactions. The following checkpoint completes handler-enabled INTR,
actual Core fault-route injection and both selected-SX DMA/HOLD conditions.
Exact reserved-slot/sampling/silicon timing limits remain explicit;
passing model tests does not upgrade their source grade. Final full dual-width
units, gates, eight applicable integrations/artifacts and complete-P actual
change review are not yet performed. No S7 implementation P is committed or
pushed, and T560/S7 remain open. Shared-six, MyNES and owner inputs remain
unchanged; only the earlier default x64 reset-checkpoint EXE has been rebuilt
in the worktree and is not this latest SMM delivery.

## Core Arbitration And Fault-Route Checkpoint

The existing scheduler had two profile predicates limited to 80286/80386:
ordinary DMA advancement and the one-quantum grant used by waited DMA cycles.
Both now include 80486SX. No transaction owner, state field, callback or public
interface was added. The actual constructor still rejects production SX;
the internal qualification fixture explicitly binds both CPU and scheduler to
SX only after constructing the admitted 386 test machine.

The existing Executor smoke now exercises 286/386/SX, ordinary and waited DMA,
RDY suppression, an occupied CPU LOCK transaction and subsequent DMA resumption.
The callback verifies an actual DMA HOLD acknowledgement, not just byte counts;
legacy phase advancement during LOCK is allowed but cannot grant service or
replace the CPU transaction. Grant release and teardown leave no HOLD owner.
Temporarily reverting the two production predicates with the completed fixture
causes an exact SX failure: `profile=6 wait=0 stage=grant granted=0 bad=1`.
The negative log is `build/t560-s7-x64-dma-hold-before-test.log`. This is not
the initial fixture error described below.

SMM CPU proof adds a real `STI; NOP; RSM` handler and a guest `IRET` interrupt
handler. Pending INTR is not acknowledged at SMM entry or immediately after
STI; it is delivered after the next instruction, returns to SMM and then RSM
restores the original IF and stack. There is no host-injected register change
standing in for STI/IRET execution.

The actual Core SMM fault test injects existing `memory_cache_control` I/O
errors on save writes or restore reads at cycle 2 and cycle 127. It verifies:

- the existing Core fault result, zero fake retired instructions/ticks and
  the existing CE diagnostic with the failing physical address;
- an exactly cancelled failed transaction and no remaining transaction owner;
- the before/after public register/segment snapshots are unchanged, rather
  than publishing a partially collected RSM candidate;
- a completed save write remains in RAM while the rejected write retains
  its original bytes; no invented rollback clears accepted effects;
- SMIACT stays asserted until existing destruction quiesces it before the
  attachment finalizer, exactly once. No recovery state or retry was added.

| Scope | Terminal logs | Result |
| --- | --- | --- |
| x64 arbitration/LOCK/scheduler cohort | `build/t560-s7-x64-dma-cohort-{build,test}.log` | 7/7; 7.72 s. |
| x86 same cohort | `build/t560-s7-x86-dma-cohort-{build,test}.log` | 7/7; 0.56 s. |
| x64/x86 extended SMM CPU case | `build/t560-s7-{x64,x86}-smm-intr-{build,test}.log` | 1/1 each; 1.33 s / 0.12 s. |
| x64/x86 final extended Core Executor case | `build/t560-s7-{x64,x86}-core-smm-fault-{build,test}.log` | 1/1 each; 1.63 s / 0.22 s. Includes earlier reset/SMM and new DMA cases. |

Initial verification mistakes are retained as investigation, not passes: a
guessed CTest regex selected zero tests; registration was queried and the
actual `unit.core-machine-executor-run-smoke` was then used. The first DMA
fixture omitted the mandatory attachment finalizer and failed during bind;
the fixture now supplies it and checks that HOLD is released at teardown.
The first Core fault assertion expected the injected lib_status as the CPU
exception code; the existing physical access owner actually records the
failing physical address. The corrected test checks that existing contract;
production error behavior was not changed. A guessed SMM build target was
rejected; the registered `core-test-cpu_smm` target was used subsequently.

Current cumulative code diff against S6, excluding manifests/docs/artifacts:
production +507/-77 (net +430), ten existing files; tests +1238/-2 (net +1236),
including the 712-line new SMM test and three existing files. This checkpoint
adds only the two production model predicates; its other changes are direct
proof in the existing two owner tests. Full S7 power/special-cycle/deferred
FLUSH implementation and final whole-repository qualification/artifacts remain
open. The intermediate default x64 EXE is not rebuilt or represented as this
latest source snapshot. No partial P is submitted.

## Power And Physical Special-Cycle Checkpoint

### Source And Boundary Review

Original Intel 273021-001 sections 9.6.4.2--9.6.4.6, Table 10-9 and section
10.3.11 distinguish a requested Stop Grant cycle from the stopped state:
external RDY/BRDY must complete that cycle first. Complete original figure
pages were inspected; the additional Table 10-9/10-32 render is
`build/t560-s7-research/special-cycle-321.png` (printed 10-43). The following
render is the separate BOFF restart discussion, not substituted authority for
this selected-SX stop-clock model. Existing state/recognition renders cover
printed 9-38/9-39. Poppler's Symbol/ArialUnicode font warnings did not obscure
the inspected table or signal labels.

Section 7.7 supplies an important additional rule: a sustained FLUSH input
repeatedly invalidates the cache and prevents further execution. A pulse is
therefore not equivalent to an indefinitely asserted level. The selected SX
does not acquire enhanced-write-back DX4 flush-ack cycles. The code and tests
now distinguish pulse, deferred low-power pulse and held FLUSH.

The CPU owns one pending physical-cycle kind, STPCLK/CLK/FLUSH/NMI input
levels, the Stop Grant resident bit and deferred FLUSH. These are distinct
inputs/outputs, not duplicated Core power state. New CPU setters and the
corresponding opaque Core input functions have real Core test consumers;
they do not run the guest or advance time. CLK=true is a qualified board
statement that restart/PLL settling is complete; the board must hold other
pins stable while CLK is stopped, and must not issue an EADS snoop then.
Exact synchronizer clocks, the original non-ULP PLL settling duration and
fitted-package timing remain C16/S9 and later board qualification, not a
made-up zero-cost physical timing claim. This is the source-qualified causal
functional model at the existing execution boundary.

### One Physical Cycle And One Time Owner

- HALT, shutdown and Stop Grant use one CPU bus poll/cancel pair. Missing
  required delivery produces the existing fault, not silent success. A false
  acknowledgement is normal outstanding RDY, not an I/O error or a retry loop.
- Core appends one transaction kind to its existing arbiter. HALT/shutdown
  distinguish byte enables B/Eh; Stop Grant additionally drives A4. Pending
  HOLD prevents initial admission. A native controller is optional, but its
  poll/complete callbacks must be supplied as a pair; poll exposes readiness
  and completion publishes once before bus release. The existing CPU RDY gate
  also applies. Reset, error and destruction cancel only an admitted cycle.
- No second retirement/timeline owner was added. A completed HLT instruction
  retains its one logical retirement; an outstanding indication cannot admit
  more instructions or interrupts. Later polls and Stop Grant waits retire
  nothing. This logical retirement boundary does not allocate exact physical
  special-cycle clock costs, which remain S9. Shutdown board reset handling
  cannot run ahead of its unacknowledged indication.
- Core returns its existing wait outcome. Board/peripheral time can advance
  through the existing scheduler while the CPU bus is waiting or stopped.
  No host sleep, thread, queue, recovery manager or duplicate executor exists.
- Releasing Stop Grant resamples held NMI/SMI/INTR before ordinary execution.
  Pulses wholly inside Stop Grant/Stop Clock are not replayed as running edges.
  A halted origin reissues HALT rather than executing the next opcode. CLK
  restart and cache snoop do not generate another Stop Grant indication.
- FLUSH invalidates the existing cache and prefetch owners, not tags/data or
  an external INVD loop. It stays deferred in HALT/Stop Grant until normal
  wake; while held in normal execution it blocks fetch and higher/lower
  interrupt work according to the original FLUSH priority.

### Direct Proof And Initial Corrections

The expanded CPU SMM test proves RDY wait versus grant, Stop Clock admission,
forbidden pin changes, pulse rejection/held wake, clock-running low-power
snoop, deferred and repeated held FLUSH, HALT return/reindication, lexical
preview without output/state mutation, earlier-family rejection, missing
backend fault, RESET/SRESET cancellation and reset inside active SMM.
LOCK XADD/implicit XCHG receive STPCLK during the actual store and finish all
memory/register/LOCK effects before the next grant. Real guest zero-count
REP INS/OUTS issue no port operation or restart record; back-to-back SMI after
an I/O-restart RSM does not claim another I/O operation occurred.

The new Core-owned `x86/core_machine_system_power_smoke.c` exercises all
three indication cycles against five dispositions: acknowledgement, RESET,
SRESET, destruction and provider failure. It verifies native pin encoding,
one held transaction, both readiness gates, cancellation before finalization,
no fake retirement, stable register snapshots, existing HOLD postponement and
peripheral/time-axis progress. It uses legitimate internal SX qualification;
the production constructor is still closed.

The first 100-case owner run passed 98 and failed two custom VME/decode buses
which omitted the now-required shutdown indication backend. Those fixtures
were completed explicitly; no existing privilege/fetch assertion was relaxed.
The first held-FLUSH/missing-backend extension exposed a cancellation attempt
without a provided poll callback; the CPU now faults without pretending a
physical request was issued. A mislocated test-edit hunk was corrected by
inspection before compilation. An initial generated build-target selection
accidentally included CMake's own executable; filtering the actual 92 Core
executables corrected the command without changing CTest registrations.

Final owner cohort logs are `build/t560-s7-{x64,x86}-power-owner-{build,test}.log`:
100/100 pass on both widths, 12.53 s / 7.38 s. This includes all 98 CPU-owned
cases and the two actual Core Executor/system-power cases. Earlier targeted
five-case correction logs are retained separately. After this run only
interface comments were clarified, not production/test behavior.

Cumulative diff against S6, excluding manifests/docs/artifacts, is production
+811/-90 (net +721), twelve existing files; tests +1795/-8 (net +1787), two
new owner cases and seven existing files. The three system helpers own cache
invalidation eligibility, a real held-cycle handshake and the existing refresh
boundary. No pure forwarding helper or second state manager was introduced.
New signal APIs are bounded Core contracts; public-six/MyNES/INI/ROM/media/
snapshot inputs are unchanged. Final whole suites, specialized gates, all
eight integrations/releases and complete-P actual-change review remain open.

### First Whole-Repository Qualification And Coverage Reconciliation

Both initial whole jobs built every repository unit target in the original
graphs. x86 passes 602/604 in 69.99 s. The two failures are duplicate
registrations of the same protected-UD source: its 0F metadata matrix still
classifies RSM/0F-AA as universally reserved. The matrix now names SX as the
minimum, and actual protected 386 RSM is added to the UD execution forms;
no earlier-family rejection or existing frame assertion is removed. Failure
diagnostics now identify a metadata mismatch rather than exiting silently.

x64 passes 601/604 in 262.27 s: the same two outdated expectations, plus
`core.test-manifest` after this executor edited the test while that whole
job was still running. This third failure is an execution sequencing error,
not a production or manifest-format defect. It is not counted as passed;
the next whole runs use the fully generated, stable manifests and no live
source/test edits. Neither job hit its 300-second aggregate deadline.

The final coverage review also adds real REP INS/OUTS receiving STPCLK at
port completion: exactly one iteration commits before grant; after release
only the remaining iteration runs, with precise ECX/ESI/EDI and no duplicate
I/O. Cancellation before a shutdown cycle is ever admitted is a no-op, and
unmasking a held qualified NMI follows the existing pending owner. These are
test-only extensions after the 100-case cohort. The corrected UD registrations
and extended SMM case pass 3/3 on x86 in 0.27 s, logged as
`build/t560-s7-x86-final-coverage-{build,test}.log`.

Current final-coverage code counts are source +811/-90 (net +721) and tests
+1853/-12 (net +1841), excluding manifests/docs/artifacts. The expanded test
count exceeds the earlier estimate by the explicit last REP/cancellation and
UD classifications above; no required proof is dropped to fit an estimate.
The next whole qualification and all release/integration duties remain open.

## Stable Whole-Repository Qualification

No production or test file changed during the final sequential whole jobs.
Both original Ninja/ccache graphs complete `run-unit-tests` successfully:

| Width | Complete units | Runtime | Log |
| --- | --- | --- | --- |
| x64 | 604/604 | 42.50 s | `build/t560-s7-x64-whole-unit-final.log` |
| x86 | 604/604 | 47.89 s | `build/t560-s7-x86-whole-unit-final.log` |

These are the entire repository unit labels, including Shared-six, all Core
branches and every App, not just CPU tests. Three automated desktop-labelled
unit cases are included per width; this does not claim manual/RDP interaction.
The first jobs' failures remain recorded above, not relabelled as successes.

Both complete `verify-current-specialized-gates` aggregates subsequently exit
0 (`build/t560-s7-{x64,x86}-specialized-final.log`). The intentional negative
ownership self-test prints a CMake error for its injected duplicate and then
explicitly reports success; it is not an unreported aggregate failure.
The global Types verifier enumerates both tracked and untracked nonignored
sources and covers the two new owner tests. Its subsequent direct dual-width
reruns also pass, reporting 1,294 code/script files. Staging the three new S7
files is preparation for final review, not an incomplete implementation commit.

Core's native constructors still initialize the CPU only during construction;
live processor reset uses the common hard/soft reset owner, and teardown
quiesces CPU outputs before the attachment and memory contexts are destroyed.
Stop Grant uses the existing HALT flag to recover its origin, rather than a
duplicate saved power state. Level inputs, pending events, accepted cycles
and actual bus ownership have separate, necessary consumers. The similar-path
sweep finds no additional transaction-kind upper bound left at MEMORY_LOCK.

The existing eight-profile/width release job is now running sequentially in
the same graphs. Default NXVM x64 passes 35/35 (55.54 s), XT x64 15/15
(31.75 s), and AT x64 17/17 (44.64 s); remaining groups, final hashes/PE/INI
checks, complete-P review and closure are not yet claimed. These aggregates
also carry the existing MyNES integration registrations, without rebuilding
or modifying its excluded binaries. The Windows31 checkpoint is still DOS
C-drive/directory/BDA proof, not Windows 3.1 GUI execution.

## Complete Executor Delivery

The last actual-code review adds direct RSM 66/67 size-prefix combinations to
all six mode round trips (including paging enabled without present page
tables), LOCK RSM delivering UD without leaving SMM, failed suffix fetch
without physical state restore, and non-mutating preview at that boundary.
An actual SMM handler enables TF with PUSH/POPF, enters its debug vector,
executes IRET and RSM, and proves that outgoing handler TF does not leak into
the interrupted application. These are extensions of the existing CPU owner
case, not additional frameworks or duplicate Core instruction tests.
Original printed 8-9 explicitly includes hidden CR1/CR2/CR4 among automatically
saved/restored state; only their unpublished physical encodings are L2 here.
DR5:0 and TR7:3 remain excluded as the original specifies.

The final frozen-test jobs pass 604/604 units per width again:
`build/t560-s7-{x64,x86}-whole-closure.log`, 45.70 s / 49.56 s. Both complete
specialized aggregates also exit 0 at
`build/t560-s7-{x64,x86}-specialized-closure.log`. A subsequent fixture-only
cleanup finalizes old physical outputs before zeroing/reusing the fixture and
before freeing it. Its only affected CPU case plus the updated manifest are
rebuilt/retested on each width, 2/2 in 1.34 s / 0.52 s
(`build/t560-s7-{x64,x86}-fixture-final-{build,test}.log`). All other tests and
all production inputs remain byte-identical to the complete closure runs;
this final recheck is reported separately, not misrepresented as another
complete 604-case execution.

All eight existing integration/release groups exit 0 once against the frozen
production inputs; logs are
`build/t560-s7-<width>-<profile>-integration.log`:

| Profile | x64 | x86 |
| --- | --- | --- |
| default-pc-at-80386-1440k-hdd | 35/35, 55.54 s | 35/35, 67.77 s |
| ibm-5160-model-268-360k | 15/15, 31.75 s | 15/15, 40.58 s |
| ibm-5170-model-339-1200k | 17/17, 44.64 s | 17/17, 55.70 s |
| compaq-deskpro-386-model-40-1200k | 17/17, 134.22 s | 17/17, 155.28 s |

Each width executes 84 registered integration rows, including the existing
MyNES rows repeated by the profile aggregates; these are not 84 unique PC
cases. Original graph/cache directories are retained. Both graphs are restored
to default and the current default artifact rebuilt; the final real-product
INI/identity case passes 1/1 on each width in 0.59 s / 0.56 s
(`build/t560-s7-{x64,x86}-final-identity.log`). All eight artifacts pass the
existing PE architecture verifier, retain version 0.5.0560 and are stripped
Release builds. Owner INIs and excluded MyNES/public-six inputs remain
unchanged. No manual RDP/desktop gameplay or Windows 3.1 GUI qualification is
claimed; the automated desktop-labelled tests are identified above.

Final deployment identities (source/test revision is the implementation commit
containing this record):

| Artifact under assets/ | Bytes | SHA-256 |
| --- | --- | --- |
| my5160/my5160_0_5_0560_x64.exe | 1450072 | `27D5087A7DEC4987A26BF40E7968E960764AEE482F3FA2C9CA3AA1A906A24D70` |
| my5160/my5160_0_5_0560_x86.exe | 1591465 | `4004C17C2C98ACED8B63E9AF15AFF2815EB2ECE8C2594BA0DE117D568833EC33` |
| my5170/my5170_0_5_0560_x64.exe | 1462948 | `665C8FE569A17174EB39C32F835E274298D450D48924DDBD0CFD9908C6672229` |
| my5170/my5170_0_5_0560_x86.exe | 1605232 | `79B8AB1BC90B69B8371382275EF9724C635F03FA6338A9BC2C4905F642C3A00B` |
| mydeskpro386/mydeskpro386_0_5_0560_x64.exe | 1449676 | `9644E5F9721464119DA8F4C927E3C553E7EC78441C625D76498E4B5F8446F8D3` |
| mydeskpro386/mydeskpro386_0_5_0560_x86.exe | 1592479 | `DC31AEEADA6A294B020951DD7A8EE27839440235097EFCB591589976A506C2AF` |
| nxvm/nxvm_0_5_0560_x64.exe | 1462118 | `E5C050186DB8DBD6425215DE23E522DBE3D981374FEFA7EEC3BA684C7FB634AF` |
| nxvm/nxvm_0_5_0560_x86.exe | 1604530 | `8793B88F172B6D2B034384A6938C456EF31E5815A027ADD6CDD1CEEA09822B28` |

### Exit Matrix And Self-Review

| S7 event rows | Direct proof / actual owner |
| --- | --- |
| E01--E02 | CPU lifecycle reset, SMM power-cancel/suspend-image and Core Executor/power RESET/SRESET preserve their respective CPU/board/time ownership. |
| E03 | Deferred and held FLUSH/cache/prefetch proof; no external INVD loop. |
| E04--E05 | 24 physical mode/prefix round trips, guest image writes, relocation, one nested pending SMI and actual Core handler/memory connection. |
| E06--E08 | Debug/NMI/INTR priority, dummy INT/IRET NMI unblocking, real STI shadow and handler-enabled single-step, followed by RSM. |
| E09--E10 | Physical restore and complete image copying, read-only exclusions, three documented invalid-state classes, invalid-RSM wake and the actual Core shutdown indication contract. |
| E11--E12 | Refetched HALT, scalar/prefixed/string I/O restart, zero-count REP, failed transfer, nested-SMI no phantom I/O. Undefined guest requests are not promised hardware behavior. |
| E13--E14 | Core RDY/held-cycle completion, existing HOLD postponement, real memory LOCK and REP-iteration boundaries, HALT reindication. |
| E15--E17 | Pulsed/held qualified inputs, clock-running snoop, Stop Clock admission and qualified restart, no extra grant indication; live Core peripheral time while CPU waits. Exact CLK/PLL durations remain S9/board qualification. |
| E18 | All 128 save/read failure positions and accepted write prefixes; real Core early/late provider faults and cycle reset/failure/destruction, without rollback. |
| E19--E20 | RSM/Stop Grant preview leaves CPU and physical outputs unchanged; earlier profiles reject SX-only inputs and retain whole-unit/eight-release regression. |

Final code counts against accepted S6, excluding manifests/docs/artifacts:
production +811/-90 (net +721), twelve existing files; tests +1934/-12
(net +1922), two new owner cases and eight existing files. The final test
growth beyond the estimate is the explicit RSM form/paging/debug/lifecycle
proof above; no production rewrite accompanies it. The manifest revisions
identify this private Core batch, not a public-six corpus import.

The two bounded enums name real reset causes and physical-cycle kinds; the
CPU fields are SMBASE, physical input/output/event state and genuine I/O
restart data. They have real consumers. Existing cache/TLB/interrupt/transaction
owners are reused. Opaque CPU/Core/attachment adapters are necessary boundary
translation, not same-owner synonym wrappers. No raw CPU getter, native host
API, new heap owner, manager, thread, queue, parallel executor or timeline was
introduced. Exact reserved encoding/ambiguous DR7/relocated-selector limits
remain explicitly L2; construction-only initialization is not advertised as
a second runtime reset path.

Executor delivery is complete and ready for its complete implementation P and
one-session coordinator actual-change review. C11/C12 acceptance is bounded
to the source-qualified functional contract above; C01 fitted-silicon limits,
C15/S8, C16/S9 and C17/S10 remain mandatory in T560. Production SX admission
stays closed. There is no assertion of a complete 486 CPU, full Windows GUI
compatibility or manual/RDP qualification.

## Coordinator Actual-Change Acceptance

On 2026-10-15 the same session switched from executor to coordinator and
reviewed pushed P1 0059bfd5117baf503f734252935d2eca20eb44e5. This is an actual
diff review under the one-session rule, not an independent-person review.
It inspected the CPU reset cause and common reset body, complete physical
SMM image and candidate RSM publication, instruction/event ordering, power
input ownership, Core transaction/native RDY handling, cancellation before
attachment teardown, and the two existing scheduler DMA/HOLD predicates.
The CPU and real Core tests, registration, manifest changes, final logs,
deployment hashes and stripped sections were reconciled to E01--E20.

Accepted invariants: CPU owns architectural state and pin/event latches;
Core owns physical routing, one transaction and timeline; the board owns
actual SMRAM decode and qualified electrical timing. Every new field and
bounded callback has a real consumer. Failure preserves accepted physical
effects and uses existing exception/fault paths, without rollback machinery,
second executor or successful stub. Shared-six/MyNES/owner inputs are absent
from the actual diff. All twelve production files and ten test files fit
the admitted mechanism; test growth supplies explicit form and failure proof.
Eight affected PC EXEs are part of the complete P1, not deferred publication.

C11/C12 are accepted at the documented functional source/model grades, with
their complete E01--E20 dispositions. C01 fitted-silicon, C15 inherited corpus,
C16 exact timing and C17 production admission limits remain visible in the
convergence ledger and proposal. No unresolved member is hidden behind a
later boot checkpoint. S7 is accepted and closed; T560 stays open and the
next planned batch is S8. No production or binary changes accompany this
acceptance record, so P1's artifact identities remain current.
