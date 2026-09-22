# M5 T534 App/Core Code-Quality Remediation

The owner admitted T534 on 2026-09-21 to repair the whole-project quality
audit's confirmed App/Core defects, repeating the audit until the frozen
App/Core corpus has an explicit disposition for each finding.  The retained
candidate proposal is [App/Core code-quality remediation](../proposals/m5-app-core-code-quality-remediation.md).

## S1: Remove Unsupported Recorder

S1 removes the unconnected instruction recorder as a complete mechanism.  It
does not replace it, add a tracing abstraction or modify Debug semantics.
Closure requires no production or test recorder route, no stale build/policy
claim, complete repository-only unit proof and actual-diff review.
