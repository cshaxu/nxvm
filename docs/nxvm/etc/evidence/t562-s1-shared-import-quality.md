# T562 S1 Shared Import And Quality

## Baseline And Scope

NXVM starts clean at 80c572112. Git archive pins SoftPC
44cad54822316ca57fb66f937607db29b80bf3f1. Eight source/test roots for Lib,
Emulator, X86 Debug and X86 IBM PC were copied byte for byte and checked by
SHA-256 before deriving repairs. The other six imported trees were already
identical; upstream Lib adds PCM relocation/progress/gain, MIDI and explicitly
unsupported FM. Uncommitted Emulator heartbeat and App work are excluded.

## Mechanism Review

- Monitor formatting checks both negative results and required length at
  least capacity; tests cover each title independently and an empty-hotkey
  status page, exact fit/one-short/one byte. Entry stubs honor the same rule;
  an oversized identity fails UI configuration without retaining a live fixture.
- Base alone extracts directories. Ordinary final separators are removed;
  POSIX, drive and UNC share roots retain their separator. The existing
  private platform seam proves nested/root paths, capacity and platform
  failure with unchanged output. X86 NXVM/MyNES join the filename once and
  test root and nested cases. SoftPC's App config caller also needs that
  root-aware join when adopting; the sibling is not edited here.
- PCM retains its existing FIFO, failure status, worker and cancellation
  event. All control/space waiters also observe worker cancellation. A failed
  notification records terminal failure and cancels that task, rather than
  silently waiting on an undelivered event. One private checked-signal helper
  owns that failure boundary; it is not a second operation or recovery loop.
  Tests inject persistent completion failure for clear/gain/flush, wake
  failure and cancel-wait signal failure. Flush proves accepted frames are
  not repeated. Existing attach/partial-prefix/native/notify failures remain.
- Console guard/comment and Linux decoder prefix follow their real owners;
  the Lib-local dependency gate rejects retired names with negative probes.
- Six source/test project pairs use shared-<package>/shared-test-<package>.
  Internal target and CTest names are preserved. No compatibility aliases.
- MyNES changes its sole audio receiver to PCM, retaining clear/active/query/
  enqueue/shutdown semantics and its 0044 artifact identity. No NES or
  snapshot algorithm changes.

## Similar-Issue Dispositions

The Td S180 formatter inventory remains the starting sweep: dynamic Debug
output measures required length, Xasm32 bounds its output, fixed short and
same-capacity prompt copies are retained for their proven capacity. Only the
three production formatter calls and their PC fixture copies need correction.
`base_process_executable_directory` has one implementation and three App
receivers across the two repositories; both local receivers are repaired.

PCM startup already observes cancellation. Control reset/native-cancel
failure occurs before publication and returns error without fabricating
completion. Enqueue preserves accepted frames on wake failure. Destruction
already checks cancellation/join and retains ownership on failure; its extra
wake is not the mandatory completion route. Native cancellation failure is
not repaired by retry or freeing a live backend. There is no new state/API.

## Qualification

Both original Ninja/ccache graphs pass the complete repository unit suite:
630/630 on x64 and 630/630 on x86. The x64 final PCM/naming delta separately
passes four affected tests; final extended-UNC cases pass the rebuilt process
test and manifest check on both widths. Both final specialized gate aggregates,
all twelve manifest hash sets, documentation structure checks for NXVM/MyNES
and staged/working diff whitespace checks pass.
Twenty-four standalone source/test configurations (six package pairs, both
widths) assert their exact CMAKE_PROJECT_NAME; this is configure evidence,
not a claim of separately rebuilding all twenty-four graphs.

Both widths pass 48 profile integration executions: default (35, including
12 MyNES), XT (3), AT (5), DeskPro (5). Restored-default identity and each of
the four actual published PC executables' INI/CLI checks also pass per width.
All ten rebuilt EXEs have checked PE width, release identity and SHA-256 in
[the artifact inventory](t562-s1-artifacts.json). Eight PC 0562 files replace
the previous 0561 pairs; MyNES retains its two 0044 names. INI, media, ROM and
snapshot files are unchanged. Original build graphs finish on default.

Raw logs are `build/t562-s1-<width>-units.log`,
`build/t562-s1-<width>-<profile>-integration.log` and the final specialized
gate/deployed-CLI logs. Automated desktop-labelled tests are distinguished
from manual physical audio/MIDI and mobile-RDP verification, which were not
performed. No full Linux runtime qualification is claimed.

## Actual Change Review

Against admission 80c572112, `git diff --numstat --no-renames` over changed
src/test code and CMake/checker files (excluding Markdown and manifests)
counts source 37 paths +1687/-899, net +788; test 22 paths +1476/-577,
net +899. Counting moves as deletion/addition makes the method reproducible.
The positive size comes primarily from imported PCM progress/gain and MIDI
transport plus their failure tests, not another local audio pipeline.

Against the pinned SoftPC export, the eight trees' local code/test changes
are only +288/-90, net +198: production/build/checkers +122/-81 and tests
+166/-9. Their 265-file inventory has 232 byte-identical and 33 derived
files, including README/manifests. Final trees intentionally differ by these
approved repairs and names; they are not advertised as unchanged upstream.

Review checked the actual import and derived changes: the old PCM paths and
target are removed, MyNES has one PCM receiver, native API vocabulary stays
inside Types/platform implementations, and nested audio owners retain the
same dependency/private-interface gates. Source/test project identities do
not rename unrelated link targets. The one new PCM checked-signal helper
owns real failure propagation; no local public API, persistent state,
worker, compatibility wrapper or polling loop is added. The imported public
PCM/MIDI/FM contracts are explicitly part of the admitted upstream change.

Root-path tests include extended-length Windows UNC and drive roots, not
only ordinary paths. Audio failure tests use injected notifications, not
physical audible output. FM remains explicitly unsupported, not a pretend
synthesizer. Native MIDI capability tests are not listening/playback proof.

The first final artifact gate correctly rejected old/new release pairs
coexisting during deployment. The qualification script now runs that gate
after both widths replace the old pairs; no production or gate weakening
was needed. Superseded binaries remain recoverable in Git history.

## Acceptance Mapping

Import is proven by the pinned Git archive and per-file hashes; formatting,
directory extraction/join and PCM failure have direct real-owner regression
fixtures; Console/Linux naming is enforced by the Lib-local negative gate;
all standalone identities are asserted from generated CMake caches; actual
receiving Apps are exercised through full units, integrations and published
EXEs. These findings are fully handled, not deferred to a later CPU task.
The existing 486SX C16/C17/final qualification stays in its separate queued
proposal and is not implied by this import's tests.
