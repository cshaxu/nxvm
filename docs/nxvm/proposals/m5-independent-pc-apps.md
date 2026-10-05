# Four Independent PC Apps

## Goal And Dependencies

Admitted as M5 T543 on 2026-10-04 after accepted T542 S20. The owner
requires one complete App per sequential S, using the exact names below. After
the accepted [shared PC Product extraction](../history/m5-shared-pc-product.md)
and [T542 remaining shared Machine/helper extraction](../history/M5-T542-shared-pc-machine-adapter.md), split
the existing four-machine NXVM shell into four top-level products:

- `src/app-my5160`: existing IBM 5160 XT.
- `src/app-my5170`: existing IBM 5170 AT.
- `src/app-mydeskpro386`: existing DeskPro 386 Model 40.
- `src/app-nxvm`: existing default 386 PC/AT hardware, named NXVM rather than
  a separate default-at product after cutover.

PC110 remains a separate [future task](m6-pc110-evidence-and-implementation.md),
not an empty fifth App. T539 chips and T540 shared board integration remain
accepted dependencies; T542's S11 relocation and corrective
[composition completion](../history/M5-T542-pc-composition-completion-proposal.md) are accepted.
The additional [S20 residual correction](../history/M5-T542-at-composition-residual-proposal.md) is
accepted. All four Apps share PC version declarations
from ibmpc/product/version_interface.h. Shared AT materialization serves all three AT models;
no future App may import a peer profile or copy the remaining generic factory,
asset-finishing or build machinery.
This task does not reimplement them, T541 Product or T542's shared adapter.
The selected composition, common candidate finishing and injected embed/deploy
recipes are already shared; this successor moves model definitions and supplies
App identities/build bindings rather than copying those implementations.

## Design And Cutover

Each App owns immutable machine identity, board composition, firmware/build
binding, matching tests and artifacts. The owner's additional direction keeps
one PC-family docs/nxvm tree, tools/nxvm tree, shared version declaration and
NXVM MTSP sequence; do not create four governance/tool/version copies. All four
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

The NXVM change target continues to own all four PC-family Apps; Shared remains
the separate target for reusable PC/build mechanisms. These are not four new
commit-target or task-number sequences. Accepted Td S175 reconciles the shared
rule's old deployment mapping with the owner's parallel assets layout.
Each migrated App uses assets/<app> directly, without a profile subdirectory.
Owner-approved Td S176 supersedes S175's directory-depth choice: rebase
relative media references one level while retaining the same external master,
access mode and all other INI settings. An unmigrated App keeps its
existing assets/nxvm/<profile> pair and adjacent owner INI.
Relative media paths must retain their meaning; no machine/media setting or
external master may be changed merely to accommodate a directory move.

## Verification And Exit

All four Apps independently configure/build using only their own and declared
shared inputs. Tests mirror test/app-<product>, with separate integration trees;
assets/<product> holds each App's own current pair and adjacent INI. Family
documentation/tools and version/MTSP ownership remain unified.
No obsolete cross-App reference, duplicate shared Product implementation or
orphaned build/tool/document path remains. Preserve every existing integration
scenario and acceptance predicate, including all 58 profile/width contexts.

Required units/integration, manifests and dependency/document gates pass.
Deliver each App's optimized stripped x64/x86 pair under the approved mapping;
retire old artifacts only after replacement verification. MyNES and external
masters remain unchanged. Missing governance, lost capability or incompatible
INI/media behavior blocks cutover rather than weakening acceptance.

## Sequential App Deliveries

The [T543 convergence ledger](../history/M5-T543-four-pc-apps.md) freezes the
four receiving owners and cross-cutting obligations. Admit only one S at a
time; a future row is a plan, not permission to execute concurrently.

| S | Complete App delivery | Required exit |
| --- | --- | --- |
| S1 | My5160: move XT composition/ROM declarations and its tests; bind the shared Product to one XT constructor and supply the App build entry. | Independent selected XT production graph, no peer-App source dependency, original XT assertions and boot checkpoint retained, complete units on both widths, usable verified XT x64/x86 pair, direct-diff review and pushed delivery. |
| S2 | My5170: move IBM AT composition/ROM declarations and its tests; resolve the current default/5170 translation-unit overlap at the shared PC mechanism owner, not by copying it. | Independent IBM AT graph, original 5170 topology/firmware/boot checks retained, complete units on both widths, verified AT pair, no new duplicate assembly, reviewed and pushed. |
| S3 | MyDeskPro386: move Model40 composition, D4 and ROM declarations, observations and corresponding tests. | Independent DeskPro graph; sole D4 owner and unchanged Compaq personality, CMOS and media semantics; complete units on both widths, verified Model40 pair and original boot checkpoint, reviewed and pushed. |
| S4 | NXVM: retain only the original default 386 machine; remove the obsolete multi-profile App shell/selection and finish current paths, names, documentation, tools and test ownership. | Independent NXVM graph, same default hardware and firmware, verified NXVM pair, complete units and all original 58 integration contexts once, all four graphs/manifests/boundaries and eight deployed binaries accepted, no obsolete live route, whole-T actual-diff review and closure. |

The receiving roots are parallel:

| App | Source | Test | Artifact target |
| --- | --- | --- | --- |
| My5160 | src/app-my5160 | test/app-my5160 | assets/my5160 |
| My5170 | src/app-my5170 | test/app-my5170 | assets/my5170 |
| MyDeskPro386 | src/app-mydeskpro386 | test/app-mydeskpro386 | assets/mydeskpro386 |
| NXVM | src/app-nxvm | test/app-nxvm | assets/nxvm |

Each S repairs its direct build/include/tool/document references in the same
delivery; no later S is a receiver for a broken earlier App. Other runnable
machines remain available throughout migration. Shared tests stay with their
real shared owner; a multi-App fixture is test-only and must not leak into a
production library. Every source-changing S rebuilds the affected pair with
shared task revision 0.5.0543; unaffected pairs retain their last verified
identity until their own delivery. Shared executable inputs changed by a batch
require the receiving-App rebuilds under the execution policy.

Lib/Common/x86 source and tests, MyNES and external masters are not change
targets. Necessary common PC work is confined to ibmpc and its tests with
separate Shared commits. Accepted deployment Td S175 governs the cutover;
approved relocation must
preserve all non-path INI values and resolve rebased paths to the same external
media. The four Apps are
not four copies of the Product runtime, tools or governance.
