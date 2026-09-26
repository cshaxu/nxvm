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
