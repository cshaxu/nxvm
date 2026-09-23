# M5 T496 IBM 5160 XT Keyboard And Bootability Completion

T496 began with the IBM XT keyboard as its independent hardware unit.  Its
later owner-approved bootability boundary consumed the finite IBM 5160 startup
ledger without replacing the completed PPI, PIT, DMA, FDC, CGA or Xebec units.

| Subtask | Accepted result |
| --- | --- |
| S1 | Original keyboard source, List 1 and List 2 freeze the keyboard/PPI ownership split. |
| S2 | The Core keyboard FIFO/reset/BAT path replaces the hidden PPI queue. |
| S3--S5 | BYOB startup diagnostics classify the first reachable owner batches without accepting a timeout as success. |
| S6 | The eight-batch XT convergence ledger selects the PIT1/DMA0/scheduler boundary. |
| S7 | Reset/ROM ownership is reconciled through the existing Core route. |
| S8 | `core_machine_fdc_read_data()` releases its own IRQ6 source when the first normal result byte is consumed; focused FDC, registered DOS Read Track, and selected XT DOS terminal evidence pass. |

The retained [proposal](M5-T496-xt-keyboard-device-proposal.md) records the
original unit boundary.  The final integration result retains one Core FDC
result/IRQ lifecycle; it adds no BIOS, VM, image or profile workaround.
