# T539 S5: PIC Boundary Review

Baseline be86dee2e. This is the admitted extraction design, not completed
hardware qualification. The finite batch is pic.c, pic.h and pic_interface.h
plus all their consumers. Existing PIC algorithms and timing grades remain the
reference; this work imports no external implementation.

## Observed Coupling

The old pic.c mixes single-controller priority, ICW/OCW, poll and INTA with
20h/21h/A0h/A1h registration, two-controller selection, source-count aggregation
and peer pointers. CPU, FDC, HDC, KBC and XT keyboard retain pair connections.
Tests also write private ICW2/IMR and inspect IRR/ISR, initialization and delay
fields. Those accesses cannot survive as public mutable shared-chip layout.

## Resulting Owners

- `x86/devices/pic8259` owns one opaque controller: command words, IRR/IMR/ISR,
  rotating priority, input levels, local acknowledge and unmask delays. It uses
  only Lib Types. Local register access does not know PC port numbers.
- NXVM `pic_bus` owns port decode, source binding/counts, single/pair wiring,
  cascade propagation and selection of which chip supplies the CPU vector.
  Its endpoint contains an opaque chip handle, not mirrored registers. CPU and
  remaining device consumers connect to that board endpoint until their own
  extraction replaces concrete board bindings with neutral signal contracts.
- One execution owner orders calls. No chip queue, lock, registry or new
  scheduler. Construction failures roll back ports and created chips; chip
  destruction happens after IRQ sources release their borrowed endpoints.

## Contract Constraints

Use create/destroy/reset, local command/data read/write, IRQ assertion/release,
cascade input/selection, observe/acknowledge and advance/next-event operations.
Only copied signal/acknowledge values cross the boundary. Board source counts
are not a copy of IRR: they represent independent producers sharing an input.
Chip input level is the resolved signal, not a source registry.

Preserve the baseline's distinction between direct latched requests and the
currently available cascade request; preserve master/slave programming checks,
SFNM re-entry, poll acknowledgement, rotating EOI and spurious-vector behavior.
No private peer access is needed to decide the owning controller's priority.
Cascade interaction must be explicit rather than moving two PICs into one
supposedly independent chip. The exact public signal operations must be checked
against all these paths before the API is frozen.

Non-mutating observations must not accidentally acknowledge a poll or rewrite
OCW3. Tests that need implementation-level state belong inside the chip's own
test component; board/CPU tests instead initialize by real ICW/OCW operations
and observe their architectural effects. Failure diagnostics must not mutate
live hardware merely to print registers.

The eight configured unmask delays belong to each chip's state, but their
values and units/provenance come from the board. Reset preserves configured
delays and clears pending countdowns exactly as before. No-event differs from
bad arguments. This extraction does not upgrade an estimated delay to L3.

## Required Review And Proof

Reconcile all source/test/build hits, not just the dedicated PIC tests. Move
single-chip cases to test/x86/devices/pic8259; retain cascade, CPU acceptance,
source lifetime, port topology and profile timing tests in NXVM. Preserve the
existing CPU-family interrupt tests while replacing private setup/inspection.
Negative corpus checks prohibit App/Common/private-peer dependencies and old
chip duplication. Current holds exact executable verification and acceptance.

## Implementation Checkpoint

The independent draft uses a construction-fixed master/slave role instead of
peer pointers. Copied input masks carry resolved levels, new assertions and
available cascade signals; local selection returns line/vector/cascade values.
The chip retains SFNM selection, while poll uses the original local priority
path without that master-only override. Slave ICW3 remains a CAS identity,
not a master's attached-slave mask. Board integration must retain these
distinctions when replacing the old pair implementation.

NXVM now uses the shared chip through pic_bus; the old implementation and
private register accesses are removed. CPU fixtures program real ICWs and
observe register effects; chip-internal command tests moved into test/x86.
Live boot diagnostics no longer rewrite OCW3 merely to inspect PIC state.

The initial x64 full run exposed an assembly transaction error: beginning PIC
registration cleared a preceding device's allocation failure. Machine assembly
now checks the preceding registration status before starting that transaction.
The original failure-injection test passes; a new test injects failure at each
of PIC's eight port registrations, verifies rollback and retries successfully.

Both widths pass full units 341/341, default integration 20/20 and independent
chip suites 12/12. All six other profile/width boots pass once. Six manifests,
specialized gates and extended dependency-negative checks pass. Eight verified
artifacts and the source mapping are in [S5 evidence](../evidence/t539-s5-pic-extraction.md).
Current owns the subsequent actual-diff acceptance and closure state.
