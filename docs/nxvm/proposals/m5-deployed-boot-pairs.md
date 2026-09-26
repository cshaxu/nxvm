# Deployed EXE/INI Boot Reliability

Owner admitted M5 T538 on 2026-09-25. Primary target: NXVM.

## Goal And Boundary

Every current XT, AT, Model 40 and default PC/AT executable, on x64 and x86,
must load its actual adjacent NXVM.ini and reach its supplied DOS boot or
installer environment after start. Diagnose pre-monitor exits, command dispatch,
guest faults/no-progress, and presentation failures separately. Existing probe
success is not deployed-product proof.

Keep selected hardware and owner media choices. Repair repeated mechanisms at
their actual owner; no BIOS-screen special case, alternative loader, synthetic
F1, success fallback or second lifecycle path. Shared source requires separate
owner review. Owner subsequently separated the accepted S2 KBC repair from S3's
Console backing-buffer repair: Shared Console broker/tests/contracts and both
products' receiver builds are admitted in S3, without MyNES source or INI changes.
No font scaling or automatic
Window fallback is allowed. Assets stay external and masters unchanged.

## Evidence-Led Subtasks

1. S1: freeze the eight-pair baseline and real-process failure matrix; identify
   repair batches and their production owners.
2. S2: repair the reproduced AT KBC typematic/serial-delivery class on both
   widths; record the resolved Model 40 capture ambiguity. Owner accepts this
   bounded keyboard repair separately from the newly reproduced Console defect.
3. S3: deliver complete Console backing storage independently of viewport size.
   Preserve font/viewport, write offscreen cells and clear stale lower rows;
   reject genuine buffer/I/O failure. No frame truncation or Window fallback.
   Verify Shared regressions/manifests and both products' dual-width receivers.
4. S4: import reviewed SoftPC Console output-extent and retained retry coverage
   unchanged; verify six shared source/test roots and both products' receivers.
   Buffer capacity, committed-frame cache and pending tail coverage remain
   separate concepts inside the same broker owner and write path.
5. Final S: dual-width release deployment, complete units/integration, repeated
   real EXE/INI boots and owner manual validation handoff.

The [convergence ledger](../etc/evidence/t538-boot-pairs.md) fixes the corpus.
Each case records executable/INI/input identity, process exit or semantic screen,
reproduction count, root-cause owner and regression evidence. Time limits bound
runs; they never mean boot success. Start with 180 seconds per launch and three
fresh launches per final pair to expose intermittent behavior. S1 permits a
300-second Model 40 x86 diagnostic rerun: its 180-second capture already began
the DOS title, so this tests completion of demonstrated progress, not an
assumption that a blank screen is healthy. Other cases retain 180 seconds.
Stop owned child
processes after capture; do not terminate unrelated applications.

## Exit

All eight pairs reach DOS prompt/date input or the DOS installer; no unresolved
exit/hang is accepted as success. Input and pause/resume remain usable. Complete
repository-only units pass on both widths and the established external integration
suite passes without deleting cases. Rebuild optimized stripped 0538 EXEs if
production inputs change, verify before replacing 0535 pairs, and retain only
eight current NXVM EXEs. INI changes require an explained defect and evidence,
not silent replacement of user choices. Record source/hash provenance, review
actual changes, commit/push per target, and leave the tree clean.
