# Independent Shared Chip Components

## Goal And Admission

First of three ordered NXVM migration candidates requested by the owner.
Unnumbered and not admitted for implementation. Extract the implemented chip
mechanisms from `src/app-nxvm/devices` into `src/x86/devices`, leaving NXVM
responsible for board composition and product adaptation rather than private
copies of shared chips.

## Boundary And Dependencies

- Each chip owns its registers, internal state, reset, functional behavior and
  internal timing. Its public boundary is opaque handles, copied values and
  bounded operations/signals, not another chip's private object.
- CPU owns instruction execution, exceptions and CPU address translation;
  board address decode, ROM/RAM maps, interrupt wiring and clock relationships
  remain composition responsibilities. CPU must not directly scan PIC objects.
- Board-provided memory/I/O cycles, interrupt acknowledgement and wait inputs
  connect the chip to its environment. Shared chips have no App dependency,
  host thread, wall-clock pacing, BIOS-specific response or machine-name branch.
- Shared CPU work extends the existing x86 package, not Common/Lib. Chip code
  depends only on declared neutral contracts and Types, not Common executors
  or the x86 Debug frontend. App assembly connects independently owned chips.
- Extract real existing Intel and related PC chip mechanisms where their
  identity and boundaries are established. Do not move the directory wholesale:
  classify every file, including memory, transactions, firmware, video, storage
  personalities, diagnostics and the machine scheduler. Non-chip mechanisms
  remain with their current board/adapter owner pending the second candidate.

## Coverage And Work Strategy

The first admitted S freezes a migration ledger covering every tracked source
file in the current devices subtree, its callers and tests. Each entry names
the chip or board responsibility, dependencies, destination and proof. Every
entry must finish as migrated or explicitly retained with a non-chip reason;
an unidentified or still product-coupled shared chip blocks completion.

Subsequent S batches follow dependency closures, not speculative preallocated
S numbers. Move coherent implementations and tests, reconnect NXVM, and remove
the old production copy in each completed batch. Preserve original cohesive
instruction-handler style and all CPU-family semantics/timing evidence.
No universal device framework, plugin registry or forwarding-only wrapper.

## Verification And Exit

- All extracted chips build/test without app-nxvm headers, source or assets;
  source-boundary checks prevent reverse and private cross-chip dependencies.
- Chip tests move to `test/x86/devices` under its independent test build;
  board and App tests stay product-owned. No lost test scenarios or fixtures
  dependent on external ROMs in unit tests.
- One chip state and implementation path remain; all current CPU families,
  controller personalities and four fixed machine capabilities are preserved.
- Complete required unit suites per S, integration at T closure, manifests,
  static/governance checks and all affected x64/x86 receiving artifacts pass
  under the execution rules. Boot success does not upgrade timing grades.
- A final ledger/diff review proves every devices-file disposition and leaves
  the board-integration candidate a precise input inventory.

## Scope Controls

Source work requires approved Shared and NXVM packets, separate target commits,
and review of every receiving App; MyNES is changed only if explicitly needed
and admitted. This candidate does not add V30, Z80, Raiden II or PC110 support,
split Apps or move protected assets. It may not silently retire hardware or
degrade timing. Stop for new licensing, public-contract or product-behavior
decisions outside the admitted batch.

Next: [shared IBM PC integration](m5-shared-ibmpc-integration.md).
