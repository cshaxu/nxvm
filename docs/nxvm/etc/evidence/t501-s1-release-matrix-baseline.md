# T501 S1 Release Matrix Baseline

`M5:T501:S1:RELEASE-MATRIX-BASELINE`

The current optimized stripped Release artifact and its existing local BYOB
probe were rebuilt from the T501 admission baseline.  All external firmware
and media remained owner-provided, read-only local inputs; this record retains
no path, byte, hash, or execution trace from them.

| Profile | CPU | Floppy format | Fresh semantic result or first boundary |
| --- | --- | --- | --- |
| IBM 5160 Model 268 | 8088 | 360 KB | `installer-ready` |
| IBM 5170 Model 339 | 80286 | 360 KB | `installer-ready` |
| IBM 5170 Model 339 | 80286 | 1.2 MB | No semantic terminal. The supplied image reaches an 80386-only execution path after its loader, so it remains the named missing 80286-compatible external-input boundary rather than a machine failure. |
| DeskPro 386 Model 40 | 80386 | 1.2 MB | No semantic terminal. Firmware reaches its `601` diskette-controller diagnostic and `RESUME`; no FDC terminal operation has occurred. The earliest unresolved owner is the firmware-visible Model-40 board/FDD diagnostic contract. |
| `default-at` | 8086 | 360 KB | `installer-ready` |
| `default-at` | 80186 | 720 KB | `date-input` |
| `default-at` | 80286 | 360 KB | `installer-ready` |
| `default-at` | 80386 | 1.2 MB | `dos-prompt` |
| `default-at` | 80386 | 1.44 MB | `dos-prompt` |

The repeated Model-40 result is neither a scheduler deadlock nor a synthetic
FDD result: Core continues guest progression through firmware diagnostics and
the observed controller state has no completed FDC operation.  S2 therefore
owns a complete original-source and read-only emulator cross-check of the
specific diagnostic contract before any behavior change.  It must not add a
second-ready drive, invent an F1 shortcut, or change scheduler progression.

The complete repository-only unit route passes 312/312 at four jobs.  This S
changes no product or test code.
