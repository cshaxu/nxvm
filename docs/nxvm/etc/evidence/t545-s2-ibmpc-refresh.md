# T545 S2 Committed IBMPC Refresh

The owner requested another IBMPC src/test import while the complete S2
receiving batch was still active. Source identity is SoftPC
`8124e551e841ccdec2ceb7f6a0f6ae5b513a7951`; both selected roots are clean.
The other six roots have no committed diff from accepted `03c979c7`.
This refresh does not authorize a sibling edit or local Shared repair.

## Source Review

The complete intervening diff covers 46 paths, 203 insertions and 7,650
deletions. It embeds two existing executor atomics in private Control storage,
removes their independent allocation and public interface, and removes
first-slot media/path union aliases. Operations retain their existing atomic
ordering and hardware behavior. Product destruction now checks UI cleanup
before releasing its borrowed Session/Machine dependencies; failed creation
cleanup retains the owning handles for retry. Imported composition assertions
exercise both failed UI binding and teardown, plus failed Machine cleanup.
The executor test covers pending reset, start and stop on embedded state.

## Removed-File Receivers

All 27 live/helper files below retain their original bodies except mechanical
include paths and reads of changed private layout. Both historical timing
generators retain their NXVM catalogs and existing targets. Shared never
depends on these product-local receivers.

| Old source | Retained NXVM receiver |
| --- | --- |
| `test/ibmpc/board-at/boot_fixture.c` | `test/app-nxvm/unit/support/ibmpc/board-at/boot_fixture.c` |
| `test/ibmpc/board-at/boot_fixture.h` | `test/app-nxvm/unit/support/ibmpc/board-at/boot_fixture.h` |
| `test/ibmpc/board-at/command_fixture.h` | `test/app-nxvm/unit/support/ibmpc/board-at/command_fixture.h` |
| `test/ibmpc/board-at/state_fixture.c` | `test/app-nxvm/unit/support/ibmpc/board-at/state_fixture.c` |
| `test/ibmpc/board-at/state_fixture.h` | `test/app-nxvm/unit/support/ibmpc/board-at/state_fixture.h` |
| `test/ibmpc/board-common/boot_fixture.c` | `test/app-nxvm/unit/support/ibmpc/board-common/boot_fixture.c` |
| `test/ibmpc/board-common/boot_fixture.h` | `test/app-nxvm/unit/support/ibmpc/board-common/boot_fixture.h` |
| `test/ibmpc/board-common/cmos_fixture.c` | `test/app-nxvm/unit/support/ibmpc/board-common/cmos_fixture.c` |
| `test/ibmpc/board-common/cmos_fixture.h` | `test/app-nxvm/unit/support/ibmpc/board-common/cmos_fixture.h` |
| `test/ibmpc/board-common/composition/machine_80186_timing_manifest_runner.c` | `test/app-nxvm/unit/board/machine_80186_timing_manifest_runner.c` |
| `test/ibmpc/board-common/composition/machine_8086_timing_manifest_runner.c` | `test/app-nxvm/unit/board/machine_8086_timing_manifest_runner.c` |
| `test/ibmpc/board-common/composition_fixture.c` | `test/app-nxvm/unit/support/ibmpc/board-common/composition_fixture.c` |
| `test/ibmpc/board-common/composition_fixture.h` | `test/app-nxvm/unit/support/ibmpc/board-common/composition_fixture.h` |
| `test/ibmpc/board-common/controller_fixture.c` | `test/app-nxvm/unit/support/ibmpc/board-common/controller_fixture.c` |
| `test/ibmpc/board-common/controller_fixture.h` | `test/app-nxvm/unit/support/ibmpc/board-common/controller_fixture.h` |
| `test/ibmpc/board-common/kbc_state_fixture.c` | `test/app-nxvm/unit/support/ibmpc/board-common/kbc_state_fixture.c` |
| `test/ibmpc/board-common/kbc_state_fixture.h` | `test/app-nxvm/unit/support/ibmpc/board-common/kbc_state_fixture.h` |
| `test/ibmpc/board-common/video_topology_fixture.c` | `test/app-nxvm/unit/support/ibmpc/board-common/video_topology_fixture.c` |
| `test/ibmpc/board-common/video_topology_fixture.h` | `test/app-nxvm/unit/support/ibmpc/board-common/video_topology_fixture.h` |
| `test/ibmpc/board-xt/boot_fixture.c` | `test/app-nxvm/unit/support/ibmpc/board-xt/boot_fixture.c` |
| `test/ibmpc/board-xt/boot_fixture.h` | `test/app-nxvm/unit/support/ibmpc/board-xt/boot_fixture.h` |
| `test/ibmpc/machine/support/common_machine_fixture.h` | `test/app-nxvm/unit/support/ibmpc/machine/support/common_machine_fixture.h` |
| `test/ibmpc/machine/support/guest_display.h` | `test/app-nxvm/unit/support/ibmpc/machine/support/guest_display.h` |
| `test/ibmpc/machine/support/guest_input.h` | `test/app-nxvm/unit/support/ibmpc/machine/support/guest_input.h` |
| `test/ibmpc/machine/support/media.h` | `test/app-nxvm/unit/support/ibmpc/machine/support/media.h` |
| `test/ibmpc/machine/support/selection.h` | `test/app-nxvm/unit/support/ibmpc/machine/support/selection.h` |
| `test/ibmpc/machine/support/vm_presentation_capture.h` | `test/app-nxvm/unit/support/ibmpc/machine/support/vm_presentation_capture.h` |

