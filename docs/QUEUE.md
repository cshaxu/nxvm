# Queue

This is the ordered, unnumbered candidate queue. It does not activate work,
reserve a numeric task identifier, or define a technical baseline. The first
approved candidate enters [STATUS.md](STATUS.md) and receives the next linear
numeric identifier under [rules/EXECUTION.md](rules/EXECUTION.md).

Before M6 admission, complete the remaining M5 candidates:

Every 80386DX candidate is matrix-driven rather than Windows-demand-driven.
Before admitting a task, audit its relevant Intel 80386 PRM instruction and
behavior forms and record each form as complete, partial, missing, outside
80386, or an explicit external-coprocessor boundary. A task may close only its
declared bounded matrix slice; it cannot claim its family complete while an
in-scope form is partial, missing, or unclassified. A family candidate advances
only when its whole assigned matrix is resolved by implementation and focused
evidence, or by an approved boundary classification.

1. **Test-corpus quality corrective.** Repair the project-owned CPU smoke corpus before
   new 80386 feature work: target-local strict GCC coverage, project-type
   vocabulary, test-only fixture consolidation, and a package audit. This is a
   bounded test/governance repair; it does not redefine or remove any 80386DX
   capability candidate below.
1. **80386DX ordinary execution and flag-completeness foundation.** Complete
   the remaining ordinary instruction, operand-size, address-size, flag, and
   string/control families against the Intel 80386 architecture. Reuse an
   existing helper only when its callers are covered; introduce an abstraction
   only after the affected callers have focused coverage and the shared
   responsibility is concrete.
1. **80386DX exception, interrupt, and processor-control closure.** Complete
   remaining architected fault, trap, interrupt, return, CR0/CR2/CR3, and
   coprocessor-interface behavior without implementing an 80387.
1. **80386DX protection and privilege-transfer closure.** Complete descriptor,
   segmentation, privilege, gate, and user/kernel transfer behavior required
   by the Intel 80386 architecture.
1. **80386DX paging and translation closure.** Complete the remaining
   non-PAE 80386 paging, protection, invalidation, and diagnostic behavior.
1. **80386DX task, local-descriptor, virtual-8086, and debug/test-register
   closure.** Complete the remaining 80386 system-state families; do not use
   a missing product consumer to withdraw a family from this approved program.
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
