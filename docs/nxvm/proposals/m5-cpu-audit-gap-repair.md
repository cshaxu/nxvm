# CPU Audit Gap Repair And Qualification

## Goal And Admission Boundary

Repair the complete implemented-family instruction/function/timing gaps found
by T544, preserving 8086, 8088, 80186, 80286 and 80386 and their sole chip/Core
owners. The owner requested this receiver first in Queue and closed T544 as an
audit, not as CPU qualification. Owner admits M5 T546 after verified T545
closure at 966249c20. S1 is accepted; Current owns the next active packet and the
[task ledger](../history/M5-T546-cpu-audit-gap-repair.md) preserves the full
transferred universe. General admission is not concrete Shared review;
each concrete repair still receives the owner's review before code changes.

The baseline is 7a759c20d and the accepted
[T544 ledger](../history/M5-T544-retained-cpu-qualification.md),
[archived proposal](../history/M5-T544-retained-cpu-qualification-proposal.md)
and [complete convergence](../etc/evidence/t544-s7-five-family-convergence.md).
The convergence's eighteen receivers and all linked S2-S6 family findings are
transferred here in full, including confirmed defects, unqualified contexts,
source conflicts and incorrect or implementation-derived regression oracles.
The receiver table below is an implementation plan, not a substitute for those
exact source pages, handler/helper paths and variant-specific dispositions.

## Coverage, Owners And Non-goals

At admission create one durable convergence ledger importing every T544 family
partition and receiver. The unit is instruction form plus generation, operand/
address/stack size, mode, privilege, reference, success/failure/delivery and
timing context. Map each finding to its repair, source decision and regression;
the 4,906 timing recipes are not an exhaustive semantic context inventory.

Shared changes belong to `src/x86/chips/cpu`, `src/x86/core` and matching
`test/x86` owners. NXVM owns its timing/decoder producers, source/gap records
and `test/ibmpc` composition regressions. Four fixed PC Apps are receiving
consumers, not places to install BIOS-specific CPU patches. Lib/Common and
MyNES implementation are outside scope; review their actual build dependency
before deciding whether any Shared change affects their artifacts. Each commit
changes exactly one approved target. Siblings remain read-only.

No new CPU model, generic device framework, parallel decoder/timing/executor,
universal undo log, board clock redesign, runtime CPU/YAML selection, x87
arithmetic implementation or unrelated controller/UX change is admitted.
80188 remains a separately evidenced absent capability; the selected PC110
486 implementation stays with its queued prerequisite. CPU ESC/WAIT, external
bus and interrupt contracts already found by T544 are included, not x87 work.

## Design Constraints

- Repair the existing owner across every affected family/form/caller; do not
  accumulate first-failure patches or copy state into App adapters.
- Preserve the original table-driven handlers and code style. Extract only
  genuinely identical semantics; remove superseded routes and wrappers.
- Separate decode/admission, execution, architectural publication and external
  irreversible effects. CPU rollback cannot undo an accepted MMIO/port effect.
- Preserve completed instruction context through asynchronous delivery; select
  one instruction timing row, then account qualified external waits once.
- Exact original numbers/formulas are L3 under their stated conditions. True
  range choices, reference models and macro ratios are L2. An unallocated
  order-only path is L1, not permission to demote an exact source.
- Resolve source contradictions through identified original evidence and, where
  justified, independent implementation/hardware evidence. Never average
  conflicting exact values or weaken an oracle merely to match current code.
- Report unupgradable L1, unresolved source authority and proposed correction
  of a false higher grade to the owner; do not silently waive a whole class.

## Initial Sequential S Plan

These are proposed numeric-only S batches, not active packets. Before each S,
confirm its complete affected variants and dependencies against the imported
ledger and obtain concrete Shared review. Evidence may split a large batch
into later consecutive S numbers; no letter suffixes or omitted receivers.
Regression corrections travel with the owning repair, not only with the final
test review. Each S exits with its complete batch disposition and full units.

