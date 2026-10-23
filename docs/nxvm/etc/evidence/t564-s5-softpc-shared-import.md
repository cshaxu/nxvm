# T564 S5: SoftPC shared-package import

## Baseline and change

Import the four committed SoftPC source/test pairs at
`898e40546f6182cc0f6649c5dfaf8aab6b414d6e`. Its eight roots are unchanged
from the S4-audited `9ee4af66`; SoftPC's tracked working tree was clean at
selection. All eight imported roots compare byte-for-byte equal to the
selected checkout. X86 Core and X86 NXVM production code are not imported.

This imports the matched PCM, Console Broker, Emulator UI/Product teardown
contracts together. The latter consume UI resources on normal error returns;
unjoinable worker paths are fatal. The App entry points use the retained
single Emulator Product run path, not an extra cleanup owner.

## Receiving regression discovered

The first x86 integration run failed five NXVM guest tests because S3 made
an unprogrammed EGA return `LIB_STATUS_UNSUPPORTED`, while their display loops
treated it as a terminal error. This is independent of the SoftPC import.
The five tests now continue executing while the frame is unready, reject other
errors and retain their existing guest output, controller and pixel checks.
The real-session rerun passed all five. The first sandboxed Window modal case
also failed; an elevated real-session rerun passed it. No user INI or master
media was changed to obtain those results.

## Verification

The existing Release Ninja/ccache trees are reused. Whole-repository unit
and specialized targets passed 697/697 on each host architecture before the
App integration-only changes. The default-profile integration rerun passed
39/39 on both widths; 5160 passed 19/19 and 5170 passed 21/21 on both widths;
x86 DeskPro passed 22/22. The x64 DeskPro batch passed 21/22: its
DeskPro boot and frame cases passed, but MyNES native Console timed out
waiting for Escape to pause. A separate real-session rerun of that exact
MyNES test passed (1/1). The first batch therefore was not wholly green;
the reason for the intermittent failure has not been established.

The counted C/H source and test changes exclude manifests, CMake, README,
documentation and generated artifacts:

| Owner | Added | Removed | Net |
| --- | ---: | ---: | ---: |
| Lib source | 67 | 83 | -16 |
| Lib tests | 102 | 23 | +79 |
| Emulator source | 22 | 44 | -22 |
| Emulator tests | 157 | 183 | -26 |
| X86 IBM PC source | 10 | 12 | -2 |
| X86 IBM PC tests | 3 | 3 | 0 |
| NXVM integration tests | 34 | 21 | +13 |

X86 Debug C/H is unchanged. The positive test growth is inherited from the
upstream worker-retirement and failure-path regressions; no additional
production ownership or compatibility path was introduced locally.

The ten regenerated release EXEs have the expected PE machine widths. The
two pre-existing owner INI edits are preserved and outside the delivery.
Native desktop tests were automated in a Windows session. Manual product
acceptance and RDP interaction were not performed.
