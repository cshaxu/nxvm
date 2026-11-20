# T590 S4 OmniBook System Card windows

The App board uses one borrowed immutable common image for both fixed
reset and banked reads. Reset fetch keeps the existing 32 KiB Core ROM
and high alias. The App exposes the lower F000 segment from card
7F0000h until a guest window selects either 16 KiB page. The
configuration latches remain behind the existing FBh/F9h gate.
Core resolves the one App provider, including spans over page boundaries.
The card uses the same guest clock and executor as the rest of the board.

The firmware evidence establishes two common pages at F0000h/F4000h,
the 14-bit page shift and the configuration data consumed at the original
1.0 ABA checkpoint. The board also models the four contiguous
configuration triplets at indices 26h..31h. For this L1 model the first
active matching window wins, the complementary flag selects an optional
attribute image, and that image is byte-wide on even addresses. Disabled
windows relinquish the route. Selected unmapped bytes read FFh and
selected writes do not change either image. The physical flag behavior,
attribute contents and full socket behavior are still unverified.

Both host widths passed the same synthetic guest test for reset
visibility, a read across two selected banks, write isolation, actual
instruction fetch from the selected card page, and separate attribute
bytes. The x64 and x86 real-card probes then used the production App
board, with no scratch providers or chipset overrides. Both entered
F000:51AC at step 149 and verified the corresponding card bytes at
8051ACh. At step 154 both stopped at F51B4h, where the App declines the
unsupported write of FEh to index 3Ah and Core reports status 6.
Independent confirmation of 3Ah/3Bh status behavior is required during
full board qualification. The original card was read only.

The card routine at offset 8008BBh selects 3Ah with one active-low
bit, waits, reads 3Ch and 3Bh as two bytes, and rotates the selected
bit through eight values. The nearby table holds keyboard scan codes.
This supports a keyboard matrix interpretation, separate from the
existing 8042 serial path. A scratch probe with released matrix inputs
continued to step 232 on both widths, at F000:20B7. There the first
read-modify-write entry selects chipset register 03h (DRAM map in the
VL82C316 manual). The next batch should verify the known SCAMP
configuration uses and identify HP extensions before assigning effects.
For the normal cold boot, the matrix model must produce its idle value
because all columns are released, without an unconditional firmware
success return.

With scratch latches for the 16-entry startup table, the original code
then reached the port 61h refresh check. It failed waiting for bit 4
to rise, returned AX=1201h, and entered the 1-3-1 beep path. Table 50 of
the [VL82C316 manual](https://bitsavers.computerhistory.org/components/vti/pc/VTI_VL82C316_VL82C323_SCAMP_II_199210.pdf)
identifies bit 4 as REFDET, toggled by refresh cycles. Table 10 gives the
default 15.625 µs interval. Reusing the existing Port B provider with
a 313-tick nominal period on the 20 MHz Core axis passed this specific
check on both widths. That scratch run next stopped at F000:2173 on a
RAM-sizing write above the 2 MiB installed range. These two checkpoints
give S5 tests for guest-time refresh and unpopulated memory. Neither
scratch prototype is part of S4 production code.

| Verification record | SHA-256 |
| --- | --- |
| x64 synthetic board | `d755cd607b9d6acac7b3dcbc3719d29cdf40e59d097e6f5bebc8d5b0ed7d2f9e` |
| x86 synthetic board | `a24d88530af76cbf1b459bcb9cff000d64829977f60e81ac03c8dd13e85ff8e4` |
| Original-card boot excerpt, identical both widths | `0ed79acf24e81bf32e55587a0d9a9b74621ffa2e9e2d57e2b5aad80f2be311a3` |
| Scratch matrix handoff, identical both widths | `3a74e9bca3af4ccc60ae30d6a036b01e0776838a46eabcc99f6f9d79ba287acc` |
| Scratch refresh-failure diagnostic, identical both widths | `cc87b04b8649ffc387a5d7ef3fb438df111cdad95ef96d34a96fe967f7139812` |
| Scratch refresh receiver, identical both widths | `ef47320bb7a2939aaee1b21aea753e721041bd965713992d7333678455a07236` |
| x64 full unit aggregate | `f8a146ad308e2f051ceb1d090d8d6ebaabe835428c110e3cf7c2b3bcffe2c78b` |
| x86 full unit aggregate | `3137189e0cd15d2eddd68bed8da8612e0adff60bc7543c6f76293b9c043e6aee` |

Both cached full unit aggregates passed all 717 registrations. The x64
run took 184.76 seconds and the x86 run took 207.37 seconds. The App has
no release binary yet. Complete POST and ROM DOS remain unverified.
