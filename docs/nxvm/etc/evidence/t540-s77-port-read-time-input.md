# T540 S77: port read-cycle time input

Baseline: S76 P2 `12e5304cb`. Coordinator admits the complete read-cycle input
receiver; executor reads and confirms the sixteen-field packet before code.

## Finite owner boundary

Source inspection finds twelve production typed read callbacks and twenty-eight
synthetic test callbacks with `void *`, `lib_u16` and `lib_u32 *` arguments.
The two Port-B route consumers share one timer-status helper; it has the sole
board-private `elapsed_ticks` read. Core time getters are deliberately stopped/
paused-only and cannot substitute in Running CPU I/O.

The existing read callback receives one additional copied `lib_u64 tick`.
CPU transfer, paused public bus, paused Debug and bounded firmware supply it
from the sole Core clock at dispatch. Each byte lane and wired-OR contributor
receives that same value. Port-B computes its existing output from this input;
other chip adapters explicitly ignore it because their current register reads
consume already-settled chip state. Write callbacks have no current time
consumer and are unchanged; adding their unused input is not required here.

No new API operation, value type, clock field, observer or dispatcher is added.
Private read-execution operations require explicit time. The existing raw
read convenience function has no production caller and keeps its synthetic
zero-time fixture semantics through the same dispatcher. The source gate will
reject any new production raw-read caller. Legacy callbacks remain existing
test-local port fixtures, not a board production path.

## Inventory and proof plan

Queries: `rg -n 'core_machine_port_(read_provider|execute_read|read\()'`
over tracked NXVM source/tests/build; inspect typed callback signatures and
their route registrations; `rg -n 'elapsed_ticks'` over board source owners.
One direct HDC provider test invocation and private port-assembly calls also
need the copied argument. Firmware callbacks can execute at reset; their input
is the then-current Core time, not a host clock or guessed elapsed quantity.

Existing fixtures will prove nonzero time across all four production entries,
the complete byte-lane/wired-OR dispatcher and Port-B toggle boundary/CPU IN
semantics. Complete dual-width units/gates, actual independent Core linkage,
eight product builds and single external boots remain required. No results
are claimed by this admission record. Attachment, direct-test classification
and physical Core/board movement remain T540 receivers after S77.

## Implementation and source review

The measured signature inventory is still exactly 40: twelve production and
twenty-eight synthetic fixture callbacks. Ten production chip/register reads
explicitly ignore the new value; only the two existing Port-B routes pass it
to their shared status helper. No register algorithm or timing grade changes.
The four Core entry owners read their own elapsed tick at actual dispatch:
CPU bus, paused bus interface, paused Debug and bounded firmware. The private
dispatcher passes it unchanged through ordinary, byte-lane and wired-OR reads.
Writes, route registration/storage, ordering, status/rollback and observation
guards are unchanged. The one direct HDC fixture call now supplies zero.
For all 32 purely mechanical consumer files (eight production adapters and
twenty-four fixtures), removing only the added tick parameter/unused-value
statement and normalizing whitespace reproduces the entire baseline file.
The remaining dispatcher/entry/board and four regression diffs are separately
read hunk by hunk; this comparison is not a substitute for their review.
`git diff --numstat` over tracked `src/app-nxvm` and `test/app-nxvm` counts
44 source/test paths, +256/-70/net +186 lines; docs, gates and generated
products are excluded. The one CMake owner gate is separately +9/-1/net +8.
The positive code delta is callback input plumbing and owned regression proof,
not a second production path, clock state or generic forwarding framework.
The live raw convenience is retained solely for existing zero-time fixtures;
direct-test classification remains the named receiver before physical movement.

The existing neutral fixture observes nonzero paused bus/Debug time. The
scalar I/O fixture verifies CPU delivery after NOP for 8086 and 80386. The
bounded firmware fixture compares its after-run read input with Core's copied
time observation. Port assembly checks every four-byte lane, a wired-OR
contributor and an input above 32 bits; it separately checks zero-time fixture
semantics. Its Port-B matrix covers 0/63/64/65/127/128/129 and a 64-bit boundary
with Core's stored tick deliberately unchanged, followed by a real CPU IN.
Existing D4/PIT, permission, failure, transaction and instruction-timing
fixtures retain their original assertions.

