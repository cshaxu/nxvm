# T582 S2: equivalent F65535 planar expansion

## Implementation and review

The only algorithm change reads each of the four source plane bytes once per
8-pixel group, then expands their bits. The existing validation, full snapshot
initialization, palette, generation flags, start address and stride remain
unchanged. No cache, public interface, state or timing change is introduced.
The old per-pixel division/modulo and repeated plane fetch loop is removed.

Production loop: +15/-10 lines. Existing chip regression: +27/-0 lines.
The regression uses nonzero start, padded pitch, three rows, distinct plane
patterns and all eight bit positions. Its flat reference computation preserves
the independent old indexing formula. Existing palette, repeated capture,
invalid-mode, bounds, text/cursor and VRAM-access assertions remain.

S1 fixed-work evidence remains in [the audit](t582-s1-performance-audit.md):
1,000 640x480 captures decreased from 1422 to 69 ms on x64 and from 1478 to
643 ms on x86, with identical complete-snapshot hashes. These are local
rendering measurements, not a claim of whole-product speedup or playability.

## Validation

- Existing F65535 contract target built and passed on x64 and x86.
- Both aggregate Release builds completed using the existing Ninja/cache trees.
- NXVM documentation gate passed.
- All 712 unit cases passed on each architecture (1,424 passes total).
- All 48 registered x64 integration cases passed in 219.69 seconds, including
  native Window/Console/MIDI fixtures and PC110 real-firmware routes. This is
  fixture qualification, not manual RDP/gameplay acceptance.
- All 48 registered x86 integration cases passed in 278.43 seconds.
- All 14 receivers are built and deployed; PE widths and hashes were rechecked
  after the final profile builds. The integrated comparison is complete below.

Task logs live in build/t582 until their final results and hashes are archived.
The T582 goal remains open. No manual desktop/RDP acceptance is claimed here.

## Built receiver inventory

Shared source/test revision: `d8a94dc61` (S2 P2). The accompanying NXVM
delivery changes only build target identity, documentation and PC artifacts.
All 14 PE machine fields match the filename width. PC110 retains its accepted
unnumbered filename pair; the embedded common PC version is 0.5.0582.
MyNES and MyRaidenII retain their existing product versions.

| Artifact | SHA-256 |
| --- | --- |
| `assets/nxvm/nxvm_0_5_0582_x64.exe` | `4D6C735041D763F8B213CA0B10EE52F45C050F8E74D9809DDBF61980226448A8` |
| `assets/my5160/my5160_0_5_0582_x64.exe` | `32A41CFB18397BD58BCB770398266BCB9811B6DBD7E64C37C5F920D18F0DDFFD` |
| `assets/my5170/my5170_0_5_0582_x64.exe` | `9D71392D18FCD7D638FAA1DCA21716E34F41B104A5CB5F14B7F91BD44E3D2843` |
| `assets/mydeskpro386/mydeskpro386_0_5_0582_x64.exe` | `A5A111C4D6BE7A757ACCA92C205A6960513425967E16F17DA2E8083E57E4827C` |
| `assets/mypc110/mypc110_x64.exe` | `637E13B79324E5C7F4964195C8A4BD2208312F74E0F590B896C557DD90E723E9` |
| `assets/myraidenii/myraidenii_0_0_0007_x64.exe` | `9E3D325CD246EE717AD72125AD2645C2EE8BF766010AA607362048749EEED59A` |
| `assets/mynes/mynes_0_0_0044_x64.exe` | `B499CD2583ECDDD1AE01EED882E21D49D20187558E2138858E5A27117CF74FC8` |
| `assets/nxvm/nxvm_0_5_0582_x86.exe` | `F5DC30BA3C792E3222AB931733C1227DBE20E09230FC47E7564594982348236B` |
| `assets/my5160/my5160_0_5_0582_x86.exe` | `84F806003A35395BC57CF97D1520D60E173A4AAFE39823D1CF17C0FF6F420644` |
| `assets/my5170/my5170_0_5_0582_x86.exe` | `CB41103F81AF5176BFAF17D39C3CAE021A5B80AB43689D1F9369D85D6B638C59` |
| `assets/mydeskpro386/mydeskpro386_0_5_0582_x86.exe` | `2E390E17E62ECBC9559DFBEB3FB2CA35E6ACF4605B508A2C3CE0ECB0F35C9316` |
| `assets/mypc110/mypc110_x86.exe` | `C4427A51DB42349F79245B9C3603A4CECE94BE1430D2D36EE6FADAA255D7FF21` |
| `assets/myraidenii/myraidenii_0_0_0007_x86.exe` | `199484A80395643EB64A087BDA8F447FDF4AD57DF9FD93E51402718475674F5F` |
| `assets/mynes/mynes_0_0_0044_x86.exe` | `718617AF2FBF6D49C36AE3BB10043DC3B2383E35440E7A371E503B8BD4B4EA16` |

