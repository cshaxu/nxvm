# T562 S3 Equivalent-Path Consolidation

## Authority And Frozen Batch

Owner approved five consolidations from the preceding read-only review and
required estimated versus actual package deltas. Baseline is clean
`59aea8649`. This is a derived NXVM Shared repair, not an upstream import.
Sibling repositories, owner INI/media/ROM/snapshots and X86 Core stay unchanged.

| Row | Retained owner/path | Removed duplication | Direct regression |
| --- | --- | --- | --- |
| FDD | Board-bound media provider read/write/format | Test-only CHS byte and sector operations/declarations | Provider, direct/readonly, close-failure and real guest FDC read-track tests; retired-symbol negative gate |
| Debug failure | X86 Debug output finalization and valid failure result | PC cancellation, failure text and separate submit/observe cleanup | Allocation failure at submit/observe/partial output, cancellation request, clean continuation, next command recovery; PC adapter injected result tests |
| Unsupported snapshot | Emulator monitor fallback | PC default snapshot callback | Existing PC save/load rejection and Emulator fallback tests |
| Fixed commands | One private immutable descriptor table | Separate lookup/help/group/argument classifications | Ten-command argument matrix, lifecycle and exact help/extension tests |
| Cursor shape | KVM Base scanline resolver | Window/Console duplicate default/clipping/full-cell calculations | Default/32/512-line, clipped/hidden/inverted/off-surface and source-phase-independent cases; leaf geometry/native Console adaptation |

Debug output failures retain their original status and replace partial output
with nonallocating text and a ready `-` prompt. Cancellation remains an
existing asynchronous Machine request, not a synchronous completion claim.
Invalid API arguments still leave the result untouched; the PC adapter does
not consume that invalid result. Ordinary grammar and Machine-access messages
remain distinct existing Debug behavior. No original instruction table is
rewritten. The validated input size is used directly on both continuation paths.

The cursor helper returns scanlines only. Window owns pixel scaling; Console
owns native percentage conversion and host blink. Full cells use the existing
Console inverted-range encoding, including logical fonts exceeding byte-sized
cursor coordinates. Console broker's independent native contract still checks
visibility and converts shape to percentage; it must not depend upward on KVM.
No phase, raster, rendering or machine-state policy moves into the helper.

## Estimates And Actual Delta

Counts are added/deleted/net lines. Actual counts use `git diff --numstat`
against the baseline, plus the new test file; documentation, manifests and
binary artifacts are excluded. Source/test CMake and verifier changes are
included, so the NXVM source column includes three gate lines.
The [path inventory](t562-s3-code-delta.tsv) records every counted file.

| Package | Estimated source | Actual source | Estimated tests | Actual tests |
| --- | --- | --- | --- | --- |
| Lib | +35/-25/+10 | +34/-25/+9 | +40/-5/+35 | +29/-4/+25 |
| Emulator | +65/-75/-10 | +65/-71/-6 | +35/-5/+30 | +19/-0/+19 |
| X86 Debug | +25/-15/+10 | +14/-7/+7 | +60/-5/+55 | +29/-1/+28 |
| X86 Core | 0 | 0 | 0 | 0 |
| X86 IBM PC | +20/-35/-15 | +18/-42/-24 | +55/-5/+50 | +54/-0/+54 |
| X86 NXVM | +0/-65/-65 | +3/-67/-64 | +25/-15/+10 | +12/-6/+6 |
| Total Shared | +145/-215/-70 | +134/-212/-78 | +215/-35/+180 | +143/-11/+132 |

Production C/H alone is **+131/-212, net -81**. Shared source/tests including
their build/gates total **+277/-223, net +54**; the net increase is regression
coverage, not new production state. The sole external caller change is the
NXVM guest read-track test, **+2/-3, net -1**. No App production code changes.
The only added public function is the approved pure KVM cursor resolver;
obsolete FDD functions are removed, and Debug's existing result contract is
documented. No new persistent state, manager, queue, thread or compatibility
route is introduced. The fixed-command enum/record is private static metadata.

## Similar-Issue Sweep And Actual-Diff Review

- Search all source/test C/H for
  `x86_nxvm_machine_fdd_(chs_valid|read_byte|write_byte|format_sector|byte_offset)`:
  no remaining caller or implementation. The existing provider owns access;
  the verifier and negative probe prevent the deleted API from returning.
- Search PC Product for `snapshot_not_supported`, `Feature not implemented`
  and `debug_cancel`: no remaining duplicated default/failure owner.
  Real injected snapshot implementations remain optional and are not collapsed.
- Inspect both Debug public entry points, post-begin returns and output finalizer:
  output failures converge; precondition failures do not expose an invalid result.
  Access diagnostics remain command semantics, not duplicate output-failure paths.
- Inspect monitor lookup, help and admission: the descriptor owns fixed metadata;
  explicit lifecycle execution remains separate. Extensions cannot override it.
- Search Lib for cursor top/bottom comparisons: KVM owns shared scanline shape;
  broker owns native Console percentage conversion under its separate contract.
  Window rectangle scaling/damage and Console lifetime remain necessary leaf work.
- Review changed tests and real provider/Session wiring, not only helper output.
  All twelve manifests use the existing sorted SHA256/LF generator; unchanged
  X86 Core manifests remain byte-identical.

First build exposed an optimizer warning caused by a constant-failure test
stub. The injection is now runtime-readable, as a real failing provider would
be; production instruction handlers and compiler warning policy are unchanged.

## Verification And Delivery

The original x64/x86 Ninja graphs and persistent ccache are retained. Both
graphs are restored to the default profile after all four receiving profiles.

- Complete repository units: **631/631 per width**, zero failures or skips.
  Each width includes 628 non-desktop-labelled units and three automated
  desktop-labelled units. These are not manual desktop acceptance.
- Profile boot/INI/Console integration: 14 executions per width across the
  four profiles, plus the restored-default INI identity check, all pass.
- Direct CLI checks of the final published eight PC EXEs: all pass.
- MyNES integration: **12/12 per width**, including two automated desktop
  Window/Console tests and ten non-desktop-labelled tests per width.
- Product targets rebuild/deploy all ten EXEs; the [inventory](t562-s3-artifacts.json)
  records PE architecture, retained 0562/0044 version, size and SHA256.
  Only two EXEs remain in each product directory; no owner input changed.

The logs remain under ignored `build/t562-s3-*`: complete unit XML, profile
integration, final published CLI, MyNES integration and specialized checks.
Both-width specialized gates pass, including dependency/Types/manifest checks
and the direct strict-compilation matrix (497 rows, zero deferred). NXVM and
MyNES documentation governance and `git diff --check` pass.
No manual mobile RDP, physical audio/MIDI, Linux or fresh full external
diagnostic-ROM qualification is claimed. Building additional integration
executables as gate prerequisites is not counted as executing their tests.

Shared P1 `c81762c5f` contains the complete package change; MyNES P2
`96b066002` publishes its verified pair. NXVM P3 carries the real provider
test caller, eight PC EXEs, packet and this evidence. Production inputs are
the Shared P1 source plus unchanged App source; the artifact hashes identify
the builds independently of the retained version strings.

One-session role-switched actual-diff review covered all added/changed/deleted
source, tests, manifests, receiver artifacts and documentation, including
the new failure-adapter test. It is not independent reviewer evidence.
S3 awaits owner acceptance; T562 stays open. This bounded repair is not a
declaration that all shared code is defect-free.

Owner acceptance was subsequently rejected. The approved
[consolidated correction](t562-s3-correction.md) supersedes the delivery
qualification above for current source/artifacts; it preserves these five
implemented consolidations and does not close T562.
