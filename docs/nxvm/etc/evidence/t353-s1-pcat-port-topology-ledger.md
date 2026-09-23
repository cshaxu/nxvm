# T353 S1: PC/AT Selected Port-Topology, Route, And Lifecycle Ledger

## Scope And Sources

This ledger reconciles the selected default PC/AT machine only.  It reads the
default profile descriptor, VM-session composition, core registration and
lifecycle owners, and the retained controller proofs.  The primary board
reference is the [IBM PC/AT Technical Reference](https://minuszerodegrees.net/manuals/IBM_5170_Technical_Reference_6280070_SEP85.pdf).
The [Intel 8237A data sheet](https://www.pcjs.org/documents/datasheets/intel/INTEL_8237A_DMA.pdf)
is the primary DMA-register reference.  Bochs and PCjs remain question sources
only; neither source, firmware, configuration, or behavior is imported.

`port_range` currently describes an address envelope.  It does **not** encode
direction, sparse leaves, secondary-controller/page leaves, lifecycle owner,
or an implicit optional device.  Treating it as an exact registration list is
the structural ambiguity this task must remove.

## Selected Topology Ledger

| Surface | Profile/composition declaration | Actual registration and lifecycle owner | Route and existing proof | S1 disposition |
| --- | --- | --- | --- | --- |
| Master/slave 8259A, `20h-21h`, `A0h-A1h` | Two PIC windows in `default_pc_at_port_ranges` | `core_machine_pic_initialize`; reset/finalize in `machine.c` | No producer route belongs to the PIC itself; T349 PIC lifecycle/OCW smokes | Exact selected topology; S2 records directional leaves, no behavior repair. |
| 8254 PIT, `40h-43h` | One PIT address window | `core_machine_pit_initialize`; reads `40h-42h`, writes `40h-43h`; reset/finalize in `machine.c` | Profile route IRQ0; T350/T346 PIT and timeline proof | Exact window but asymmetric direction; S2 makes that direction explicit. |
| Primary 8237A, `00h-0Fh` | One DMA range only | `core_machine_dma_initialize` registers primary command leaves, then page leaves `81h/82h/83h/87h/89h/8Ah/8Bh/8Fh` and secondary controller `C0h-CFh` even leaves | FDC consumes channel 2 through `core_machine_configure_dma`; T348 DMA layout/transaction evidence | **Topology declaration gap.**  The profile names only the primary window; S2 must represent the complete selected dual-DMA surface and its sparse/directional leaves. |
| 8042 KBC, `60h-64h` | One KBC range and IRQ1 route | `core_machine_kbc_register_ports` owns `60h` and `64h` only; core reset/finalize and KBC timeline owner | T351 controller/keyboard/AUX evidence; IRQ12 AUX extension remains bounded debt | **Envelope ambiguity.** `61h-63h` are not KBC providers. S2 must stop treating a contiguous range as an exact registration claim; it must not claim port `61h` PPI/speaker support. |
| CMOS/RTC, `70h-71h` | Exact range and IRQ8 route | Session builds `core_machine_rtc_cmos_config`; `core_machine_configure_rtc_cmos` installs index-write/data-read-write; core owns reset/finalize/timeline | T350 RTC/CMOS/IRQ8 proof | Exact selected topology; S2 records direction and composition receiver. |
| Fast A20/system control, `92h` | No profile declaration | `core_machine_memory_register_ports` registers read/write; memory owner maps A20/reset semantics; core reset/finalize owns RAM state | T346 memory/A20/ROM evidence and T351 KBC output-port A20/reset proof | **Topology declaration gap.** S2 must declare this selected built-in system-control leaf with its memory owner, without duplicating KBC state or inventing broader chipset support. |
| VADP attribute `3C0h-3C1h`, sequencer `3C4h-3C5h`, graphics `3CEh-3CFh` | Exact windows, consumed by session display config | `core_machine_vadp_initialize`; VADP owns registers/apertures/reset/capture and timeline advance | T352 register/CGA/EGA/composition proof | Exact selected topology; S2 records read/write asymmetry, no VADP expansion. |
| VADP CRTC/mode/status, `3D4h-3DAh` | One contiguous VADP range | VADP registers `3D4h/3D5h`, `3D8h/3D9h`, and status read `3DAh`; `3D6h/3D7h` are not providers | T352 selected CRTC/mode/status proof | **Envelope ambiguity.** S2 must represent sparse selected leaves and directions; it must not infer broad CRTC/CGA/EGA compatibility. |
| FDC, `3F2h-3F7h` | One range, IRQ6/DMA2 route | Session derives topology from range/route and `core_machine_configure_fdc` installs DOR/status/data/direction-control leaves; core owns reset/finalize and T346 readiness tick | T347/T348 FDC service/DMA evidence | **Envelope ambiguity.** S2 must encode the selected sparse directional leaves, not claim `3F3h/3F6h` provider behavior. |
| ATA/HDC, `1F0h-1F7h` and `3F6h` | Separate `hdc_pio` descriptor, IRQ14/no-DMA/16-bit-data fields; absent from `port_ranges` | Session validates and materializes `hdc_pio`; `core_machine_configure_hdc` atomically registers command block and alternate-status/device-control leaf; core owns reset/finalize/readiness refresh | T347 ATA PIO service and controller authority proof | **Split declaration gap.** The independent descriptor is intentional for ATA widths/features, but its port leaves must join the same explicit topology ledger in S2 rather than remain invisible to range-based verification. |

## Composition, Lifecycle, And Firmware Visibility

`core_machine_create_internal()` owns base registration in this order: memory
`92h`, VADP, KBC, dual DMA, dual PIC, and PIT.  Session composition then derives
VADP, DMA/FDC, CMOS, and media/controller bindings from the profile before it
freezes providers.  FDC and HDC are separately configured only through
`vm_session_machine_devices_configure_controllers()`.  The cold-reset owner
resets CPU/FPU, port dispatcher, RAM, KBC, DMA, configured RTC, FDC, HDC, PIC,
PIT, VADP, and the one timeline; destruction finalizes the inverse controller
state before port/memory storage.

The profile's firmware-service list is therefore a consumer map, not a second
port registry: INT 10h consumes VADP, POST/IRQ0/INT1Ah consume PIT, POST/IRQ1/
INT16h consume KBC, CMOS POST consumes RTC, FDC POST/IRQ6/INT13h/INT40h consume
FDC, and HDC INT13h consumes ATA.  T353 does not create firmware behavior from
that list.

## Required S2--S4 Receivers

| Receiver | Bounded work | Explicit exclusions |
| --- | --- | --- |
| S2 | Replace the overloaded range-only assumption with one selected profile-topology declaration that can express exact/sparse leaves, read/write direction, IRQ/DMA, and the existing ATA feature fields; make composition validate it before registration.  Cover all rows above, including DMA secondary/page ports and `92h`. | No generic provider framework, changed port ABI, hardware expansion, or controller semantics. |
| S3 | No selected implementation is admitted by this ledger. Retain port `61h` PPI/speaker under its existing corpus-gated TODO; retain serial, parallel, game, and advanced system-control breadth under their named TODOs. | No placeholder provider, host passthrough, fabricated IRQ/NMI source, or Windows claim. |
| S4 | Build a session-level topology proof that checks profile materialization, selected registration, IRQ/DMA bindings, freeze/reset/finalize and firmware-visible controller consumers; build the T353 artifact and close only the selected graph. | No physical bus duration, ISA arbitration, controller service-time, or cycle-exact claim; those remain the following L3 bus-timing candidate. |

## Similar-Issue Sweep And Result

The sweep searched profile port/routing fields, session device materialization,
core registration (`core_machine_*_initialize`, `configure_*`, and
`core_machine_memory_register_ports`), reset/finalize, firmware services, and
the retained port/controller/DMA/RTC tests.  It found no duplicate registered
provider owner or reproduced runtime transaction failure.  It found the three
declaration-model gaps above: incomplete dual-DMA coverage, invisible `92h`,
and range envelopes that conceal sparse/directional registration; ATA is a
fourth representation split, not a second ATA owner.  S2 owns the common
declaration/validation mechanism so they are repaired holistically rather than
as isolated address additions.

Promotion: retain through T353 closure, then merge the final topology and
transfers into the T353 history record.
