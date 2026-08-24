# M5 T456: Core PIC 8259A Phase Contract

T456 implements the finite Core 8259A command, priority, cascade,
acknowledgement, EOI, mask and reset contract after a manual-first inventory.

## Active Subtask

- S1 reconciles T450's original-source and current-code ledger with rendered
  manual pages and independently versioned emulator observations before
  implementation. It defines the bounded S2--S4 plan only after every retained
  row has one Level-1, Level-2, or Level-3 disposition.
- S2 gives the existing sole CPU delivery route a traceable logical first-INTA
  acknowledgement before its existing vector/frame entry. PIC remains the sole
  owner of IRR/ISR state; it claims no electrical duration or waveform.
- S3 makes the existing paired-controller selector consume ICW1.SNGL and both
  ICW3 forms rather than treating IR2 as an unconditional cascade line.

## Accepted Progress

S1 is accepted at `254acf94`. It preserves T450's 18-row source universe,
visually checks the scanned Intel data sheet, and records independently pinned
86Box, Bochs, MAME, PCjs and QEMU observations without promoting any of them
to specification. The result identifies the one current configuration conflict
(ICW3/SNGL accepted but not consumed) and the one contract receiver (logical
request-to-CPU acknowledgement phase). The [S1 evidence](../etc/evidence/t456-s1-pic-source-reference-contract-reconciliation.md)
defines the bounded S2--S4 order; no runtime code or artifact changed.

S2 is accepted at `66d68469`. Its
[evidence](../etc/evidence/t456-s2-pic-logical-acknowledgement-contract.md)
records the manual-derived phase order, one production-caller sweep, focused
trace proof, corrected protected-mode rejection observations, full 293-test
current-gate result and retained ICW3/SNGL and physical-boundary receivers.

S3 is accepted at `92df2836`. Its
[evidence](../etc/evidence/t456-s3-pic-programmed-cascade-contract.md)
records the one PIC-local ICW3/SNGL relation, its separate synthesized cascade
request, AT/custom/mismatch/single-mode/reinitialization proof and the
retained poll-command receiver. The configured build, focused 11-test group,
documentation governance, and independent 293/293 current-gate replay pass.
No VM/profile topology or additional selector is introduced.
