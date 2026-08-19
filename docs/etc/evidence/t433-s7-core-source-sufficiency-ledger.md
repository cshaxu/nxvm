# T433 S7: Core Source-Sufficiency And Ownership Ledger

`M5:T433:S7:SOURCE-SUFFICIENCY:OK`

## Purpose and decision vocabulary

This corrective ledger answers a narrower question than T433 S6: whether the
source material already recorded in this repository is sufficient to admit an
L3 rule. It does **not** treat a feasible Core mechanism, a functional smoke,
or a generic chip manual as proof that a selected machine has all inputs for a
timing rule.

Each row has two independent dispositions:

| Field | Meaning |
| --- | --- |
| Rule owner | `Core` owns architecture/chip-state-machine rules. `VM profile` owns selected board, wiring, clock, personality, and media facts. `Host/product` is outside guest time. `Unsupported` needs a separate admission. |
| Source status | `semantic available` identifies a usable manual/specification class for the Core rule. `partial` has a source but lacks a selector, revision or integration contract. `absent/unselected` has no admitted source for the needed rule. `not guest-time` needs no L3 timing source. |

`Profile input` is not a missing Core algorithm: Core must validate and copy it
from VM. A profile must not supply an algorithm by callback. Conversely, a
missing Core rule blocks even a profile that has values.

The citations below identify existing committed source inventory. This S does
not download or re-qualify external manuals; a future admission still applies
the source policy to a newly proposed source or reference contract.

## Source basis already retained

| Source class | Retained evidence and bounded use |
| --- | --- |
| CPU architecture/timing | [T433 S2](t433-s2-core-cpu-capability-reconciliation.md) and [S6](t433-s6-core-l3-admission-feasibility-ledger.md) retain 8086/80186/80286/80386 table/form sources for selected rows only. |
| AT CPU/board interface | [T369](t369-s1-pcat-bus-fact-route-ledger.md) retains IBM 5170 and Intel 80286 interface sources, and expressly rejects deriving a machine-wide wait table. |
| Controller state machines | [T370 S1](t370-s1-pcat-device-service-owner-ledger.md) retains 8259A, 8237A, 8254, MC146818A, UPI-41/42, uPD765 and IBM AT topology; these are semantic sources, not generic phase scalars. |
| Selected clocks/display | [T375 S2](t375-s2-model339-rtc-pit-clock-contract.md) and [T370 S5](t370-s5-planar-cga-transfer-audit.md) retain bounded clock/topology facts while transferring unsupported physical cadence/contention. |
| Debt boundary | [TODO](../../states/TODO.md) retains the 8086/80186 range-formula gap, broad x87, Model-339 MFM distinction, and unadmitted physical device timing. |

## Thirty-capability reconciliation

`Missing owner` owns the missing fact, not all surrounding code. `Eligibility`
states the precondition before a future task can register an L3 rule.

