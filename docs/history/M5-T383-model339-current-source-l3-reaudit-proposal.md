# IBM PC/AT 5170 Model 339 Current-Source L3 Re-Audit

## Purpose

Re-establish the ready/not-ready decision for the frozen IBM PC/AT 5170 Model
339/Type 3 against the current runnable source graph. T379 accepted the prior
deterministic L3 contract; T380 S2 subsequently changed the selected profile's
strict-start composition. This candidate follows that correction and precedes
the DeskPro 386 sequence. It is an independent audit, not a repair task.

## Required scope

Replay the selected 8 MHz 80286, 512 KB planar RAM, Rev.3 ROM-slot, CGA,
101-key keyboard, 1.44 MB field-upgrade FDC, no-fixed-disk configuration
against the current product path. Reconcile the S2 FDC bounce-page and BDA
memory publication, keyboard translation, conventional-media behavior and
A20 absent-memory provider with every prior selected-device, timing, reset,
NMI, cancellation and deterministic-order proof. Re-run the current gate and
make one fresh owner-visible L3 ready/not-ready decision. External ROM and
guest media remain owner-managed inputs only and are neither committed nor
identified by local path.

## Non-goals and stop conditions

No runtime repair, new device capability, Model-339 variant, MFM/ATA route,
physical waveform claim, DeskPro/XT work, Windows claim, firmware/media import
or reuse of T379's result without current-source replay. Stop on any missing
functional or timing proof; transfer the defect to its earliest owner rather
than absorbing implementation into this audit.

## Evidence standard

Require an independent current-source requirement-to-evidence matrix, explicit
pre-T380 versus post-T380 change reconciliation, selected-profile reset/input/
FDC/A20 replay, applicable current-gate evidence, and a ready/not-ready result
with every residual receiver named. The result supersedes T379 only for the
current Model-339 runnable graph; it makes no broader PC/AT or physical-timing
claim.
