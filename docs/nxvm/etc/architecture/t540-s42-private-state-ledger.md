# M5 T540 S42 Private Core/Board Ownership Ledger

This source-only intake is based on the post-S41 `machine.c`, `machine.h`,
`machine_board.c`, `machine_plan.c`, `machine_scheduler.c`, `cpu_bus.c`,
`board_advance.c` and `board_deadline.c`. It does not move code, change a
runtime contract or assert that the mixed header is ready for Shared. The
existing `core_machine` allocation, `core_machine_create`, cold reset, run,
and destroy remain the only production paths.

## S89 Physical IBM-PC Receiver

At S88 baseline the remaining App board corpus has 34 files. S89 receives
the complete media registry and display-provider slot (four source/header
files). Both own their existing opaque allocation, binding/freeze and release
mechanisms; neither needs a board-private layout, App policy or native file API.
Their sole receiver is flat x86/ibmpc-common, with all callers and standalone
tests. Actual-commit review accepts Shared P1 `4e23f14b6` and NXVM P2
`eed6b8e54`; this is a physical implementation receiver, not another Core seam.

PIC/PIT/DMA port/source adapters, FDC/HDC/video wiring and the shared board
construction/time/deadline owner remain the subsequent common-board batch.
AT KBC/RTC/Port-B and XT PPI/keyboard wiring require actual family contracts;
their current shared private board layout cannot be moved into peer directories
and advertised as independent components. D4 remains an explicitly retained
machine-specific electrical mechanism, not silently renamed generic AT.
Guest input/display adapter values remain classified by actual App consumers.
All these members remain open under T540; S89 accepts none by implication.

S89's contract-preserving source review also retains one existing unused
display mode-notification binding (two stored fields and the App callback).
The next common-board/provider caller batch removes that whole unused ABI
half; snapshot dispatch remains the only live path. This residual is not
accepted as a required shared capability merely because its files moved.

## S90 PIT Binding Receiver

The entire pit_bus.c/h class has one physical Shared receiver. Its existing
embedded device/base layout is not a public contract: delete it. Stateless
per-selector callbacks borrow the opaque chip directly, so no new binding
object or allocation is necessary. Board owns each primary/auxiliary chip and
its clock/OUT lifecycle; Shared owns atomic four-port publication. Core owns
the routes and existing serialized machine teardown. All constructor/failure/reset/deadline/test paths are in this
batch. S90 also removes the preceding unused display mode-binding half.
Actual-commit review accepts Shared P1 `ddc957eaf` and NXVM P2 `f33730fea`:
all old PIT binding paths are deleted and all primary/auxiliary callers use the
Shared atomic installation. The [S90 evidence](../evidence/t540-s90-pit-port-extraction.md)
records the complete independent/full-suite and product verification.
PIC/DMA aggregation, AT/XT wiring and D4 remain open; no narrow route proof
qualifies those distinct owners.

## S91 Whole PIC Receiver

Current source inventory at b44ac5dee measures fifteen production and
sixty-five test files referencing PIC endpoints, source leases or their
layout. The entire aggregation lifetime and port/cascade/IRQ operation class
is assigned to one Shared receiver, including FDC/HDC/KBC/RTC/PIT consumers,
reconnect/reset, initial allocation failure and diagnostic observation.
Endpoint and lease layouts remain private; no public chip getter is admitted.
DMA and the remaining board/family/D4 classes are not disposed by this move.

## Existing facts and intended sole owners

