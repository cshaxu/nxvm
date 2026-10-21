# Unplanned Debt

## High: COPX-D2 Board Receiver Contract

Owner: MyRaidenII Core COP/Board. The selected Raiden II set's audited
`copx-d2.u0313` image is a real 262144-byte input identity, but current
Product, Machine and Board deliberately have no COPX role or consumer.
M1 T7 S5 verified that the inspected L2 external reference likewise declares
the region without an explicit access contract and describes its lookup/math
purpose tentatively.

Risk: the bounded twelve-image startup route cannot qualify COPX-dependent
game behavior, including any effect such as the externally reported first-boss
issue. Adding a configuration key with no Board consumer would be false
support; any fallback lookup requires an explicitly admitted, bounded L1
contract rather than an implicit guess.

Admission: a later MyRaidenII Board/COP receiver must use an inspected
primary-manual rule (L3) or bounded external behavior contract (L2) when one
exists. If neither can establish the complete contract, it may instead admit
an explicit L1 model with stated request/index/result/reset/failure semantics
and synthetic non-ROM proof. In every case it attaches an image only at the
existing Machine-to-Board transaction and never commits external payload.

## Medium: Selected-Set Sprite Blend Fidelity

Owner: MyRaidenII Video. The Board now owns the `0x6ce--0x6cf` sprite-flags
latch and the selected set executes through the frame-copy path on both host
widths. The current compositor deliberately has no flag-dependent blend or
shadow behavior. L2 external implementations demonstrate that some sprite
colours need special compositing, but their table data is not copied into this
repository.

Risk: a normal native frame does not prove blend/shadow pixels are faithful.
A later bounded Video-fidelity task may independently model the behavior from
approved L2 observations, with synthetic pixel oracles and no imported source,
table or protected media. This does not block the completed Board/Core frame
qualification or authorize a second renderer.

## Medium: Residual COP Semantic Fidelity

Owner: MyRaidenII Core COP. T7 S65 admits the three remaining selected-set
command forms as an explicit L1 compatibility completion so that they retain
the one command route without invented outputs. Their true operands, result
publication, timing and failure contracts remain unknown.

Risk: continued execution does not establish that any game logic which depends
on a concrete result is faithful. A later receiver may replace this L1 model
only after a primary contract or independently reproducible L2 behavior model
exists; it must preserve the single classifier, pending owner and dispatcher,
and must not copy source code, tables or protected media.

## Medium: Shared Complete-Span Memory Query Classification

Owner: Shared X86 Core Machine memory query and entry-plan preflight. T3 S15's
ROM-free actual Board probe writes GPR bytes3412 at4c0h: query size2 reports
PROVIDER, size4 reports ORDINARY_RAM, while the actual size4 read returns
34120000 through the device. memory_query_physical resolves only the complete
span; actual read/write paths additionally inspect each byte when that span
declines. Thus a provider whose declared data width is1/2 can disappear from
the wider classification. Core's CPU cache-fill eligibility has its own
per-byte scan, so this is not evidence of a current V30 cache/runtime fault.

Risk: entry-plan preloads trust the complete-span classification to require
ordinary RAM; misclassification may admit a device-containing preload, even
though the subsequent data path still reaches its devices. Admission requires
explicit Shared owner review: reconcile query with the existing bytewise
route owner, preserving accepted-prefix semantics and proving entry preflight,
mixed spans and every affected consumer. Do not duplicate a query workaround
in each App or claim an actual preload corruption without a direct test.
MyRaidenII's current COP uses1/2-byte DMA cycles and does not consume wide
entry-plan preloads. S15's negative-capability ranges claim their full span
locally, but do not repair this general Shared classification issue.

## Medium: Remaining Shared CPU Test Setup Duplication Review

Owner: Shared X86 Core CPU-owned tests. S51's bounded test-growth audit removes
the confirmed four-way protected-stack bootstrap and nine-way segment compare
copies, but does not claim every84k test line has been manually reconciled.
Further normalized-body candidates include three identical GPR lookup functions
in cmpxchg/bswap/xadd, three matching seeds in gpr_push_pop/pusha_popa/
push_immediate, and matching protected-state setup in control_state/debug_state.
These affect test maintenance, not an established CPU runtime fault.

