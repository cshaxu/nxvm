# Shared IBM PC Board Integration

## Goal And Dependency

Admitted as T540 after closed [independent chips](../proposals/m5-shared-chip-extraction.md).
Extract the neutral x86 executor and the actual common
board mechanisms of XT, AT, DeskPro 386 and default PC/AT into the flat
`src/x86/ibmpc-common`, `src/x86/ibmpc-at` and `src/x86/ibmpc-xt` components.
This is reusable PC assembly, not another chip library or a host executor.

## Ownership

- `x86/chips` is the sole ownership location for each extracted chip's state
  and behavior; S4 removed the retired `x86/devices` source path.
- `x86/ibmpc-common` owns demonstrably shared IBM-PC port routing and signal wiring.
  It integrates chips only through their public contracts. The generic x86
  machine executor -- memory, ports, transactions, guest timeline, CPU bus and
  plan application -- is not IBM-PC code; its required neutral receiver is
  `x86/core`, established before independent Apps stop depending on the current
  App-owned Core interface.
- Common remains owner of host execution/lifecycle coordination; Lib remains
  owner of platform, file and audio services. There is no second lifecycle
  FIFO, host worker, scheduler time source, renderer or storage backend here.
- Each product composition selects its actual chips, wiring, clock inputs,
  address maps, constraints and immutable firmware roles. Board-unique Compaq
  or XT/AT details stay at their genuine owner; do not force four boards into
  one configuration full of model-name switches.
- Use small typed descriptions or direct assembly where semantics genuinely
  match. Accept resolved external assets through neutral inputs; no product
  INI parser, asset-root lookup or protected ROM bytes enter shared board code.
- Own board-local mutable glue once and define its reset/event lifetime;
  never mirror device registers to make routing convenient. Chip cycles and
  deadlines feed one guest timeline; host elapsed time cannot create ticks.

## Coverage And Work Strategy

S1 inventoried the four construction graphs and retained non-chip files. Its
durable ledger distinguishes common mechanisms,
genuine machine differences and App/host adaptation. Include bus transactions,
interrupt acknowledgement, DMA/refresh, firmware mapping, reset, time and
failure cleanup; similarity of filenames is not proof of shared semantics.

