# Independent Shared Chip Components

## Goal And Admission

First of three ordered NXVM migrations requested by the owner, admitted as
[M5 T539](../history/M5-T539-independent-shared-chips.md). S1 is research/design
only. The owner's subsequent automatic-S authorization permits the coordinator
to admit each bounded production batch after its design review, without another
manual approval round. The active packet records each batch and its targets.
Extract the implemented chip
mechanisms from `src/app-nxvm/devices` into `src/x86/devices`, leaving NXVM
responsible for board composition and product adaptation rather than private
copies of shared chips.

## Boundary And Dependencies

- Each chip owns its registers, internal state, reset, functional behavior and
  internal timing. Its public boundary is opaque handles, copied values and
  bounded operations/signals, not another chip's private object.
- CPU owns instruction execution, exceptions and CPU address translation;
  board address decode, ROM/RAM maps, interrupt wiring and clock relationships
  remain composition responsibilities. CPU must not directly scan PIC objects.
- Board-provided memory/I/O cycles, interrupt acknowledgement and wait inputs
  connect the chip to its environment. Shared chips have no App dependency,
  host thread, wall-clock pacing, BIOS-specific response or machine-name branch.
- Shared CPU work extends the existing x86 package, not Common/Lib. Chip code
  depends only on declared neutral contracts and Types, not Common executors
  or the x86 Debug frontend. App assembly connects independently owned chips.
- Extract real existing Intel and related PC chip mechanisms where their
  identity and boundaries are established. Do not move the directory wholesale:
  classify every file, including memory, transactions, firmware, video, storage
  personalities, diagnostics and the machine scheduler. Non-chip mechanisms
  remain with their current board/adapter owner pending the second candidate.

## Coverage And Work Strategy

S1's [design review](../etc/architecture/t539-independent-chip-design.md) and
[81-file ledger](../etc/evidence/t539-chip-migration-ledger.md) document current
coupling, proposed component boundaries, receiving tests and pending decisions.
They are research outputs, not permission to change Shared code. Resolve the
CPU firmware hook and FDC unready-response ownership before their extraction;
retain behavior meanwhile. Do not label AT KBC or XT Mode-0 PPI models as full
general-purpose MCU/8255 implementations.

S2's [concrete contracts](../etc/architecture/t539-boundary-contracts.md) refine
these findings: the software-interrupt interception slot has no current provider
consumer and needs no replacement API; FDC unready semantics still require their
own full-class review. The owner approved PIT extraction and NXVM reconnection
on 2026-09-28; admit its source batch only after the S2 design delivery.

S3 implements that PIT batch: [delivery evidence](../etc/evidence/t539-s3-pit-extraction.md)
records the chip-only library, removed old route, board reconnection and receiver
verification. The owner subsequently authorized automatic admission of the
remaining S batches on 2026-09-28; this does not waive their boundary reviews,
verification, separate target commits or exit criteria.

S4 extracts RTC/CMOS under that authorization; its
[evidence](../etc/evidence/t539-s4-rtc-extraction.md) records the chip/board
split and malformed-month safety boundary. Current owns its acceptance state.

S5 extracts the single PIC mechanism and reconnects NXVM's board-owned cascade
and IRQ-source aggregation. Its [boundary review](../etc/architecture/t539-s5-pic-boundary.md)
and [evidence](../etc/evidence/t539-s5-pic-extraction.md) cover all callers,
private-test migration and construction rollback. Current owns acceptance.

S6 is an evidence-led DMA prerequisite: [first-service repair](../etc/evidence/t539-s6-dma-first-service.md).
Source review found that the secondary first transfer bypasses the phase handler,
contradicting the prior blanket timing-order audit. Repair and characterize the
whole affected channel/mode family before extraction; do not publish a shared
phase-bypass API merely to preserve this defect. S6 did not complete DMA migration.

S7 completes that DMA cutover under the [boundary review](../etc/architecture/t539-s7-dma-boundary.md):
one opaque controller in Shared, page/lane and paired-bus wiring in NXVM.
Its [delivery evidence](../etc/evidence/t539-s7-dma-extraction.md) records the
original-test migration, allocation rollback and receiving-product verification.
Current owns acceptance; no remaining chip batch is implicitly accepted.

S8 separates the qualified AT controller, keyboard and AUX device under its
[boundary review](../etc/architecture/t539-s8-kbc-boundary.md). The controller
owns transport; endpoints own commands/BAT/typematic/packet state; NXVM retains
ports, IRQ, A20/reset and clock attachment. Its copied connection contract and
stateless scan codec retain the original ordering without private peer access.
The [delivery evidence](../etc/evidence/t539-s8-kbc-extraction.md) maps original
cases, removed state, verification and artifact identities.

S9 consumes the XT PPI/keyboard rows under its
[boundary review](../etc/architecture/t539-s9-xt-boundary.md). It separates
register/serial mechanisms from board inputs and wiring, and characterizes
refused serial completion before moving the implementation. Current owns
execution status; this admission does not accept either remaining XT row.

The first admitted S freezes a migration ledger covering every tracked source
file in the current devices subtree, its callers and tests. Each entry names
the chip or board responsibility, dependencies, destination and proof. Every
entry must finish as migrated or explicitly retained with a non-chip reason;
an unidentified or still product-coupled shared chip blocks completion.

Subsequent S batches follow dependency closures, not speculative preallocated
S numbers. Move coherent implementations and tests, reconnect NXVM, and remove
the old production copy in each completed batch. Preserve original cohesive
instruction-handler style and all CPU-family semantics/timing evidence.
No universal device framework, plugin registry or forwarding-only wrapper.

## Verification And Exit

- All extracted chips build/test without app-nxvm headers, source or assets;
  source-boundary checks prevent reverse and private cross-chip dependencies.
- Chip tests move to `test/x86/devices` under its independent test build;
  board and App tests stay product-owned. No lost test scenarios or fixtures
  dependent on external ROMs in unit tests.
- One chip state and implementation path remain; all current CPU families,
  controller personalities and four fixed machine capabilities are preserved.
- Complete required unit suites per S, integration at T closure, manifests,
  static/governance checks and all affected x64/x86 receiving artifacts pass
  under the execution rules. Boot success does not upgrade timing grades.
- A final ledger/diff review proves every devices-file disposition and leaves
  the board-integration candidate a precise input inventory.

## Scope Controls

Source work requires approved Shared and NXVM packets, separate target commits,
and review of every receiving App; MyNES is changed only if explicitly needed
and admitted. This candidate does not add V30, Z80, Raiden II or PC110 support,
split Apps or move protected assets. It may not silently retire hardware or
degrade timing. Stop for new licensing, public-contract or product-behavior
decisions outside the admitted batch.

Next: [shared IBM PC integration](m5-shared-ibmpc-integration.md).
