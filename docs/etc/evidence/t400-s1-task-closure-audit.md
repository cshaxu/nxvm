# T400 S1 Generic-Reference Continuation Closure Audit

## Decision

T400 closes. Its finite three-row ledger is exhausted: every HDC, CECG and
CPU/DMA/BWAIT row is accepted only as a Tier-2/Tier-3 deterministic logical
contract, with a project-owned regression and a named physical receiver. No
row claims an original DeskPro controller, CECG raster, board clock, BWAIT
waveform or Model-L3 result.

| Ledger row | Actual change and proof | Retained boundary |
| --- | --- | --- |
| HDC multi-sector PIO/IRQ14 | The two-sector project-owned HDC smoke now covers both READ and WRITE sector advancement, per-sector IRQ, alternate-versus-normal status acknowledgement and final completion. The existing HDC owner already conformed; this repairs a missing regression. | DeskPro controller identity, latency and media mechanics. |
| CECG generic EGA STATUS1 | The shared configured-EGA VADP reader now supplies the PCjs/86Box-compatible alternating diagnostic bits 4/5 while retaining attribute-flip-flop reset; its focused EGA and existing CECG regressions pass. | Compaq color mux, monitor, firmware-visible raster behavior and physical phase. |
| 80386 DMA ownership | The shared Core arbitration owner now applies its existing HOLD request/acknowledge/release envelope to 80386 as well as 80286; the independent 80386 competition replay passes. | HOLD/HLDA, DACK/AEN, BWAIT/DCLK and board waveform conversion. |

The full current gate passed 285/285 tests on 2026-08-17 after the D4 parity
smoke was corrected to enable the active-low DeskPro IOCHK source before it
expects an NMI. That is a test repair for the already accepted port-61h
contract, not a new T400 hardware claim. Documentation governance and diff
hygiene pass. No third-party source, ROM, guest media, configuration, trace or
binary was imported, and no Core/VM public ABI changed.

## Transfer

T400 does not make a physical DeskPro or L3 claim. Its retained exact/physical
boundaries stay with their existing DeskPro receivers. The next approved queue
candidate is the four-profile CPU instruction-correctness audit; it begins as
a separately admitted task and may use emulator implementations only as
secondary cross-checks after Intel primary-manual requirements are fixed.
