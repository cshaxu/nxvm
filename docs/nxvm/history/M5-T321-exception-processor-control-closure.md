# M5 T321: Exception And Processor-Control Closure

## Bounded Scope

T321 completes the former P0 80386DX exception, interrupt, and
processor-control composition candidate.  It does not claim all 80386 forms
or all processor-control state.  The detailed crosswalk and transfers are in
[T321 S6 evidence](../etc/evidence/t321-s6-processor-control-closure.md) and
the current [80386DX closure map](../etc/evidence/80386-closure-map.md).

## Delivered Composition

- S1 audited active producers, retained form matrices, and their required
  successor slices before source changes.
- S2 added exact `#DE`, `#PF`, and `#MF` vector selection in `ExecFinal` and
  proved real/protected producer frames with
  `core-machine-exception-delivery-s2-smoke` plus retained owner smokes.
- S3 proved NMI priority over IRQ0 and TF across real, protected, and VM86
  delivery with `core-machine-hardware-delivery-s3-smoke`.
- S4 proved software `INT` to `IRET` to pending-IRQ composition with
  `core-machine-interrupt-return-composition-s4-smoke`.
- S5 made VM86 `LGDT`/`LIDT` reject with `#GP(0)` before ModRM or source-memory
  access and proved its exact VM86 error frame using
  `core-machine-vm86-lgdt-lidt-s5-smoke`.
- S6 reconciled the retained `CLTS`, MSW, descriptor-table, and ESC/WAIT CPU
  interface matrices with the delivered exception/event paths and documented
  every remaining transfer.

## Artifact And Verification

- Implementation and artifact-configuration commit: `606157ca`
  (`M5 T321 S6 P1: reconcile processor control closure`).
- Current developer artifact:
  `build/output/nxvm_0_5_0321.exe` (`vm-0-5-0321`, build version `0.5.0321`).
- SHA-256:
  `1F3718072E0AAA05AA12510299DF191CB9F56FD7F746DC2C939D7BB3DCEE9374`.
- Runtime identity: `Neko's x86 Virtual Machine`; the console executable has
  no non-interactive version switch, so the CMake build version and retained
  product banner identify the developer artifact.
- Fresh `mingw-gcc-x64` configuration, `verify-current-artifact-target`,
  documentation governance, and `git diff --check` passed.  The complete
  `current-gate` CTest label passed 202/202 tests with `-j 4`.

## Retained Transfers

The first remaining Queue candidate is the **80386DX ordinary execution and
flag-completeness foundation**, which receives T316's residual primary-form,
stack, string, port-string, and EFLAGS breadth.  Protection/privilege and
paging remain separate P1 candidates.  Task/LDT/debug/VM86 breadth, legacy
80186/80286 LOCK legality, VME/PVI, broader x87 execution, and Windows
readiness retain their explicit Queue or TODO destinations.  These are
transfers, not exemptions from the owner-approved architecture program.

## Ordinary-Mode Acceptance

The coordinator independently reviewed the T321 S2--S6 production, CMake,
owner-smoke, evidence, artifact, Queue, closure-map, and Status changes
against the active packet.  No unclassified T321-owned composition row
remains.  T321 is therefore closed at this bounded architecture-state
boundary.
