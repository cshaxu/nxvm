# T487 S3 IBM 5160 Board Current-Code Gap List 2

`M5:T487:S3:IBM-5160-BOARD-LIST-2:ACCEPTANCE-CANDIDATE`

The complete S2 ledger maps to one existing direction:

```text
immutable XT profile -> copied Core configuration/plan -> Core machine owner
-> Core transaction, port, memory and time publication -> copied observation
```

No VM path writes guest board state, and no second XT scheduler, port map,
memory map or NMI owner was found. The source sweep covered `src/core/machine`,
`src/vm/profile/xt`, session composition, matching tests and CMake entries.

| List-1 keys | Current owner and production path | Current disposition | Whole-batch action |
| --- | --- | --- | --- |
| XT-B01--B02 | `vm_profile_xt_5160_268_declaration_create` copies `ticks_per_instruction = 1`; `core_machine_create_internal` copies `time_axis` and clock domains; `core_machine_capture_time_observation` publishes the copied result. | The profile declares 8088 and 256 KiB but contributes no IBM board clock plan. The Core clock-domain ratios already express exact device-to-CPU-clock fractions; only the optional host-pacing rate is integer-only. | Select the IBM board clock relations through the existing copied Core clock plan. Keep exact host pacing unavailable rather than expanding the global time-axis contract merely to represent `14,318,180 / 3`. |
| XT-B03 | `core_machine_cold_reset` is the sole reset path; Core resets XT PPI, PIC, DMA, PIT, memory-facing state and providers. | Reset has one owner. No source-selected board-edge relation is represented; the manual states edge synchronization but no reset duration. | Preserve Core reset ownership. Add no delay or second reset path; the implementation proof must show reset is not charged as invented time. |
| XT-B04--B06 | `core_machine_cpu_external_cycle_trace` collects a CPU external-cycle lifecycle, and `machine.c` publishes an external wait only after a Core lifecycle commit. | The generic `external_access_wait_windows` is an **additive** wait mechanism. Filling it with IBM's four/five clocks would double-count the Intel Table-2-21 instruction-form clock totals. Neither IBM nor the admitted reference implementations supplies an independently verifiable sub-instruction phase model. | Retain the existing CPU table owner and leave XT additive waits unselected. A physical bus-phase classifier is explicit L2, not a missing L3 implementation. |
| XT-B07 | Core PIT output 1 is already bound to the DMA refresh callback; DMA and PIT own their state. | XT topology selects one DMA controller, but no 5160 source ratio/cycle relation reaches the shared Core axis. The PC/AT refresh programming is explicitly a different board path. | Make the selected XT relation consume the same rational board axis and existing PIT/DMA owners; do not reuse PC/AT port-61/AT refresh policy. |
| XT-B08--B09 | `core_machine_set_cpu_bus_ready` is the single mutable Core input, gated by the immutable transaction contract; Core port/memory/transaction layers own addresses and accesses. | No XT board/card provider selects READY or classifies a channel transaction. A profile must not hold mutable READY state. | The same Core classifier accepts only a sourced board/card readiness input through the existing bounded operation; no VM shortcut or profile state mirror. |
| XT-B10--B11 | XT PPI owns port `60h`--`63h`, PPI fault inputs and the callback to `core_machine_xt_ppi_request_nmi`; Core owns the CPU NMI latch. | The source-backed XT PPI/NMI production path already exists. RTC `70h` NMI masking is a distinct PC/AT route and is absent from XT topology. | No board implementation change unless the dedicated 8255/PPI unit discovers an in-scope defect; retain this owner/path. |
| XT-B12 | The immutable resolver copies 256 KiB; Core owns RAM creation and all memory maps. | Selected XT memory construction is already present and proved by `vm-xt-5160-268-profile-smoke`. | Retain the sole profile-to-Core construction path; no board memory mirror. |
| XT-B13 | The XT BYOB provider validates caller bytes and passes one immutable `F0000h` mapping to Core firmware registration. | The documented map is present without bundled ROM bytes or a second mapping route. | Retain it; access-duration timing is not documented by this board source. |
| XT-B14 | Existing Core plan topology selects one PIC/one DMA/XT PPI, while VADP/FDC/HDC remain their own owners. | The board profile composes existing owners but its timing input is not yet a single qualified board contract. | The batch may supply only the board clock/cycle contract; it must not recreate PIC, DMA, PIT, PPI, FDC, CGA or Xebec state. |

## S4 Boundary

S4 is a no-code reconciliation, not a profile-only constant edit. Intel's
original wording makes Table-2-21 its instruction-form **clock-period** total;
the table already contains transfer terms. PCjs states that its cycle counts
are approximate and identifies prefetch/wait-state obstacles; the inspected
86Box 5160 board source does not provide a reusable phase model. These sources
therefore reject, rather than justify, an additive Core classifier.

S4 must prove that the existing CPU owner remains the sole clock-total owner,
the XT profile keeps additive waits unselected, all board construction/NMI/map
paths remain singular, and the unresolved physical bus phase is visibly L2.
Whole-machine physical pacing remains explicitly unclaimed.
