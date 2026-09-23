# M5 T456: Core PIC 8259A Phase Contract

T456 implements the finite Core 8259A command, priority, cascade,
acknowledgement, EOI, mask and reset contract after a manual-first inventory.

## Completed Subtasks

- S1 reconciles T450's original-source and current-code ledger with rendered
  manual pages and independently versioned emulator observations before
  implementation. It defines the bounded S2--S4 plan only after every retained
  row has one Level-1, Level-2, or Level-3 disposition.
- S2 gives the existing sole CPU delivery route a traceable logical first-INTA
  acknowledgement before its existing vector/frame entry. PIC remains the sole
  owner of IRR/ISR state; it claims no electrical duration or waveform.
- S3 makes the existing paired-controller selector consume ICW1.SNGL and both
  ICW3 forms rather than treating IR2 as an unconditional cascade line.
- S4 closes the retained command/lifecycle matrix, including the corrected
  rotate-on-specific-EOI state transition and a full minimalism audit.
- S5 corrects the omitted initialized no-request acknowledgement result to the
  manual's default master IR7 vector without an electrical-race claim.

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

S4 is accepted at `c3fbe185`. Its
[closure evidence](../etc/evidence/t456-s4-pic-retained-command-closure.md)
maps every retained row to its source-backed logical proof or maintained
boundary, records the specific-EOI repair, and preserves the selected-board and
electrical non-claims.

S5 is accepted at `727c2ae5`. Its [default-IR7 acknowledgement evidence](../etc/evidence/t456-s5-pic-default-ir7-acknowledgement.md)
corrects S4's omission: an initialized empty acknowledgement returns the
master's ICW2 base plus IR7 and leaves IRR/ISR intact. It preserves one
scan-gated CPU delivery path; the 293/293 current gate and documentation
governance pass.

The P1 actual-change review confirms the one PIC-local command correction,
removed pseudo-public priority helpers and OCW1 alias, the two missing
command/poll regressions, and no new dispatch or state owner. Its focused
group, configured build, documentation gate and isolated 293/293 current-gate
replay satisfy the bounded task exit.

## Closure

T456 closes the prepared 8259A L3 implementation increment. S1 reconciled
every source row; S2 supplied the sole CPU logical acknowledgement boundary;
S3 made the one selector consume programmed cascade state; S4 closed the
retained commands and lifecycle; and S5 restored the missing default-IR7
result. The retained [proposal](M5-T456-core-pic-8259a-phase-contract-proposal.md)
and S1--S5 evidence preserve all source, proof and maintained-boundary detail.
L4 electrical timing is deliberately outside this task. XT/AT personality
selection, later ELCR/PCI routing and APIC/IOAPIC remain the separate
source-gated TODO receivers; none is evidence of an incomplete L3 contract.

## Corrective S5

The owner reopened the most recently closed T456 after the S4 `PIC-F8` claim
was found to omit default-IR7 acknowledgement. S5 is accepted at `727c2ae5`:
its focused regression, configured build, documentation governance and 293/293
current gate close the correction without an L4 claim. Its evidence is
[T456 S5 default-IR7 acknowledgement](../etc/evidence/t456-s5-pic-default-ir7-acknowledgement.md).
