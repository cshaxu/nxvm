# M5 Core L3 Integration Closure And VM Contract Export

## Purpose

Integrate the reconciled timing plan, CPU programs, transaction lifecycle and
device-phase contracts into one Core L3 capability surface. Publish the stable,
validated contract catalog that a later VM profile resolver may select, without
implementing that resolver or changing a profile in this task.

This candidate is also the sole planned receiver for the host-paced guest-time
contract deferred by T459. It completes that contract only from the validated
controller deadline capabilities accepted by the preceding candidates; it does
not promote an L2 polling backoff into a clock model.

## Shared Admission Baseline

This final candidate verifies and exports the boundary defined by the
[Core specification-driven timing design](../etc/architecture/specification-driven-l3-timing.md)
and consumes the remaining integration-only receiver batch from the
[T433 S6 Core L3 admission ledger](../etc/evidence/t433-s6-core-l3-admission-feasibility-ledger.md):
`DISPLAY-PRESENT`, `INPUT-HOST`, `TRACE-DEBUG`, `PLATFORM-MAILBOX`,
`PLATFORM-RESOURCE`, `PLATFORM-WAIT`, `SESSION-COMMAND`, and `PRODUCT-DEBUG`.
These rows are copied-observation, host-adapter or product boundaries, not
machine-chip timing rules; this candidate must preserve that classification
while exporting only neutral validated contract metadata.

The [T433 S7 source-sufficiency ledger](../etc/evidence/t433-s7-core-source-sufficiency-ledger.md)
is mandatory: it confirms that these boundary rows require isolation proof,
not fabricated guest-time manual rules.

## Required Scope

Consume all preceding Core candidates and their closure ledgers. Reconcile the
master ledger against the actual Core API, default plan and regressions; prove
that every public capability has one L3 contract, explicit L2 fallback, not
applicable or unsupported disposition. Export only neutral contract IDs,
versions, validation requirements and observability declarations. VM machine
identity, inheritance, provenance and YAML policy remain outside Core.

For each selected profile with a verified physical guest-timebase, compose the
earliest validated guest-observable deadline of every timed controller that can
wake or visibly change the guest. Core remains the sole owner of guest time:
it selects and advances the ordered guest timeline, including reset and
cancellation effects, before publishing a bounded copied observation. VM may
use only that already-advanced observation and the selected profile timebase to
pace against its monotonic wall-clock budget: Standard waits only when guest
progress is ahead. Turbo uses the same Core-owned progression without that
host pacing wait. Neither mode may manufacture, compensate, skip or infer
guest ticks from QPC, `Sleep`, or recurring maintenance callbacks.

The exported Core/VM boundary must be opaque and value-based. It exposes no
controller pointer, timeline internals or second scheduler. A controller or
profile without the required validated deadline/timebase keeps its explicit L2
or unsupported disposition; this candidate must not claim L3 synchronization
for that configuration.

## Dependencies

Last in the ordered Core L3 sequence. A later VM/profile program may consume
its exported catalog only after this candidate accepts it.

## T469 Subtask Plan

The task is admitted as T469.  Each S consumes one complete batch and must not
create a second scheduler, device clock or VM-to-Core mutation path.

1. **S1 - deadline eligibility ledger and session cleanup.** Freeze every
   currently timed Core owner and its wake/visible-event condition; classify
   it as a source/model/Input-L3 deadline, L2 estimate, L1 order-only, L0
   missing logic, L4 physical exclusion or unsupported.  Correct Td S148's
   accidental L4-to-L0 wording before consuming it through a complete
   corrective re-audit of all 145 PIC, DMA, PIT, RTC, KBC, 8272A FDC,
   VADP-CGA, VADP-EGA and ATA/HDC rows.  Audit the actual Core/VM call path
   and repair the duplicated RTC timing conditional in `session.c` through
   the one session construction path.  This S creates no deadline API or
   synthetic timing value.  A manual numeric value or formula remains Manual
   L3; only an unsourced conversion of it into Core ticks can be L2.
2. **S2 - composed Core deadline observation.** Add the smallest opaque,
   copied Core observation that reports the earliest eligible guest-observable
   deadline and its reset/cancellation validity.  Core, not VM, selects order
   and advances all intervening controller state.  An ineligible controller or
   profile remains explicitly L2/L1/unsupported.
3. **S3 - source-qualified Core deadline realization.** Consume the S1
   deadline ledger's first finite source batch: PIT output and L3-source RTC
   periodic/update IRQ transitions.  Core computes their earliest next state
   change from its existing owner state and clock ratio, advances only through
   one Core-owned bounded operation, and publishes the copied observation.
   An active KBC, FDC, HDC, DMA or any L2/L1 contributor makes the observation
   unavailable; it is never estimated or skipped.  This replaces neither a
   controller state machine nor the one timeline.
4. **S4 - immutable plan qualification and contract export.** Validate that a
   selected profile has every required deadline contributor and physical
   guest-timebase before exposing the bounded contract metadata. Reject an
   incomplete selection; do not expose controller pointers, timeline internals
   or profile names.
5. **S5 - two-mode VM pacing.** Retain the existing `SPEED STANDARD|TURBO`
   command and sole session speed state.  Standard waits only when an eligible
   already-advanced guest observation is ahead of its wall-clock budget;
   Turbo omits that wait while using the same Core-owned progression.  Remove
   the fixed `Sleep(1)` fallback only where the composed contract makes that
   safe; do not generate, compensate or batch guest ticks in VM.
6. **S6 - integration closure.** Reconcile the frozen capability ledger with
   code and regressions; prove reset/cancellation, input, debugger, selected
   profile isolation, Standard/Turbo behavior and ineligible fallback.  Build
   the stripped Release `nxvm_0_5_0469.exe`, run all applicable gates and
   perform the independent closure audit.

The RTC cleanup is deliberately limited to the duplicate conditional found by
Td S148.  It is in S1 because it is the same session-construction mechanism;
it must not grow into an unrelated RTC feature or timing repair.

## Evidence And Completion Standard

Require end-to-end Core composition tests across the supported CPU and shared
controller configurations, immutable-plan and rejection tests, trace checks,
all current gates, and an independent ledger-to-code closure audit. The audit
must prove no profile name or profile-local timing algorithm entered Core and
no public capability lacks a declared disposition. Where the host-paced
contract is eligible, proof must cover deadline composition, ordered Core
advance, reset/cancellation, bounded export, Standard's ahead-only pacing and
Turbo's no-wait use of the same progression. Where it is not eligible, proof
must retain the declared fallback without fabricated time.

## Non-goals And Stop Conditions

Do not claim every historical machine is L3, implement profile inheritance or
user YAML, admit new CPUs/devices/chipsets, import firmware/media, or perform
L4 work. Stop if a remaining gap belongs to VM/profile ownership; transfer it
explicitly to that later program. Do not substitute a fixed host sleep, host
clock tick injection, or a controller-private deadline query for this composed
contract.
