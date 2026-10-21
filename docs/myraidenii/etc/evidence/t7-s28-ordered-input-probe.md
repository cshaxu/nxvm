# M1 T7 S28 — Ordered Coin/Start Probe

## Scope and model grade

S28 changes only the existing selected-set integration Probe and its wrapper.
The new `--coin-start-sequence` mode reuses the existing Machine driver's
ordinary KVM delivery.  It creates no production input path, host automation,
guest-state mirror, or timing owner.

The normalized instruction-budget thresholds are an L1 diagnostic schedule,
not an asserted board-time equivalence to MAME's frame-based input schedule.
The selected-ROM observations are L2 runtime evidence only.

## Schedule and parser contract

The Probe now accepts exactly one of the existing held, simultaneous-pulse or
ordered-sequence modes.  The ordered mode applies four distinct edges through
the same driver:

| Edge | Instruction-budget fraction |
| --- | ---: |
| Coin 1 press | 1/4 |
| Coin 1 release | 1/2 |
| Start 1 press | 3/5 |
| Start 1 release | 13/20 |

Conflicting control options are rejected before image loading.  The wrapper
forwards the same single option and rejects multiple options itself.

## Dual-width owner-ROM observation

Each host width ran the selected set once at the existing 20,000,000-instruction
cap.  The complete neutral summaries had the same SHA-256:
`D4C0D9F8E98EE39B0345CE294F28650876DB754F1DA221F0D292FBA86231AE61`.

| Marker | x64 and x86 |
| --- | --- |
| Four ordered deliveries | all successful |
| Executed / elapsed ticks | 20,000,000 / 324,088,505 |
| Final frame digest / nonblack pixels | `ccad0e0867e39b9c` / 74,865 |
| Frame transitions | 3 |
| YM address/data writes | 11,047 / 11,047 |
| OKI 1/2 writes | 3,657 / 3,657 |
| Active OKI voices / generated nonzero PCM | 0 / 0 |

The existing 2,000,000-instruction focused confirmation also matched on both
widths at every emitted sample.  All twelve MyRaidenII unit tests passed on
each width.

## Disposition

Changing from a simultaneous Coin/Start pulse to an ordered one does not
produce a demonstrated local title, credit, start, sound or gameplay result
within this bounded observation.  It proves only that the existing electrical
input route accepts the four edges deterministically.  It does not identify
COPX-D2 as a cause or justify a guessed Board receiver.

The wrapper removed every staged image directory.  The task-owned diagnostic
roots and temporary neutral logs were removed after their processes exited.
