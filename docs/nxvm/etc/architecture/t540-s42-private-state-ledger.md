# M5 T540 S42 Private Core/Board Ownership Ledger

This source-only intake is based on the post-S41 `machine.c`, `machine.h`,
`machine_board.c`, `machine_plan.c`, `machine_scheduler.c`, `cpu_bus.c`,
`board_advance.c` and `board_deadline.c`. It does not move code, change a
runtime contract or assert that the mixed header is ready for Shared. The
existing `core_machine` allocation, `core_machine_create`, cold reset, run,
and destroy remain the only production paths.

## Existing facts and intended sole owners

| State or behavior in the present private header | Present users and final owner | Receiver |
| --- | --- | --- |
| `lifecycle`, `stop_requested`, `fault_detail`, `elapsed_ticks`, `timeline`, `time_axis`, L1 policy, provider clock, execution provider and freeze state | `machine.c`, scheduler, public observation; neutral Core owns execution and guest time | S54 |
| CPU profile/quirk, FPU, opaque CPU execution context, maximum instruction time, retirement contract/qualification, CPU diagnostic and retirement observation | create/run/CPU bus and CPU observation; neutral Core owns the single CPU execution lifetime | S54 |
| Transaction state/contract, external-cycle page/overlap/pending fields, CPU retirement waits and source ticks, CPU/DMA ready/wait arbitration fields | `cpu_bus.c`, `machine_scheduler.c`, `machine.c`; neutral Core owns the single bus/retirement scheduler, with board signals delivered through callbacks | S54 |
| `executor_memory`, `executor_port`, immutable ROM mapping table/count, entry-plan marker, Core trace and firmware operation guard/context | memory/port/ROM/entry/trace/firmware implementations; neutral Core owns routes and operation bounds | S54 |
| `timing_plan` and `timing_plan_copied` | `machine_plan.c` assembles and publishes a frozen plan; Core consumes declarations/time rules. Keep one copied plan, not Core and board mirrors; split its board topology at application, not by copying the plan twice | S53 |
| Six named device clocks and KBC timing/configuration, keyboard/display topology, DMA wiring/bindings, RTC/CMOS and FDC/HDC topology/configured flags | board advance/deadline, plan, controller adapters; one board attachment owns device timing and topology | S44 |
| PIC pair and PIT0/RTC PIC IRQ source bindings | board constructor, PIC advance/deadline and finalization; one board attachment owns PIC lifetime | S45 |
| PIT pair and auxiliary-PIT configured state | board constructor, PIT advance/deadline and finalization; one board attachment owns PIT lifetime | S46 |
| DMA latch and primary/secondary controllers | board constructor, DMA effects and finalization; one board attachment owns DMA lifetime | S47 |
| RTC chip and selected register | board constructor, RTC I/O/deadline and finalization; one board attachment owns RTC lifetime | S48 |
| FDC and HDC controller instances | board constructor, controller adapters and finalization; separate bounded board batches | S49–S50 |
| KBC, XT PPI and XT keyboard instances | board constructor, input adapters and finalization; one keyboard owner group | S51 |
| VADP instance | board constructor, display adapters and finalization | S52 |
| D4 memory/platform, absent-memory windows, planar parity, XT speaker, D4 refresh latches and DMA refresh wiring | board memory, port, NMI, refresh and speaker adapters; one board attachment owns machine-specific electrical state | S53 |
| Firmware provider/context binding and F0000h alias choice | `machine_board.c` owns board role/alias and binding; Core retains the single bounded firmware invocation and ROM table. Do not duplicate either side | S53 |
| `board_*_provider` callback slots and `board_owner` | Core scheduler/CPU bus call bounded board operations; board installs/revokes them with its lifetime. The shutdown callback currently takes `const core_machine *` because test schedulers replace `board_owner` with a probe, so do not silently change that ABI | S53–S54 |

