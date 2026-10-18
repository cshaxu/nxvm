# T564 S1 — Committed Shared Frame Import

## Authority And Baseline

The owner admitted the complete import and ten receiving releases after T563
closure. The [proposal](../../proposals/m5-softpc-rgb-frame-import.md) freezes
the finite receiving universe; [Current](../../states/CURRENT.md) owns task
state and owner acceptance. The S1 acceptance addendum below records closure;
this report does not close T564.

The clean receiving baseline was `71be4639d`. The read-only SoftPC source is
committed `dfc85aac4387600033d3ed11d7fb26e74b9eb631`, exported through Git
objects, not copied from its mutable working tree. Eight canonical roots are
the source/test pairs for Lib, Emulator, X86 Debug and X86 IBM PC. All 297 files,
including manifests and notices, must match that export byte for byte. X86 Core,
MyRaidenII, owner INI, firmware inputs, media and snapshots are not changed.

During final review SoftPC HEAD advanced to
`5e76dd4949610d71457ccc65672b1668f6138893` (owner-accepted governance closure).
Git comparison from the frozen commit reports no changes in any of the eight
roots, and those roots have no uncommitted changes. Thus the pinned imported
bytes also match that latest committed SoftPC baseline; no second batch is
silently mixed into qualification.

## Single Receiving Path

- Lib owns native presentation and the imported four-slot frame exchange.
  Emulator Machine owns each exchange; Session/UI pass publication metadata,
  not additional complete frame copies. Window graphics are numeric RGB32;
  Console still accepts text only.
- The PC driver converts its existing guest snapshot directly into the granted
  writable slot. A source snapshot remains necessary between bounded guest
  execution and the driver callback; it is not a second host frame cache.
  One private validator is shared by source readiness and conversion. Invalid
  input is rejected before mutating the destination or consuming pending output.
- The NES driver resolves its unchanged immutable palette directly into RGB.
  The former per-frame indexed palette search and colour-cube fallback are
  deleted. Existing ASCII conversion stays in the NES driver; its existing
  sixteen colours also populate the complete text/Console representation.
- Explicit diagnostic copies remain outside transport. PC bound observers copy
  only published frames; unbound component observers use the guest snapshot.
  The legacy indexed diagnostic view reconstructs a lossless palette from RGB
  and rejects overflow rather than quantizing. No production compatibility
  schema, renderer, state machine, thread or queue is added.

The retired Emulator frame envelope and obsolete imported tests are deleted,
not retained behind adapters. The only additional local declaration is a
private PC source validator. Direct Lib frame-value dependencies are declared
in the PC conversion target and its dependency gate; operation headers remain
forbidden, with two explicit negative probes. There is no new local public
capability beyond the imported frame contract.

## Actual-Change Review

The [counted diff](t564-s1-diff.json) counts changed C/H/CMake inputs and root
presets, including new files; it excludes docs, manifests and EXEs. The
[canonical inventory](t564-s1-canonical.json) separately proves the exact import.
The broad import's positive line count mainly belongs to the reusable exchange
and ownership/failure regressions. The NES receiver removes more production
lines than it adds. CPU, guest hardware algorithms and firmware are unchanged.

| Counted owner | Added | Deleted | Net |
| --- | ---: | ---: | ---: |
| Lib source | 430 | 192 | +238 |
| Emulator source | 157 | 163 | -6 |
| X86 NXVM source | 79 | 60 | +19 |
| MyNES source | 36 | 67 | -31 |
| Lib tests | 1,127 | 599 | +528 |
| Emulator tests | 308 | 295 | +13 |
| X86 Debug tests | 5 | 2 | +3 |
| X86 IBM PC tests | 10 | 24 | -14 |
| X86 NXVM tests | 137 | 107 | +30 |
| Default NXVM tests | 107 | 92 | +15 |
| DeskPro tests | 7 | 7 | 0 |
| MyNES tests/diagnostics | 118 | 122 | -4 |
| Root presets, PC build and metadata gate | 14 | 6 | +8 |
| Total | 2,535 | 1,736 | +799 |

The unchanged X86 Debug/X86 IBM PC production roots contribute zero source
lines; they are nevertheless part of the byte-exact imported universe.

Review includes all converter callers, snapshot readiness, invalid-output
preservation, typed-frame self-pointers, RGB assertions, text mapping, native
integration observers, opt-in diagnostic compilation, declared dependencies,
retired symbol searches, manifests and whitespace. Large test frame fixtures
use static or heap storage rather than overflowing the native thread stack.
Self-owned frame storage is not shallow-copied as another usable frame.

## Initial Failures And Corrections

Initial builds caught an unused test variable, an obsolete driver argument list
and one retired include. They were corrected in receiving tests; strict warning
policy was not weakened. The first complete x64 unit run passed 663/666:
the new direct frame dependencies were absent from the local corpus declaration,
the receiving test manifest was stale, and a migrated RGB test used the wrong
colour assumption. All three were corrected.

The runner cadence fixture programs a nonstandard palette: guest index 15 is
`0xAAAAFF`, not standard white. The regression now checks both the original
guest index and its exact RGB publication at a confirmed paused boundary;
production palette and cadence logic were not altered to satisfy the test.

