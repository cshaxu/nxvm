# T538 S4: Console Repair Import And Six-Root Equality

## Admission And Provenance

S3 was already accepted and closed at 763a70134. The owner explicitly admits
S4 to import the reviewed SoftPC repair and verify all six shared roots.
Source is clean SoftPC b79769c19b9b69d2c751fad942f4bc3a1490e7c2. The six changed
files are copied unchanged at the same relative paths:

- src/lib/console-broker/win32/console.c
- src/lib/README.md and src/lib/MANIFEST.sha256
- test/lib/console_broker_display_smoke.c
- test/lib/lib_console_io_contract_smoke.c and test/lib/MANIFEST.sha256

This is the previously admitted project-owner-provided shared corpus under the
repository's MIT grant, not SoftPC's recovered-machine source. No independent
third-party notice is introduced or removed. No App/MVDM code, external asset,
configuration, executable or sibling build input is imported. Public API and
product adapters are unchanged. Sibling checkout remains read-only.

## Mechanism And Similar-Issue Sweep

The broker remains the only Win32 backing-frame owner. Capacity is queried
after palette application. Cached completed cells and retained clearing extent
are independent facts: failed/partial output invalidates the former, preserves
the latter; success commits the new frame and reduces coverage. Host shrink
clamps coverage independently of whether the new frame requires expansion.
Steady frames write their active extent, not the backing buffer's spare capacity.

Search: `rg -n 'coverage_rows|previous_rows|write_rows|ensure_text_surface'
src/lib/console-broker test/lib -g '*console*'`. Activation resets both facts;
stream writes invalidate cached cells but retain frame coverage; raw-frame
failure retains attempted coverage; complete output commits the new cache.
The only native rectangle writer is in Win32 console.c. Linux returns explicit
UNSUPPORTED for this operation; KVM leaves do not own native buffer geometry.
Existing cooked-history rollback debt stays in TODO and is not claimed fixed.

Counted three C source/test files via git diff --numstat: production +26/-8;
tests +66/-11; total +92/-19, net +73. The increase buys one private extent
field and failure/shrink regressions, not a new layer or parallel renderer.

## Equality

Recursive relative-path set comparison plus SHA-256 of every file proves:

| Root | Files | Differences |
| --- | ---: | ---: |
| src/lib | 109 | 0 |
| src/common | 23 | 0 |
| src/x86 | 14 | 0 |
| test/lib | 51 | 0 |
| test/common | 20 | 0 |
| test/x86 | 10 | 0 |

All 227 files, including manifests and README files, match the pinned clean
source. test/register.cmake is also identical (SHA-256
28DA71E009EFEC16E2A27E23CA6E7F9856552409FC8097238930A93A2943BF34).
All six local manifests pass. No extra local file or missing source file is
excluded from this comparison.

Manifest revision comments are retained verbatim, including the inherited
shared-m6-t43-s10-p1 label. That label is not treated as a unique commit ID;
the pinned source commit and complete path/hash comparison identify this import.

The staged Git tree objects also match the pinned source, independently of
the working-file comparison:

| Root | Git tree |
| --- | --- |
| src/lib | ce92eff3105a27f8919e790ad861cdb95fe5e086 |
| src/common | f2e9b1cdc260b272f87fdd19a1405607054d1a55 |
| src/x86 | 59b7e4178a2cd1eece0ce0b93a1de49cbc7fdcff |
| test/lib | 85573eb1954da931f7f131793b08f552f800e068 |
| test/common | e558dbd8c80222725cb31fb27015c249169e74d6 |
| test/x86 | 27991b3ad11f6de0ce8e54cd4db444fcd5a7b740 |

## Verification And Delivery

NXVM full unit suites pass 335/335 on each of x64 and x86; x64 external
integration passes 20/20. MyNES full suites pass 132/132 on each width, including
serial desktop tests. All ten receiver builds pass. Six manifests, component
boundaries and Types gates pass. Owner INIs and MyNES snapshot remain unchanged.
T538 repeated real-process boot qualification stays pending; these suites do
not substitute for that final deployed-process matrix.

Eight NXVM product targets have completed optimized stripped Release builds
and native architecture checks. Product source remains the S2 baseline; Shared
inputs are the byte-identical SoftPC revision above. All retain revision 0538:

| Artifact | SHA-256 |
| --- | --- |
| nxvm_at_0_5_0538_x64.exe | 7979C8E38272322654B49B9156716825B2AC0BEB12AACAB498E527E7FAAF8341 |
| nxvm_at_0_5_0538_x86.exe | B8DEDA06092912DBC981B23A081849E00A713AFB07065D66A8BB249BAC1A38CC |
| nxvm_xt_0_5_0538_x64.exe | EFA5E91236B94F740F31D50C9C40D1A8712CA5FC1E06727050C227990482990F |
| nxvm_xt_0_5_0538_x86.exe | F972E349789F4AF52E6AC914AF7357BDA452FA059B4D26CC973267FE5B7B3F23 |
| nxvm_default_0_5_0538_x64.exe | 8D02E1D09BB054E88AA648D081BA692C54590244F7A22E2A148389CC2EC4B015 |
| nxvm_default_0_5_0538_x86.exe | D6CA1B414CDF005FA10340EFE2C23D23B97EC54EE3995FF060BFD84579AA5F26 |
| nxvm_model40_0_5_0538_x64.exe | 76636DEC7E4BFA96CDE4AD122D5BAB07A4040909CDF80313AB7318D8C7ADA749 |
| nxvm_model40_0_5_0538_x86.exe | 721F8C1AD7876842DF703922A1014B5622A74B30A332A8E31FF4DBE0FB0941C3 |
