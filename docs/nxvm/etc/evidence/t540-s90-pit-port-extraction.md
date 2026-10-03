# T540 S90 PIT Port Attachment Evidence

Baseline: accepted S89 `47dfd87b7`. S90 is accepted; T540 remains open.

## Complete Receiving Boundary

The old pit_bus.c/h embedded device/base glue moves to flat ibmpc-common.
The final design needs neither a new binding allocation nor a public layout:
seven stateless per-selector callbacks borrow the existing opaque PIT. One
operation installs all four ports atomically through Core. This preserves
arbitrary valid base addresses, including non-aligned bases, without deriving
selectors from address low bits. The control port remains write-only.

Board composition now explicitly creates each primary/auxiliary chip, installs
its ports and retains its unchanged reset/clock/OUT/destruction operations.
Every deadline, refresh, speaker, IRQ and direct fixture uses that same chip.
Port publication failure leaves caller-owned chip storage intact; existing
whole-candidate rollback destroys it. Core attachment finalization destroys
the chip before Core discards routes, without intervening dispatch. Independent
owners remove chip-token routes or destroy Core before destroying a chip.
No getter, new scheduler, state mirror or chip-operation wrapper is introduced.

The entire unused display-mode half is removed: callback type, bind arguments,
two stored fields, sole App callback and test arguments. Copied snapshot
dispatch and freeze behavior retain their original path.

## Observed Proof And Remaining Work

The independent corpus builds and all 124 registered cases pass, including
both chip personalities, three base-address forms, selector read/write,
duplicate rejection, route removal/reinstallation and caller-owned chip
survival. Existing App allocation-failure coverage retains all seven port
failure positions; its old chip-null assertion is replaced by the actual new
ownership invariant, rather than dropping rollback coverage.

The first draft of the new independent test incorrectly used public bus access
before freeze/reset and latched CR before its chip input clock transferred it
to CE. The test now uses the actual lifecycle and one chip clock; no production
behavior or expected hardware value was changed to satisfy that draft.

An initial sandboxed Ninja process was alive without compiler children or new
output. Its exact owned CMake/Ninja identities were inspected and terminated;
the same tree then built successfully outside the sandbox. This containment
is not a test pass and did not launch overlapping builds.

Full x86 unit/build/gate job 5701 is terminal success: 472/472 units in 64.50
seconds and all 121 subsequent build/gate steps, with the 402-row strict matrix
retaining 381 strict and 21 deferred entries. The x64 full suite and eight
product-build jobs are now launched and must be polled, not restarted.
The x64 result, affected eight product checkpoints, final six
manifests/documentation checks, line counts and actual-diff acceptance remain
required before any complete delivery. No MyNES build or owner INI change is
authorized. Ignored `build/s90-*` helpers/logs are retained for this active
receiver's proof and immediate next-board comparison.

Remaining T540 members are PIC/DMA aggregation, shared board construction/time,
AT/XT family wiring and genuine machine-specific D4 ownership. This PIT
receiver does not imply that those members are complete.

## Executor Actual-Diff Review

Compared against `47dfd87b7`, the old route selector subtracts the stored base
address; the new seven callbacks encode the same selector without retaining
that redundant binding state. Read results are still published only on success,
writes still narrow to one byte, and the control port is still write-only.
Creation/destruction now belongs visibly to board composition; its original
failure cleanup and PIT-before-PIC teardown order remain intact. Primary and
auxiliary clock/deadline and electrical bodies differ only in opaque-chip
member access. The existing IRQ0, reset/deassertion, refresh, parity and D4
assertions retain their exact expected values. Integration changes are only
the same two diagnostic chip accesses, not checkpoint or input changes.

The Shared source gate permits only the real new Core/PIT public dependencies;
negative probes reject their private headers. The App authority gate forbids
both retired PIT source paths, and the tail gate follows the new chip-member
spelling rather than losing that check. No chip algorithm, MyNES or owner INI
content differs from the baseline. All six complete manifest inventories,
documentation governance and whitespace checks pass. The first direct Lib
manifest command used its wrong parameter name; explicit `LIBRARY_ROOT`
verification then confirmed the separate `test/lib` inventory as well.

Final index/worktree `git diff --numstat HEAD` over source/test/build/tool
paths initially counted 31 C/header/CMake paths, +298/-200, net +98. C/header-only
counts are 24 paths, +285/-192, net +93. Counts exclude documentation,
manifests and generated/binary artifacts; moves are counted as old-path
deletion plus new-path addition. The increase pays for stateless selector
callbacks and the new six-context public-boundary test, not another owner or
production path. Shared staging contains only its thirteen source/test/manifest
and README paths; the App deletions belong to the separate NXVM delivery.
Production C/header changes alone are eleven paths, +138/-129, net +9;
test C/header changes are thirteen paths, +147/-63, net +84.

Root x64 verification handle 74568 is terminal success: 472/472 units in
248.18 seconds, all subsequent 122 build/gate steps and the same 402-row
strict/deferred matrix pass. The longer measured runtime is recorded as such,
not replaced with the earlier x86 time. Eight-profile build handle 95040
continues producing output; default x64/x86 builds are successful. Their
one-shot original-INI checkpoints start only after the native unit suite has
finished. Other products/checkpoints remain pending. No test/build timeout
or observation delay is treated as terminal success.

Default x64 and x86 each pass their single original-INI checkpoint with
`dos-prompt`; each also runs the independent neutral-link proof successfully.
The two retained `s90-default-*-boot.log` records are not to be rerun while
qualifying the remaining XT/AT/Model40 pairs. The product-build helper remains
live for those pairs; no native test suite overlaps these boot runs.