The 28th removal, unregistered vm_debug_authority_smoke.c, calls retired
flagTrace/traceCount APIs. No live CMake target refers to it. Current
machine/debug_budget_smoke covers trace budgets 1/10/4096, breakpoint and
watchpoint completion and reset. Retiring this obsolete API test does not
remove an original registered test or justify changing a boot predicate.

## Fresh Verification

Raw file paths/counts and every SHA-256 match all eight source roots.
Source counts are Lib 109, Common 23, x86 95 and IBMPC 125; corresponding test
counts are 49, 20, 180 and 241. All eight manifests pass via the universal
Common verifier. IBMPC corpus, test inward-boundary and test Types checks pass.
NXVM historical fixture-shape gate retains 133 constructors; keyboard
transport and NXVM documentation governance pass.

The final receiving graph passes complete units: 531/531 x64 in 223.46s and
531/531 x86 in 147.44s, using the unchanged eight-job, 300-second aggregate.
Both product-target builds succeed. Old snapshot verification is history only.
The x64 specialized aggregate and 23 supplemental manifest/corpus/Types/DAG/
layout/naming/negative gates pass. The corresponding x86 supplemental set also
passes 23/23 in 55.08s. Documentation governance and diff checks
pass; no integration result is claimed by S2.

The refreshed source exposed a receiver-only static-link dependency: the
firmware floppy fixture linked Core before the board libraries and depended
on an obsolete indirect reference to pull the port-route object. The selected
Core still implements that API. A plain ordering change qualified the firmware
fixture but not the converged App/fixture integration graph: CMake's deduplicated
archive closure still placed Core before board providers. NxvmProduct therefore
declares the selected board-common/AT/XT/Core set as a RESCAN link group on
toolchains requiring archive rescans; automatic-rescan toolchains retain the
ordinary target list. This is a receiving composition link contract, not a new
runtime, dependency direction or API. No imported file, device behavior or
legacy wrapper is changed. Full units and integration builds qualify this
receiver graph for both widths.

