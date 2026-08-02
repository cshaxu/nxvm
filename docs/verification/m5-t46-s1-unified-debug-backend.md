# M5 T46 S1 Unified Debug Backend

`core_product_debug_target` is the single product-neutral debugger backend. Its
VM adapter reads and writes the one live full-PC machine and now exposes pause
state, reason, request, continue, and step operations. The retained debugger
UI continues to use `debug_access`, so its existing command grammar reaches
that same backend.

Windows GCC, `M5:T46:S1:UNIFIED-DEBUG-BACKEND:OK`, pause-boundary,
full-authority, FDD/HDD full-PC profile, retained debug-target, retained
Console, dependency-DAG, and live-authority gates passed.

Artifact: `build/output/nxvm-m5_t46.exe`.
SHA-256: `95A2FDD774AE6B20E2A7B3726121FD9565AEBE34B2B12F025A63599D21B25944`.
