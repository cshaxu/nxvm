# T503 S11 Cross-Profile Closure

## Finite Matrix

| Profile | CPU/board route result | Controller-route evidence |
| --- | --- | --- |
| IBM 5160 | XT PPI keyboard, single PIC and selected FDC/HDC topology retain their Core-only paths. | `vm-xt-5160-268-profile-smoke`; T503 S3--S9 route evidence. |
| IBM 5170 Model 339 | CGA presentation, AT firmware/FDC and timer/DMA/PIC construction retain their selected wiring. | Model-339 composition, CGA-topology and firmware-FDC smokes. |
| DeskPro 386 Model-40 | Compaq EGA, FDC/DMA2/IRQ6, RTC/KBC and selected fixed disk retain distinct frozen board inputs. | Model-40 composition, CECG, FDC/FDD and HDC smokes. |
| default-at | Generic PC/AT configuration retains Core-owned port/device wiring and immutable profile construction. | default-profile/apply/ROM materialization, FDC/DMA, RTC and HDC smokes. |

The transient cross-profile cohort passed 27/27. Complete repository-only unit replay passed 312/312 after a clean Debug regeneration. The owner-managed BYOB integration suite passed 20/20, including DOS keyboard/video/FDC/HDC, EGA/CGA and Windows 3.1 checkpoint paths.

## Actual Cross-Route Result

No cross-profile signal-chain defect was reproduced. The Release compiler did reject an implicit declaration introduced by S10's text fallback. The immediate minimal repair is one VADP-private forward declaration; the fresh Debug rebuild and complete unit suite prove no stale object masked it.

The sole current product target is `vm-0-5-0503`. Its stripped Release output is `build/output/nxvm_0_5_0503.exe`, 1,240,043 bytes, banner `0.5.0503`, SHA-256 `3AF1408F5E1C3760CF2A3D38D0EA8B7A739122BD2F4B7FBB49A527566E7A8373`. The runtime debugger remains present; compiler debug information is absent.

## Simplicity And Closure

S11 adds no profile-side route, controller state, scheduler, compatibility branch or external asset. It advances the one existing artifact target and repairs only C declaration order. All T503 routes are accepted through owner-specific S3--S10 evidence and this four-profile replay; retained physical timing and unselected controller capabilities remain outside the frozen universe under existing TODO receivers.
