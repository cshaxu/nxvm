# M1 T5 S12 Selected-Set Sound Terminal

## Method

The existing `myraidenii-selected-set-startup-probe` was rebuilt from the
cached x64 and x86 Ninja trees. The existing inventory verifier and staging
tool read the owner-supplied external archive, staged only its already
inventoried runtime images under a task-owned disposable build directory, then
removed each generated child directory. No archive member, trace, screen,
PCM sample, ROM name or new hash is retained here.

The probe output was extended only with already-public CPU diagnostic summary
fields: whether a CPU fault snapshot exists, its numeric mask/code/linear
position when valid, and delivered-exception count. It never emits the
diagnostic byte window.

## Result

Both widths produce the same neutral observation:

| Host | Executed | Elapsed ticks | Stop reason/detail | CPU snapshot | COP failure | PCM frames |
| --- | ---: | ---: | --- | --- | --- | ---: |
| x64 | 12,721 | 205,420 | `STOP_FAULT` / `LIB_STATUS_UNSUPPORTED` | absent | absent | 615 |
| x86 | 12,721 | 205,420 | `STOP_FAULT` / `LIB_STATUS_UNSUPPORTED` | absent | absent | 615 |

`reason=7` is `X86_CORE_MACHINE_STOP_FAULT`; `detail=3` is the existing
`LIB_STATUS_UNSUPPORTED` value. This is not a guest CPU exception: the public
CPU diagnostic has no fault snapshot and no delivered exception.

The existing Board deadline advance calls Sound first and COP second. Its COP
failure observation is empty, so the matching unsupported result is attributed
to the current local Sound execution/decoder path. This is a source-and-run
ownership diagnosis, not a claim about original hardware behavior.

## Evidence Grade And Transfer

This S adopts no manual-PDF formula, constant or register rule, thus makes no
L3 claim. It adopts no external emulator behavior, thus makes no L2 claim.
The terminal is merely the repository's current implementation fact. A later
Sound receiver must determine the missing Z80 operation from an eligible
manual fact (L3), bounded external behavior (L2), or explicitly labeled local
model (L1); it must not hide the failure in Product, Machine, Core Board or a
test-only fallback.

The probe proves neither audible output, valid game startup, graphics fidelity,
input handling, desktop presentation nor gameplay. It is a bounded external
integration diagnostic only.
