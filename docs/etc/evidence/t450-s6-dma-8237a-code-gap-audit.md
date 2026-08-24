# T450 S6 DMA 8237A Current-Code Gap Audit

## Scope And Method

This audit consumes, without changing, all 16 rows in the T450 S5
[DMA source checklist](../research/t450-s5-dma-8237a-function-timing-checklist.md).
The reviewed production owners are `src/core/machine/dma.c`, `dma.h`,
`machine_scheduler.c`, `machine_board.c` and `machine.c`; composition is an
input only. Focused evidence is `tests/machine/core_machine_dma_channel_smoke.c`,
`core_machine_dma_binding_token_smoke.c`,
`core_machine_dma_rtc_authority_smoke.c`, and
`tests/core/machine_competition_s3_smoke.c`. Every nonconforming row transfers
once to the queued [Core DMA 8237A phase contract](../../proposals/m5-core-dma-8237a-phase-contract.md).
No source or test is modified by this audit.

## Row Dispositions

| S5 ID | Current owner and observed behavior | Current proof | Disposition and unique receiver |
| --- | --- | --- | --- |
| DMA-R1 | `dma.c` low/high byte helpers, address/count writes and `Transmission` retain base/current pairs, inclusive count, address direction and auto-init reload. | `core_machine_dma_channel_smoke.c` programs/reads channel pairs, decrement and auto-init. | Conforming. |
| DMA-R2 | `dma.h` retains every command/mode bit and `dma.c` owns controller enable, rotate, M2M, address direction, transfer/service modes and port decoding. However DREQ/DACK polarity, extended/late write selection and compressed-timing command choices are merely stored or incompletely consumed. | Channel smoke covers command/mode ports and selected mode effects. | Partial: command personality is not fully consumed. Receiver: queued DMA phase contract. |
| DMA-R3 | `dma.c` owns request/mask/status/temp registers, status-read TC clear and software-request eligibility. | Channel smoke covers software request, TC/status, mask and M2M temporary-latch lifecycle. | Conforming for retained logical register behavior. |
| DMA-R4 | `dma_port_write` implements first/last clear, master clear, clear/single/all mask commands; `doReset` resets volatile state while bindings persist at machine ownership. | Channel smoke covers both controller command families and reset; DMA/RTC authority smoke verifies machine reset binding retention. | Conforming. |
| DMA-F1 | `dma_pending_requests` and `GetRegTopId` implement masked request selection and fixed/rotating priority; request bindings make only the Core DMA pair mutable. It has no DACK state/callback and does not consume programmed DREQ/DACK polarity. | Channel smoke exercises primary priority, held/deasserted request and binding validation. | Partial: documented request-acknowledge/polarity relation is missing. Receiver: queued DMA phase contract. |
| DMA-F2 | `machine_scheduler.c` routes DMA through T449's hold and transaction owners for 80286/80386, and `core_machine_dma_advance_transaction` performs one primitive after grant. The source state sequence (SI/S0/S1--S4/READY) is not represented as a selected 8237A service phase. | Competition smoke proves hold request/ack/release, DMA transaction order and selected BUSRDY wait gate. | Partial: generic grant/wait knobs are not the selected 8237A service-phase contract. Receiver: queued DMA phase contract. |
| DMA-F3 | Demand, single and block transfer behavior is implemented in `Execute`; cascade immediately completes and has no delegated-controller service. | Channel smoke covers demand, single, block and held request; no cascade transfer test exists. | Missing: cascade is not the manual's delegated priority slot. Receiver: queued DMA phase contract. |
| DMA-F4 | `Transmission` implements verify without memory controls and direct device-to-memory / memory-to-device callbacks with transaction-protected memory access. | Channel smoke covers device-to-memory, memory-to-device, verify and rejected memory route behavior. | Conforming for logical transfer types. |
| DMA-F5 | `core_machine_dma_request_terminate`, `Execute` and `dma_complete_transfer` own EOP/TC, mask, callback and auto-init outcomes. No physical EOP pin is claimed. | Channel smoke covers external termination, terminal callback, TC, auto-init and M2M terminal owner. | Conforming for retained logical EOP/TC behavior. |
| DMA-F6 | `Execute` supplies channel-0-to-1 M2M, current-register movement, temporary byte and channel-1 terminal outcome in one transaction. It collapses the manual's distinct read and write state groups into one atomic copy. | Channel smoke covers M2M, auto-init, termination and invalid source/destination routes. | Partial: source/destination service phases are unrepresented. Receiver: queued DMA phase contract. |
| DMA-F7 | `dma_physical_address` and page ports provide selected 16/24-bit address formation, but the compressed command bit does not choose a state path and no AEN/ADSTB/address-latch event exists. | Channel smoke covers page-port mapping, word address formation and primary/secondary routing. | Partial: compressed/address-latch timing remains unallocated; pin waveforms stay excluded. Receiver: queued DMA phase contract. |
| DMA-T1 | `core_machine_dma_initialize` creates the paired controllers; `core_machine_configure_dma` validates the two-controller/cascade wiring and binds FDC channel 2. | DMA/RTC authority and binding-token smokes cover valid/invalid wiring and FDC binding identity. | Conforming selected topology. |
| DMA-T2 | `dma_port_read/write` owns documented page ports and secondary word addressing; `dma_physical_address` applies the word controller's forced low address bit. | Channel smoke checks all selected page-port values, channel mapping, word routes and sparse secondary ports. | Conforming selected page/address behavior; exact page-register reset source is not separately selected. |
| DMA-T3 | `machine_scheduler.c` advances a plan-derived DMA clock and T449 transaction path, while the AT's 3 MHz/five-clock formula and PIT-channel-1-to-DMA-channel-1 refresh route are not selected or bound. | Competition smoke proves generic wait/BUSRDY behavior only; no 5170 3 MHz or refresh-chain test exists. | Missing input and missing test: selected 5170 clock/refresh contract. Receiver: queued DMA phase contract, consuming the later PIT contract rather than duplicating its owner. |
| DMA-T4 | Opaque request bindings, two-controller cascade and FDC channel 2 prevent cross-machine/invalid-channel mutation. No peripheral-visible DACK lifecycle or selected DRQ source contract exists beyond FDC's current direct binding. | Binding-token, DMA/RTC authority and FDC DMA-boundary tests prove isolation and FDC channel route. | Partial: board DRQ/DACK/page/refresh interaction needs one selected contract. Receiver: queued DMA phase contract. |
| DMA-T5 | T449 owns transaction, arbitration, reset, time and observation; DMA calls only its transaction/hold APIs. The explicit 8237A selection has not yet supplied the required request/grant/page/refresh terms. | Competition smoke proves the single owner path and reset release. | Unallocated L2 input, not a code defect. Receiver: queued DMA phase contract. |

