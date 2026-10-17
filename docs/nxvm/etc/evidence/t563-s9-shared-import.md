# T563 S9 — Committed Shared Import

## Scope And Frozen Sources

Owner admission continues S9 from clean NXVM `7b85ae008`. The immutable
SoftPC source is `4a675c00390af2efb9adb9c5d0607d569ab0ba05`, rechecked at
execution. Export is from committed Git objects, never the sibling worktree.
The sibling is read-only. Its later input-ownership corrections belong to a
subsequent approved import, not a local edit to this transfer.

The eight canonical roots are `src/{lib,emulator,x86-debug,x86-ibmpc}` and
their same-named `test` packages. Their 288 payload files and eight manifests
are transferred unchanged. X86 Core's accepted S8 CPU repair is not replaced.
No App, external firmware/media, recovered implementation or sibling artifact
is imported. Root MIT owner authorization and existing source notices apply;
the existing provenance index records the import.

## Initial Actual-Diff Audit

Numbers use `git diff --no-index --numstat` between the receiving root and
the immutable archive before import. Renames are detected; documentation,
CMake and manifests are excluded only in the C/H columns.

| Root | All additions/deletions | C/H additions/deletions |
| --- | --- | --- |
| src/lib | +969/-698 | +839/-613 |
| src/emulator | +10/-7 | +7/-4 |
| src/x86-debug | +0/-0 | +0/-0 |
| src/x86-ibmpc | +16/-16 | +15/-15 |
| test/lib | +2263/-1236 | +2035/-1128 |
| test/emulator | +262/-174 | +218/-140 |
| test/x86-debug | +2/-1 | +0/-0 |
| test/x86-ibmpc | +13/-12 | +10/-10 |

The Lib change centralizes native keyboard decoding in Base and copied neutral
input definitions in Types. Console and KVM consume one decoder, while KVM
retains hotkey state and character-to-chord synthesis. Receiver adaptations
use actual new symbols; they do not add aliases or another input path.
Console input/output, executable directories, storage paths and Window titles
now use explicit UTF-8/native wide-character boundaries. Storage preserves open
status. Raw mouse registration serializes ownership without holding that lock
across focus/capture messages. PCM destroy now consistently discards pending
playback; explicit flush is required when the caller needs submission first.
MyNES shutdown is discard semantics, not audible-tail completion.

Emulator's small changes preserve complete UTF-8 scalars across output chunks,
adopt neutral input constants and use lowercase `window` in the status text.
X86 Debug production bytes are unchanged. X86 IBMPC's production difference
is input vocabulary only.

## Known Owner-Approved Upstream Follow-up

The owner requests this committed import first and a second import after
SoftPC fixes the following reviewed cleanup. They are not represented as fixed
by this packet: input-domain values belong in Base rather than Types; six
hotkey aliases duplicate neutral input values; Window modifier queries use
hotkey vocabulary; KVM delivery borrows native decoder result codes; KVM Base
retains an unnecessary native link; related documentation and Types-negative
coverage need alignment. No local canonical-tree divergence is introduced.

## Receiving Infrastructure And Ownership

Committed root `test/register.cmake` and its registration selftest support
source-relative target names and exactly one unit/integration classification.
Actual X86 Core/NXVM target references are updated mechanically; CPU code,
assertions and timing allocations are unchanged.

The upstream root test boundary verifier cannot replace the receiving one:
it lacks the two additional local packages and existing recursive/case-aware
CMake guards. Its useful production-DAG-backed unit-owner checks are merged
into the existing root verifier. Existing six-package, nested-CMake, foreign
fixture and outward-link selftests remain. The existing Types verifier and
its local directory exclusions remain; upstream does not require their removal.
These receiving changes are outside the eight canonical roots.

Actual receiving code is X86 NXVM's keyboard mapper and MyNES's Machine input,
hotkey registration, matching tests and opt-in diagnostic. All keep their
existing single KVM event path. No INI, snapshot, media, guest firmware or
deterministic NES algorithm is changed.

## MIDI Transfer And Qualification Boundary

The native MIDI backend and smoke body are unchanged from NXVM. The test moves
to `lib.integration/native_midi_output`, gains `native_audio` classification
and a native MIDI resource lock. This correctly removes native-session
availability from unit qualification. It does not repair S8's native
`midiOutOpen` return 1, establish audible output or waive that integration.
Current native results must be reported separately from the complete units.

## Verification And Delivery

Execution uses the existing x64/x86 Ninja/ccache graphs and persistent cache.
The required inventory is complete repository units, both specialized Core
qualification aggregates, canonical manifests/bytes, registered native shared
integration once per width, all four PC receiving profiles and MyNES integration,
and ten current published artifacts. Native desktop runs are serialized across
widths. No manual RDP or physical audio result is inferred.

The final receiving results are recorded below. This is bounded import
qualification, not a claim that every known upstream cleanup is repaired.

