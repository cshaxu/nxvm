# T508 S3 Unqualified-Axis Disposition

## ATA Service Research

ATA-3 X3T13/2008D specifies interface ordering and selected interface bounds,
including PIO completion state within 400 ns after the final byte.  It does not
specify one command-to-DRQ or media-service duration for every ATA device.
That value depends on the attached drive and command.

The local external implementations confirm that distinction rather than supply
a portable constant: 86Box combines IDE transfer time with per-drive seek and
speed presets; Bochs owns a separate seek timer; PCjs records an arbitrary
large IRQ-delay experiment.  These are useful L2 design corroboration, but
they are mutually non-equivalent device models, not a generic PC/AT timing
contract.

The retained default-PC/AT profile deliberately has no named board, clock
source, drive model, or complete CPU-cycle axis.  Assigning a service number
or a nominal MHz rate would therefore be an unqualified product fiction.  Its
`service_ticks == 0` remains the explicit immediate HDC owner boundary, not
an L1 compatibility wait and not an L2 mechanical claim.

## Model 40 Axis Research

The selected Model 40 material supports individual board relations already
present in the profile, including the documented processor/memory interface
and the auxiliary-PIT DCLK relation.  Core's current elapsed axis, however,
advances through the qualified retirement/transaction contract and is not
proven to equal one Model-40 processor cycle.  Marking that axis as a 16 MHz
macro rate would falsely convert one current elapsed tick into a CPU cycle.

Model 40 consequently retains its unqualified host-pacing axis.  Its FDC
receives its explicit 8 MHz formula input and its Compaq/WD owner receives the
separate Other-L2 service quantum; neither route enters L1 compatibility.

## S3 Result

The two rows are one complete unqualified-axis class.  Neither can be repaired
with a controller-local constant.  The explicit boundary is the correct
disposition until an earlier CPU-retirement/board-axis task establishes a
complete conversion.  S4 tests that all existing built-in profiles remain
outside the bounded L1 compatibility disposition without promoting either
axis.
