# Shared Input Reset And Equal Presenter Handoff

## Goal And Dependency

Consume [lifecycle](shared-lifecycle.md) and the [M1 context](first-emulator-increment-planning.md).
Preserve Common's existing mode switching from published frame kind and display
settings, and qualify safe input/raw-cooked transitions for both presenters.
No ASCII conversion, Lib implementation change or producer-format negotiation
is part of this candidate.

## Scope And Proof Batches

Use fake machine providers to publish complete graphic and text frames through
the real Common route. A graphic frame may activate Window according to existing
policy; text follows the existing Console/display policy. MyNes later selects
Console by making Core publish converted text, not by teaching Common about NES
or requiring it to request a different representation. Preserve optional graphical
Console status behavior for other consumers; it cannot count as ASCII gameplay.

Freeze a routing matrix over existing display settings, frame kind/availability,
machine state and raw/cooked ownership. Cover graphics-to-text and text-to-graphics
transitions, pause/resume, stale generations and switching without reload.
Preserve content/extent/sequence/generation together and reject stale completions.
Verify text delivery and invalid graphic-to-Console submission against the selected
unchanged Lib. If the required rejection/character-map contract is absent, report
the upstream prerequisite and block that proof; do not patch Lib locally.
There is no requirement to replace graphics-driven Window selection with a new
capability negotiation service. Repair only evidenced Common contract gaps.

Freeze reset scenarios for focus loss, source loss, replacement, hotkey-prefix
cancellation, pause/resume, remapping generation, close and shutdown. Release
Lib key/chord state, accepted session sources and driver-mapped state in order,
including while paused and before resume. A source reset is not permanent retirement;
one source's release must not clear another source's keys. Reject stale queued makes.

Broker replacement retires/joins the old reader, applies native mode and publishes
the new binding; failure restores the old binding or reports terminal failure.
Prove one native reader, cooked buffer/cursor preservation, complete-line consumption
once, notification cancellation and prompt rearming, callback lifetime and held-key
cleanup on both native backends. Cooked text never reaches guest input. Configuration
mechanisms validate before publication; product mappings/grammar remain outside Common.

## Acceptance And Stop Conditions

Complete the routing/reset/failure matrices with fake providers and relevant native
Window/Console tests on both architectures. Verify there is no alternate dispatcher,
native reader or product-name policy branch. Run the common completion contract.
Host tests that cannot run remain blocked, not silently skipped acceptance.
Stop if proof requires NES input/PPU or ASCII sampling: those are M3 outputs.
Hand off verified frame-kind mode transitions, text-only Console proof and any
unresolved upstream-baseline prerequisite to
[transfer qualification](shared-transfer-qualification.md).
