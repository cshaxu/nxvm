# M5 Candidate: OmniBook 300: ROM Windows, Word and Excel

Owner queued this bounded T candidate on 2026-11-19. Numeric T/S identifiers
are allocated only at admission. [Queue](../states/QUEUE.md) owns priority;
[family research and shared boundaries](m5-omnibook-emulator.md) apply.

## Objective and scope

Run original OBSETUP to establish the writable C-drive files and configuration.
Exercise the original ROM Windows launcher/module access, ROMUpdates and HP
video, mouse and power interfaces. The file ZIP is a verification/reference
view, not a substitute for the original ROM memory mapping.

Complete the required HP input/display behavior and fixes revealed by Windows.
Run Word and Excel supplied by the card environment; exercise text entry,
editing, window repaint, document/workbook save and reload. Verify persistence
across guest and emulator restarts, including expected write failures.
Measure application launch, input response and redraw using explicit budgets
set before qualification; a Windows desktop alone is insufficient.

## Dependencies

Accepted ROM DOS and performance usability gate.

## Non-goals

No host Windows loader, replacement Windows installation or 425/430 qualification.

## Completion and stop conditions

Both widths reproducibly initialize C, enter original ROM Windows and
complete the named Word/Excel interaction and persistence scenarios. Record
software identities, device coverage and responsiveness. Preserve original
media and evidence-backed models; no claim of Windows 95 or every Office feature.

At admission, plan bounded numeric S batches from the convergence ledger rather
than one S per symptom. Reuse existing tests where they cover the contract.
Missing manuals alone are not an indefinite blocker: document L1 assumptions,
L2 reference models or L3 explicit manual formulas/constants honestly. Public
interface changes follow the applicable review rules. New evidence that
invalidates a prerequisite requires a recorded design/scope decision, not a
second execution route or an unreported transfer.
