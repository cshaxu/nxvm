# T374 S1: Model-339 Selected-Device Functional Owner Inventory

## Boundary And Method

This inventory converts T373's selected Model-339 partial rows into bounded
source-owner and test evidence. It does not judge a surviving code path
complete: a focused smoke proves only its named observable boundary. The
inventory excludes ATA/HDC, MFM/ST-506, EGA/VGA and AUX because the selected
descriptor either suppresses them or T373 assigns them elsewhere.

Static sweep scope: `src/core/machine`, `src/vm/profile/default_profile`,
`src/vm/composition/session`, `tests/machine`, `tests/core`, and
`CMakeLists.txt`. It used the descriptor/route symbols and device-owner names
listed in the T374 S1 packet. The relevant Model-339 descriptor is
`vm_profile_ibm_5170_model_339_descriptor_get()` in
`src/vm/profile/default_profile/pc_at_profile.c`; composition is selected by
`VM_SESSION_PROFILE_IBM_5170_MODEL_339`.

## Selected Functional Matrix

| Selected capability | Production owner, observable route and consumer | Existing focused proof | Functional gap for later T374 repair | Timing-only handoff |
| --- | --- | --- | --- | --- |
| Profile, 80286 contract, 512 KB and abstract Rev.3 ROM slot | `pc_at_profile.{c,h}` supplies the descriptor: 80286, 512 KB, `IBM_5170_REV3_ABSTRACT`, no HDC, planar parity and field-upgrade FDD. `session.c` composes its core machine. The generated profile firmware remains a consumer, not an external IBM ROM. | `vm-ibm-5170-model-339-composition-smoke` proves descriptor selection, 80286, 512 KB, no HDC port/HDD insertion and planar parity configuration. | Validate the selected reset/ROM-visible and firmware-service functional contract without treating the abstract slot as vendor-ROM execution. | ROM fetch/bus availability and board reset phase transfer to 5170 timing. |
| Memory control, planar parity, reset and NMI | `core/machine/{memory,rom_mapping_interface,machine}.{c,h}` own mapping/reset; `core_machine_configure_planar_parity`, NMI mask access and selected reset are in `machine.c`; `session.c` binds profile topology. | `core-machine-immutable-rom-mapping-smoke`, `core-machine-memory-reconfigure-smoke`, `core-machine-planar-parity-nmi-s3-smoke`, `vm-pcat-composition-s4-smoke`. | Audit selected parity latch/status/clear, reset-visible memory state and consumer behavior; T365's missing independently selected I/O-check source must stay unselected. | NMI delivery and memory/bus service phase transfer to 5170 timing. |
| PIC and PIT | `core/machine/{pic,pit,machine}.{c,h}` own command/register state, IRQ selection and reset; descriptor supplies 20h/21h/A0h/A1h and 40h--43h port leaves plus PIT IRQ0 route; generated firmware consumes IRQ0/INT 1Ah. | `core-machine-pic-irq-lifecycle-smoke`, `core-machine-pic-command-priority-smoke`, `core-machine-pic-ocw3-smoke`, `core-machine-pic-lifecycle-s4-smoke`, `core-machine-pit-readback-smoke`, `core-machine-pit-waveform-smoke`, `core-machine-pit-irq0-s2-smoke`. | Establish a Model-339 functional matrix for command/error/reset/acknowledgement behavior; existing unit boundaries do not establish all selected controller states as a profile contract. | IRQ arbitration, PIT cadence and board phase transfer to 5170 timing. |
| DMA and RTC/CMOS | `core/machine/{dma,rtc,machine}.{c,h}` own DMA/CMOS state and reset; `session.c` configures DMA and RTC; the descriptor supplies DMA leaves and CMOS 70h/71h, plus CMOS IRQ8 route. | `core-machine-dma-channel-smoke`, `core-machine-dma-binding-token-smoke`, `core-machine-dma-rtc-authority-smoke`, `core-machine-rtc-smoke`, `core-machine-rtc-cmos-s3-smoke`, `core-machine-rtc-storage-s4-smoke`. | Validate selected channel/command/error/reset/IRQ-visible behavior and firmware consumer path without importing an IBM ROM. | DMA grant/service and RTC cadence/IRQ phase transfer to 5170 timing. |
| 8042 and 101-key AT keyboard, IRQ1 | `core/machine/{kbc,machine}.{c,h}` owns controller bytes, output path, reset and IRQ source; `keyboard_mapper` and platform mapper submit scan sequences; descriptor maps 60h/64h and IRQ1. Firmware consumes IRQ1/INT 16h. | `core-machine-kbc-controller-smoke`, `vm-keyboard-set1-mapper-smoke`, `vm-pcat-topology-s2-smoke`. The AUX-specific smokes are generic-only evidence and excluded. | Define selected keyboard controller command/status/buffer/error/reset/IRQ1 matrix and prove it without AUX/IRQ12 scope. | Command response/typematic/service phase transfer to 5170 timing. |
| FDC and aftermarket 1.44 MB drive compatibility, IRQ6/DMA2 | `core/machine/fdc.{c,h}` owns FDC state; `core_machine_configure_fdc` and `vm_session_machine_devices_configure_fdc` bind DOR/MSR/data/control and the descriptor's IRQ6/DMA2 route. `vm_machine_fdd_*` owns host media lifecycle; firmware consumes POST, IRQ6, INT 13h and INT 40h. | `vm-ibm-5170-model-339-firmware-fdc-topology-smoke`, `core-machine-fdc-smoke`, `core-machine-fdc-topology-port-smoke`, `core-machine-fdc-media-change-port-smoke`, `vm-fdc-port-smoke`, `vm-fdc-dma-boundary-smoke`, `vm-fdc-authority-smoke`, `vm-fdc-read-track-dos-smoke`. | Define the selected controller/drive command, result, error, media-change, reset/cancel, DRQ/DMA2 and IRQ6 contract. Preserve `diskette_drive_a_field_upgrade`; never call it a factory 339 drive. | Command completion, DRQ/IRQ service, drive rotation and DMA availability transfer to 5170 timing. |
| CGA digital display | `core/machine/vadp.{c,h}` owns CRTC/mode/status and display snapshots; descriptor retains CGA VRAM and 3D4h/3D5h/3D8h/3D9h/3DAh leaves while composition rejects EGA leaves. Firmware consumes INT 10h. | `vm-ibm-5170-model-339-cga-topology-smoke`, `core-machine-vadp-text-smoke`, `core-machine-vadp-text-status-smoke`, `core-machine-cga-graphics-port-smoke`, `core-machine-cga-640-port-smoke`, `vm-cga-graphics-system-smoke`, `vm-cga-640-system-smoke`. | Define every retained digital CGA register/mode/status/reset/snapshot contract; do not count EGA tests or composite output. | Frame cadence, bus contention and display phase transfer to 5170 timing. |

