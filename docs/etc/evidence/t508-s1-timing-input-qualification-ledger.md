# T508 S1 Timing-Input Qualification Ledger

## Scope And Evidence Method

This is the finite T508 construction-time input universe, not a second timing
authority.  It maps current values to their sole Core owner and records source
qualification.  Core receives copied values only; Manual-L3 and Other-L2 are
evidence dispositions, not runtime branches.

The manual corpus was visually checked in its owner-managed archive: IBM 5160
Technical Reference (6280089), IBM 5170 Technical Reference (1502243), Intel
8237A (231466-005), Intel 8253 (231306-001), Intel 8254 (231164-005), Intel
8272A (210608-001), Intel 8259A (231468-003), MC146818A, and ATA-3
X3T13/2008D.  The retained 86Box, Bochs and PCjs source checkouts corroborate
deadline ownership and identify ATA delay models as implementation choices; no
external source is imported.  Prior rendered T487--T494 and T507 evidence
provides the corresponding complete chip List 1/List 2 details.

`CORE_MACHINE_TIME_AXIS_MACRO_PROPORTIONAL` is intentionally not an L-level:
it controls host-pacing eligibility.  A manual-derived device ratio can be L3
on that axis without asserting that the complete machine has physical wall-clock
qualification.

## Core Input Seam

| Input family | Copied Core input and sole consumer | Qualification result |
| --- | --- | --- |
| CPU, transaction, time axis | `core_machine_config` copied by the Core plan; CPU retirement and scheduler own progression. | The seam accepts exact integers/rational values; it does not need source-provenance behavior. |
| Device clocks | `core_machine_clock_plan` ratios for DMA, PIT, auxiliary PIT, RTC, VADP, KBC and provider; scheduler converts each owner-local due value. | L3-capable rational input; all-zero is a legacy identity shorthand, not a second clock. |
| Controller relation claims | `core_machine_controller_timing_rules` validates sourced rational/phase relations where represented. | Provenance is currently partial and applies only to selected relations; ledger/evidence remains the authority for L3 versus L2. |
| FDC byte service | `core_machine_fdc_config.clock_ticks_per_second`; FDC converts NEC/Intel microsecond bounds through its sole deadline. | L3-capable formula input. |
| HDC service | `core_machine_hdc_config.l2_service_ticks`, copied by the HDC owner. | Semantic defect: the name incorrectly excludes a possible manual L3 value. S2 must rename it without adding a new route. |
| PIC/D4 | PIC publishes immediate state; D4 publishes one ordered next-Core-step transaction. | Neither has a missing duration to fill; both are non-duration owner relations. |

## Product Profile Disposition

| Profile | Qualified supplied inputs | Below-L2 or L1 finding | Owner route and S disposition |
| --- | --- | --- | --- |
| IBM 5160 XT | 4.772727 MHz board axis and 8253 quarter-rate are manual L3 values; DMA ratio is explicit; 8272A byte formula is supplied from the same board rate. | No active L1 compatibility route.  Full physical wall-clock qualification is outside the macro-axis claim. | Core plan -> DMA/PIT/FDC owners -> scheduler. Retain values; S4 tests the active wait matrix. |
| IBM 5170 Model 339 | 8 MHz macro axis, 8254 1.193182 MHz and RTC 32.768 kHz are sourced L3 relations; DMA phase is sourced; VADP rate and HDC 2 ms service quantum are Other-L2. | No active L1 compatibility route. | Default-PC/AT resolver -> copied plan -> Core owners. Retain values; S2 validates source-neutral HDC input. |
| DeskPro Model 40 | Board-local clock ratios and 8 MHz FDC formula input are frozen; Compaq/WD 2 ms is Other-L2. | Its parent/child Core configuration leaves `time_axis` unqualified.  This blocks host pacing but does not trigger an L1 controller wait. | Model-40 resolver/composition -> copied plan -> Core owners. S3 decides whether a non-speculative macro axis exists; no guessed rate. |
| default-PC/AT | Explicit topology and logical ratios permit Core deadline ordering; ATA's immediate owner boundary preserves its command state semantics. | The profile is intentionally generic, with an unqualified axis, no FDC microsecond conversion rate, and no ATA service duration. These are not active L1 compatibility states, but they are below the requested L2 target. | Default-PC/AT resolver -> copied plan -> Core owners. S3 must either establish one evidence-backed generic macro contract or retain the whole generic profile as explicitly unqualified; it may not pick an arbitrary MHz value. |

## S1 Decision

There is exactly one active L1 compatibility predicate: an externally
constructed DMA configuration with a pending request and no explicit DMA
ratio.  No built-in product profile reaches it.  The only implementation batch
proved by this ledger is source-neutral naming of the copied HDC service input.
The two unqualified generic/profile time axes form one S3 decision class; they
cannot be repaired independently with a fabricated ATA delay.
