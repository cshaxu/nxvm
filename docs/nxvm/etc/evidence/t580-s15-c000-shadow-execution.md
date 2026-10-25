# T580 S15 — PC110 C000 shadow execution

The selected PC110 TFT firmware owns a 48 KiB C0000h video shadow.  Its
ordinary data callback already selected that shadow for both reads and writes,
and the existing EC/ED control route determines whether a guest write changes
the shadow.  The instruction callback instead copied from the immutable raw
Flash bank.  Consequently, a permitted shadow write could be read back by
data access but could not be executed.

S15 removes that duplicate backing-store path.  Instruction reads now call the
existing data-read callback, so C0000h data and instruction access have one
selected shadow owner.  The existing raw immutable firmware owner still serves
all unclaimed locations, and F0000h/reset overlays remain unchanged.

The direct configuration route regression starts C0000h with HLT, confirms a
locked write cannot alter it, unlocks the existing shadow through EC/ED,
writes NOP, and executes `CS:EIP = C000:0000` through the ordinary Core debug
step.  The step reaches its one-instruction budget only when instruction fetch
observes the same shadow as the preceding data read.  Reset then restores the
selected raw byte.  This passed on x64 and x86.

The unchanged real-firmware C000-to-port-61 diagnostic also passed on both
widths.  This repair establishes memory-owner consistency only; it neither
qualifies a full firmware POST nor adds a panel, power, guest, storage, or
original-DSTN model.
