# M5 T540 S30 Plan, Construction and Reset Intake

S30 is source-only. The former instruction to move mixed plan/create/reset/
destroy in one S is not an owner-sized change: the inspected `machine.c`,
`machine_plan.c` and `machine_board.c` alone contain roughly 2,885 nonblank
lines. Entry, ROM and trace add roughly 351. This intake freezes the existing
order and assigns every live part to a bounded linear receiver before the
neutral Core physical move. No implementation, ABI, asset or test input was
changed.

## Current owner map

| Source and current sequence | Sole destination/owner | Boundary to retain |
| --- | --- | --- |
| `machine_plan.c` validation, timing declarations and frozen topology (`157–232`, `297–430`) | IBM-PC board composition owns the complete product plan; neutral Core receives only validated immutable CPU/time/transaction inputs | No duplicated Core plan or profile name in neutral Core. The plan is validated before allocation. |
| `machine_plan.c` topology application (`233–296`) | IBM-PC board composition invokes bounded Core memory/port registration and device configuration | Memory aliases precede absent-memory/parity/D4/display/DMA/RTC/FDC/HDC. First error aborts; `create_from_plan` destroys the partially built machine and clears the result pointer. |
| `machine.c` create preflight and Core allocation (`405–620`) | Neutral Core: lifecycle, CPU/FPU, transaction, timeline, memory/port, CPU bus and instruction timing. Board composition owns keyboard/PIT/PIC/DMA choice and controller clock inputs. | Preserve early invalid-argument/no-memory returns, CPU/FPU teardown and single `core_machine_destroy` path after bus/memory construction. |
| `machine.c` controller creation and wiring (`621–746`) | IBM-PC board composition: 92h, VADP, XT PPI or KBC, DMA, PIC, PIT, IRQ0, speaker, reset/A20 and optional auxiliary PIT | Preserve port registration checkpoint, DMA before PIC before PIT, PIT0→IRQ0 binding, XT/AT exclusive branch, PIT1 unbound until post-reset wiring, and on-failure rollback/destroy. |
| `machine.c` `create_from_plan` (`754–778`) | One board-composition prepare/apply sequence around one neutral Core constructor | Validate first; on apply failure destroy one constructed machine and return a null output. Copy plan only after all topology effects succeed; do not retain caller-owned retirement qualification pointer. |
| `machine.c` cold reset (`834–925`) plus `machine_board.c` board reset (`286–344`) | Core resets CPU/FPU, memory/port, transaction/timeline, CPU bus locality and lifecycle; board resets keyboard, DMA/RTC, FDC/HDC, PIC/PIT, D4/parity/speaker/video and controller clocks | Original order is CPU/FPU→port/memory/D4→keyboard→DMA/RTC→board state→FDC/HDC→PIC/PIT→post-PIT board wiring→video→Core counters/transaction/timeline/clocks→providers→firmware→STOPPED/trace. Firmware failure leaves INITIALIZED. |
| `machine.c` processor-only reset (`929–949`) | Neutral Core CPU-local reset | Preserve RAM, board devices, timeline and scheduled work. Never reuse cold reset here. |
| `machine.c` destroy (`1524–1557`) and partial initialization branches | Core destroys CPU/FPU, port/memory/timeline/trace/bus and owned ROM mappings; board destroys only created chips/medium connections | Reverse-safe partial teardown; ROM aliases do not free another mapping's image. No parallel destructor or hidden board-owned Core free. |
| `rom_mapping_interface.c` and `entry_plan_interface.c` | Core owns memory routes, immutable ROM copies/aliases/rollback and CPU prepared entry; board/firmware supply bytes and requested entry | Candidate before publication, reset-overlay priority, bounded preload validation, failure rollback and single mapping table remain. |
| `trace_interface.c` | Core owns bounded transaction/CPU trace recording; board emits only typed device events | No second trace buffer or board-owned guest timeline. |

The reset and construction paths are not symmetric by design: a cold reset
reinitializes the board, while an 8042 processor pulse resets only CPU-local
state. That distinction must survive all receivers.

## Revised linear receivers

| S | Complete bounded work |
| --- | --- |
| S31 | Separate frozen plan validation and timing declarations from neutral Core configuration. Keep one board-owned plan and one Core construction input, with no copied mutable plan. |
| S32 | Isolate neutral Core preflight/allocation/CPU-FPU/transaction/timeline/memory/port construction and its early failure exits. |
| S33 | Isolate board controller construction, port registration checkpoint and `create_from_plan` topology application/rollback while preserving the original device order. |
| S34 | Split cold reset into Core/board stages at the existing sequence points; retain processor-only reset and firmware-failure lifecycle exactly. |
| S35 | Split partial-failure and final teardown by owner, including immutable ROM image versus alias ownership, without adding a second destruction route. |
| S36 | Qualify entry/ROM/trace Core boundaries and remove any remaining board-state access before physical relocation. |
| S37 | Audit the actual S31–S36 source diff and all construction/reset/rollback callers; repair any gap, then freeze the finite neutral Core file ledger. |
| S38 | Move only the proven neutral Core and its direct tests to `src/x86/core` and `test/x86/core`, delete the App copy, reconnect NXVM and review other consumers. |
| S39 onward | Extract only proven IBM-PC common, AT and XT board mechanisms into the flat Shared layers, each with a unique owner and receiving App; finish with the full T-wide external integration gate. |

The prior prospective S31 physical move and S32 board extraction are
superseded only for unadmitted work. Earlier accepted S packets and their
historical planning rows remain evidence, not an alternative live schedule.
No S suffix, generic device framework, duplicate plan, second reset or
second guest clock is authorized. Each code S must meet its own dual-width
unit, specialized-gate, four-profile boot and eight-product exit checks.

## Source-only verification

The S29 accepted baseline remains unchanged: x64/x86 **469/469** units,
specialized gates, **8/8** boot checkpoints and eight optimized 0540
products. This source-only intake changes no production, test or executable
input. Documentation governance and `git diff --check` are the S30 checks.
T540 stays open for S31 onward.
