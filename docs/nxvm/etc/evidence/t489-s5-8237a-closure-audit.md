# T489 S5 IBM 5160 8237A Closure Audit

`M5:T489:S5:8237A-CLOSURE:OK`

## Complete-unit result

T489 followed the required independent-hardware-unit sequence: S1 visually
checked the Intel 8237A and IBM 5160 sources; S2 froze the finite 16-row List
1; S3 mapped every row to the sole current owner before any source change; and
S4 consumed the one admitted implementation batch.  It is not a sequence of
symptom repairs.

| Frozen List-1 rows | Closure disposition | Evidence / owner |
| --- | --- | --- |
| DMA-R1--R4 | Accepted: register, port and reset state has one Core owner. | `src/core/machine/dma.c`; S3 List 2. |
| DMA-F1--F7 | Accepted: request/grant, transfer modes, TC/EOP, cascade, compressed timing and memory-to-memory remain on the one Core DMA path. | `dma.c`; `machine_scheduler.c`; S3 List 2. |
| DMA-X1--X5 | Accepted: IBM-selected single-controller channels, refresh route and primary page ports are represented without AT-only aliases. | S2 List 1; S4 closure. |
| DMA-T1--T2 | Accepted: source-defined logical sequence is owned by the DMA service/arbitration path. | `dma_service_advance`; S3 List 2. |
| DMA-T3--T4 | Transferred: IBM's physical formulas are retained Manual-L3 facts, but converting them to the present unqualified Core elapsed axis is L2 and no code gap. | S2/S3; `Profile physical-timebase closure` TODO receiver. |
| DMA-T5 | Accepted: reset/cancellation and timer/request route retain their existing owners; no VM timing or second scheduler exists. | S2/S3. |

## Batch and simplicity review

The sole batch was the XT page-port topology correction: primary `81h`--`83h`
remain registered for the selected controller, while secondary-controller
`87h`, `89h`, `8Ah`, `8Bh` and `8Fh` are registered only when the immutable
plan selects a second controller.  It changed the existing owner rather than
adding state, a scheduler, a profile mutation route, a wrapper or a parallel
DMA implementation.  Production and focused-test code changed by +25/-5
lines; the remainder of the S4 commit is evidence/indexing.

The full tracked DMA/XT proposal set was reviewed against the shared
specification-driven admission contract.  The next controller candidates
(8254, 8255, 8272A, CGA/VADP and Xebec) each retain the same mandatory order:
original-source ledger, complete List 1, complete List 2, then one owner-local
full batch.  The final XT and pre-Windows audits remain transfer-only and may
not repair a discovered controller gap.

## Focused proof retained

S4 rebuilt and passed `core-machine-plan-smoke`,
`core-machine-dma-channel-smoke`, and `vm-xt-5160-268-profile-smoke`.  The
plan smoke now proves the selected XT retains primary page ports and omits all
secondary/AT page ports; the DMA smoke retains programming, transfer and
reset coverage; the profile smoke retains immutable XT topology coverage.

No unresolved implementation batch remains in T489.  The next active unit is
the independent IBM 5160 8254 PIT task; it must establish its own sources and
both ledgers before changing PIT code.
