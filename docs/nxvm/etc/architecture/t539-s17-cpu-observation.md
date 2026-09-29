# T539 S17: CPU Observation Prerequisite

Baseline a3bb1e3f8. Current owns admission and acceptance. This is a bounded
prerequisite to the CPU ledger row, not its completed extraction.

## Findings And Final CPU Boundary

The CPU context still holds RAM, port, PIC and transaction private pointers.
Its eventual independent owner must receive physical memory/I/O operations,
interrupt sampling/acknowledgement and copied diagnostics instead. CPU retains
translation, instruction/exception state and internal timing. NXVM retains A20,
reset-ROM mapping, address decoding, PIC cascade and transaction arbitration.
The unused firmware interception route has seven providers, all with null
software-interrupt slots; it must be removed, not replaced with another hook.

The timing source also contains board external-cycle/page-wait accounting and
DMA handoff invalidation. Those functions stay board-owned; CPU timing formulas
and repeat state move with CPU. Generated timing catalog ownership and private
CPU consumers must migrate together, without a machine-shaped Shared context.
Opaque CPU extraction follows this prerequisite under a separate reviewed batch.

S17 repairs an existing obstacle to that contract: preview suppresses transaction
and diagnostic callbacks but still calls ordinary physical reads. Those reads
invoke MMIO data callbacks and RAM parity notification. Three control-stack
timing helpers also reread descriptors using that route. EGA reads visibly
replace its latches. Calling this path observation is therefore incorrect.

## One Route, Explicit Read Intent

Use one physical address resolver and one read implementation, parameterized by
observation intent. Ordinary reads preserve all existing effects. Observation
reads produce the same selected bytes without parity notification or mutable
device effects; they are serialized with machine execution, not concurrent
snapshots. Preserve range validation, per-byte provider crossings, A20, overlay,
reset alias and error semantics. Never bypass a selected provider to read RAM.

The existing bounded device-read callback receives the explicit intent; all
registered providers must honor it. Pure ROM, backing-memory and open-bus readers
need no extra state. Device readers delegate to their owner. Shared video gets
a bounded inspect operation using the same read implementation: calculate EGA
read-mode results from local latch values, and publish latches only for an actual
guest read. No whole-device clone, save/restore, second VRAM or public latch getter.

CPU instruction preview and timing descriptor observations use this route.
Ordinary operand, DMA and firmware bus accesses remain operational. Review
display/debug readers separately by their declared observation semantics; do not
silently change an operation that intentionally models a guest bus cycle.

## Verification And Limits

Prove ordinary-versus-observational reads for RAM parity, routed providers,
crossing ranges, A20/reset aliases and EGA planar/color-compare/chain-4 behavior.
Use write-mode-1 consequences or same-owner tests to verify latch preservation;
do not add test-only public state. CPU preview preserves decoded bytes and page
table state. Existing CPU timing suites retain exact results and grades.

The similar-issue sweep records every read provider and observational caller.
Current names full receiving suites, artifact and gate requirements. Preserve
original instruction handler formatting; this batch changes boundary calls,
not the opcode tables. It neither accepts CPU extraction nor changes the finite
T539 completion predicate. No timing downgrade is authorized.
