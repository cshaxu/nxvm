# T399 S4 B4 Acceptance Audit

The coordinator reviewed the exact PCjs DeskPro configuration and its HDC
reference, current B4 packet, Core HDC and Model-40 tests, and actual worktree.
The configuration's two generic AT Type-1 10.16 MB drives conflict with the selected
Compaq WD 40 MB route, so generic behavior cannot normalize B4. The complete batch is
therefore honestly rejected rather than silently substituted.

The three focused CTests pass and documentation governance passes. B4 is
accepted as `M5:T399:S4:B4-REFERENCE-REJECTED`; it retains only the existing
logical route and transfers all timing/media claims to the named TODO receivers.
It makes no physical or reference-derived L3 claim.