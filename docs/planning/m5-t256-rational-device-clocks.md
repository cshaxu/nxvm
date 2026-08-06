# M5 T256: Level 1 Rational Device Clocks

**Status:** S4 active.

## Original Request

Implement the Level 1 timing foundation required by future DeskPro and PC110
profiles: one core-owned, deterministic scheduler with integer rational device
clocks and per-machine phase state. Profiles provide frozen ratios and reset
phases; composition may pace host execution but never advances guest time.
This is not instruction-cycle, bus, or cycle-exact emulation.

## Requirements And Non-Goals

| Requirement | Owner and evidence | Non-goal |
| --- | --- | --- |
| One guest reference time | `core_machine.elapsed_ticks`, advanced after completed coarse instruction attribution only | Host wall clock, float/double, process-global or TLS clock |
| Rational device clocks | `core/machine` stores one integer numerator/denominator/phase state per core device and frozen provider domain | Real x86 instruction-cycle attribution |
| Fixed delivery order | Core dispatches DMA, PIT, VADP, KBC, frozen execution provider, then PIC visibility | VM-side device loop or a profile-selected scheduler order |
| VM-only time | The frozen execution provider is the sole delivery boundary for RTC and future timed VM devices | Composition directly advancing RTC/FDC/HDC or a new host timer |
| Profile configuration | `vm/profile` supplies frozen core-domain and VM-provider ratios/reset phases | Runtime profile/ROM/topology/clock switching |
| Deterministic corpus | Same guest instruction stream and external inputs at identical execution boundaries have equal checkpoints under different run quanta, pause/resume, and reset | Cycle accuracy, DMA arbitration/wait states, prefetch, or full bus microstate |

## S1 Current-State Audit

| Current path | Evidence | T256 disposition |
| --- | --- | --- |
| Reference ticks | `core_machine_run()` increments `elapsed_ticks` by `ticks_per_instruction` after each completed instruction. | Retain as Level 0 coarse attribution; use it as the sole Level 1 input unit. |
| Core scheduling | `core_machine_advance_scheduler()` calls DMA, PIT, VADP, KBC, execution provider, then PIC refresh. | Retain this order and make every delta rational-domain output. |
| PIT conversion | `pit_elapsed_tick_remainder` divides elapsed ticks by a PIT-only integer. | Replace with the same generic rational phase accumulator used by every domain. |
| DMA/VADP/KBC | Each currently receives raw elapsed ticks. | Give each a default identity ratio while allowing a frozen profile override. |
| VM provider | The frozen execution provider calls VM lifecycle advance; it currently forwards raw elapsed ticks to CMOS. FDC/HDC have no timed behavior yet. | Deliver only converted provider ticks. RTC consumes them; FDC/HDC remain untimed until a corpus admits a timed operation. |
| Profile | Default PC/AT declares coarse CPU ticks, PIT divider, RTC frequency, and device topology. | Replace the PIT-only field with one immutable generic core clock plan, including provider ratio/reset phase. |

### Clock Contract

For a domain ratio `numerator / denominator`, core delivers
`floor((phase + elapsed * numerator) / denominator)` device ticks and stores
the remainder as that domain's new phase. All fields are unsigned integers;
`denominator` and `numerator` must be nonzero, `reset_phase < denominator`, and
a zero-initialized ratio resolves to identity `1/1` with phase zero for retained
callers. A cold reset restores every phase to its frozen reset phase.

The exact delivery sequence per completed guest instruction is: update reference
elapsed ticks, advance DMA, PIT, VADP, KBC, and the frozen provider by their
converted deltas, then refresh PIC visibility. Prefixes do not receive a
separate clock event. A paused/stopped machine and host pacing do not advance a
domain. A run budget can only stop *before* the next instruction when its coarse
tick allowance is insufficient, so instruction-boundary splitting cannot change
the accumulated device phase or event order.

### Corpus And Future Profile Capability

Existing `core-machine-time-smoke`, `core-machine-scheduler-smoke`,
`core-machine-pit-divider-smoke`, `core-machine-timing-checkpoint-smoke`, and
VM RTC/BIOS timing probes establish the retained baseline. S2 adds a focused
rational-domain probe; S3 adds quantum, pause/resume, and reset checkpoint
comparison. Future DeskPro/PC110 profiles must declare CPU reference attribution
and the DMA/PIT/VADP/KBC/provider ratios/reset phases they require, then prove
their own device-visible checkpoints. They cannot create a second scheduler.

## Similar-Issue Sweep

Defect class: raw elapsed-tick delivery or device-specific phase conversion
outside the sole core scheduler. S1 query:
`rg -n "elapsed_ticks|pit_elapsed_tick_remainder|_advance\(" src tests`.
Production dispositions are the scheduler's DMA/PIT/VADP/KBC/provider calls;
VM CMOS is the sole current time consumer behind the provider callback; FDC/HDC
have no time advance API. S2 must re-run this query and reject new direct
composition-to-device time calls.

## Rules, Matrix, And Stop Conditions

Applicable rules: one machine/session/runtime path; core has no VM dependency;
profile is frozen declaration; composition is the only binding owner; platform
does not mutate guest state; no host-clock shortcut or global/TLS state. Run
focused timing probes, current static ownership gates, FDD/HDD boot, DOS prompt,
Console/debugger, CGA/EGA, ATA, and RTC coverage before closure. An optional
PCjs/Bochs status-sequence comparison must use project-owned checkpoints and
the hardware-template instruction/time/no-progress/trace budgets; it is not
required for acceptance and is never a runtime dependency.

Stop and split on any need for floating point, a host-time guest clock,
composition-side device advance, profile-selected dispatch order, a second
machine/executor, or a claim of instruction/bus/cycle accuracy.

## S1 Exit

S1 completes with this contract, current-path inventory, and approved S2 scope.
It changes no guest behavior and creates no artifact.

**S1 result:** the retained `core-machine-time`, `core-machine-scheduler`,
`core-machine-pit-divider`, `core-machine-timing-checkpoint`, and
`vm-cmos-rtc-port` probes pass with their recorded markers. Their current
outputs establish the pre-conversion behavior that S2 must preserve at identity
ratios and at the retained default PC/AT PIT ratio.

## S2 Result

`core_machine` now owns five per-machine rational domains: DMA, PIT, VADP, KBC,
and the frozen execution provider. The default PC/AT profile declares identity
ratios except for its retained PIT `1/4` input ratio. Core resets every domain
to its frozen phase, advances all domains with integer quotient/remainder
arithmetic after a completed instruction, and dispatches them in the S1 order.
VM RTC delivery remains behind the registered execution-provider callback;
composition no longer advances VM-owned time consumers. The focused profile,
rational-clock, retained PIT divider, core time, scheduler, timing checkpoint,
and CMOS/RTC probes pass.

## S3 Scope

S3 records identical frozen-provider event deltas for the same four completed
instructions when executed in one quantum, two budget quanta, or four separate
quanta, then repeats the two-quantum case after cold reset. Retained PIT and
CGA timing probes cover the adjacent core-device checkpoints. The full current
matrix must additionally prove that VM RTC delivery remains behind the frozen
provider boundary and that reset, Console, debugger, FDD/HDD, CGA/EGA, and ATA
behavior remain unchanged.

**S3 result:** the rational-clock probe records the same `2, 1, 2, 1`
provider-tick sequence for one four-instruction quantum, two two-instruction
quanta, four one-instruction quanta, and a cold-reset replay. The full
`current-gates-gcc` matrix passes **90/90**.