## Qualification defect: Console fixture teardown

Both selected-set MyRaidenII startup probes complete 2,000,000 instructions,
33,287,472 guest ticks and the same frame digest `db1f73fa40511ae0`.
The x64 product Console probe passed. The x86 probe printed its OK marker but
then exited with access violation 0xC0000005. This was a failing test, not a pass.
GDB located teardown in CRT exit/load initialization and GetFileType after
FreeConsole, with an invalid-handle exception. An independently compiled
original fixture reproduced the crash; a private cleared-handle candidate
exited successfully with the same product.

The root `test/setup/console_process.c` now records the caller's independent
standard file/pipe handles, excludes console handles invalidated by detach,
and restores the standard-handle slots after releasing the borrowed Console.
It also checks the child's actual exit code after waiting. This adds 17 test
fixture lines, no production changes, ABI or recovery mechanism. The finished
fixture passes the selected-set product control probe on both architectures.
Both architectures also pass the three other configured PC product sets:
My5160 3/3, My5170 5/5, DeskPro 6/6; the nine affected Default-PC/PC110
fixture regressions pass on each width. No cases were skipped. The full unit
suite links all App unit owners independently of the selected product binding.
The 8 superseded 0581 EXEs were removed only after replacement verification.

The opt-in probes prove bounded startup/control, not MyRaidenII gameplay or
sound qualification. Those existing product limitations remain unchanged.

## Integrated performance experiment

A task-owned public-factory receiver uses identical copied CMOS and overlaid
disks. Old and new F65535 implementations are linked separately against the
same current libraries. The link wrapper records existing completed Core
quantum results once per host second; it neither reads private live state nor
changes the executor. RTC is captured through the existing public unbound
quiescent interface before binding and after shutdown/unbinding. Each run is
bounded to 300 seconds, with public frame copies every 10 seconds. Run the
four width/implementation combinations sequentially after other tests finish.
This avoids sampling suspension overhead and concurrent qualification loads.
All four runs completed successfully. Results are reviewed below; these runs
do not prove native Window/RDP interactivity or whole-product playability.

## Receiver verification summary

The full registered suites ran on both widths. The three additional fixed PC
configurations each ran their own integration rows (3 + 5 + 6 per width),
and the changed Console fixture's nine default/PC110 receivers passed after
the correction on both widths. MyRaidenII's two external startup probes and
two final product control probes passed. No new production optimization was
added after this qualification. The original aggregate graphs were restored
to Default-PC/AT after the profile matrix; intermediates and ccache were reused.

### Completed x64 integrated pair

The x64 old-loop receiver exits successfully after 302.50 host seconds including
composition/teardown (300 seconds requested run time). Its final sampled Core
elapsed count is 779,168,653 ticks, approximately 23.61 seconds on the admitted
33 MHz guest axis; final RTC seconds are BCD 23 (initial 00). Thus RTC agrees
with slow guest-time progress at this checkpoint; it is not evidence for a
separate RTC-frequency defect.

The x64 candidate exits successfully after 302.54 host seconds. Its last
sample contains 839,882,676 guest ticks and 154,944,942 instructions, versus
779,168,653 ticks and 144,742,980 instructions for the baseline. The sampled
tick increase is 7.79%; the requested run duration is identical. Final RTC
seconds are BCD 25, consistent with 25.45 seconds on the 33 MHz guest axis.
The first ten-second frame sample matching the existing Personaware image is
sample 24 (about 250 seconds), versus baseline sample 26 (about 270 seconds).
Both match SHA256
`85A35680232CA0AD623DB7DD18D23A79396931ACB133791581D40C37048AEF41`.
This is a single sequential pair, not a variance study or an exact startup
timestamp. The improvement does not establish playability: guest time still
advances much more slowly than host time.

