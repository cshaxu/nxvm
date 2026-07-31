# CPU Correctness And Differential Verification Requirements

## Current Claim And Risk

The retained NXVM CPU is an 8086-plus implementation with partial i386 decode
and execution coverage. It must not claim complete 80386 correctness. Its
`i386(...)` dispatch markers describe intended coverage, not proof of correct
semantics, exceptions, prefixes, addressing, flags, privilege behavior, or
timing.

An owner-local MS-DOS 6.22 observation is a blocking compatibility signal:
after boot, invoking `MEM` currently reaches an invalid-opcode (`#UD`) path.
The guest image is protected local media and is not committed, hashed in this
repository, or used as release evidence. Before any fix, the reproducer record
must capture the profile id, CPU capability set, CS:IP/linear PC, instruction
bytes and prefixes, register/FLAGS state, interrupt/exception state, and a
bounded trace window.

## CPU Capability Contract

Machine and execution profiles declare required CPU capabilities rather than a
marketing CPU label. Capability claims are additive and evidence-backed:

```text
x86.real_mode_8086
x86.protected_mode_286
x86.i386_decode
x86.i386_real_mode
x86.i386_protected_mode
x86.i386_paging
```

`nxvm.machine.pc_at_builtin` may initially require only the capability subset
proven by its M1 boot regression and focused probes. It may not require or
advertise `x86.i386_real_mode`, `x86.i386_protected_mode`, or
`x86.i386_paging` until their corresponding probe suites and differential
results pass. A future Deskpro 386 profile must declare the exact capability
floor it needs; profile selection fails before reset when that floor is absent.

Each admitted capability has:

- an instruction/decode family matrix, including prefix and addressing forms;
- deterministic positive, negative, fault, and flag/register probes;
- bounded memory/port/interrupt side-effect assertions; and
- a reference or project-owned trace verdict where a reference is available.

## #UD Triage Contract

`#UD` is not suppressed, converted to a successful no-op, or worked around by
profile-specific BIOS/DOS behavior. The triage workflow is:

1. reproduce with a bounded instruction, wall-clock, and no-progress budget;
2. capture the required CPU context and normalized trace window;
3. classify decode absence, prefix/addressing defect, execution defect,
   exception-delivery defect, or earlier state corruption;
4. add a minimal project-owned probe that fails before the repair;
5. compare the probe against the approved reference method where applicable;
   and
6. repair the CPU path, preserve exception semantics, and retain the probe.

The local MS-DOS `MEM` observation remains a regression scenario after the
minimal probe exists. It cannot be the only acceptance test.

## Bochx/Bochs Differential Bridge

Bochx is an optional developer-only paired-step bridge derived from the
owner's historical NXVM `bochx` work. Its historical mechanism is relevant:
Bochs copies CPU state to NXVM, NXVM executes one instruction against bridged
physical memory, and the bridge compares normalized post-step state. It also
records linear memory access and can stop the reference CPU on divergence.

The future tool lives only under `tools/research/bochx/` and is excluded from
default CMake targets, artifacts, releases, product runtime, and normal tests.
It may use locally installed Bochs source and a separately reviewed local patch
set. Bochs source, binaries, patches, BIOS files, disk images, and generated
traces are not committed here. Historical Bochx code may be imported only with
NXVM provenance; Bochs-derived material requires its own license review and is
never copied merely because the bridge is useful.

### Experiment Contract

Each Bochx experiment declares:

- NXVM commit/build and local Bochs version/patch identity;
- CPU mode, profile capability set, initial registers, memory map, and input
  probe identity;
- start/end PC or explicit instruction budget;
- compared registers, FLAGS mask, segment/control state, memory ranges, and
  permitted implementation masks;
- trace event schema, ring-buffer size, timeout, and no-progress budget; and
- cleanup owner and resulting project-owned regression probe.

The bridge is a CPU oracle only within its declared experiment. It is not an
oracle for BIOS, POST, disk, PIC/PIT timing, platform windows, DOS services, or
whole-system boot. It must stop at the first divergence and report both states,
the instruction bytes, and the bounded preceding trace.

### Cleanup And Retention

No M5 task closes with Bochx instrumentation enabled in the default product
build. Temporary direct bridge hooks, reference configuration, and raw traces
are removed after an experiment reaches a verdict. The optional research tool
and project-owned neutral trace sink may remain for later defect work, but only
behind an explicit developer build option and with no reference dependency in
the released executable.
