# M5 Verified-Axis Ahead-Only Host Pacing

## Purpose

Repair the T470 closure finding: a future verified-axis HLT path must not
advance Core immediately to a deadline in Standard mode. Core remains the
sole guest-time owner; the host may only wait when already completed Core time
is ahead of the verified profile's monotonic host-time budget.

## Required Scope

Use the existing copied `core_machine_time_observation` only. Establish one
session-local pacing baseline of host monotonic origin plus observed Core tick
origin. For a verified physical axis, Standard compares completed Core ticks
with that budget and sleeps only for the positive lead. It neither creates nor
compensates guest ticks. Turbo uses the identical Core execution/deadline path
and simply omits that host wait.

On HLT, Standard must wait until the copied, Core-selected next deadline is
within its wall-clock budget before asking Core to advance. Turbo may ask Core
to advance the selected deadline immediately. Unqualified profiles retain the
existing explicit L2 path unchanged.

## T471 Subtask Plan

1. **S1 - pacing contract and deterministic proof seam.** Freeze the exact
   completed-tick/budget comparison, reset/cancellation lifecycle, integer
   conversion and test seam. Audit T470's verified-axis branch and all host
   clock consumers; no runtime behavior change.
2. **S2 - session-local ahead-only wait.** Add the minimal high-resolution
   host observation and session pacing baseline. Route Standard's normal run
   path through the one comparison; Core APIs remain readers/advancers only
   under their existing owner rules.
3. **S3 - HLT deadline policy and closure.** Make Standard wait for a
   Core-selected verified deadline before requesting its advance, while Turbo
   omits only the wait. Prove reset, stop, pause, debugger, unavailable
   profiles and synthetic verified-axis behavior; build one stripped Release
   artifact and close the result.

## Non-goals And Stop Conditions

Do not qualify a profile, estimate rates, expose Core device state, inject
ticks from host time, add a second scheduler, replace runtime debugger waits,
or change controller timing. Stop and retain unavailable behavior if an exact
comparison would need a nominal frequency, a controller estimate, an external
source or a VM-to-Core time writer.
