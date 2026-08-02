# M5 T32 S1 Core-Machine Interface Naming

Public core-machine contracts now use `*_interface.h`; the private live
instance layout is `core/machine/machine.h`; and externally supplied callback
contracts use `*_provider` names. The M3 minimal machine, retained NXVM
Console, and full-PC composition retain their behavior.

Windows GCC build, core machine instance/lifecycle/debug/trace smokes,
retained Console smoke, and dependency-DAG verification passed. Markers:
`M3:T1:S2:MACHINE-INSTANCE:OK`, `M3:T2:S1:LIFECYCLE:OK`,
`M3:T4:S2:DEBUG:OK`, `M3:T2:S2:TRACE:OK`, and
`M5:T6:S1:NXVM-CONSOLE:OK`.

Artifact: `build/output/nxvm-m5_t32.exe`.
SHA-256: `E2EFC457EF39031A2522623AAD59B0DAACA39169184EA0BAF7EAAC2234B63A4F`.
