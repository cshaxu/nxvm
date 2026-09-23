# M5 T332: VM Session Construction Transaction

## Scope

T332 is the owner-approved VM-composition construction-drift package admitted
from its retained [proposal](M5-T332-vm-session-construction-transaction-proposal.md).
It has three bounded subtasks: configuration materialization, lifecycle failure
atomicity, and test-fixture lifecycle convergence. Detailed completion evidence
is appended as each accepted subtask is committed.

## Boundaries

The task preserves the existing `vm_session_config` API, core/device ownership,
Console/debugger behavior, and local instruction/device fixture state. It does
not start mantle or alter CPU/device behavior except for a reproduced
composition-lifecycle defect within the admitted transaction boundary.

## S1: Configuration Materialization

S1 replaced the two default/configured `vm_session_create` branches with one
private profile materialization followed by one explicit memory/CPU/FPU override
step. The retained session initialization atomicity smoke now proves default
and explicit configuration images, profile-derived timing/clock/KBC state, and
the actual constructed core's memory/CPU/FPU values. It retains the existing
failure/recovery evidence and emits `M5:T332:S1:SESSION-CONFIG-MATERIALIZATION:OK`.

The T332 developer artifact is `vm-0-5-0332`, SHA-256
`1DF13B58B316026C233127B9407A03003C8B997221B431BB321F01E4D0C54A7E`.

## S1 Acceptance

Coordinator review found the private helper surface, explicit override set,
retained-config behavior, and actual-core observation aligned with the S1
packet. Fresh configuration, the focused session smoke, the artifact verifier,
49 specialized verifiers, documentation governance, diff check, and the full
212-test current CTest selection passed. The aggregate wrapper exceeded the
environment's 124-second no-output limit; its two complete dependencies were
run separately and passed.

## S2: Lifecycle Transaction And Failure Atomicity

S2 records the private VM session construction-stage map and converges the two
identical early-storage teardown branches at `vm_session_storage_rollback`.
It retains the necessary lifecycle and outer-create rollback owners for later
stages, then proves core, firmware, FDC, HDC, and late FDD/HDD image failures
do not publish a session and permit recovery. The detailed stage and sweep
record is [T332 S2 evidence](../etc/evidence/t332-s2-vm-session-construction-transaction.md).

The rebuilt T332 artifact remains `vm-0-5-0332`, SHA-256
`0429D3ED2D0D55CB8DB7E8BE0F89344B2FF842C1A0DD25E33A07C4FAD7463D5D`.

## S2 Acceptance

Coordinator review confirms that the early storage rollback only replaces
identical early-stage teardown, while lifecycle and outer-create rollback retain
their distinct initialized-state ownership. The focused session smoke, fresh
artifact build, 49 specialized verifiers, documentation governance, diff
check, and full 212-test current CTest selection passed.

## S3: CPU Fixture Lifecycle Convergence

S3 uses the pre-existing T317 fixed 47-owner inventory to retain the one
`tests/support` bind/freeze/reset lifecycle path, migrates five inherited
historical sources that would otherwise bypass it, and adds a narrow static
closure verifier. The residual older CPU/device fixture audit is explicitly
transferred to TODO rather than treated as equivalent without a source-to-target
and semantic inventory. The detailed classification is [T332 S3 evidence](../etc/evidence/t332-s3-cpu-fixture-lifecycle.md).

## S3 Acceptance And Task Closure

Coordinator review confirms that the five inherited sources now use the same
private support lifecycle as their wrapper targets, while the remaining direct
constructors are either pre-bind instruction-specific setup or explicitly
deferred for a separately inventoried historical audit. The fixture verifier,
affected smoke executions, fresh artifact build, 52 specialized verifiers,
documentation governance, diff check, and full 212-test current CTest
selection passed. T332 is therefore closed; its proposal is retained beside
this history record.