Admission: a later explicitly bounded Shared test-maintenance batch, or when
one of these owners next needs modification. Read each use and its oracle
role, reuse the existing CPU fixture only when responsibility is genuinely
identical, and preserve every independent case/assertion. Do not add a generic
test framework, move CPU-private fixtures outward, merge distinct GDT/VM86
handlers or delete receiver tests based on opcode-name overlap. The V30
transfer/nominal-transfer domains are distinct and remain intentionally kept.

## Medium: PC Integration Aggregate Selects Other Owners

Owner: Shared root test registration and NXVM aggregate runner. During S51,
CTest's complete integration inventory contains PC integration.* plus Lib
native-device rows and MyNES integration rows. The PC run-integration-tests
target depends only on PROJECT_ACTIVE_INTEGRATION_TEST_TARGETS, but
RunTestAggregate.ps1 selects the integration label without an owner filter.
Consequently an aggregate invocation can pick unrelated native requirements
or MyNES binaries which this PC target did not build. No production fault or
failure of those unrelated tests is inferred from a CTest listing.

S51 executes all registered integration.* PC rows directly for each fixed
profile, preserving the real owner suites and excluding unrelated Lib/MyNES
requirements. Admission: a later explicit Shared/NXVM test-tooling batch must
align aggregate execution selection with its actual build dependencies using
the existing registration owner, without duplicating source lists, adding a
preset parser or pretending foreign tests were executed. Verify the aggregate
against registered names with other products enabled before closure.

## Medium: Repository Unit Build Does Not Include Every Enabled Product

Owner: Shared root build aggregation and product test registration. T3 S8
finds that the repository unit runner selects every unit label, while its
build target did not depend on the later-registered MyRaidenII executable.
S8 repairs its own local registration. The same read-only inspection finds
MyNES has a separate mynes-unit-test-binaries target which is absent from the
root build-unit-tests dependency line. No MyNES test source or behavior is
changed by S8; existing binaries pass but cannot prove that a later aggregate
will rebuild modified inputs.

Admission: a Shared test-tooling task should align the enabled products'
aggregate build dependencies with the executed unit inventory, using existing
product-owned aggregates rather than duplicated source lists. Verify actual
compile/link commands after an owned test input changes. Risk is stale unit
EXEs giving misleading qualification; this is not an observed MyNES runtime
failure. The current MyRaidenII registration is repaired locally without
changing another product or Shared source.

## Medium: Shared Boundary Selftest Interrupted-Run Cleanup

Owner: Shared test infrastructure, test/test_boundaries_selftest.cmake. S49's
x64 timeout left its44-byte audio/midi/probe.c negative input behind; the next
run rejected a legal owner because it also saw that abandoned input. No process
still owned the probe; only that verified generated file was removed. No Lib
or shared-test implementation change is admitted by this CPU S. A later Shared
test-governance batch should make its owned probe lifecycle safe across an
interrupted run, with a seeded-abandoned-probe regression. Do not weaken the
dependency checker or clear an entire reusable build tree. Risk is misleading
subsequent gate failure, not an established production dependency violation.

S3 independently reconfirmed the same ownership issue in the reusable Ninja
tree: parallel verifier self-tests use overlapping generated fixture paths, so
one interrupted/overlapping run can make a later negative expectation observe
another probe instead. S3 therefore qualified its aggregate serially in each
separate architecture tree; both complete passes are valid. The later Shared
repair must give each concurrent probe a distinct owned fixture root, or
otherwise serialize the fixture owner, then prove both clean and interrupted
replays without deleting the broader cache tree.

## Resolved: Lossless Graphics Publication Contract

### Original T1 Finding And Disposition (Superseded)

Original owner: Lib KVM Window's graphics contract, with Emulator and product Machine
consumers. T1 S1 confirmed that `kvm_window_graphics_frame` has 8-bit indices
and 256 palette entries, whereas the selected board reference has 2048 palette
entries. Dimensions fit; the number of distinct simultaneous colours is not
proven to fit. Existing PC/NES qualification does not qualify this board.

Risk: a complete game frame cannot be guaranteed to cross the current contract
without lost colours. This is a public output-contract limitation, not a reason
to truncate palette indices, quantize colours or add a second presenter.

Retained proposal, NOT approved: one canonical `lib_u32` 0x00RRGGBB graphics
pixel contract, removing the host indexed-palette path. Guest palette state
would remain in each Machine/Core owner; text would be unchanged. Review Lib
frame validation/copy/render, Emulator receiving tests, PC/MyNES adapters,
memory/copy costs and complete dual-width consumer/artifact qualification.

