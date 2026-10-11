# Core Code Quality Audit And Repair

## Owner Request And Admission

The owner requested a deeper Core audit, then on 2026-10-10 requested a new
T proposal at the head of the NXVM queue, with the discovered issues recorded
in S1. The owner clarified that the objective is comprehensive Core code
quality audit and repair: special cases are only one form of poor code, and
source/test dependencies, symbol naming, duplicate paths and clean architecture
must also be examined. This is an unnumbered implementation candidate;
numeric T allocation and execution require admission through CURRENT.md.

## Goal And Boundaries

Audit all production, test and build code owned by Core against its actual
ownership, dependency, interface, state-machine and failure contracts. Repair
demonstrated defects and unjustified structural complexity at their owners.
Compatibility branches are one audit dimension, not the task's definition.
Check their hardware-source and caller contracts
without deleting legitimate CPU-family, chip-personality or XT/AT wiring
differences. Boot success and implementation-mirroring tests do not establish
hardware correctness or a physical timing grade.

Core owns reusable chip and board mechanisms. The four PC Apps own their
selected compositions; MyNES is outside this repair surface. SoftPC is a
read-only sibling consumer: no sibling edit or import is authorized by this
proposal. No firmware, media, INI or protected-source change is planned.

## Audit Dimensions And Required Evidence

The inventory covers `src/core`, `test/core`, their CMake/registration,
manifests and verifiers, and incoming/outgoing caller edges. Enumerate every
chip and the x86, board-base, board-at, board-xt, machine and product owners;
generated/third-party inputs receive explicit dispositions rather than silent
exclusion. External callers are examined for boundary reach, not admitted as
an unrelated whole-repository refactor.

| Dimension | Required audit and repair evidence |
| --- | --- |
| Source and test dependencies | Record separate include, symbol-use and build/link graphs down to Core subcomponent owners. Inspect transitive targets, aggregate libraries, callbacks and fixtures for cycles, reverse/peer edges, private-header access and hidden App dependencies. Distinguish allowed same-owner white-box tests from cross-owner private-layout coupling. |
| Symbol and file naming | Inventory public/private types, functions, globals, macros, headers, targets, routes and test helpers against the established owner vocabulary. Identify ambiguous, misleading, stale or conflicting names and oversized mixed-responsibility files. Trace callers for every rename; directory movement alone does not justify changing a stable prefix. |
| Duplicate production paths | Map each operation from admission through validation, preparation, publication, execution, reset and teardown. Identify duplicate dispatch/translation/validation, wrappers without a responsibility, fallback implementations, mirrored state, multiple registries and alternate failure paths. Consolidate only where semantics and lifetime match; document genuinely distinct hardware or platform paths. |
| Clean architecture | Verify the declared DAG, composition roots, policy/mechanism separation, minimum opaque interfaces, lifetime/lease ownership and one state/time/route owner. Inspect Core-to-App policy leakage, raw mutable layouts, implicit globals and secondary scheduler/framebuffer/command paths. Prefer existing bounded contracts over new frameworks or forwarding facades. |
| Logic and failure quality | Examine error propagation, initialization, bounds/overflow, allocation and rollback, partial publication, cleanup failures, reentrancy, cancellation and concurrency. Check progress/deadline agreement and compile-warning suppressions. Large files and ignored return values are leads, not automatic defect verdicts. |
| Special cases and compatibility | Separate sourced CPU/chip/board differences from ROM/workload patches and unexplained constants. Record independent source support, timing grade, active caller reach and a retain/repair/transfer decision for every branch. |
| Test ownership and strength | Map files and behavior predicates to the component that owns them. Lower-level correctness belongs to its own unit receiver; higher-level tests may use real lower components but assert only their own behavior or relevant boundary. Remove duplicate coverage only after preserving unique predicates, variants and failure cases. Target one owner per predicate, not a mechanical one-test-per-file quota; use independent oracles and meaningful failure combinations. |

