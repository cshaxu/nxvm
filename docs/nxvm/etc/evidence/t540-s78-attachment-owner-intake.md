# T540 S78 Attachment Ownership Intake

## Baseline And Scope

Accepted S77 P2 is `e0c2b45fc`. This is a documentation-only prerequisite,
not attachment implementation or physical relocation. S77's runtime proof
and eight 0540 products remain unchanged. The executor confirms the S78
packet in CURRENT; no Shared, MyNES, source, test, INI or artifact change is
authorized in this receiver.

## Measured Remaining Surface

Tracked NXVM `.c`, `.h` and `.cmake` files were counted by literal regex
matches, not line counts. `->board\b` has 1,331 occurrences in 110 files:

| Production owner | Occurrences |
| --- | ---: |
| `machine_board.c` | 363 |
| `board_advance.c` | 44 |
| `board_deadline.c` | 21 |
| `d4_memory.c` | 14 |
| `machine_display.c` | 6 |
| `machine_plan.c` | 3 |
| Total | 451 |

The other matches are 856 in 97 test files and 24 in seven CMake files.
Gate literals are not runtime dependencies. Board-header inclusion appears
107 times in 105 files (six production, 99 test); private Core-header
inclusion appears 135 times in 134 files (19 production, 115 test).
These test sets overlap: they are not counts of tests eligible for Shared.

Callback-slot/owner matches occupy five production files, three tests and
nine CMake files. Production consumers are `machine_board.c`, `machine.c`,
`machine_firmware.c`, `machine_scheduler.c` and `cpu_bus.c`. The three
fixtures are controller authority, scheduler and D4 refresh-hold smoke.

## Complete Callback Contract

The private Core layout currently has nineteen callbacks and `board_owner`.
Fourteen scheduler/signal callbacks comprise deadline, refresh request and
completion, DMA ticks/request/advance, PIT ticks/PIC advance, PIC pending and
acknowledge, shutdown-reset qualification, media, RTC and peripherals.
Five additional callbacks are reset devices, reset clocks, refresh NMI,
finalize devices and firmware completion. This is larger than S59's original
scheduler binding scope; S59 established lifetime safety, not a public
attachment publication boundary.

Thirteen callbacks already accept `void *owner`. Shutdown qualification
accepts `const core_machine *`; the four phases and firmware completion
accept `core_machine *`. Constructor code assigns each slot separately.
That private publication, not the chip algorithm, is the next complete cut.

## Construction And Destruction Findings

The composition creates neutral Core, allocates one board state, installs
phase/firmware callbacks before fallible clock setup, and then installs
scheduler callbacks. Allocation/setup failures destroy the unpublished Core.
Preserve this cleanup coverage when replacing field-by-field installation.

Core destruction clears firmware context/provider, calls the board finalizer,
then releases CPU/FPU, routes, ROM mappings, RAM and Core storage. The board
finalizer destroys its chips and releases its single state allocation.
Remaining route destruction does not dispatch callbacks; a second revoke
pass is unnecessary. The NXVM driver destroys Core before its display
provider, media registry and plan. No new lifecycle worker or queue is needed.

Firmware completion belongs inside the existing Core publication/rollback
transaction. A board callback failure must keep the same rollback semantics;
it must not create another firmware path.

## Approved Target Boundary

- Board composition owns an opaque board handle backed by the existing
  single board-state allocation. It may retain an opaque Core execution
  handle for bounded public operations; it never reads Core's layout.
- Core owns guest execution/time/routes and one copied attachment binding,
  containing callbacks plus opaque context. It does not own chip fields or
  a named board-layout pointer in the final boundary.
- Publish the complete binding once during construction, before freeze and
  before any fallible operation requiring attachment cleanup. Reject invalid
  or duplicate publication without changing the current binding.
- Callback context belongs to the attachment and stays valid until the
  existing Core destruction callback releases it. Ownership transfers only
  through successful construction; failure has one cleanup owner.
- Board-facing APIs consume the opaque board handle. Execution/debug APIs
  consume the opaque Core handle. Composition relates the two; no global
  registry, raw-state getter or second copy of chip state is introduced.

This is a target, not a declaration that these APIs already exist. A
`get_board_context()` accessor returning the existing private pointer would
only conceal the same dependency and is explicitly rejected.

## Next Measured Implementation Receiver

S79 shall replace all nineteen private callback slots and their owner with
one public copied binding, reconnect all five production consumers, and
publish it once from the board constructor. Phase, firmware and shutdown
callbacks must use the explicit attachment context too. No parallel old
slots remain after this receiver.

The controller-authority fixture currently replaces four phase callbacks;
the scheduler fixture substitutes thirteen callbacks and the owner while
retaining production phase callbacks. Their context identities cannot be
silently merged. Migrate each fixture as a complete binding with explicit
valid context; preferably use neutral Core for synthetic scheduler probes.
Do not add runtime rebinding or extra production state solely for fixtures.
The D4 fixture observes real callbacks and remains board integration.

S79's guard/test sweep covers the three fixtures and nine callback-related
gates, including rejected publication and partial-construction cleanup.
Its packet must require complete dual-width units, independent Core linkage,
one boot per existing eight profile/width rows, and affected dual products.

The subsequent board-handle cut receives all six production owners above and
classifies the 97 direct-board test files by actual responsibility. It must
not move machine-specific composition tests into Shared. Only after both
cuts may the measured neutral Core and flat IBM-PC families physically move.
Each later batch receives its own linear numeric S packet; S78 does not
invent a completed API, retire tests, or close T540.

## Delivery Verification

Documentation links, scoped diff and unchanged S77 executable identities are
checked for this delivery. No new runtime assertion or executable build is
claimed for a design-only change. Document review also found and corrected
two pre-existing relative proposal links in the retained T540 proposal.
The retained S77 full unit and boot results
describe that baseline only, not unimplemented attachment behavior.
