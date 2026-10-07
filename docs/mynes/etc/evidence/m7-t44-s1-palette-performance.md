# M7 T44 S1 Palette Lookup: Baseline And Qualification

## Approved Boundary

Owner expanded the still-uncommitted S1 on 2026-10-06 from baseline research
to baseline plus palette optimization. This is not reuse of a closed S.
MyNES owns production/test/tool/document/artifact changes. Owner separately
approved exactly two root Shared build-preset target updates, 0043 to 0044.
No Shared production, NXVM, sibling, owner INI, ROM or snapshot was edited.

The original production baseline is fc0d31623. Pre-change probes compiled the
same benchmark source against its unchanged driver, before the production edit:
x64 SHA-256 A750D1B180036D1BBD3BD784D31B2A0923F71C2C47608C574424CE00F30F10B2;
x86 SHA-256 0751D9A79ED6EB298AB97C1C6C73D9060EBB902BC2BE4FB82F4834D59EA1A78E.
The ignored reference directory retains these binaries for immediate S2 comparison.

## Method And Limits

Both caches are MyNES-only Release with -O3 -DNDEBUG and static runtime linking.
x64 compiler is WinLibs GCC 16.1.0; x86 is MSYS2 GCC 16.2.0. Compare each width
with itself, not cross-width/compiler speed. Existing Lib monotonic counters
bound actual Core execution and frame conversion separately. No timer, worker
or profiler was added to production.

Each of six existing external ROMs uses graphics and text output, five fixed
diagnostic batches, 60 warmup and 120 measured completed guest frames.
Input is fixed: Start during frames 60-65, Right+B from frame 120, periodic A.
Each quantum uses the real Core API (256 instructions / 1024 cycles); PCM is
drained and hashed outside timed intervals, never sent to a physical endpoint.
Frame and serialized guest-state fingerprints are also outside timed intervals.
Each batch checks exact frame progression, no traps and no dropped PCM.

Early unrestricted scheduling produced nearly twofold variation in unchanged
Core cost. Those runs are diagnostic, not the accepted speed baseline.
The tool now temporarily pins itself before child creation so the leaf inherits
logical processor zero before executing, then restores the tool's affinity.
Both variants use that same processor; production scheduling remains unchanged.
Every leaf is waited, bounded to 60 seconds, killed on timeout and disposed.
The probe creates no worker/presenter/child, so process-tree ambiguity is absent.

Accepted raw CSVs are ignored under build/t44-s1-fixed-{before,after}-{x64,x86}.
Each has 60 rows. All paired ROM SHA, processor, guest cycles, instruction count,
frame, PCM and serialized-state fingerprints match, as do guest identities
between graphics and text. FNV-64 fingerprints are diagnostic checksums, not
cryptographic proof; the code-owned palette test compares every pixel and
all 256 palette entries directly.

This measures unpaced guest computation plus conversion, not displayed FPS,
native paint cost or separate CPU/PPU/APU percentages. Common publication and
native-stage timing remain unmeasured. Native integration proves function,
not performance. Existing 60 Hz pacing was not removed or counted as a gain.

## Fixed-Processor Results

Medians in milliseconds per completed frame; reduction refers to conversion,
not the complete emulator. Total includes Core plus conversion only.