| State or behavior in the present private header | Present users and final owner | Receiver |
| --- | --- | --- |
| `lifecycle`, `stop_requested`, `fault_detail`, `elapsed_ticks`, `timeline`, `time_axis`, L1 policy, provider clock, execution provider and freeze state | `machine.c`, scheduler, public observation; neutral Core owns execution and guest time | S54 |
| CPU profile/quirk, FPU, opaque CPU execution context, maximum instruction time, retirement contract/qualification, CPU diagnostic and retirement observation | create/run/CPU bus and CPU observation; neutral Core owns the single CPU execution lifetime | S54 |
| Transaction state/contract, external-cycle page/overlap/pending fields, CPU retirement waits and source ticks, CPU/DMA ready/wait arbitration fields | `cpu_bus.c`, `machine_scheduler.c`, `machine.c`; neutral Core owns the single bus/retirement scheduler, with board signals delivered through callbacks | S54 |
| `executor_memory`, `executor_port`, immutable ROM mapping table/count, entry-plan marker, Core trace and firmware operation guard/context | memory/port/ROM/entry/trace/firmware implementations; neutral Core owns routes and operation bounds | S54 |
| `timing_plan` and `timing_plan_copied` | `machine_plan.c` assembles and publishes a frozen plan; Core consumes declarations/time rules. Keep one copied plan, not Core and board mirrors; split its board topology at application, not by copying the plan twice | S53 |
| Six named device clocks and KBC timing/configuration, keyboard/display topology, DMA wiring/bindings, RTC/CMOS and FDC/HDC topology/configured flags | board advance/deadline, plan, controller adapters; one board attachment owns device timing and topology | S44 |
| PIC pair and PIT0/RTC PIC IRQ source bindings | board constructor, PIC advance/deadline and finalization; one board attachment owns PIC lifetime | S45 |
| PIT pair and auxiliary-PIT configured state | board constructor, PIT advance/deadline and finalization; one board attachment owns PIT lifetime | S46 |
| DMA latch and primary/secondary controllers | board constructor, DMA effects and finalization; one board attachment owns DMA lifetime | S47 |
| RTC chip and selected register | board constructor, RTC I/O/deadline and finalization; one board attachment owns RTC lifetime | S48 |
| FDC and HDC controller instances | board constructor, controller adapters and finalization; separate bounded board batches | S49–S50 |
| KBC and XT PPI instances | board constructor, input adapters and finalization; one keyboard owner group | S51 |
| XT keyboard chip pointer left in flat Core after S51 | board constructor, input, advance/deadline, reset and finalization; no Core-only consumer | S53 |
| VADP instance | board constructor, display adapters and finalization | S52 |
| Planar parity configuration/port/NMI latches | board memory, port and NMI adapter | S54 |
| D4 platform configuration/port/failsafe/NMI latches | board port, reset and NMI adapter | S55 |
| D4 refresh pending/pulse/address latches and DMA refresh wiring | board refresh producer, neutral Core HOLD consumer | S56 |
| XT speaker gates and output | board PPI/PIT signal adapter | S57 |
| Absent-memory windows | board memory route adapter; Core owns only checked memory operation | S58 |
| Firmware provider/context binding and F0000h alias choice | `machine_board.c` owns board role/alias and binding; Core retains the single bounded firmware invocation and ROM table. Do not duplicate either side | S59 |
| `board_*_provider` callback slots and `board_owner` | Core scheduler/CPU bus call bounded board operations. The board installs 14 providers at one construction point; synchronous whole-machine destruction prevents any call after board release, so a redundant slot-clearing pass is not needed. The shutdown callback takes `const core_machine *` because test schedulers replace `board_owner` with a probe; do not silently change that ABI | S59–S64 |

These rows cover all contiguous state groups in the current `struct
core_machine`, including the appended scheduler state. `core_machine_plan`
likewise contains one frozen configuration, topology, declarations and
controller timing plus board D4/media/display/FDC inputs; the split must
retain one success-only publication and one topology rollback. The private
firmware context remains an operation guard, not a second machine owner.

## Mixed source functions and direct consumers

- `machine.c` lines 22–68 contain XT/KBC/PPI board callbacks. Lines 345–361
  validate board topology; lines 484–661 create/register/wire the board.
  S43 moves these implementations to the existing board owner and deletes
  their old definitions. No second constructor or registration transaction.
- `machine.c` lines 87–343, 362–483 and 663–747 resolve/validate/create
  the neutral CPU/time/memory/port stage and coordinate exactly one board
  create; lines 748–1350 own reset/run/deadline/lifecycle/fault/destruction.
  They remain Core candidates, but their board callback and mixed-field
  access must be resolved before physical relocation.
