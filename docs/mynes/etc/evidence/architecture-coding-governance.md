# Architecture And Coding Governance Evidence

## Scope And Authorization

Baseline: MyNes 9746fad. On 2026-09-19 the owner approved the explained
architecture, requested architecture/coding governance inside M0, and clarified:
all belong to the same Td and use S. This is M0 Td S2, following accepted S1;
planning remains a separate, unadmitted S candidate. No numeric T is allocated.
Evidence ownership is the coordinator; this record supports acceptance and
retains its historical scope without defining current architecture or state.

Same session performs executor self-review and coordinator actual-change review;
this is not independent multi-agent review. Applicable reusable skills were read
from the local cshaxu skills checkout: architecture-governance, coding-governance
and documentation-governance. Their method supplements repository authorities.

## Frozen Review Universe

The finite scope is the owner-approved macro architecture and coding contract:
the following rows must all have an authoritative disposition before S2 closes.
Detailed capability planning is an explicit receiver, not missing macro design.

| Approved outcome | Authority and review focus | Disposition |
| --- | --- | --- |
| Four components and dependency direction | design/ARCHITECTURE.md: complete allowed graph and forbidden bypasses. | Accepted by actual-change review; covered by documentation gate. |
| Common/Core integration | Same authority: direct public machine contract, MyNes-specific Common, no generic driver framework. | Accepted by actual-change review; covered by documentation gate. |
| State, composition and resource ownership | Same authority: owner matrix, Core hardware assembly and sole bus routing, Common runtime and App product composition. | Accepted by actual-change review; covered by documentation gate. |
| Run, pause, stop, step, reset and faults | Same authority: single-caller control state, bounded pumps, result/error distinction and recovery. | Accepted by actual-change review; covered by documentation gate. |
| ROM loading and replacement | Same authority: App path, Lib bytes, Core validation/storage, Common candidate publication and rollback. | Accepted by actual-change review; covered by documentation gate. |
| Timing, observation and teardown | Same authority: guest versus host time, explicit precision, snapshots/peek semantics, producer-before-sink teardown. | Accepted by actual-change review; covered by documentation gate. |
| C source map and vocabulary | design/CODING.md: owners, paths, names, interfaces, lib scalar/status/C facade and platform boundary. | Accepted by actual-change review; covered by documentation gate. |
| Coding discipline | rules/CODING.md: formatting, integer semantics, validation, cleanup, failure propagation and meaningful tests. | Accepted by actual-change review; covered by documentation gate. |
| Architecture review discipline | rules/ARCHITECTURE.md: explicit contract/lifetime/failure/concurrency review. | Accepted by actual-change review; covered by documentation gate. |
| Same Td with sequential S | rules/EXECUTION.md, Current and roadmap: no new T; planning stays within M0 Td. | Accepted by actual-change review; covered by documentation gate. |
| Separate planning receiver | Queue and narrowed proposal: exact capability limits, toolchains and implementation breakdown are next-S work. | Accepted by actual-change review; covered by documentation gate. |

## Similar-Issue Sweep

Review every current Markdown authority, public orientation and proposal for
old first-emulator-increment-design links, M1-as-design routing, unresolved macro
Common binding, ambiguous state ownership and claims of source implementation.
Historical bootstrap evidence remains factual at its original closure and is not
rewritten to pretend that S1 froze this architecture.

NXVM e5e32089bf7607c4ed5039ff43250c1f16d0232c is read-only style context:
src/lib/types/types_interface.h, its README, storage/file.c and base's clock
interface informed vocabulary and layout. No source unit is copied. New brace
and readability rules are deliberate MyNes conventions; compact historical
NXVM functions are not silently presented as the required new-code style.

## Verification And Acceptance

The active-packet documentation gate and its self-tests passed under Windows
PowerShell. Same-session coordinator review inspected the actual diff against
9746fad, reconciled every row above, and confirmed that only documentation and
governance changed. The final idle closure and whitespace check run before commit.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tools/Verify-DocumentationGovernance.ps1 -RepositoryRoot .
powershell -NoProfile -ExecutionPolicy Bypass -File tools/Verify-DocumentationGovernance.ps1 -SelfTest
git diff --check
git diff --cached --check
git diff --name-status 9746fad
rg -n 'first-emulator-increment-design|M1: Design|will settle|must be settled|requires design admission' README.md docs/design docs/rules docs/states docs/proposals
```

The stale-current-decision scan has no unresolved match. All frozen rows are
accepted; feature coverage, exact signatures and test-corpus selection have the
explicit separate planning receiver. No production test applies to this change.
The NXVM reference worktree remains unchanged and no local game ROM is staged.

## Delivery Boundary

No emulator source, build, runtime import, ROM execution or implementation plan
is delivered. No hardware-compatibility or runtime test result is claimed.
Delivery uses the configured-remote rule; the baseline has no remote.
The next planning candidate consumes these macro decisions. S2 closure does
not close M0 or silently execute the remaining planning subtask.

Accepted delivery subject: `M0 Td S2 P1: govern MyNes architecture and coding contracts`.
