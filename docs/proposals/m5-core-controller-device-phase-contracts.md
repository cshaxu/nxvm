# M5 Core Controller And Device Phase Contracts

## Purpose

Move every currently public shared controller, device and integration phase
onto the Core timing-plan and transaction lifecycle: command acceptance,
service readiness, DRQ/IRQ assertion and withdrawal, acknowledgement, reset,
cancellation, error/result completion and presentation boundaries.

## Shared Admission Baseline

This candidate applies the
[Core specification-driven timing design](../etc/architecture/specification-driven-l3-timing.md)
to the earliest controller/device receiver batch in the
[T433 S6 Core L3 admission ledger](../etc/evidence/t433-s6-core-l3-admission-feasibility-ledger.md):
`CTRL-PIC`, `CTRL-DMA`, `CTRL-PIT`, `CTRL-RTC-CMOS`, `CTRL-KBC-NMI`,
`CTRL-FDC`, `CTRL-HDC`, `MEDIA-BACKING`, and `DISPLAY-VADP`. It admits only
source-backed or labelled reference contracts through the plan/transaction
owners; electrical waveforms and unadmitted machine facts remain visible L2
exceptions rather than generic Core behavior.

## Required Scope

Consume the reconciled device ledger and retained PIC/DMA/PIT/RTC/KBC/FDC/HDC/
display/device-service evidence. Work by frozen controller capability batches,
not by guest symptom or machine name. Each batch must state its producer,
consumer, phase rule, clock/transaction relationship, reset/cancel behavior,
source tier and regression owner.

Use an L3 rule where the retained contract supports it; otherwise preserve the
current deterministic L2 behavior as an explicit exception. Special machine
personality selection remains data supplied through the timing plan, never a
Core machine-name branch.

## Dependencies

Consumes the plan, CPU timing and transaction/arbitration contracts. It
precedes the integrated Core closure and VM contract export.

## Evidence And Completion Standard

Require complete finite capability batches for all currently public shared
controllers/devices/integrations, command-to-event traces, reset/cancellation
proof, focused regressions and current gates. Every batch row must close as
L3, accepted L2 fallback, not applicable or unsupported, with no silent
compatibility behavior.

## Non-goals And Stop Conditions

Do not expand the supported device surface, implement VM profiles, claim
monitor/electrical behavior without a contract, or undertake L4 work. A
profile-local or unknown hardware fact is transferred to an explicit receiver,
not made a generic Core rule.