- `machine_plan.c` owns frozen board assembly; `board_advance.c` and
  `board_deadline.c` consume named device clocks/chips. `cpu_bus.c` and
  `machine_scheduler.c` consume board callbacks, not named chip fields.
- Eighteen production `.c` files in `src/app-nxvm/devices` and 114 test
  files under `test/app-nxvm` directly include this mixed private header.
  This count includes fixture headers and is a migration inventory, not a
  claim that all 114 tests should become Shared. Board fixtures and profile
  tests stay NXVM; only tests that compile against the neutral private
  contract alone can move with Core.

## Linear receiving plan

1. **S43** — Move remaining XT/KBC/PPI callbacks, board-config validation
   and board creation out of `machine.c`. Preserve one public create, one
   candidate allocation, port checkpoint, chip order and destruction route.
2. **S44** — Move device clocks, topology/configuration and timing fields to
   one board attachment. Repoint only their real readers/writers; preserve
   one frozen input plan and unchanged device phases. No duplicate fields.
3. **S45–S53** — Move board chip instances in actual owner groups: PIC and
   its IRQ source bindings (S45), PIT (S46), DMA (S47), RTC (S48), FDC
   (S49), HDC (S50), KBC/XT PPI (S51), VADP (S52), and the XT keyboard chip
   pointer overlooked in S51 (S53). Each group retains
   its existing constructor, reset, deadline, ports, IRQ wiring, teardown
   and tests; no group adds a second chip lifetime.
4. **S54–S59** — Move the remaining electrical groups separately: parity
   (S54), D4 platform/NMI (S55), D4 refresh/DMA wiring (S56), XT speaker
   (S57), absent-memory routes (S58), and board callback install/revoke plus
   firmware binding audit (S59). Preserve Core operation guards, the board's
   F0000h choice, one copied plan and one rollback.
5. **S60** — Measure the mixed private/public header and split its oversized
   neutralization. The [source intake](../evidence/t540-s60-neutral-header-intake.md)
   assigns distinct owner and compile boundaries.
6. **S61–S64** — Move D4 board memory state (S61), split frozen-plan board
   types without mirroring (S62), neutralize the private header (S63), then
   measure and allocate the oversized public interface (S64).
7. **S65–S69** — Split neutral construction input from board configuration,
   board public values and operations, the remaining direct Core-to-board calls,
   then prove an independent neutral compile. The [S64 intake](../evidence/t540-s64-public-interface-intake.md)
   owns this revised numeric receiver map.
   S65 moves the two construction calls to board composition with a temporary
   neutral executor value; S68 retains four reset/clock/NMI/finalization calls.
   S66 moves adjacent public board operation declarations with their types;
   S67 receives the remaining five neutral validators implemented in board
   `machine_plan.c`, rather than repeat an already completed declaration cut.
8. **Former prospective S70** — Move only proven neutral source/tests to `src/x86/core` and
   `test/x86/core`, delete App copies, reconnect NXVM, verify independent
   Shared build/tests, both widths and four fixed-profile boots. S71 onward
   owns separately audited IBM-PC common/AT/XT moves.

Post-S69 receiving correction: independent Core linkage is accepted, but
board-to-Core private access is not a public contract. S70 now receives all
board CPU NMI/reset signaling without borrowing the CPU context. Memory/port
construction and rollback, firmware publication, provider binding/board
attachment ownership and direct-test classification remain preconditions of
the physical move. Each receives a measured next numeric S; the former S70
and S71 physical/board-move numbers were prospective and are superseded.

S71 receives RAM aliases and planar parity publication through neutral
construction operations. D4 already uses the existing memory transaction.
Board port checkpoints and timer port writes remain the next bounded receiver,
not part of the neutral RAM state contract.

S72 receives only constructor checkpoint/status/rollback: typed Core batches
already propagate each failure, and the sole destructor owns the unpublished
candidate. The reset/configure refresh timer writes are a distinct next
receiver, followed by firmware/attachment boundaries and physical relocation.

S73 receives all three refresh-programming callers using the sole PIT chip
contract after source proof that construction scratch is unobservable and
cold reset leaves the same zero value. Guest port dispatch remains Core-owned;
firmware/attachment and direct-test classification remain measured receivers.

