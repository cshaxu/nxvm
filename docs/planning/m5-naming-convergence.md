# M5 Naming Convergence

## Authority And Status

This plan reopens M5 after T47 for source-name and public-contract
convergence. It does not reopen DOS, CLI, Win16, Microsoft NTVDM, or
compatibility-corpus work.

The compatibility gate is unchanged: no task may alter retained NXVM Console
text or grammar, debugger behavior, boot/reset ordering, media behavior, or
visible runtime behavior without explicit owner approval and recorded
before/after evidence.

## Version And Artifact Rule

Beginning with M5 T48, each completed runnable task uses the task number as
the patch version:

```text
T48 -> 0.5.0048 -> build/output/nxvm_0_5_0048.exe
T49 -> 0.5.0049 -> build/output/nxvm_0_5_0049.exe
```

Use four decimal digits for the task component. The same rule later applies
to `ntvdm64_0_5_NNNN.exe` after a runnable VDM product exists. A design-only
subtask inside an implementation task does not produce an executable; the
artifact is produced when the runnable task completes.

## Naming Rules

- Public headers consumed outside the owning module are named
  `<subject>_interface.h`.
- Callback/provider contracts supplied to a module are named
  `<subject>_provider.h` when they are large enough to stand alone. Provider
  types end in `_provider`; binding functions end in `_bind_provider` or
  `_install_provider`.
- Private module headers use the plain subject name, such as `machine.h`,
  `cpu.h`, or `debug.h`.
- Legacy `v*` device names are transitional only. Canonical names describe the
  device or authority directly: `cpu`, `memory`, `port`, `pic`, `pit`, `dma`,
  `kbc`, `vadp`, `cmos`, `fdd`, `fdc`, `hdd`, `hdc`, `bios`, and `debug`.
- Device models stay as flat files unless a real multi-file subsystem exists.
- Source moves use `git mv`; each implementation task changes one naming
  family at a time and repairs direct includes and CMake paths before moving
  on.

## Task Breakdown

| Task | Purpose | Gate |
| --- | --- | --- |
| M5 T48 | Freeze the naming plan, then rename low-risk shared core device files: `keyboard_controller.*` to `kbc.*` and `video_adapter.*` to `vadp.*`. | GCC build, source-DAG, authority smokes, retained Console smoke, `nxvm_0_5_0048.exe`. |
| M5 T49 | Rename VM machine device files from legacy `v*` spellings to canonical owner names: `vcmos`, `vdebug`, `vfdd`, `vfdc`, `vhdd`, and `vhdc`. | Same behavior and lifecycle order; GCC, device authority smokes, retained Console smoke, `nxvm_0_5_0049.exe`. |
| M5 T50 | Rename default-profile firmware files whose names still encode legacy VM implementation rather than profile role, beginning with `vbios.*` to `bios.*`; audit QD firmware names separately before changing them. | BIOS/POST/reset-vector and FDD/HDD gates pass, `nxvm_0_5_0050.exe`. |
| M5 T51 | Split or rename remaining public/provider contracts where current files mix interface and injected callback ownership. | Header dependency scan and focused provider smokes pass, `nxvm_0_5_0051.exe` if runnable paths changed. |
| M5 T52 | Audit ambiguous implementation names such as `machine_memory.c`, `machine_port.c`, `debug.*`, and platform Win32/Linux files; apply only approved low-risk renames. | No module-boundary regression; GCC and retained UX gates pass, artifact if runnable paths changed. |
| M5 T53 | Decide `vglobal.h` retirement path and remove only declarations already replaced by `type.*`, `version.*`, or owner-local headers. | No broad compatibility header remains for new code; GCC and retained fixture gates pass, artifact if runnable paths changed. |
| M5 T54 | Close naming convergence with a source/CMake/doc audit and final retained NXVM artifact. | No stale `device.h` claim, no unintended legacy `v*` source names, zero forbidden dependencies, final artifact. |

## Non-goals

- Do not implement multi-session execution, owned DOS services, VDM CLI, host
  drive mapping, or CPU instruction fixes in this naming plan.
- Do not remove temporary direct aliases such as live-object accessors merely
  because their names are old; alias removal requires a separate state-authority
  or session-boundary task.
- Do not move shared Win32/Linux mechanisms to `core/platform` unless the task
  has first proven that the code is mechanism-only and contains no VM policy.
