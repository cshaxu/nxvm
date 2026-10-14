# PC110 F65535 Display And Japanese Font ROM

## Admission And Dependencies

Unnumbered M5 candidate in the [NXVM queue](../states/QUEUE.md), created by the
owner-requested PC110 task split. Allocate a numeric T only at admission.
Depends on: PC110 board/ROM routing and frozen display/controller documents.
Use the [PC110 programme contract](m5-pc110-evidence-and-implementation.md)
and its convergence ledger; the task packet freezes the exact batch and source
baseline before implementation. Research references are not implementation imports.

## Objective And Scope

Implement standard/extended video register behavior, VRAM/banking, panel scanout and font-ROM bank/window behavior. Reuse existing copied frame publication to the host; qualify text, graphics, palette, cursor and generation/reset semantics.

Expected owners: PC110 display/font wiring; reusable F65535 mechanism in Core only with a reviewed chip boundary; test/core and test/app-mypc110.

## Non-goals And Stop Conditions

No replacement host fonts, Personaware text repair, forced window dimensions or second renderer. Shared frame/API changes require separate owner review if a real capacity gap is proven.
Stop and report source/model conflicts, missing lawful assets needed by this
batch, or a required Shared-six modification; do not invent an undocumented
fallback. Continue independent in-scope work where possible.

## Suggested S Boundaries

Controller/VRAM; panel and scanout; font-window and product frame-path qualification.

These are planning boundaries, not allocated S identifiers. Split an oversized
batch before admission; never use letter-suffixed S numbers or an endless
first-failing-BIOS-instruction repair sequence.

## Verification And Exit

Selected panel modes produce correct frames from guest register/VRAM state; font banks are bounded and read-only, reset/boundary failures are tested. Representative framebuffer vectors and real firmware mode transitions are the evidence, not screenshots alone.

Use direct owner tests plus the real production connection, full repository
unit gates and required external integrations under Execution. Reuse the two
existing Ninja/ccache graphs, rebuilding each shared target once per host
architecture/configuration. Rebuild and deploy every affected admitted App pair;
no PC110 artifact claims runnable qualification before real-firmware proof.
Record actual source/test diff, source grades, review, unresolved scope and
artifact identities; preserve owner INI, ROM masters, disks and snapshots.
