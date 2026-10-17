# T563 S10 — Reviewed Committed Shared Import

## Frozen Scope And Authority

Owner admission on 2026-10-17 continues from clean NXVM `81474a506`.
The immutable source is committed SoftPC
`1451522f42efc8152d4d4e75fbf97f80015e297b`. Export uses committed Git
objects; sibling files and later changes are not imported or modified.
The eight canonical roots are source/test pairs for Lib, Emulator, X86 Debug
and X86 IBMPC. Existing MIT authorization and notices remain; no App,
recovered-machine implementation, firmware/media or sibling binary is copied.

The initial reviewed eight-root estimate is 54 paths, +561/-530 including
manifests and the new/deleted header. The actual receiving delta is recorded
after implementation. Input-domain values move from Types to Base; normalized
header comparison proves identical values and layout. Neutral KVM submission
returns lib_bool; native adapter decoding retains its necessary three outcomes.
PCM gain is applied to input channels before native conversion, on the same
worker and submission path. No endpoint-wide volume is changed.

## Receiving Boundaries And Similar-Issue Sweep

Nine actual receiving C/H files adopt Base key names: X86 NXVM's mapper and
matching tests; MyNES driver, composition, media tests, three integrations and
its optional diagnostic. No compatibility aliases, extra input queue, state
mirror or execution path is introduced. X86 Core source/tests, App hardware,
NES algorithms, owner INIs, firmware, ROMs, media and snapshots remain unchanged.

The search covers all src/test C/H and relevant CMake/docs for old input names,
Types input includes and hotkey aliases. Retired-name strings in negative gates
remain intentional. DeskPro's unrelated kbc_self_test_input_values field is not
an obsolete include. Root test infrastructure is not overwritten: upstream
helpers have no new delta, and the receiving recursive/six-package guards stay.

The retained owners are Base decoder/vocabulary, KVM matcher/delivery and the
existing machine input adapter. The obsolete Types input header and six
hotkey aliases are removed by the unchanged transfer. Window modifier lookup
uses Base directly; the unnecessary KVM Base user32 link is removed.

## Execution And Qualification

Both original release-ccache Ninja graphs use the same persistent build/.ccache,
-j8 and their existing x64/x86 compilers. No fresh build graph or cache is used.
The whole unit suite runs after successful builds. Four PC profile integrations
reuse those same graph/library objects; MyNES integrations and shared native
integrations run once per width. Graphs return to Default after publication.
Required specialized/strict gates, manifest/canonical checks, documentation
checks and ten-artifact CLI/PE/hash verification are part of delivery.

Automated native integration is reported separately from repository-only units.
It does not prove manual desktop/RDP interaction or physical audible output.
The optional MyNES input diagnostic is compiled, not run with an owner ROM.

## Final Receiving Results

Both widths complete the same required receiving inventory:

- Whole repository units: 655/655, zero failures/skips in final complete runs;
  this includes 121 imported-package, 166 X86 Core and 45 MyNES units.
- Four PC profiles plus MyNES: 48/48 integration executions per width,
  Default 35 (including twelve MyNES cases), XT three, AT five and DeskPro five.
- Shared native integrations: 4/4 per width, zero failures/skips. MIDI explicitly
  opens/releases the default output; no physical playback claim is made.
- Both complete specialized aggregates and all 498 direct strict rows pass,
  with zero deferred rows. Optional MyNES diagnostics compile in both widths.
- Ten Release artifacts pass PE width/no-debug-section and deployment proof;
  eight real published PC CLI/INI cases pass. Current PC revision remains 0563,
  MyNES remains 0044. Owner configuration and protected external inputs are unchanged.
- Eight canonical roots contain 296 files and match the committed archive
  byte for byte. Source/test manifests, inward-dependency/Types/test-boundary
  checks, both product documentation gates and whitespace checks pass.

