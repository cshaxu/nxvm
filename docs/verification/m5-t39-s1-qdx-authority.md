# M5 T39 S1 QDX Authority

`qdxTable` now directly aliases the one dispatch table in
`vm_composition_live_machine.default_qdx_storage`. QDX's CPU opcode hook,
firmware registration order, reset behavior, and stop/reset semantics are
unchanged.

Windows GCC, `M5:T39:S1:QDX-AUTHORITY:OK`, FDD/HDD full-PC profile,
debug-target, retained Console, and dependency-DAG gates passed.

Artifact: `build/output/nxvm-m5_t39.exe`.
SHA-256: `0453066DE64E409729383AD88E088815CC1BC1B1D85E8057DE9716B19EB00D0C`.
