# M5 T31 Mapper 2 / UxROM Design

## Authority And Current-State Audit

The [NESdev UxROM record](https://www.nesdev.org/wiki/UxROM) defines Mapper 2
as a 16 KiB switchable CPU window at `$8000-$BFFF` and a final 16 KiB fixed
window at `$C000-$FFFF`; it has fixed board-wired horizontal or vertical
mirroring and a CPU write register spanning `$8000-$FFFF`. The [NESdev
submapper record](https://www.nesdev.org/wiki/NES_2.0_submappers) identifies
submapper 1 as no conflict and 2 as AND bus conflicts. The [bus-conflict
record](https://www.nesdev.org/wiki/Bus_conflict) explains that the effective
latch input is the bitwise AND of CPU data and ROM data.

T30 deliberately accepts only the fixed legacy iNES Mapper-2 shape (8 PRG
banks, no CHR-ROM), rejects NES 2.0 and currently returns
`LIB_STATUS_UNSUPPORTED` before publication. Current Core already has the
right ownership seams:

| Owner | Existing seam | T31 disposition |
| --- | --- | --- |
| Cartridge | CPU reads/writes, PPU pattern reads/writes and CIRAM mapping | Add a Mapper-2 branch and a private 16 KiB bank latch. |
| CPU bus | Forwards `$8000-$FFFF` reads/writes to Cartridge | No mapper policy added. |
| PPU | Delegates pattern memory and nametable mapping to Cartridge | No mapper policy added. |
| Media/Driver/App/Common/Lib | Format admission, publication and presentation | Unchanged. |

No owner ROM was read. Jackal remains an owner-local, ignored acceptance input.

## Fixed T31 Contract

For the one T30-admitted Mapper-2 profile:

- Reset/create selects PRG bank 0 at `$8000-$BFFF`; `$C000-$FFFF` always maps
  the final PRG bank (bank 7).
- Every CPU write in `$8000-$FFFF` samples the byte currently driven by its PRG
  address; the bank latch receives `cpu_value & rom_value`. The low resulting
  bank value selects the switchable 16 KiB bank modulo the finite bank count.
- The header's horizontal/vertical bit remains fixed: Mapper-2 writes do not
  change CIRAM mapping.
- Zero CHR-ROM retains Cartridge's existing allocated 8 KiB CHR-RAM. PPU
  reads/writes at `$0000-$1FFF` are direct and bank-free.
- Reads/writes outside Cartridge ranges preserve existing false/zero behavior.
- NES 2.0 submapper inputs remain rejected by T30. A later format expansion,
  if admitted, may distinguish no-conflict and AND variants; T31 does not
  silently reinterpret them.

## S Sequence

| S | Outcome | Estimated tracked surface |
| --- | --- | --- |
| S1 | Research, ownership audit, fixed legacy policy and complete implementation/test design. | 2 documentation files. |
| S2 | Enable the finite Mapper-2 constructor profile and implement CPU window/latch, conflict semantics, CHR-RAM and fixed-mirroring behavior. | Cartridge source/header and one dedicated Core test, about 3--5 files. |
| S3 | Prove all mapper behavior through Cartridge and CPU-bus fixtures; run ignored owner-local Jackal header/load scenario only when locally configured; rebuild, dual-architecture regression and evidence. | Core/integration tests, evidence and executable pair if production build changes, about 4--7 files. |
| S4 | Independent T31 closure audit. Any contract/evidence gap receives a corrective S before T32 admission. | Evidence/state only unless a defect is found. |

## Acceptance Matrix

| Required outcome | Synthetic proof receiver |
| --- | --- |
| Create/reset bank 0 and fixed bank 7 | Bank-labelled 128 KiB fixture reads `$8000`/`$C000` before and after writes. |
| Switchable 16 KiB bank | Writes select every finite bank and out-of-range values wrap modulo eight. |
| AND bus conflict | A write at a fixed-bank location uses the current ROM byte; a deliberately incompatible write shows the AND result rather than CPU value. |
| CHR-RAM | PPU write/read at both pattern-table halves persists directly. |
| Header mirroring | Horizontal and vertical fixtures retain their CIRAM relationship after mapper writes. |
| Bus routing | `core_bus_write` reaches the latch; an address below `$8000` cannot alter it. |
| Format/lifecycle | Existing T30 malformed/transactional checks remain green; owner-local Jackal load/run/pause/stop evidence is added only without tracking ROM bytes. |

The design deliberately does not claim Mapper 94/180, UOROM capacities beyond
the finite T30 profile, PRG-RAM, IRQ, audio, dynamic mirroring, NES 2.0
submappers or no-conflict variants.
