# T400 S1 Generic-Reference Convergence Ledger

## Decision

`M5:T400:S1:GENERIC-REFERENCE-LEDGER-FROZEN`

T400 uses three explicitly separated evidence tiers.  Tier 1 is original
DeskPro-specific primary material; Tier 2 is a read-only generic emulator
implementation; Tier 3 is a project-owned generic IBM PC/AT logical skeleton.
A Tier-2 or Tier-3 result may repair a deterministic logical Core/VM behavior,
but it is never evidence of a DeskPro board identity, physical period, raster
phase, firmware outcome, or Model-L3 readiness.

## Finite Batch Ledger

| Row | Tier and source | Admitted behavior | T400 disposition | Physical receiver |
| --- | --- | --- | --- | --- |
| HDC multi-sector PIO / IRQ14 | Tier 2: PCjs `c7f21b4fa2bdedac3d5c73094a6402fdc8b24c70`, generic IBM AT HDC; Tier 3 project-owned CHS fixture | Data-sector completion advances CHS/count; normal status acknowledges IRQ while alternate status does not; next data sector and final completion each signal IRQ. | Accepted. The HDC smoke fixture was extended from one to two 512-byte sectors and now replays both transfers, status-port acknowledgement and completion. Existing production state machine already satisfied the contract; the missing regression was repaired. | Exact DeskPro controller identity, controller latency and media mechanics remain outside this row. |
| CECG generic EGA register/raster ordering | Tier 2: generic EGA models in PCjs/86Box; Tier 3 only if a register-visible behavior is independently reproducible | Reset, register gating and Input Status ordering only; no Compaq raster inference. | Next admitted candidate. | CECG analogue/raster phase, monitor and firmware-visible behavior remain separate. |
| CPU/DMA/BWAIT transaction ordering | Tier 2: generic emulator transaction abstractions; Tier 3 project-owned AT transaction skeleton | Deterministic arbitration/cancellation order only, if independently reproducible. | Queued after CECG; no DCLK/BWAIT scalar is inferred. | DeskPro BWAIT electrical/clock conversion remains a physical receiver. |

## HDC Probe And Result

The project-owned fixture uses a two-sector, 1-cylinder, 1-head, 2-sector CHS
media provider. It performs a READ SECTORS command with count two after an
ordinary one-sector write has supplied the first sector. The replay verifies:

- first word `0x5aa5` (the byte order resulting from the preceding PIO write);
- sector-number progression 1 to 2 and sector-count progression 2 to 1 to 0;
- IRQ pending after each ready sector and after final completion;
- alternate-status reads preserve a pending IRQ; normal status reads clear it;
- both sector transfers enter `PENDING_READ_SECTOR` before the deterministic
  next advance.

Verified on 2026-08-17 with a clean, single-threaded Debug build:

```text
cmake -S . -B build/t400-s1-gcc -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build/t400-s1-gcc --target core-machine-hdc-smoke -- -j1
ctest --test-dir build/t400-s1-gcc -R '^current\.core-machine-hdc-smoke$' --output-on-failure
# 1/1 passed
```

No third-party source, configuration, ROM, guest media, trace or binary was
imported. This is a regression-coverage repair, not a claim that PCjs’s generic
IBM HDC is the original DeskPro controller.