# M5 T319: LGDT/LIDT History

## Scope And Implementation

T319 S1 closes only Intel 80286/80386 non-VM86 `LGDT m16&24/m16&32`
(`0F 01 /2`) and `LIDT m16&24/m16&32` (`0F 01 /3`).  It neither claims a
complete processor-control family nor changes the external-coprocessor,
VM86-delivery, paging, task-switch, or legacy LOCK-policy boundaries.

The local `INS_0F_01` `/2,/3` paths now reject non-VM86 protected CPL>0
before ModRM/source-memory processing.  This corrects the prior ordering in
which the helper-level privilege guard occurred after the pseudo-descriptor
had already been read.  `_s_load_gdtr` and `_s_load_idtr` retain their guards
for their other callers; `/0,/1` remain the T318 stores and `/4,/6` remain the
T316 S63 MSW forms.

`tests/machine/core_machine_lgdt_lidt_smoke.c` is the standalone owner smoke,
registered once as `current.core-machine-lgdt-lidt-smoke`.  Its marker is
`M5:T319:S1:LGDT-LIDT:OK`.  The detailed form matrix, source/table atomicity
record, and direct-caller sweep are in
[T319 S1 evidence](../etc/evidence/t319-s1-lgdt-lidt-matrix.md).

The accepted matrix proves both forms across 80286/80386 real and protected
CPL0 operation; 80386 `66`, `67`, and combined layouts; DS, SS-default, and
ES-overridden sources; 80186/profile, memory-only ModRM, and 80386 LOCK
rejection; protected source-limit atomicity; and no-shadow PIC ordering.  A
post-load consumer is proved for each table: the loaded IDT supplies the real
IRQ0 vector after `LIDT`, while `LGDT` is followed by `MOV DS,AX` resolving a
descriptor that exists only in the newly loaded GDT.  A local protected
fixture proves both CPL>0 `#GP(0)` paths before source read or table
publication.

## Verification And Artifact

- Implementation source commit: `e17531ea` (`M5 T319 S1 P3: complete
  non-VM86 LGDT LIDT loads`).
- Fresh `mingw-gcc-x64` configuration, focused owner target and marker, exact
  current-gate discovery, documentation governance, and `git diff --check`
  passed before the implementation commit.
- Full current gate: 196/196 CTests passed with `-j 4`.
- Current developer artifact:
  `build/output/nxvm_0_5_0319.exe`.
- SHA-256:
  `5F9E0AEBFEE90E8C8F5337BA48E86BC699935AE1AAAEB517E07EBCD286476ABF`.
- Runtime identity: `Neko's x86 Virtual Machine`; current target build version
  is `0.5.0319`.

## Retained Transfers

VM86 `LGDT`/`LIDT` rejection and delivery are not tested or claimed here. They
depend on the queued VM86-to-protected exception and interrupt delivery
foundation, including the TSS-selected CPL0 stack and frame semantics.  The
80286 LOCK legality question remains the separately queued legacy LOCK-prefix
matrix.  These are transfers, not exemptions from the processor-control
program.

## Ordinary-Mode Acceptance

After the pushed implementation P3, the coordinator independently reviewed
the local handler ordering change, both bounded table consumers, the owner
smoke, CMake target/registration and strict compile command, active packet,
matrix evidence, artifact wiring, and retained transfers.  The focused marker,
exact registration, artifact-target verifier, artifact rebuild/hash,
documentation governance, diff check, and full 196-test gate all passed.
T319 is therefore closed at this bounded non-VM86 table-load boundary.
