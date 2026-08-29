# M5 T500 Unit And Integration Test Convergence

| Subtask | Accepted result |
| --- | --- |
| S1 | Accepted: established the sole `test/` source-root, transient-focused policy and repository-only versus external-integration closure contract; the existing non-media baseline passes 287/287. |
| S2 | Accepted: reconciled all 302 configured CTests plus 43 unregistered test targets, including external inputs, serial/workspace properties, shared-source and generated-output decisions; the repository-only baseline passes 287/287. |
| S3 | Accepted: defined the one-tree component-owned module, route, generated-output, platform-conditional and migration policy consuming every S2 class; the repository-only baseline passes 287/287. |
| S4 | Accepted: moved all retained test sources to the single `test/` root, split the CTest registry into 287 unit and 15 integration tests, and moved timing-runner output into the build tree; unit 287/287 and static route checks pass. |
| S5 | Accepted: retained only seven narrow host-thread serial exceptions, proved four-way execution, and ran the owner-managed integration route 15/15 without tracking assets or generated output. |
| S6 | Accepted: restored the completed-proposal index after the first closure record exposed the mandatory documentation-link rule; governance then passed without re-queuing T500. |

The active [proposal](../proposals/m5-current-gate-execution-tiering.md)
defines the complete test inventory, component-owned test design, migration and
parallel-isolation sequence. T500 may not weaken coverage, retain a second
test tree or runner, or turn owner-managed external media into unit input.

The retained [S2 audit](../etc/evidence/t500-s2-test-audit.md) and complete
[machine-readable inventory](../etc/evidence/t500-s2-test-inventory.json)
are the convergence ledger for the remaining test-architecture and migration
work.

The [S3 architecture](../etc/evidence/t500-s3-component-test-architecture.md)
was the exclusive implementation plan for the completed migration.

Closure audit: all 302 original registrations remain in exactly one route;
unit and integration verification passed; `test/` is the only live test source
root; and the worktree was clean after both routes. The task adds no production
behavior or external assets. The next queued work is the four-profile media
closure.
