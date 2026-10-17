# T1 Freeze Ledger

Universe: the owner-supplied original V30 RAIDEN II US set 1 runtime data,
the inspected hardware/reference sources, and the existing host/CPU integration
boundaries. This ledger qualifies planning inputs, not emulation or gameplay.
Each row has a finite receiving proof; required runtime work remains in Queue.

| ID | Freeze item | Evidence / owner | Disposition after S1 |
| --- | --- | --- | --- |
| F01 | Program identity and lanes | Inventory: two 524288-byte images, matching pinned parent SHA-1 | Proven data identity; Board must test lane composition and reset fetch |
| F02 | Sound program file vs mapped region | Inventory: 65536-byte file, 131072-byte reference region | Proven file identity; Sound must test bank/copy mapping, not pad a fictitious ROM |
| F03 | COPX, characters, backgrounds, sprites, samples | Inventory: 10 further required runtime images, byte counts and two hashes | Proven runtime data identity; not protection/decryption correctness |
| F04 | PAL/JED distinction | Archive has one extra 821-byte JED; reference has two different binary PAL dumps | Extra JED retained outside runtime role list; no claim of a complete MAME PAL audit |
| F05 | CPU primary authority | NEC October 1986 manual, hash, sections 6--12 | Acquired and relevant reset/native/emulation/prefix sections inspected |
| F06 | Sound primary authorities | Zilog UM008011-0816 and Yamaha December 1991 scans pinned; OKI scan located | Z80/Yamaha identity established; OKI full-byte acquisition/inspection remains explicit receiving work |
| F07 | Board/control authority | Original three-page operator manual; pinned MAME board reference | JAMMA/vertical/two-player directions/buttons verified; board maps/timing are L2 leads, not schematics |
| F08 | V30 CPU extension | CPU profile, dispatch/preview, reset and timing audit | Explicit family classification and NEC decode required; no Shared code authorized |
| F09 | CPU execution and clock owner | Public neutral Machine routes/attachments/retirement contract | Reuse neutral Core Machine; V30 physical-time qualification required before board admission |
| F10 | Host input/control/audio | Emulator driver/provider and Lib KVM/PCM contracts | Existing ownership fits; no second worker/provider/parser; no guest sound synthesizer supplied |
| F11 | Full-colour video handoff | Lib 8-bit/256-entry graphics vs board reference 2048 palette entries | Owner-directed High TODO; Lib unchanged; full-colour release cannot be claimed |
| F12 | Acquisition and publication | Owner supplied local ZIP; read-only audit; no downloaded game or ROM commit | Metadata only; no redistribution rights inferred from possession or checksum match |
| F13 | Finite game qualification | Existing seven capability receivers + semantic checkpoints below | Planned/unimplemented; actual stage enumeration and runtime signatures remain to freeze |

F06 and F13 receiving details and the final public-boundary review are not
silently treated as proven. T1 remains open until its required freeze predicate
is reconciled, or the owner explicitly approves a changed task boundary.

## Runtime Coverage Receivers

CPU owner tests cover reset/FLAGS/native decoding, ModRM/operand lengths,
arithmetic/strings/NEC extensions, IRQ/NMI/WAIT/HLT and mode transitions with
register/memory/port effects and source-graded timing. A whole V30 claim also
requires its documented 8080 mode; the distinct Z80 sound CPU is not that mode.
All existing Intel profiles require before/after regression evidence.

Board owner tests cover every installed bus range, unmapped accesses, bank
edges, byte lanes, IRQ acknowledge, COP commands/DMA, input and sound latch
ownership. Video covers decode/encryption, layer/sprite priorities, scroll,
raster, palette and rotation. Sound covers CPU/timers/IRQ, sample selection,
mixing, partial PCM acceptance and cancellation. Product covers real control,
frame/audio delivery, lifecycle, invalid assets and cleanup in both widths.

Real-game checkpoints are reset-vector/initialization, protection/graphics
initialization, attract/title, coin/start, player movement/fire/bomb, boss and
stage transitions, late-game/loop boundary, game-over/continue and restart.
Actual selected-set stage identities and deterministic input/observation
signatures must be collected before claiming complete gameplay coverage;
they are not invented from elapsed time or a synthetic title screenshot.
Wall-time, instruction and trace-byte budgets contain experiments only.