### Completed x86 integrated pair

The old/new receivers completed in 302.16/302.49 host seconds. Last sampled
guest ticks are 653,989,119/671,470,920 (2.67% higher); instructions are
116,879,758/122,302,839. Final RTC seconds are BCD 19/20, consistent with
the guest axis. Neither run produced the complete reference Personaware image
within its 300-second window. Inspection of the candidate's last frame shows
the menu still being drawn (Schedule, ToDo list and Notebook visible).
This is slow progress, not demonstrated usable performance or a proven hang.

The local loop repair is retained because fixed-work snapshots are identical
and the large-frame cost reduction is substantial. Integrated gains are much
smaller; the remaining execution costs require S3. Do not extrapolate these
single pairs to a universal percentage or claim realtime execution.

## Actual-change review and disposition

No Lib, Emulator, X86 Core, other chip algorithm, owner INI/CMOS/media or
snapshot input changed. The source loop is +15/-10; the existing chip test
is +27/-0; root Console fixture is +17/-0. Version, preset and manifest
changes are mechanical. MyNES was rebuilt but its two artifact hashes did
not change; therefore it has no binary diff. The other twelve EXEs are
delivered in their owner-scoped commits. Eight obsolete 0581 files are removed.
No additional production paths, cache state or public APIs were introduced.

Coordinator review checked the actual diff, manifest identities, test scope,
artifact PE widths/hashes and unchanged owner inputs. S2 is a bounded local
optimization delivery, not T582 closure. Continue with S3's measured CPU/memory
audit; whole-product performance and manual desktop/RDP remain unqualified.

Integrated evidence SHA256:

- Driver log: `5FC9F8A993D15A80C590815E64C0E8278F2DB6D1B8AE02983D36EBE916F087D5`
- Probe builder: `BC957AA1784E15CB85564F5538640B66EDE87D5DFAD0C4DF4CCCDA868062C0D0`
- x64 baseline: `252AB710184FEE80553C778D5329C3B4613FAC6333869A3BCF7EAB1571C4F928`
- x64 candidate: `0243974E132D85FD673E53EC2EF3398192BCE01FFAAA70FEF80B6A75CD4D4195`
- x86 baseline: `27E0FC83631D65F0729328FE644A82BB7E2345D67FB991F84ABEB47F9EF0A2B4`
- x86 candidate: `E2BC033A741DFE1AF1022B68236C65F92E80E2BB2F5B14BA243045781A9579A7`

## Archived validation identities

| Log | SHA-256 |
| --- | --- |
| `s2-full-build.log` | `C68EB9F202E13CEB31E272BD2DA7FE0998FCB5B0AB810197EFB975BED2606BB0` |
| `s2-extra-products.log` | `A6E31969FC4686E4F2C2E5532FA265A71EA083346129BCE976EE5226E5897186` |
| `s2-unit-x64.log` | `0D719838A9923C1F180AADDDE274AB8FAFEF153F91D05777ABB56FE938270F72` |
| `s2-unit-x86.log` | `3D5BBBB56815BF925A822AD3980EC5DF9BC0C40D433F5F8142AAC258B68E9B68` |
| `s2-integration-x64.log` | `BDD7910DDC9068BDBCDF8F0219E6EB88C9B03CEBE2455D404B7FC0A706AD7CE2` |
| `s2-integration-x86.log` | `6C05A3B01C6A188DFE3101CEC03F6994BFA8B4C8CD6673A9F423D097E2995AB2` |
| `s2-other-profiles-and-fixture.log` | `789A1BDE8C9CFB69E59181AD50A0B19285DA449B82E2916768CDDBF2CD96F481` |
| `s2-console-fixture-validation.log` | `F8AE1554E092ECEDC6449E978F428232B2A26F7E5E6F62C8ECD9CD809B4AA409` |
| `s2-chip-regression.log` | `15D1D8D9BFEDBAF6B3111090094055D3A81350BA3955A8F59B30FCF5E1CB5528` |
