# T366 S1: PC/AT Bus Profile And Route Inventory

## Decision

T366 cannot yet allocate a PC/AT availability, wait-state, or DMA-exclusion
model. The retained default machine is a selected deterministic PC/AT-shaped
composition, not a declared physical IBM 5170 board revision with a fixed
memory population, ISA-card set, disk controller, video adapter, and measured
firmware/DOS corpus. The current sources expose synchronous commit boundaries
and a deterministic equal-tick order, but no board-specific availability
inputs or resumable CPU/DMA phases.

IBM's PC/AT Technical Reference documents the AT system-board and dual-8237A
integration, including the channels-5--7 word-addressing constraint, but it
does not identify a universal CPU memory/I/O wait table suitable for this
composition. The primary source therefore supports topology and controller
semantics, not a timing value. 86Box, MAME, and PCjs remain permitted
reference-model leads when a selected contract supplies a comparable probe;
their current public machine/source surfaces do not select this project's board
or convert a model-local implementation into an IBM timing measurement.

The sole receiver is the open T366 bus-timed-PC/AT candidate: it requires an
owner-selected board/profile and a bounded executable corpus before a later S
may introduce availability state. This decision leaves the T365 NMI-source
receiver intact; no absent NMI source is treated as an availability signal.

## Current Route Ledger

| Family | Current owner and observed order | Missing physical-contract fact | S1 disposition |
| --- | --- | --- | --- |
| CPU fetch, physical memory, and port I/O | CPU helpers begin then synchronously commit/cancel the machine transaction around checked memory or frozen-port access; retirement publishes instruction ticks afterwards. | Board memory/ROM/ISA wait states, READY behavior, and whether a particular access may suspend the CPU. | Retain synchronous transaction only; no wait value or resumable execution. |
| DMA ordinary and memory-to-memory | The dual-8237A owner preflights routes, commits a DMA memory transaction, and the arbitration callback grants the retained one service before PIT/PIC. | Selected board's HRQ/HLDA, DACK, AEN, refresh, and cycle-steal duration. | Retain deterministic grant order; do not equate it with physical HOLD/HLDA. |
| PIC and logical acknowledgement | Device sources reach PIC refresh after DMA/PIT; CPU delivery performs the existing logical acknowledgement. | INTA cycles, spurious/electrical behavior and board timing. | Retain logical delivery; transfer physical INTA. |
| FDC DRQ/service | Readiness advances and refreshes FDC after arbitration; FDC owns deferred command/completion and its DMA binding. | Drive/controller/card timing and a measured DRQ-to-grant contract. | Retain controller lifecycle; no same-tick or delay inference. |
| ATA/HDC PIO service | Readiness advances and refreshes HDC after FDC; HDC is retained PIO. | A selected adapter/card, PIO timing, and any DMA ownership. | Retain PIO lifecycle; no ISA/ATA timing allocation. |
| Timeline/reset/trace | Machine owns callback scheduling, transaction cancellation on reset, and copied trace events. | Mapping from the deterministic tick domain to selected board bus phases. | Preserve lifecycle and trace boundary. |

## Source And Reference Disposition

- [IBM PC/AT Technical Reference, September 1985](https://www.bitsavers.org/pdf/ibm/pc/at/6139362_PC_AT_Technical_Reference_Sep85.pdf)
  is the primary topology/controller authority used for the dual-DMA context;
  it is not treated as a universal wait-state table.
- [86Box IBM AT machine notes](https://github.com/86Box/docs/blob/master/hardware/machinespecific.rst),
  [MAME PC driver](https://github.com/mamedev/mame/blob/master/src/mame/pc/pc.cpp),
  and [PCjs source repository](https://github.com/mgarlanger/pcjs) are
  non-imported secondary model leads. Under the source policy they may inform a
  later probe design only after the owner selects one comparable board/corpus;
  none is copied, linked, or used as a timing authority here.

## Required Owner Decision And Transfer

To admit a timing implementation S, select one physical target (at minimum:
board/revision, CPU clock, RAM and ROM population, video and storage adapter,
and enabled ISA devices) plus a reproducible firmware/DOS probe corpus. The
next packet must then map each measured or primary-specified availability fact
to CPU, DMA, PIC, FDC, HDC, transaction, reset, and trace behavior. If a fact
is only a range or absent, the project may use the named reference-model probes
only within that range and must retain an explicit receiver otherwise.

Until that decision, no code change is admissible. This is not a model-L3,
physical-timing, or cycle-exact claim.

## Verification

- Reviewed T354 S1--S4 transaction/competition/closure evidence and T347/T348
  controller-service lifecycle evidence.
- Rechecked current CPU transaction, DMA transaction, arbitration, readiness,
  PIC-refresh, instruction-retirement, reset and trace call sites.
- Consulted the primary IBM PC/AT reference and the owner-authorized 86Box,
  MAME, and PCjs reference-model surfaces without importing source or assets.
- Documentation governance and `git diff --check` are required before S1
  acceptance; this S changes no runtime path or developer artifact.
