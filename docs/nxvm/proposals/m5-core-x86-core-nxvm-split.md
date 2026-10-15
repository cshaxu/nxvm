# Split Core Into X86 Core And X86 NXVM

## Admission And Objective

Admitted as M5 T561 on 2026-10-15 after owner-approved T560 staged closure
`453455e59`. The owner inserts S1 before the extraction: import SoftPC's
Lib, Emulator, X86 Debug, X86 IBM PC and X86 NXVM source/test packages
byte-for-byte, including the owner's corrected X86 IBM PC scope.
The original owner request is:
"core -> x86-core + x86-nxvm，其中x86-core可供myraidenii使用", followed by
"回到主工作树，把这个写成proposal加入队列首位".

Make top-level capabilities independently composable: the four NXVM Apps use
X86 Core plus X86 NXVM; MyRaidenII can consume X86 Core without PC hardware or
product policy. Lib and Emulator retain their existing responsibilities.
SoftPC can continue using its own machine implementation with X86 Debug and
X86 IBM PC; it is not required to adopt this execution core. MyNES and MyNeoGeo
must not acquire an x86 dependency merely to use shared host facilities.

The accepted implementation baseline is clean `453455e59`, including frozen
T560 S9 `b906c7138`. The CPU successor retains all remaining C16/C17/final
qualification; this refactor does not resume that work or upgrade its grades.
Earlier planning inspection used `6621f09f0`; it is not the extraction baseline.

## Target Components

```text
src/x86-core/                  test/x86-core/
  cpu/                          cpu/
  fpu/                          fpu/
  machine/                      machine/

src/x86-nxvm/                  test/x86-nxvm/
  chips/<chip>/                 chips/<chip>/
  board-base/                   board-base/
  board-xt/                     board-xt/
  board-at/                     board-at/
  machine/                      machine/
  product/                      product/
```

The three X86 Core subcomponents have these owners:

| Subcomponent | Responsibility | Existing source |
| --- | --- | --- |
| cpu | Instruction semantics, registers, CPU profiles, exceptions, interrupt inputs, CPU timing and bus requests. Future V30 support belongs here, not in game-name branches. | core/chips/cpu |
| fpu | Floating-point implementation and CPU connection. Preserve existing CPU dependency honestly; allow no-FPU machines without inventing a guest FPU. A separately selectable target must be proven by the build graph. | core/chips/fpu |
| machine | Board-neutral CPU composition, guest time, event/deadline execution, run budgets, memory/port routes, bus transactions, lifecycle and debug observation. | Audited core/x86 |

Keep clock, timeline, memory, port, transaction and debug as cohesive internal
files initially; this split does not require a top-level package for each one.
Machine is an x86 execution environment without an installed PC motherboard.

The six X86 NXVM groups have these owners:

| Group | Responsibility |
| --- | --- |
| chips | Existing dma8237, fdc8272, hdc, kbc8042, keyboard, pic8259, pit825x, ppi8255, ps2mouse, rtc146818, video and xtkeyboard. Each leaf retains its own interface, target and tests. |
| board-base | Shared PC wiring, device composition, memory/firmware layout policy, controller bindings and board progression. |
| board-xt | XT-specific wiring and behavior. |
| board-at | AT-specific wiring and behavior. |
| machine | PC-to-Emulator driver, host pacing, input/frame conversion, media, lifecycle reception and X86 Debug adaptation. |
| product | Four-App INI/composition and injected info, speed, media and snapshot commands. Reconcile the preserved x86-nxvm speed/info declarations with their actual implementation owners. |

Chips remaining here are not declared inherently PC-only. Promote an individual
chip later when another real consumer and independent contract justify it.
App identity, fixed model composition and asset declarations remain with each
of the four Apps. Do not replace them with product-name branches in a factory.

## Dependency And Contract Boundaries

```text
four PC Apps -> x86-ibmpc -> x86-debug
four PC Apps -> x86-nxvm -> x86-core
MyRaidenII  -> x86-debug + its own machine/board -> x86-core
```

Packages use Lib and Emulator only where their responsibilities require them.
X86 Core must not depend on X86 NXVM, X86 IBM PC, X86 Debug, an App, BIOS
configuration, a disk image policy or a sibling repository. Emulator owns the
host executor/session lifetime; X86 Core owns guest execution and time. Do not
introduce a second host execution thread or competing guest time authority.

Audit core/x86 rather than renaming the directory wholesale:

| Mechanism | X86 Core owns | Board/product owns |
| --- | --- | --- |
| Reset | Architectural CPU reset state and execution lifecycle | ROM bytes, reset-vector decode and board reset effects |
| Interrupts | CPU signal sampling and architectural delivery | PIC or other source, wiring and event timing |
| Memory and ports | Routes, transactions, read-only mapping mechanism and side-effect-free observation | Physical decode, aliases, unmapped behavior, BIOS layout and PC gate policy |
| Time | Guest time axis, budgets, event ordering and deadline execution | Device deadlines, clock relationships and board-specific progression |
| Debug | CPU state, observation and execution control primitives | Frontend protocol translation and machine-specific interpretation |

The current attachment includes pit_pic, rtc and media-specific conventions.
Move those receiving policies to the PC board side and expose only justified
device/event contracts. Renaming a callback while retaining hidden PC ordering
does not satisfy the split. Audit defaults, A20 routing, minimum RAM allocation,
firmware construction, refresh/DMA, idle advancement and error cleanup. A host
allocation limit is not automatically a guest mapping defect; qualify the
actual guest-visible route before changing behavior.

