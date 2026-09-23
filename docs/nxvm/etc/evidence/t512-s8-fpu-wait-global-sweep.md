# T512 S8: FPU/WAIT global same-class sweep

## Trigger and scope

The 80386 `WAIT` reclassification exposed a semantic defect, not merely a
timing-label defect: `core_machine_fpu_complete_wait()` formerly cleared
`busy` whenever the CPU retired `WAIT`, although no FPU completion had
occurred.  This sweep therefore covers every supported CPU/FPU pairing and the
shared Core FPU owner; it does not create a VM/profile-side CPU path.

## Source and reference findings

The Intel 80286/80287 Programmer's Reference Manual, section 3.12.1 and
Appendix-B `WAIT` row (printed pp. 3-30 and B-108), states that `WAIT` waits
for BUSY inactive; its three CPU clocks do not make BUSY complete.  Chapter 2
of the same manual describes concurrent 80286/80287 execution and gives
80287 typical/range clocks, explicitly noting the CPU and NPX clock frequencies
and bus transfers when calculating elapsed operation time.  Thus it supplies
the interface rule but not one universal Core completion scalar.

86Box independently uses a single FPU-cycle countdown that is decremented by
CPU cycle consumption (`src/cpu/808x.c` and `src/cpu/386.c`).  This is an
admissible External-L2 concurrency model, not an imported implementation.
PCjs deliberately executes FPU operations synchronously and leaves `opWAIT()`
to the CPU default; it is not an asynchronous timing model.  Bochs likewise
models `FWAIT` as TS/pending-exception handling when its FPU executes
synchronously, so it does not supply an elapsed-BUSY model for NXVM.

## Pairing inventory

| CPU profile | allowed FPU | current shared outcome | status |
| --- | --- | --- | --- |
| 8086/8088, 80186 | 8087 | ESC starts a Core-owned 8087 External-L2 completion interval; `WAIT` contributes only its remaining Core ticks | repaired to External-L2 |
| 80286 | 80287 | exact CPU-side `WAIT` cost is retained; an 80287 typical/range External-L2 interval supplies its completion | repaired to External-L2 |
| 80386 | 80287/80387 | CPU-side `WAIT` lower-bound and ESC delegation are L2; selected 80387 metadata now schedules an External-L2 Core deadline | repaired to External-L2 |
| any supported CPU | none | no outstanding FPU BUSY state exists | not affected |

## One-owner repair

The repair replaces the artificial `wait_iterations` clear with Core FPU
completion state and a scheduler deadline. ESC records the interval in that
owner; ordinary Core time advancement retires it; `WAIT` only consumes the
remaining ticks and pending ERROR remains separate. Manual exact values and
ranges are retained as source facts, but their projection onto the unqualified
Core axis is explicitly External-L2; unclassified valid commands use the
named 86Box-style proportional fallback (85 pre-387, 28 for 80387). The
focused pairing suite proves all five valid pairings and their no-FPU controls.
