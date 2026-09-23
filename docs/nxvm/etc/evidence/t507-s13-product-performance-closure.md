# T507 S13 Product And Performance Closure

`M5:T507:S13:PRODUCT-PERFORMANCE:P1`

## One Progression Path

The final S1/S12 matrix has one Core elapsed-tick writer and one scheduler
composition: immediate producer settlement, earliest qualified deadline, or
the explicit bounded L1 receiver.  Standard and Turbo call that same Core
path.  Standard alone compares completed Core progress with its host pacing
budget; Turbo omits that wait.  Neither VM mode supplies a guest tick count,
device deadline or fast-forward distance.

The focused speed-policy and scheduler cohort passes 4/4, including the L1
precedence regression.  The complete repository-only unit corpus passes
312/312 in 15.40 seconds.  The owner-managed external-asset integration corpus
passes 20/20 in 20.37 seconds, including DOS prompt, keyboard/video, FDC/HDC,
CGA/EGA, full-PC, and Windows 3.1 checkpoint coverage.  These are measured
host costs for the current configured suite, not a claim of physical machine
speed.

## Release Artifact

The current artifact target is now `vm-0-5-0507`; its CMake preset continues
to select the Release configuration.  The target's existing compiler/linker
route uses `-O3`, `-DNDEBUG` and `--strip-debug`, while retaining the linked
runtime debugger product path.

| Field | Result |
| --- | --- |
| Artifact | `build/output/nxvm_0_5_0507.exe` |
| Size | 1,242,348 bytes |
| SHA-256 | `5975E348032644A29C3876EA072B7C3DB5D7A9ED438842DBA4797CE6FC5CEFDF` |
| Section inspection | `objdump -h` reports no `.debug` section. |
| Session companions | Existing `default-pc-at.yaml` and `ibm-5170-model-339.yaml` are copied beside the executable. |

No debug trace was enabled in the Release Core (`CORE_MACHINE_RUNTIME_TRACE_ENABLED=0`).
The runtime debugger is intentionally retained and was covered by the
integration debugger boundary tests.

## Coordinator Acceptance

Actual-diff review accepted `a3574918`: it changes only the current artifact
target/preset and this release evidence.  The preceding T507 implementation
range keeps one existing Core scheduler and VM wait consumer; its complete
matrix and line accounting are recorded in the
[T507 closure history](../../history/M5-T507-core-scheduler-standard-turbo-performance-closure.md).
No source change followed the final unit and integration runs.
