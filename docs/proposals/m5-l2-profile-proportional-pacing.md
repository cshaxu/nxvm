# M5 L2 Profile-Proportional Pacing

## Purpose

Replace Standard mode's fixed one-millisecond HLT poll with the only two
honest runtime outcomes: a profile may supply an immutable L2 macro rate that
limits already-completed Core progress against a monotonic host clock, or it
has no rate and both Standard and Turbo run without a host wait. Neither case
lets the host create, compensate, or skip guest ticks.

## Evidence Boundary

The 1984 IBM 5170 Technical Reference (1502243, System Board 1-6) specifies
the original 80286 at 6 MHz and describes its system-clock/bus cycles. It is
not the selected Rev-3 Model-339 configuration. MAME distinguishes 6 MHz and
8 MHz 5170 BIOS families; PCjs Rev-3 5170 configurations use 8 MHz. Thus the
already-selected Rev-3 Model-339's nominal 8 MHz source is adequate only for
an explicitly labelled L2 macro rate. It does not establish a complete
physical Core axis. The generic default PC/AT and DeskPro Model-40 have no
admitted rate in this task and therefore receive no host wait.

External emulators are corroboration only: they are not imported, copied, or
made a product dependency. The retained physical-timebase TODO continues to
own a future verified physical rate.

## Design

Add one immutable Core time-axis qualification: `MACRO_PROPORTIONAL`. It
carries a positive ticks-per-second rate, is copied at construction, and is
observable as pacing available but never as physical time. Existing physical
qualification remains stricter.

The session uses the existing single host-origin/Core-origin comparison for
both qualified pacing kinds. Standard waits only while Core's observed,
already-completed time is ahead of the chosen rate; Turbo takes the identical
Core path without that wait. On HLT, a source-qualified Core deadline is first
budget-gated in Standard, then Core alone performs the advance. A profile with
no rate does not sleep in the runner.

## T472 Subtask Plan

1. **S1 - source crosswalk and contract admission.** Record the 5170
   primary/manual and emulator corroboration, classify all three current
   profiles, and add the compact immutable macro-axis contract with focused
   validation/observation tests.
2. **S2 - one pacing path.** Generalize the existing session pacing comparison
   and HLT deadline gate to the macro qualification; preserve physical
   qualification and Core-owned advancement unchanged.
3. **S3 - profile selection and closure.** Select the Model-339's retained
   8 MHz L2 macro rate, leave unsupported rates unavailable/no-wait, sweep all
   runner sleep paths, run focused/full gates, build stripped Release 0472,
   and record the retained physical-timebase debt.

## Non-goals

Do not claim physical real time, derive guest time from elapsed host time, add
a VM scheduler, revise CPU/controller timing, select a nominal rate for a
profile without retained evidence, or alter pause/debugger control waits.
