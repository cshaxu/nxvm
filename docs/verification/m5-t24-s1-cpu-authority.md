# M5 T24 S1 CPU Authority

`core/machine/vcpu.c/.h` were moved into the canonical `cpu.c/.h`; the retained
decoder/executor now lives in `cpu_instructions.c/.h`. The former small M3 CPU
state API is merged into `cpu.h` and implemented by `machine.c`, so no
temporary CPU-state source remains.

`vm_composition_live_machine` embeds one `t_cpu` and one `t_cpuins` object.
`core_machine_cpu_current()` and
`core_machine_cpu_instructions_current()` point directly to those objects while
the VM session is live. The retained `vcpu` and `vcpuins` spellings are direct
temporary aliases; they have no global storage, cache, or synchronization
path. A control-only caller that has no active handle receives this same handle
for its existing lifetime and releases it at finalization.

Windows GCC built the T24 authority, retained live-machine, CPU stop, CPU
probe, full-PC profile, Console, debugger-target, dependency-DAG, and T24
artifact targets. Results:

- `M5:T24:S1:CPU-AUTHORITY:OK`
- `M5:T23:S1:LIVE-MACHINE:OK`
- expected reset-vector `#UD`, then `M5:T14:S3:CPU-STOP:OK`
- expected probe `#UD`, then `M5:T1:S1:CPU-PROBES:OK`
- `M3:T3:S1:FULL-PC-PROFILE:OK` with the established local FDD/HDD images
- `M5:T14:S3:VM-DEBUG-TARGET:OK`
- dependency DAG: `0 known migration edges`

The ignored local developer artifact is `build/output/nxvm-m5_t24.exe`, built
from the T24 completion source state. It emits
`Neko's x86 Virtual Machine [0.4.015d.m5t24]` and accepts retained Console
`HELP`/`EXIT`. SHA-256:
`586A37CC5A5634015C9D83469A2BA22368BDA03010F9BF233274AFAFF3C9FDD0`.

The old interactive debugger's piped-input exit sequence is not a reliable
automation vector; its focused debugger target gate passed, and no Console or
debugger source behavior was changed by this task.
