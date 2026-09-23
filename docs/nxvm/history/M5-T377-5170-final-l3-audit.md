# M5 T377: IBM PC/AT 5170 Final Model-L3 Audit

## Task Record

T377 makes the final evidence-backed ready/not-ready decision for the frozen
IBM PC/AT 5170 Model 339/Type 3 baseline: 8 MHz 80286, Rev.3 BIOS, 512 KB,
CGA and no fixed disk. It consumes the completed capability, functional,
board/phase and raw-IMG fidelity work. It is audit-only: a negative result
must return a bounded repair to its earliest owner rather than silently widen
or downgrade the baseline.

## Active Progress

### S1: Requirement and evidence reconciliation

S1 inventories every selected Model-339 capability and checks its functional
and timing evidence against the original final-audit proposal. It must
distinguish the factory configuration from the supported aftermarket 1.44 MB
drive/raw-sidecar condition and retain physical-media/board-phase exclusions.

P1's preliminary ready conclusion was rejected in same-session coordinator
review: its full-gate replay proves that native Win32 keyboard input bypasses
the required transport ingress, which directly invalidates the selected
Model-339 input path. Corrective P2 records the negative decision and ordered
repair/re-audit receivers in [the independent final audit evidence](../etc/evidence/t377-s1-model339-final-l3-audit.md).
No Model-339 L3 acceptance is made by this task.

## Closure Audit

T377 closes with the required evidence-backed **not-ready** decision. It
re-read all selected CPU, memory/ROM, bus, DMA/PIC/PIT/RTC, input, FDC/floppy,
CGA, NMI/reset/cancellation and cross-device rows; its focused replay passes.
The configured current gate exposes a real selected-keyboard production defect,
so the task neither hides it behind controller tests nor treats the other two
global failures as an all-green result. The immediate queued repair owns the
native input and dependency failures, and the following re-audit owns the next
and only further Model-339 readiness decision. Physical-media and board-phase
boundaries remain explicitly excluded. The task proposal is retained beside
this record.
