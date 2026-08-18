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
| CECG generic EGA register/raster ordering | Tier 2: PCjs and 86Box generic EGA STATUS1 paths; Tier 3 project-owned EGA port replay | STATUS1 reads reset the attribute address/data flip-flop and provide a deterministic diagnostic-bit 4/5 compatibility fallback. | Accepted. VADP now alternates bits 4/5 on each configured-EGA STATUS1 read, resets that phase on VADP reset, and preserves all existing CECG port tests. | CECG analogue/raster phase, monitor, firmware-visible behavior and Compaq pixel-mux diagnostics remain separate. |
| CPU/DMA/BWAIT transaction ordering | Tier 2: PCjs asynchronous DMA ownership model; Tier 3 project-owned transaction/HOLD replay | Deterministic DMA ownership request/acknowledge/transfer/release order only. | Accepted. The arbitration callback now applies the existing logical HOLD lifecycle to 80386 as well as 80286 DMA service; an independent 80386 competition replay preserves the 80286 replay. | DeskPro BWAIT electrical/clock conversion, DCLK scalar and pin waveform remain physical receivers. |

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
## CECG-Shared Generic EGA Result

Both PCjs and 86Box retain a generic EGA STATUS1 bit-4/5 diagnostic compatibility
path that alternates those bits to satisfy EGA BIOS diagnostics; 86Box uses a
separate Compaq path when a full raster color multiplexer is available. NXVM now
implements only the generic fallback in the shared configured-EGA VADP status
reader: the first read after reset sets bits 4/5, the next clears them, and a
STATUS1 read still resets the attribute-controller address/data phase. The
project-owned `core-machine-ega-planar-port-smoke` verifies the alternation and
reset restart; all six existing CECG regressions pass unchanged.

This is Tier 2 logical compatibility, not a Compaq color-mux, monitor, or
physical-raster claim. No third-party source, configuration, ROM, guest media,
trace or binary was imported.
## 80386 DMA Ownership Result

The shared arbitration callback previously applied its project-owned logical
HOLD request/acknowledge/release envelope only when the CPU profile was 80286.
A 80386 machine therefore performed the identical DMA transfer outside that
observable transaction ownership boundary. PCjs also treats pending DMA as a
CPU-visible ownership concern, rather than an ordinary unrelated callback.

NXVM now applies the existing deterministic lifecycle to both 80286 and 80386
profiles when a DMA clock tick has a pending request. The new
`core-machine-competition-80386-s1-smoke` replays a CPU memory transaction and
retirement followed by DMA HOLD request, HOLD acknowledge, DMA begin/commit,
HOLD release, then DMA/PIT/PIC/FDC/HDC service ordering. The original 80286
competition regression remains separately registered and passes. This is a
Tier-2/Tier-3 logical ordering repair only: it does not model 80386 HOLD/HLDA,
DACK/AEN, BWAIT, or any DeskPro frequency or waveform.