# Td S105 Tiered DeskPro Receiver Ledger

`M5:TD:S105:TIERED-DESKPRO-QUEUE:OK`

## Decision

The T398 physical/firmware boundary remains not ready for Model-L3. That is not a reason to leave DeskPro work unplanned: the following finite receivers are eligible for one-at-a-time implementation using the strongest stated input. `original`, `reference-derived`, and `generic-at` are evidence tiers, not interchangeable claims.

| T398 receiver | Existing owner | Strongest usable input | Bounded contract and regression | Disposition |
| --- | --- | --- | --- | --- |
| CPU-to-board conversion / BWAIT | Core transaction, CPU execution and machine timeline | `original` | T408 records D3PE pages 60--63: initial/row-miss CPU memory reads insert two waits and row hits insert zero; an implementation needs a Core-owned external CPU-memory-cycle boundary plus source-backed D4 row/bank decoding, reset/cancellation and profile isolation. | Retained: the current logical access path mixes fetch, paging and data, and the available material does not yield a safe address-to-row decoder. |
| Device DRQ / IRQ / NMI phase | Core KBC/FDC/PIC/DMA and machine peripheral tick | `reference-derived` for the DeskPro configuration shape, `generic-at` for an unmeasured keyboard serial delivery mechanism | T406 owns keyboard-native-byte pacing only: host-mapped native bytes enter the existing KBC serial queue; at most one is released per configured virtual-time cadence; a zero cadence preserves existing immediate behavior; reset cancels queued cadence state without losing the existing KBC reset contract. Regression: byte ordering, IRQ1 visibility, full-output retention, zero-cadence compatibility, cadence expiry, and reset. | First implementation receiver. It does not set a Compaq physical rate or accept L3. |
| Compaq fixed-disk media | Core HDC and VM media/profile binding | `generic-at` | A future AT HDC phase mechanism may use only the existing HDC command/data/IRQ14 owner; media geometry and content stay user-provided and out of repository scope. Regression: command phase, reset, IRQ edge, and media-validation failure. | Retained: available PCjs configuration has a different HDC/media selection, so it cannot supply a Compaq fact. |
| CECG / monitor / firmware behavior | Core VADP and Model-40 profile | none | A future receiver needs an explicit CECG register/raster or monitor contract and a project-owned VADP regression; it must not substitute PCjs IBM EGA behavior. | Retained: reference configurations are not CECG-equivalent. |
| Firmware-visible lifecycle | VM composition/session and profile firmware provider | none | A future receiver needs a lawful user-supplied firmware lifecycle contract, with explicit slot, mapping, reset and validation ownership. No firmware byte, path, hash catalogue, or discovery mechanism may enter the repository. | Retained under the BYOB admission boundary. |

## Reference Qualification

The local, read-only PCjs DeskPro configuration identifies a PC AT keyboard and a DeskPro chipset, while its browser keyboard component uses a configurable transmit cadence. It is a useful reference-derived scheduling shape, not a measurement of the selected 1986 Model 40. The local, read-only 86Box source separates an AT keyboard and Compaq-386 chipset implementation, which confirms that input delivery and board/chipset behavior are distinct owners; it supplies no selected-machine keyboard rate. Neither source, its firmware/media, nor its text is imported or transliterated.

The reference configurations differ in video, media, and controller choices. Those deltas are why this ledger uses the references to choose the existing owner and scheduling mechanism only, and labels the actual T406 cadence mechanism `generic-at` until primary hardware evidence exists.

## T406 Admission Target

Admit the next numeric task as **T406, DeskPro generic-AT keyboard serial delivery cadence**. Scope is one Core KBC mechanism plus its profile clock binding and project-owned tests. It may extend the existing profile-to-Core configuration record, but must not create a Core/VM bypass, host-time input, new firmware dependency, or selected-Compaq rate claim. It stops if the existing serial queue cannot carry cadence state without changing the frozen KBC command/FIFO/IRQ contract.

T406 is a concrete receiver for only the device-phase row. It transfers all other rows and all physical/L3 acceptance to later evidence-led work.