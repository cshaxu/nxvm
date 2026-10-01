# T540 S2 Board-Adapter Ledger

This ledger is the finite S2 design universe.  It records a destination only
after inspecting state ownership, all consumers, reset/finalize ordering and
dependencies.  `Candidate` is deliberately not a permission to move source.

## First-Principles Boundary

`app-nxvm/devices` currently contains two different layers:

1. a generic x86 machine executor -- CPU bus, RAM, port space, transaction,
   guest clock/timeline, scheduler, immutable ROM mapping, reset and plan
   transaction; and
2. IBM-PC board attachments -- PIC/PIT/DMA port and signal wiring, FDC/HDC,
   8042 or XT PPI keyboard, VADP aperture, Port-B/speaker/refresh and the
   Model-40 D4 board.

Only the second layer belongs below `x86/ibmpc`.  The first is required by any
future non-IBM-PC x86 product, including the planned arcade work; putting it
under `ibmpc` would encode a false dependency.  Before independent PC Apps
can stop including `app-nxvm/devices/machine_interface.h`, that first layer
needs one neutral `src/x86/core/` destination.  This is a required predecessor
to the App split, not permission to move it during T540.

## Adapter Dispositions

| Current family | Sole mutable-state owner | Current consumers | Destination / disposition | Extraction proof and regression owner |
| --- | --- | --- | --- | --- |
| `machine`, `machine_plan`, `machine_scheduler`, `cpu_bus` | One Core instance owns executor state, plan application and guest timeline | Every current profile through `vm_machine` | **Future `x86/core`**, not `ibmpc`; do not move in T540 until the public interface and CMake/static gates cease naming App paths | `core_machine_plan_smoke`, executor/scheduler/CPU-PIC tests, then all four profile smokes |
| `memory`, `port`, `transaction`, `clock`, `timeline` | One Core instance owns memory mappings, port registrations and timing/transaction state | Every current profile and board adapter | **Future `x86/core`**, not `ibmpc` | Existing Core memory/port/transaction/time tests; no profile-specific rewrite |
| entry/lifecycle/ROM/debug/trace/retirement/display-provider interfaces | Core plan or Core instance owns the corresponding state; App only binds neutral inputs | Every current profile or host adapter | **Future `x86/core`**; retain product debug/asset adapters outside it | Existing Core contract tests and App machine adapter tests |
| `pic_bus` | Board attachment owns PIC port routes, cascade link and IRQ-source aggregation; `x86/pic8259` owns PIC registers | XT single PIC; 5170/default/Model40 cascaded PIC | **`x86/ibmpc/common` candidate**; topology is a copied construction value, not a model switch | Prove one initialize/reset/finalize transaction works for one and two PICs; retain XT and AT/Model40 interrupt regressions |
| `pit_bus` | Board attachment owns port registration; `x86/pit825x` owns counters/waveforms | XT 8253; 5170/default 8254; Model40 system and auxiliary PIT | **`x86/ibmpc/common` candidate**; its current personality/base-port inputs already express the differences | Prove port registration rollback and destroy order for system and auxiliary PIT; retain PIT/auxiliary-PIT tests |
| `dma_bus` | One Core-owned primary/optional-secondary DMA pair and page latches | XT primary-only; AT/Model40 cascaded pair | **`x86/ibmpc/common` candidate, unsplit initially**.  The current controller-count input is a hardware topology value, not a product name. | Compare the primary-only and paired reset/finalize/page-latch paths; do not clone an XT DMA implementation. Retain DMA/FDC and XT/Xebec regressions. |
| `fdc` | Board attachment owns PC register ports, FDC media binding, IRQ/DRQ bridge and terminal observation; `x86/fdc8272` owns command state | XT, 5170/default and Model40 | **`x86/ibmpc/common` candidate** if all consumers use the existing copied `core_machine_fdc_config` / drive bindings | Prove no profile/asset/media-policy include is needed; retain XT, 5170/default and Model40 FDC tests. |
| `hdc` | Board attachment owns media IDs, port routes and IRQ/DRQ bridge; `x86/hdc` owns ATA, WD and Xebec protocol personalities | XT Xebec; 5170 WD1003; Default ATA; Model40 Compaq WD | **`x86/ibmpc/common` candidate** only as a protocol-neutral attachment. Personality/geometry/port selection remains composition data. | Prove the adapter has no protocol-specific branch outside its copied config; retain Xebec, ATA, 5170 and Model40 HDC tests. |
| `vadp` | VADP attachment owns one guest port/aperture mapping and copied-frame boundary; `x86/video` owns video state | XT CGA; 5170 CGA; Default EGA/CGA; Model40 Compaq EGA | **`x86/ibmpc/common` candidate** only for the single owner path; no second CGA/EGA/VGA route | Retain CGA/EGA/Compaq aperture and frame regressions; stop if a proposed helper needs profile/ROM policy. |
| `kbc` | 8042 attachment owns port routes, IRQ1/IRQ12 bridge and Core-memory/A20 link; chips own controller/keyboard/mouse state | 5170, Default, Model40 | **`x86/ibmpc/at` candidate**.  XT does not use this topology. | Compare 5170/default/Model40 reset, AUX absence and input-port configuration; Model40 joins only where identical. Retain 5170 POST/KBC and Model40 tests. |
| `xt_ppi_keyboard` | XT board attachment owns PPI ports, DIP/NMI/speaker inputs and IRQ1 bridge | XT only | **`x86/ibmpc/xt` candidate** | Preserve its one reset/NMI/IRQ and PPI port lifetime; retain XT keyboard/profile tests. |
| `machine_board` Port-B/speaker/refresh pieces | Core board state currently owns Port-B latches and PIT output callbacks | XT speaker, AT planar parity/refresh, Model40 D4 refresh/failsafe | **Must split by mechanism before moving.** Shared helper only where output binding and reset lifetime are identical; XT, AT and D4 pieces cannot become one mode-switch object. | Establish separate XT, AT and Model40 source allocations; retain refresh, speaker, NMI and D4 tests. |
| `d4_memory` | Model-40 D4 state/mapping and failure behavior | Model40 only | **Retain Model40 composition/Core extension**; no `ibmpc/common` consumer exists | Model40 D4 map/parity/failsafe regressions. A later DeskPro App owns its move. |
| `controller_interface` and neutral copied wiring types | No runtime state; defines the DMA/PIT contracts used by attachments | DMA/FDC/HDC/board code | **Move with the first receiving neutral board mechanism**, not as a standalone facade | One receiving public interface, no duplicate aliases; static boundary gate updated with the move. |

