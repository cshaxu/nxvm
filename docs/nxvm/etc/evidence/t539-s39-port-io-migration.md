# T539 S39: Port-I/O Test Owner Migration

## Original-case receiving map

| Retired source / original family | Sole receiver |
| --- | --- |
| Scalar IN/OUT: four profiles × eight forms (32), 80386 size/address attributes (24), provider failure (4), VM86 (1), TSS I/O bitmap (3), invalid legacy attributes and LOCK (104) | `cpu_port_io_smoke.c`, using only the CPU bus and fault contract |
| Scalar IN/OUT: pending PIC IRQ without an instruction shadow (2) | `machine_port_io_board_smoke.c`, using the real PIC, public CPU snapshot and interrupt frame |
| Scalar provider failure: host-visible routing and fault publication (4) | `machine_port_io_board_smoke.c` supplies complementary board observations for the same four CPU failures |
| INS/OUTS: single profile/width/address forms (14), REP three/zero/one cases (12), segment override and DF (6), invalid profile/prefix/LOCK forms (20) | `cpu_port_strings_smoke.c` |
| INS/OUTS: single and REP protected segment-limit faults (4) | `cpu_port_strings_smoke.c` tests hidden-cache and rollback semantics; `machine_port_strings_board_smoke.c` tests guest-loaded descriptors and board fault delivery |
| INS/OUTS: single and REP PIC IRQ/restart (4) | `machine_port_strings_board_smoke.c`, using the real PIC, stack frame and provider transaction |
| Port ownership: provider read/write and failure, FDC read independence and conflicting write registration | `machine_port_ownership_board_smoke.c`, retaining the public port registry and real board test |

The two mixed scalar/string tests are deleted. The port-ownership source is
renamed to reflect its board owner; it no longer includes the private CPU
fixture or reads FDC internal state. Its FDC assertion is through port 3F4.
The two CPU receivers link only `x86-cpu`; the two board receivers use public
machine setup and real PIC delivery. A test-local CPU bus fixture records
three transfers, the maximum in this original corpus. No new production or
public API is introduced. The original tests had no explicit timing
assertions, and the historical T316 success markers remain on CPU receivers.

## Verification

The change is limited to NXVM tests, test-only fixtures, and CMake/static
registration. It does not change production CPU, port routing, firmware,
profile, INI, Shared components, MyNES, or EXE inputs. The direct-private
`.c` inventory falls from 61 to 58; the previously recorded common fixture
header remains assigned to later S tasks.

Complete x64/x86 builds and repository-only unit suites pass 413/413 each.
The focused five new/renamed unit targets pass on x64. All 66 specialized
gates pass on x64; T317/T332/T344 inventory checks pass with 35 remaining
historical strict CPU owners. The T344 x64 direct-compilation matrix passes
412 rows. The unchanged six Shared manifests pass as part of 11/11 manifest
tests. The x86 specialized-gate aggregate and direct matrix likewise pass.
The thirteen code/test/build/gate paths add 1,239 and remove 1,564 lines,
net minus 325; this 44-line evidence report is separate. The complete staged
change adds 1,283 and removes 1,564 lines. `git diff --cached --check`
passes. The full builds did not modify a tracked EXE.

Implementation P1 `07019f588` is pushed. Its actual-commit review and S39
acceptance are recorded in `docs/nxvm/states/CURRENT.md`. T539 remains open.
