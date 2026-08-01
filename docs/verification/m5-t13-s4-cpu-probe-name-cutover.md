# M5 T13 S4 CPU-Probe Name Cutover

The VM-only finite CPU probe source/header and its focused smoke were moved with
`git mv` to `cpu_probe`/`nxvm-vm-cpu-probe` names. Probe bytes, capture layout,
reset behavior, and the retained illegal-instruction output are unchanged.

GCC built the user artifact and focused targets. CPU probe output retained
`M5:T1:S1:CPU-PROBES:OK`; request transport, FDD lifecycle, FDD/HDD reset
vector, no-media Console, and delayed debugger gates passed. No raw recorder
ran.