Every finding records its precise location, owner, violated contract, trigger,
severity, proof, affected callers and proposed receiving batch. Separate
confirmed defects from design concerns and unverified hardware hypotheses.
Passing static gates or current units is not a substitute for actual-code
review of the graphs and mechanisms.

## S1: Core Inventory And Convergence Ledger

S1 establishes the full source/test/build inventory and convergence ledger
across all audit dimensions before any implementation. The two demonstrated
8042 defects below are initial findings, not the whole Core audit. Record
other findings with owner, priority and later receivers so S1 does not grow
into an unbounded rewrite.

### Demonstrated Defects

1. **Reply-delay/status-poll circular wait.** In
   `src/core/chips/kbc8042/controller.c`, `read_status()` decrements the
   response-poll counter only after the response delay reaches zero, while
   `advance()` advances that delay only after the poll counter reaches zero.
   With response ticks 2 and status polls 1, command 20h still has ticks 2,
   polls 1, one pending reply and OBF clear after 100 iterations of advancing
   100 ticks and reading status. The public configuration permits both
   values; existing local tests exercise them separately.
2. **Non-executable reply reported as immediately due.** With serial delay
   3, native bytes 1Ch/32h queued, and command 20h pending with zero reply
   delay/polls, `ticks_until_event()` repeatedly returns zero although reply
   publication waits for the keyboard backlog. After 100 query/advance
   iterations, serial ticks remain 3, two bytes remain queued, the reply
   remains pending and OBF is clear. Board deadline observation translates
   zero into immediate work; scheduler observation prioritizes that over
   future deadlines. The chip defect is reproduced; a complete App hang is
   not yet claimed.

The exploratory probe compiled the current controller, keyboard and mouse
sources directly. Its ignored build output is not durable qualification:
S1 must retain independent owner-local regressions and a reproducible evidence
record before relying on these results for acceptance.

### Verification And Exit

- Retain a source/caller/evidence ledger, complete Core inventory and
  separate source/test dependency maps.
- Record the reproduced 8042 mechanism states, direct callers and the
  minimal owner-local repair design without changing runtime behavior.
- Freeze the numbered receiver plan below. A newly found issue may be added
  only with a ledger row, a bounded owner and a new linear S number.
- Run only the read-only/static and focused baseline checks needed to prove
  the inventory; no production implementation is accepted in S1.

## Frozen Linear S Plan

The task uses a single linear sequence. Each S has one mechanism owner and
one independently reviewable exit; no alphabetic child tasks or mixed-owner
"cleanup" batches are permitted.

| S | Owner and bounded objective | Exit condition |
| --- | --- | --- |
| S1 | Core inventory and convergence ledger; reproduce and design the two KBC defects. | Complete disposition ledger and frozen receiver map; no runtime change. |
| S2 | `core/chips/kbc8042`: response delay progresses independently of the visibility-poll gate. | Owner-local controller proof covers delay/poll combinations, controller/keyboard/AUX replies, OBF, IRQ and ordering on x64/x86. |
| S3 | `core/chips/kbc8042`: truthful executable deadline while serial/FIFO work blocks a reply; board deadline assertion only if required to prove forwarding. | Query/advance agreement for serial/FIFO/scan/translation/reset combinations; no false zero deadline or fabricated tick. |
| S4 | KBC caller qualification through board-at, board-base, x86 scheduler and machine waiting. | One non-duplicative cross-owner route proves the chip deadline contract reaches the scheduler; affected PC input route is qualified or explicitly unavailable. |
| S5 | Core build and warning qualification, beginning with the CPU GNU `-w`; first verify whether any claimed Debug warning exception actually exists. | Every exception is independently reproduced and retained, narrowed, repaired or transferred; no blanket warning-policy change. |
| S6 | Core test ownership and fixed-profile qualification support, including Model 40 setup/App observation edges. | Each support edge has an explicit owner and predicate disposition; no production App dependency is introduced. |
| S7 | Machine/lifecycle/failure-return candidates: runner result state, media replacement and teardown close behavior. | Each candidate is reproduced then repaired, retained or transferred with a direct contract test. |
| S8 | Board compatibility/source receivers, including Compaq CECG alias and PC/AT bounded-L1 policy. | Primary-source or explicit model-grade evidence supports retain/repair/transfer; no speculative simplification. |
| S9 | CPU/Core boundary reconciliation for compatibility timing recipes. | CPU-owned items are transferred to the active CPU authority or closed with current evidence; no duplicate CPU implementation path. |

