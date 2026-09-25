# M6 T43 S4 SoftPC Six-Component Import Audit

## Decision And Frozen Scope

The candidate is eligible for unchanged source import, followed by a complete
receiving build and product regression. This is an audit verdict, not a claim
that either receiving product has already been rebuilt or qualified with it.
No Shared or product implementation is modified by S4.

- Canonical receiving baseline: NXVM `440ae83bc6e201bea2dcbdb84cc7fa442e9024dd`.
- Clean candidate: SoftPC `dc9c34ce3fa2794840762e26c26801cd098c41ab`.
- All differences originate in SoftPC `503d66328eb3e4e85b71116cc5a5f266877683bb`.
- Universe: every tracked file in the six roots, including manifests, tests,
  documentation and build descriptions. Both worktrees were clean at capture.
- Method: tracked-path inventories, per-file SHA-256, complete `git diff
  --no-index`, then review of each changed implementation and its callers.

| Root | Files in each repository | Different files | Disposition |
| --- | ---: | ---: | --- |
| src/lib | 109 | 5 | Four 80x50 implementation/contract records and manifest |
| src/common | 23 | 2 | Status-frame geometry and manifest |
| src/x86 | 14 | 0 | Identical |
| test/lib | 51 | 7 | Six corresponding regressions and manifest |
| test/common | 20 | 3 | Two publication/composition regressions and manifest |
| test/x86 | 10 | 0 | Identical |

All 227 paths exist on both sides: 210 are byte-identical and 17 differ.
There are no additions or deletions. Types, its gate/self-test and the earlier
canonical process/storage/audio capabilities are retained byte-for-byte.
The source/test diff excluding README and manifests is +106/-28, net +78:
production +19/-11 and tests +87/-17. This count describes the candidate;
S4 itself changes documentation only.

## Complete Difference Ledger

Paths below are relative to their named root. Every row was reviewed.

| Root/path | Change and review result |
| --- | --- |
| src/lib/console/console_interface.h | Text capacity 80x25 to 80x50; active rows remain explicit. Neutral bounded value contract. |
| src/lib/kvm-base/frame_interface.h | Same capacity increase; fixed 80-cell row stride retained. Palette and cursor contracts unchanged. |
| src/lib/console-broker/win32/console.c | Surface growth takes active rows and returns bounded write height. Clears lower rows after a taller frame, preserves existing viewport/buffer dimensions, checks native failures and clipped writes. Existing output lock and cache owner remain. |
| src/lib/README.md | Documents capacity versus active geometry and bounded clearing. Matches implementation. |
| src/lib/MANIFEST.sha256 | Hashes the four preceding changes; verified against committed export. |
| src/common/ui/ui.c | Graphical-status text explicitly stays 25 rows; text placement uses active dimensions. Prevents capacity becoming a new default mode. |
| src/common/MANIFEST.sha256 | Hashes that UI change; verified. |
| test/lib/console_broker_display_smoke.c | Adapts owner-local surface helper calls; retains 13-row, scrolled/wide viewport and rejected/clipped resize tests. |
| test/lib/kvm_console_retirement_barrier_smoke.c | Rejects 51 rows; proves cell 3999 reaches the logical Console with character bank and colors. |
| test/lib/kvm_frame_copy_smoke.c | Moves capacity-end validation to cell 3999 and checks new struct sizes; retains active-tail and copy guards. |
| test/lib/kvm_frame_damage_mouse_smoke.c | Adds 22/25/43/50-row rendering, equal-pixel-size 25-to-50 transition, bottom-right cell and cursor checks. |
| test/lib/lib_console_io_contract_smoke.c | Captures native writes, verifies taller last cells, shrink clearing and failed viewport growth without a write. |
| test/lib/lib_console_smoke.c | Updates value size and unsupported extent to 51. |
| test/lib/MANIFEST.sha256 | Hashes those six tests; verified. |
| test/common/composition_smoke.c | Confirms 25-row status and 50-row application frame delivery through Common UI. |
| test/common/machine_wait_smoke.c | Confirms last-cell changes affect publication sequence and unchanged frames do not republish. |
| test/common/MANIFEST.sha256 | Hashes both tests; verified. |

