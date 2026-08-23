# M5 VM Native Display Resource Lifetime Repair

## Purpose

Repair native display resource lifetime and thread-affinity defects in the
existing Windows and Linux adapters without merging their host policies.

## Required Scope

Pair every Win32 `GetDC` with the owning `ReleaseDC` on all paths. Ensure the
Linux curses lifecycle, including `endwin`, runs on the thread that initialized
it, with orderly error and shutdown handling. Preserve the current copied-frame
and exclusive-surface boundaries.

## Dependencies And Completion

Completion requires adapter-local error/cleanup tests or deterministic seams,
static ownership sweeps for the relevant APIs, supported-host build checks, and
current gates available in the admitted environment.

## Non-goals And Stop Conditions

Do not introduce a cross-platform renderer, alter guest display timing, add
host support claims, or access native APIs from Core. Stop where a required
native runtime test environment is unavailable and record the exact retained
verification boundary.
