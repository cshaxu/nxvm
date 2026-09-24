# M6 Snapshot State

## Outcome

Deliver a MyNes-specific, portable state image and the SoftPC-shaped Console workflow that saves a running or paused cartridge, and loads it only from a stopped machine. The saved result is paused; a load is validated as a complete candidate before it replaces live Core state and is also paused. Neither operation opens a Window while the machine is not running.

## Ownership

App owns command parsing, exact messages, paths, file open/close, atomic output replacement and Common lifecycle requests. Core owns capture, encode, bounded decode, cartridge-reference validation and restoration of deterministic NES state. Lib remains the only file/byte-stream provider; Common remains the only executor/lifecycle owner and neither learns this format.

The format mirrors SoftPC's approach of a product-private image with bounded components rather than copying the x86 image layout. It is a MyNes format:

| Order | Field | Rule |
| --- | --- | --- |
| 1 | magic + format version | Fixed little-endian identifier; any other version is rejected. |
| 2 | cartridge descriptor | Exact supported iNES profile, normalized immutable image length and content identity. The snapshot refers to an already available matching cartridge path; it does not contain ROM bytes or redistribute ROM content. |
| 3 | section directory | Fixed section kinds, little-endian byte counts and a total image bound. Unknown, duplicate, absent-required or oversized sections are rejected. |
| 4 | machine section | CPU/bus RAM, execution/interrupt/DMA state, counters and debug-safe observation facts. |
| 5 | device section | PPU, APU and controller mutable state, excluding host audio FIFO/output state. |
| 6 | cartridge-mutable section | Mapper registers, PRG-RAM/CHR-RAM and battery dirty state. |

Every decoded scalar has range validation. A section reader may consume exactly
its declared length; all declared bytes must be consumed, and EOF/trailing data
is rejected. The image is intentionally not a raw structure dump, contains no
pointers/native handles/host timestamps, and has no compatibility promise for
unversioned pre-M6 files.

## UX And Lifecycle

The Console additions use SoftPC vocabulary and ordering:

```text
save <file>    save a running or paused machine
load <file>    load a snapshot while stopped
```

`save` begins through the Common executor. A running capture waits for a safe
Core boundary and publishes no half-state; it then completes as paused. A paused
capture does not resume. App writes a temporary destination and atomically
replaces the named file only after Core encoding and file close succeed. `load`
opens and fully decodes/validates a candidate while stopped, including requiring
a currently inserted cartridge whose normalized identity matches the descriptor.
Only then does Core replace its state and App routes the completed state as
paused. This avoids recording ROM paths in the image and follows the existing
cartridge insertion model. A missing/mismatched cartridge, malformed image or
I/O failure leaves the stopped/loaded state unchanged.

`save` rejects stopped machines; `load` rejects running and paused machines;
all path errors use `Usage: save <file>` or `Usage: load <file>`. Success and
failure text follow the product's blank-line prompt convention. Existing Window
suppression remains in force until `start` or `resume` actually runs.

## Acceptance

1. Unit tests round-trip every owned Core state section, validate canonical
   scalar/range constraints and prove malformed/truncated/unknown/duplicate/
   trailing input does not alter a destination image.
2. Driver/App tests prove state legality, exact help/usage/outcome wording,
   atomic save replacement and no Window creation on stopped/paused load.
3. Integration tests save a deterministic controlled cartridge in one process,
   load it into a matching stopped cartridge in a fresh process, then compare
   architectural checkpoint/frame/input results on x64 and x86.
4. Failure tests prove a write failure preserves the previous snapshot file and
   a load failure preserves the pre-load Core and Common state.

## Execution Sequence

1. **Schema and transaction contract.** Freeze the section inventory, scalar
   validation, cartridge-match rule, lifecycle legality, Console text and
   failure boundaries in the project authorities. Outcome: the next source
   change has one unambiguous portable-image contract.
2. **Core image.** Add owned capture/encode/decode/restore APIs and deterministic
   unit proof for the complete supported machine. Outcome: Core can make and
   validate a candidate image without paths, Common or host state.
3. **Driver and App workflow.** Connect Common's existing state stream to Core,
   implement `save`/`load`, atomic replacement, all state transitions and
   Window suppression. Outcome: a user can make and restore a state through
   the cooked Console without a second lifecycle authority.
4. **Cross-process acceptance.** Exercise matching/mismatched cartridges,
   malformed files, write failure and x64/x86 state continuation. Outcome: the
   task closes only when saved state has reproducible behavior and every
   rejected input preserves the documented live state.

## Exclusions

No save slots, automatic saves, rewind/replay, compression, netplay, browser/UI
menus, mapper expansion, Common/Lib snapshot ABI, ROM embedding or compatibility
with unknown/experimental old files.