S2-S4 fixed the finite adapter ledger, target layout and chip path. S5 first
mapped the generic-Core/IBM-PC cut; S6 removed fixed port 92h from generic
memory. [S7's private-state handoff](../etc/architecture/t540-s7-core-board-handoff.md)
corrects the earlier S7/S8 file-move order: raw port/RAM layouts, board
deadlines, PIC acknowledgement and reset ordering must have one bounded
receiver before the neutral Core can move. Its prospective linear S8-S17
batches are admitted one at a time from current source evidence. Then extract
and reconnect one complete board dependency batch at a time. Finish with
NXVM using the shared implementation and delete its duplicate common board
paths. Preserve one construction/rollback owner and one reset path. No generic
board inheritance, universal event bus or new per-machine execution loops.

S60 measured the remaining private/public Core header and found a one-step
neutralization unsafe. Its [finite receiving plan](../etc/evidence/t540-s60-neutral-header-intake.md)
assigned S61-S64 to D4 memory, frozen plan, private and public interface
boundaries. S64's [measured public intake](../etc/evidence/t540-s64-public-interface-intake.md)
further splits configuration, board values, operations, Core handoff and
independent compilation into S65-S69. Its prospective first neutral Core
physical move was S70; the post-S69 consumer correction below supersedes
that receiving number before execution.
These are prospective numeric receivers, not a claim that Shared Core
already builds independently.

S69 proves actual independent compile/link/run, but its receiving audit finds
that board consumers still borrow neutral private state. Therefore the former
S70 physical-move row is superseded before execution: S70 closes the complete
CPU-signal class (three NMI sites and one processor-reset site). Subsequent
bounded receivers close memory/port construction transactions, firmware
publication and board provider/attachment ownership, then classify direct
tests before the physical move. Exact eighteen-header intake finds 306
source/test/build consumers; this is a receiving inventory, not permission to
move every test or to export private layouts. The coordinator admits each
next numeric receiver from its measured diff. Physical relocation remains
required, not replaced by independent compilation or this signal cut.

S71 receives the complete RAM-alias/parity construction class using one
Core-owned alias batch and the existing memory-route transaction. Board keeps
reset-address decoding and parity electrical policy. Port construction,
firmware publication and provider attachment remain separately measured
receivers before the physical move; no private pointer facade qualifies them.

S72 removes the redundant board-constructor port checkpoint after auditing
all failure propagation and whole-candidate destruction. Core typed batches
retain sole route transaction ownership. Reset-time refresh programming has
a distinct lifecycle and bus side effect; it receives the next measured S,
not a direct chip-write shortcut in this constructor cut.

S73 completes that distinct refresh-programming receiver after proving Core
scratch/width observability across construction and cold reset. Board uses
the existing PIT register contract; guest port I/O keeps one Core dispatch.
No construction-I/O facade is added. Firmware/attachment and test-owner
classification still precede physical relocation.

S74 receives the complete firmware bind transaction and its board reset-alias
completion. Core owns publication/rollback; board supplies PC policy through
bounded ROM operations, without private registry access or a second provider.
Attachment ownership and direct-test classification remain before relocation.

S75 receives six board input/display lifecycle checks through the existing
copied Core operation and places both READY definitions with the neutral
scheduler. Running Port-B time and deadline qualification are the next measured
scalar receiver; the existing stopped-time observer cannot be substituted in
running port dispatch. Attachment and test classification still precede the
required physical source cut.

S76 closes the full declaration publication/qualification boundary through
neutral batch validation/publication and a copied qualification value on the
existing deadline callback. Its intake assigns the distinct Running Port-B
read to S77's actual I/O-cycle input contract; a paused-time getter shortcut
cannot close it. Both receivers remain before attachment and source movement.

S77 supplies copied Core time on the existing typed read callback, including
CPU, paused bus/Debug and bounded firmware dispatch. Every lane and wired-OR
contributor receives the same value; both Port-B routes retain their existing
status algorithm. The write contract has no current time consumer and stays
unchanged. Zero-time raw reads remain synthetic fixtures only, enforced by
the production owner gate. Attachment, direct-test classification and source
movement remain separate receiving work, not inferred from this input closure.

S65 separates the private neutral construction value without rewriting the
public plan ABI: one board composition derives the temporary value and owns
the existing public create/test-allocation pipeline. Core does not retain
it. This removes the direct board validation/create calls from `machine.c`;
S68 receives the remaining four reset, clock-reset, NMI and finalization
calls. S66 owns the public board configuration/value boundary and its adjacent
operation declarations: by-value XT enum parameters cannot remain in a neutral
header without restoring the board dependency. Implementations remain unchanged.
S67 therefore receives the measured residual implementation dependency instead:
five neutral validation functions currently defined in board `machine_plan.c`
but consumed by neutral Core construction and timing declarations. Move them
verbatim to an existing neutral owner; do not add a validator facade or a
second validation path. S68-S70 retain their existing receiving boundaries.

S78 measures the remaining attachment class before implementation: nineteen
private callback slots, six production board-access owners and 97 direct-board
test files. The [attachment intake](../etc/evidence/t540-s78-attachment-owner-intake.md)
specifies one copied public binding and one opaque board handle, preserving
construction rollback and finalization order. Prospective S79 receives the
whole callback publication cut, not one slot; board-handle/test classification
then precedes physical relocation. No getter, registry, parallel binding or
new lifecycle worker is eligible. This design receiver changes no runtime.

S79 implements that complete copied callback binding: one public typed value,
one configuration-only publication and one Core finalization owner replace
all nineteen private callback slots. Board callback algorithms and dispatch
order stay unchanged. Its [evidence](../etc/evidence/t540-s79-copied-attachment-binding.md)
records verification and the retained Core-handle context bridge. The opaque
board handle and direct-test owner classification are still required before
physical relocation; a copied callback bundle alone does not complete them.

## Verification And Exit

- Shared board code depends on chip public contracts and declared neutral
  capabilities, never an App path; independently built x86 tests prove this.
- Common board contract tests live with their flat receiver under
  `test/x86/ibmpc-common`, `test/x86/ibmpc-at` or `test/x86/ibmpc-xt`; specific machine
  composition/firmware tests remain product-local. Units use synthetic owned
  inputs, not external files; integration retains all four real machine sets.
- Every ledger member has a verified shared implementation or a justified
  retained machine-specific owner; no duplicate common implementation remains.
- Required full unit/integration, manifests, static/governance checks and
  affected dual-width artifacts pass. Every existing boot scenario and timing
  classification is preserved; structural extraction makes no new L3 claim.
- The four App split receives a concrete source/test/build owner map, not an
  unresolved catch-all legacy directory.

## Non-goals And Stops

No App split, new hardware, PC110 implementation or asset relocation. Scope is
Shared plus NXVM, with other receivers explicitly admitted if affected; one
target per commit. Stop for a contract/behavior change beyond extraction,
unresolved shared ownership, or a requirement to import protected material.

Next: [four independent PC Apps](../proposals/m5-independent-pc-apps.md).
