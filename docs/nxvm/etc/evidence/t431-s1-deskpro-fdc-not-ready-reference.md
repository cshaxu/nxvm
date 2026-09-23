# M5 T431 S1 DeskPro FDC Not-Ready Reference Completion

## Evidence Tier And Boundary

`reference-derived`: the local read-only PCjs DeskPro FDC implementation was
consulted transiently. It describes an empty-diskette DeskPro BIOS path that
expects an IRQ and a Not Ready `ST0` result. Its author explicitly records no
real DeskPro hardware verification; it is therefore not original-machine
measurement.

`original`: NEC uPD765 material was consulted only for the general Ready,
IRQ and ST0 controller semantics. It does not establish the selected DeskPro
READ result.

No source text, ROM, media, PDF, hash, path or third-party code is retained.

## Implemented Contract

Core owns one copied immutable FDC configuration policy. The generic policy
retains the established `ST1=No Data` completion. Model-40 composition alone
selects the reference-derived policy: an execution-phase READ of an unready
selected drive completes through the existing pending/result/IRQ path with
`ST0=48h`, `ST1=00h`, `ST2=00h`.

The completion uses the existing FDC result, IRQ, DOR-reset cancellation and
Sense Interrupt Status owners; it creates no VM command path or scheduler.
The terminal-observation success predicate now also requires normal `ST0`, so
this Not Ready completion cannot be recorded as successful. The completion
helper preserves accumulated scan `ST2`, repairing the adjacent discovered
regression.

## Verification

- `core-machine-fdc-smoke` passes, including retained deleted-data and scan
  markers.
- `vm-model40-fdc-s24-smoke` proves Model-40 policy selection, `48h/00h/00h`,
  result phase and IRQ6, DOR reset cancellation, post-reset Sense acknowledgements,
  and final IRQ clearing; it prints `M5:T431:S1:MODEL40-FDC-NOT-READY:OK`.
- `vm-default-pc-at-apply-smoke` proves generic PC/AT selects the generic
  policy and prints its retained marker.
- Serial `current-gate`, documentation governance, and `git diff --check`
  pass at closure.

## Similar-Issue Sweep And Transfer

All FDC topology validation, configuration initializers, completion sites,
Model-40 binding and generic-PC/AT selection were inspected. The policy is
validated once at Core topology application, defaults safely to generic, and
has one explicit Model-40 selection; no duplicate VM or command path remains.

This closes only a reference-derived logical empty-drive READ completion.
Physical FDC rates, motor/head waveform, board propagation/arbitration and
DeskPro physical/L3 acceptance remain receiver-2 work in the M5 proposal.