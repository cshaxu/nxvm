# 80286 Appendix-B Timing Source Reconciliation

## Purpose

Independently verify every recorded disagreement between the Intel 80286
manual's formal Appendix B opcode-clock table and prose timing descriptions.
Appendix B remains the current project authority until this work records a
more authoritative primary-source resolution.

## Required scope

Inventory every selected T357 80286 row whose table and prose disagree,
starting with `NOP` (Appendix B: three clocks; prose: two). Obtain and compare
bounded primary Intel editions/corrections, record exact pages and assumptions,
and, where feasible, run a controlled reference or measurement probe with
declared limits. For every conclusion, map affected ledger entries, timing
consumers, focused regressions, artifact impact, and compatibility outcome.

## Non-goals and stop conditions

Do not change a source-backed value merely to match a secondary emulator,
host observation, or an unbounded benchmark. Do not expand the instruction
corpus, physical bus model, or cycle-exact claim. If primary material remains
ambiguous, retain the Appendix-B allocation and a precise open debt rather
than inventing an average.

## Evidence standard

Require the exact primary-source comparison, an explicit authority decision,
affected-row/caller sweep, focused and full-gate results for any changed row,
and a retained ambiguity record for every unresolved discrepancy.
