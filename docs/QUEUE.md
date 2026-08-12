# Queue

This is the ordered, unnumbered candidate queue. It does not activate work,
reserve a numeric task identifier, or define a technical baseline. The first
approved candidate enters [STATUS.md](STATUS.md) and receives the next linear
numeric identifier under [rules/EXECUTION.md](rules/EXECUTION.md).

Before M6 admission, complete the remaining M5 candidates. The completed
VM86-to-protected delivery prerequisite lets the ordered candidates below begin
with its direct consumer. The bounded LGDT/LIDT load work retains its non-VM86
boundary until that consumer re-admits the VM86 case.

Every 80386DX candidate is matrix-driven rather than Windows-demand-driven.
Before admitting a task, audit its relevant Intel 80386 PRM instruction and
behavior forms and record each form as complete, partial, missing, outside
80386, or an explicit external-coprocessor boundary. A task may close only its
declared bounded matrix slice; it cannot claim its family complete while an
in-scope form is partial, missing, or unclassified. A family candidate advances
only when its whole assigned matrix is resolved by implementation and focused
evidence, or by an approved boundary classification.

The [80386DX closure map](etc/evidence/80386-closure-map.md) is the supporting
crosswalk between those horizontal form matrices and the vertical architecture
state closures below. Every admitted 80386DX candidate identifies the map rows
it advances and any residual row it transfers. A completed smoke count or
opcode slice is never a substitute for a closed state transition.

1. **P0 - 80386DX exception, interrupt, and processor-control closure.**
   Complete remaining architected fault, trap, interrupt, return, CR0/CR2/CR3,
   descriptor-table load, and coprocessor-interface behavior without
   implementing an 80387. Re-admit the VM86 LGDT/LIDT `#GP(0)` no-source-read
   proof only after the preceding delivery foundation is complete.
1. **P1 - 80386DX protection and privilege-transfer closure.** Complete
   descriptor, segmentation, privilege, gate, and user/kernel transfer
   behavior required by the Intel 80386 architecture. This consumes the proven
   exception/TSS boundary rather than duplicating it in individual instruction
   tasks.
1. **P1 - 80386DX paging and translation closure.** Complete the remaining
   non-PAE 80386 paging, protection, invalidation, and diagnostic behavior.
   Admit after the exception and privilege paths can report `#PF` and
   protection failures reliably.
1. **P2 - 80186/80286 legacy LOCK-prefix legality matrix.** Resolve the
   retained cross-cutting `PREFIX_LOCK` debt with Intel profile-by-profile
   legality rules, a complete affected opcode/ModRM matrix, valid-memory and
   invalid/register coverage, and retained 80386 regressions. It is ordered
   after the high-ROI VM86/exception/protection foundation because it changes
   shared legacy prefix behavior; it must not be special-cased per opcode.
1. **P2 - 80386DX task, local-descriptor, virtual-8086, and debug/test-register
   closure.** Complete the remaining 80386 system-state families, including
   the VM86 instruction breadth unlocked by the delivery foundation; do not
   use a missing product consumer to withdraw a family from this approved
   program.
1. **80386DX architecture-coverage closure audit.** Reconcile every in-scope
   Intel 80386 architectural form and behavior with implementation evidence,
   a focused regression, or an explicit external-coprocessor boundary.
1. **Windows 3.x readiness map.** Record the latest bounded checkpoint and
   every remaining CPU, device, timing, profile, and media prerequisite; do
   not make a Windows 95 claim.
1. **M5 closure audit.** Reconcile current source, product UX, contracts,
   CMake, evidence, and open debt before any M5-close decision.

After M5 closes, the following four candidates establish the M6 source-locked,
single-session mantle experiment. They do not define a DLL/SDK or external ABI.
Mantle assembles one `core_machine` from typed profile and host providers
supplied by a VDM adapter; it does not own DOS behavior, host path policy,
product UI, or a VM profile. The adapter remains responsible for connecting DOS
and host-facing providers.

1. **Pre-decode transition gateway and mantle probe, conditional.** Admit only
   with a concrete first-party mantle consumer. Register finite decoded forms
   during configuration and freeze them before reset. A handler receives copied
   execution state and checked capabilities, and may only return unhandled,
   handled-resume, stop, or fault. It cannot mutate CPU mode, control state, or
   descriptor caches; its register/IP/FLAGS patch surface is an explicit core
   whitelist. No guest-service ABI, second decoder, or product-specific marker
   enters core.
1. **Ordinary-RAM transaction for staged runtime input.** Add a
   stopped/paused copied-value multi-span operation only after its mantle
   staging consumer and full preflight contract are explicit. It validates all
   spans as non-overlapping ordinary RAM before any write, reuses the existing
   checked route, and promises no partial write on validation failure.
1. **Transition-local combined commit, conditional.** Combine a checked staged
   RAM operation with an allowed transition CPU patch only when the admitted
   mantle consumer needs both at one boundary. Otherwise withdraw this
   candidate; do not create an unused transaction abstraction.
1. **Minimal mantle single-session closure.** Implement one policy-free mantle
   session that creates, configures, freezes, resets, bounded-runs, diagnoses,
   and destroys exactly one `core_machine`. It accepts typed profile and host
   provider bundles from internal or separately admitted external VDM adapters,
   while consuming only public core interfaces. Prove two independent sessions,
   retained NXVM regressions, no VM/private borrow, and no DOS or host-policy
   implementation in mantle.
