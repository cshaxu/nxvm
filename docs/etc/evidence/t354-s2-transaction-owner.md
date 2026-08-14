# M5 T354 S2: Selected CPU/DMA Transaction Owner

## Decision

S1 found the same execution-time construction in two owners: the CPU reached
checked physical memory and frozen ports directly, while the 8237A DMA owner
reached checked physical memory directly. The existing stopped/paused
`core_machine_bus_*` facade is deliberately not involved. S2 therefore adds
one core-local, synchronous transaction boundary rather than a guessed wait
table or a second bus facade.

The boundary records `begin`, then either `commit` after the existing owner
has completed its validated access or `cancel` on a checked failure. It has no
duration field and does not make an instruction interpreter resumable. That is
intentional: the current executor commits an instruction atomically; the
selected result is one observable ownership/publication boundary per physical
CPU or DMA access. S3 receives any admissible between-owner competition policy
and must not claim instruction-internal HOLD/HLDA phases from this work.

## Route And Lifecycle Sweep

| Route | S2 owner path | Commit/cancel boundary | Retained semantics |
| --- | --- | --- | --- |
| CPU fetch/data/page-table physical reads | `_kma_read_physical` | Begin before checked read; commit after it; cancel before `#CE` on failure. | A20, immutable ROM, mapping and CPU exception semantics remain memory/CPU-owned. |
| CPU physical writes | `_kma_write_physical` | Begin before checked write; commit after it; cancel before `#CE` on failure. | Existing instruction atomicity and memory-owner validation remain unchanged. |
| CPU port reads/writes | `_p_input` / `_p_output` | Permission check remains before begin; port execution failure cancels before `#CE`; successful CPU-side publication commits. | Frozen registry/provider ABI and device-local callback semantics remain unchanged. |
| DMA ordinary device-to-memory / memory-to-device | `Transmission` | Route preflight remains before begin; checked physical-access failure cancels; successful transfer commits before channel progress. | DMA request, count/address, EOP and provider bindings remain DMA-owned. |
| DMA memory-to-memory | `Execute` M2M branch | Both routes validate before begin; either checked access failure cancels before progress; only a completed copy commits. | M2M terminal and auto-init lifecycle remain the accepted T348 owner. |
| Reset and trace | `core_machine_cold_reset`, transaction callback | Active transaction receives one cancel notification, then state/counters reset while the trace binding is retained. | Timeline/device reset order and stopped/paused API behavior remain unchanged. |

The 8237A documents request/hold/acknowledge ownership at a hardware boundary;
the selected emulator boundary above deliberately does not invent an exact
HOLD/HLDA cycle duration. The IBM PC/AT reference establishes the selected
DMA/PC-AT context but does not provide a safe universal CPU wait constant for
this core.

## Focused Proof

`core-machine-transaction-s2-smoke` executes a real CPU instruction stream
through the reset vector: memory fetch/read and ordinary port write emit
adjacent begin/commit trace records under the core trace provider. A local DMA
channel-2 device-to-memory request uses the same transaction state and proves
one DMA write begin/commit record plus the expected memory byte. It separately
opens a transaction, resets it, and proves the cancellation callback occurs
while reset leaves no active owner or retained counters.

The reset-vector fixture maps 16 bytes because the decoder's initial fetch
window can inspect more than the eight bytes in the sample program. This is a
fixture-width correction, not a transaction-state failure.

Existing DMA channel/arbitration and port/memory trace smokes remain the
regression owners for provider, route, EOP and stopped/paused contracts.
Four retained deterministic-L3 trace fixtures were corrected to map the
decoder's 16-byte reset fetch window and to select their named timeline events
rather than rely on fixed raw trace indexes/capacities. Their DMA-to-PIT-to-PIC,
readiness, and KBC-to-VADP ordering assertions are unchanged; the extra begin and
commit records are now intentionally observable rather than silently dropped.

## CMake And Strict Ownership

The owner smoke is a new pure project test executable and enters the retained
T345 S2 strict-test cohort. Its addition changes that exact cohort's
configuration assertion from 127 to 128 targets; all four GCC strict options
remain target-local. It does not promote a linked production target by
association.

## Transfers

S3 owns deterministic competition between this synchronous boundary, DMA
requests, FDC/ATA service and PIC acknowledgement visibility. S4 owns final
reset/session/trace and firmware/DOS reconciliation. Exact DRAM wait states,
prefetch/pipeline phases, electrical HOLD/HLDA/DACK/INTA waveforms and any
unselected device duration remain explicit non-goals.
