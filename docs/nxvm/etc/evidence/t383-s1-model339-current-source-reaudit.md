# T383 S1: Model-339 Current-Source L3 Re-Audit

`M5:T383:S1:MODEL339-CURRENT-REAUDIT:OK`

## Decision

**Ready.** The frozen IBM PC/AT 5170 Model 339/Type 3 remains ready under the
declared deterministic event-and-bus L3 contract on the current runnable
source graph. This supersedes T379's readiness decision only for that same
bounded configuration: 8 MHz 80286, abstract Rev.3 BIOS slot, 512 KB planar
RAM, CGA, 101-key keyboard, 1.44 MB field-upgrade FDC, and no fixed disk.

## Current-Source Reconciliation

| Requirement row | T379-to-current change and current proof | Result |
| --- | --- | --- |
| Frozen profile identity and absence | The current descriptor still requires 80286, 512 KB, planar parity, CGA, abstract Rev.3 slot, 1.44 MB field upgrade, no HDC, no EGA and no fixed-disk CMOS type. The composition smoke verifies the selected descriptor, suppresses HDC ports/media, and preserves the separate generic ATA profile. | Accepted. |
| CPU, memory, ROM and deterministic bus | The 80286 retirement and logical bus evidence retained by T379 has no changed owner. T380 adds only a selected `00100000h..00ffffffh` absent-memory provider after A20: it returns `FFh`, discards writes and does not add RAM. The composition smoke proves the provider and retained 512 KB capacity. | Accepted; no electrical READY, wait-state or pin-phase claim. |
| FDC, DMA2, BIOS BDA and raw IMG | T380 moves the selected BIOS bounce segment from the generic `9fc0h` to profile-owned `7000h`, publishes 512 KB in the BDA, and accepts conventional media lacking optional address-mark metadata. The FDC/DMA smoke proves both generic and Model-339 paths, 64 KB DMA behavior, BDA `0200h`, page `07h` and sector result; current FDC topology/media-change and provider smokes pass. | Accepted within logical raw-IMG support. |
| PIT, PIC, RTC, CGA and reset | No current-source change invalidates the retained rational-clock, IRQ/reset, selected CGA topology or default evidence. The out-of-gate Model-339 clock-contract replay proves PIT, RTC, VADP, typematic and reset values; the current CGA topology smoke proves the selected CGA-only route. | Accepted; no monitor waveform, snow or ISA contention claim. |
| 8042, keyboard, IRQ1 and ingress | T380 enables the reset translation bit and makes host mapping query the attached native scan set. The current keyboard ingress and DOS keyboard smokes pass; the latter reaches its DOS input path using the owner-managed current media fixture. IRQ12/AUX remain outside this selected machine. | Accepted for the IRQ1-only 101-key route. |
| Planar-parity NMI and cancellation | No relevant owner change appears after T379. The existing selected planar-parity, NMI-mask, reset and deterministic cancellation proof remains applicable; the current full gate contains the related shared controller coverage. I/O-channel-check NMI remains unselected. | Accepted within the selected logical-NMI contract. |
| Product session/profile path | T381 changes Console session selection but preserves strict Model-339 override rejection and profile-owned composition. The current Console lifecycle smoke and session/profile coverage pass; the Model-339 composition smoke starts through the same production session path. | Accepted. |

## Fresh Verification

The clean `audit-current-head-winlibs3` GCC 16.1 build compiled the current
source successfully. Its serial current aggregate passed **251/251** in
87.77 seconds; the aggregate deadline and cleanup verifier emitted
`M5:T382:S1:CURRENT-GATE-AGGREGATE:OK`. The specialized verification target
then completed after the documentation packet was normalized to the repository
ASCII policy.

The current focused CTest replay passed 10/10: Model-339 composition, CGA
topology, firmware/FDC topology, FDC/DMA boundary, media provider, core FDC
topology and media-change ports, keyboard host ingress, Console lifecycle,
and DOS keyboard. Direct current binaries emitted:

```text
M5:T366:S5:MODEL339-COMPOSITION:OK
M5:T380:S2:MODEL339-NO-XMS-PROBE:OK
M5:T348:S4:FDC-DMA-BOUNDARY:OK
M5:T380:S2:MODEL339-512K-FDC-START:OK
M5:T366:S6:MODEL339-CGA-TOPOLOGY:OK
M5:T375:S15:MODEL339-REV3-CGA-DEFAULTS:OK
M5:T366:S7:MODEL339-FIRMWARE-FDC-TOPOLOGY:OK
M5:T375:S2:MODEL339-CLOCK-CONTRACT:OK
M5:T375:S13:MODEL339-CGA-REFERENCE-CONTRACT:OK
M5:T375:S22:MODEL339-TYPEMATIC:OK
M5:T375:S23:KBC-F3-CADENCE:OK
```

`160a34e1..HEAD` was reviewed across the profile descriptor/composition,
firmware, FDC, KBC/input, memory/A20, session/profile, Console and CTest
registrations. The only selected-contract changes are the T380 repairs and
their current proofs above; T381 and T382 change session and quality boundaries
without changing the selected device contract.

## Boundary And Transfers

This decision does not claim physical waveform or pin timing, ISA contention,
CGA snow or monitor output, flux/index/weak-bit/physical CHRN behavior,
fixed-disk MFM/ST-506 or ATA behavior, generic PC/AT support, EGA/VGA/AUX
support, ROM/media provenance, or a DeskPro/XT result. Those boundaries retain
their existing Queue/TODO receivers. No new selected-contract defect was found,
so T383 creates no additional implementation receiver.