[Results](t563-s10-results.json), [canonical inventory](t563-s10-canonical.json),
[actual per-root diff](t563-s10-diff.json) and [ten-artifact hashes](t563-s10-artifacts.json)
retain compact proof. Initial/final CTest XML/logs and profile build logs remain
under ignored build/t563-s10-* for diagnosis. DeskPro boot groups complete in
116.78 seconds (x64) and 178.40 seconds (x86), within unchanged budgets.
The two existing graphs are restored to Default, with no replacement graph.

Persistent ccache counters before/after this work show 1,872 additional
cacheable calls: 1,400 hits and 472 misses; its pre-existing error counter stays
at nine. Cache hits do not replace actual linking or test execution. Per-profile
build logs show reused component libraries and only necessary profile/observer
recompilation and linking. No forced clean or artificial timestamp rebuild occurs.

## Actual Change And Review Method

The initial x64 whole-unit run was 654/655: X86 NXVM's dependency-negative
check exceeded its existing 30-second budget under concurrent qualification.
The unchanged single check passed in 18.13 seconds; the subsequent complete
655-case x64 run passed in 64.84 seconds. The initial XML/log and isolated
recheck remain under the S10 ignored build evidence. No assertion, timeout,
production or verifier implementation was changed to obtain the pass.

The first x64 35-case product integration group failed MyNES native Console's
existing Escape/pause assertion; the complete unchanged group then passed,
including all twelve MyNES cases. This repeats the documented MyNES native
qualification debt, not an established new import regression or proven host
cause. Initial failure XML/log are retained separately; MyNES TODO records
the recurrence. The final result does not claim that intermittency is repaired.

Counts use `git diff --numstat --no-renames` against the clean S9 baseline,
including the new Base header as an addition and removed Types header as a
deletion. C/H-only production is +299/-318, net -19; C/H-only tests are
+277/-246, net +31. Documentation, manifests, CMake and EXEs are excluded from
those code counts. The eight imported roots retain the reviewed +561/-530
total including their metadata. [Per-root diff](t563-s10-diff.json) separates
the canonical import from mechanical receivers.

All nine receiver files were compared against the baseline after only the
specified token substitutions; every comparison is equal. The input header's
guard/name-normalized comparison is also equal. The review inspects actual
PCM multiplication/division and buffer acceptance, native decoder versus
neutral delivery outcomes, removed aliases/link and owner-negative checks,
not only the diff statistics. PCM signed products fit lib_i32 over the full
signed-16 sample/unsigned-16 gain domain; native ReleaseBuffer success remains
the boundary that advances accepted frames. Gain tests cover mono/stereo,
zero/unity/half, signed extremes, native failures and partial accepted prefixes.
No production fallback, helper framework, worker or clock is added locally.

S10 awaits the owner's acceptance before closure. T563 remains open; no final
CPU or PC110 qualification claim is created by this receiving import.

## Target-Isolated Delivery And Actual-Change Review

Shared P1 **a936e554b** contains only the eight unchanged import roots and
X86 NXVM mapper/manifest receiving changes (58 paths). MyNES P2 **621b98d15**
contains its seven mechanical receiver files, four relevant documentation
records and two rebuilt releases (13 paths). Both commits are pushed.
The original owner request, active packet, actual source/test deltas, removed
Types header, new Base header, accepted PCM path and receiving artifacts were
reviewed in coordinator role against the recorded proof. This is a one-session
actual-change review, not an independent second reviewer.

NXVM P3 carries only the eight rebuilt PC EXEs, NXVM task/provenance records
and complete evidence. No executable source changes after the qualified
Shared/MyNES commits. Its delivery does not record owner acceptance or close
S10/T563. All ten artifacts are rechecked against the committed inventory;
the eight canonical packages retain their pinned bytes, and owner inputs and
X86 Core source/tests remain unchanged against the S9 baseline.

On 2026-10-17 the owner accepts this delivery. Coordinator actual-commit
review confirms all three pushed target scopes, unchanged canonical bytes,
true final XML and ten committed hashes. S10 is accepted and closed; the
[whole-T audit](t563-final-closure.md) separately confirms T563 completion.
The complete active packet remains in P3. No executable input changes at
acceptance, so the current ten releases are retained without rebuilding.
