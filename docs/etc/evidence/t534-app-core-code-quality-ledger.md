# T534 App/Core Code-Quality Convergence Ledger

This ledger is the finite disposition record for the code-quality remediation
task.  It is evidence, not a second current architecture authority.

| ID | Mechanism / affected variants | Initial evidence | Required end state | Disposition |
| --- | --- | --- | --- | --- |
| CQ-1 | App recorder command, lifecycle and Core debug observer | `record start` creates a writer, but production has no observer binding; only its smoke calls `vm_app_recorder_observe` | Remove the unsupported recorder as one complete mechanism | Repaired by S1; see `t534-s1-recorder-removal.md` |
| CQ-2 | Machine display conversion and latest-frame publication | Converter marks destination valid before rejecting a source; prior validity can survive failure | Failure-atomic conversion and one truthful publication result | Repaired by S2; see `t534-s2-presentation-single-path.md` |
| CQ-3 | Core presentation mailbox versus Common UI frame | Production publishes both; integration captures the Core-only mailbox while UI captures Common frame | One production presentation route; integration observes it | Repaired by S2; see `t534-s2-presentation-single-path.md` |
| CQ-4 | Profile resolution/provenance and board materialization | Recursive parent/field-owner resolver and generic Machine FDC/HDC board materializer remain live | Direct frozen profile plans; Profile owns board construction | Repaired by S3; see `t534-s3-profile-direct-plan.md` |
| CQ-5 | Root `type.*` host/file facade and App INI path loading | App INI bypasses Lib storage; facade retains platform/file helpers; root-directory edge case | One canonical host/file capability and correct path resolution | Pending |
| CQ-6 | Repeat App/Core audit | Initial audit is structural and key-path review, not proof of absence | Re-audit each completed batch and record every new finding/receiver | Pending |

## Completion Predicate

Every row is repaired with code/test evidence, or transferred with a named
receiver and reason.  The final audit must find no unclassified live App/Core
duplicate owner, reverse composition dependency, disconnected operation or
failure-atomicity violation.
