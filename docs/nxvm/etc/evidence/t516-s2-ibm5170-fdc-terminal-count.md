# T516 S2 IBM 5170 FDC Terminal-Count Repair

## Evidence

The owner-managed Intel 8272A data sheet, 1982 edition, page 7, states that
the controller transfers data until its TC input becomes active; completion of
the execution phase after Terminal Count enters the result phase.  Its EOT
definition is the final sector number, not an instruction to reject a command
before data transfer.

IBM 5170 Rev-3 BIOS source `DSKETTE.ASM` corroborates the observed workload:
`MD_TBL2` describes 360 KB media in a 1.2 MB drive as 9 sectors at 300 kbps,
while the normal 1.2 MB table uses 15 sectors at 500 kbps.  Its `SETUP_DBL`
uses Read ID and the physical returned cylinder to select double stepping.
The current Core Read ID result already returned that physical value, so no
profile or BIOS-specific detection path was added.

## Sole-Owner Repair

`core_machine_fdc_start_transfer()` no longer rejects Read/Write solely because
the provisional EOT exceeds the mounted medium geometry.  The existing FDC
execution route still rejects a sector that is actually reached but absent;
the existing DMA terminal callback ends a shorter transfer first.  Thus one
controller route now models the source-defined precedence:

`FDC sector service -> DMA TC -> FDC result/IRQ`

No profile logic, ROM behavior, media path, second geometry state, or test
media file was added.  The focused Core regression uses one-sector media with
EOT 15 and a 512-byte DMA request, and requires normal TC completion.

## Cross-Profile Result

All use their unchanged YAML-relative external input and VM-owned in-memory
overlay:

| Row | Result |
| --- | --- |
| IBM 5170 1.2 MB | `installer-ready` |
| IBM 5170 360 KB | `installer-ready` |
| default PC/AT 360 KB | `installer-ready` |
| IBM 5160 360 KB | `installer-ready` |

Focused `core-machine-fdc-smoke` and
`vm-ibm-5170-model-339-composition-smoke` pass.  The complete unit suite and
T-level integration suite remain S2/S4 closure obligations.  The complete
repository-only unit suite ran 302 `unit.*` cases with no failure record.