| Width | Workload | Conversion before | Conversion after | Reduction | Total before | Total after |
| --- | --- | ---: | ---: | ---: | ---: | ---: |
| x64 | drmario | 0.186356 | 0.062887 | 66.3% | 4.043428 | 3.686858 |
| x64 | jackal | 0.202330 | 0.065694 | 67.5% | 3.123882 | 2.965949 |
| x64 | smb1 | 0.110859 | 0.063294 | 42.9% | 2.878009 | 2.874612 |
| x64 | smb2 | 0.315308 | 0.064458 | 79.6% | 3.672163 | 3.424774 |
| x64 | smb3 | 0.259722 | 0.063113 | 75.7% | 4.264478 | 3.892594 |
| x64 | tmnt3 | 0.128708 | 0.064938 | 49.5% | 4.403875 | 4.258920 |
| x86 | drmario | 0.211622 | 0.058568 | 72.3% | 4.334321 | 4.116848 |
| x86 | jackal | 0.223048 | 0.061548 | 72.4% | 3.536606 | 3.332698 |
| x86 | smb1 | 0.112701 | 0.063883 | 43.3% | 3.149530 | 3.227281 |
| x86 | smb2 | 0.389099 | 0.058939 | 84.9% | 4.358539 | 3.674972 |
| x86 | smb3 | 0.292763 | 0.061851 | 78.9% | 4.850928 | 4.336613 |
| x86 | tmnt3 | 0.138098 | 0.064844 | 53.0% | 4.829033 | 4.731852 |

Graphics conversion improves across every fixed game/width route, approximately
43-85%. Text conversion is unchanged by this implementation; measured text
medians remain approximately 0.17-0.22 ms/frame. Do not call their variation a gain.
All six x64 graphics total medians improve; five of six x86 totals improve.
SMB1 x86 initially changes 3.149530 to 3.227281 ms despite conversion improving.
A same-processor reversed-order check changes total 3.916488 to 3.072579 ms
and conversion 0.130978 to 0.057637 ms. The total slowdown is not reproducible;
unchanged Core timing still varies. Consequently no universal whole-emulator
percentage, significance claim or native-FPS increase is asserted.

Isolated sequential synthetic medians (before -> after, ms/frame):
x64 flat 0.081080 -> 0.071809, mixed64 1.035238 -> 0.063294,
overflow512 0.145300 -> 0.149972, equal-RGB aliases 0.087126 -> 0.061927;
x86 flat 0.093669 -> 0.050246, mixed64 1.731698 -> 0.053417,
overflow512 0.167375 -> 0.141332, aliases 0.093987 -> 0.056892.
Every synthetic fingerprint matches. The x64 overflow change is only 0.004672 ms
and the unchanged second-pass quantization remains its dominant path; no special
overflow optimization or parallel implementation was added.

## Sole Mechanism And Quality Review

driver.c retains one graphics conversion path. A local lib_u16[512] map uses
FFFF as unknown, masks the sample to its existing low-nine-bit RGB meaning,
and records the assigned index after the first RGB-deduplicated encounter.
Repeated samples become direct lookups. Different samples with equal RGB still
share their original raster-order palette index. Index 255 is not a sentinel.
The original >256-color cube palette and second pass remain unchanged.

The map costs 1024 bytes of function-local stack, no allocation, mutable global,
new public API, object field or cross-frame lifetime. It cannot require reset,
pause, mapper or snapshot invalidation. Guest pixels and timing are read-only.
The existing immutable RGB table was retained, not duplicated.

Production driver delta by git diff --numstat is +10/-1, net +9 lines.
Net deletion was a preference, not a reason to obfuscate the lookup or remove
required RGB deduplication/overflow behavior. New regression and diagnostic
code is counted separately in the final delivery review.

Counted source/test paths: driver.c, product/CMakeLists.txt,
integration/CMakeLists.txt, integration/performance_probe.c,
unit/core/CMakeLists.txt and unit/core/palette_smoke.c beneath their MyNES
roots. Staged git diff --numstat totals +336/-9, net +327, including the
226-line reusable diagnostic probe, 80-line exact-output regression and
build registration/version changes. The separate 101-line MyNES measurement
tool provides bounded process ownership, affinity, input identities and CSVs.
No diagnostic code is linked into the product executable. Documentation and
generated/artifact paths are excluded from these code counts.

