# T484 S4 IBM PC/XT 5160-268 Board Topology And Gap Ledger

`M5:T484:S4:XT-BOARD-TOPOLOGY:OK`

`M5:T484:S4:XT-B2-GAP-LEDGER:OK`

## Method And Boundary

This is the B2 construction audit required by the active packet.  It consumes
the accepted IBM source facts in T483 S1--S3 and T484 S1--S3, then compares
them against the current Core plan and VM construction sources.  It imports no
firmware, media, third-party source or local path.

The only admissible construction path remains:

`immutable 5160-268 declaration -> copied board plan -> Core sole state owner -> copied observation`.

`Direct` means that the existing owner can consume the selected XT fact after
the board declaration is made.  `Gap` means that the owner exists but its
current construction contract prevents the fact being selected.  Neither term
permits borrowing a PC/AT descriptor or holding guest state in VM.

## Frozen B2 Topology

| Relation | Selected source-backed fact | Current sole owner and audited disposition | Earliest receiver; prohibited shortcut |
| --- | --- | --- | --- |
| CPU/board | 8088 maximum-mode attachment; 4.77 MHz derives from 14.31818 MHz / 3. | S3 supplies the distinct Core 8088 profile and four-byte queue.  Its current `ticks_per_instruction = 1` is a deterministic construction value, not a board clock. | B2 consumes the CPU profile only.  XT timing later consumes cycle formulas; no host tick injection or 8086 alias. |
| RAM/reset ROM | 256 KiB installed RAM; two external 32 KiB ROM devices; owner supplies ROM material. | Core already creates any nonzero RAM size, including 256 KiB, and owns immutable maps/reset entry.  The 2 MiB constant constrains only stopped-state reconfiguration.  The actual gap is that no 5160 declaration materializes the existing low-memory and external-ROM capabilities. | B2 publishes the sourced 256 KiB construction value and one BYOB ROM-pair validation/mapping route.  Do not allocate 2 MiB and call it 256 KiB, embed a ROM, or add a reset path. |
| PIC | One 8259A; IRQ0 timer and IRQ1 keyboard; no slave PIC. | Core owns PIC state, but `core_machine_create_internal` always initializes and ports both master and slave (`20h/21h`, `A0h/A1h`). | B2 adds immutable single-versus-cascaded PIC topology at the Core plan boundary.  Do not merely leave the slave uninitialized or hide its ports in VM. |
| PIT | 8254 input 1.193182 MHz; channel 0 -> IRQ0; channel 1 -> refresh/DMA request. | Core owns the primary PIT; `core_machine_configure_dma` already binds channel 1 to the Core refresh request. | Direct reusable state path once B2 selects XT board topology.  The source clock/phase formula transfers to XT timing; do not use an AT auxiliary PIT. |
| DMA/refresh | One 8237A: three external channels plus channel 1 refresh; 8-bit channel; AEN is board ownership. | Core owns DMA and refresh binding, but `core_machine_dma_wiring_is_valid` requires two controllers and cascade channel 4. | B2 adds an immutable one-controller DMA topology and leaves all DMA register/state ownership in Core.  Do not configure the PC/AT cascade and call it XT. |
| NMI/reset/port B | Parity NMI and `RESET DRV` are board facts. | Core owns reset and board-side effects, but available planar parity configuration is explicitly PC/AT port-B semantics. | B2 adds a source-qualified XT board port/reset/parity declaration, or transfers any unavailable exact relation.  Do not reuse PC/AT planar parity by port number alone. |
| Keyboard | Enhanced keyboard is selected; system board uses the XT serial keyboard interface; AUX mouse is absent. | Core KBC owns keyboard delivery, but current initialization binds AT 8042 services and the `kbc_aux_absent` flag only removes AUX; it does not create XT/PPI serial semantics. | B3 creates one source-qualified XT keyboard-board binding that publishes through Core input/PIC state.  Do not treat keyboard-only 8042 as an XT implementation. |
| FDC | One half-height drive; the catalogue does not select drive geometry or service timing. | Core owns 8272A/FDC/media state.  Session materialization currently reads fixed PC/AT profile leaves and expects its FDC route. | B3 supplies one source-qualified XT drive declaration and the existing Core FDC receives the copied IRQ/DRQ configuration.  Do not substitute 1.44 MB AT media. |
| CGA | IBM CGA is the selected adapter; aperture `B8000h`; no adapter ROM is selected. | VADP is sole register/VRAM/snapshot owner.  Core display configuration already exposes CGA VRAM; current `session.c` obtains all port leaves from PC/AT descriptor. | B4 passes the selected CGA CRTC/aperture declaration through the same copied plan.  VM/firmware cannot mirror video state. |
| Xebec fixed disk | Optional IBM adapter: byte stream `320h`--`323h`, DMA3, IRQ5, option ROM `C8000h`--`C9FFFh`; factory 5160-268 has no disk. | Core HDC owns existing ATA, Compaq WD and IBM WD1003 personalities only; its public config is task-file shaped. | B5 replaces that shape for one explicit Xebec Core personality and one BYOB geometry/media declaration.  Do not create an ATA shim, option-ROM bytes, duplicate CHS state or a second media cache. |
| Absent devices | No RTC/CMOS, second PIC, 16-bit AT channel or PS/2 AUX. | Core can host independent PC/AT capabilities, while `vm_session_storage_initialize` currently requires CMOS leaves, IRQ8 and a dual-DMA descriptor. | B2 factors construction so an XT declaration omits these devices.  Do not create disabled AT devices or route XT through the default PC/AT descriptor. |
| Product/session | A fixed selected profile is usable only with owner-supplied BYOB resources; no generic XT variants. | S3 resolver declaration is intentionally available but `vm_session_create` returns `TYPE_STATUS_UNSUPPORTED` before B2. | B6 exposes the completed fixed plan through existing typed request authority.  Do not re-encode profile data as CLI text or permit mutable CPU/RAM/device selection. |

## Minimal Construction Repair Order

1. **B2 board-plan seam:** make the already copied Core plan express only the
   selected one-versus-two PIC/DMA, absent RTC, existing low-memory/ROM, and board
   reset/NMI choices.  Move the PC/AT-only leaf collection out of the generic
   session construction route.  This is one removal of an accidental PC/AT
   dependency, not a second XT composition path.
2. **B2 selected shared routes:** bind primary PIT/refresh, eight-bit bus
   relationships and the selected CGA configuration to the same plan.  The
   named Core owners retain all mutable state.
3. **B3--B6 device work:** add the XT keyboard binding, source-qualified FDC
   drive declaration, explicit Xebec personality/BYOB media and finally the
   typed product integration, each through the B2 plan.  These batches may not
   bypass it.

## Timing And Availability Partition

The IBM clock, normal four-clock bus cycle and selected five-clock I/O facts
remain Manual-L3 inputs to the later XT board/device timing candidate.  They do
not make the current deterministic Core elapsed axis physical or enable
host-paced Standard mode.  T484 B2 therefore materializes no host-time writer,
service-duration estimate, or pace claim.  Until the later timing receiver
closes those facts onto the common Core axis, the existing Standard/Turbo
policy remains unchanged.

## Audit Conclusion

The 5160 declaration is correctly unavailable today: it has the B1 CPU
identity but none of the B2 board assembly.  The resulting gaps are finite and
have one receiver: B2 owns low RAM/ROM, single PIC, one DMA, absent RTC and
board reset/NMI selection; B3 owns XT input/FDC bindings; B4 owns existing CGA
selection; B5 owns Xebec; B6 owns product exposure.  No row needs a generic XT
framework, a parallel controller, or an AT compatibility path.
