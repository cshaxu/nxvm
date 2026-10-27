# M5 T580 S69 — POST error and post-reset investigation

## Initial evidence

An x64 retirement trace from cold startup directly observed register `0Eh`
read as `00h`, then written as `00h`, `40h`, `60h`, and finally `64h` before
the Setup handoff. The x86 trace independently observed the same writes up to
`64h`. The BIOS at `F000:5E3F` adds the bytes at CMOS `10h..2Dh`,
compares the sum with `2Eh..2Fh`, and branches to `5ECF` to set diagnostic
bit `40h` on inequality. A separate `F000:5408` path sets bit `20h` after
the firmware checks that diagnostic state. The raw 128-byte S67 capture sums to `056Dh` over
`10h..2Dh` while its stored `2Eh..2Fh` is `0391h`; the pinned historical
Open-Source-PC110 JSON has the same values. Bounded x64 and x86 replays
directly observed the unchanged BIOS compare `DX=056Dh` against `CX=0391h`,
take its mismatch branch, and write `40h` at 3,286,153 retired instructions.
Both exited zero only for this observation checkpoint; their identical logs
have SHA-256 `A8255AAFAD7AD91D31C524E7F383811A87E8C6697123458F41E30EE00FD3579B`. The
comparison therefore matches the captured image in this execution; this is
not evidence of an RTC read error and does not authorize changing the seed.
Disassembly and the retirement tail also show that the firmware's later
`F000:5408` path ORs `20h`, and `F000:5474` tests the diagnostic bits and
subsequently ORs `04h` on its invalid-time path. The selected BIOS's error
string table contains `173` at `F000:E35B` and `163` at `F000:E343`.
At `F000:4F8C`, the firmware selects `173` when diagnostic bit `40h` is set;
the invalid-time path at `F000:54F8` selects `163`. This supplies a direct
firmware mapping, distinct from attributing the error to an RTC hardware
failure. The display itself was observed in S68.

An additional x64 replay supplied the two normal Enter transactions. The
guest wrote `03h` to `2Eh` and `2Eh` to `2Fh`, then requested its fifth CPU
reset. A test-only non-mutating RTC inspection at that boundary observed the
configuration sum and stored checksum both equal to `032Eh`. The run exited
zero for its Setup/input/reset checkpoint at 144,068,624 retirements. The
log SHA-256 is `9F27F02ED4A533D5F0A02620E5106D5BADD6441F1873EAA5DB7242A8CCCCD25E`.
Temporary private inspection is not part of production or a public API. This
proves same-machine configuration repair, not preservation when the process
is closed and restarted, nor a successful boot after the reset.

The owner-supplied TFT CMOS seed used by the current construction has byte
`0Eh = 00h` and `0Dh = 80h`. S68 observed the firmware subsequently reading
`0Eh = 64h` in the date/time editor, then writing `60h` before requesting
reset. The seed therefore does not simply contain the later `64h` value; the
guest execution changes that byte before the observed confirmation. This is
not yet a complete account of which firmware operation sets each bit.

The local Open-Source-PC110 `Discovery/Chipset/readme.md` at commit
`02ccf2e8e19b298a2cf04c5d430278eb087cc368` reports a real machine with RTC
valid-RAM register `0Dh = 80h` and POST diagnostic register `0Eh = 00h`.
That is an L2 reference observation, not a complete error-code specification.
Current `x86-nxvm/chips/rtc146818/rtc.c` sets the RTC valid-RAM bit and
preserves the calendar/configuration across its ordinary reset. The retained
S68 trace does not yet prove what the real firmware reads after its reset.

The current construction loads its compiled seed as Board defaults; the
inspected MyPC110 and X86 NXVM Machine paths do not show a persistent CMOS
save/load route. Thus a processor reset within the same machine and closing
and reopening the program are different questions. The latter cannot be
claimed to preserve a guest's changes merely from the RTC reset implementation.

PC110-QEMU commit `bae390ebf21665ec97860d28e26ec46f1fe8f4e9` describes its
assisted Easy-Setup path and an intentionally emptied POST error log. That
hook supplies a research lead, not an acceptable substitute for identifying
the source of NXVM's observed errors.

## First post-reset replay

