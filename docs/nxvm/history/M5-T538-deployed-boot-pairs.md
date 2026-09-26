# M5 T538: Deployed Boot Pairs

Owner admitted 2026-09-25. [Proposal](../proposals/m5-deployed-boot-pairs.md).
S1 inventories and reproduces all deployed pairs; task remains open.

Identifier reconciliation: T537 is already present in immutable NXVM history
(including 630a5b014). The later T41 shared-adoption record is preserved as a
historical cross-product hosting record; neither it nor MyNES T43 resets the
NXVM sequence. This new implementation uses T538, not a reused lower number.

Coverage and results: [ledger](../etc/evidence/t538-boot-pairs.md).

## S1 Executor Delivery

Eight EXE/INI pairs and external inputs are frozen and attempted. Seven pairs
have an observed DOS/installer checkpoint; Model 40 x86 remains unaccepted.
The owner's intermittent 5170 303 report on both widths remains unresolved
despite three additional successful cold starts per width. No repair is claimed.
Full x64 units pass 335/335; x86 passes all 335 via non-desktop/desktop partitions.
The NXVM documentation gate passes. All deployed EXE and INI hashes are unchanged.

Actual-change self-review covers the Windows-only diagnostic observer, its
NXVM CMake target and four task documents. No Shared/MyNES or production source
is changed. The observer uses the real monitor, has bounded observation and
external watchdog containment, and deliberately has no passing CTest assertion.
It does not define a second guest execution path. The new diagnostic C file is
238 lines, plus eight CMake lines (246 added, zero removed, counted as physical
source/build lines excluding documentation/artifacts). This positive increase
provides real deployed-process evidence missing from the old monitor-only test.
Native APIs are confined to this Windows diagnostic executable, not product ABI.
Raw logs remain ignored for the immediately following diagnostic batch.
No product rebuild is required for this test/documentation-only S.

## S1 Coordinator Acceptance

Reviewed the actual six-file `5e6ed82cd` diff against the original request and
S1 packet: scope is baseline diagnosis, all eight are attempted, unresolved
members remain explicit, and neither a green unit suite nor a successful cold
boot is presented as T completion. CMake adds only the diagnostic executable;
it cannot become an automatic boot pass. The native observer's owned process
cleanup and stale Window-handle reset are checked. Documentation links, packet
shape, identifier history and unchanged Queue agree. S1 is accepted and closed;
T538 remains open for the two named residual classes. No Shared authority was
consumed and no product behavior was modified.

## S2 Executor Delivery

Owner explicitly accepts the 5170 keyboard repair as a separate S and transfers
the independently reproduced Console capacity failure to S3. KBC now prevents
unmatched Set-2 breaks from starting typematic and routes repeats through the
same serial/inhibit boundary as native input. This is one controller-owner
repair, not a BIOS or profile exception. Both original widths reproduce 303;
both repaired widths reach DOS Setup under the same 50 ms Return-release input.
Reset/resume and stop/start also reach Setup. Model 40 x86's complete visible
frame and paused memory resolve the earlier capture ambiguity without a guest fix.

The isolated S2 source uses the committed Shared corpus, passes 335/335 units
per width and 20/20 external integration, and rebuilds eight optimized stripped
0538 products. Final isolated AT processes each reach Setup and remain alive
through 180 seconds. The ledger records source/artifact hashes, unchanged INIs,
actual-diff review (+188/-55 across four source/test/build files), and the
same-mechanism sweep. Only NXVM changes belong to this delivery. Pending Shared
and MyNES work is preserved for S3. Whole-task repeat coverage remains pending;
neither the S acceptance nor the green integration suite closes T538.

## S2 Coordinator Acceptance

Reviewed actual implementation 1d80f11d8, not only its test summary: the KBC
change keeps one serial publisher, the new regressions cover unmatched breaks
and inhibited repeats, and the observer never treats its exit code as boot
acceptance. Its native APIs remain test-local. The eight deployed binaries are
the isolated KBC build recorded in the ledger; no Shared or MyNES files are in
this commit and no INI is changed. Old artifacts are recoverable from Git.

Original request, latest owner-directed split, packet, actual diff and retained
coverage agree. S2 is accepted and closed after its successful push. S3 is the
authorized next receiver for Console capacity and the already-preserved Shared
candidate; T538 remains open and the complete repeat matrix is not waived.
