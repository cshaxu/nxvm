# M5 T584 — MyPC110 desktop and interactive product qualification

Status: closed after S1–S7. The owner directed completion of the first four PC110 queue
receivers after the stage closure of T582. This is the second receiver. Its
[proposal](M5-T584-pc110-product-interaction-proposal.md) defines the retained
interactive, lifecycle, input and environment boundaries. T583 first-Setup
evidence and its inconclusive initial DSTN x86 teardown observation are in the
[coverage ledger](../etc/evidence/t583-first-setup-ledger.md).

Actual products on both widths and both firmware variants reached Personaware,
used World Clock and completed pause/resume. Window-mode DSTN x64 and TFT x86
also navigated to Calculator and evaluated `1 + 2 = 3`. `display=console`
exercised both console-control policies, graphical handoff and normal exit.
All four Product-factory/Machine cases completed guest four-corner calibration
and retained CMOS calibration across independent restart. Native keyboard
messages and Machine-injected pointer events are not physical host input.

The complete x64 repository unit suite passed 712/712, the complete x86 suite
passed 712/712, and the PC110 firmware integration suite passed 9/9 on each
width. A first x86 unit run had two timeouts; focused and full serial reruns
passed without changing budgets. Task-owned observer errors, corrections and
the earlier inconclusive T583 exit remain in the ledger. Documentation and
actual diff were reviewed; T584 changed only NXVM governance/evidence and one
obsolete UI description, not production, Shared code, tests or EXEs.

The host had no active RDP session, and switching or clipping the user's
desktop was outside the authorized background observation. Physical
Ctrl+Alt chords, real host mouse capture and interactive RDP remain unverified
and transfer to final product qualification. T582's startup performance and
the separately recorded X86 Core masked-access debt remain open. Closure does
not represent those items as passed or claim whole-product qualification.