S74 receives firmware publication as one Core binding/rollback transaction
with board-selected reset alias policy and bounded ROM operations. General
attachment ownership and test classification still precede physical movement.

S75 receives all six input/display lifecycle consumers through the existing
copied Core lifecycle operation and moves both bus READY definitions to the
neutral scheduler. The remaining Running Port-B time read and three deadline
qualification reads are assigned to S76 intake before attachment and physical
movement; lifecycle observation does not justify relaxing time-read guards.
That intake also includes both plan timing-declaration publication sites:
the array write and the copied marker. The complete timing owner boundary,
not merely its first reader, must close before the physical move.

S76 intake separates that class by real ownership: the complete declaration
publication and deadline qualification receiver closes in S76; the one Running
Port-B observation requires the separate I/O-cycle input receiver S77. Neither
an extra board clock nor relaxing the paused-time getter is an eligible fix.
Both remain preconditions of attachment and physical movement.

S77 closes the complete typed port-read input boundary: CPU, bus/debug and
bounded firmware provide a copied Core guest tick; the two Port-B consumers
use it through their existing helper. Every current typed read callback is
reconnected in the same receiver. No clock mirror or running getter is added;
write callbacks have no current time consumer and keep their existing ABI.
P1 `bd0256b47` is accepted after actual pushed-diff review and full proof.
Attachment ownership and direct-test classification remain the next measured
receivers before physical Core/IBM-PC relocation; this row is not that move.

S78 measures the remaining attachment boundary: six production owners have
451 board association accesses, and nineteen callbacks still publish through
private Core slots. The [attachment intake](../evidence/t540-s78-attachment-owner-intake.md)
assigns the complete copied-binding cut to prospective S79, then the opaque
board-handle/test-classification cut before physical relocation. These are
real ownership contracts, not a private-pointer getter or completed move.

S79 replaces the full nineteen-slot callback class with one copied
`core_machine_attachment`. Core owns validation, publication and destruction;
board construction supplies the callbacks/context once. All phase, shutdown
and firmware callbacks now take the same explicit context. The scheduler
fixture forwards every phase with its correct production context instead of
mixing private owners. The earlier S59 callback row is historical: its
Core-typed shutdown exception and fourteen-slot publication no longer describe
the current implementation. The [S79 evidence](../evidence/t540-s79-copied-attachment-binding.md)
records complete-class proof. The six private board-access owners and direct
board tests remain assigned to the opaque-board/test-classification receiver;
physical Shared movement has not occurred.

S80 implements the entire nineteen-callback context receiver: the single board
allocation retains an opaque Core handle, and the binding publishes that board
allocation instead of Core. Board advance/deadline and phase/finalization
callbacks consume their own state. Their declaration/publication class and
same-module forwarding fixture are checked together. Public board operations,
chip wiring callbacks and direct-board fixtures still retain the Core-to-board
association; these are live board API consumers, not a second attachment path.
Their whole caller migration and test classification remain required before
the physical move. [S80 evidence](../evidence/t540-s80-board-callback-context.md)
records the complete class and verification, not a completed public board cut.

S81 consumes the frozen-plan constructor publication class: one existing
candidate factory returns the board allocation to plan assembly, which publishes
Core and board outputs only after complete topology/timing success. Every plan
caller receives that real handle; driver destruction clears its borrowed lease.
Configuration-only fixtures and public board operations remain next receivers,
not a private-state getter or completed physical move. The
[S81 evidence](../evidence/t540-s81-plan-board-publication.md) records this cut.

S82 completes configuration construction publication across 125 caller files,
including the two existing allocation-failure seams. The board output is
optional and borrowed; requested outputs share the existing allocation and
clear on failure. All prior calls adopt the signature without changing their
test behavior. Public board operations and direct field fixtures still require
actual board-handle migration; constructor publication does not remove that
private association. [S82 evidence](../evidence/t540-s82-config-board-publication.md)
records the complete class rather than a finished physical extraction.

