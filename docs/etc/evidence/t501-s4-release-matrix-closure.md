# T501 S4 Release Matrix Closure

`T501-S4-RELEASE-MATRIX-CLOSURE`

The normal current target rebuilt as stripped Release artifact
`nxvm_0_5_0501.exe` (`T501-S4-ARTIFACT`), SHA-256
`3651CD9245D11374F6089199E83A15F1F4C67B43E6A27F2751F74D3B57B89297`.
It retains the product runtime debugger and excludes compiler debug
information.

All owner-provided firmware and media were read-only transient inputs. This
record deliberately retains no local path, byte, hash, or execution trace
from them.

| Profile | CPU | Floppy format | Fresh semantic result or boundary |
| --- | --- | --- | --- |
| IBM 5160 Model 268 | 8088 | 360 KB | `installer-ready` |
| IBM 5170 Model 339 | 80286 | 360 KB | `installer-ready` |
| IBM 5170 Model 339 | 80286 | 1.2 MB | Named 80286-compatible external-input boundary: the supplied loader reaches an 80386-only path. |
| DeskPro 386 Model 40 | 80386 | 1.2 MB | Source-gated firmware-visible ROM/FDD board diagnostic boundary transferred by [List 2](t501-s2-model40-diagnostic-list-2.md); no controller, scheduler, or firmware shortcut was added. |
| `default-at` | 8086 | 360 KB | `installer-ready` |
| `default-at` | 80186 | 720 KB | `date-input` |
| `default-at` | 80286 | 360 KB | `installer-ready` |
| `default-at` | 80386 | 1.2 MB | `dos-prompt` |
| `default-at` | 80386 | 1.44 MB | `dos-prompt` |

The complete Debug repository-only unit route passed **312/312** at four
jobs in 14.08 seconds. The complete external-input integration route passed
**20/20** at four jobs in 15.99 seconds, including the Windows 3.1 checkpoint.

S4 changes only the normal artifact identity and its required closure
evidence. Core remains the sole guest-time/controller/state owner; no release
row creates a second time path, controller owner, media cache, or profile-side
compatibility path.