## Cross-Source Reconciliation

The S5 qualification is the source-tier authority for these same 16 IDs.
`Manual L3` below means the existing code audit is judged against a direct
Intel/IBM requirement. `Other L3` identifies only corroborating emulator
behavior; it never upgrades an unproven current path. Where neither tier
supports a specific Core claim, this audit records `fallback to L2`.

| S5 ID | Qualification | Cross-check result for current disposition |
| --- | --- | --- |
| DMA-R1 | Manual L3; Other L3: MAME/QEMU/86Box/PCjs. | Conforming register behavior remains accepted; service ordering is fallback to L2. |
| DMA-R2 | Manual L3; Other L3: MAME. | MAME's fuller command consumption confirms NXVM's unconsumed polarity/write/compressed bits are a real gap; fallback to L2. |
| DMA-R3 | Manual L3; Other L3: MAME/QEMU/86Box/PCjs. | Conforming retained register behavior; peripheral request origin remains fallback to L2. |
| DMA-R4 | Manual L3; Other L3: MAME/QEMU. | Conforming command/reset behavior; no host-delay claim. |
| DMA-F1 | Manual L3; Other L3: MAME/Bochs. | Missing DACK/polarity lifecycle remains a DMA-task gap; fallback to L2. |
| DMA-F2 | Manual L3; Other L3: MAME. | MAME's explicit service states confirm generic hold/transaction proof is insufficient; fallback to L2. |
| DMA-F3 | Manual L3; Other L3: MAME/Bochs. | Immediate cascade completion is nonconforming; delegated cascade service is required before L3. |
| DMA-F4 | Manual L3; Other L3: MAME/QEMU. | Retained logical transfer behavior conforms; waveform work remains excluded. |
| DMA-F5 | Manual L3; Other L3: MAME/Bochs. | Retained logical TC/EOP behavior conforms; no physical-pin claim. |
| DMA-F6 | Manual L3; Other L3: MAME. | Atomic M2M copy lacks the required source/destination service phases; fallback to L2. |
| DMA-F7 | Manual L3; Other L3: MAME. | Stored compressed bit without a state path is a gap; selected clock/latch timing is fallback to L2. |
| DMA-T1 | Manual L3; Other L3: QEMU/Bochs/PCjs. | Selected topology conforms. |
| DMA-T2 | Manual L3; Other L3: QEMU/86Box/PCjs. | Selected page/address behavior conforms; unselected reset detail remains fallback to L2. |
| DMA-T3 | Manual L3; Other L3: none. | IBM formula/refresh route is absent from Core; fallback to L2 until the DMA and PIT owners compose it. |
| DMA-T4 | Manual L3; Other L3: MAME/Bochs/PCjs. | Binding isolation conforms, but peripheral-visible DACK and selected DRQ/page/refresh lifecycle are fallback to L2. |
| DMA-T5 | fallback to L2. | T449's owner boundary is retained, but the selected 8237A integration terms are still unallocated L2 input. |

## Completeness, Minimality And Transfer

All `DMA-R1`--`DMA-R4`, `DMA-F1`--`DMA-F7` and `DMA-T1`--`DMA-T5` rows have
one disposition. The audit finds no second DMA state owner and no need for a
new wrapper or framework: the later candidate must make existing `dma.c` consume
the selected phase/board terms through T449's existing transaction and
arbitration owners. It must not create device-local time, duplicate page state,
or make FDC a second grant authority. The retained gaps are limited to command
personality, request/DACK and service/cascade phases, M2M and compressed phase
selection, and the selected AT clock/refresh wiring plus associated tests.
