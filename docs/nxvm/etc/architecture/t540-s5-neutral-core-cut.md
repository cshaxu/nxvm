# M5 T540 S5 Neutral Core Cut

This is the source-inspected receiver map for the neutral `x86/core` work. It
supersedes only S2's prospective statement that Core must wait until after
T540: the owner subsequently required Core and the IBM-PC layers in this T.
The S2 observations about existing owners remain valid. No source or ABI
changes occur in S5.

## Current coupling that forbids a whole-directory move

- `machine.h` defines one `core_machine`, but it embeds both the CPU, memory,
  port, transaction and guest time state **and** PIC/PIT/DMA/RTC/KBC/FDC/HDC,
  video, XT keyboard, Port-B, speaker and Model-40 D4 state. Moving this
  structure unchanged would make the neutral Core own PC and Compaq boards.
- `machine_scheduler.c` computes one guest deadline but directly polls PIT,
  RTC, DMA, FDC, HDC, KBC, FPU and D4. Keep the sole time axis; split the
  neutral advance/deadline reduction from the actual board-device sources.
- `machine_plan.c` has one ordered plan application and rollback, yet its
  steps configure display, DMA, RTC, FDC, HDC, planar parity and D4. The
  transaction must not be cloned into XT/AT plans. Board attachment ordering
  stays at the composition boundary; neutral Core only owns its own frozen
  execution/memory/entry state.
- `cpu_bus.c` performs generic CPU memory/port transactions but also scans
  and acknowledges the IBM-PC PIC pair. A PIC acknowledgement is a board
  service supplied at the Core boundary, not a reason for Core to include
  `pic_bus.h`.
- `memory.c` is a checked physical-memory router except for its fixed 92h
  A20 port callbacks and registration. That port is a board route currently
  installed by `machine.c`; its current behavior must be preserved during
  extraction. Hardware applicability is a later board qualification, not an
  implicit behavior change in the file move.

## Finite receiver allocation

| S | Receiver | Exact current family and disposition |
| --- | --- | --- |
| S6 | `x86/core` pure mechanism target | Move `timeline.c/h`, `transaction.c/h` and `clock.c/h`. Move the clock-ratio value declaration out of the mixed machine interface without adding an alias. Keep board integration tests in NXVM; add or move only genuinely mechanism-local tests to `test/x86/core`. |
| S7 | `x86/core` checked bus storage | Move `port.c/h`, `memory.c/h` and their neutral public operations. First extract the 92h route to its IBM-PC board owner; do not copy it or leave a forwarding function. ROM/memory provider and failure semantics remain one path. |
| S8 | `x86/core` execution lifetime | Split the mixed `machine.h`, `machine.c`, `cpu_bus.c`, `machine_scheduler.c` and related entry/observation files by state owner. Core keeps one CPU/execution lifetime, guest time and generic retirement/interrupt contract. IBM-PC board retains the actual chip fields, device deadlines, PIC acknowledge and construction order. `machine_plan.c` remains the sole board-attachment transaction until its generic operations can be moved without creating a second plan. Reconnect NXVM through one public opaque Core interface and remove old source definitions. If this batch proves too large at intake, split its unstarted scope into subsequent linear S numbers before editing rather than hiding a partial cutover. |
| S9 onward | `x86/ibmpc-*` | Move only the board mechanisms proven by the S2 adapter ledger; machine-specific firmware, D4, media and fixed topology stay in NXVM composition. |

The remaining interface/observation files have these receivers once S8 splits
the machine layout:

| Current files | Intended owner | Boundary reason |
| --- | --- | --- |
| `port_interface.c/h`, `memory_interface.c/h`, `entry_plan_interface.c/h`, `rom_mapping_interface.c/h` | `x86/core` | Generic checked port/memory/entry and immutable ROM operations; the PC F0000h reset-alias selection is not part of the ROM provider mechanism. |
| `trace_interface.c/h`, `retirement_observation_interface.c/h`, `debug.c`/`debug_interface.h` | `x86/core` | CPU execution diagnostics and copied observations, not product CLI or a second guest executor. |
| `display.c`/`display_interface.h`, `guest_display_frame.h` | `x86/core` | Copied video observation boundary; actual video state remains in `x86/chips/video`, and display ports/apertures remain board wiring. |
| `machine_firmware.c`, `firmware_interface.h` | Split at function boundary in S8 | Generic bounded firmware invocation belongs to Core; PC F0000h source selection and reset-alias construction remain with board composition. |
| `media_interface.c/h` | IBM-PC FDC/HDC board attachment | It binds media IDs and sector providers for current PC storage controllers, not CPU execution. Move with the first proven FDC/HDC board receiver, without moving product file leases. |

These are intended owners, not permission to move a mixed file unchanged.
The S8 call graph decides the minimal exact source split; it may not introduce
a forwarding facade or a second reset/plan path.

## Invariants and proof

- There is still one Core machine lifetime and one guest clock. The eventual
  IBM-PC board attachment holds only board-owned state; no chip registers or
  elapsed ticks are mirrored in Core or the App.
- Shared `x86/core` includes only Lib types and declared chip/core public
  contracts. It never includes `app-nxvm`, `ibmpc-*`, product profiles,
  firmware assets or Common session. IBM-PC board code may depend on Core,
  never the reverse.
- Derive the smallest private board binding from actual scheduler/bus/reset
  call sites in S8. Do not add a generic device framework, second event
  queue, alternate executor, forwarding header or copied machine structure.
- S6 and S7 each require independently compiled Shared tests, both-width full
  repository-only units, manifest/static gates and all affected product
  artifacts. S8 additionally compares all four profile construction/reset,
  CPU interrupt and halted-deadline regressions. T540 closes only after its
  full external integration suite and final sole-owner audit.

## S6 Intake Correction

The first proposed S6 move is not safe as a separate Shared target:
`core_machine_timeline` exposes its event heap and
`core_machine_transaction_state` exposes mutable transaction counters because
both are presently embedded in the one private `core_machine`. Moving only
their `.c/.h` files would either export those private layouts across components
or add an opaque allocation/forwarding layer solely for the staged move.
Neither improves the final architecture. Their sole owner remains the neutral
Core instance, so they move with that instance after the board fields are
separated.

S6 therefore removes the fixed 92h A20 port from `memory.c` and installs it
once from the existing IBM-PC board owner, preserving the current route on all
four profiles. This is the first actual source cut needed before neutral memory
can move. S7/S8 intake must reallocate the remaining Core source batches
according to private-state ownership; the S5 table is historical intent, not
permission to publish private structs or create a temporary facade.

The [S7 handoff](t540-s7-core-board-handoff.md) is that reallocation. It also
corrects the prospective destination of the mixed display observation: the
VADP-facing capture remains board-facing, while Core retains only neutral
execution/observation state.
