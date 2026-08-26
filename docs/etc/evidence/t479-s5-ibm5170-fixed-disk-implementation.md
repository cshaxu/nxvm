# T479 S5 IBM 5170 Fixed-Disk Implementation

`M5:T479:S5:IBM5170-HDC:OK`

## Result

Model 339 now declares one immutable Type-3 IBM WD1003/ST-506 fixed-disk
personality. `core_machine_hdc` remains the sole owner of its task file,
phase, DRQ, IRQ and step-selector state; the existing VM HDD remains the sole
owner of sector bytes and persistence. The profile selects neither ATA nor a
second controller or CHS cache.

## Ledger Disposition

| Rows | Implemented result | Proof |
| --- | --- | --- |
| A1--A5 | Model 339 exposes one HDC, CMOS type 3 for drive C, and INT 13 registration. | `vm-ibm-5170-model-339-composition-smoke` |
| A6--A10, A12 | The explicit IBM personality accepts documented CHS PIO read/write variants, reset/seek/verify/diagnostic/set-parameters grammar and preserves command-write/status-read IRQ acknowledgement plus DRQ phases. ATA Identify is rejected. | `core-machine-hdc-smoke` |
| A11 | `3F6h` is write-only IBM fixed-disk state: bit 3 extends the logical head and cannot mutate drive select or become ATA alternate status/device control. | `core-machine-hdc-smoke` |
| A14 | The Core-owned IBM state records selector 0 as 35 microseconds (280 ticks at frozen 8 MHz), selectors 1--15 as 0.5--7.5 ms, and the 1023-pulse restore/diagnostic bound. It publishes no rotation, ready or full-seek deadline. | `core-machine-hdc-smoke` |
| A13, A15 | Raw MFM, ECC/long/format/recovery, mechanics, adapter parity and NMI remain absent and explicitly transfer to the physical-media residual. | S4 gap ledger and `TODO.md` |

The 8 MHz source is copied by the immutable IBM profile into the HDC config.
It is only used to represent the manual selector in Core ticks; Core remains
the guest-time owner and no host time reaches the controller.

## Retained Regression Results

- `core-machine-hdc-smoke`: ATA owner/media/service and IBM WD1003 checks pass.
- `vm-ibm-5170-model-339-composition-smoke`: Model 339/profile route passes.
- `core-machine-compaq-hdc-s5-smoke` and
  `core-machine-compaq-hdc-machine-s5-smoke`: selected Compaq normal route
  and shared `3F7h` behavior pass.
- `vm-hdc-port-smoke`: default-at ATA PIO port/progress route passes.

## Simplicity Accounting

The implementation changes the former false no-HDC branch into the existing
HDC owner; it adds no controller object, media cache, compatibility wrapper or
runtime-selectable profile state. The changed source and tests are +221/-42
lines before evidence/status closure. The only generic ATA wording in the
shared firmware route is replaced with fixed-disk PIO wording.