An intermediate concurrent run timed out the existing 30-second PC dependency
negative test. An unchanged isolated run passed in 25.68 seconds. The complete
final x64 unit run passed 666/666, including that test in 26.04 seconds. No
timeout or assertion was relaxed. The early specialized artifact gate correctly
rejected the transitional old/new release layout; final qualification must use
the fully replaced release pairs rather than that incomplete layout.

The first x86 run likewise passed 665/666, with only that concurrent timeout.
The receiving negative test now has CTest `RUN_SERIAL` scheduling: its complete
whole-corpus probes run without other CTest filesystem/compiler probes competing
for the same 30-second deadline. Its timeout and every rejection assertion are
unchanged. The scheduled x86 rerun passed the probe in 24.00 seconds. This local
receiving test-registration change is outside the eight canonical import roots.

Full specialized qualification exposed a pre-existing lexical mismatch in
`cmake/nxvm/verify_legacy_profile_metadata.cmake`: the already accepted V30
work replaced enum ordering with `cpu_profile_has_32bit_architecture`, while
this repository-level gate still searched for the former comparison. The gate
now scopes its LOCK assertions to the actual `PREFIX_LOCK` owner and checks
that named branch. Two isolated source mutations prove that a wrong 32-bit
branch and a missing 286 privilege condition are both rejected even though
other CPU functions remain intact. This is +9/-1 in the receiving checker;
no CPU implementation, profile metadata, architecture semantics or shared
import bytes were changed. The final specialized targets pass on both widths.

## Qualification And Delivery

[Dual-width results](t564-s1-results.json) and
[ten release hashes](t564-s1-artifacts.json) record the complete receiving run.

| Proof | x64 | x86 |
| --- | ---: | ---: |
| Whole-repository units | 666/666 | 666/666 |
| Default PC plus MyNES product integrations | 35/35 | 35/35 |
| My5160 integrations | 3/3 | 3/3 |
| My5170 integrations | 5/5 | 5/5 |
| DeskPro386 integrations | 5/5 | 5/5 |
| Shared native integrations | 4/4 | 4/4 |
| Specialized target | pass | pass |
| Direct strict-compilation matrix | 498/498 | 498/498 |

Whole units include Lib 74, Emulator 23, X86 Debug 12, X86 IBM PC 10, X86 Core
179, X86 NXVM 229, the five Apps' 117 cases and 22 repository qualification
cases. MyNES contributes 45 units and
12 of the default group integrations per width. A final x64 focused run also
passes 46/46 (MyNES units and the updated receiving test manifest) after the
last formatting/registration cleanup. Existing warning exceptions remain
exceptions; this does not claim all legacy source is waiver-free.

Both original Release Ninja graphs use the same persistent ccache and `-j8`;
profile switches reuse their compiled shared components rather than creating
four new graphs. MyNES's optional performance/controller diagnostics compile
on both widths, but owner-ROM/controller diagnostic execution is not claimed.
The selected DeskPro boot reaches its original checkpoint in 149.22 seconds
on x64 and 180.85 seconds on x86, within the unchanged 300-second budget.

The shared native group actually exercises three desktop/Console/Window cases
and native MIDI open/release, not just mocks. MyNES native Window and Console
integrations also pass in each 12-case group. These executions are distinct
from manual RDP scaling/input, physical audible output, mobile key capture and
whole guest GUI qualification, which were not performed. Prior MyNES RDP and
native Console intermittency TODOs are not declared repaired by this import.

The eight PC releases are 0.5.0564; MyNES remains 0.0.0044. All ten stripped
PEs have the correct architecture/version, eight deployed PC CLIs pass their
real executable/adjacent-INI identity test, and the MyNES deployment hashes
match the linked outputs. Eight superseded PC 0563 EXEs were removed only
after validating all replacements; they remain recoverable from Git. Exactly
ten current EXEs remain across the five existing artifact directories. Owner
INIs, media, snapshots, external firmware and all MyRaidenII paths have no diff.

Both product documentation governance checks and `git diff --check` pass.
One target per P delivers Shared, MyNES, then NXVM. Coordinator actual-change
review covers the complete three-target result, not merely the import hash or
test counts. At P3 delivery S1 and T564 remained open for owner verification;
no assertion that the entire tree is defect-free or another task is admitted
is made.

Shared `d30b69b2a` and MyNES `906e61b0c` were individually pushed before the
NXVM P3 carrying this evidence and the eight PC EXEs. The source baseline for
all ten hashes is the complete locally reviewed S1 change set, not the
temporarily incomplete intermediate cross-target commit. P3 completes that
set; no unqualified intermediate commit is advertised as a runnable release.

## Owner Acceptance And S1 Closure

On 2026-10-17 the owner stated that verification was complete and requested S1
closure. The coordinator reviewed the actual delivery commits `d30b69b2a`,
`906e61b0c` and `b9433b31d`, mapped the original finite scope to the proof
above, and rechecked all ten deployed hashes, 297 canonical files and both
widths' result records. The complete pre-closure packet remains in P3.
All S1 exit criteria are accepted; S1 is closed. T564 stays open, no active
S packet remains, and no next task is admitted. Governance-only acceptance
changes no executable input, so it reuses the qualified ten releases rather
than recompiling them. Prior MyNES TODOs and unperformed manual verification
are unchanged. This closure does not promote automated native tests into
physical audible/RDP proof or claim the entire tree has no defects.
