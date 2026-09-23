# M2 App/Core Design Evidence

## S1 Baseline And Inspection

M1 closure: 5d70a12. Shared source pin: 0fb40f48, 180 unchanged units.
Inspected common/machine/machine_interface.h and machine.c, session provider
contracts and control dispatch. Core implementation does not yet exist.

Observed constraints: start only STOPPED; reset STOPPED/RUNNING/PAUSED; media
STOPPED/PAUSED; ERROR rejects those operations. Media returns bool, not a rich
parser error. Reset-to-paused relies on the driver invoking the executor callback
before guest work. Paused callbacks service media, so a parked run stack cannot
retain a retired hardware pointer. Debug payload capacity is 128/1536 bytes.
These are code observations, not new shared interfaces.

## Design Coverage And Remaining Work

| Batch | S1 disposition | Next proof owner |
| --- | --- | --- |
| Ownership/dependencies/composition | Specified in architecture; value-only Core debug header exception explicit. | T4 detailed contracts and later include/link gates. |
| Load/run/reset/stop/shutdown | Production flow and failure boundaries specified against current Common. | T4 command/state matrix, then integration candidate. |
| Debug/config | Copied protocol, paused ownership, capacities and failure constraints specified. | T4 exact public layouts/commands and bounded operation matrix. |
| CPU/bus/cartridge | Proposed finite mapper-0 synthetic scope; no game or full CPU claim. | T4 exact opcode/address/timing/interrupt and ROM validation tables before implementation. |
| Dual KVM | Existing equal-backend/Core-text ownership retained. | M3 admission; no M3 task decomposition here. |
| Product implementation | Not admitted; ordered M2 candidates established. | Owner review and future packets after design acceptance. |

T4 closes only after all implementation-critical rows have exact contracts and
named acceptance cases. S1 establishes architecture and planning; it does not
close T4 or M2. No shared-core-neutralization work or redundant import proof is
introduced. The next dynamically admitted S resolves the detailed design rows.

## Verification And Review

The complete documentation gate and its self-tests passed. Actual-diff review
confirmed no diff in the four shared roots. Gate tests cover the explicit M2
design state and reject product source, misplaced tests, artifacts and wrong
milestone. No runtime suite is claimed for these documentation/tooling changes;
M1 runtime evidence remains in its own task history.

Source/test counted paths: zero changed. Governance tooling: one tracked file,
43 added lines, zero removed (Git numstat, excluding documentation). The increase
adds one explicit design delivery mode plus positive/negative fixtures; it keeps
the existing gate as the sole production verification path. No runtime wrapper,
parallel validator or product scaffold was introduced. One initial self-test
failure exposed an unrelated fixture root README; its fixture was reconciled
with the declared four-root inventory and the complete suite then passed.

## S1 Coordinator Acceptance

Reviewed actual delivery `39435f4`, including all new proposals/evidence, design
authority changes and the gate/fixtures. The original request maps to M1 closure
`5d70a12`, explicit M2 admission, T4 design admission, and the ownership/production
flow delivered in S1. Full governance and self-tests pass; four shared roots have
zero changes since M1 acceptance. No source/build scaffold or runtime artifact
was created. S1 is accepted; T4 remains open for detailed contracts recorded above.
There is no active execution packet between subtasks. Local-only delivery applies
because no remote is configured. Temporary gate fixtures were cleaned by the
self-test; no unrelated user assets were removed.

## S2 Complete Design Delivery

The owner's expanded request was to finish all App/Core design and stop for
interactive verification, explicitly consulting authoritative archives. The
[D01-D15 ledger](../app-core-verification.md) is the complete design universe.
All S1 outstanding rows above now have concrete contracts and named future
acceptance cases. They are historical pending rows, superseded by S2's design
dispositions, not outstanding runtime work inside T4.

