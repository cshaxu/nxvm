# T507 S4: 8237A DMA Controller Re-Audit

## Research quality and boundary

The primary source is Intel `231466-005`, *8237A High Performance
Programmable DMA Controller* (September 1993), located in the owner-managed
manual archive. It is a 19-page text-native PDF. Text extraction was used only
to locate material; rendered pages 4, 5, 7, 8, 9 and 10 were visually reviewed.
They establish the controller state machine, four transfer modes, command/mode/
request/mask/status registers, software commands, reset, priority, cascade,
TC/EOP and the clocked S1--S4 and S11--S24 phases.

IBM `IBM_5160_Technical_Reference_APR83` page 1-4 and
`IBM_5170_Technical_Reference_1502243` page 1-7 were also rendered and
visually reviewed. The XT source explicitly describes PIT channel 1 requesting
DMA refresh and distinguishes its refresh transfer; the AT source identifies
the 3-MHz DMA clock, five-clock data cycles, 8-bit channels 0--3, 16-bit
channels 5--7, and channel 4 as the cascade route. Together with the previously
accepted T506 evidence, they establish the selected system routes: PIT1 to
refresh DREQ0, 8272A to DMA2, and IBM XT Xebec to DMA3. Intel specifies
electrical clock and pin timing, but it does not map those values into every
NXVM Core time axis. Only the existing copied Model-339 rational DMA-clock plus
documented phase contract can therefore publish a sourced Core deadline.

Available local implementations were inspected as non-normative corroboration:

- 86Box `src/dma.c` separates request state, priority, cascade and controller
  register handling, but contains machine-wide compatibility machinery and is
  not a design source.
- Bochs `iodev/dma.cc` corroborates that a disabled controller publishes no
  request, the secondary cascade is an arbitration path, and TC clears the
  service path. Its bulk host-buffer service is intentionally not adopted.
- PCjs `machines/pcx86/modules/v2/chipset.js` corroborates request-register,
  mask, mode, master-clear and PC/AT port-family handling. Its asynchronous
  host transfer path is not adopted.
- No local MAME or QEMU 8237 implementation was available. No external source
  was copied, imported or treated as a timing authority.

## List 1 / List 2 reconciliation

| ID | Required 8237A or board relation (List 1) | Sole current owner and direct consumer (List 2) | Disposition |
| --- | --- | --- | --- |
| D1 | Reset and Master Clear clear command, status, request, temporary and first/last state, then mask every channel. | `core_machine_dma_controller_reset()` is used by reset and both Master Clear ports. | Manual-L3. |
| D2 | Address/count accesses share a first/last flip-flop; status read clears only TC bits; page registers retain their documented PC/AT routing. | `dma_{read,write}_{address,count}()` and `dma_port_{read,write}()` own both controller port families. `core_machine_dma_channel_smoke` covers primary, secondary and page routes. | Manual-L3. |
| D3 | Hardware DREQ and block-mode software request are prioritized; the software request is non-maskable and clears on TC/EOP/reset. | `dma_pending_requests()` combines masked hardware status with the required non-maskable block-mode request register. | Manual-L3. |
| D4 | Controller disable suppresses every service from the disabled controller. | `core_machine_dma_advance_one()` and `core_machine_dma_has_pending_request()` gate the secondary first and primary separately. The controller smoke covers either disable gate. | Manual-L3. |
| D5 | Fixed/rotating priority selects one eligible channel; a secondary controller is reached only through the primary cascade slot. | `GetRegTopId()` and `core_machine_dma_advance_one()` retain one derived cascade path; channel 4 is neither bindable nor software-forced. | Manual-L3 function/order. |
| D6 | Demand, single, block and cascade preserve their documented request/release behavior. | `Execute()` and `dma_complete_transfer()` retain the sole mode and completion lifecycle. | Manual-L3 function/order. |
| D7 | Normal service follows S1--S4; compressed timing omits S3; memory-to-memory follows S11--S14 then S21--S24. | `dma_service_begin()` and `dma_service_advance()` own the sole logical phase state. | Manual-L3 logical phases. |
| D8 | A transfer updates current address/count, observes transfer direction and page routing, and completes through TC/EOP/auto-initialize exactly once. | `Transmission()`, `Execute()` and `dma_complete_transfer()` own memory transaction, provider callback, TC, mask and reload behavior. | Manual-L3 function/order. |
| D9 | PIT1 refresh is a board route into DMA0, not another DMA state machine. | Immutable board configuration binds the PIT callback to the existing DMA request binding. | Manual-L3 route; its cadence is only L3 where the copied board/clock contract qualifies it. |
| D10 | 8272A DMA2 owns byte-ready DRQ and terminal semantics; DMA owns arbitration and transfer only. | `fdc.c` uses the opaque DMA2 binding and its one terminal callback; it never advances DMA itself. | Manual-L3 route; FDC byte timing remains FDC-owned. |
| D11 | IBM XT Xebec DMA3 owns its request, terminal response and IRQ5 behavior; ATA, WD1003 and Compaq/WD receive no DMA alias. | `hdc.c` owns Xebec provider callbacks and the opaque DMA3 binding. | Manual-L3 route; HDC command duration remains HDC-owned. |
| D12 | Pending DMA can publish the next logical service phase only with a source-qualified controller clock and phase contract. | `machine_scheduler.c` asks the existing copied DMA clock only for a qualified Model-339 plan; all other plans keep their stated lower-tier receiver. | Model-339 Manual-L3 plus sourced board contract; other profiles L1/L2, not guessed. |
| D13 | DREQ/DACK/HRQ/HLDA/EOP pulse widths and READY wait insertion are electrical or board-time relations. | No pin waveform model exists; no VM/profile or scheduler estimate is introduced. | L2 boundary for non-qualified board timing; no L4 claim. |

## Outcome and simplicity review

The current owner already satisfies every source-backed functional row and the
one qualified scheduler publication relation. No DMA code change is warranted:
adding a second request predicate, device timer or scheduler receiver would
duplicate the existing owner path. The retained path is:

`device/PIT request -> opaque DMA binding -> DMA owner arbitration/phase -> Core transaction -> provider terminal callback`.

The scheduler only observes the owner through `core_machine_dma_has_pending_request()`
and, for the already qualified Model-339 plan, the existing DMA clock domain.
It owns neither DMA state nor a separate service loop.

The transient focused set is the existing DMA controller channel, binding-token,
RTC-authority, Xebec wiring and plan smokes. The complete repository-only unit
suite and documentation governance remain the S closure gates.

## Verification

- Focused DMA cohort: 5/5 passed (`core-machine-dma-channel`,
  `core-machine-dma-binding-token`, `core-machine-dma-rtc-authority`,
  `core-machine-xebec-wiring` and `core-machine-plan` smokes).
- Complete repository-only unit: 312/312 passed with `ctest -L unit -j 8`.
- Documentation governance: passed after this evidence and its index were
  added.

The tracked implementation/test delta is `+0/-0`: the code review found no
missing owner-local behavior, and this S deliberately adds no unnecessary
production or test layer.