Initial receiving checks caught a stale `board_tests.cmake` manifest hash and
obsolete empty source directories left after file relocation; both were
corrected without changing incoming bytes. The empty directories had made the
new source-counterpart negative probe incorrectly succeed. The existing mutable
state inventory now classifies upstream's native raw-mouse registration lock
at its actual process-wide resource owner; no broad exemption is added.

One premature x86 CTest attempt encountered not-yet-linked renamed targets;
its Not Run results are not qualification. Final unit runs are explicitly
chained after successful complete builds. Two x64 links were temporarily unable
to replace existing output EXEs; resuming the same graph completed both without
source changes, renamed outputs or a fresh cache/build directory.

The optional MyNES controller diagnostic also retained four native decoder
type/result identifiers after the first neutral-key sweep. Those are migrated
mechanically to the same Base adapter contract. Both opt-in diagnostic targets
compile in both widths; the owner-ROM/RDP diagnostic is not executed. No
production or registered unit/integration source changed after qualification.

## Final Receiving Qualification

| Check | x64 | x86 |
| --- | --- | --- |
| Complete repository units | 655/655 | 655/655 |
| Imported four-package units (included above) | 121/121 | 121/121 |
| X86 Core units (included above) | 166/166 | 166/166 |
| Product integrations | 48/48 | 48/48 |
| Shared native integrations | 4/4 | 4/4 |
| Direct strict compilation | 498/498, zero deferred | 498/498, zero deferred |
| Complete specialized gates | Pass | Pass |
| Optional MyNES diagnostic compilation | Pass | Pass |
| Final published PC CLI/INI checks | 4/4 | 4/4 |
| Release artifact PE/deployment proof | 5/5 | 5/5 |

All reported XML executions have zero failures and zero skips.
Product integration selects Default's 35 cases (including twelve MyNES cases),
XT's three, AT's five and DeskPro's five. Shared native cases run once per
width, not once per profile. The DeskPro boot cases complete in 122.11 seconds
(x64 aggregate) and 169.38 seconds (x86 aggregate), within unchanged budgets.
The existing graph is restored to Default in both widths afterward. Component
libraries are reused across profile selections; no replacement build graph or
cache is created.

Both current native MIDI XML outputs explicitly report `default output opened
and released (no playback claim)`, not capability-unavailable success. This
resolves the transferred receiving test in this host session; the unchanged
backend does not establish a code repair for S8's native failure. Neither
physical audible output nor manual mobile RDP was tested.

[Results](t563-s9-results.json), [canonical inventory](t563-s9-canonical.json),
[actual per-root diff](t563-s9-diff.json) and [ten artifact hashes](t563-s9-artifacts.json)
retain compact reproducible receiving proof. Published artifacts are eight
PC 0563 EXEs and the MyNES 0044 pair. PC PEs are `8664`/`014C`, all ten have no
debug sections, and MyNES deployment hashes equal their linked outputs. Owner
INIs, firmware source, media and snapshots are unchanged.

Actual C/H change is production **+908/-679, net +229**, tests
**+2313/-1328, net +985**. It includes all additions/deletions and rename
detection against the frozen pre-import source/test archive, including removed
files whose diff destination is `/dev/null`; docs, manifests, CMake, tools and
artifacts are excluded. X86 Core source and tests remain **+0/-0**. Receiver
changes are symbol substitutions, not another production path. Positive
production growth comes from UTF-8/native boundary validation and native mouse
ownership; test growth adds owner-local/failure/UTF-8 proof. Root infrastructure
retains the previous guards and adds upstream owner/registration proof without
another dependency DAG.

## Actual-Change Review And Delivery Boundary

Executor review inspected the actual eight-root production changes, moved and
new tests, receiving symbol/CMake changes, root test-checker merge and release
outputs. The eight roots contain exactly 296 files and match the pinned archive
byte for byte. Source/test manifests, Types, production dependency and test
ownership gates pass in the complete suite. Both product documentation gates
and `git diff --check` pass.

Delivery separates Shared, MyNES and NXVM changes into their admitted target
commits. The coordinated cross-target result is accepted only after all parts
are pushed; no intermediate commit is described as the complete receiving
product tree. T563 remains open. The owner's second import will receive the
separately committed upstream input-ownership cleanup; this delivery does not
silently make those corrections or claim a completely clean upstream corpus.

Coordinator actual-commit review accepts Shared **3a07320e4** (181 paths),
MyNES **ffb9ce6a5** (eleven paths) and NXVM **55eb866ed** (twenty paths).
Review covers the actual source/test changes, canonical-tree inventory,
root-infrastructure merge, real receiver and optional diagnostic adaptations,
target isolation, committed release hashes and original owner request. A first
automated commit-scope rejection was resolved with read-only ownership proof:
X86 NXVM is the Shared package used by preceding Shared deliveries, and the
root CMake is shared configuration. No target was relabelled or approval bypassed.

Both original graphs are restored to Default. Current artifact hashes are
rechecked after the final diagnostic/gate builds; X86 Core source/test and
owner INIs/firmware/media/snapshots remain unchanged. S9 is accepted and closed;
the complete packet remains in P3. This acceptance changes no executable input
and does not close T563 or admit the second upstream import prematurely.
