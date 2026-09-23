# T395 S1 Route Registry Acceptance Audit

## Complete Disposition

| Requirement | Accepted proof |
| --- | --- |
| Bounded growth | Private Core storage grows from 12 to an explicit limit of 64. |
| Failure atomicity | Direct injected growth allocation failure preserves 12 providers/capacity; a full registry rejects provider-plus-observer registration with no VADP state publication. |
| Preserved semantics | Owned regression proves registration priority, `TYPE_STATUS_UNSUPPORTED` decline fallback, overlay behavior and frozen-registration rejection. |
| Surface sweep | Core registration, overlay, combined observer registration, ROM rollback, VADP, machine callers, test fixtures and both representative compositions reconciled. |
| Runnable verification | `vm-0-5-0395`, SHA-256 `5DBD4B50762BAD5E393CD8EFA8A0CB7E06FD9A1304A44F8993CC64915A99D4A6`; focused 4/4 and full current gates pass. |

## Acceptance Boundary

S1 accepts the generic Core route-capacity repair; task-level closure remains explicit follow-up work. It adds no DeskPro
clock, address-decode, ROM/firmware, physical-retirement or L3 conclusion. The
queued DeskPro board-level timing closure remains the earliest receiver for
selected Model-40 board contracts.