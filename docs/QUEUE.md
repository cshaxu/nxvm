# Queue

This is the ordered, unnumbered candidate queue. It does not activate work,
reserve a numeric task identifier, or define a technical baseline. The first
approved candidate enters [STATUS.md](STATUS.md) and receives the next linear
numeric identifier under [rules/EXECUTION.md](rules/EXECUTION.md).

The first four candidates establish a source-locked, single-session mantle
experiment. They do not define a DLL/SDK or external ABI. Mantle assembles one
`core_machine` from typed profile and host providers supplied by a VDM adapter;
it does not own DOS behavior, host path policy, product UI, or a VM profile.
The adapter remains responsible for connecting DOS and host-facing providers.

1. **Pre-decode transition gateway and mantle probe, conditional.** Admit only
   with a concrete first-party mantle consumer. Register finite decoded forms
   during configuration and freeze them before reset. A handler receives copied
   execution state and checked capabilities, and may only return unhandled,
   handled-resume, stop, or fault. It cannot mutate CPU mode, control state, or
   descriptor caches; its register/IP/FLAGS patch surface is an explicit core
   whitelist. No guest-service ABI, second decoder, or product-specific marker
   enters core.
2. **Ordinary-RAM transaction for staged runtime input.** Add a
   stopped/paused copied-value multi-span operation only after its mantle
   staging consumer and full preflight contract are explicit. It validates all
   spans as non-overlapping ordinary RAM before any write, reuses the existing
   checked route, and promises no partial write on validation failure.
3. **Transition-local combined commit, conditional.** Combine a checked staged
   RAM operation with an allowed transition CPU patch only when the admitted
   mantle consumer needs both at one boundary. Otherwise withdraw this
   candidate; do not create an unused transaction abstraction.
4. **Minimal mantle single-session closure.** Implement one policy-free mantle
   session that creates, configures, freezes, resets, bounded-runs, diagnoses,
   and destroys exactly one `core_machine`. It accepts typed profile and host
   provider bundles from internal or separately admitted external VDM adapters,
   while consuming only public core interfaces. Prove two independent sessions,
   retained NXVM regressions, no VM/private borrow, and no DOS or host-policy
   implementation in mantle.

The remaining candidates are an evidence-led 80386 capability package. A
full-system checkpoint may discover a new blocker, but ordinary development
and regression must run a seconds-scale prepared-state corpus. No candidate
claims general 80386 or whole-system equivalence from an isolated opcode.

5. **Package A: 80386 admission map and checkpoint replay.** Inventory every
   80386 decoder form as implemented-and-proven, implemented-but-unproven,
   intentionally rejected, reserved/later-CPU, or absent. Capture the next
   owned system checkpoint once, then create a bounded prepared-state replay
   containing its copied memory, descriptors, paging state, registers, and
   instruction window. Add a fixed trace/no-progress/time budget and optional
   bounded differential evidence. Do not change CPU behavior merely to make
   the map green. Exit when each future CPU change can name a replay corpus,
   and a full-system run is only a low-frequency progress check.
6. **Package B: 32-bit protected execution baseline.** Starting only from a
   Package-A replay, admit the smallest failing family among 32-bit
   descriptor/cache semantics, code/data/stack access, operand/address-size
   defaults, and same-privilege far control transfer. Each increment must
   prove selector/type/present/limit behavior and pre-fault non-mutation. Do
   not add LDT, task-gate, virtual-8086, or a generic mode switch merely for
   completeness. Exit when the replay and its next real checkpoint pass using
   an explicit 32-bit protected execution path.
7. **Package C: 32-bit exception, interrupt, and return delivery.** From a
   failing Package-B or system replay, admit one bounded IDT/gate/frame family
   at a time: exception versus software interrupt admission, error-code
   formation, privilege-stack selection, 32-bit frame delivery, and matching
   `IRET`. Preserve an explicit distinction between a core diagnostic and a
   delivered guest exception. Exit when the required replay survives its
   fault/interrupt path without a runner shortcut or hidden state mutation.
8. **Package D: CPL3 paging and user/kernel boundary.** Starting from a real
   user-mode replay, add only the needed 80386 paging semantics: U/S and R/W
   permission checks, CR0.WP, precise page-fault bits and CR2, cross-page
   accesses, or `INVLPG`. Each addition requires mapped, unmapped, protection,
   and pre-fault atomicity corpus cases. Do not claim paging complete, add PAE,
   or substitute host virtual memory. Exit when the selected user/kernel
   checkpoint is deterministic across bounded run quanta.
9. **Package E: trace-driven 80386 system extensions.** Admit 32-bit TSS,
   task gates, nested-task return, LDT breadth, virtual-8086, debug registers,
   or remaining system-instruction forms only when Package A or a later
   checkpoint identifies a concrete consumer. Each family remains its own
   candidate with a focused prepared-state corpus; present FPU expansion stays
   separate. Exit per admitted family rather than by an undefined "complete
   386" label.
10. **Windows 3.x readiness map.** Record the latest bounded checkpoint and
    every remaining CPU, device, timing, profile, and media prerequisite; do
    not make a Windows 95 claim.
11. **M5 closure audit.** Reconcile current source, product UX, contracts,
    CMake, evidence, and open debt before any M5-close decision.
