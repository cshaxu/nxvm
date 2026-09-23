# Design-Stage Reconciliation Evidence

## Request And Inspection

M0 Td S4 follows MyNes 1746f42. The owner clarified that architecture/governance
must precede milestone/task/queue planning and source import. The immediately
preceding import approval is therefore deferred. Preserve its intended result:
generic Lib/Common plus their source/tests, suitable for identical reimport by
SoftPC/NXVM with their own machine-specific debugger integration.

Inspection found a clean documentation-only repository: no src/, test/ or product
build was imported. S3 changed documentation only. This correction reconciles
premature design/planning implications, not removal of nonexistent imported code.
All changes remain documents; no sibling repository or ignored ROM is changed.

## Research Carried Into The Design

The preceding read-only investigation used SoftPC
6541e43d61d9f1bd923ac0bc477186bf38e86451. Its four source/test roots had no local
diff; unrelated SoftPC edits were preserved. Lib/Common manifest and dependency
checks passed; no compilation/runtime compatibility claim was established.

| Observed source | Finding and design consequence |
| --- | --- |
| common/machine/machine_interface.h | x86 debug operation/snapshot types require separation from neutral rendezvous. |
| common/session/presentation_plan.c; common/ui/ui.c | Graphics forces Window/status routing; policy must be injected for equal backends. |
| lib/kvm-console/console.c | Graphical submission skips rendering; generic ASCII conversion is new work. |
| lib/kvm-window/win32/component.c; common/session/control.c | Focus cleanup is mouse-only and paused key releases are not delivered; reset must reach mapped state before resume. |
| common/machine/machine.c | Start invokes reset; stop is not suspend; stop/wake hooks can run on control thread; media path is not complete transactional ROM replacement. |
| common/machine/machine.c; common/session/control.c | Heartbeat is delegated, not generic pacing; session queue grows dynamically, not a proven configured bound. |
| common/session/session_interface.h; common/ui/ui_interface.h | Creation-time options do not establish live configuration transactions. |
| src/common/CMakeLists.txt; test/common/CMakeLists.txt | C17 selection and unconditional x86 debug targets need explicit compatibility/build work. |

These are evidence-backed planning inputs, not claims of implemented repairs.
The current authorities carry ownership decisions; this record cannot create a
second architecture or task queue.

## Coverage And Coordinator Review

| Requirement | Changed authority or retained evidence | Disposition |
| --- | --- | --- |
| Design first; no import | Execution gate and Current; clean source-free baseline verified. | Accepted in actual-diff review. |
| Formal planning not yet performed | Roadmap retires provisional M1/M2 headings; Queue has planning receiver only. | Accepted in actual-diff review. |
| All current architecture coherent | DAG, executor/signal, lifecycle, pacing and debug ownership reconciled. | Accepted in actual-diff review. |
| Unchanged downstream reuse | Goal, architecture and source policy distinguish neutral corpus from product adapters. | Accepted in actual-diff review. |
| Four roots under src | Source Layout maps source and tests; Coding rule points to one tree without competing paths. | Accepted in actual-diff review. |
| Equal backends and cooked UX | Existing UI requirements retained; graphics routing no longer mandates Window. | Accepted in actual-diff review. |
| No lost research findings | Planning consumes named API/build/input/renderer gaps. | Accepted in actual-diff review. |
| Authority discipline | Entry, rules, design, proposal and Current agree; S1-S3 evidence unchanged. | Accepted in actual-diff review. |

## Verification And Delivery

Active documentation gate, checker self-tests and actual-diff review passed. Final closure gate, whitespace and staged scope checks run before commit. Commands: documentation governance gate with active and
closed Current, checker SelfTest, actual Git diff review, whitespace/staged scope
checks and source-tree absence check. Same session executes and reviews; no
independent reviewer or product test is claimed. No artifacts are applicable.

Same Td, next S; no numeric implementation T or next S is allocated. M0 remains
open pending formal planning. Intended complete local delivery:
`M0 Td S4 P1: reconcile design governance before implementation`.
