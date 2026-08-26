# M5 Core Physical Time Axis And Standard Pacing Qualification

## Purpose

Establish the one evidence-backed Core physical-time axis required before a
profile may claim that one guest second equals one host-wall-clock second in
Standard mode.  T469 correctly removed VM host-to-guest tick injection and
retained `UNAVAILABLE` for every current profile.  This task supplies the
missing qualification path; it must not relabel a nominal CPU MHz, a PIT input
clock, an instruction count, or the existing mixed `elapsed_ticks` value as a
physical timebase.

## Required Scope

Core remains the sole owner of guest progression.  CPU execution outcomes and
qualified controller deadlines settle on one monotonically advancing physical
axis.  VM reads only copied Core progress and a validated immutable profile
timebase, then waits only when Standard mode is ahead of its monotonic host
budget.  Turbo uses the identical Core progression and omits that wait.

The physical axis covers every path that consumes or waits guest time:

1. CPU normal completion, faults, interrupt acceptance, halt, and external
   memory/port transactions;
2. source-qualified controller deadlines, reset, cancellation and stop; and
3. the selected profile's oscillator, divider and board wait-state chain.

An unqualified path blocks physical pacing for that profile.  It remains an
explicit L2/L1/unsupported disposition; it is never estimated merely to make
Standard appear real-time.  Existing internally correct relative device timing
remains runnable while pacing is unavailable.

## Ownership And Data Flow

```text
CPU execution outcomes + qualified device deadlines
                    -> Core physical-time owner
                    -> copied Core observation + immutable profile rate
                    -> VM Standard ahead-only wait
```

No VM code may generate, compensate, batch-inject or skip guest ticks from
QPC, `Sleep`, or a host elapsed-time accumulator.  A Core API may accept an
opaque advancement ceiling only if Core itself selects every intermediate
instruction/device transition and it cannot expose controller state, timeline
internals or a second scheduler.

## T470 Subtask Plan

1. **S1 - physical-axis evidence and writer ledger.** Inventory every current
   writer and reader of elapsed/virtual guest time, every CPU result path,
   deadline owner and selected-profile clock assertion.  Reconcile each with
   Intel/IBM primary material already admitted by the source policy and the
   retained controller evidence.  Freeze the complete physical-eligibility
   ledger; no runtime behavior changes.
2. **S2 - one Core physical-axis contract.** Define one opaque, integer or
   rational Core-owned physical-time representation and one conversion/advance
   boundary.  Separate its meaning from scheduler-maintenance and instruction
   counting values without mirrored clocks or compatibility paths.  No profile
   becomes physically qualified in this S.
3. **S3 - CPU outcome settlement.** Route each admitted CPU execution outcome
   through the one physical-axis owner using source-backed form/context costs;
   retain a visible blocking disposition for every unallocated form, exception
   or bus condition.  CPU code must not become a device scheduler.
4. **S4 - controller and transaction settlement.** Connect only source-backed
   device deadlines, transaction waits and board inputs to the same axis.
   Existing T469 deadline composition is consumed rather than duplicated.
   Any L2 estimate, causal-only ordering or incomplete controller state blocks
   physical qualification instead of receiving a fabricated conversion.
5. **S5 - profile timebase proof.** Validate each selected profile's oscillator,
   divider and wait-state provenance against the full CPU/device ledger; expose
   `VERIFIED_PHYSICAL` only for a closed evidence chain and leave all others
   `UNAVAILABLE` with one named receiver.
6. **S6 - Standard pacing and closure.** Enable VM ahead-only pacing solely for
   a verified copied contract, retain the current L2 HLT load-backoff for every
   unavailable profile, and prove reset, cancellation, halt, input, debugger,
   Standard/Turbo and long-run pacing behavior.  Build the current stripped
   Release artifact and perform an independent ledger-to-code closure audit.

## Evidence And Completion Standard

Each physical-time ledger row records owner, lifecycle, source tier, unit,
conversion, selected profile applicability, regression and disposition.  A
profile is physically paced only when its CPU outcome, transaction, controller
deadline and oscillator chains are all complete.  Standard must never run a
qualified guest ahead of budget; host performance may leave it behind.  Turbo
must differ only by the absence of that host wait.

## Non-goals And Stop Conditions

Do not implement L4 electrical timing, waveform measurement, caches,
prefetch/pipeline speculation, new chipsets, a second Core scheduler, VM
profile-local timing algorithms, or host-driven guest-time injection.  Stop
and transfer a row when the primary source cannot establish its physical
relation or when it requires an unadmitted chipset/bus mechanism.
