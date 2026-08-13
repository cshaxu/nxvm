# T337 S2: Shared Delivery Reconciliation

## Mechanism inventory

The shared delivery owner is not a single serializer. `ExecFinal` owns
synchronous instruction faults and rollback; `ExecInt` owns post-instruction
external priority and TF; `_e_int3`, `_e_into`, and `_e_int_n` own software
origins; `_e_intr_n` owns external origin; and task switching invokes
`_e_except_n(1)` only after committing a new task. The architectural frame
layout remains selected by the existing real/protected serializers.

| Shared row | Production route | Current owner proof | Disposition |
| --- | --- | --- | --- |
| Synchronous exception frame and rollback | `ExecFinal` -> `_e_except_n` | T326, T331, T337 S1 | Complete for the admitted fault vectors; vector-specific producer work remains with its form owner. |
| `INT3`, `INT ib`, and `INTO` | `_e_int3`, `_e_int_n`, `_e_into` | `core-machine-software-int-s50-smoke`, T321 S4 | Complete shared software-origin composition; DPL/form matrices remain their instruction owners. |
| NMI, PIC IRQ, and TF priority | `ExecInt` -> `_e_intr_n` | `core-machine-hardware-delivery-s3-smoke`, T321 S3 | Complete: unmasked NMI precedes IF-qualified PIC IRQ, which precedes TF; each uses the preselected real/protected/VM86 frame. |
| TF post-instruction vector 1 | `ExecInt` -> `_e_intr_n(1)` | `core-machine-tf-db-s60-smoke` | Complete for real/protected saved-IP and rejection ordering. |
| Software-return and pending IRQ composition | `_e_int_n` / `_e_iret` / `ExecInt` | `core-machine-interrupt-return-composition-s4-smoke`, T321 S4 | Complete shared composition; task and VM86 breadth stay profile/state owned. |
| Incoming-TSS debug trap | task transition -> `_e_except_n(1)` after commit | `core-machine-task-switch-smoke`, T329 S7 | Complete as a post-commit task mechanism; it must not go through pre-instruction rollback. |
| DR moves versus hardware breakpoints | `MOV DRx` handler only | `core-machine-debug-mov-s59-smoke` | Register transfer is bounded evidence; DR6/DR7 breakpoint matching and `#DB` cause bits transfer to 80386DX. |
| Reset request | KBC -> `core_machine_cpu_execution_request_reset` -> machine lifecycle | `cpu_execution_context_smoke` | Host/machine lifecycle request exists; architectural reset, shutdown, and triple-fault policy are separate debt. |

## Shared construction conclusion

No same-semantic pair uses competing constructors: synchronous faults restore
the saved instruction state before serializer entry; external events are
post-instruction and therefore save the advanced IP; the TSS debug trap is
explicitly post-commit and therefore saves the incoming task IP. Collapsing
these into one generic `#DB`/interrupt path would erase Intel-required timing
differences. No production repair is justified by this audit.

## Exact transfers

| Residual | Sole next owner |
| --- | --- |
| DR6/DR7 matching, breakpoint types, cause bits, and ordinary vector-1 `#DB` interaction | [80386DX extended instruction and system-state closure](../../proposals/m5-80386dx-extended-state-closure.md) |
| Reset, shutdown, and triple-fault architectural policy | `TODO(Medium)`: a later shared CPU/machine lifecycle admission must define the Intel boundary and product reset contract before implementation. |
| VME/PVI and post-80386 debug extensions | Existing explicit outside-80386 boundary in the 80386DX proposal. |

## Real-mode `#UD` owner inventory

The first full S2 gate following S1 exposed stale negative real-mode `#UD`
expectations. The durable CMake inventory now contains all 71 current-gate
`tests/machine` sources that name a `#UD` producer or assertion. It classifies
each source mechanically as one of: an owner that calls the explicit
invalid-vector-6 preflight, one of four owners that proves valid vector-6
delivery, or one of ten owners with no real-mode negative `#UD` path. A
negative instruction test normally needs the deliberately invalid-IVT contract
so that it can prove producer rollback without conflating it with
interrupt-frame publication. No owner may rely on an all-zero IVT entry as an
implicit contract. The invalid-IVT preflight is part of that negative test's
initial state, so an owner captures any full-CPU rollback snapshot only after
applying it.

| Class | Owners | Required disposition |
| --- | --- | --- |
| Shared reset-fixture users | `prefix-attributes-s64`, `legacy-lock-s1`, `setcc`, `movx`, `bit-test`, `inc-dec`, `rotate`, `pushf-popf-s47`, `fs-gs-stack`, `lss-lfs-lgs`, `les-lds`, `les-lds-s41`, `pusha-popa`, `enter-leave`, `gpr-push-pop`, `push-immediate`, `legacy-sreg-stack`, `lea`, `xchg`, `sign-extend`, `moffs`, `gpr-mov`, `sreg-mov`, `movs`, `stos`, `lods`, `scas`, `cmps`, `port-strings`, `port-io-s55`, `lahf-sahf`, `direct-flags`, `double-shift`, `bit-scan`, `imul2`, `imul-immediate-s56`, `lar-lsl-s57`, `verr-verw-s58`, `dttr-s61`, `clts-s62`, `descriptor-system`, `sgdt-sidt`, `lgdt-lidt`, `80286-protected-mode`, `arpl`, `bound-s54`, `segment-selector`, `cpu-profile-gate`, `80386-paging`, `fpu-interface-s65` | Add the shared explicit invalid-vector-6 preflight only to the real-mode `#UD` negative execution path; retain any owner-installed vectors for successful interrupt/IRQ tests. |
| Direct reset fixtures | `real-mode-tick`, `instruction-timing`, `real-mode-corpus`, `cpu-fault-diagnostic` | Add the same explicit invalid-vector-6 preflight at the concrete negative execution boundary; do not alter their local reset semantics. |
| Delivery/state-specialized fixtures | `cli-sti-s48`, `hlt-s49`, `software-int-s50`, `iret-s51`, `iret-outer-s52`, `arpl-s53`, `exception-delivery-s2`, `vm-fault-outcome-runner` | Preserve the owner-specific software-interrupt, return, privilege, or VM setup and declare the negative `#UD` path independently. |

The valid-delivery class is `real-ud-delivery-s1`, `real-mode-corpus`,
`debug-mov-s59`, and `tf-db-s60`. Each source has a dedicated mechanical
marker and execution proof. The configure-time check rejects an unclassified
current-gate owner, a terminal owner simultaneously listed in another class,
or a delivery owner that loses its source declaration.

## Verification record

The retained owners above are current-gate targets. Fresh configuration proves
the 71-owner classification; focused descriptor-system, segment-selector, and
aggregate delivery owners pass; and the full current-gate passes 217/217.
Documentation governance and `git diff --check` also pass. This evidence is a
reconciliation of existing mechanism proofs, not a claim that all 80386
debug-register behavior is implemented.

T337 S1 changed the real-mode `#UD` contract from a terminal diagnostic to
architectural vector-6 delivery when the IVT entry is valid. Therefore S2
updates the two retained owners that had deliberately used an uninitialized
real IVT: the 80186/80286 `MOV DRx` profile rejection and the legacy
prefix/LOCK rejection while TF is set. Each now installs a vector-6 `HLT`
handler and proves the delivered `#UD`, restart IP, 16-bit three-word frame,
IF/TF clearing, handler progress, and no later `#DB`. This is an evidence
alignment with the shared repair, not a new debug-register or TF mechanism.