XT x64 and x86 also each pass their one original-INI `installer-running`
checkpoint and neutral-link proof (boot handles 51645 and 61789, terminal
success). Both AT and both Model40 checkpoints remain required and have not
yet been run at that stage. AT x64 subsequently passes its single
`installer-running` checkpoint and neutral-link proof (64433, terminal zero);
AT x86 and both Model40 checkpoints are still pending. The staged new test
passes the complete global Types scan;
the manual first invocation omitted its specific source-root argument, then
the configured argument was used successfully (handle 37469, terminal zero).
AT x86 subsequently passes its single `installer-running` checkpoint and
neutral-link proof (54524, terminal zero). Only both Model40 checkpoints remain
unrun; no preceding successful checkpoint is repeated.

## Final Link-Selection Correction

Both Model40 single-run checkpoints subsequently pass (`installer-running`,
handles 17289 and 32165, terminal zero), completing the eight original-INI
checkpoints. Their exact pre-correction product hashes are retained in
`build/s90-prelink-artifacts.log`.

The final generated-link audit then finds that the common board archive's
transitive `x86-core` link brings the ordinary Core into observation tests
alongside `x86-core-observable`. Debug's shared trace setting concealed the
selection error. Composition now selects exactly one Core implementation;
the board archive consumes the public header contract without selecting an
implementation. The independent route test explicitly selects ordinary Core.
Two negative probes prohibit either Core implementation as a transitive
common-board dependency. No runtime algorithm or new API is introduced.

Earlier unit/build results are superseded for final delivery by verification
of this corrected graph. Final x86 full units pass 472/472 in 66.00 seconds
and all specialized gates pass (handle 17330, terminal zero). The independent
suite passes all 130 registered cases, including 126 unit cases, in 61.26
seconds (18967, terminal zero). Four Release observation tests pass in 0.54
seconds (38914, terminal zero). Final x64 full verification is running as
10179; final product builds are running as 72873, 36657, 49796 and 83975.
These specific handles must be observed to terminal status. Completed boot
proof is retained only for byte-identical final products; any changed product
requires its own final checkpoint. No prior pass establishes that equality.

All four final product-build handles are now terminal zero. All eight final
products pass PE width, stripped-section, 0540 banner and input-freshness
checks, but all eight differ from their pre-correction hashes. Consequently
all eight final checkpoints remain required, once each, after x64 native unit
completion. The six manifest inventories and documentation governance pass.
All generated link lines in both full-unit trees and the Release tree contain
at most one Core implementation. Final source/test/build counts are 31 paths,
+304/-200, net +104; the production C/header net remains +9 and test net +84.

Final x64 handle 10179 is terminal zero: 472/472 units in 238.43 seconds,
all specialized gates and the 402-row compilation matrix pass (381 strict,
21 existing deferred). Final boot handle 85953 runs the eight final products'
matching unchanged-INI fixtures once each; default x64/x86 reach `dos-prompt`
and XT x64/x86 reach `installer-running`. AT and Model40 are pending at this
recording point. The previous pre-link checkpoints are not final evidence.

## Final Product Identity

Each filename remains under its existing `assets/nxvm/<profile>/` directory.
PE architecture, stripped sections, 0540 revision and input freshness pass.
These hashes, not the earlier pre-link hashes, identify the final delivery.

| Product | Bytes | SHA-256 |
| --- | --- | --- |
| Model40 x64 | 1335136 | `669781C52F2BD7CC2D210D9C1266F3894839CF352C917F2E505F1B74D5A6E7BA` |
| Model40 x86 | 1505971 | `1F0C415D27C5AE7AFBF44E0592B71B4FAC58BE70DBDEE78B0A7271604918D54E` |
| Default x64 | 1351453 | `7C038E2817D486C3E44BC9D5A587BAB1EDF3BD5B7B0BE6A89D0BC7AF92FBEFA0` |
| Default x86 | 1522286 | `402BC4CA2219CF7F9FA9CA7C555A7CFDEAB7A4885A31F93E37B58A7A83E7039A` |
| XT x64 | 1351421 | `4FCD8921F49EA8582112416A0643A64545AFEF1ACC396CAD623CAE4316162604` |
| XT x86 | 1522253 | `18A85AD3EFEE39AA1A78098DBB5D67D6AF346AA92B445F4757CD19022D63A9A7` |
| AT x64 | 1351487 | `30CED290C53B766492CB5559AB5EA4AEE6DC644226860EDE2E55E930A8BB0B61` |
| AT x86 | 1522321 | `FA7A2D052158B9ED653D3EB52F39CE6A0BE3FA5829EFE3EF1B56229ECC151B24` |

Final boot handle 85953 is terminal zero. All eight final fixtures pass their
one unchanged-INI checkpoint and neutral-link proof: Default reaches
`dos-prompt`; XT, AT and Model40 reach `installer-running`, both widths each.
The final `build/s90-final-*-boot.log` records supersede pre-link proof.
No MyNES source/test/document/artifact, chip algorithm or owner INI differs
from the accepted baseline. The complete S90 brief is ready for its ordered
Shared and NXVM implementation commits and subsequent actual-commit review.

## Coordinator Acceptance

Actual pushed commits `ddc957eaf` (Shared P1) and `f33730fea` (NXVM P2)
are reviewed against the complete packet, original binding bodies, all changed
fixtures/build gates and final proof above. All original hardware assertions
remain; the seven port-allocation failures retain atomic-route rollback and
now prove caller-owned chip survival. Core and chip private layouts are not
exported. The dead mode ABI has no remaining source/test/build caller. Each
commit contains only its declared target, and all eight deployed hashes match
the recorded final products. No excluded App, INI or chip algorithm changed.
This accepts S90 only; the remaining common/family/D4 owners remain required.
