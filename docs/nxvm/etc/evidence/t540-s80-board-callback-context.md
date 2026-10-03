# T540 S80 Board Callback Context

## Contract And Complete Batch

Baseline is accepted S79 P2 `511c302b9`. The executor confirms S80's complete
nineteen-callback receiver, not a public board API migration or physical move.
The existing board allocation owns its borrowed opaque Core execution handle;
its lifetime ends through the existing Core-owned attachment finalizer.
No public API, registry, state mirror, clock or execution path is added.

All nineteen callbacks in `attachment_interface.h` now receive that allocation:
deadline, refresh request/completion, DMA ticks/request/advance, PIT ticks/tail,
PIC pending/acknowledgement, shutdown qualification, media, RTC, peripherals,
device reset, clock reset, NMI refresh, finalization and firmware completion.
Clock initialization also operates on the actual board allocation.
All callbacks retain their original algorithms and ordering. Firmware uses
the borrowed Core handle for the same bounded ROM publication/rollback path.

## Construction And Failure Ownership

The constructor allocates one board, records its Core handle and publishes one
complete copied binding before fallible clock setup. Allocation failure still
destroys the unpublished Core. Rejected publication finalizes the unpublished
board and clears its temporary construction association before Core destruction.
Every later failure has the existing Core-owned finalization path.
The finalizer destroys chips in the original order and releases its own board
allocation; it no longer traverses or writes Core layout. The remaining private
Core association is not dispatched after that release: Core then destroys its
execution resources and itself, exactly as before. No separate revoke loop is
introduced. Public board operations still need their later whole caller cut.

## Sweep And Regression

Searches cover NXVM production, tests and CMake: attachment context, nineteen
callback definitions, direct callback calls, finalizer/clock initialization and
old callback-owner casts. The scheduler fixture forwards with its saved complete
production context, not `probe->machine`; its standalone deadline calls use the
published context. Controller authority asserts context equals the actual board,
differs from Core, and its borrowed Core handle equals the constructed Core.
Existing rejected/frozen publication and partial-construction tests remain.
The existing null-context NMI no-op is preserved and directly exercised by
the controller fixture. Review caught an initial mechanical dereference before
that guard; the correction is included in the final dual-width verification
and all eight products, not left behind an earlier successful run.

All nineteen definitions are matched against their board-context declaration by
the existing controller gate. Advance/deadline may not traverse `machine->` or
`board->core->`. Four existing DMA, PIT, refresh and PIC gate literals follow the
same owning state rather than deleting their assertions. A bounded ignored
negative copy rejects the six previous owner violations plus a restored
Core-context callback variant. Positive and all seven negative checks pass;
the temporary copied source is removed.

Remaining chip wiring callbacks in `machine_board.c` still receive opaque Core
or direct chip context according to their existing registration. They are not
the attachment callback class and are retained with the public board API caller
migration, together with D4/plan/display and direct-board fixture classification.
Core-only NMI/reset/A20 callbacks call bounded Core operations, not CPU layout.
Board advance retains the private trace declaration include; moving that existing
trace declaration to its durable neutral boundary belongs to physical-header
receiving work, not a new trace API or behavior in this context receiver.

## Verification

The two initial scheduler/controller probes pass. Final complete units pass
470/470 on x64 (278.48 s) and x86 (79.47 s), using `ctest -L unit -j 8` in
their existing independent trees. Both complete specialized-gate runs pass;
the strict compilation inventory retains 377 strict and 25 deferred rows of
402. This does not silently accept those previously declared deferred rows.

Source/test changes span six files, adding 197/removing 187 lines, net +10.
The sole borrowed handle and its null-context regression account for the small
net increase; no state mirror or forwarding API is created. The sweep searches
all nineteen callback names in `src/app-nxvm` and `test/app-nxvm`, then reviews
the complete constructor, teardown and NXVM gate diff. Only the declared
scheduler forwarding fixture had direct calls to migrate.

The initial specialized run rejected a stale PIC gate literal after the owner
move. Its literal now follows the actual board state, retaining the assertion;
both final complete gate runs pass. After the null-context correction, x64
units and the first-built default products were explicitly rebuilt/reverified;
x86 and the other products already consumed that final source. Native desktop
unit suites do not overlap each other or the boot runs. Separate product trees
may compile concurrently.
During the final product generation, read-only process inspection found
concurrent sibling-project x64/x86 builds. They were not terminated or changed;
the NXVM build was allowed to reach its own terminal result.

All eight product builds and independent neutral Core executions pass. Every
existing real-INI profile/width row ran exactly once and passed: default x64/x86
reach `dos-prompt`; XT, AT and Model40 x64/x86 reach `installer-running`.
The existing `vm-profile-floppy-boot-matrix` receives `assets/nxvm`, the unchanged
profile-relative `NXVM.ini` and 180000 ms containment; its observed checkpoint,
not timeout or an inferred speed result, determines success. Core emits
`M5:T540:S69:NEUTRAL-LINK:OK` in all eight product trees.
Compact logs are `build/s80-unit-<width>.log`,
`build/s80-gates-<width>.log`, `build/s80-<machine>-<width>-{build,neutral,boot}.log`
and `build/s80-artifacts.log`; the final default rebuild also has its successful
terminal command record. These proofs do not establish indefinite absence of
intermittent faults or close T540. Actual pushed-diff acceptance follows P1.
Shared, MyNES, INIs and external media masters have no admitted change.

Incremental trees and compact S80 logs remain needed by this receiver and the
immediate whole board-API receiver. No unbounded trace is produced.

## Product Artifact Identity

All eight existing Release products were rebuilt from S79 P2 plus this
source delivery and unchanged approved embedded BYOB inputs. PE architecture,
absence of `.debug` sections, `0.5.0540` banner and freshness against both board
state and final callback source pass. No protected original or generated byte
source is imported; MyNES products and owner INIs are unchanged. These are
current developer/product artifacts, not new hardware or timing qualification.

| Product path | Bytes | SHA-256 |
| --- | ---: | --- |
| `assets/nxvm/compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x64.exe` | 1335479 | `50627B759865F5AADE51EB807CCDFBD0875471AE1996D3AD59BB723ADB8C6F86` |
| `assets/nxvm/compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x86.exe` | 1505286 | `FE4E65B7F20897D63BCA37E7427F12856EFDAE9C04D5AB8D676E40956A4E7C25` |
| `assets/nxvm/default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x64.exe` | 1351796 | `E0B6BEBBC07BA83DA6190647A6B2CA1B0F6E671961C0BDD27403DAD5FAB8C904` |
| `assets/nxvm/default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x86.exe` | 1521601 | `F057B969AF87BFDF9A2724AFE9178FC71FB77BDA60BD4E5795A72051C02E9831` |
| `assets/nxvm/ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x64.exe` | 1351764 | `EFF1B400209CAA0CCA6D7034317BB8627E2975D5B3A6AB45689794D5744CDD56` |
| `assets/nxvm/ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x86.exe` | 1521568 | `E345C7C170F96B9E62B24727FC87C60D39C4E74681BDF509B4148763E25832BC` |
| `assets/nxvm/ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x64.exe` | 1351830 | `BE3D4FEAB9B96E21D75B1FCF2E7B94C511B8F398579A9C8D341EB0780EFA891E` |
| `assets/nxvm/ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x86.exe` | 1521636 | `A4D0C60FF52B33C1C867E9ADCDC0982EE45BEDDA6FB1023A7A2DFDB410EA4DBA` |
