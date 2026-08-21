# T435 S2 - 80386DX L3 Implementation Tracker

The [80386DX manifest](t435-s2-80386-timing-manifest.json) is the sole
closure ledger. It expands every S1 successful-retirement base form, plus
659 finite legal size, REP-phase and multiplier contexts. Each key retains one Chapter
17 rule, route, status, batch and regression identity.

Run `powershell -NoProfile -ExecutionPolicy Bypass -File
tools/Verify-80386TimingManifest.ps1` before any tracker P. The current
baseline is deliberately strict: no aggregate smoke or selector presence can
make a key conforming. Closure requires every generated key to emit its ticks,
formula inputs, origin and `unallocated=false`, then be marked L3 conforming
under the shared [manifest result contract](t435-s2-manifest-result-contract.md).

| batch | required result |
| --- | --- |
| B0 | one 80386DX form/context publisher; no parallel successful selector |
| B1 | every Chapter-17 constant/formula is selected by decoded form |
| B3 | all r/m, size, mode/path, next-component, repeat and multiplier inputs are published |
| B4 | per-key result report has no nonconforming status |

Markers: `M5:T435:S2:80386-IMPLEMENTATION-TRACKER:OK`;
`M5:T435:S2:80386-TRACKER-CLOSURE-PREDICATE:OK`.
