# M1 T7 S75 — Gameplay Progression Investigation

## Paired selected-set observations

After the S72 sprite-order and S73 portrait changes, the existing x64 probe
ran the verified owner set twice with a 20,000,000-instruction budget: once
without input and once with Coin1 and Start1 pressed and released at the
existing vblank checkpoints. The x86 probe independently ran the same input
case. Each stopped at its budget without a reported Board or COP fault.

All four sampled frame digests match between the x64 input and no-input runs;
the x86 input samples match the x64 input samples. The input run delivered all
four edges and recorded reads from the player, system and coin paths while
the corresponding inputs were held. Thus those bounded observations do not
establish an accepted credit or entry into gameplay. Identical sampled frames
do not, by themselves, identify where acceptance failed.

Both input and no-input runs generated no nonzero PCM in this interval, despite
YM register activity; the input run started no OKI voices. The current YM mixer
in `src/myraidenii/core/sound.c` explicitly labels its output an L1 diagnostic
waveform rather than YM2151 synthesis. That is an independent outstanding
sound-fidelity gap, not proof of the cause of this interval's silence.

The existing staging wrapper now launches its owned probe and product-smoke
children hidden, in accordance with the owner's background-execution rule.
The wrapper removed each generated staging child. No screenshot, ROM payload
or raw trace is retained. S75 remains active while the investigation seeks a
gameplay-semantic checkpoint and a concrete owner for the observed result.

A one-off x64 20-million-instruction input run also wrote its final native
frame to a temporary image for inspection. It displayed a large stylized
graphic and a narrow, difficult-to-read text region; it was not a black frame.
This observation does not identify the exact screen or establish correct
orientation, readable text, accepted credit, or play. The temporary image and
the one-off probe instrumentation were removed immediately afterwards. The
new evidence motivated examining text-layer decoding and composition before
concluding why the Coin/Start sequence has no observable effect.

## Character and background bit order

The pinned MAME Raiden II layout already cited in T4 S41 supplies character
and background bit offsets. Its generic decoder at MAME tag `mame0289`,
`src/emu/drawgfx.cpp`, reads each listed bit offset most-significant-bit first
and assigns the first listed plane the highest pixel bit. The corresponding
`src/emu/digfx.cpp` uses no reversal flag for these ordinary 8-bit ROM regions.
The former local decoder used the reverse convention for both operations.
This independently established L2 discrepancy applies to both the 8x8 and
16x16 layout, not the separately decoded sprite stream.

The local arithmetic now uses the correct bit numbering and plane weights.
Synthetic unit fixtures independently identify all four plane weights, both
halves of each tile layout, and the existing layer and palette compositions.
No external implementation or data was copied.

A second temporary x64 selected-set frame after the change visibly contains
the Raiden II title and legible licensing/copyright text after the existing
portrait rotation. Both temporary frame files and the uncommitted probe
instrumentation were deleted. This is a useful title-screen observation, not
proof of credit acceptance, playable stages, correct full video fidelity,
sound, desktop/RDP behavior, or release readiness.

## Later Coin/Start comparison

A one-off x64 run moved the existing Coin/Start edges to vblanks
1200/1230/1260/1290 and extended the budget to 30 million instructions.
Its final frame showed an aircraft, terrain and score display. However, a
same-budget no-input run produced exactly the same four sampled frame digests
and the same final digest. The displayed scene therefore cannot be attributed
to the inserted Coin/Start events; it is consistent with an attract sequence.
Neither run produced nonzero PCM. Both completed their budgets without a
reported Board or COP fault. Temporary timing edits and frame data were removed.
The actual Coin acceptance and gameplay-input path remains unqualified.

Both x64 and x86 repository-only unit suites passed 698/698 after the decoder
fix. Existing MyRaidenII Product candidates and affected test targets were
relinked in both stable Ninja trees. The corrected 20-million-instruction
selected-set input run also completed on x86 and matched the x64 final frame
digest. Visual inspection was x64 only; desktop, RDP and audible playback
were not tested.