The first sandboxed full-unit runs hit the unchanged 300-second aggregate
deadline during concurrent builds: x64 completed 203/531 and x86 334/531,
with no reported failed case before containment. These incomplete runs are
not accepted. Verification is repeated with the regenerated link graph in
the authorized native build environment, preserving the deadline and all
531 tests. MyNES's complete product unit sets pass 43/43 for each width
(23.97s x64, 40.43s x86); both 0043 artifacts were rebuilt without product
source or owner configuration changes.

## Counted Change And Sole Owners

Across tracked src/test C, headers and CMake files, `git diff HEAD --numstat`
with Git's rename detection counts 444 code paths, +3106/-2091 (net +1015).
Documentation, manifests and generated/artifact paths are excluded. Production
code alone is 17 paths, +96/-126 (net -30). The positive test delta preserves
NXVM-only assertions removed from the upstream shared package and imports
additional owner-local fixtures, boundary checks and lifecycle failure tests;
it does not create another product execution path. The complete receiver maps
account for moves rather than reporting them as newly invented implementations.

Core remains the execution/guest-time owner, IBMPC owns board wiring and its
sole Common adapter, and each fixed App owns its existing profile/firmware
composition. Receiving App production source, MyNES product tests, owner INIs,
external media and snapshots are unchanged. The newer IBMPC archive SHA-256
is `14C59C316968380A17B5A7D834E910B478E6977A7A8B60912E6714975B9464A8`.

## Verified Receiving Artifacts

All ten artifacts are optimized Release, with verified PE width and no compiler
debug sections. PC revision is 0.5.0545; MyNES remains 0.0.0043. Runtime Debug
is retained. The initial code baseline is badc7588e plus the exact import and
receiver batch delivered by this S; fixed Shared revision is fc3c73aa1 and
MyNES artifact delivery is f010812fd. Hashes identify these actual builds
independently of timestamps.
Eight superseded PC 0543 EXEs were removed only after all replacement pairs
passed verification; old artifacts remain recoverable from Git history.
Adjacent INIs and other assets are unchanged.

| App / width | SHA-256 |
| --- | --- |
| my5160 x64 | 8F887AE27DE655E320C505EBDE8ED930864CCE62B37ABA877CA40FDD0313DDB1 |
| my5160 x86 | 4C5CA62862BE430BBBDD912E398DF78ED63379AC62A24062FDB39E62EA113C41 |
| my5170 x64 | 5E96C231BCDE0EB7FBFA66972E705A361D721BBAF2E739692829016145ECE85F |
| my5170 x86 | C17898CEC70DD8AC136C351DD478D09C320729F8BD59F646014FA66B17EE5338 |
| mydeskpro386 x64 | A1077C1221B41F86FB890299E205ED17CA2E7E7CA81429CBC5F8768E080FC9EF |
| mydeskpro386 x86 | BA36DCCC56D28A6D8F172FC504D7E4C200B8BEE76675263943475ABE5D5A9025 |
| nxvm x64 | 2027574FB8841CC59C73B3E9957C129821E728E3467BBD271B04B7ED746352A6 |
| nxvm x86 | 1A8DE463ACADE916F4E88614C8459C56CFCF41EA45DABA87F21A54D2028D2E06 |
| mynes x64 | BDE39D3B2C60CFDB6F6CCBE30650C167709D5699FBCBA271781D907C03AAF3C1 |
| mynes x86 | 3B2E99A756AA583EE905329B60D66F6BEF07E03E98B22FE9CE68CEE988D89097 |

## Actual-Change Review

Coordinator review checks the complete S1 difference universe, the latest 46
IBMPC paths, all 71 product-only receivers and receiving CMake/include changes,
not just passing aggregates. Imported production changes remain the reviewed
upstream implementation; no local Shared patch or second owner is introduced.
The two additional integration include repairs point at the same moved fixture.
All original registered assertions and all 58 original external integration
contexts remain required. The unregistered retired Debug test is the sole
explicit obsolete-API disposition. S3 owns fresh external qualification; S2
acceptance does not close the T or claim new CPU correctness.
