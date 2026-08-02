# M5 T43 S1 Debug Instrumentation Authority

`vdebug` now directly aliases the one `t_debug` embedded in
`vm_composition_live_machine`. Breakpoint, bounded-trace, recording, and
stop-callback state are all in that object. Existing debugger commands and the
retained debug-target binding use the same public operations as before.

Windows GCC, `M5:T43:S1:DEBUG-AUTHORITY:OK`, FDD/HDD full-PC profile,
debug-target, retained Console, and dependency-DAG gates passed.

Artifact: `build/output/nxvm-m5_t43.exe`.
SHA-256: `07354B991197ED9C693EDC44BC9E5CE79C744BB1CC32E2980875F87DC4F4C245`.