These rows cover all contiguous state groups in the current `struct
core_machine`, including the appended scheduler state. `core_machine_plan`
likewise contains one frozen configuration, topology, declarations and
controller timing plus board D4/media/display/FDC inputs; the split must
retain one success-only publication and one topology rollback. The private
firmware context remains an operation guard, not a second machine owner.

## Mixed source functions and direct consumers

- `machine.c` lines 22–68 contain XT/KBC/PPI board callbacks. Lines 345–361
  validate board topology; lines 484–661 create/register/wire the board.
  S43 moves these implementations to the existing board owner and deletes
  their old definitions. No second constructor or registration transaction.
- `machine.c` lines 87–343, 362–483 and 663–747 resolve/validate/create
  the neutral CPU/time/memory/port stage and coordinate exactly one board
  create; lines 748–1350 own reset/run/deadline/lifecycle/fault/destruction.
  They remain Core candidates, but their board callback and mixed-field
  access must be resolved before physical relocation.
- `machine_plan.c` owns frozen board assembly; `board_advance.c` and
  `board_deadline.c` consume named device clocks/chips. `cpu_bus.c` and
  `machine_scheduler.c` consume board callbacks, not named chip fields.
- Eighteen production `.c` files in `src/app-nxvm/devices` and 114 test
  files under `test/app-nxvm` directly include this mixed private header.
  This count includes fixture headers and is a migration inventory, not a
  claim that all 114 tests should become Shared. Board fixtures and profile
  tests stay NXVM; only tests that compile against the neutral private
  contract alone can move with Core.

## Linear receiving plan

1. **S43** — Move remaining XT/KBC/PPI callbacks, board-config validation
   and board creation out of `machine.c`. Preserve one public create, one
   candidate allocation, port checkpoint, chip order and destruction route.
2. **S44** — Move device clocks, topology/configuration and timing fields to
   one board attachment. Repoint only their real readers/writers; preserve
   one frozen input plan and unchanged device phases. No duplicate fields.
3. **S45–S52** — Move board chip instances in actual owner groups: PIC and
   its IRQ source bindings (S45), PIT (S46), DMA (S47), RTC (S48), FDC
   (S49), HDC (S50), KBC/XT keyboard (S51), VADP (S52). Each group retains
   its existing constructor, reset, deadline, ports, IRQ wiring, teardown
   and tests; no group adds a second chip lifetime.
4. **S53** — Move D4/absent-memory/parity/speaker/refresh latches and board
   callback installation/revocation. Keep the Core operation guard and the
   board's F0000h choice on opposite sides of one bounded interface. Audit
   the copied plan's topology application against the sole rollback.
5. **S54** — Finish the neutral private header: no named PC chip, topology,
   D4, speaker or profile include. Keep only Core state and the bounded
   board attachment/callback contract; migrate remaining direct test
   consumers by actual dependency, not filename. Compile Core without the
   NXVM board target before claiming the split.
6. **S55** — Physically move only the proven neutral implementation and
   neutral test receivers to `src/x86/core` and `test/x86/core`, delete the
   old App copies, reconnect NXVM, verify independent Shared build/tests,
   both product widths and all four fixed-profile boots. S56 onward owns
   the separately audited `ibmpc-common`, `ibmpc-at` and `ibmpc-xt` moves.

The S37 prospective “S43 physical move” is superseded by this measured
split; it was never executed. The S45 source inventory finds PIC in three
production and fifty direct test files, PIT in four/eight, DMA in three/
thirteen, RTC in three/ten, FDC/HDC together in six/thirty-one, keyboard
in three/eleven and VADP in three/two. That exceeds one bounded S45 owner
move, so S45–S52 are assigned by chip group before implementation. If a
later actual diff still exceeds one safe owner boundary, create further
**numeric, linear** receivers and shift S53–S55
forward before implementation. Do not use S suffixes or leave a mixed state
group without a receiver. This intake changes no source, test, asset, INI,
binary or Shared/MyNES file.
