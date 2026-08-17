# T390 S16: Model-40 Checkpoint And Finite-Corpus Disposition

## Audit Result

S6--S15 validly repaired individual, observed Intel-primary 80386 forms, but the accumulated first-unallocated/replay sequence is not the task's completion method. S15 removes source-unallocated successful retirement through the existing 2,000,000-retirement containment run, yet that run does not reach the S2 `0:7C00` checkpoint. Budget exhaustion is neither a boot failure nor corpus completion.

The selected composition remains the S2 backbone: 80386, 1 MiB, no FPU, one 1.2 MB removable drive, no fixed disk, cold reset and the normal external-ROM validation/mapping route. A fresh owner-managed, temporary normalized replay completes its 2,000,000-retirement containment budget with all successes classified and no run-status or fault terminal. It observes both protected-mode and later real-address execution, including large finite loop-shaped work. This excludes an unallocated-success blocker and an immediate CPU/session fault at that boundary. It does **not** identify the ROM's POST phase, prove forward FDC/media progress, or prove a control-flow defect: the retained capture intentionally has no PC trace, port identity, device-state trace or firmware semantics. The truthful diagnosis is therefore **pre-boot phase unresolved**, not "normal POST" and not "FDC failure."

## Frozen Finite Corpus

The first batch corpus is named **M40-C0: protected-return initialization**. It begins at the first successful retirement after cold reset and ends at the first later successful real-address retirement after at least one successful protected-mode retirement has been observed. The transition is defined entirely by the copied retirement observation's mode fields; it requires no ROM address, byte, asset identity or external emulator. This is a finite semantic checkpoint, distinct from the later boot-sector checkpoint.

A later, separately admitted **M40-C1: boot transfer** corpus begins after M40-C0 and ends only at the existing S2 `0:7C00` checkpoint. C1 must add semantic device/readiness checkpoints before it can diagnose POST versus FDC/media progress. Neither C0 nor C1 is a board-timing or L3 claim.

## Batch Receiver And Physical Boundary

The next T390 continuation receives M40-C0, not the next first-unallocated opcode. It must:

1. extend the existing lifecycle-owned copied observer only as needed to stop at the M40-C0 semantic checkpoint and aggregate every successful form/context without persisting raw PC, firmware bytes, media data or asset identity;
2. normalize each aggregate class by CPU family, prefix/escape, opcode group, operand form, mode, CPL, operand/address size, repeat/LOCK and every classifier-specific fact needed to choose an Intel row;
3. create one finite matrix for the whole C0 run, batch-map every class to an exact Intel-primary row with project-owned regression, and transfer every missing-row or missing-capture class before physical selection; and
4. rerun C0 to its semantic checkpoint with no unallocated success, fault, form-capacity or containment terminal.

Only after those four conditions may a later T390 decision consider **M40-C0-only** physical retirement. It cannot enable physical retirement for C1, publish a CPU-to-board ratio, or begin DeskPro board timing. The existing 2,000,000-retirement cap remains a research containment limit; it is not a C0/C1 success proof.

## Similar-Issue Reconciliation

S2 already defines `0:7C00` as a later boot-transfer checkpoint and declares caps non-semantic. S5/S6--S15 progressively used first-unallocated terminals to make Core repairs; their exact rows and rejection boundaries remain accepted, but their transfer wording must not imply that another one-form continuation is the default. S13 correctly made budget exhaustion explicit. S16 reconciles these facts: a finite semantic corpus plus a batch matrix, rather than an unbounded boot replay or success-by-budget, is the required receiver.

## Coordinator Review Scope

This S changes no runnable source, external asset, product interface, physical contract or technical artifact. Documentation governance and diff hygiene verify the authority/index changes; the retained local normalized replay is diagnostic evidence only. The next implementation S must run the stated focused and full source gates after it changes the capture path.