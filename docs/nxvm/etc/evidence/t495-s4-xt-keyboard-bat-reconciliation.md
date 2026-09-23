# T495 S4 Withdrawn XT Keyboard BAT Diagnostic

`M5:T495:S4:XT-PPI-KEYBOARD-BAT:WITHDRAWN`

## Diagnostic And Transfer

| Relation | Finding | Transfer |
| --- | --- | --- |
| PPI boundary | The completed PPI owner correctly owns ports `60h`--`63h`, the received-byte latch and IRQ publication, but it is not the keyboard. | T496 creates a distinct keyboard-device audit; PPI remains the sole port/latch/IRQ owner. |
| Reset/BAT claim | The April 1983 corpus did not establish the sequence at diagnostic time. T496 subsequently acquired and visually checked the IBM March 1986 101/102-keyboard chapter. | T496, not PPI, now owns the source-backed device audit and any later implementation. |
| External comparison | Read-only 86Box inspection has a distinct XT keyboard device; PCjs has a model-specific keyboard producer. Neither is normative. | Use only as bounded corroboration of the separation, never as imported behavior. |

The former PPI BAT batch is not accepted, has no runtime result, and does not
close an XT boot row.  T496 owns the complete source/List-1/List-2 audit and
will admit any implementation only after its actual source boundary is known.
