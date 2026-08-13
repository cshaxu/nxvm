# M5 T346 S5: Input, Display, And Presentation Timing Boundary

## Guest-Time Owner

Before S5, `core_machine_advance_scheduler()` advanced KBC and VADP directly
from the post-retirement elapsed batch.  S5 removes both calls.  A third
reset-safe timeline callback now advances one due tick in the fixed order:

```text
KBC/AUX guest response and typematic state -> VADP guest raster state
```

It follows the S3 arbitration and S4 readiness callbacks.  The complete
same-tick machine order is now:

```text
DMA -> PIT -> PIC -> FDC media observation -> ATA refresh -> RTC -> KBC -> VADP
```

Each device retains its own configured clock-domain ratio.  Reset clears the
timeline and creates exactly one arbitration, one readiness, and one peripheral
callback for due tick one.  No batch scheduler still advances KBC or VADP.

## Proof And Regression Ownership

`core-machine-input-display-s5-smoke` proves CPU retirement followed by the
two peripheral records at due tick one, their sequence order, three pending
successors, and reset replacement.  Its marker is:

```text
M5:T346:S5:INPUT-DISPLAY-TIMELINE:OK
```

The retained KBC controller/AUX/guest-keyboard and VADP text/CGA/EGA/display
smokes remain behavior proof.  S2's foundation smoke now asserts only its
owned CPU-retirement and `DMA -> PIT -> PIC` prefix, not an accidental fixed
count of later subsystem trace records.  S3 filters its own arbitration chain;
S4 proves its readiness chain.  This prevents another device migration from
silently weakening or repeatedly rewriting an earlier mechanism's proof.

The CMake pure owner-test cohort rises from 121 to 122; the S5 smoke receives
the existing verified target-local strict GCC options.

## Host Boundary Sweep

`vm_session_capture_display_snapshot()` obtains a copied core display snapshot.
`vm_session_publish_display()` copies that snapshot into a platform
presentation mailbox.  Platform console/window implementations only capture
the mailbox; none calls a core clock, mutates VADP/KBC/guest memory, or supplies
host elapsed time.  Keyboard/mouse host policy similarly crosses through the
existing bounded request/input path before core KBC submission.  These are
deliberately host boundaries, not guest-time callbacks.

This S therefore makes no renderer cadence, browser/native event-loop, thread,
or host-polling fidelity claim.  Speaker/PPI, advanced AUX, additional video
modes, and host presentation policy retain their existing separate TODO/Queue
boundaries.

## Verification

Fresh configure, S5 focused proof, retained KBC/AUX/VADP/display and S2--S4
timing tests, exact registration, documentation governance, diff check, and
the full current gate are required.  No external source, firmware, media,
renderer, or host clock is introduced.

Promotion: retain through T346 S6, then merge the guest-time and copied-host
boundary conclusion into T346 history and the Windows readiness handoff.