No new API function, component edge, product branch, queue, state owner,
platform dependency or external asset is introduced. Review followed the local
architecture-governance and coding-governance skills: capacity and native output
remain with their existing owners, and the tests exercise the actual shared path.
The changed units retain the owner-provided Shared provenance grant recorded in
`../provenance/softpc-shared.md`; no independent third-party code/notice was added.

## ABI And Receiving Products

Text values grow: `kvm_text_frame` and `lib_console_text_frame` 8084 -> 16084;
`kvm_window_text_frame` 16276 -> 24276; `kvm_console_text_frame` 9108 -> 17108.
`kvm_window_frame` stays 984084 because its graphics union member dominates,
but text-field/font offsets change. Recompile all callers and libraries from
the same revision; equal outer size does not permit mixing old/new objects.
There is no changed serialized product format in this diff.

MyNES `src/app-mynes/core/driver.c` explicitly publishes 80x25 ASCII output,
fills 25 rows and retains 16-pixel cells. It does not use the capacity constant
as a mode default. Graphics remains on the unchanged Window image path.
Its driver compiles unchanged against candidate headers on both host widths.

NXVM `src/app-nxvm/machine/frame.c` copies the product's 2000-cell event into
the larger zero-initialized destination. Its actual geometry remains the
source geometry. The existing frame adapter smoke passes on both widths.
However, `devices/display_interface.h`, `devices/guest_display_frame.h` and
`machine/event_interface.h` still own 25-row product capacities; the display
producer clamps to its product bound. This import does not implement NXVM
43/50-row guest modes. That would require a separately admitted NXVM change
covering the complete device-snapshot/event/adapter/font chain and regressions.
The adapter's Shared-capacity check alone must not be used to declare those
larger product events supported.

The similar-issue sweep checked both product source/test trees for
`KVM_TEXT_ROWS`, `LIB_CONSOLE_TEXT_ROWS`, active row assignment, fixed text
buffers and literal struct-size assumptions. No receiving change is required
for the currently produced 25-row/text or graphics frames. This does not prove
complete application execution after import; the import task must run both
products' required dual-width builds and unit/regression gates.

## Verification And Limits

The SoftPC committed six roots were exported with `git archive` into an isolated
audit directory under this repository's ignored build tree. The original sibling
was read-only. All six exported manifests, test Types boundary, Lib component
DAG/Types layout, Common corpus and x86 corpus checks passed. This also proves
the candidate hashes are reproducible from its Git commit, not just its worktree.

Independent strict C11 Release builds used the candidate's unmodified CMake
entries. Common x64 tests passed 18/18. All six changed Lib executable tests
passed on x64, including the native Console viewport test. The existing NXVM
frame adapter test was built and passed on x64 and x86 with candidate headers
first in the include path. MyNES driver compiled with those headers on both
widths under `-Wall -Wextra -Wpedantic -Werror`.

These are bounded compatibility checks, not a full dual-product test run or
Linux runtime claim. SoftPC's own current record reports 120/120 background
tests per width, with five desktop tests excluded and a separately deferred
product BIOS null-dispatch fault. That product fault is outside these 227 paths.
Its record is supporting context, not receiving-product acceptance.

Manifest comments still name `shared-m6-t43-s3-p1` despite four manifests having
new hashes. They pass verification, but that label alone no longer identifies
these bytes. An import must pin the exact SoftPC commit and preserve all six
roots, including tests and manifests. A future upstream label refresh would
need a newly pinned audit; it is not a reason to edit an unchanged import locally.

S4 requires MyNES documentation governance, whitespace checks and actual-change
review. No new executable is deployed for this audit-only S. The next authorized
import can use the pinned candidate unchanged, rebuild all dependencies and
verify both products before reporting runtime acceptance. T43 remains open.
