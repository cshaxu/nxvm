# T379 S1: Model-339 Final L3 Re-Audit

`M5:T379:S1:MODEL339-REAUDIT:OK`

## Decision

**Ready:** the frozen IBM PC/AT 5170 Model 339/Type 3 baseline now satisfies
the project's deterministic event-and-bus L3 contract. This is only the
8 MHz 80286, Rev.3 BIOS slot, 512 KB planar RAM, CGA, AT keyboard and
no-fixed-disk baseline. The 1.44 MB TEAC route remains aftermarket only.

| Requirement group | Independent evidence and disposition |
| --- | --- |
| CPU, memory/ROM and bus | T368/T369/T375 successful retirement, transaction owner and no-invented READY/wait boundary; accepted deterministic ordering, no electrical-phase claim. |
| DMA/PIC/PIT/RTC and reset | T374 S15 and T375 S2/S16/S25 source-backed lifecycle/rational-clock/reset proof; pin propagation remains explicit physical exclusion. |
| Keyboard/8042 | T374 S17/S18 and T375 S22/S23, plus T378's repaired production ingress proof and passing keyboard gate; accepted IRQ1-only Model-339 route. |
| FDC/floppy | T374/T375 functional and cadence work plus T376 raw IMG sidecar, Deleted Data and Scan proof; physical CHRN/flux/index/search limits remain outside the declared raw-IMG L3 surface. |
| CGA | T374 S11--S16 and T375 logical raster/status/default/cadence proof; no contention/snow/monitor waveform claim. |
| NMI/cross-device cancellation | T365/T372/T375 selected parity/reset/cancellation and deterministic ordered time; unselected I/O-check NMI remains excluded. |

## Fresh Replay

The full current gate passed after the T378 baseline repair. Fresh focused
Model-339 composition, clock, CGA topology, firmware/FDC topology, native
keyboard ingress, raw-IMG provider and core FDC smokes also passed. T345's
configured ownership verifier passes with the regenerated ledger.

## Boundary

This acceptance does not claim physical waveform timing, fixed-disk MFM/ATA,
generic AT behavior, EGA/VGA/AUX product support, ROM/media provenance or any
later-machine result. Those remain distinct declared boundaries. T379 is the
only required 5170 re-audit receiver and records no unresolved selected
functional or deterministic-L3 row.
