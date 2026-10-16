# T562 S3 Consolidated Acceptance Correction

## Baseline And Boundaries

The owner rejected the initial S3 delivery and approved this correction.
NXVM baseline is 376a960f0. The selective upstream reference is committed
SoftPC 7a523a42; its shared trees were clean during review (only task documents
were uncommitted). No sibling file is modified. Shared lineage and existing
MIT notices remain unchanged; no independently licensed unit is introduced.

The imported mechanism is executor heartbeat gating, not a MIDI-specific
service in Emulator. SoftPC's driver/Compat uses it for DSP and host services;
MyNES uses it for PCM and pacing. The NXVM PC driver's existing no-op remains
valid. Emulator never learns about DSP, PCM endpoint state or guest hardware.

## Requirement And Same-Issue Disposition

- Emulator Machine disables the existing gate before parking and before
  PAUSED/RESET_COMPLETED notification; enables it before actual continuation.
  Private state capture may continue without a public RUNNING notification.
  Stop/cancellation from parking never re-enables the gate. Existing cold-run
  entry and unwind disable remain the same owner; repeated disable is safe.
- The upstream `test/emulator/machine_wait_smoke.c` heartbeat coverage is
  imported with the existing NXVM request-failure/publication regressions
  intact. An additional assertion directly checks no enable after parked
  stop/reset/shutdown. Snapshot continuation checks both gate values and
  absence of extra public RUNNING/PAUSED facts; Debug-stop uses the same path.
- MyNES's existing pacing test exercises the real driver: repeated disable
  discards staged PCM and enable rebases pacing to current guest cycles. This
  tests adapter state, not physical sound. Receiving integrations exercise
  the real lifecycle/snapshot/debug path separately.
- Raw NULL production tokens in the five outer Shared packages now use
  LIB_NULL. Their existing source gates reject reintroduction and negative
  tests inject it. A repository-wide token search leaves only native Lib
  Types' definition and prose outside these changed packages. Assembler
  comments describing an absent SIB base/index are not C pointer usage and
  remain unchanged; instruction algorithms/tables are untouched.
- Debug's target-wide format-truncation warning downgrade is removed.
  Both optimized compiler builds enforce the existing strict warning policy;
  no replacement target/file suppression is added.
- X86 Debug, IBM PC and NXVM source gates scan nested CMakeLists and included
  .cmake files, recognize mixed-case command names, and reject global or
  property link declarations. Target names retain their case. Emulator gains
  included .cmake coverage and newline-separated property checks; X86 Core
  gains global/property rejection and no longer skips arbitrary verify_* files.
  Each gate inspects only its own root, not outer product code.
- The NXVM chip factory's exact existing contract remains checked. Its
  product/build.cmake has two existing caller-owned App composition link
  statements; only those exact statements are excluded from package-library
  edge interpretation, not the whole build file. All other declarations are
  checked. The verifier itself is excluded to avoid parsing regex literals
  as build instructions. These remain bounded static policy checks, not a
  complete interpreter for arbitrary CMake metaprogramming.
- PC Entry injects independent title/status failures and checks disposal
  exactly once. Failed title prevents status formatting; status failure after
  title success is exercised separately. The real long-name truncation case
  remains and now also checks ordering/disposal. No production Entry change.
- Lib directory tests add exact-fit and one-byte-short capacities for every
  existing path/root case, checking untouched output on failure. No duplicate
  directory-test framework or Lib production change.

The sweep inspected all set_heartbeat callers, executor parking/continuation
and cleanup, outer Shared NULL tokens, the Debug compile policy, each changed
package's CMake declarations and negative tests, PC Entry formatting/disposal,
and directory extraction capacities. The scoped findings are repaired.
NXVM's PCM cancellation/completion-failure tests, S3 Debug finalizer, removed
composition wrapper and shared-* project names are preserved; upstream files
were not blindly overwritten. No new public function/type, persistent state,
thread, fallback or runtime path is introduced.

## Estimated And Actual Delta

Actual counts use `git diff --numstat 376a960f0 -- src test`, excluding
MANIFEST.sha256, documentation and artifacts. Source includes build/gate
scripts; tests include their owner-local negative scripts.

| Package | Estimated added/deleted/net | Actual added/deleted/net |
| --- | --- | --- |
| Lib | 10/2/+8 | 5/0/+5 |
| Emulator | 150/100/+50 | 260/183/+77 |
| X86 Debug | 50/30/+20 | 69/22/+47 |
| X86 IBM PC | 70/25/+45 | 84/24/+60 |
| X86 Core | 40/15/+25 | 14/2/+12 |
| X86 NXVM | 40/15/+25 | 96/36/+60 |
| Shared total | 360/187/+173 | 528/267/+261 |

The larger Emulator churn is mechanical NULL replacement; the larger NXVM
gate diff preserves and moves its exact chip-factory check into recursive
validation. Production C/H alone is +193/-187, net +6: the runtime gate and
its contract comment, with otherwise mechanical vocabulary changes.
MyNES adds 16 test lines, no production changes. Most net growth is explicit
regression/gate coverage, not additional runtime abstraction.

## Verification

Shared correction is committed and pushed as `86175d302` (P4). The
coordinator reviewed its actual commit against the approved correction scope.
MyNES receiving test, rebuilt pair and failure ledger are `66d02ba64` (P5).
NXVM P6 contains eight rebuilt PC EXEs, this evidence and updated task records.

The final frozen x64 and x86 builds each pass all 631 repository units:
628 non-desktop and 3 automated desktop-labelled tests per width. An earlier
x64 run encountered a manifest mismatch after a late test assertion; that
run is not counted as qualification. The final manifest refresh and complete
rerun pass. Negative fixtures reuse one copied corpus per matrix instead of
recopying the whole package for every link spelling.

Both widths complete 15 selected PC profile/restored-identity integration
executions, four published PC CLI checks and 12 MyNES integrations. All ten
EXEs are rebuilt; PE width, release version and SHA-256 are recorded in the
[artifact inventory](t562-s3-correction-artifacts.json). INI and external
media/ROM/snapshot inputs are unchanged. Original Ninja/ccache graphs are reused.
Both widths pass the final specialized gates, including all 497 direct strict
compilation rows (zero deferred). Both product documentation checks and
git diff --check pass; manifests are refreshed and checked by their owners.

The first x86 MyNES suite had one native-console Escape/pause assertion failure
after 10 seconds; its other 11 tests passed. With unchanged code, the isolated
test passed in 0.62 seconds and a complete 12-test rerun passed in 2.09 seconds.
The failure is retained, not reclassified as success or claimed repaired.
Its cause remains unproven and is recorded in the MyNES review ledger. x64
passed all 12 initially. Each width includes two automated desktop integrations;
the remaining ten MyNES integrations and PC CLI checks are background checks.

Final actual-diff self-review covered runtime ordering, all changed gate
expressions and their negative probes, preserved failure tests, scoped paths,
manifest changes and artifact identity. It found no additional in-scope
production defect; this is not an independent reviewer or whole-tree proof.
Logs remain under build/t562-s3-correction-* (including the failed first suite
and the explicit x86 MyNES rerun). S3 awaits owner acceptance; T562 remains
open. No manual RDP, physical MIDI/audio, Linux or full external diagnostic-ROM
qualification is claimed.
