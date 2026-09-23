# T469 S4: Pacing Timebase Contract

`M5:T469:S4:PACING-TIMEBASE-CONTRACT:OK`

## One Qualified Value Boundary

`core_machine_guest_timebase` is copied from `core_machine_config` when Core
is created. Its only positive form is `VERIFIED_PHYSICAL` with a nonzero
source-tick rate; unavailable carries no rate. Core exposes that result through
the copied `core_machine_pacing_contract`, never a profile name, controller,
timeline, or host-clock callback.

The timebase value is deliberately separate from S3's dynamic deadline
observation. A physical source does not say that an IRQ deadline is currently
safe, and an available PIT/RTC deadline does not make mixed retirement ticks a
physical source. S5 must require both values: the static pacing contract and a
current Core-selected observation, and must still ask Core to advance.

## Current Qualification

T388 proves every current CPU retirement axis has successful nonphysical
routes. Therefore default PC/AT, IBM 5170 Model-339 and DeskPro Model-40 all
retain `UNAVAILABLE`, even though Model-339 has sourced PIT/RTC ratios and its
descriptor names a nominal 8 MHz device source. That rate is not copied into
the pacing contract. The focused positive test supplies a synthetic verified
Core input only to prove validation/copy semantics; it asserts no real profile
qualification.

## Verification And Simplicity

`core-machine-time-smoke` rejects a zero-rate verified input and proves one
valid copied 8 MHz contract. Model-339/default and Model-40 BYOB composition
smokes prove the three current profiles export no pacing rate. Core plan and
documentation gates pass. The changed mechanism has one config-to-Core copy
and one getter; the existing S3 observation remains the only deadline path.
`git diff --numstat` for tracked source and tests records 73 added and 0
removed lines; the positive increase is the typed value, validation, copy,
getter and three-profile proof. No pre-existing production path was duplicated
or retained as a compatibility route.

## S5 Transfer

S5 must delete the Model-339-specific platform virtual-time source and its
host-to-`core_machine_advance_time()` route. It may pace only after Core has
advanced a source-qualified deadline and only if this contract is available;
under the current evidence that means the L2 HLT backoff remains until a later
physical-timebase admission qualifies a real profile.
