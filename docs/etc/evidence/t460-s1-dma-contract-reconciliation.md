# T460 S1 DMA Source, Reference And Contract Reconciliation

## Scope And Method

This S1 consumes, without renumbering, the qualified 16-row universe in
T450 S5/S6: `DMA-R1`--`DMA-R4`, `DMA-F1`--`DMA-F7`, and
`DMA-T1`--`DMA-T5`. Intel *8237A High Performance Programmable DMA
Controller*, order `231466-005` (Sep. 1993), is the normative chip source;
the IBM *Personal Computer AT Technical Reference*, order `1502243` (Mar.
1984), is normative only for the selected AT board facts. Both are scanned
PDFs with OCR text layers. S1 rechecked the rendered Intel printed pages
2--11, including the service-state, mode, M2M and timing diagrams; rendered
prose/tables/diagrams govern, never OCR.

The dated MAME, Bochs, QEMU, 86Box and PCjs observations already qualified by
Td S144 remain corroboration only. MAME's explicit state machine confirms the
manual interpretation; QEMU's deliberately unsupported command bits, PCjs'
mask-command TODOs and 86Box's later-machine extensions demonstrate why no
emulator behavior becomes a requirement. No source, test, dependency or
timing value is imported from them.

## One Production Path

`src/core/machine/dma.c` owns both controller register files, request/mask/
status/ISR state, page state, channel bindings, transfer dispatch, TC/EOP and
the paired-controller cascade relation. `machine_board.c` is the sole
validated binding adapter from a device request token to that DMA owner.
`machine_scheduler.c` alone requests/acknowledges/releases the existing T449
DMA hold and submits work to the existing transaction owner. `machine.c`
constructs/resets those owners and no VM path reaches DMA state. The registered
DMA channel, binding-token, RTC-authority, FDC-boundary and competition smokes
are the present focused proof surface.

Thus the repair must refine `dma.c`'s existing service progression and use the
existing scheduler/transaction calls. It must not add a DMA scheduler, a
second page store, a peripheral grant authority, or a host-time conversion.

## Exhaustive Reconciliation

| Rows | Manual-L3 result and current owner | Current disposition and single receiver |
| --- | --- | --- |
| `DMA-R1`, `DMA-R3`, `DMA-R4` | `dma.c` owns base/current registers, byte selector, request/mask/status/temp and reset commands. | Retain and extend focused regression only; S2/S3 must preserve the one owner. |
| `DMA-R2` | Command and mode bits are Manual L3. `dma.c` stores every bit, but DREQ/DACK polarity, write selection and compressed state are not all consumed. | S2 consumes request/acknowledge polarity and write selection; S3 consumes compressed state. |
| `DMA-F1` | Request eligibility, fixed/rotating selection and DACK-on-acceptance are Manual L3. | Selection exists, but acknowledge has no logical lifecycle; S2 adds it inside the sole DMA owner. |
| `DMA-F2` | SI/S0/S1--S4, READY wait and release order are Manual L3 causal states. | Current scheduler has the sole transaction/hold route but `Execute` collapses service to a transfer; S2 gives that route logical service/release states, without deriving electrical clocks. |
| `DMA-F3` | Demand, single, block and cascade behavior are Manual L3. | Demand/single/block are partly present; cascade immediately completes and is nonconforming. S3 makes cascade delegate through the existing paired-controller path. |
| `DMA-F4`, `DMA-F5` | Transfer direction, verify, TC/EOP and auto-init are Manual L3. | Retained transfer/terminal mechanism is one path; S3 proves and corrects only mode/lifecycle differences exposed by service phases. |
| `DMA-F6` | M2M has distinct channel-0 read then channel-1 write state groups. | Current code atomically copies; S3 separates those logical phases inside `dma.c` and retains one T449 transaction owner. |
| `DMA-F7` | Compressed-state selection and address-latch update relation are Manual L3. | Compressed bit is inert. S3 represents only logical state selection; AEN/ADSTB pulse shape and latch propagation remain L4. |
| `DMA-T1`, `DMA-T2` | The paired 8-/16-bit AT controller, cascade and page mapping are Manual L3. | Existing paired Core topology/page owner conforms; S4 supplies only missing board-proof cases. |
| `DMA-T3` | IBM states 3 MHz/five-clock bus cycles and PIT1 refresh request to DMA channel 1. | Existing clock-domain and PIT1 board refresh paths are separate and do not bind a DMA request. S4 composes them only if the retained PIT contract supplies a compatible request event; otherwise this row remains explicit L2 fallback, not an invented rate. |
| `DMA-T4` | AT reserves DRQ4, uses active-low DACK, and peripherals retain DRQ through DACK. | Channel-4 rejection and opaque request binding conform; S2 supplies DACK/DRQ logical lifecycle and S4 proves selected AT binding. |
| `DMA-T5` | This is the internal L2 ownership boundary, not a manual chip fact. | T449 remains the sole transaction, arbitration, reset and time owner. S4 records any required integration term; DMA never owns guest time or CPU state. |

Every row is therefore source-qualified and has exactly one disposition. The
only maintained non-L3 claims are electrical signal/pin parameters and an AT
clock/refresh publication until its source producer is available. They cannot
be silently upgraded by an emulator or a scheduler constant.

## S2--S5 Admission Plan

| Subtask | One outcome | Exit evidence |
| --- | --- | --- |
| S2 | One `dma.c` logical request-to-service/release progression consumes Manual-L3 request polarity, acknowledgement, priority, cancellation and reset through the existing hold/transaction owner. | Focused request/priority/polarity/release/reset traces and full caller sweep. |
| S3 | The same progression completes demand/single/block/cascade, TC/EOP/auto-init, M2M read/write and compressed logical states. | Focused mode, cascade, M2M, terminal and compressed-state regressions; no second transaction or page path. |
| S4 | The selected AT paired-controller/page/DRQ4/DACK binding is proven; PIT1 refresh and five-clock terms are composed only from an existing authoritative producer or retained as explicit L2. | AT topology/binding tests and a source-to-owner timing disposition. |
| S5 | All 16 rows are reconciled to source-backed proof or maintained boundary, then the current gate and stripped T460 artifact close the task. | Closure matrix, minimalism/owner sweep, full gate and `nxvm_0_5_0460.exe` publication evidence. |

`M5:T460:S1:DMA-CONTRACT-RECONCILIATION:OK`
