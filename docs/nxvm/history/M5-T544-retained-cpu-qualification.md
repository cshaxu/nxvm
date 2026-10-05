# M5 T544 Retained CPU Qualification

## Admission And Baseline

Owner approved the first queued candidate on 2026-10-05. T543 is closed at
dd9af8951. This task qualifies retained CPU contracts, not another extraction.
Current owns the active packet; the proposal owns scope and initial S plan.

## Frozen Coverage Universe

The implemented public enum has DEFAULT plus 8086, 8088, 80186, 80286 and
80386. DEFAULT is a selection sentinel, not a sixth CPU. Product bindings are
My5160=8088, My5170=80286, MyDeskPro386=80386 and NXVM=80386; retained CPU-only
callers/tests continue to exercise unused 8086 and 80186.

Coverage consists of each CPU's legal decoder form and its source-defined
operand/address size, memory/register, prefix, repeat, mode, privilege and
success/fault/delivery context. F01-F14 and architectural state/delivery rows
in the existing [List 1](../etc/evidence/t512-s2-five-cpu-function-state-timing-list-1.md)
provide the named family partition, not proof that every expansion is correct.
8088 retains a distinct transfer/bus timing rule despite sharing base forms.

The five timing manifests expand to 4,906 canonical keys: 8086=1,053,
8088=1,053, 80186=616, 80286=771, 80386DX=1,413. This is a timing catalog,
not the number of instructions or complete semantic test contexts.
The larger opcode/ModR/M inventories are an admission mask, not semantic proof.
Each family S must reconcile its whole source-defined form/context batch,
including source-valid forms absent from the current decoder.

## Dispositions And Completion Predicate

- Accepted: direct current source-to-code-to-regression proof with exact context.
- Non-applicable: source-defined absence, with reason and negative admission proof.
- Pending: implementation/source/context reconciliation still required in T544.
- Retained non-eligible: named missing capability with explicit receiver.
- Blocked: authority or implementation prerequisite, reported without a grade claim.

No accepted family may hide a pending form behind a later boot checkpoint.
T closure requires every implemented-family unit to be accepted or source-proven
non-applicable; any approved transfer must name its complete batch and receiver
and narrow the claimed qualification accordingly. Complete units, external
integration and current affected dual-width artifacts are closure evidence,
not a substitute for source/function/timing reconciliation.

## S1 Inventory Batch

Tracked inventory contains nine CPU implementation/header files and 90 CPU
test files (81 C sources, nine fixture headers). File counts identify the
surface to read, not the number of independently proved instruction forms.
The four profile bindings were checked directly, not inferred from EXE names.

| Surface | Actual owner/evidence | S1 disposition and receiver |
| --- | --- | --- |
| Models and fixed bindings | x86/chips/cpu/cpu_interface.h; four App profiles | Inventoried; all five retained, no new model introduced. |
| Decoder/handlers, state and delivery | x86/chips/cpu/cpu_instructions.c and cpu.c; CPU-only test/x86/chips/cpu | Pending shared-boundary S2 and complete family S3-S6 source reconciliation. |
| Timing selection/model | x86/chips/cpu/cpu_timing.c and cpu_timing_model.c | Pending family S3-S6; one selector retained, no board-side CPU model. |
| Time publication and fault non-retirement | x86/core plus test/x86/core and PC composition regressions | Pending S2 cross-family boundary audit. |
| Timing catalogs/results and decoder producers | docs/nxvm/etc/cpu-timing, tools/nxvm, cmake/nxvm/NxvmProduct.cmake; test/ibmpc/board-common/composition | Structurally counted; fresh full units pass on both widths. Family-specific manual/state proof remains S3-S6. |
| Original source identity | [T512 source record](../etc/evidence/t512-s1-five-cpu-source-cross-validation.md) | Prior identity/citation index retained; page/form revalidation pending appropriate family S. No fresh manual-reading claim. |
| Prior state/code gap and tier results | T512 List 2 and [final audit](../etc/evidence/t512-s9-five-cpu-final-tier-owner-audit.md) | Historical accepted results, not current-source whole-family proof. Reconcile after chip/Core ownership changes. |
| 80188 | No implemented public enum; no selected product | Retained non-eligible; independent evidence/implementation requires separate owner admission, not an 80186 alias. |
| 486 / PC110 | No implemented public enum or App | Retained non-eligible; queued [PC110 evidence/component receiver](../proposals/m6-pc110-evidence-and-implementation.md) must select and implement the actual variant. |

