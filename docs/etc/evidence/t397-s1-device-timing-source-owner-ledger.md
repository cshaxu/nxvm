# T397 S1 DeskPro Physical-Device Source, Owner And Probe Ledger

## Admitted Evidence Boundary

Read-only owner-managed D3PE research material contains the 1986 technical-spec
text/scan set; no asset bytes, firmware
or media are imported. It contains useful signal facts, including CLK16/BWAIT
logic and DMA wait-state diagrams, but it does not provide an NXVM
retirement-to-signal conversion or a guest-visible phase trace. The existing
T396 neutral-clock result therefore remains a prerequisite, not an invitation
to encode a scalar.

PCjs has an explicit DeskPro 386 model, second PIT ports, port-61 NMI bits and
some FDC/HDC compatibility branches. Its source is a secondary behavior lead
only: it has no accepted Model-40 physical tick contract and cannot authorize
an NXVM timing value. T387 already rejects generic/reference substitution.

## Selected-Route Universe

| Route | Current owner and preserved behavior | Primary source / observable status | Reset/cancellation boundary | S1 disposition |
| --- | --- | --- | --- | --- |
| 1.2 MB FDC, DMA2, IRQ6 | Core FDC/DMA/PIC owns command, DRQ/IRQ and logical 500 kbit/s service; VM selects Model-40 topology and drive | D3PE establishes selected topology; no source-backed FDC request-to-DACK or command-to-IRQ phase in Core ticks. PCjs has DeskPro NOT_READY behavior but is secondary | Core cold reset clears FDC/DMA/PIC before timeline rearm; FDC requests become eligible through Core arbitration | Retain functional route. A future bounded probe must name FDC command, DRQ/IRQ checkpoints and physical timebase conversion; no service scalar now. |
| Compaq 40 MB controller, IRQ14 | Core HDC owns normal Compaq PIO state and IRQ; VM owns checked geometry/raw IMG attachment | T386 and D3PE identify normal controller/geometry path. `TODO(Medium)` retains Read/Write Long, format, ECC, physical sector and drive-service timing; PCjs HDC behavior is secondary | HDC reset and device-media reset precede readiness callbacks; IRQ clear is current logical behavior | Explicit physical-media non-admission. Do not reinterpret RAW-IMG as an ECC/sector-time model. |
| CECG, firmware and ISA/video visibility | Core VADP owns selected digital personality and copied presentation; VM supplies selected composition/firmware provider | D3PE/CECG records support existing configuration/port facts. `TODO(Medium)` retains option-ROM/BIOS programming, raster/monitor, sync, ISA availability/waits and board-memory duration | VADP resets before its clock domain; host presentation consumes copied state outside guest time | Explicit physical/firmware non-admission. Requires a source-backed firmware-visible observable, not generic EGA/VGA timing. |
| KBC, reset and selected NMI/port-61 routes | Core KBC, D4/platform, PIT and CPU-reset owners retain functional response/latch/reset behavior | D3PE maps selected signals; the source corpus lacks propagation/settle duration and the required physical timebase. PCjs port bits corroborate topology only | Core cold reset clears KBC/D4 latches and domains, then provider/firmware reset; KBC reset request has a defined lifecycle path | Retain logical ownership. Physical key-controller, fail-safe and IOCHK visibility stays non-admitted pending an observable contract. |
| DMA/BWAIT and board-visible availability | Core transaction/DMA owner keeps deterministic arbitration; T396 records 4 MHz DCLK and one logical delivery path | D3PE text has CLK16/BWAIT/DMA wait diagrams, but no proof that a Core DMA callback equals a DCLK cycle or that one CPU retirement spans a named signal phase | Transaction, DMA and clocks reset before due callbacks; trace exposes logical advance only | Preserve source facts as a boundary. Any implementation must extend the T396 physical contract with a named transaction trace; do not add a Model-40 scheduler. |

## Complete Non-Admission Rule

S1 admits no implementation batch. Every selected route has a single current
Core owner and VM topology declaration, but none has all of: (1) primary-backed
physical unit/phase, (2) a named guest-visible checkpoint, and (3) conversion
from the qualified CPU retirement contract. A later subtask may create a
bounded probe only after it declares those three facts and remains asset-free.

## Verification

- Static sweep used source/device owner queries over `src/core`, `src/vm`,
  `tests`, `docs/states/TODO.md`, T386/T387/T396 evidence and read-only
  `O:\repos.external` references.
- Focused CTest command covered Model-40 integration, FDC, FDD, D4 parity/compatibility, CECG S9/S10/S11/S12/S13/S28 and Core competition/input-display ordering; all 13 passed.