Each completed implementation S receives a target-scoped commit, dual-width
build and focused unit evidence. Only T closure performs the complete Core and
affected-App qualification matrix required below.

### S9 Boundary Disposition

`core/chips/cpu` is the only timing-recipe owner.  Its selector chooses every
candidate source and the retained deterministic compatibility endpoint, labels
the resulting origin and source-allocation state, and returns one copied timing
result.  The Core executor consumes that result only to account for external
cycles, publish a retirement observation and enforce the already-declared
physical-time eligibility policy.  It does not select, alter or reproduce a
CPU timing formula.

The retained compatibility endpoint is deliberately deterministic but
source-unallocated.  It may support non-physical execution progress, but it
cannot qualify a physical retirement observation.  CPU-local timing-manifest
and transfer-boundary tests own exact formula/origin proof; Core observation
tests own propagation and eligibility proof.  S9 therefore makes no duplicate
Core implementation or speculative formula change.  Its evidence record must
name this split and the direct dual-width receivers.

## Later Audit Receivers And Convergence

Before the first implementation S, establish one durable convergence ledger
enumerating every Core owner/file and each audit dimension, source grades
where relevant, caller reach, independent predicates, dispositions and
remaining receivers. Include audited-with-no-finding rows and reasons for
exclusions. Sequential later S packets consume bounded owner domains or
cohesive cross-owner mechanism repairs; do not plan only around special cases
or the first two failures. Freeze numeric S allocations at admission from
the inventory, and update the ledger when deeper findings change the scope.

Initial unresolved receivers are the Compaq CECG B0000h/B8000h alias,
unconditional PC/AT bounded-L1 policy, and CPU compatibility timing recipes.
These are audit candidates, not demonstrated hardware errors. For the video
alias, existing tests prove implementation behavior but not independent
hardware provenance. Reconcile primary-source decode evidence before changing
it. Preserve explicit L1/L2/source-unallocated distinctions and physical-mode
guards for timing fallback. Each receiver must finish with a supported
retain/repair/transfer decision; deleting a compatibility branch alone is
not closure.

Retain earlier audit leads for explicit validation: media replacement and
teardown discard close errors; keyboard mapping/native-scan-set failures can
be reported as successful delivery; CPU files suppress all GCC warnings;
the runner may inspect an uninitialized result on an error return. These are
not all equally proven or equally reachable. Reproduce and check each
contract before assigning a repair verdict or priority.

## Whole-Task Exit And Review

Every inventory row and dimension has reviewed evidence and a disposition;
every confirmed in-scope defect has a repair with owner-local proof, or an
explicit owner-approved deferred receiver and residual risk. Acceptance must
name remaining debt rather than imply complete code or hardware correctness.
Review the final actual source/test/build diff for the dependency DAG, naming,
unique operation/state ownership and obsolete-path removal. Record code-size
changes and the semantic reasons for retained alternate paths.

Run the complete Core unit corpus on x64/x86 and all applicable manifest,
Types, naming, dependency, architecture and documentation gates at final
qualification. Run affected four-PC App units and required integration routes
from the change/caller matrix; rebuild and qualify product artifacts when
admitted production changes require them. Preserve existing machine/CPU
capabilities and meaningful test predicates. No weakened checks, duplicate
truth sources or unrelated cleanup can count as quality improvement.

Apply the product reading set, shared execution/architecture/coding/document
rules and source policy at admission. CURRENT.md remains the sole active
packet and baseline authority.