The whole-production owner gate rejects board-family private clock reads and
raw read convenience use outside its sole definition. An isolated positive
probe and three injected negative probes pass: aliased board clock read,
indirect board clock read and production raw read. The negative probe root is
bounded below the ignored build directory and removed after execution.

Development correction: the first named focused build used a nonexistent
short target name; the final Port-B extension initially missed its existing
Debug interface include. Both were corrected before acceptance runs. A test
launched after the latter failed build used an older executable and is not
acceptance evidence. The successful rebuilt focused run is the relevant proof.

## Completed unit and gate proof

`build/s77-units.ps1` serially builds each current unit executable target,
runs complete `ctest -L unit -j 8 --output-on-failure`, then builds
`verify-current-specialized-gates` for the existing x64 and x86 test trees.
The complete script exits zero: both widths pass 470/470 units and all
specialized gates, including the new whole-production owner checks. Native
desktop suites do not overlap. Logs are `s77-build-*`, `s77-unit-*` and
`s77-gates-*` below ignored `build/`. Focused rebuilt regressions and the
separate isolated positive/negative owner probe also exit zero.

Product building is separate from boot execution. The complete eight-product
build script exits zero; each tree also builds its independent neutral Core
proof and existing INI boot runner. Boots do not overlap native units or other
boots. Default x64/x86 reach `dos-prompt`; XT, AT and Model-40 x64/x86 reach
`installer-running`. Each of the eight neutral executions and eight single
real-INI boots exits zero. No checkpoint is retried or replaced by a stale
executable. The build, unit, gate, neutral and boot logs are kept below the
ignored build directory; product inputs remain the existing four owner INIs.

## Artifact identity

All eight current 0540 products have the expected x64/x86 PE architecture,
optimized Release build verification, no compiler debug sections and a build
timestamp after S77 admission. The identity inspection exits zero. No owner
INI or MyNES executable is modified or rebuilt.

| Product | Bytes | SHA-256 |
| --- | --- | --- |
| `assets/nxvm/compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x64.exe` | 1335414 | `C96D98ABA7BEF1467F4A7DF04BC8FE2A947547FF62497BF97E00C5FFA90EA722` |
| `assets/nxvm/compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x86.exe` | 1505220 | `01332B01DB92AEFF99C3F70B2EF61D0AEBBF14358D35078C45F6FA6156C23553` |
| `assets/nxvm/default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x64.exe` | 1351731 | `AE3378CD7DE8180DC27ED99988C5556E74EE028CC36E7CC8D6CA7E8938C4F265` |
| `assets/nxvm/default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x86.exe` | 1521535 | `13CFAAB602A835A5F9B33D1CF41A7926FCB1F5245E21E4BD2252938DAFCF07C2` |
| `assets/nxvm/ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x64.exe` | 1351699 | `9E8258B3020A151C6DFBD6875DC61F57C8B76EC5BF913B468B2E15A7FD4C6F6F` |
| `assets/nxvm/ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x86.exe` | 1521502 | `6C380F7A035F12231532CD37919A018A4A3F1F4756E6B368B713AB718E57A1A1` |
| `assets/nxvm/ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x64.exe` | 1351765 | `FF9D5CD351FD2A5A78A8041073A0A0047CBE7F2A70B5BF80AD3498BFD5BB9C7E` |
| `assets/nxvm/ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x86.exe` | 1521570 | `89460A957B5D1F62145EE18DDA24A868B53CF3BBCD7FB8583262BE45990B95D0` |

These headless external-firmware/media checkpoints do not establish native
UI/audio behavior or indefinite absence of intermittent boot faults. Full T
integration and physical relocation are not claimed. Ignored incremental
trees/logs are retained for actual-change review and the next attachment
receiver; the isolated injected-negative source copies were removed.
