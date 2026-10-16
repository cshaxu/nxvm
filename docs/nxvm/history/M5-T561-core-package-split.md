# M5 T561 Core Package Split

## Admission And Frozen Universe

Owner admission on 2026-10-15 consumes the first queued [Core split proposal](../proposals/m5-core-x86-core-nxvm-split.md).
Baseline is clean `453455e59`; frozen CPU implementation `b906c7138` is retained.
S1 is the owner's inserted exact import of SoftPC's five source/test pairs,
not extraction or CPU timing completion. The CPU residual candidate remains
queued with its complete original ledger and requires later admission.

| Coverage unit | Owner and completion predicate | Initial disposition |
| --- | --- | --- |
| I0 exact ten-tree prerequisite | Lib/Emulator/X86 Debug/X86 IBM PC/X86 NXVM src/test match pinned SoftPC commit; manifests, root registration and migrated product receivers verified. | S1 accepted after pushed P1/P2 actual-change review; 264 exact files and full receiving proof below. |
| I1 complete disposition | Every current Core source/test/target and public contract has one destination, dependency and verification owner; all PC assumptions and SoftPC split prerequisites classified. | S2 accepted after actual-change review of pushed `82e3257cc`: complete 679-path inventory and explicit mixed-contract receiving decisions; no implementation qualification implied. |
| I2 CPU/FPU extraction | One independent shared CPU/FPU implementation and corresponding tests, correct interfaces, all old consumers migrated and superseded paths removed. | S3 accepted after actual-change review of Shared `8bb5bcaa5` and NXVM `039d946ab`; original CPU functionality/grades retained. I3/I4 private-test receiving obligations remain explicit. |
| I3 neutral execution core | CPU composition, memory/port routes, guest time, transactions and debug independent of PC policy, with real ROM-free non-PC board proof. | S4 P2/P3/P5 accepted in `f136911be`; S5 P4/P6 accepted in `d72a0165d`. P7/P8 and actual extraction/non-PC proof remain open, with one executor/timeline. |
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
`6c95c2da3` provides Core receiving code, tests, documentation and artifacts.
The coordinator reviewed the combined actual diff and accepts S1. I1--I5
remain open and the CPU successor stays paused; S1 is not Core extraction or
complete 486SX qualification. No further S is admitted in this delivery.

The full admission packet is retained in P2's CURRENT revision. Closure
rechecks the ten-tree equality, artifact hashes, preserved input/CPU/MyNES
diffs and push state. The preflight lesson is to build the complete replacement
artifact set before its version-sensitive gate and freeze upstream from a Git
archive before validating against a concurrently changing sibling worktree.

## S2 Disposition And Contract Freeze

Owner-approved documentation prerequisite uses frozen `8db742b6d` and the
architecture-governance then coding-governance skills. Their ownership-first
review rejects a wholesale rename of core/x86: the PC-wide timing table,
device progression order, board trace/default policy and A20 selection require
explicit receiving decisions before moving neutral execution.

[S2 review and reproducible inventories](../etc/evidence/t561-s2/review.md)
account for 197 source and 482 test-package paths, 15,158 declaration/definition
records (not unique exported APIs), 2,326 includes, 2,099 affected CMake
statements and 1,858 external reference lines. Existing graphs supply 460
compiled Core-related targets and 656 registered tests per width; these are
listing/ownership evidence, not a new runtime pass. Mixed 80286/80386 timing
runners remain board-composition qualification. Neutral private fixtures must
not leak into higher-package tests after extraction.

The remaining plan is five bounded implementation/acceptance batches: CPU/FPU;
PC metadata/default/trace separation; neutral execution and real non-PC proof;
remaining NXVM package/receiver consolidation; final qualification. They are
planned S3--S7, not active packets. Each carries its receiving repairs, owner
tests, dual-width proof and applicable artifacts before acceptance.

S2 changes NXVM documentation only. Source/test/build delta is zero and all
ten current EXEs, owner INI, external ROM/media and MyNES inputs are preserved.
Inventory equality and documentation governance are its applicable checks;
S1 remains the runnable dual-width baseline. I2--I5 and T560 C16/C17/final
qualification stay open in their original owners. T561 is not closed.

Coordinator acceptance reviews the original request, complete S2 packet and
actual P1 `82e3257cc`, not merely its report. Fourteen NXVM documentation files
change (+25,912/-9, mostly machine-readable evidence); source, tests, build
inputs and assets have no diff against `8db742b6d`. The reproduced inventories,
Core source/test manifests, corpus boundary and documentation gate pass. The
packet's design-only exception is explicit; no dynamic qualification is claimed.
I1 is accepted as planning completeness, while I2--I5 remain future work.

## S3 CPU/FPU Delivery

[S3 actual-change record](../etc/evidence/t561-s3/review.md), with hashed file
and symbol ledgers, accounts for 12 implementation/header files, 106 matching
test files and six moved CPU-specific checkers. Nine CPU and three FPU source
files exactly preserve the original algorithm/table/layout after the declared
canonical-name and include-path changes. Two test-only Core utility includes
are replaced by their equivalent address/bit expressions. The new independent
package has 112 owner tests and mirrored source/test subcomponent directories.

Original graphs pass complete repository units 627/627 per width, all
specialized gates, 48 receiving profile integrations per width and the restored
default identity. Eight current optimized/stripped 0561 PC EXEs pass PE,
identity, version, SHA-256 and real published CLI/INI checks. Imported ten trees,
Lib/Emulator, MyNES and owner INI/firmware/media/snapshots are unchanged.
Manual RDP/physical/full Windows GUI or new CPU qualification is not claimed.

