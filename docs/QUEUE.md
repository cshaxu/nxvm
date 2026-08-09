# Queue

This is the ordered, unnumbered candidate queue. It does not activate work,
reserve a numeric task identifier, or define a technical baseline. The first
approved candidate enters [STATUS.md](STATUS.md) and receives the next linear
numeric identifier under [rules/EXECUTION.md](rules/EXECUTION.md).

The first candidates are the direct M5 80386 32-bit protected execution and
delivery package. They use focused synthetic architecture probes during
construction; a full-system checkpoint is only a debug/progress tool and never
a build prerequisite. Each task closes every 80386-legal form in its named
family, including 16/32-bit cross forms, profile rejection, fault precedence,
and pre-fault non-mutation. Reserved and later-CPU forms remain `#UD`.

For every candidate in this package, the task packet must record the relevant
Intel 80386 Programmer's Reference Manual sections, the matching local Bochs
and PCjs implementation/behavior locations and revisions, and the project
probe result. Intel defines required semantics; Bochs and PCjs are read-only
behavior references, never copy sources or replacement architecture. After
each family closes, run the owner-supplied local Windows Setup flow once and
record only the observed checkpoint, stop, or timeout. That observation does
not expand the completed family or require an immediate fix. When a probe and
the three references leave correctness uncertain, use the optional local
Bochx/Bochs paired-step bridge with declared state masks and instruction,
time, no-progress, and trace-size budgets; its source, binaries, media, and
raw traces remain outside the repository and default build.

1. **32-bit operand, address, and stack-instruction family.** Complete the
   admitted 80386 66h/67h forms for ModRM/SIB effective addresses, instruction
   fetch, general data accesses, `PUSH`/`POP`/`PUSHA`/`POPA`, `PUSHF`/`POPF`,
   `ENTER`/`LEAVE`, and string/REP accesses under 32-bit code/data/stack
   segments. Cover wrapping, limit and segment-default rules. Do not bulk-add
   unrelated arithmetic or alter paging policy.
1. **32-bit control-transfer instruction family.** Complete the 32-bit
   same-privilege `JMP`, `CALL`, `RET`, `RETF`, conditional-relative forms, and
   loop-family behavior required by 80386 protected execution, including
   operand-size variants, code-limit checks, and frame commit ordering. Do not
   add task switching, privilege transitions, or exception delivery here.
1. **32-bit descriptor-table and system-control instruction family.** Complete
   the 80386-legal 16/32-bit forms of `SGDT`/`SIDT`/`LGDT`/`LIDT`, `SLDT`/
   `LLDT`, `STR`/`LTR`, `SMSW`/`LMSW`, `CLTS`, and permitted `MOV CRx` forms,
   with exact profile/privilege/fault rules. Exclude debug registers, 32-bit
   TSS switching, paging-policy expansion, and any later-CPU opcode.
1. **32-bit interrupt and exception-entry family.** Complete 32-bit IDT
   interrupt/trap-gate admission and entry for `INT`, `INT3`, `INTO`, hardware
   interrupt delivery, and architecturally admitted faults. Cover DPL, target
   CS, gate present/type, error-code formation, frame order, and recursive
   delivery containment. Core diagnostics remain distinct from a delivered
   guest exception.
1. **32-bit `IRET` and protected-return family.** Complete same-CPL and outer
   32-bit `IRET`, and align protected `RETF` return forms with matching 32-bit
   frame, selector, flags, stack, and commit semantics. Cover failed-return
   preservation before visible state mutation. Do not implement task-return or
   virtual-8086 return here.
1. **32-bit privilege-transition instruction family.** Complete the required
   CPL3-to-CPL0 interrupt/trap/call-gate entry and matching outer returns using
   32-bit TSS `SS0:ESP0`, 32-bit stack frames, and descriptor validation. This
   task covers the whole admitted gate/return family, but excludes task gates,
   nested-task return, V86, and general task switching.
1. **32-bit exception-delivery consistency closure.** Sweep the prior families
   for error-code normalization, exception precedence, stack/cache atomicity,
   and terminal fault diagnostics. Add a bounded double-/triple-fault policy
   only when required to make an existing family architecturally coherent; do
   not invent guest recovery behavior.
1. **80386 form audit and debug capture.** Maintain a static map of
   decoder/profile forms as proven, unproven, rejected, reserved/later-CPU, or
   absent. A full-system checkpoint may record a short debug trace for the next
   issue, but no captured guest image or long-start corpus becomes a build
   fixture.
1. **CPL3 paging and user/kernel boundary.** Add only needed 80386 paging
    semantics: U/S and R/W permission checks, CR0.WP, precise page-fault bits
    and CR2, cross-page accesses, or `INVLPG`. Each addition needs mapped,
    unmapped, protection, and pre-fault atomicity probes; no PAE or host-memory
    substitution.
1. **Trace-driven 80386 system extensions.** Admit 32-bit TSS switching, task
    gates, nested-task return, LDT breadth, virtual-8086, debug registers, or
    remaining system-instruction forms only for a concrete consumer. Present
    FPU expansion remains separate.
The following four candidates establish the M6 source-locked, single-session
mantle experiment. They do not define a DLL/SDK or external ABI. Mantle
assembles one `core_machine` from typed profile and host providers supplied by
a VDM adapter; it does not own DOS behavior, host path policy, product UI, or a
VM profile. The adapter remains responsible for connecting DOS and host-facing
providers.

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
1. **Windows 3.x readiness map.** Record the latest bounded checkpoint and
    every remaining CPU, device, timing, profile, and media prerequisite; do
    not make a Windows 95 claim.
1. **M5 closure audit.** Reconcile current source, product UX, contracts,
    CMake, evidence, and open debt before any M5-close decision.
