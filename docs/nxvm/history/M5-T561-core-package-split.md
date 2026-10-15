# M5 T561 Core Package Split

## Admission And Frozen Universe

Owner admission on 2026-10-15 consumes the first queued [Core split proposal](../proposals/m5-core-x86-core-nxvm-split.md).
Baseline is clean `453455e59`; frozen CPU implementation `b906c7138` is retained.
S1 is the owner's inserted exact import of SoftPC's five source/test pairs,
not extraction or CPU timing completion. The CPU residual candidate remains
queued with its complete original ledger and requires later admission.

| Coverage unit | Owner and completion predicate | Initial disposition |
| --- | --- | --- |
| I0 exact ten-tree prerequisite | Lib/Emulator/X86 Debug/X86 IBM PC/X86 NXVM src/test match pinned SoftPC commit; manifests, root registration and migrated product receivers verified. | S1 qualified; implementation delivery and acceptance audit below. |
| I1 complete disposition | Every current Core source/test/target and public contract has one destination, dependency and verification owner; all PC assumptions and SoftPC split prerequisites classified. | Next inventory batch; not discharged by import. |
| I2 CPU/FPU extraction | One independent shared CPU/FPU implementation and corresponding tests, correct interfaces, all old consumers migrated and superseded paths removed. | Future batch; original CPU functionality/grades retained. |
| I3 neutral execution core | CPU composition, memory/port routes, guest time, transactions and debug independent of PC policy, with real ROM-free non-PC board proof. | Future batch; one executor/timeline. |
| I4 NXVM consolidation | Chips, board-base/XT/AT, driver and family Product in X86 NXVM; all four Apps use the same package, no duplicates or old-path aliases. | Future batch; real PC product/runtime receivers mapped before moving. |
| I5 final qualification | Full dual-width units/gates, all required PC integrations and affected artifacts, independent package builds, exact ownership and documentation, no dependency on retired roots. | T exit; not implied by staged S1 tests. |

The non-PC proof covers reset fetch through board-owned ROM mapping, RAM/MMIO,
unmapped/alias behavior, interrupt entry, HALT/device progression, bounded run
and pause, side-effect-free debug observation, reset and teardown. It uses
project-authored bytes, not a guest BIOS shortcut or game ROM.

S2's inventory refines I1 to a file-level disposition before extraction; it
cannot reduce I0--I5 or lose T560 evidence. X86 Debug and X86 NXVM import paths
are not copied CPU/board implementations. S1 replaces the original X86 tools
and PC Product path completely, without permanent compatibility aliases.
No new CPU feature, V30, arcade or PC110 qualification is
admitted. Source/asset, scope-separated P and actual-change review rules apply.

## S1 Plan And Review Surface

Pinned upstream: clean SoftPC `3af1955932fcb9e61ceec3456493580e316a12c9`,
owner-accepted split implementation `8046b085`. The ten paths are
`src/{lib,emulator,x86-debug,x86-ibmpc,x86-nxvm}` and matching `test/` roots. Lib and
Emulator each source/test pair already match all paths and SHA-256 values.
New split packages contain 35 source-package and 19 test-package files,
including their independent registration and manifests.
No independent third-party source/license or protected asset is introduced.

Shared changes are exact package import, retirement of superseded src/test/x86,
and necessary test/build registration. NXVM changes adapt existing Core tools,
factory and tests to the split contracts, without changing CPU algorithms.
The PC shell receives only machine lifetime; NXVM information/speed remain
owned by Core and the imported X86 NXVM contract. No new public compatibility
API is introduced. Rebuild and deploy eight 0561 PC EXEs after qualification.
MyNES does not consume the new x86 packages; unchanged Lib/Emulator do not
justify manufacturing a MyNES version change, but full repository regression
still includes MyNES. Preserve all owner INI/media/ROM/snapshot inputs.

Pre-implementation estimate: the split itself is mostly file relocation and
namespace replacement; roughly 35 source-package files and their tests move
to three roots, with about 150--350 receiver/build/checker lines added and
150--350 removed. Refine from the actual normalized diff before acceptance.

## S1 Qualified Delivery

[Actual-change evidence](../etc/evidence/t561-s1-softpc-ten-tree-import.md)
records exact equality of all 264 files at the pinned source revision,
621/621 units and all specialized gates per width, 48 profile integration
executions plus restored-default identity per width, all eight published
INFO/SPEED/INI receivers, and the eight PE/hash-verified 0561 EXEs. Lib,
Emulator, CPU source, MyNES and owner inputs remain unchanged. The sandboxed
aggregate deadline is retained as an aborted attempt, not a pass; native
qualification passes without a budget/assertion change.

Shared P1 `15d86fc8e` imports the split and root infrastructure. NXVM P2
provides Core receiving code, tests, documentation and artifacts. The
coordinator reviews their combined actual diff before S1 acceptance. I1--I5
remain open and the CPU successor stays paused; S1 is not Core extraction or
complete 486SX qualification. No further S is admitted in this delivery.
