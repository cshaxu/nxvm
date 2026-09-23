# M6 T39 S2 PPU A12 Publication

## Repair

`core_ppu_cpu_write` now publishes the completed `$2006` PPU address level to
the existing Cartridge A12 operation. Pattern-table `$2007` writes now use the
same operation already used by pattern-table reads. PPU remains the only line
publisher and Cartridge remains the only MMC3 counter/IRQ owner.

## Direct Regression

`mynes.core.mapper4-smoke` programs an enabled zero-latch IRQ, completes a
`$2006` address at `$0000`, advances eight low PPU dots, then completes one at
`$1000`. The final address completion must assert the Cartridge IRQ. This is a
project-owned fixture and does not require an owner-local ROM.

## Verification

| Architecture | Product CTest result | Focused linkage |
| --- | --- | --- |
| x64 | 53 MyNes cases passed (the build tree's aggregate log ends at 428/428 because it includes repository cases). | `mynes.core.mapper4-smoke`, `mynes.core.ppu-smoke`, `mynes.integration.controller-rom-smoke` passed. |
| x86 | 53 MyNes cases passed (the build tree's aggregate log ends at 128/128 because it contains the MyNes-only configuration). | Same three tests passed after rebuilding Core hardware and the fixture. |

The repair implements the existing standard Mapper-004 filtered-A12 profile.
It does not claim subcycle-accurate behavior for every MMC3 chip revision,
MMC6, or every game; those are outside the admitted cartridge profile.