Delivered module/public interfaces, construction/unwind, command/state and
configuration grammar, nine bounded debug messages, all 151 official opcode
forms, bus/interrupt/reset sequencing, strict cartridge transactions, PPU/APU/DMA
and scheduler boundaries, equal pixel/text routes, controller mapping, host
pacing, persistence prerequisites and independent verification ownership.
M3-M5 contracts describe architecture only; no tasks for those milestones exist.

Inspected the adopted Common machine/session/UI implementation to reconcile
startup media failure before session_run, the mandatory callback before guest
execution, stale hardware pointers across parked media replacement, actual
frame-kind routing, no paused frame publication, source-retirement synthesis,
paused input suppression and ordered teardown. Core's signal-only reset and
bounded resume drain accommodate current input behavior; the real Common
integration cases must prove it during implementation. No shared patch is made.
Guest traps now enter inspectable PAUSED; genuine host failures remain terminal
ERROR. This explicitly supersedes S1's all-faults-ERROR treatment.

The [source register](../hardware-references.md) identifies original MOS editions,
consulted archival transcriptions, NES-specific hardware research and local ABI.
Visual6502's recognition gates corrected an overly simple interrupt-vector
takeover description before delivery. Archive access limitations are recorded;
there is no claim to have downloaded or hashed an inaccessible scan. No manual,
test ROM or emulator implementation is imported. MyNes scope, deterministic power
seed, approximate palette, ASCII-only paths and text algorithm are product
choices, not claims about Nintendo hardware.

The three unadmitted M2 implementation candidates were consolidated into one
execution/management candidate so its first source delivery can contain a real
App/Core executable. The full former scope remains in that proposal and matrix.
No numeric identifier or source admission results from this planning refinement.

Verification: full documentation governance and whitespace checks passed;
numerical inspection confirms 151 unique opcodes, 56 mnemonics, 105 excluded
bytes and eight conditional branches, ROM bound 40976 and debug response bounds
272/60 bytes. Actual source/test/tools/build diff against S1 acceptance d2f5093
is empty. No new runtime test result is claimed or required for these design-only
changes. Existing M1 runtime evidence is retained. Source/test added/removed/net
lines are all zero. Future Lib audio and atomic replacement are named upstream
capability prerequisites with M4/M5 receivers; no local workaround is authorized.

Review boundary: design completeness is delivery for owner interaction, not
owner acceptance of each product choice or permission to implement. T4's main
history records coordinator review and closure separately.

## S3 Component Boundary Correction

Owner review found that naming a separate core/host layer misrepresented the
Lib/Common host boundary and the requested NXVM organization. The correction
places actual NES driver integration in core/machine, with no new execution or
presentation mechanism. Public composition uses core_driver; deterministic
core_machine remains independently owned and replaceable on media acceptance.
Source map, dependency graph, protocol include path, test ownership and build
groups are reconciled. Historical S1/S2 terminology above is preserved as evidence,
not current architecture. Runtime sources, tests and shared APIs remain unchanged.
Documentation governance and whitespace checks pass. Actual-diff review covers
the removed paths, renamed proposed handles, receiving proposal and the complete
file responsibility table. Source/test/tools/build delta is zero. S3 P1 is a
reviewable design delivery; owner confirmation and coordinator closure remain
pending, with the corrective packet retained.

S3 P2 responds to the owner's P1 tree correction: flat src/core and test/core;
only app/core/common/lib/integration test roots. Removed the support-root design,
kept helpers within their owning component and reconciled all current include,
test and proposal paths. Historical directory descriptions above are superseded.
The whole updated tree passes documentation and whitespace checks; source delta
remains zero. This is corrected design delivery, not the final acceptance P.

S3 acceptance: owner instructed closure after P2. Coordinator reviewed both S3
deliveries and confirmed the final flat component layout, existing removable
media route and unchanged shared baseline. T4 is closed; its main history records
the final verification and queue transfer. The earlier pending-review statements
above are retained chronological evidence and no longer current status.
