# T4 S4: Sprite Flags L2 Contract

## Admission

The owner explicitly authorized use of other emulators as L2 behavioral
references when no L3 source is available. This is not an import approval:
the source policy continues to prohibit copying, transliterating, linking or
redistributing their code, algorithms, tables or protected data.

The prior S1--S3 records remain accurate about the absence of independent
hardware evidence. This record supersedes only their implementation block for
the bounded receiver below.

## Bounded Model

The current MAME RAIDEN II map is used as an L2 behavior reference: it names
`0x6ce--0x6cf` as a write-only sprite flags destination and combines byte-lane
writes into one 16-bit value. The local model independently implements only:

- byte `0x6ce` as the low byte and `0x6cf` as the high byte of a Video-owned
  `lib_u16` latch;
- reset to zero;
- guest reads and inspection rejected, while the existing Core byte splitter
  handles a V30 word write as two valid byte writes;
- retention of bit 15 in the latch for the later decoder boundary.

No decryption, sprite-buffer transfer, render, palette, priority, frame or
new host path is implemented or implied. In particular, bit 15 is not claimed
to cause an effect until a qualified local decoder consumes it.

## Ownership And Verification

Video owns the latch and its reset. Board only maps the two byte lanes and
forwards them to Video; COP and Shared components do not mirror it. The unit
suite verifies direct byte merge/reset/error semantics and actual Board route
write/read/inspection behavior. This is an L2 progress boundary, not a
playable-game, original-hardware or completed Video claim.

## Bounded Selected-Set Evidence

The opt-in owner-ROM probe was run on both host widths after the route was
installed. Each run reached the same later CPU fault after 68,245 instructions
and 1,076,579 ticks; neither stopped at the prior flags write. The probe keeps
only this neutral result and deletes its staged program lanes. That proves the
selected program executed the actual Board route; it does not identify the
next receiver or qualify graphics output.
