# T353 S4: Selected PC/AT Composition Closure

## Ownership And Reset Path

S4 consumes the exact directional leaf and named-route contract accepted by
T353 S2. The default profile remains the sole declarative source; VM session
composition validates it before core publication, binds the selected firmware
and device owners, and freezes core registration. Core remains the sole owner
of port dispatch, controller state, IRQ/DMA requests, reset, and deterministic
timeline service.

The exercised reset path is:

```text
vm_session_reset
  -> execution-context reset callback
  -> core_machine_reset / cold reset
  -> port, memory, KBC, DMA, RTC, FDC, HDC, PIC, PIT, and VADP reset
  -> timeline and all device clock domains reset
  -> three due-event chains rearmed at tick 1
  -> VM device reset and profile-firmware reset callbacks
```

This is a composition proof, not a second reset implementation. It neither
adds a generic port wrapper nor reconfigures a device after construction.

## Focused Proof

`current.vm-pcat-composition-s4-smoke` creates an ordinary default session and
checks the complete profile surface both at initial construction and after
session reset:

| Boundary | Proof |
| --- | --- |
| Exact selected topology | Every one of the 75 profile leaves has matching frozen core read/write registration. All five named bindings agree with the actual PIT IRQ0, KBC IRQ1/AUX IRQ12, CMOS IRQ8, FDC IRQ6/DMA2, and ATA IRQ14 state. |
| Sparse/absent boundary | Ports `61h`--`63h`, `3D6h`--`3D7h`, and `3F3h` remain unregistered in both states. Thus session reset cannot convert an envelope into a provider claim. |
| Firmware-visible reset | The session reset vector is `F000:FFF0` before and after reset. |
| Deterministic L3 rearm | A real instruction advances the core timeline; following session reset, `now` returns to zero and exactly the three ordered core chains are pending with reset sequence state. |
| Selected mutable reset state | The CMOS index-port write masks NMI through the selected adapter before reset. Reset clears that mask while preserving the complete validated topology. |

The focused test is deliberately media-free and uses only committed profile and
firmware material. It does not use Windows or a host clock as an oracle.

## Similar-Issue And Transfer Closure

The S4 sweep covers session initialization, profile validation, controller
configuration, execution-provider reset, provider lifecycle reset/finalize,
profile-firmware reset, core cold reset, and timeline rearm. No selected
composition defect was reproduced, so no production path changed.

Port-`61h` PPI/speaker, parity/I/O-channel NMI, serial, parallel, and game
interfaces retain the S3/TODO admission conditions. Physical bus wait states,
INTA waveforms, DMA/refresh contention, and controller-service durations
remain the next L3 bus-timing package; this selected-device closure does not
claim cycle-exact timing.

Promotion: retain with T353 history at task closure.