## Composition Ownership That Does Not Move

The following current profile work remains machine composition even when it
calls a shared adapter:

- XT: 8088/8253 choice, 4.772727 MHz board input, 256 KiB/open-bus map, XT ROM
  roles, 360 KiB drive, Xebec presence and its selected geometry.
- 5170: IBM ROM/CMOS roles, 80286/clock selection, 1.2 MiB drive, WD1003
  personality and IBM-specific display/open-bus values.
- Default: project firmware, selected 386/memory/video/personality values and
  default media policy.
- Model 40: Compaq ROM/CMOS, D4 map, second PIT, EGA/CECG, two-drive behavior,
  Compaq WD-40MB configuration and all distinct timing values.

`profiles/default_profile/pc_at_profile.c` may retain its compact descriptor
to topology construction until a later S removes only the mechanical parts.
It must not be replaced by a Shared descriptor language: that would duplicate
the existing Core plan and make Shared choose product policy.

## Build And Test Migration Rule

Each later source S moves one complete mechanism with its direct tests and
changes `cmake/nxvm/NxvmProduct.cmake` plus any static gates that name its old
path.  It adds `test/x86/ibmpc` only for an actually public shared mechanism.
The existing `test/app-nxvm/unit/core/machine` profile tests remain the
four-board integration boundary.  No test is deleted merely because a helper
moved; unit coverage remains synthetic and external-ROM/media scenarios remain
in `test/app-nxvm/integration`.