Similar-issue sweep:
`rg -n 'palette_count|candidate.*palette|palette\\[candidate\\]|rgb_table' src/app-mynes`.
One production indexed-palette search owner exists. Its first-encounter search
remains necessary; the existing RGB table is already precomputed. Text's RGBI
nearest-color selection has a different averaged-input contract and is unchanged.
Mapper, IRQ and PPU candidate work is excluded from S1; no invented cache layer.

## Verification And Artifacts

- Strict -Wall -Wextra -Wpedantic -Werror builds for changed driver/probes.
- New code-owned palette unit: 512 single-color references and eight scenes,
  including alternating frames, reverse order, ignored high bits, RGB aliases,
  overflow and pseudo-random samples; exact pixels/palette and same-revision
  no-publication checks pass on both widths.
- Complete MyNES suites once each: x64 56/56, 49.09 seconds; x86 56/56,
  66.09 seconds. Each consists of 44 unit/static and 12 integration cases,
  including native Window/Console, input, reset/lifecycle and snapshots.
- External six-ROM matrix: 24 width/output routes, paired fixed batches above.
  Media identities below match before/after; no protected data is committed.
- Both approved root current-product presets resolve the 0044 targets and
  subsequent invocations report no work to do.
- Documentation governance and git diff --check pass. Actual-diff review
  confirms all Shared source/test corpora and other Apps unchanged.
- Product builds are optimized stripped Release. PE machine x64=8664,
  x86=014C; objdump section inspection finds no compiler .debug sections.
- 0044 x64: 249358 bytes,
  SHA-256 04F356B079A775454697A43FF36C7CA672F3C8A4232C51C2BD443AA0C50017D9.
- 0044 x86: 275982 bytes,
  SHA-256 B1A2E4663E690317F7AE6A3AC4C5F2092FFE77D51EFCA7DE1A5EBE66375D8C32.
- Owner INI SHA-256 remains
  198846D0D4EB3C7CAB1357992D7AB3441BECA676706AF1BAB0DAF507FB6BCC15.
  Superseded 0043 binaries are retired only after this paired qualification;
  they remain recoverable in Git history. Owner snapshot is untouched.

### Input Identity

- drmario: `C272ED5CF4EB70A5F84506E8F6C7124901F7585D3BA9FEB6118E08907DA172D1`
- jackal: `97E0E7D84CE6CCA3FD3EC5EABB3FA00252DE54F1354156CA2B95B95A39237373`
- smb1: `F61548FDF1670CFFEFCC4F0B7BDCDD9EABA0C226E3B74F8666071496988248DE`
- smb2: `9F73E5A91857FF58024769FB299BF8C2404DD9B7E4E0CB26CB7F346DF05409DA`
- smb3: `2DBFF658378216B3D4E59FDB38926D0BDDABD9E78D75E8819E3824D5554DAED8`
- tmnt3: `D6DC27E16AFD5D2AD151C0D593908AFF9E59C9F710D2C559D5F13C856350560E`

### Reproduction

Build mynes-performance-probe in each matching Release cache. Use
tools/mynes/Measure-Performance.ps1 with -BuildDirectory,
-AssetDirectory ../nxvm-assets/roms-mynes, an ignored build OutputDirectory
and -ProcessorIndex 0. For before, additionally pass the retained
build/t44-s1-reference/probe-{x64,x86}.exe as -ProbeExecutable.
Run each variant sequentially without builds/other probes competing.
For synthetic data invoke each probe with --conversion graphics.
For complete product tests use ctest --test-dir <matching-cache>
-R "^mynes\." --output-on-failure -j 1 --timeout 90.
Timing is not a pass/fail assertion in ordinary CTest.

## Executor Disposition

Palette batch is eligible for coordinator actual-diff review: bounded local
implementation, repeatable conversion benefit, complete output/correctness
proof and paired artifacts. Native performance and remaining Mapper work are
not qualified by S1; T44 stays open. No subsequent S is automatically admitted.
Warm caches, fixed CSVs and reference probes remain needed for immediate S2.
Excluded unrestricted diagnostic outputs may be removed after process exit.