| Planned S | Whole mechanism and exit proof |
| --- | --- |
| S1 | Reset and architectural images: one reset owner and generation-specific writable/readout/saved FLAGS, early FFFF:0000, 286 MSW and 286/386 cached limits. Prove first fetch and defined fields without asserting undefined values. |
| S2 | Runtime decode/admission: checked fetch/decode failures, thirteen unchecked early sites, 286 ten-byte/UD and 386 fifteen-byte/GP limits, immediate/group/memory-only and privilege admission. Prove preview and runtime variants at their one boundary. |
| S3 | Effective address and segment spans: actual address-size wrap, XLAT, real/VM/unreal, expand-down empty ranges and aggregate reference widths. Prove exact valid endpoints and failure ordering for all callers. |
| S4 | Admission versus next fetch: remove unjustified post-instruction ESP/next-fetch checks while retaining real branch/frame checks. Prove committed instruction versus subsequent fetch-fault attribution. |
| S5 | Host arithmetic/count: widen before signed shifts/multiply, unsigned bit literals, guest count versus carry-ring count, SAR and undefined-result exclusions. Prove all affected widths/forms without host-language undefined behavior. |
| S6 | Stack/frame publication: PUSHA, ENTER operand size versus SS.B, LEAVE, POP alias/discarded slots and frame failure ordering. Prove valid access and architectural rollback boundaries; do not reject unused restored ESP. |
| S7 | FLAGS privilege and return: POPF/CLI/STI/IRET, reserved bits, RF fault images/preservation and prior TF semantics. Prove generation/mode/privilege matrices; task loads remain distinct from POPF. |
| S8 | Asynchronous arbiter: NMI/INTR/debug priority, short shadows, MOV/POP SS versus LSS, mask/in-service, expiry/reset/IRET and simultaneous requests. Prove one arbiter and correct generation-specific delivery. |
| S9 | Exception delivery/shutdown: return addresses, complete ordered fault pairs, DF/error/shutdown and late task context. Prove serial handling and CPU shutdown without equating it to product stop/reset. |
| S10 | Descriptor/query/table: generation-specific types/layout, P-independent queries, null/invalid LDT, privilege priority and busy/accessed publication. Prove LAR/LSL/VERR/VERW independently of current rejection expectations. |
| S11 | Gate and outer-return: descriptor/operand/stack/TSS widths, CPL, LDT/conforming/ring1/2 and segment cleanup. Resolve retained original-source contradictions before changing disputed predicates. |
| S12 | Task transition: staged admission/commit, incoming CR3/LDT/selector/CPL, outgoing dynamic save, busy/backlink/NT, VM/debug and late fault context. Prove one transition/finalizer across CALL/JMP/INT/IRET rather than parallel patches. |
| S13 | Paging and implicit references: segment-before-page, U/S combinations, cross-page spans, CR2/error/A/D and task/descriptor/frame references. Prove preview has no architectural translation/fault side effects. |
| S14 | String/port restart: checked current element, prior completed elements, full prefix/repeat restart and irreversible I/O. Prove one REP path with explicit memory/port failure and commit ordering. |
| S15 | CPU external/NPX/bus: interruptible WAIT/TEST/BUSY, ESC references/errors, integrated 186 escape input and automatic/explicit LOCK interval. Prove the sole Core arbitration contract without host locks or BIOS shortcuts. |
| S16 | Scalar/formula/transfer timing: actual widths, 8088 transfers, odd/reference counts, 186 width/direction, 286 LEAVE, 386 conversion/segment POP/VM FS-GS allocation, branch decisions and full task matrices. Prove one selector against source-conditioned expectations, not copied old constants. |
| S17 | Retirement/external waits: keep completed decode/outcome through entry, account qualified wait/overlap once and separate instruction/delivery/compatibility progress. Prove successful, faulted and asynchronous variants at one time publisher. |
| S18 | Cross-family regression/source convergence: reconcile every imported finding, false oracle and missing predicate after the owning fixes; review remaining original conflicts and L1/false-tier claims. Prove no unowned or silently deferred member remains. |
| S19 | Final qualification: actual-change/code-quality review, full dual-width unit and original external integration suites, receiving-App artifacts and complete ledger audit. Close only to the level the direct evidence proves. |

S1-S4 establish state/admission/access seams used by later batches. S6-S9 and
S10-S13 must reconcile their shared frame/fault/task boundaries; no S declares
a caller correct while its required owner remains unqualified. S16 consumes
actual branch/transfer outcomes, and S17 consumes the completed execution and
delivery contract. S18 is a convergence check, not a repository-wide test
rewrite or a place to postpone regressions required by S1-S17.

## Verification, Artifacts And Closure

### Approved S2 Model40 Clock-Input Dependency Review

The corrected captured opcode facts expose Model40's refresh-poll sampling
dependency: existing PIT input is one per guest tick, counter 1 reloads 18,
and measured CPU poll-loop retirements total 32 ticks while every 61h sample
remains high. CPU source clocks must not be changed to evade this alignment.
The owner separately approves profile clock-input reconciliation on 2026-10-06;
this expansion does not authorize CPU timing perturbations or new shared APIs.

Original D3PE page 5 gives 14.31818 MHz OSC divided by twelve for TIMCLK;
the current App already declares a 16 MHz macro axis. The proposed primary
PIT estimate is therefore `{715909, 9600000, 0}` (OSC/(12*16 MHz)), labelled
L2 macro conversion, not verified physical L3. Audit other Model40 clock
inputs and actual counter pin sources in the same data-owner batch; an
approximately-under-8 MHz BCLK/DCLK description cannot silently justify the
current 5/16 auxiliary input. Do not guess a phase, perturb instruction ticks,
add a BIOS condition or invent a new shared clock interface without review.
The approved batch also checks RTC and media duration units against the same
existing macro axis, distinguishing service ticks from physical input clocks.

Each implementation S reads its original source pages and whole current owner,
records its similar-issue sweep, corrects/adds owner-local code-defined unit
contexts, and runs the complete repository-only unit suite on x64 and x86.
Focused selections are transient and appear only in the active packet.
Unit tests do not load external ROM/CMOS/media/YAML/font files.

T closure runs all original 58 external integration contexts through their
actual fixed bindings and INI/media, each group once unless a new failure
requires a documented diagnosis. Preserve checkpoints; boot is regression
evidence, not proof of all instruction/timing contexts. Source changes rebuild
every actually affected receiving App's optimized stripped x64/x86 pair,
deploy only the latest pair to its existing asset root and preserve owner INI.
Record source revision, SHA-256, PE width and manifest/corpus checks. A CPU-only
change does not itself authorize rebuilding or changing MyNES.

The task exits only when every imported and newly discovered in-scope member
is directly proven repaired/qualified or source-proven non-applicable; any
unresolved authority or necessary exclusion requires an explicit owner-approved
disposition with its complete receiver and a narrowed qualification claim.
Green catalog rows or boot success cannot hide pending semantic contexts.
Documentation governance and coordinator actual-diff review are mandatory.
Do not declare all-family completeness, L3 or physical-time closure beyond
the sources and contexts actually proved.