### Structural Catalog Observation

Verify-CpuTimingManifestContract.ps1 reports 4,906 keys and 4,092
nonconforming template statuses. These are retained planning/status snapshots:
the exporter marks current_ticks as not-observed. They are not 4,092 demonstrated
runtime failures or L1 instructions. Generated result files and their verifiers
are separate evidence. T512's final historical counts remain historical; neither
observation alone changes current timing grades.

### Fresh Runtime Catalog Baseline

The x64 complete unit run regenerated all five timing/decoder result files.
These counts describe executed catalog recipes, not an independent manual
oracle or proof for unenumerated state contexts:

| CPU | Timing keys | Reported L3 | Reported L2 | Failed/unallocated | Lexeme opcode/ModR/M candidates |
| --- | ---: | ---: | ---: | ---: | ---: |
| 8086 | 1,053 | 989 | 64 (Group 3 model) | 0 | 57,926 |
| 8088 | 1,053 | 989 | 64 (Group 3 model) | 0 | 57,926 |
| 80186 | 616 | 580 | 36 (midpoint) | 0 | 61,530 |
| 80286 | 771 | 771 | 0 | 0 | 61,803 |
| 80386DX | 1,413 | 1,411 | 2 | 0 | 63,021 |

The result schema's L2 labels remain unchanged. No new L1 or tier downgrade
has been demonstrated by this inventory. The executable CPU admission mask,
source-legal forms, timing key contexts and architectural-state/delivery
contexts are distinct universes and must not be conflated.

### Exact Sweep And Planned Proof Boundary

The read-only sweep used `git ls-files src/x86/chips/cpu`,
`git ls-files test/x86/chips/cpu`, `rg --files` for CPU timing/decoder records,
and `rg -n` for profile bindings, metadata admission, timing selection,
source_timing_unallocated and successful retirement in CPU/Core. Actual
public enum, fixed App selections, reset-code-base switch, minimum-CPU
metadata gate, timing selector and Core publication seam were inspected.

There is one chip-owned timing selector and one Core-owned successful-time
publication seam. Core resolves DEFAULT to 80386. Their structural presence
does not qualify every caller: S2 must cover all reset/delivery/retirement
contexts, and S3-S6 must prove every source-defined family expansion.
Historical old paths are retained as historical evidence; only the live
proposal's stale owner/deployment assumptions were corrected. S1 added no
synthetic test/decoder or alternate source table.

### Verification And Review

S1 complete repository-only units pass once per width: x64 506/506 in
63.27 seconds; x86 506/506 in 60.58 seconds. Both used RunTestAggregate.ps1
with the existing default receiving cache, four jobs and a 300-second deadline.
Both generated result sets contain the same five counts and no failed or
unallocated recipe. The catalog structural check, documentation governance,
changed-document relative links, 16-field packet check and git diff --check pass.

Production source, test bodies, manifests, assets, INI and MyNES remain
unchanged; no new EXE is needed for this inventory/design-only S. Source/test
code diff is zero. Full external integration and new artifact production are
reserved for actual runnable changes/T closure, not fabricated design proof.
Incremental baseline caches are retained for the next qualification S.
Executor P1 is 742c31f23, pushed to origin/master. Coordinator actual-change
review checked all five changed documents against the immutable S1 packet:
five CPU enum/bindings, finite catalog/context distinction, exact source/test
owners, deferred 80188/486 prerequisites, initial receiver partition and both
fresh complete suites are present. Only NXVM documents changed. Skills informed
the one-owner boundary and rejection of a duplicate decoder/timing path.
The entire S1 inventory batch is accepted; unresolved manual/state proof stays
explicitly pending in S2-S6, not silently qualified. S1 is closed; T544 is open.
No owned test process remains active. The next S requires its own packet.