| ID | Current source status and bounded evidence | Rule owner / missing-fact owner | Eligibility and receiver |
| --- | --- | --- | --- |
| `CPU-EXEC` | **Partial.** Selected Intel form tables exist; [S6](t433-s6-core-l3-admission-feasibility-ledger.md) and [TODO](../../states/TODO.md) retain unresolved 8086/80186 MUL/DIV range selectors and other form/context gaps. | Core rule; operand/context selector is **Core**. Bus availability is **VM profile**. | Require exact formula or deterministic range selector; otherwise `SOURCE_UNALLOCATED`. Receiver: CPU timing programs. |
| `CPU-EXCEPT` | **Partial.** Architecture delivery semantics exist, but no complete per-profile delivery-stage elapsed matrix is admitted. | Core sequence; stack/vector/descriptor availability is **VM profile/transaction**. | Admit a finite delivery matrix plus selected transactions. Receiver: CPU timing programs after transaction work. |
| `CPU-PREFETCH` | **Partial.** CPU manuals can describe queues, but no selected CPU/board queue width, fill, flush and availability contract is registered. | Core mechanism; CPU personality/bus behavior is **VM profile**. | Require CPU manual plus board/bus contract. Receiver: CPU-to-board contracts. |
| `CPU-RETIRE` | **Semantic available.** This is Core publication/lifecycle, not a chip timing table. | Core; qualification comes from CPU/transaction rows. | Implement observation/validation now; do not label an unqualified elapsed result L3. Receiver: timing plan. |
| `CPU-FPU` | **Partial.** A finite 8087 surface exists, but [TODO](../../states/TODO.md) defers 80287/80387, broad formats, environment and protected-mode delivery; no complete current timing corpus is admitted. | Core coprocessor rule; personality is **VM profile**. | Separate x87 manual/corpus and no-FPU/error route admission. Receiver: named x87 TODO. |
| `TIME-CLOCK` | **Partial.** Core rational conversion exists; a project tick is not an oscillator and does not select every source/phase/wiring relation. | Core conversion; source frequency/phase/wiring are **VM profile**. | Plan validates declarations; profile selects values only with source-labelled board facts. Receiver: timing plan. |
| `TIME-LIFECYCLE` | **Semantic available.** Reset/cancel/finalize is a Core lifecycle contract. | Core; no external timing fact. | Register timed state and reject undeclared lifecycle. Receiver: timing plan. |
| `TXN-MEMORY` | **Partial.** T369 has CPU READY/topology but expressly no per-access wait table. | Core lifecycle; width/window/READY/DRAM availability is **VM profile/board**. | No L3 completion rule without selected board evidence. Receiver: CPU-to-board contracts. |
| `TXN-PORT` | **Partial.** Port semantics exist; selected ISA/device READY, width and completion do not. | Core lifecycle; decoded device/board availability is **VM profile**. | No scalar before selected interface source. Receiver: CPU-to-board contracts. |
| `TXN-ARBITRATION` | **Partial.** 80286/8237A define functional boundaries; T369 rejects inferred priority/grant time. | Core mechanism; board signals/priorities are **VM profile**. | Require selected board signal contract. Receiver: CPU-to-board contracts. |
| `MEM-RAM-A20-PARITY` | **Partial.** Logical route exists; no selected controller/planar contract defines classes, detection latency or latch visibility. | Core logical route; controller/planar is **VM profile**. | Require selected chipset/planar source. Receiver: CPU-to-board then device phases. |
| `MEM-ROM-FIRMWARE` | **Partial.** Copied-ROM lifecycle exists; decode/alias/shadow/availability are board facts, and vendor bytes are unnecessary. | Core lifecycle; mapping/speed/reset availability is **VM profile**. | Validate mappings now; require board documentation for L3 access. Receiver: timing plan. |
| `MACHINE-CONFIG` | **Semantic available.** Copied topology validation is a Core design rule, not a chip rule. | Core; no external algorithm. | Implement immutable plan and reject absent declarations. Receiver: timing plan. |
| `CTRL-PIC` | **Partial.** 8259A semantics and IBM cascade topology exist; selected acknowledgement/visibility contract is not registered. INTA waveform is outside L3. | Core state machine; cascade/wiring/delivery boundary is **VM profile/board**. | Admit command/ack phase and topology without waveform claims. Receiver: device phases. |
| `CTRL-DMA` | **Partial.** 8237A transfer semantics exist; DREQ source, service/grant relation and cascade binding need a contract. | Core state machine; device wiring/board grant conditions are **VM profile**. | Add sourced channel/mode/topology after transaction/arbitration work. Receiver: device phases. |
| `CTRL-PIT` | **Partial.** 8254 mode/count/output semantics exist; its input frequency, reset phase, GATE and OUT routing do not follow from that manual. | Core 8254 state machine; clock/wiring is **VM profile**. | Require copied PIT clock/wiring declaration and board evidence. Receiver: device phases. |
| `CTRL-RTC-CMOS` | **Partial.** MC146818A calendar/periodic/alarm/register-C semantics exist; oscillator/divider-reset/wiring to IRQ8 needs registration. | Core RTC state machine; oscillator/wiring is **VM profile**. | Require selected source/phase and IRQ topology; never host wall time. Receiver: device phases. |
| `CTRL-KBC-NMI` | **Partial.** IBM/UPI covers controller/FIFO/route semantics; controller clock, keyboard serial cadence and command delay are unselected. | Core state machine; keyboard/controller personality/wiring is **VM profile**; host arrival is **Host/product**. | Require selected keyboard/controller contract for guest phases. Receiver: device phases. |
| `CTRL-FDC` | **Partial.** uPD765 command/result/DRQ/IRQ semantics exist; clock, drive SRT/HLT/HUT, byte pacing, motor and rotational policy are unselected. | Core state machine; controller/drive/wiring is **VM profile**. | Require selected controller/drive contract; flux media remains unsupported. Receiver: device phases. |
| `CTRL-HDC` | **Absent/unselected.** Generic ATA behavior exists, but no admitted ATA/IDE revision plus controller/backing timing contract; Model-339 is not ATA and has no fixed disk. | Core ATA rule **missing**; controller/media also **VM profile**. | Block timing rule until ATA/IDE source and selected contract are admitted. Receiver: device phases. |
| `MEDIA-BACKING` | **Partial.** Provider lifecycle exists; geometry, error model and latency belong to controller/media format. | Core lifecycle; medium/personality is **VM profile**. | Register immutable logical results only; admit format contract for L3 phases. Receiver: device phases. |
| `DISPLAY-VADP` | **Partial.** IBM adapter/CGA context supports selected digital state; no selected generic VADP revision/cadence/contention covers every exposed adapter. | Core state machine; adapter revision/clock/wiring/monitor is **VM profile**. | Require selected adapter/CRTC source and bounded cadence contract. Analog/composite remains unsupported. Receiver: device phases. |
| `DISPLAY-PRESENT` | **Not guest-time.** Copied frame publication is a host-consumer boundary. | **Host/product**; no Core timing source. | Prove copy/lifetime isolation only. Receiver: integration/export. |
| `INPUT-HOST` | **Not guest-time for arrival; partial for accepted ingress.** Host timestamps/repeats are not source facts; guest phase is `CTRL-KBC-NMI`. | **Host/product** arrival; Core KBC receiver uses profile phase. | No host cadence rule; test ingress isolation. Receiver: integration plus KBC phases. |
| `TRACE-DEBUG` | **Not guest-time.** Copied observation has no chip/manual timing rule. | Core/product boundary. | Prove non-perturbation only. Receiver: integration/export. |
| `PLATFORM-MAILBOX` | **Not guest-time.** Mailbox lifetime is host integration. | **Host/product** boundary. | Prove publication cannot mutate guest timeline. Receiver: integration/export. |
| `PLATFORM-RESOURCE` | **Not guest-time for host I/O; semantic available for lifecycle.** Host latency cannot be guest time. | Core lifecycle plus **Host/product** resource boundary. | Validate deterministic provider results only. Receiver: integration/export. |
| `PLATFORM-WAIT` | **Not guest-time.** Host sleep/pacing cannot be a guest clock. | **Host/product** boundary. | Plan rejects it as a guest timing source. Receiver: integration/export. |
| `SESSION-COMMAND` | **Not guest-time.** Session/console control is product policy. | **Product** boundary. | Preserve stop/reset ownership; no plan entry. Receiver: integration/export. |
| `PRODUCT-DEBUG` | **Not guest-time.** Debugger tooling is product observation/control. | **Product** boundary. | Preserve non-perturbation; no plan entry. Receiver: integration/export. |

## Reconciliation result and immediate implementation gate

All 30 frozen IDs are classified without treating an external input as a
missing Core manual. `CPU-RETIRE`, `TIME-LIFECYCLE`, and `MACHINE-CONFIG` have
Core project-contract semantics and can enter the plan boundary now.
`TIME-CLOCK` and `MEM-ROM-FIRMWARE` can enter as copied validated declarations,
but a profile may select an L3 value only with source-labelled board inputs.
CPU forms, transactions, controllers, media and VADP have usable semantic
source classes but cannot register an L3 rule where the table names a missing
selector or profile contract. `CTRL-HDC` lacks a complete admitted current
source set; host/product boundaries are not L3 work.

The next implementation package remains **Core timing contract and machine
plan**, the first Queue candidate. Its first S consumes `TIME-CLOCK`,
`TIME-LIFECYCLE`, `CPU-RETIRE`, `MEM-ROM-FIRMWARE`, and `MACHINE-CONFIG`; it
must make a partial/absent row an explicit rejection or L2 fallback, never a
guessed default. T433 needs no further S merely to begin that bounded work.

`M5:T433:S7:OWNERSHIP-RECONCILIATION:OK`
