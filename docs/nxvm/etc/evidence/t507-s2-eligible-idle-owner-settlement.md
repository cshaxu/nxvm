# T507 S2: Eligible Idle-Owner Settlement

M5:T507:S2:CORE-EARLIEST-SETTLEMENT:OK

## Implemented batch

The scheduler still advances the single Core axis and retains its fixed
same-tick order. The focused source audit found the pre-T507 scheduler already
uses earliest known deadlines; it does not retain T499's former recurring
timeline callbacks. The only unconditional readiness work with a provably
empty owner was FDC/HDC maintenance.

Readiness now advances and traces FDC only when the Core plan configured FDC.
It advances and traces HDC only while the configured owner's phase is active.
RTC remains in the same readiness position. No existing FDC/HDC state is
moved, mirrored or made visible to VM; no deadline, duration or new API was
introduced.

## Matrix reconciliation

| Rows | Result |
| --- | --- |
| C3--C8, C10--C13 | Existing timeline, clocks, due queries and same-tick sequence were verified as the one current Core path. Their deltas are still required to preserve state between CPU boundaries; no duplicate fast path is admissible. |
| C9 | Idle HDC no longer receives a scheduler call or development trace. A live phase retains its same Core-local immediate/L1 path. |
| Unconfigured FDC | No FDC call or trace is emitted. A configured FDC retains its owner-local absolute due and elapsed-state progression. |
| C2/C12 | Unchanged explicit lower-tier receivers. |
| V1--V4 | Unchanged; this batch deliberately does not alter VM pacing. |

The updated trace regressions assert actual owner work rather than the retired
polling order. DMA/PIT/PIC ordering and all actual FDC/HDC routes remain
covered by the competition and controller tests.

## Verification and simplicity

Focused scheduler, competition, RTC and input/display tests pass. Complete
repository unit passes 312/312. The DOS prompt integration baseline remains
successful in 2.69 seconds before this code batch.

Tracked code/test delta excluding documentation: +31/-43 lines across one
scheduler source and four focused tests, net -9. The retained production path
is one Core scheduler -> configured/active owner progression. No new wrapper,
state, timer, profile path or public boundary exists.