## Exclusion And Ownership Sweep

`ibm_5170_model_339_descriptor` has `hdc_present == false`, no HDC firmware
hook and zero HDC ports; the Model-339 composition smoke verifies those
properties. ATA/HDC therefore remains solely current-product work and is not a
T374 repair target. The descriptor's `ega_present == false`; the Model-339 CGA
topology smoke asserts that EGA port/configuration paths are unavailable. AUX
is a generic descriptor route and remains current-product work. IBM MFM/ST-506
is neither descriptor nor core owner and remains the explicit TODO admission.

## Focused Existing-Binary Replay

The repository's existing `build/mingw-gcc-x64` binaries were replayed without
media insertion or mutation. They reported success for:

```text
M5:T366:S5:MODEL339-COMPOSITION:OK
M5:T366:S6:MODEL339-CGA-TOPOLOGY:OK
M5:T366:S7:MODEL339-FIRMWARE-FDC-TOPOLOGY:OK
M5:T283:S2:CORE-FDC-MEDIA:OK
M5:T347:S2:FDC-SERVICE:OK
M5:T227:S3:KBC-CONTROLLER:OK
M5:T349:S4:PIC-LIFECYCLE:OK
M5:T191:S2:PIT-READBACK:OK
M5:T230:S3:DMA-CHANNEL:OK
M5:T350:S3:RTC-CMOS:OK
M5:T366:S4:PLANAR-MEMORY-PARITY:OK
```

`cmake` is not on this shell's command path, so these are replay results of
existing binaries rather than a fresh build proof. They are adequate for the
inventory only and do not certify subsequent source changes.

## Next Repair Boundary

S2 must convert this inventory into one or more small, owner-local functional
repairs with fresh builds and regressions. It must first choose a concrete
reproducer from the selected matrices; a broad controller rewrite or a timing
value is not admitted by this inventory.
