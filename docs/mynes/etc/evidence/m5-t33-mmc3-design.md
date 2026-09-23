# M5 T33 Complete Profiled MMC3 Design

The [NESdev MMC3 record](https://www.nesdev.org/wiki/MMC3) fixes this task's
standard Mapper-004 profile: up to 512 KiB PRG, up to 256 KiB CHR or 8 KiB
CHR-RAM, optional 8 KiB PRG-RAM, all bank-register modes, H/V mirroring,
qualified A12 IRQ and `$A001` RAM enable/write protection. MMC6 and NES 2.0
variants remain format rejections.

Current code already has bank registers, both PRG/CHR modes, mirroring and
A12 IRQ, but admits only the SMB2 8/16 ROM shape and lacks PRG-RAM and robust
CHR-RAM behavior. T33 retains Cartridge as the owner; Bus gains only the
existing neutral `$6000-$7FFF` forwarding where required, and PPU stays a
pattern/mirroring forwarder.

| S | Outcome |
| --- | --- |
| S1 | Authority audit, finite standard profile and gap/design ledger. |
| S2 | Expand constructor and Mapper-4 state/mapping: capacity edges, PRG-RAM enable/protection, CHR-RAM and full register fixtures. |
| S3 | Validate A12/IRQ timing and product lifecycle using ignored SMB3/TMNT3 probes when configured; rebuild and full dual-architecture regression. |
| S4 | Independent closure audit; corrective S if any standard-profile claim lacks direct evidence. |

Fixtures must cover register pairs 0--7, PRG mode, CHR inversion, fixed banks,
RAM enable/protection, mirroring, all capacity corners and qualified IRQ
transitions. Owner-local SMB3/TMNT3 evidence remains optional and records no
ROM path or bytes.
