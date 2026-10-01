# M5 T540 S8 Atomic Board Port Routes

S8 gives the existing Core port owner one typed, atomic, noncontiguous route
batch. The 92h A20 board endpoint, single/cascaded PIC, system/auxiliary PIT,
and XT PPI register through that operation. Their old raw `t_port` callback and
adapter-local checkpoint paths were deleted. The private port table remains the
only route storage; the existing contiguous registration uses the same private
single-route installer. No chip algorithm, address, profile input, INI or media
contract changed.

The PIC/PIT/XT attachments now compile with the current Core runtime because
they call its registration contract. The KBC attachment also moves compilation
owner, without source change, because it already calls the PIC board IRQ-source
functions. This avoids a circular static-library dependency; S9 still owns
KBC's raw-port conversion. The three PIC board tests link the complete Core.
The T345 residual inventory dropped these now-strict adapters and four stale
Shared CPU rows; its ownership verifier and negative self-test pass.

## Review and verification

- Actual NXVM source, test and CMake diff reviewed against the S8 packet:
  28 tracked code paths, 771 added and 621 removed lines, net +150, by
  `git diff --numstat`. Production source is +173/-102 (net +71); unit tests
  +591/-503 (net +88); CMake +7/-16 (net -9). The positive source delta is
  the one route-batch contract and typed callback conversion; no second table
  or forwarding registration path remains.
- Route-batch regression proves a failed allocation rolls back a noncontiguous
  two-route candidate without removing an existing route, followed by a clean
  retry and duplicate rejection. Existing PIC/PIT allocation-failure and XT
  PPI rollback cases now exercise the same Core-owned table. Focused PIC
  single/cascade, IRQ, PIT, auxiliary PIT, 92h and XT tests pass.
- Full repository-only x64 and x86 `ctest -L unit -j 8` pass 467/467 each.
  Both complete Debug trees build. NXVM documentation governance,
  `verify-t345-deferred-direct-ownership`, its negative self-test, CMake's
  source-owner configuration gate and `git diff --check` pass.
- Four selected fixed profiles build optimized Release artifacts on x64 and
  x86. The deployment gate verifies each PE architecture; `objdump -h`
  confirms no `.debug` section in any of the eight EXEs. Their adjacent INIs
  remain byte-unchanged. The broad Debug build incidentally regenerated two
  MyNES EXEs; both were restored to HEAD before review, and no MyNES path is
  part of the delivery.

| Profile | x64 SHA-256 | x86 SHA-256 |
| --- | --- | --- |
| XT 5160 | `5B60C3C4182A0A42BE56DC21655FB1B2B51F87A8DDEDCABA577C88B974F2332C` | `88F35BEF033E026E72C3EF3A4885129849A9BE7DCC8633D65A47645F7EDD25F1` |
| AT 5170 | `D464921C344F80848CE9837401DF7ABBACD5D8265933D0D7F3F0C5359296255D` | `CCE4E27E15D190F89C2EED68C4D5B75EBF3D1F145429960555C1D87DDAAB0EC7` |
| Model 40 | `13BFC992922A38F1ACAD17E5D53A443AC1AC11D2D9C23BAEF543F683FD370504` | `4F013D673FEB324B676A6FAB2686554B60BF3587AFD253ECDD8199920F097201` |
| Default PC/AT | `2957505D399CC601BB9787549E4FA45630FF5E876165EEE1C754055A7F2087ED` | `64684BF7305B88663689D951B27FC414F56579C4091B6EF51942FC9BA5866E41` |

S9 receives KBC/FDC/DMA ports, S10 VADP/HDC/RTC and the 3F7 wired-OR.
The unchanged raw registrations in those owners are not an S8 duplicate
production path. S11-S15 remain the memory/time/reset and physical neutral
Core cut. The complete external four-machine integration gate remains T540's
task-level exit, not an S8 claim.
