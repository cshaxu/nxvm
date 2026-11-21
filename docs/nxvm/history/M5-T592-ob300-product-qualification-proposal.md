# M5 Candidate: OmniBook 300: complete product qualification

Owner queued this bounded T on 2026-11-19. Admitted as M5 T592 after
[original DOS qualification](../history/M5-T591-S4-ob300-rom-dos-closure.md).
[Queue](../states/QUEUE.md) owns priority;
[family research and shared boundaries](../proposals/m5-omnibook-emulator.md) apply.

## Objective and scope

Reconcile the frozen machine/device ledger with all implementations and
remaining limits. Complete required power/peripheral behavior or obtain explicit
owner scope decisions; unimplemented paths cannot disappear from the ledger.
Qualify lifecycle, reset/stop/start, display switching, input release, storage
failure/persistence and documented configuration. Keep emulator pause distinct
from guest power/suspend behavior.

Integrate the App with the existing build/publication tools and source/test/
asset layout. Publish one x86/x64 pair with documented external System ROM
inputs. Follow current shared-change receiver obligations, updating the actual
product matrix when the new App is admitted; do not hardcode the old fourteen-
EXE count as the future complete matrix. Run cached required unit/integration
qualification and preserve owner configuration and media.

## Dependencies

Accepted [real-ROM machine assembly](M5-T591-S4-ob300-rom-dos-closure.md). This product
delivery provides the executable for the later performance and
ROM Windows/application receivers.

## Non-goals

No implicit full-family support, unsupported peripheral fidelity claim or redistribution of Microsoft-bearing ROM contents.

## Completion and stop conditions

A repeatable new-user setup reaches interactive ROM DOS on both widths.
Release identity, artifact hashes, test coverage and provenance are recorded.
Record responsiveness against the existing budgets and hand verified results
to the next performance stage. Original ROM Windows qualification
remains the final stage.
Manual checks and environment limitations are reported separately. Obtain
owner acceptance of the usable product and each explicit remaining limitation.

At admission, plan bounded numeric S batches from the convergence ledger rather
than one S per symptom. Reuse existing tests where they cover the contract.
Missing manuals alone are not an indefinite blocker: document L1 assumptions,
L2 reference models or L3 explicit manual formulas/constants honestly. Public
interface changes follow the applicable review rules. New evidence that
invalidates a prerequisite requires a recorded design/scope decision, not a
second execution route or an unreported transfer.

## T588 handoff

Consume the [OB300 design](m5-ob300-architecture.md), including its O01-O12
owner/proof table, external System Card contract and numeric usability gates.
The reset experiment reaches the first FBh write only; no hardware, DOS or
Windows completion is inherited from design closure. Reconcile the complete
receiving batch before selecting fixes, and reuse existing shared tests.
