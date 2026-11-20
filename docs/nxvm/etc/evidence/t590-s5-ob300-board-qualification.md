# T590 S5 OmniBook board qualification

## Board changes under qualification

The App uses the existing Board Port B provider on the same 20 MHz
guest time axis. SCAMP II Table 10 gives a 15.625 µs default refresh
interval and Table 50 defines Port B bit 4. The observable L2 model
rounds 312.5 guest ticks to 313. This does not attest to physical DMA
refresh scheduling or programmable slow refresh.

The App installs an unpopulated bus range from the end of its 2 MiB
RAM to the 8 MiB probe limit. The normal Board provider returns FFh
and ignores writes in that range. Installed RAM remains writable.
The new unit case observes both refresh-bit polarities while executing
guest instructions and checks installed RAM and the original probe
address at 7FFFFFh. This case passes in both cached builds.

## Keyboard matrix and chipset startup

The HP keyboard matrix selects active-low rows through index 3Ah, with
columns read through 3Bh/3Ch. The observed scan rolls one active bit
through eight rows. Each selected row contributes its active-low column
state. The existing 8042 serial route remains available. The unit
test selects a row, observes a press and release in its column, and
still verifies serial IRQ1 and IRQ12. KVM key-to-matrix translation has
not been qualified.

The startup register list admits the original read-modify-write table
and subsequent configuration writes with an explicit allowlist. It
models the identified ordinary latches where their other electrical
effects are not established. Index 0Eh implements the independent
read and write choices in VL82C316 Table 14 through the existing
memory-device route, one field per 32 KiB block. A test writes into
the shadow while reads are still external, then enables the read
route and verifies the saved byte. The VGA memory owner still receives
the accesses that the shadow declines.

## Original-board replay

The unchanged 1.0 ABA card reaches F000:11F3 after REFDET and F000:2173
at the RAM-sizing write, passes the A/B RAM test, and executes the
startup tables. At step 1,276,742 it takes the guest exception handler
with a saved E000:FFFF address. The preceding BIOS code calls E000:0003,
but the board-only construction has not supplied that firmware window.
Both host widths give identical probe results. This is the boundary
for the original-ROM machine composition, including the ROM DOS option
code and access to the selected writable medium.

A separate probe supplied the candidate immutable mapping of the 32 KiB
card span at 7E8000h into E0000h. The original code then entered E000:0003
at step 1,243,975 on both host widths and reached the port 61h speaker
routine at F000:2C14 in the four-million-step trace. This is a
mapping and execution checkpoint. It does not establish the complete
E-region decode, POST completion or an interactive DOS environment.

The longer probe was still in the speaker path after 12,000,000
iterations on both widths. A stopped-stack inspection identified
firmware diagnostic 0280h. Its caller reads CMOS through ports
70h/71h, selecting F8h (index 78h with NMI masked), and uses data
bit 6 to reach that diagnostic. The current board config has only
the lower RTC RAM enabled. Machine composition should verify the
upper CMOS access and its contents before treating the speaker loop
as a timer failure or increasing the instruction budget. Enabling
the existing extended CMOS option in a scratch build passed this
part of the firmware and exposed 0302h, the 8042 self-test reply timeout.
One status-poll delay still produced 0302h. A scratch 256-tick command
delay advanced past it to diagnostic 1100h. The nominal delay is a
guest-visible candidate, without physical controller timing proof.

| Verification record | SHA-256 |
| --- | --- |
| Production board replay, both host widths | `808d453956a78ef6150b9feb1e2ec0db794c048e57ba8609908f5e2a3aa75f70` |
| Scratch E0000h receiver, both host widths | `f389d9c9abac33032c8c98a777ab5f61b63350f00dff59040add737db7d16dfd` |
| Scratch CMOS diagnostic stack, x64 | `7fc7e72b928a372f77f63b3e618a21659be04d1af5f03050948e9b0c796999ed` |
| Scratch extended CMOS and 8042 timeout, x64 | `055a141a57cd1c0df2fdd8fb95d9a3265d3c98a0e9b0671d70cd0189b25d2122` |
| Scratch 8042 delay receiver, x64 | `fac59c8c35000e42c991047b75dd995c518d6efb4520efbb6d9b6cb536804b81` |
| Cached x64 unit aggregate | `a34551ffdcd7efeaeb1337ba7d7891376f1dd66462600c22258a1900a1382583` |
| Cached x86 unit aggregate | `a930bbad5d0ad9f24a41a146b981a7cdfa22d8e0bb8fb3709f1269fed756be28` |

Focused board tests and all 717 unit registrations passed in each cached
build. Aggregate times were 172.99 seconds x64 and 209.51 seconds x86.
