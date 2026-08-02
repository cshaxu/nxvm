# M5 Naming Convergence

## Authority And Status

This plan reopens M5 after T54 for second-pass source-name, public-symbol,
platform-ownership, and retained-alias convergence. T54 is first closure
evidence, not final naming closure. This plan does not reopen DOS, CLI,
Win16, Microsoft NTVDM, or compatibility-corpus work.

T63 closes this plan's naming-only scope. M5 T64 is separate explicit
instance-authority design work: it does not revise T48--T63 results, but it
does replace the retained selected-session aliases through a later bounded
context-passing plan. Its forward authority is
`architecture/core-machine-instance-design.md`.

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
| M5 T48 | Freeze the naming plan, then rename low-risk shared core device files and current-object functions: `keyboard_controller.*` to `kbc.*` and `video_adapter.*` to `vadp.*`. | GCC build, source-DAG, authority smokes, retained Console smoke, `nxvm_0_5_0048.exe`. |
| M5 T49 | Rename VM machine device files from legacy `v*` spellings to canonical owner names: `vcmos`, `vdebug`, `vfdd`, `vfdc`, `vhdd`, and `vhdc`. | Same behavior and lifecycle order; GCC, device authority smokes, retained Console smoke, `nxvm_0_5_0049.exe`. |
| M5 T50 | Rename default-profile firmware files whose names still encode legacy VM implementation rather than profile role, beginning with `vbios.*` to `bios.*`; audit QD firmware names separately before changing them. | BIOS/POST/reset-vector and FDD/HDD gates pass, `nxvm_0_5_0050.exe`. |
| M5 T51 | Split or rename remaining public/provider contracts where current files mix interface and injected callback ownership. | Header dependency scan and focused provider smokes pass, `nxvm_0_5_0051.exe` if runnable paths changed. |
| M5 T52 | Audit ambiguous implementation names, beginning with the former `machine_memory.c`, `machine_port.c`, and core presentation names; apply only approved low-risk renames. | No module-boundary regression; GCC and retained UX gates pass, `nxvm_0_5_0052.exe`. |
| M5 T53 | Decide `vglobal.h` retirement path and move obvious shared type/constant declarations to `type.h`; keep `vglobal.h` as a transitional include only. | GCC and retained fixture gates pass, `nxvm_0_5_0053.exe`. |
| M5 T54 | First closure audit for T48-T53. This recorded remaining alias and platform questions and is not final naming closure. | No stale `device.h` claim, no unintended legacy `v*` source filenames, zero forbidden dependencies, `nxvm_0_5_0054.exe`. |
| M5 T55 | Reopen second-pass naming cleanup and freeze the T55-T63 plan. | Governance is consistent, M6 is deferred again, `nxvm_0_5_0055.exe`. |
| M5 T56 | Correct current/future documentation facts, especially absent `vdm/main.c` and actual `type.h` contents. | Docs match current tree and foundation headers, `nxvm_0_5_0056.exe`. |
| M5 T57 | Rename VM default-profile firmware public symbols from old `nxvm_firmware_default_profile_*` spelling to ownership-prefixed `vm_profile_default_firmware_*`. | Firmware/profile smokes pass, `nxvm_0_5_0057.exe`. |
| M5 T58 | Rename low-risk VM session/composition public symbols with mixed `nxvm_product_nxvm_*` / `nxvm_vm_*` prefixes to clearer ownership-prefixed names. | Session/product smokes pass, `nxvm_0_5_0058.exe`. |
| M5 T59 | Audit `vm/platform/{win32,linux}` ownership while preserving symmetric filenames such as `win32app`, `win32con`, future `linuxapp`, and `linuxcon`. | Function-level ownership map recorded, `nxvm_0_5_0059.exe`. |
| M5 T60 | Move only proven shared, policy-free platform mechanisms to `core/platform/{win32,linux}`; keep VM policy and symmetric adapter filenames in `vm/platform`. | Platform and retained Console gates pass, `nxvm_0_5_0060.exe`. |
| M5 T61 | Inventory retained runtime aliases such as `vcpu`, `vram`, `vport`, `vfdd`, `vhdd`, `vdebug`, and `vbios`, separating compatibility aliases from state-authority debt. | Alias/debt table recorded, `nxvm_0_5_0061.exe`. |
| M5 T62 | Apply only low-risk alias cleanup approved by the T61 inventory; defer stateful/multi-session aliases to explicit state-authority work. | Authority gates and retained UX pass, `nxvm_0_5_0062.exe`. |
| M5 T63 | Final second-pass naming closure audit over source, CMake, docs, public symbols, platform ownership, and retained alias exceptions. | Final audit evidence and `nxvm_0_5_0063.exe`. |

## Non-goals

- Do not implement multi-session execution, owned DOS services, VDM CLI, host
  drive mapping, or CPU instruction fixes in this naming plan.
- Do not remove temporary direct aliases such as live-object accessors merely
  because their names are old; alias removal requires a separate state-authority
  or session-boundary task.
- Do not move shared Win32/Linux mechanisms to `core/platform` unless the task
  has first proven that the code is mechanism-only and contains no VM policy.
- Do not rename `win32app`, `win32con`, or `linuxcon` merely for style.
  Symmetric host-surface names are acceptable; ownership is determined by
  dependencies and policy content.
- `win32app`, `win32con`, `linuxapp`, and `linuxcon` name host surface modes.
  They may exist under `core/platform/<os>` only for policy-free shared host
  mechanisms; VM/VDM lifecycle, CLI, debugger, or guest input semantics stay
  under the owning product shape.
