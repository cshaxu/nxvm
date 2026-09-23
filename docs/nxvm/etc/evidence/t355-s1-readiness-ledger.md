# T355 S1: Windows 3.x readiness evidence ledger

## Decision

The existing `vm_windows31_*` sources are diagnostic probes, not current-gate
Windows compatibility evidence.  They are Windows-host-only executables,
require caller-supplied image paths, and use host threads and `Sleep` to observe
an emulated session.  They are useful leads, but neither their timeout nor a
successful screen string defines guest timing or device architecture.

T355 therefore allocates the first readiness repair to a **reproducible,
bounded checkpoint harness contract**, before selecting a new guest device.
That later receiver must use owner-approved BYOB configuration, declare a
checkpoint, keep host observation outside core guest time, and report a
reproducible first prerequisite failure.  It must not bundle Windows media or
promote a probe to current-gate merely by adding a timeout.

## Existing probe inventory

| Source / target | Declared checkpoint | Configuration / host boundary | Current status | Disposition |
| --- | --- | --- | --- | --- |
| `vm_windows31_checkpoint.c` / `vm-windows31-checkpoint` | DOS prompt then `C:`/BDA HDD visibility | Win32 thread, keyboard injection, `Sleep`; caller-supplied FDD/HDD | Not in `PROJECT_CURRENT_*_SMOKE_TARGETS` | Harness receiver; its BIOS/ATA observation is not a Windows boot result. |
| `vm_windows31_setup_probe.c` / `vm-windows31-setup-probe` | Typed `EWIN31\\SETUP.EXE`, `Reading SETUP.INF`, optional welcome | Win32 thread/input and 20--720 second host waits; caller-supplied HDD | Not current-gate; no automatic execution in S1 | Harness receiver; do not use timeouts as a device latency requirement. |
| `vm_windows31_hdd_admission_probe.c` / `vm-windows31-hdd-admission-probe` | BIOS INT 13h HDD geometry/MBR/VBR data | Temporary synthetic FDD plus caller HDD path, Win32 file API | Not current-gate | Retained ATA/BIOS consumer; T347/T354 service boundary is green, not a new storage blocker. |
| `vm_windows31_int13_trace_probe.c` / `vm-windows31-int13-trace-probe` | HDD INT 13h geometry and reads | Caller FDD/HDD paths; bounded instruction loop | Not current-gate | Retained BIOS/ATA trace consumer; future harness must preserve its checkpoint semantics. |

## Dependency classification

| Boundary | Current evidence | Readiness disposition |
| --- | --- | --- |
| CPU profiles and protected/VM86 state | T343 accepted four-profile closure; x87 execution remains excluded | Baseline for a non-x87 checkpoint; no Windows compatibility claim. |
| Selected L3, DMA/PIC/PIT/FDC/ATA | T347 pending readiness service and T354 transaction/competition/reset closure | Regression prerequisite, not a missing FDC/ATA implementation. |
| Video/input | Selected CGA/EGA and KBC/AUX evidence exists; VGA/VBE, broader EGA and advanced AUX remain TODO | No current probe names a required video/input contract; do not select a device until the harness or a corpus identifies one. |
| PPI/speaker and board NMI | No selected owner/corpus contract | Explicit non-blocking unknowns for this map; remain TODO and must not be synthesized. |
| Media/provenance | Current local-only record names owner-supplied runtime media and prohibits repository paths/bundling | A future run must validate the operator configuration without persisting local paths or media content. |

## Ordered receivers

1. **T355 S2:** define the bounded checkpoint-harness/provenance contract and
   make one existing probe reproducible without importing media or feeding
   host wait time into guest state.
2. **T355 S3:** execute only that approved checkpoint; classify the first
   repeatable prerequisite failure or stable checkpoint and allocate a device
   owner only from that observation plus primary/device evidence.
3. **T355 S4:** reconcile the result with CPU/device/L3 transfers.  Setup,
   Standard Mode, Enhanced Mode, installation and broad compatibility remain
   separate owner decisions.

## S1 verification

The ledger is a source/configuration audit only.  It inspected all four
`vm_windows31_*` sources, their CMake targets and current-media target lists,
the approved local-media record, T347/T354 storage evidence, the CPU closure
map, and current device/timing debt.  No Windows executable or guest media was
run, copied, changed, or added.