The x64 observation continued past S68's first reset with the same machine
and unchanged firmware. The BIOS read RTC index `0Eh` as `00h` during the
following execution, then produced another processor reset at `FFFF:FFF0`
after 152,372,044 total retired instructions. The run exited 1 because the
requested later INT 19h checkpoint was not reached; this is a diagnostic
result, not a passing boot. The trace retained only its first 128 port events.

One earlier event is a `92h` read returning `03h` followed by a `03h` write.
The current Board requests fast reset on a zero-to-one edge, retaining the
bit through processor reset, so this particular same-value write is not
evidence of the second reset's cause. The second reset's final I/O and CPU
context are being captured by the next replay. The x86 repetition also
reached the sixth reset at 152,372,044 instructions and exited 1 at the
same diagnostic stop; neither run reached the requested INT 19h checkpoint.

The x64 tail replay stopped at the same sixth reset and retained the preceding
instruction sequence. Its final observed instruction starts at `F000:5516`
with bytes `B0 FE E6 64` (`MOV AL, FEh` followed by `OUT 64h, AL`); the last
retired I/O before it was a `61h` write at `F000:4B91`. The OUT itself was
not included in the retirement tail, so this is strong control-flow evidence
for the normal KBC reset sequence rather than a directly captured retired
OUT. PC110-QEMU's pinned KBC-reset reference also identifies this BIOS entry
and describes its CPU-only reset. A sixth reset is therefore not by itself a
failure: the diagnostic's temporary stop condition is too early to decide
whether the firmware later progresses.

A further x64 replay allowed that KBC reset to complete and continued the
unchanged firmware. It did not reach the requested `F000:52BD` boot decision:
the CPU stopped at `F000:6999` (`CLI; HLT`) after 153,210,996 instructions.
S43 recorded this address after a PIT/refresh calibration failure, but the
local PC110-QEMU reference also identifies `6998/6999` as a shared CLI/HLT
wait routine. The retained retirement tail reaches it after a return from
`F000:53C0`, so the address alone cannot identify the calling condition or
prove a timer failure. The subsequent x64 replay observed no passage through
`F000:6981` in the post-reset phase before the wait. Its simple `6999`
address flag is shared by the helper and therefore cannot classify a failure.
Both local PC110-QEMU and PC110-EMU reference implementations explicitly
skip or resume this `CLI; HLT` helper rather than model the waking event.
That shows their runnable outcome cannot by itself establish which hardware
event should release this particular wait; it is still useful as an L2
comparison of the firmware path.

The subsequent x64 run captured the actual text display as `102` and the
stack return offsets `6936`, `4D07`. Inspection of the selected BIOS shows
`4D04` calls the PIT test at `68C2`; the failure path at `6930` calls the
error printer and then the common wait, returning to `6936` only if awakened.
The test first accumulates timer counts with OR until all bits have appeared,
then with AND until all bits have cleared; either phase may enter `6930`.
This is earlier than the `6981` timing-delta comparison, explaining why no
measurement was observed. The exact failed phase and its device cause are
not yet established; the next diagnostic captures those values without
changing guest execution.

The next x64 trace entered `F000:6930` with `AX=5501`, `BX=55AA` and neither
counter-bit loop previously visited. The BIOS disassembly has two earlier
branches into that error: a channel-2 output test and a test of the BIOS
`0040:006B` interrupt flag. The observed `AL=01h` is consistent with the
latter path, since the code sets bit 0 in AL before testing the flag; the
other early branch masks AL to `20h`. This is an inference from the trace and
instruction sequence, not yet a directly recorded branch outcome. The existing
IRQ handler writes that flag at `F000:EB67`. The next trace records both
conditional outcomes and handler entry before attributing it to stale IRQ0.

That x64 trace directly observed the handler at `F000:EB67` with `AH=01h`,
the channel-2 check at `F000:68F4` falling through, and the flag check at
`F000:68FF` taking the branch to `6930`. This confirms the immediate cause of
the `102` screen is an interrupt flag already set before the BIOS begins the
counter-bit loops. It does not yet distinguish a previously latched IRQ from
a new event during PIT/PIC programming. The x86 repetition observed the same
handler entry, conditional outcomes and `102` failure at 153,210,996 retired
instructions; it exited 1 and is not a passing boot.