S83 receives the full five-operation input class: byte/stream/scan-set, XT
fault signals and relative mouse. All actual callers use the constructor's
board output; Core mutation eligibility and lifecycle observation retain
one implementation. Its [evidence](../evidence/t540-s83-board-input-handle.md)
records caller/status proof. Display/configuration and chip wiring still have
separate complete receiver classes before association deletion and movement.

S84 receives the entire display configuration/observation/capture class on
the actual borrowed board handle, including every caller and the plan's
internal topology application. The sole configuration-open implementation
is exposed through Core's neutral contract; display owns its existing VADP
cache and never borrows Core layout. Other board configuration, chip wiring
and fixture classification remain separate receivers before physical moves.
The [S84 evidence](../evidence/t540-s84-board-display-handle.md) records proof.

S85 implements the complete DMA/RTC/FDC/HDC configuration and binding class
on the borrowed board, including seven callback contexts and cold-reset
registration. All sixty original calls use that handle. Core alone retains
route publication, configuration eligibility and finalization; FDC's distinct
opaque Core connection serves only its port registration. The
[S85 evidence](../evidence/t540-s85-board-controller-handles.md) records the
whole-definition/caller review and complete verification. Coordinator accepts
pushed P1 `8b67d2cdb` after actual-diff review. This class no longer needs a private Core-to-board
lookup. The remaining boundary is the complete parity/D4/speaker/absent-memory
class and RAM-resize veto, followed by constructor/direct-fixture association
removal and physical Core/flat IBM-PC relocation. These are whole receiving
boundaries, not one-function repairs or a completed move.

S86 consumes the complete remaining electrical/RAM-admission class: parity,
D4, speaker, absent-memory and D4 memory no longer obtain their owner through
Core's private board field. The existing attachment supplies the exact board
resize veto, with one neutral Core resize implementation. Its
[evidence](../evidence/t540-s86-board-electrical-boundary.md) records source
comparison and complete proof. The constructor/private association and full
direct-fixture class are the next single receiving boundary; actual neutral
Core and flat IBM-PC movement follow. Independent linkage and these handle
receivers do not complete physical extraction.

S87 completes the whole constructor/private-association and direct-fixture
class before physical movement. Core no longer retains a named board pointer;
the public neutral constructor and board factory publish their actual handles
without a getter. Production board headers no longer import private Core
layout. All 91 executing direct-board candidates retain their actual owner
and assertions; the separate negative-injection literal remains intentional.
The [S87 evidence](../evidence/t540-s87-construction-private-boundary.md)
records both-width full units/gates, all eight single boot checkpoints and
fresh artifacts. Actual Core and flat IBM-PC extraction remain unaccepted.
The next receiver delivers the whole neutral Core source/test/build component,
removing its old App source ownership; no further per-function preparation is
allocated merely to defer that delivery.

S88 physically receives all neutral source/header groups above into
`src/x86/core`, including execution/time, CPU lifetime, memory/port routes,
transaction and firmware publication. Its sole Shared target owns the source
list; NXVM board targets link it. Eight neutral tests move with that owner,
while actual 92h/KBC and firmware fixtures remain board-owned. The
[accepted evidence](../evidence/t540-s88-neutral-core-extraction.md) records
complete verification and actual pushed-change review; the flat common/AT/XT board groups
remain outstanding and are not hidden by Core's independent build.

The S37 prospective “S43 physical move” is superseded by this measured
split; it was never executed. The S45 source inventory finds PIC in three
production and fifty direct test files, PIT in four/eight, DMA in three/
thirteen, RTC in three/ten, FDC/HDC together in six/thirty-one, keyboard
in three/eleven and VADP in three/two. That exceeds one bounded S45 owner
move, so S45–S53 are assigned by chip group before implementation. S53
found one residual XT keyboard chip pointer and 212 board/electrical/callback
accesses in the remaining private sources; these are not a bounded single
receiver. They are assigned to S54–S59 before their implementation. If a
later actual diff still exceeds one safe owner boundary, create further
**numeric, linear** receivers and shift S53–S55
forward before implementation. Do not use S suffixes or leave a mixed state
group without a receiver. This intake changes no source, test, asset, INI,
binary or Shared/MyNES file.
