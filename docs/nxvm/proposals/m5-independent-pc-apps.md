# Four Independent PC Apps

## Goal And Dependencies

This is the first queued successor, not active T541 and not yet numerically
allocated. The owner separated this cutover from T541 on 2026-10-04. After
the accepted [shared PC Product extraction](../history/m5-shared-pc-product.md)
and [T542 remaining shared Machine/helper extraction](../history/M5-T542-shared-pc-machine-adapter.md), split
the existing four-machine NXVM shell into four top-level products:

- `src/app-mypcxt`: existing IBM 5160 XT.
- `src/app-mypcat`: existing IBM 5170 AT.
- `src/app-mypcdeskpro386`: existing DeskPro 386 Model 40.
- `src/app-nxvm`: existing default 386 PC/AT only.

PC110 remains a separate [future task](m6-pc110-evidence-and-implementation.md),
not an empty fifth App. T539 chips and T540 shared board integration remain
accepted dependencies; T542's S11 relocation and corrective
[composition completion](../history/M5-T542-pc-composition-completion-proposal.md) are accepted.
The additional [S20 residual correction](m5-at-composition-residual.md) must be
accepted before split admission. All four Apps share PC version declarations
from ibmpc/product/version_interface.h. Shared AT materialization serves all three AT models;
no future App may import a peer profile or copy the remaining generic factory,
asset-finishing or build machinery.
This task does not reimplement them, T541 Product or T542's shared adapter.
The selected composition, common candidate finishing and injected embed/deploy
recipes are already shared; this successor moves model definitions and supplies
App identities/build bindings rather than copying those implementations.

## Design And Cutover

Each App owns immutable machine identity, board composition, firmware/build binding
and its product-local documentation, tests, tools and artifacts. All four
consume the same accepted ibmpc/product command/API/INI/startup/UX implementation,
x86 Core/chips, the sole T542 PC Machine adapter and appropriate IBM-PC board
contracts. Apps supply fixed composition, not copies of app-nxvm/machine.
No App imports a peer
App or copies a parser, runner, queue, debugger or presenter.

Freeze the complete source/test/configuration/CMake/tool/document/artifact
receiving-owner map before moves. Shared tests stay shared; board/firmware and
external integration scenarios follow their concrete App. Preserve original
assertions, provenance, CPU families and all existing machine capabilities.
Do not reintroduce runtime model selection, YAML or host BIOS shortcuts.

Before new-target commits, a separately admitted Td establishes allowed
commit targets, product guides, executable names/version continuity and
deployment directories. Until then assets/nxvm/<profile> and adjacent owner
INIs remain authoritative. Relative media paths must retain their meaning;
moving source alone does not authorize INI or external-archive rewrites.

## Verification And Exit

All four Apps independently configure/build using only their own and declared
shared inputs. Tests mirror test/app-<product>, with separate integration trees.
No obsolete cross-App reference, duplicate shared Product implementation or
orphaned build/tool/document path remains. Preserve every existing integration
scenario and acceptance predicate, including all 58 profile/width contexts.

Required units/integration, manifests and dependency/document gates pass.
Deliver each App's optimized stripped x64/x86 pair under the approved mapping;
retire old artifacts only after replacement verification. MyNES and external
masters remain unchanged. Missing governance, lost capability or incompatible
INI/media behavior blocks cutover rather than weakening acceptance.
