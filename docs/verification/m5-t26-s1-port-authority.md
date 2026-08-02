# M5 T26 S1 Port Authority

The retained full-PC dispatcher now lives in canonical `core/machine/port.*`.
`vm_composition_live_machine` embeds its sole `t_port`; `vport` directly
aliases the bound object. The independent M3 minimal-model implementation is
now `machine_port.*` and is not constructed by NXVM composition.

Windows GCC passed core/minimal-instance, live CPU/RAM/port authority, CPU
stop/probe, debugger target, FDD/HDD full-PC profile, retained Console, and
zero-edge dependency-DAG gates. The port smoke also confirmed port `0x92`
still dispatches the A20 callback. The local task artifact is
`build/output/nxvm-m5_t26.exe`, SHA-256
`26733E3E193C4B1ABFEC28ADBE6ADEB20C55CA55B43FF4A6728850AE23281968`.