The x64 PIT/PIC trace then narrowed the timing: BIOS writes PIT control
`43h=30h`, unmasks IRQ0 with `21h=FEh`, and enters its IRQ handler, which reads
PIC in-service state `01h`, masks IRQ0 again and sends EOI. It subsequently
programs and successfully reads channel 2 as `55AAh`; the later flag branch
takes `102`. This establishes that IRQ0 was delivered immediately after the
unmask, before the channel-2 check. The trace does not itself show when the
PIC first latched that request or prove that delivering it was incorrect.
The first attempt to retain pre-test port history used a machine trace callback
that did not observe the desired operations in this diagnostic; its empty
history is not evidence of absent operations. A subsequent run uses the
retirement observer already shown to capture those port transactions.
That replay records two master PIC initializations immediately before the
PIT test, each ending with all IRQ lines masked (`21h=FFh`). Thus the request
cannot be assumed to have survived PIC initialization. The trace alone does
not expose when IRR bit 0 becomes set; a non-mutating register snapshot is
needed to distinguish the interval before and after PIT control programming.
The following x64 replay captured the live master PIC without altering it:
at `F000:68C2`, `IRR=01`, `IMR=FF`, `ISR=00`, and PIT0 output was high.
After PIT mode programming and IRQ0 unmask at `F000:68D1`, `IRR=01`,
`IMR=FE`, `ISR=00`, and PIT0 output was low. Therefore IRQ0 was already
pending before the self-test's PIT reprogramming; unmasking exposed it. This
does not by itself prove the pending edge was incorrectly generated or specify
the correct PC110 reset/timing model.
An additional non-mutating x64 sampling run recorded a master PIC
initialization clearing IRR, followed by a PIT0 low-to-high transition while
the BIOS was executing its `F000:EA88` delay loop. At that transition IRR bit
0 became set while the line was masked (`IMR=FF`). This is a newly latched
edge, not an IRQ surviving initialization. It remained pending at the test
entry. The trace does not yet establish whether PIT0's retained configuration,
the duration of the BIOS wait, or a PC110-specific reset interaction differs
from the intended machine; the PIC's normal edge-latching must not be changed
merely to hide the POST error.
A subsequent x64 observation sampled PIT0's retained control word `36h` and
reload `65536`: it was active in square-wave mode, not programmed to a small
divisor. Immediately after the final PIC initialization its remaining count
was 11918; it subsequently rose during the `EA86` refresh wait and continued
switching before the test entered. This is the model's observed state, not yet
proof that the state or the elapsed guest time matches the PC110 hardware.
Another x64 observation recorded the final PIC initialization at timeline
tick `788828877`; the first new IRQ0 edge appeared in the refresh wait at
`788993680`, and later rising edges at `790806213` and `792618746`. With the
App's 33 MHz source clock, consecutive rising edges are about 54.9 ms apart,
consistent with the configured 65536/1193182 second PIT period. The BIOS was
still inside the refresh loop after several PIT cycles. Thus the observed
pending request follows the present clock and masked-edge model; it is not a
demonstration of a misprogrammed PIT divisor. The PC110-specific behavior that
would avoid the early IRQ remains to be established.
The BIOS's `EA86` loop reads port `61h` bit 4 and counts refresh transitions.
The reference PC110-EMU implements its PIC as a mask-register stub and
injects a warm-reset BDA value in its KBC reset handler; PC110-QEMU similarly
contains firmware-specific progress hooks. These are useful comparison facts,
but neither supplies evidence that an 8259-compatible PIC should discard a
masked edge. Applying either hook here would change the guest path rather than
explain the device interaction.

## Validation and scratch disposition

The final diagnostic built for both host widths; the final bounded checksum
observation returned zero on both. The post-reset BIOS replay failed at `102`
on both widths and is recorded as a failure, not a passing boot. The x64
Setup save/reset replay returned zero for its named input/reset checkpoint.
No product executable, production component or external asset was modified.
All owned diagnostic processes completed. The reviewed scratch directory
contained 75 files / 35,480,115 bytes, including a derived Setup reference
with SHA-256 `7128F7C5D4907A8C7F1D7B8C0D64F8617CDA43464F36599D21ED7BBCCAB8FE8B`;
it was removed after the observations and hashes were recorded. No full
repository, desktop or RDP qualification is claimed by this S.