Preserve one implementation of each mechanism and explicit ownership on
construction failure, reset, stop and destruction. Do not copy CPU sources,
leave permanent old-path aliases, or create a second scheduler to avoid a
difficult interface. Current timing grades and qualification restrictions must
survive the move; structural independence is not new hardware qualification.

## SoftPC Split And Ownership Prerequisite

The read-only SoftPC split reference is owner-accepted `3af19559`, implementation
`8046b085`: x86-debug holds tools, x86-ibmpc holds PC product interaction, and
x86-nxvm temporarily holds NXVM speed/info contracts. Freeze and review the
exact available revision at admission. If NXVM has not received that split,
admit its necessary import/receiver work explicitly before replacing old X86
paths; this proposal is not an instruction to copy sibling files now.

The new full X86 NXVM package absorbs those preserved contracts, not a second
parallel package with the same name. Keep actual speed behavior in its machine
owner and command/config interpretation in product; remove redundant frontend
enum conversions when the receiving contract permits it.

Admission names NXVM and Shared change targets, exact consumers and verification,
with target-separated commits. X86 Core and tests become Shared-owned; X86 NXVM
and tests remain NXVM-family facilities despite their top-level path. Update
architecture, coding, package manifests, ownership/gates and documentation
maps in the admitted work. Sibling repositories remain read-only. MyNES, SoftPC,
MyRaidenII and MyNeoGeo implementation changes are not implicit scope.

## Planned Batches

S1 is admitted for the exact ten-tree import from clean SoftPC
`3af1955932fcb9e61ceec3456493580e316a12c9`. Lib/Emulator's four trees already
match byte-for-byte. Add the three split packages and their independent tests;
retain all upstream bytes and manifests. Necessary registration infrastructure
must not weaken the existing Core or shared tests. X86 IBM PC is explicitly
included by the owner's scope correction.

S1 switches running Apps to the complete imported product/tools split and
retires old src/test/x86. Adapt Core consumers without compatibility aliases
or CPU semantic changes. Canonical packages remain byte-identical; Core owns
the necessary NXVM information/speed and machine-lifetime adapter.
Do not claim the header-only X86 NXVM package already implements the full
planned machine/chip/board package.

The following remaining batches are plans, not allocated S identifiers:

| Batch | Deliverable | Exit evidence |
| --- | --- | --- |
| 1. Inventory and contract freeze | File/symbol/test/target disposition for all Core paths; SoftPC split dependency; consumer DAG; PC-assumption ledger and accepted baseline. | Every source/test has one destination or justified disposition; exact cross-target boundary and invariants. |
| 2. CPU and FPU extraction | Shared cpu/fpu sources, public contracts, independent targets and matching tests; PC receivers updated. | No PC/App includes or private-layout access; both host widths pass moved owner suites and relevant CPU qualification. |
| 3. Neutral machine extraction | Shared execution, routing, time and observation; move PC policies to board receivers. | Non-PC synthetic board proof plus PC timing/transaction regression; one execution/time owner. |
| 4. NXVM package consolidation | chips, board-base/xt/at, machine, product; reconcile speed/info and four-App includes/builds. | Four Apps use the same package; no duplicate old Core/X86 compatibility implementation. |
| 5. Complete acceptance | Final DAG/manifest/test ownership review, full dual-width regression, required four-profile integration and artifacts, current docs. | Actual-diff review, source/test ownership ledger, qualified artifacts and explicit residual limits. |

Choose independently buildable commit boundaries during admission; extraction
and receiver updates must not leave a broken tree. Do not merge architecture
extraction with V30 implementation or game compatibility work.

## Verification And Test Ownership

Mirror source ownership in test/x86-core and test/x86-nxvm. CPU/FPU semantics
and neutral routing/timing belong to their shared owners; PC wiring and product
behavior stay with NXVM. App tests may use real lower layers but assert their
own composition, not duplicate entire instruction or device suites. Record
moved/deleted tests by behavior; deduplication must not silently lose coverage.

Require a ROM-free synthetic non-PC board that uses public X86 Core interfaces
only and links no PC chip/board/product or private test fixture. Prove real CPU
reset fetch through board ROM mapping, RAM/MMIO and unmapped/alias behavior,
interrupt entry, HALT with device advancement, bounded run/pause boundaries,
side-effect-free debug observation and reset/teardown. Use project-authored
program bytes; no game ROM, BIOS shortcut or fake successful device is needed.
This proves independence, not V30/Raiden II support.

Check standalone package builds on x86/x64 hosts, public-header and link DAGs,
corpus manifests, negative dependency gates, owner suites and the accepted
CPU functional/timing qualification matrix. Run full repository units on both
widths and required four-profile integrations for final acceptance; check the
eight PC release artifacts when rebuilt under the admitted release rules.
Preserve owner INI, ROM, media and snapshot inputs and their existing format
contracts. Include MyNES/shared receiving tests where shared changes affect
them; do not claim sibling SoftPC runtime qualification without executing it.

## Scope Limits And Completion

No new V30/Z80/68000, sound chip, arcade video/protection, game ROM loader or
game compatibility is delivered here. No universal multi-CPU framework or
plugin architecture is justified by this extraction. The separate
[Raiden II candidate](myarcade-raiden2.md) consumes the accepted boundary;
its older MyArcade naming and product admission remain its own receiver work.

Stop and re-scope if a public contract requires product/game identifiers,
shared interfaces leak PC storage, correctness needs a second time owner,
an accepted hardware feature/test is being retired, or unrelated work must be
overwritten. Preserve and report source conflicts instead of lowering timing
grades. Completion requires both unchanged qualified PC behavior and a real
non-PC public-interface consumer, with no remaining production dependency on
the retired core roots. The candidate's queue priority is not acceptance of
either the split or the future arcade product.
