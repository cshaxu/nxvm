# M5 T430: DeskPro Dual Fixed-Disk Selection

## Closure

T430 S1 closes the bounded DeskPro Compaq WD 40 MB C:/D: functional receiver.
The Core HDC topology can carry immutable primary and optional secondary media
identifiers. Only the Compaq personality selects the secondary identifier from
the existing Drive/Head task-file bit. ATA master-only, LBA, and Identify
behavior remain unchanged.

Model-40 composition retains VM ownership of both HDD backings. Its original
single-drive startup default is unchanged; an optional second 925/5/17 RAW
image is validated and bound before the registry freezes. No firmware, guest
media, or external code was imported.

## Evidence And Verification

- Original retained DeskPro fixed-disk material establishes C:/D: capability.
- WD1003-IWH is used only as a controller-matched secondary reference.
- Core and full Model-40 synthetic tests prove isolated C:/D: words,
  drive selection, IRQ14 behavior, and reset.
- `vm_model40_d4_compatibility_s25_smoke` had an obsolete zero-retirement
  expectation for its reset-vector HLT. It now matches the adjacent Model-40
  ROM test: HLT retires one instruction and then waits for interrupt.
- Focused CTest, documentation governance, whitespace validation, and the
  serial 289-test current gate pass.

## Retained Boundary

This is a functional selection result, not physical/L3 acceptance. Controller
service durations, DRQ/IRQ waveforms, Long/Format/ECC/recovery, physical
sector identity, firmware setup behavior, and remaining DeskPro physical-cycle
work stay transferred to the active physical-cycle queue program.