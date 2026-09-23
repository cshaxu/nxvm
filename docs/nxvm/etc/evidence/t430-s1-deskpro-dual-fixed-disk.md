# T430 S1 DeskPro Dual Fixed-Disk Selection

## Evidence Tier

The selected-machine C: and D: fixed-disk capability is an original-source
fact from the retained DeskPro 386 Volume II fixed-disk material. The
WD1003-IWH controller behavior is a controller-matched secondary reference:
it supplies the WD task-file family, including normal CHS access and the
absence of ATA Identify. No firmware, guest medium, source text, or external
code is imported.

## Implemented Contract

`core_machine_hdc_topology` now carries immutable primary and optional slave
media identifiers. The existing Drive/Head select bit chooses one of those
identifiers only for the Compaq WD 40 MB personality. It does not modify the
generic ATA master, LBA, or Identify paths. Core retains controller, port,
status, IRQ14, reset, and media-operation ownership; Model-40 composition
binds two VM-owned `t_hdd` providers and retains their paths and geometry.

The second Model-40 image is optional. The default remains one 925/5/17 RAW
40 MB image. If supplied, the second image is checked against the same
documented capacity and bound before the media registry freezes.

## Verification

- `core-machine-compaq-hdc-s5-smoke` reads different synthetic words after
  primary and slave Drive/Head selection and retains the existing IRQ14 path.
- `vm-model40-hdc-s26-smoke` loads two synthetic 925/5/17 images, proves the
  frozen secondary slot, and reads isolated C:/D: data through the full VM to
  Core topology.
- Full MinGW build succeeds. The focused regressions print
  `M5:T430:S1:COMPAQ-HDC-DUAL-DRIVE:OK` and
  `M5:T430:S1:MODEL40-HDC-DUAL-DRIVE:OK`.

## Retained Boundary

This establishes functional drive selection only. Long/Format/ECC/recovery,
physical-sector identity, drive service duration, DRQ/IRQ waveforms, firmware
setup behavior, and physical/L3 acceptance remain outside this task.
## Incidental Regression Repair

The serial current gate exposed an obsolete assertion in `vm_model40_d4_compatibility_s25_smoke`: its synthetic reset vector contains `HLT`, yet it required zero retired instructions. The adjacent Model-40 ROM layout regression already proves the same reset-vector `HLT` retires exactly one instruction before `CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT`. The D4 test now uses that shared CPU contract; no CPU or D4 implementation changed.