Admission condition: explicit owner approval of a later Shared change after
the concrete interface, affected files, before/after semantics and diff estimate
are reviewed. On 2026-10-17 the owner declined approval, required no Lib change
and directed this proposal into TODO. Do not implement it under the standing
MyRaidenII goal. It remains a required full-colour release gate, not waived
by finishing CPU, board or indexed test frames. Other independent work proceeds.

### Resolution Through Accepted Shared Work

On 2026-10-17 the owner separately admitted and accepted NXVM T564 S1:
Shared `d30b69b2a`, MyNES `906e61b0c`, NXVM `b9433b31d`, closure `53e1e775d`.
That complete import provides the proposed RGB32/four-slot contract, with full
receiving dual-width proof and ten verified EXEs. This supersedes the original
non-approval above; the host palette-capacity gap is resolved without any
MyRaidenII-local Lib modification. See the
[accepted import evidence](../../nxvm/etc/evidence/t564-s1-shared-frame-import.md).
Actual RAIDEN II board palette, pixels, rotation and publication are not yet
implemented or qualified; their video task remains a required release gate.

V30, COP, video/sound and host integration remain planned Queue work, not debt
silently substituted for their implementation proposals.

## Medium: Selected-SX System-Event Completion Metadata Review

Owner: Shared X86 Core selected-80486 SMM/STPCLK event-prefix accounting.
S29's completion sweep finds two different publishers in
`cpu/cpu_instructions.c`: `_execution_refresh()` after `_smm_enter()` and
`_system_boundary()` after interrupt arbitration during STOP-GRANT release.
They can set the no-retirement delivery flag after an attempt which has also
stopped CPU execution. Unlike ordinary HALT wake, these selected-SX paths own
accepted-prefix/event_ticks accounting and existing source/L2 qualifications.

Risk: the flag's successful-delivery meaning may diverge on these failed
system-event paths. No current guest failure is established: Machine reports
the fault before consulting that flag, and V30 cannot enter either SX-only
path. Do not change their event time, accepted-prefix work or shutdown policy
merely to make their Boolean resemble the ordinary HALT path.

Admission: the next explicitly admitted Shared CPU completion/failure audit,
or before an actual consumer relies on this flag after a failed SX system
event. Reproduce failed SMI and STOP-GRANT interrupt entry through their real
providers, pin existing failed-prefix/time expectations, then qualify the
result contract on both widths. This is outside S29's ordinary V30/HALT
result batch and is not a replacement for planned V30 physical timing work.

## Medium: Older Intel LOCK Arbitration Callback Qualification

Owner: Shared X86 Core CPU and neutral Machine bus-arbitration contracts.
S45's complete lock-admission sweep confirms that the existing modern RMW
callback gate admits80486SX; the newly implemented whole-instruction gate
admits V30 only. Older Intel profiles retain their existing prefix/operand
semantics without a qualified physical lock-callback lifetime. S45 does not
silently extend their behavior or treat passing legacy timing tests as pin proof.

Risk: bus-aware consumers must not infer Intel physical HOLD exclusion from
decoder acceptance alone. No current guest corruption or new regression is
established; its relation to legacy serialized scheduling needs direct proof.
Admission: a separately admitted Shared CPU/neutral-Machine arbitration audit,
before relying on those profiles' physical exclusion. Pin each Intel primary
contract, actual memory/port/DMA interleaving and existing source/time grades,
then qualify all relevant profiles and receiving products on both widths.
This is not a transfer of V30's original V05/V14/V15 obligations or a blocker
for independent MyRaidenII work; its complete V30 time/Machine receivers remain.

## Medium: Legacy Intel No-FPU Escape Bus-Effect Audit

Owner: Shared X86 Core CPU/FPU escape and bus contracts. S46's complete
escape sweep confirms older Intel profiles still use the common
X86_FPU_ESCAPE_CONSUME_NONE path without a CPU dummy memory read. V30's
original NEC FPO1/FPO2 memory effect is implemented independently in S46;
that does not establish the exact Intel-generation bus contract.

Risk: an absent coprocessor must not be assumed to erase CPU-owned address/
read effects merely because its arithmetic is absent. No new Intel
regression or current guest failure is established. Admission: a separate
Intel-primary-source audit of8086/8088/186/286/386 ESC memory/register forms,
FPU absent/present/unsupported actions and actual bus/failure callbacks,
before changing those existing profiles or relying on their physical effects.
This is not a V30 obligation transfer or a reason to add a second ESC path.