Shared publication is `8bb5bcaa5`; NXVM receiver delivery `039d946ab` retires
the old CPU/FPU implementation and tests. The temporary additive publication
is not a compatibility production path. Coordinator actual-change acceptance
reviews both commits and all 438 frozen file hashes before discharging I2. The execution and
board white-box CPU-header consumers are explicitly recorded as S5/S6
receiving work; final I5 acceptance rejects unresolved private-header borrowing.
I3--I5 and T560 C16/C17/final qualification remain open. T561 stays open.

The governance-only acceptance confirms no retired CPU/FPU paths in the Git
tree or targets in either current graph, eight unchanged qualified artifact
hashes, the ten-tree/MyNES/INI preservation diff and documentation governance.
The full S3 packet remains in the P2 revision of CURRENT. S3 is closed, not T561.

## S4 PC Policy Separation Delivery

[S4 evidence](../etc/evidence/t561-s4-pc-policy-separation.md) accounts for
170 changed source/test/build/manifest paths and eight published artifact
hashes. P2/P3/P5 prerequisite ownership moves the full timing declaration
contract/storage and PC defaults into Board-Base, while producer-owned trace
IDs preserve all 28 numeric values and one neutral buffer. Whole plan rules,
deadline calculations and device emission order match the prior implementation
after the declared owner substitutions. CPU/FPU algorithms and closed 486SX
production admission are unchanged. Production C/H is +282/-266 (net +16),
test C/H +602/-460 (net +142), family checks +20/-8 (net +12).

Both original graphs pass 627/627 complete repository units, specialized gates,
48 receiving integration executions plus restored-default identity per width,
and all eight published CLI/INI/PE/version/hash checks. Eight current 0561 PC
EXEs are rebuilt/deployed. Imported packages, X86 Core CPU/FPU, MyNES and owner
inputs remain unchanged. Manual RDP/physical/full Windows GUI is unverified.
This delivers the I3 prerequisite, not neutral Machine extraction, I4/I5 or
T560's remaining CPU qualification. Coordinator actual-change review accepts
pushed implementation `f136911be`: the admitted scope, table/default/trace
ownership, guard/failure semantics, complete moved policy bodies, test routing,
170 frozen file hashes and eight published artifacts match the evidence.
Imported packages, CPU/FPU, MyNES, INI and other inputs have no change.
Documentation governance and whitespace checks pass. S4 is closed; I3 actual
extraction, I4 and I5 remain open. The full S4 packet is retained in P1's CURRENT.

## S5 Advancement Boundary Delivery

After accepted S4, the coordinator refines the former single large neutral
extraction batch into S5 P4/P6 progression/interrupt separation, S6 P7/P8
decode/construction review, S7 neutral extraction/non-PC proof, S8 NXVM
consolidation and S9 final qualification. I0--I5 and CPU residuals do not change.
The actual scheduler/memory audit justifies these bounded prerequisites before
a directory move can hide PC policy; the current proposal/packet owns the plan.

[S5 evidence](../etc/evidence/t561-s5-neutral-advance.md) records two neutral
advance phases replacing named device slots, preserved Board effect bodies,
neutral interrupt operations and complete dual-width receiving qualification:
627/627 repository units, all specialized gates, 48 receiving integrations plus
restored identity per width, and eight rebuilt/deployed qualified PC EXEs.
Coordinator actual-change review accepts pushed implementation `d72a0165d`:
actual callback/Board/IRQ changes, preserved function/effect bodies, all 21
frozen hashes, test and checker changes, eight artifact hashes and unchanged
protected/other-target inputs satisfy the packet. C/H is +64/-111 production
and +130/-82 tests; six checks are +47/-24. Full S5 admission remains in P1's
CURRENT revision. S5 is closed, not T561; I3 P4/P6 prerequisites are accepted.
The 25 existing higher Board/adapter test files that
directly include execution private headers join S2 T1's explicit S7/S8 receiver
cleanup obligation; final I5 rejects all unresolved cross-package borrowing.

## S6 Address Decode And Construction Delivery

[S6 evidence](../etc/evidence/t561-s6-address-decode.md) and its
[36-file hash review](../etc/evidence/t561-s6-file-review.tsv) record I3 P7/P8:
Board owns PC A20 policy and Boolean pin interpretation; the neutral route
owner consumes an explicit inclusive window/reset-clear-mask, with identity
as its zero configuration. The same live state becomes a clear mask, with no
second decoder, per-byte callback or mirrored Board flag. Actual ROM/cache/
alias/provider/DMA and firmware/entry bodies retain their prior semantics.
Production C/H is +110/-89 (net +21); tests +174/-43 (net +131); family CMake
is +19/-4. Complete scope and retained/private-test receivers are in evidence.

Both original graphs pass 627/627 repository units, all specialized gates,
48 receiving integration executions and restored-default identity per width.
Eight rebuilt/deployed 0561 PC EXEs pass actual published CLI/INI/PE/version/
release-count/hash checks. Three automated desktop unit cases per width pass;
manual RDP/physical/full interactive Windows GUI is not claimed. Lib/Emulator,
imported packages, CPU/FPU, MyNES and owner inputs remain unchanged. Delivery
awaits coordinator actual-change acceptance. I3 actual Machine extraction and
non-PC proof, I4 and I5 remain open; CPU C16/C17/final qualification is not resumed.
