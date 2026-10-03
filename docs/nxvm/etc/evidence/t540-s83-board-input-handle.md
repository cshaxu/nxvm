# T540 S83 Board Input Handle

## Scope And Confirmation

The executor confirms CURRENT's S83 packet against accepted S82 P2
`39695a3d4698c9c273ccb53d4de5db7b3fd524a5`. This is the complete
keyboard-byte, keyboard-stream, scan-set, XT fault-input and relative-mouse
receiver class. It is NXVM-only, not a Shared source move or new timing claim.
Existing source authorization and the embedded-artifact exception remain.

## Ownership And Actual Diff

All five public declarations and definitions receive the board handle already
published by construction. Input resolves chip state directly from that sole
allocation. Its borrowed opaque Core handle supplies the existing lifecycle
observer and mutation eligibility check. The latter declaration moves from
the private header to the public neutral contract; its implementation, return
type and firmware-reentry rule do not change. There is no getter, wrapper,
mirror, second factory, allocation, destructor or runtime rebinding.

The caller sweep covers 36 calls: three production calls in machine.c, eight
integration files and three unit fixtures, including multiline calls. A
full-tree search finds no remaining input call with a Core receiver. Each
production/integration file matches its baseline plus exactly the relevant
receiver substitution; unrelated Core calls are unchanged. All five XT fixture
constructors now retain the actual board output, and their associated direct
chip assertions use that handle rather than Core's private association.
The paging/IRQ fixture passes both outputs through its existing preparation.

The original lifecycle assertions remain. Their fixture now runs for both
XT and AT and checks bounded-firmware reentry, null receivers, null query
output and unsupported XT/AT fault-input selection. Disabled AT auxiliary
input still returns INVALID_STATE; extraction does not enable it. The first
expanded-test run exposed an incorrect new expectation of OK for that disabled
device; correcting the test to the unchanged chip contract passed all three
regressions. No production behavior was changed to make that test pass.

## Verification

The existing markers remain, with added
`M5:T540:S83:BOARD-INPUT-HANDLE:OK`. The controller-authority gate checks all
five real receiver declarations/definitions, forbids private board recovery
inside the complete input block, and requires its four mutating paths to use
the sole Core guard. Positive copied-source proof and nineteen injected
negative variants pass, including each of the five wrong receiver types,
private board recovery and guard removal. Copied probe sources are bounded
below build and removed after proof.

Complete units pass 470/470 in both widths: x64 286.59 seconds, x86 88.42
seconds. The serial CPU-boundary negative passes in 57.46/59.32 seconds.
Complete specialized gates pass in both widths, including the 402-row matrix
(377 strict, 25 explicitly deferred); their intentional negative self-test
diagnostics are caught as expected. No complete-unit failure or retry occurs.
All eight current 0540 product builds and independent neutral executions pass.
Each unchanged INI/external-media boot runs exactly once: default x64/x86 reach
the DOS prompt; XT, AT and Model 40 x64/x86 reach the running installer. The
180-second limit is containment, not a success checkpoint or repeated sample.
These eight samples do not prove indefinite absence of intermittent faults.

Every product passes PE width, 0.5.0540 banner, source freshness, optimized
Release and absence-of-compiler-debug-section inspection. Shared's six
source/test corpora, MyNES's 0043 pair, INIs and external masters are unchanged.
Full documentation governance, changed-document local links and diff checks
pass. Ignored scripts/logs/build trees remain needed by the next receiver;
copied negative sources have been removed and all owned verification commands
have exited.

| Artifact | Bytes | SHA-256 |
| --- | ---: | --- |
| `assets/nxvm/compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x64.exe` | 1336056 | `704291027167BEE082E783EA1D5A12DBA2383264B50A2ACBA23E381E9604FAEB` |
| `assets/nxvm/compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x86.exe` | 1506888 | `1C659CB04BD55EF564EF3E74EF80876BACB8BF3ED72083914D23F720245B38CF` |
| `assets/nxvm/default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x64.exe` | 1352373 | `F6866A2728A7589A2CAF133FE14D63B95E0287E5CAB4344FB212505762BBE30C` |
| `assets/nxvm/default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x86.exe` | 1523203 | `E4163E30FCDD13071CFAFDC61C8A94BBAC61F7C39425BBF4114B3C0753F90D8B` |
| `assets/nxvm/ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x64.exe` | 1352341 | `9ED831B8B3E6A6F1C8A1195A6D18F10DAC2450E0FC85AD49A33B98CCCCCA4323` |
| `assets/nxvm/ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x86.exe` | 1523170 | `C1C397C0E7DF6B9E6D5A1C177FC07872E907CD202397489EFCE3BF37C4B85F65` |
| `assets/nxvm/ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x64.exe` | 1352407 | `4A1015198A5EC6E18C75E646A2A4CED3A61EDDD4877FF6D36B857E22E29C0A73` |
| `assets/nxvm/ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x86.exe` | 1523238 | `D1309B994B48BCF1E01EEC5F1FB3608F59CA08470F253C502B662626F3FF8650` |

Source/tests occupy 16 changed paths, +115/-84, net +31. Production alone is
five paths, +38/-35, net +3; tests add 28 net lines for retained-handle setup
and lifecycle/reentry proof. The existing gate adds 24 lines. No new production
function or executable route accounts for this growth.

## Retained Owners And Next Receiver

Public display/configuration operations, chip-wiring contexts, direct-board
fixture classification and the private Core board association remain open.
Those complete classes precede physical neutral Core/IBM-PC relocation.
Core remains the unique execution/time/route/destruction owner; board remains
the unique chip/wiring owner. S83 does not close T540.
