# T514 S3: Win32 Keyboard Regression Matrix

The sole Core-platform normalizer state owns UTF-16 pairing and the Window
zero-scan virtual-key/character correlation.  Console and Window retain only
packet adaptation; VM retains only sink binding and the F9 product command.

| Packet disposition | Owner-local proof |
| --- | --- |
| Native recovered virtual key | `VK_F1` with zero scan resolves and reaches the VM request route. |
| Native enhanced key | Existing `E0` scan remains unchanged. |
| Layout character | `a` and shifted `A` emit physical make/break sequences with no stuck key. |
| Matching duplicate | A recovered `VK_A` consumes its following `a` character once. |
| Nonmatching/stale duplicate | An unmatched character and a released recovered key do not suppress later text. |
| UTF-16 | Lone low surrogate and completed non-BMP pair are unsupported without sink mutation. |
| Unsupported scalar | Lone surrogate is rejected without sink mutation. |

Focused tests are `core-platform-win32-keyboard-smoke`,
`vm-keyboard-host-ingress-smoke`, and `vm-platform-win32-keyboard-scan-smoke`.
They pass on 2026-09-01.  The complete repository-only unit gate and
documentation governance are the S closure gates.
