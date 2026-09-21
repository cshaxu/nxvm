# Fixed-Machine Products And Architecture Consolidation

## Purpose And Approval Boundary

Implement the owner's revised product direction: one multi-machine architecture,
one build-fixed Standard 386DX executable and one build-fixed PC110 executable.
Preserve the complete CPU-family implementation/profile/selection-table corpus.
This proposal updates the current governance work; it is not an admission of a
new numeric task or a reopening of the closed layout task.

[Architecture](../design/ARCHITECTURE.md) is normative.
[Research and code observations](../etc/research/fixed-machine-selection.md)
record current evidence. Standard's exact board/ROM set is not yet frozen.

## Scope And Non-goals

Converge build selection, profile construction, machine-owned resources and
obsolete personality retirement. Keep App/Common/Lib/x86 single-path.
No VDM, profile plugin framework, new scheduler or shared-corpus fork.
No automatic deletion based on chip vendor/name. Missing Standard-specific
non-Intel hardware requires explicit selection approval. PC110 is an approved
exception to the standard-Intel component restriction.

This structural task cannot claim a new CPU/chip implementation from a wrapper.
A component missing complete function/timing support receives its own bounded
implementation proposal and ledger before product qualification.

## Observed Simplification Set

- `app/config.c`: move machine-name parsing and board-specific CPU, ROM count,
  CMOS, floppy and disk restrictions out of App. Keep one syntax/path parser;
  the selected profile owns hardware constraints. Retain CPU tables at Core,
  not a second reduced App-only model authority.
- `core/profile/profile_resolver.*`: replace recursive parents, owned/provided
  masks and copied per-field owner strings with direct construction of one
  immutable plan. Keep real port/window/route validation; removing inheritance
  must not remove conflict detection.
- `core/machine/machine.c`: replace separate Model-40, XT and PC/AT asset/create
  branches with one prepare/validate/create/bind/reset/publish path and cleanup.
- `core/machine/machine_private.h`: remove simultaneous resolved-profile,
  firmware-kind, board-flag and multi-board ROM storage. Keep only selected
  profile resources and Core/Common handles; no replacement mega-union.
- `core/machine/model40_composition.c` and firmware selection in `lifecycle.c`:
  retire unselected board construction and reset/provider switches together.
  Profile-local facts populate the same generic plan; no per-board runner.
- `core/core`: inventory Compaq CECG/WD/D4, DeskPro FDC and XT-specific paths;
  retain only mechanisms required by the approved device set or preserved CPUs.
  Separate reusable Intel PPI/chip semantics from XT keyboard glue before
  retirement. Generic real-mode/VM86, DMA, A20 and transaction behavior survives.
- Media adapters remain only where they add geometry/change/lifecycle semantics.
  Do not recreate Lib Storage or blindly merge FDD/HDD. Reuse existing Common
  queues, paused-debug lease and Lib presenters instead of new wrappers.
- Build/test matrices factor into retained CPU conformance, selected-device
  conformance and two product integration matrices. Preserve behavior coverage;
  do not keep meaningless Cartesian products of unsupported machine/CPU choices.

These are inspected opportunities, not a claim that code has already changed
or that every candidate file is dead.

## Convergence Ledger

Before deletion, list every current profile, board personality, configuration
field, constructor/reset branch, ROM slot, test and build entry. Give each one
exactly one receiver: retained CPU/shared mechanism, Standard, PC110, retired
product-only behavior, or a blocked hardware prerequisite. Record caller proof,
source evidence and regression owner. Unknown dependencies block deletion.
Preserve old successful input/evidence records externally; old tests are retired
only with explicit coverage disposition, never converted from failures to passes.

## Proposed S Batches

- **S1 - evidence and deletion map.** Freeze Standard board/revision/CPU/chip
  list, video and storage choices, firmware byte layout and CMOS seed semantics.
  Compare Intel Model 302 and AMI 386XT Series-4 evidence; neither is currently
  fully admitted. Inventory PC110 prerequisites and every retention/deletion
  row. Stop board-specific implementation if firmware/hardware identity is
  unresolved; do not silently pick a synthetic PC.
- **S2 - build-fixed selection.** One parameterized target recipe, exactly one
  selected profile implementation per EXE, common source lists and checked
  product identity. YAML cannot select another board. Preserve Core CPU tables
  and tests. Update artifact/status validation from the current single-target
  predicate to the admitted fixed-product set as part of this cutover, not by
  weakening today's gate. PC110 readiness is explicit, never a fake bootable target.
- **S3 - flat plan and unified construction.** Remove profile inheritance and
  duplicated board policy; unify external asset lifetime, initialization,
  rollback and reset. Publish one live instance only after validation.
- **S4 - selected-device and legacy-path retirement.** Execute the complete
  dependency map, including old App fields, Core personalities and tests.
  Keep real shared mechanisms; no stubs or disabled obsolete production paths.
  New-chip implementation is not concealed inside this cleanup batch.
- **S5 - whole cutover audit.** Prove one factory/reset/ROM/media/display path,
  no alternate product selector, retained CPU coverage and exact shared-corpus
  identity. Reconcile all old integration rows and run required complete suites.
  Build both host architectures for each genuinely implemented product;
  transfer PC110 implementation to its explicit queue receiver.

## Exit And Stop Conditions

All structural ledger rows have direct proof or an explicit hardware receiver.
No missing Standard identity, firmware or required hardware can be transferred
away while claiming Standard usability. Complete unit/integration closure,
dual Release artifacts, actual-diff review and code-size accounting follow
Execution. Do not close both products on the strength of the structural task:
PC110 hardware and qualification remain separate required work.

Never remove a retained CPU or its tests because it is unused by a product.
Do not modify user output YAML, copy protected assets into the repository,
weaken boot predicates or invent timing evidence. An unresolvable board/ROM
identity requires owner selection, not guessed firmware.
