# T3 S3: Main IRQ, Inputs And Sound Latches

## Boundary

This S consumes B04--B06 only.  It adds no Shared interface, host timer,
sound CPU, audio output, COP behavior or game ROM input.  The MyRaidenII Core
board is the sole owner of its input levels, interrupt request and sound-latch
state; X86 Core remains the sole CPU/timeline owner.

## Evidence And Grade

The frozen S1 ledger's `0x30` main interrupt vector remains the B04 L2 board
fact.  The verified MAME research revision is
`c1425a1af00b88aa07477b1b23a30ed5e5f856ff`; it was read as LGPL-2.1+ research
only, with no source, table or code imported.  The two temporary files had
these SHA-256 values before cleanup:

| File | SHA-256 | Observed L2 fact retained here |
| --- | --- | --- |
| `src/mame/seibu/raiden2.cpp` | `FDDE57F6E9024657F2A9830864523305139562229750B87C967E597324CDB26A` | Main low-byte sound window is `0x700--0x71f`; DSW, players and system words begin at `0x740`, `0x744`, `0x74c`; player controls and system bits are active-low. |
| `src/mame/shared/seibusound.cpp` | `60EBD1049B83274A82FCF51FD2F59C6E3F07A96309DA66FF76A173C82A8ABE71` | Main command bytes are offsets 0/1, replies are offsets 2/3, command-pending is offset 5, and command delivery uses the documented offset-6 latch transition. |

The operator-manual control evidence supports the semantic two-player
direction/two-button/start/service model.  Coin inputs are held as active-low
board state for the later sound receiver because the selected reference places
coin consumption on the sound side; this S does not invent a V30-visible coin
register.  The exact board electrical implementation and timing are not
claimed as L3.

## Implementation And Ownership

- The existing X86 Core attachment calls the Board's pending/acknowledge pair;
  acknowledge clears the one-shot Board request and returns vector `0x30`.
- Three replacement memory routes return the Board-owned DSW, player and
  system words.  Writes are deliberately accepted and discarded, rather than
  falling through to ordinary RAM.
- The sound window is a replacement low-byte interface.  Odd-byte lanes read
  `0xff` and ignore writes.  It owns two command bytes, two response bytes,
  command pending, and a deferred sound-interrupt request.  Future sound work
  consumes the local typed operations; it does not borrow Board storage.
- Reset restores released active-low inputs, all-high DSW/coin defaults, bank
  zero, no pending main/sound interrupts and cleared latches.

## Regression Owner

`test/myraidenii/unit/core/board_bus_smoke.c` is extended rather than creating
a CPU duplicate.  It executes a real V30 `STI/NOP/HLT` path through the public
Machine, requests the Board interrupt and confirms vector-`0x30` handler entry
and acknowledgement.  It also exercises input polarity, low-byte sound
command/reply/status behavior, high-lane replacement behavior and reset.  The
existing X86 Core tests remain the sole owner of generic attachment/CPU
semantics.

## Deferred Receivers

Video supplies periodic raster/VBlank requests.  Sound supplies Z80 execution,
the actual RST18 service, YM/OKI and coin consumption.  COP/DMA remains T3 S4.
No external ROM, desktop, audio or gameplay claim is made by this evidence.

## Focused Verification

The modified Board source and its existing focused smoke target were rebuilt
with the reusable Ninja graphs for both GCC widths.  Both
`myraidenii.core/board-bus-smoke` runs pass, as do both complete current
MyRaidenII unit selections (`myraidenii.unit.tools.rom-inventory` and the
Board smoke).

The required repository-only unit aggregate then ran serially within each
independent reusable architecture tree.  Both x64 and x86 complete with
`687/687` passing.  The earlier parallel attempt remains non-evidence: its
independent Shared verifier self-tests use overlapping generated probe paths
beneath a reusable build tree.  That Shared fixture-isolation debt is retained
in MyRaidenII TODO for its correct receiving owner; it does not alter this
Board result or authorize a Shared change in S3.
