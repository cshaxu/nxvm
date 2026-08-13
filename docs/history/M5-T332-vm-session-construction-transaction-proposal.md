# VM Session Construction Transaction

## Admission Context

The composition audit found one repeated construction mechanism in
`vm_session_create`: the default-profile and caller-configured branches each
materialize the same `core_machine_config` fields. `vm_session_storage_initialize`
also repeats part of its failed-construction teardown. Existing session
initialization evidence covers profile, core, and controller failures, but not
the later image/media creation path as one explicit session-construction
transaction.

The owner approved this candidate on 2026-08-12 after the preceding Td
documentation correction. It is intentionally inserted ahead of the 80386DX
coverage audit because it repairs a cross-component construction-drift finding;
it does not redefine CPU completeness or start M6.

## Objective

Give VM session creation one reviewable private construction contract:

1. materialize profile defaults once and apply explicit caller overrides once;
2. make storage/control/provider/media failure handling one staged transaction
   with one owner-local rollback path; and
3. converge only genuinely common CPU smoke lifecycle setup in `tests/support`,
   while keeping instruction- and device-specific fixture state local.

No public configuration facade, product-visible behavior, core ownership, or
test mirror state may be introduced.

## Planned Subtasks

### S1: Configuration Materialization

Replace duplicated default/config branches in `vm_session_create` with one
private profile-derived `core_machine_config` materialization and bounded
override step. Prove default and configured session construction retain the
same machine profile timing/clock/KBC fields and the intended explicit
memory/CPU/FPU selections. Preserve the existing public `vm_session_config`
shape and create/reconfigure behavior.

### S2: Lifecycle Transaction And Failure Atomicity

Map storage, control, provider, firmware, controller, media/image, and device
creation stages. Replace only identical partial teardown with one private
staged rollback boundary. Add deterministic failure coverage for every admitted
stage, including late image/media paths, and prove no active session, core
machine, provider binding, or externally visible partial state remains after a
failure.

### S3: Test Fixture Lifecycle Convergence

Audit direct CPU smoke `create -> owner setup -> bind/freeze/reset -> cleanup`
shapes. Move only the common lifecycle tail into a setup-only helper under
`tests/support`; preserve owner-local provider/GDT/IDT/memory setup and all
test assertions. Add a narrow structural inventory/verifier and rerun every
affected owner smoke plus the full current gate.

## Boundaries And Stop Conditions

Stop for owner direction if a proposed helper would expose a public session or
core mutable layout, merge distinct device semantics, require a second product
composition path, change VM Console/debugger behavior, or reveal a core/device
runtime defect outside the private VM composition transaction. Record such a
defect as a separate bounded admission rather than hiding it in this task.

The task does not implement mantle, DOS, NXVDM, CPU instructions, device
protocol breadth, x87 execution, or a repository-wide test framework.

## Completion Standard

T332 closes only when the duplicated configuration materialization has one
private owner, every admitted session-construction failure stage has a tested
rollback/nonpublication contract, and the fixture sweep distinguishes common
setup from necessary domain fixtures. Each accepted subtask rebuilds the T332
developer artifact and passes its focused evidence, documentation governance,
and the full current gate.
