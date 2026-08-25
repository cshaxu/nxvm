# T462 S4: Controller Contract Closure

T462 closes its finite controller board-timing ledger without a fourth timing
path. The serial current gate, specialized controller gates, documentation
governance and stripped Release artifact were rerun after the corrective pass.

| Ledger batch | Final disposition |
| --- | --- |
| PIC-B1 | Explicit L2: Intel defines logical causal order, but no source defines an elapsed Core visibility term. No placeholder field exists. |
| PIC-B2/B3 | Existing selected cascade topology remains the sole owner; alternate topology and electrical timing remain transferred. |
| DMA-B1/B2 | Model 339 selects the source-qualified `3 / 8` nominal clock plus service-phase rule through the copied plan. Generic conversion remains L2. |
| DMA-B3/B4 | Existing fixed refresh route and transaction/arbitration owner remain unchanged; no route or scheduler field was added. |
| PIT-B1 | Model 339 selects the existing source-qualified `596591 / 4000000` ratio through the copied plan. |
| PIT-B2/B3/B4 | Existing Core routes and reset owner remain unchanged; power-up/electrical facts remain L2/excluded. |

The final source sweep finds one construction path only: descriptor, CPU contract,
session copy, validated opaque plan, then the existing Core controller owners.
Runtime IRQ, DREQ, EOP, BUSRDY and GATE operations remain dynamic controller
inputs and are not profile setters.

The current artifact is `build/output/nxvm_0_5_0462.exe`, 1,187,190 bytes,
SHA-256 `3E93ECCEF5ED8AC904EE63ECA081F966E9611C98F2912263CFF213F0D1D6D4AA`.
`current-gcc` rebuilt it through the Release-only `vm-0-5-0462` target, whose
GNU link route uses `--strip-debug`; the target's pre-link guard verified the
optimized Release configuration.

## Corrective Regression Evidence

T461 correctly made a completed PIT count write transfer from CR to CE at the
next input clock. Seven older tests had instead assumed immediate count/load
visibility. Their corrected assertions prove the pending count, first-clock
load, subsequent decrement, output transition and D4 transaction order. The
bounded D4 trace probe expands from 512 to 1024 records so the required DMA
event is retained after the additional input edge; its ordering assertion is
unchanged.

`core-machine-plan-smoke` emits both `M5:T462:S4:PIC-L2-BOUNDARY:OK` and
`M5:T462:S4:CONTROLLER-LEDGER-CLOSURE:OK`. Focused PIC lifecycle, auxiliary
PIT, D4 platform/refresh, prefetch locality, planar parity, Model-40 D4,
media-provider, debugger-lifecycle and all Model-40 plan-composition smokes
pass. The final serial `ctest --label-regex ^current-gate$ --parallel 1` result
is 294/294 passing in 348.65 seconds; the final specialized gate is 77/77;
documentation governance passes.

The similar-issue sweep covered plan construction/validation, profile
descriptors, all PIT-dependent current-gate tests, controller dynamic inputs
and order consumers. There is one descriptor-to-CPU-contract-to-session-to-
opaque-plan-to-owner construction route; no profile callback, device pointer,
live timing setter, extra scheduler or source-less promotion exists. The
corrective source/test delta is +28/-18 lines (net +10): one source-formatting
change needed by the static readiness check, two marker lines, and the
otherwise necessary regression assertions and bounded trace capacity. No
production controller state or timing algorithm was added.
