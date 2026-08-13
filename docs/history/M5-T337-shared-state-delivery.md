# M5 T337: Shared CPU State And Delivery Foundations

## Task Record

T337 implements the first shared-delivery rows produced by T336. S1 owns only
real-mode vector-6 invalid-opcode delivery and the affected current-gate
fixture migration. The active packet in [CURRENT](../states/CURRENT.md) defines
the complete S1 contract.

Later T337 subtasks may address only the remaining shared delivery rows
identified by T336, beginning with debug/breakpoint and common producer/frame
semantics. They are dynamically admitted after S1 evidence; they are not
pre-allocated by this record.

## S1 Result

S1 establishes the real-mode vector-6 `#UD` finalizer policy. The retained
evidence records valid-IVT frame/handler proof for primary, `0F`, operand,
profile, and LOCK producers, a failed-IVT rollback boundary, and a
configure-time inventory of every current-gate `tests/machine` source naming a
`#UD` producer or assertion. Protected and VM86 behavior remains T326 evidence.

S1 implementation `8de04d4b` and this governance acceptance were verified by
fresh GCC configuration, the strict owner smoke, documentation governance,
diff check, and 217/217 direct current-gate tests. The project wrapper target
did not emit a test failure but exceeded the local 124-second command limit;
the direct CTest run is the retained full gate result.

## S2 Result

S2 reconciles every current-gate owner affected by the S1 real-mode `#UD`
contract. The CMake inventory classifies all 71 current-gate `#UD` sources as
explicit invalid-IVT rollback, valid vector-6 delivery, or no real-mode
negative path; configuration rejects an omitted or contradictory class. The
descriptor-system and segment-selector LxS owners now apply the rollback
preflight at their concrete negative execution boundary, and the aggregate
delivery owner consequently remains green. It retains the separate
pre-instruction fault rollback, post-instruction external/TF delivery, and
post-commit task debug-trap paths; their distinct saved-IP rules are
architectural, not construction drift. Ordinary DR6/DR7 breakpoint behavior
transfers to the 80386DX package, while reset/shutdown/triple-fault policy has
a dedicated shared CPU/machine TODO. Fresh configuration, focused owners,
documentation governance, diff check, and 217/217 direct current-gate tests
pass. The T337 developer artifact is `build/output/nxvm_0_5_0337.exe`
(`vm-0-5-0337`, banner `0.5.0337`), SHA-256
`342FFC183170169729B5BDEE8F4F3512EB86FD27B4EF979D875466927B75E4F0`.

## S3 Result

S3 completed the required task-level closure audit. The original proposal's
common exception, IRQ/NMI, frame-layout, privilege/TSS-stack, rollback,
inhibition, and task-debug rows are mapped to retained T321/T326/T329/T331 and
T337 evidence. Intel-required pre-instruction, post-instruction, and
post-commit timing distinctions remain separate constructions. DR6/DR7 has
one 80386DX owner; reset/shutdown/triple-fault has one bounded CPU/machine
TODO; VME/PVI remains outside the four-profile program. The audit is retained
in [T337 S3 evidence](../etc/evidence/t337-s3-task-closure-audit.md).

## Closure

T337 is closed. Its retained proposal is
[historical background](M5-T337-shared-state-delivery-proposal.md); the Queue
now advances to the 8086/80186 profile-closure candidate. No shared delivery
row remains implicitly owned by T337.
