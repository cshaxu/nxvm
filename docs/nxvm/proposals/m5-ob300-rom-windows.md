# M5 Withdrawn Candidate: OmniBook 300: original ROM Windows

The owner withdrew this candidate on 2026-11-21. No T/S identifier was
allocated. This proposal is retained as a record. [Queue](../states/QUEUE.md)
owns current priority;
[family research and shared boundaries](m5-omnibook-emulator.md) apply.

## Objective and scope

Run original OBSETUP to establish the writable C-drive files and configuration.
Exercise the original ROM Windows launcher/module access, ROMUpdates and HP
video, mouse and power interfaces. The file ZIP is a verification/reference
view, not a substitute for the original ROM memory mapping.

Complete the required HP input/display behavior and fixes revealed by Windows.
Exercise input, window repaint and file operations in the original Windows
environment. Verify persistence across guest and emulator restarts, including
expected write failures. Measure Windows startup, input response and redraw
using explicit budgets set before qualification.

## Dependencies

Accepted ROM DOS and the [performance usability gate](m5-ob300-performance.md).
[T592 closure](../history/M5-T592-ob300-product-qualification.md) hands off
x86 Standard/Off completion and real desktop/RDP verification alongside the
original Windows installation and persistence cases below.

## Non-goals

No host Windows loader, replacement Windows installation or 425/430 qualification.
The owner explicitly removed Word and Excel from the qualification scope.

## Completion and stop conditions

Both widths reproducibly initialize C, enter original ROM Windows and
complete the Windows interaction and persistence scenarios. Record software
identities, device coverage and responsiveness. Preserve original media and
evidence-backed models.

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
