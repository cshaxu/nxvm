# T468 S4 ATA Write Media Closure

## Change

ATA-T3 had one concrete implementation defect: `WRITE SECTORS` entered the
PIO data-out phase by calling the read helper, so an otherwise valid writable
destination failed whenever its provider could not read the old sector.

`hdc.c` now resolves the selected CHS or LBA logical sector once through one
owner-local helper. It queries and validates the medium, writable capability,
address form, geometry and offset. Read consumes that result with exactly one
media read; write completion consumes it with exactly one media write; write
admission consumes it without a media transfer. The existing later completion
validation remains necessary because provider state can change while the host
supplies a sector. No cache, shadow address state, second write route or
provider API was introduced.

The replacement deletes 71 tracked `hdc.c` lines and adds 13 tracked test
lines: source is 27 added / 98 removed and the Core smoke is 15 added / 2
removed, for 42 added / 100 removed (-58) across the changed source/test
paths. The retained production path is `hdc.c` -> existing media provider.

## Proof

- ATA-3 printed page 48 defines the 512-byte PIO data-out transfer; printed
  pages 106--110 define the PIO command procedure. Neither permits a prior
  read of the destination as a write precondition.
- The S1 cross-model ledger records 86Box's direct PIO data-out path as
  corroboration only; ATA-3 remains the authority.
- `core-machine-hdc-smoke` now forces every media read to fail permanently,
  then proves a valid `WRITE SECTORS` enters DATA_WRITE, performs no read and
  commits its sector through the existing write provider.
- Focused Core ATA, VM ATA, Core Compaq and Model-40 HDC smoke binaries pass.
  `verify-hdc-portal-closure` and `verify-core-controller-authority` pass.

The Compaq WD branch continues through the same previously covered CHS media
form. ATA controller/device selection, backing-media grammar, DMA, physical
media mechanics, board routing and timing values remain unchanged explicit L2
boundaries.
