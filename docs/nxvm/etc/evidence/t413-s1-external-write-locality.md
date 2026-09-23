# T413 S1 External-Write Locality Bridge

M5:T413:S1:EXTERNAL-WRITE-BRIDGE:OK

The original D3PE page-RAM description states that the four CPU write cycle
types are analogous to the read cycle types: initial and page-miss CPU memory
cycles insert two wait states, and a sequential same-2048-byte-page hit inserts
zero. It distinguishes write timing by MWE and a one-CLK32-later, one-CLK32-long
write CAS. The D4-RCTL equations also explicitly name CPU write, write hit and
write miss states.

T413 extends the Core generic bridge from T412 so a committed CPU data-memory
write participates in the same configured 2 KiB locality tag as instruction
prefetch reads. A miss adds the configured two retirement ticks and a hit adds
zero for the Model-40 configuration. A cancelled write changes neither tag nor
pending timing; reset clears both. This is an original-fact-informed
generic-at bridge, not a claim that its page tag reproduces the D4 row/bank
comparators, idle/hold behavior, CAS phase, or CPU overlap.

Read-only PCjs confirms separated DeskPro CPU/chipset/DMA composition but does
not implement D4 page-RAM write timing, so it supplies no timing scalar. The
focused smoke proves a real CPU MOV moffs write stores its byte and publishes
the initial prefetch plus write-page miss surcharge at retirement. It also
proves same-page write hit, cancellation and reset on the existing Core
external-cycle observer boundary.

The T413 developer artifact is vm-0-5-0413,
build/output/nxvm_0_5_0413.exe, 3,211,636 bytes, SHA-256
72F747A75460856B2C967A4763565CE11915BC8E0D562DF4EFF2ABCF98DF3D4A.

Exact D4 row/bank mapping, idle and system-board transition, write-CAS phase,
BWAIT, DMA/refresh arbitration, locked/RMW breadth and Model-L3 acceptance
remain transferred to the DeskPro physical-cycle proposal.