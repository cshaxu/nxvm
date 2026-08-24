# T456 S3: PIC Programmed Cascade Contract

## Scope and authority

This evidence consumes the accepted ICW3/SNGL receiver from T456 S1. Intel
8259A `231468-003` defines ICW3 on a master as the bit map of inputs carrying
slaves and ICW3 on a slave as that slave's master input identity. ICW1.SNGL
selects a single controller, so no ICW3 cascade relation exists. The admitted
IBM AT initialization remains master `04h`, slave `02h`.

This S implements only the one existing paired Core master/slave controller.
It does not claim arbitrary multi-slave wiring, introduce a VM/profile
topology, or infer electrical timing from the command words.

## One-owner implementation

Before S3, selection, SFNM selection and refresh treated master IR2 as the
paired slave unconditionally. The command parser stored ICW1 and ICW3 but the
sole runtime owner did not consume them.

`pic.c` now has one PIC-local relation predicate: both controllers must be
cascaded, the slave's three-bit identity must name a master input, and the
master ICW3 must declare that input. Selection, SFNM and refresh all consume
that predicate. A declared-but-unpaired master slave bit is not reclassified
as a device IRQ.

`cascade_irr` is a PIC-owned synthesized request bitset, separate from device
IRR. Refresh derives it once from the selected paired slave request; master
acknowledgement clears that synthesized bit while slave acknowledgement clears
its device request. Thus the selected slave still provides the vector without
duplicating PIC state in CPU, VM or profile code. ICW1 initialization clears
it with the other controller request state, and reset clears the whole
PIC-owned data object.

## Similar-issue sweep and retained boundary

The S3 marker sweep covered all `ICW1_SNGL`, `icw3`, `cascade_irr` and
hard-coded master-line-2 occurrences in `src/core/machine` and
`tests/machine`. No hard-coded cascade decision remains in selection, refresh,
acknowledgement or SFNM. The remaining line-2 literals are the AT default test
observation and the current board-global IRQ-source binding, which reserves
global IRQ2 for the selected AT cascade; neither is a selection/refresh
topology decision.

The existing per-controller OCW3 poll callback can acknowledge its selected
master request but has no paired-controller argument with which to perform the
second slave acknowledgement. S3 intentionally does not create a second
cross-controller command path; the complete retained-command/poll proof is
the bounded S4 receiver.

## Verification

The existing command-priority smoke programs ICWs through real PIC ports and
proves, on the sole selection/refresh/peek/get route:

- AT `04h/02h` maps IRQ14 through master IR2 and returns slave vector `76h`.
- Programmed master/slave line 5 maps IRQ14 through IR5 and still returns
  slave vector `76h`.
- A mismatched master/slave ICW3 pair and master SNGL suppress the cascade.
- A new master ICW1 clears an already synthesized AT cascade request and
  enters ICW2; reset remains covered by the controller lifecycle tests.

The focused command-priority build/current CTest replay passes, as do the
affected 11 PIC/IRQ smokes, the complete configured 547-step build and the
documentation-governance gate. The first isolated 293-test current-gate replay
had one unrelated `vm-model40-hdc-s26-smoke` failure; its immediate isolated
replay passed. The complete independent replay then passed 293/293 in 95.51
seconds. It therefore does not evidence a stable PIC regression.

The implementation adds one controller-local byte and two compact relation
helpers, replacing every hard-coded cascade decision in the affected paths.
It adds no public ABI, wrapper, compatibility path or second selector.
