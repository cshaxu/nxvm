# T470 S5: Profile Timebase Proof

`M5:T470:S5:PROFILE-TIMEBASE-PROOF:OK`

## Selected Profile Universe

| Profile | CPU retirement | Transaction and board | Controller deadline | Oscillator/divider | Result |
| --- | --- | --- | --- | --- | --- |
| default PC/AT | Deterministic 80386 execution; T388 residual-form prohibition remains. | No physical external-wait or board relation. | Default controller rules are L2, so no composed deadline. | No complete axis rate. | `UNQUALIFIED`, zero rate. |
| IBM 5170 Model-339 | 80286 source rows exist but T388 leaves complete successful retirement ineligible. | No complete physical bus/wait-state chain. | PIT/RTC rational deadline is source-qualified when unblocked. | PIT/RTC ratios are not a CPU-axis seconds proof. | `UNQUALIFIED`, zero rate. |
| Model-40 | Deterministic 80386 plus retained T388 residual boundary. | External-cycle waits and D4 slowdown are explicit S4 blockers. | Current controller inputs do not close every active state. | No complete CPU/board oscillator chain. | `UNQUALIFIED`, zero rate. |

`CORE_MACHINE_TIME_AXIS_VERIFIED_PHYSICAL` is therefore selected by no product
construction path. The only verified-axis assignments are focused synthetic
tests; they prove Core validation/owner behavior, not a profile claim. Session
waiting reads a copied observation and returns without advancing when that
axis is unavailable, so neither QPC nor sleep generates guest progress.

## Crosswalk And Result

T388 S9 remains decisive for each CPU profile: a complete successful-retirement
axis cannot be physical while any residual route is unallocated or explicitly
nonphysical. T470 S3 blocks such retirement before elapsed publication; T470
S4 separately blocks external wait, BUSRDY, D4 and deterministic advance. T469
permits only the already source-qualified PIT/RTC deadline composition, and
only when no active controller blocker exists. None of the three profiles
closes all those chains or contributes a full seconds-per-axis-unit proof.

No code changes are required: every current construction already leaves the
time axis zero/unqualified and selects deterministic retirement. Changing a
profile would be a false claim, not a repair.

## Verification

- Static sweep of profile descriptors, session composition, Core creation,
  time observation and VM waiting finds no product verified-axis assignment,
  physical-retirement selection or host-to-Core time conversion.
- Existing Model-339 and Model-40 contract smokes assert unavailable/zero
  observations and deterministic retirement; Core time smoke confines its
  positive axis to synthetic validation.
- The retained T388 S1/S3/S9 and T470 S3/S4 ledgers supply the complete CPU
  and non-retirement blocker crosswalk.

## S6 Transfer

S6 may only preserve the unavailable-profile L2 HLT backoff and verify the
Standard/Turbo product policy. It must not enable host pacing for a current
profile. The future verified-axis path is already present in the copied Core
observation and remains dormant until a separately closed profile chain exists.
