# M5 T587 S5 - Token-contract import

Owner approved exact SoftPC 4cd56d018abd5328f0578dbfe0b7524f4bcfcce4 import
and receiving migration on 2026-11-18. The S4 audit is the design review.
PC110 now uses vdisplay dstn/tft and pccard insert slot mode common attribute;
quoted paths are lexed only by Emulator. No old grammar compatibility path.
Estimated exact import: 17 files +179/-93. Receiving adaptation: approximately
12-18 source/test files, production net reduction expected from removing
normalization, tokenization, trimming and raw-tail parsing. No extra framework.

## Implementation and actual-diff review

The eight imported trees retain every upstream byte. Lib has no change.
X86 NXVM Product now consumes name/argument tokens for info, speed and floppy;
its old whole-line normalization and path splitting are removed. Keyword case
comparison retains existing PC tolerance without modifying path bytes.
PC110 consumes the same tokens for vdisplay and pccard, removing its local
lexer, trim and pipe separator. Card opening, pending ownership, rollback and
stopped-only admission retain their original owners and implementation.
MyNES removes its second quote parser, validates/copies the decoded ASCII path,
and retains the NES Debug console parser and snapshot/ROM execution paths.

No legacy raw callback, reconstruction bridge, alternate dispatch, new thread,
persistent command state or changed INI format is introduced. Emulator owns
borrowed token storage for the synchronous callback only. Removable-media
Machine copies its path; PC110 opens files before returning; MyNES copies its
path and opens snapshot files synchronously. No receiving path retains tokens.

Existing tests were extended, not duplicated: PC110's production provider now
lexes quoted temporary card paths before the real Storage/Machine operations;
old pipe/equal syntax is rejected without replacing accepted pending media.
MyNES tests distinguish the shared whole-line bound from its own path bound.
The X86 NXVM factory test preserves case-insensitive speed and argument counts.
Upstream tests cover bounded tokens, quoting, malformed input and Debug N paths.

Historical records retain the original syntax. Current UX, help and asset
instructions use the new syntax. The remaining old syntax in active tests is
intentional negative coverage. Protected configuration/media hashes are checked
again at delivery. Dynamic qualification is recorded below when complete.


## Qualification and publication

Both stable Ninja/ccache Release trees built successfully. Each architecture
passes 716 unit and 48 integration cases. Additional profile checks pass on
both widths: My5160 20, My5170 22, MyDeskPro386 23. Only new or changed test
commands/executable hashes are selected for each profile; identical baseline
checks are reused. Default build configuration is restored afterwards.

MyRaidenII's existing opt-in selected-set wrapper passes its 2,000,000-instruction
startup probe and deployed-product lifecycle/Console smoke on both widths.
These four checks are not gameplay qualification. No runtime failure was retried,
no test assertion/budget was weakened, and no required case was skipped.
Automated native Window, Console and audio paths are included where registered;
manual desktop/RDP/physical input acceptance has not been performed.

Fourteen stripped Release EXEs are published in their existing seven asset
directories with correct PE widths and unchanged product version/filenames.
The 29 protected configuration, CMOS, media and snapshot files have unchanged
hashes. No Lib or X86 Core production/test changes were made. Eight imported
trees match the fixed upstream revision byte-for-byte: 296 files, zero
missing, extra or differing files.

Actual package diff (including package README/manifests):

| Scope | Added | Deleted | Net |
| --- | ---: | ---: | ---: |
| Imported eight trees | 179 | 93 | +86 |
| X86 NXVM receiving source/test | 48 | 65 | -17 |
| PC110 receiving source/test | 97 | 101 | -4 |
| MyNES receiving source/test | 30 | 41 | -11 |

Across C/H alone, production is +167/-216 (net -49), tests +160/-71
(net +89). Existing test fixtures own the added assertions; no new test
framework or redundant production dispatch path was added.

The [verification index](../etc/evidence/t587-s5-import-verification.json)
contains suite counts, command outcomes/timings, manifest results, artifact
SHA-256 values and log digests. Build commands retain the repository-local
ccache routing and stable Ninja trees. Aggregate cache counters are not
misrepresented as this task's hit rate.

Coordinator actual-diff review confirms the imported ABI migration is complete
across all receivers. The task owns the syntax change, receiving artifacts and
associated UX/evidence; no unrelated cleanup is included. All owned jobs are
finished before scratch cleanup; stable caches remain intact. S5 is delivered
for owner verification, not closed. T587 remains open.

Delivery commits: Shared import/PC extension adaptation `f060090a4`, MyNES
adapter and receivers `6f6a83af9`, MyRaidenII receivers `3ca9f9052`. The final
NXVM P4 owns PC110 adaptation, ten PC EXEs, provenance and the completed S5
qualification record. Every scoped P is pushed immediately after commit.
