# M5 T539 S76 Shared CPU Cutover Evidence

## Scope

S76 establishes `src/x86/devices/cpu/` as the canonical nine-file CPU
corpus.  `x86-cpu-shared` owns the four compiled implementation units;
NXVM's historical `x86-cpu` target is now only a CMake alias to that one
target.  The alias preserves existing consumer spelling without a forwarding
library, duplicated objects, CPU mirror, or second runtime owner.

The Core board continues to own the callback-only CPU bus and PIC wiring.
The CPU continues to own its opaque execution context.  This is a location
and build-graph cutover: it changes neither instruction semantics nor board
wiring.

## Deliberate staged remainder

`src/app-nxvm/devices/cpu*` remains in the tree solely as the historical
source copy while S77--S80 move bounded CPU-only receivers.  It is not named
by any active NXVM build target or active consumer include.  S81 owns its
physical deletion and the final no-duplicate-source sweep.  Therefore this
record does not claim the S81 deletion exit early.

The inherited instruction engine's temporary diagnostic exception remains
location-only in the Shared target: `cpu.c` and `cpu_instructions.c` retain
their former non-strict diagnostic treatment.  S82 must remove it or record
an explicit, reviewed retained exception; S76 does not suppress new warnings
outside that inherited source boundary.

## Verification

| Check | x64 | x86 |
| --- | --- | --- |
| Configure and compile `x86-test-cpu_contract` | pass | pass |
| Public opaque CPU contract | pass | pass |
| CPU-bus authority negative controls | 72 CPU + 5 board + 96 migrated-board controls pass | same |
| CPU timing focused runners (T359 S2--S6) | pass | pass |
| Repository-only unit aggregate | 427/427 pass | 427/427 pass |

`src/x86/verify_corpus.cmake` and both Shared manifests pass as part of the
Shared corpus test suite.  The pre-existing stale `video.c` manifest hash was
corrected in the Shared P1 so that this check once again proves the entire
Shared source manifest, rather than passing around a known mismatch.

## Artifact determination

CPU object code is a runnable NXVM input.  The eight 0539 profile/host-width
Release artifacts were therefore rebuilt from the S76 graph.  Each recorded
PE machine value is read from the rebuilt executable, rather than inferred
from its filename.

| Profile | x64 SHA-256 (`0x8664`) | x86 SHA-256 (`0x014c`) |
| --- | --- | --- |
| Default PC/AT | `DFA052D4CCE4FC2BB27A6341C5C1A61FC05C6375F790730DB6AD40FCC2723AAE` | `C21F997689098866997849DEE7A532214EDB49DE130416B475B8A6F9E6C67DE3` |
| IBM 5160 | `F189B8B98CA3A9722A4A46FA025A841BC8961F92E5C540BE3EFE69B36ABCE088` | `4AA564E52DAAF988CF7ADF47B9491A626B1759528615823B6576E3C6499E6DCA` |
| IBM 5170 | `BE057148A5F6914335CBE9D222940F53B7CBB24371012375C3ACF7CDCAB786FB` | `4142D2ACE9A4CB85FAEFFCEB6C50B9C47330395D55EF4328D4C3C28D53209257` |
| DeskPro 386 Model 40 | `E69EE5DF1E96688D9A3F592BC03082DC6A39555576531DDF0900803E7486D7C8` | `11B914450F928ACDEBB41B06D0799A7D117EC72A4C97F0918A9288CA5556FFDD` |
