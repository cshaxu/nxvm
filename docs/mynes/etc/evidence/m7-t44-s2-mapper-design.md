# M7 T44 S2 Mapper Optimization Design Audit

Owner accepts S1 and admits this reasonableness checkpoint on 2026-10-06.
Baseline is ac5861d90 and the qualified S1 0044 pair. This is a code-reading
audit, not a new performance measurement or implementation result. Current
owns the active packet; production, tests and binaries remain unchanged.
This describes the initial checkpoint. Owner subsequently approved this design
and implementation, including the reported CNROM validation repair; execution
and qualification results belong to the S2 implementation evidence.

## Per-Mapper Disposition

- NROM: its direct read and fixed 16 KiB mirror path are already simple. Leave
  them alone; do not introduce a general window-cache owner for them.
- MMC1: the current parser admits exactly 32 KiB PRG and 32 KiB CHR ROM.
  The read path repeatedly derives two PRG banks and eight 4 KiB CHR pages.
  Constants or masks valid for this existing finite profile can replace that
  work without extra persistent fields. Preserve register bytes, serial-write
  commit/reset behavior and all modes. No expanded MMC1 board support implied.
- UxROM: the parser admits exactly eight 16 KiB PRG banks and 8 KiB CHR RAM.
  Writes already normalize the selected bank after ROM bus conflict. The read
  modulo can use its verified eight-bank domain, including restored raw bank
  bytes, rather than requiring another stored window descriptor.
- CNROM: its read path already uses the selected CHR bank directly. No
  performance cache is justified. One through four CHR banks are accepted,
  including three, so replacing its write modulo with a bit mask is incorrect.
- MMC3: every PRG fetch currently selects the window/bank and performs runtime
  modulo; every CHR fetch selects the register, paired-bank half and modulo.
  This is the justified derived-window candidate: four 8 KiB PRG offsets and
  eight 1 KiB CHR offsets, lib_u32[4] plus lib_u32[8], 48 bytes. Current maximum
  PRG/CHR sizes fit these offsets. Integer offsets avoid pointer ownership and
  any on-disk/native-pointer ABI. Adoption still requires actual measurements.

Do not replace MMC3 modulo with a power-of-two mask: its admitted capacities
include non-power-of-two bank counts. Do not precompute decoded CHR pixels or
skip cartridge reads. PPU memory access currently calls A12 tracking before
the cartridge read; that order and every access must remain intact.

## Ownership And Rebuild Boundaries

The same cartridge owns source registers and derived offsets. One private
mapping builder handles construction, relevant bank-select/data writes and
validated snapshot candidates. No registry, plugin, lazy flag, generation
counter, public setter or duplicated legacy production route is needed.

Snapshot v3 explicitly serializes register bytes, not the whole C structure.
New offsets must be excluded from its stream and reconstructed from validated
registers plus immutable original sizes before committing a staged candidate.
core_snapshot_read currently starts by copying the live cartridge; without
reconstruction that would retain old offsets beside restored registers.
Failed loads must preserve live state. Valid snapshots keep their format.

core_machine_reset currently does not mutate cartridge mapping registers.
Consequently valid derived offsets remain valid through that reset; do not
invent an additional cartridge reset or clear registers for cache maintenance.
Reload/construction creates a fresh cartridge. All register-write paths live
in cartridge.c; the snapshot reader is the other production mutation path.

## Existing Safety Gap Found

snapshot_read_cartridge validates MMC1 ranges but not CNROM selected-bank bounds.
CNROM PPU reads index CHR as bank * 8192 + address without wrapping. With an
accepted three-bank (24 KiB) image, a malformed snapshot restoring bank 3 would
pass that validation and make the next CHR read index outside its allocation.
This is an existing source-confirmed bounds gap, not an S2 regression or a
measured crash. It must be reported and given a validation/failure-atomicity
regression disposition before code implementation; never mask it with a cache.

## Proof And Design Decision

Use owner-local table-driven fixtures to compare original address formulas
with candidate reads over window boundaries, register values, MMC3 PRG/CHR
modes, odd paired-bank values, non-power-of-two capacities and CHR RAM writes.
Cover construction, bank switches before reads, preserved mapping across reset,
snapshot restore to different banks and rejection of unsafe restored mapping.
Retain A12/IRQ tests and whole guest/frame/PCM comparisons through the existing
S1 fixed-processor probe; benchmark against the S1 converter, not pre-S1 code.

Full MyNES unit/integration qualification and both updated 0044 artifacts are
required only after runnable edits, not for this read-only audit checkpoint.
No runtime gain or net line-count reduction is promised before a prototype.
Recommend proceeding with this selective design, not a uniform all-mapper
cache framework; reject any candidate whose whole-frame gain does not justify
its state/reconstruction/code cost. Shared/NXVM and sibling paths are excluded.

Source queries: cartridge.c/h in full; machine.c reset; snapshot.c cartridge
read/write and staged commit; owned mapper/snapshot tests; rg for mmc1/mmc3 bank,
UxROM/CNROM register assignments and PPU A12/read call sites throughout MyNES.
