# T591 S3 OmniBook Machine assembly

The App now prepares the raw System Card with the existing owned-file reader,
rejects a non-16-MiB image and transfers the allocation with the existing
X86-NXVM Machine profile. Core and board teardown finish before the App frees
the image. There is no second Machine or frontend path.

The existing board mapping still owns the reset alias and banked card windows.
The fixed E-region option ROM is installed through Core's firmware provider.
Both the board fixture and the assembled Machine use the same provider. The
profile passes the existing C&T display snapshot to the Machine display slot.

The Board/Machine fixture uses a synthetic card and rejects a short file.
It removes the source file after preparation, constructs and resets the actual
Machine, and reads the E-region signature through the existing debug driver.
This verifies ownership and receiver wiring. The App preparation entry takes
a verified raw path; the final Product will still need an identity policy for
the owner-supplied card before publishing a usable App.

Cached x64 and x86 builds passed the existing `myob300.unit/board` entry with
the added Machine cases. The composed driver returned no frame at the blank
reset state, then published a 640-by-480 graphics frame with a white pixel at
column zero and a black pixel at column one after the VGA register and memory
writes. The fixture uses the same mode programming as the direct board test.
This proves the C&T-to-Machine frame path without certifying real POST video.
The board and Machine use a single topology declaration. Previously the
profile installed RTC only on the board plan, leaving Machine's copied RTC
description absent and causing public CMOS export to return invalid state.
The same fixture now writes extended index 78h through the Machine and
exports it in the 128-byte CMOS image. Index 38h remains distinct.
Separately, a temporary dual-width driver probe
executed the synthetic reset-vector far jump and stopped through the existing
Machine debug callback at `F000:8000`, with protected mode clear. The probe
can capture GDT, IDT, selector and instruction bytes at the original stop
without patching the guest or changing the production execution route.

The current assembly also passed the complete repository-only unit
inventory on both stable cached Release trees. This excludes owner-media
integration.

| Host | Unit results | Wall time | Log SHA-256 |
| --- | ---: | ---: | --- |
| x64 | 717/717 | 106.15 s | `295d3dc11b158e62fb345fda2239e44811d5eb1b10389df37390aca41676f28d` |
| x86 | 717/717 | 112.26 s | `eaade9865c8844b22c46ceebad1bc7a1f8b6022e6e86c659f0610c4a7f9a495f` |

The owner unlocked the System Card volume. An original-card Machine run
reached the first `MOV ES,AX` with `AX=0048h`, `CR0=1` and GDT base `E8A0h`.
Selector 0048h initially described writable present data at `100000h`, and
that instruction executed successfully. Before the second iteration, memory
clearing with A20 low had damaged the descriptor at `E8E8h`. Its access byte
had changed from `93h` to `00h`, and loading it shut the CPU down with
detail `20000000h`. The firmware had already executed `IN AL,EEh` at
`FCEA2h`, but the current board had no EEh route and A20 remained disabled.

[VL82C316 SCAMP II, Table 55](https://bitsavers.computerhistory.org/components/vti/pc/VTI_VL82C316_VL82C323_SCAMP_II_199210.pdf)
defines EEh as the dummy read-to-set and write-to-clear alias of port 92h bit 1,
visible when MISCSET bit 7 is clear, with the resulting latch ORed with KBC
A20. An isolated read-only EEh experiment bypassed the damaged-table stop
on x64 and x86 and reached the end of the extended-memory probe. It does not
prove correct EEh disable or latching. The production proposal exposes the
board's existing A20 latch through one scoped profile callback, with EEh and
MISCSET decode remaining local to MyOB300. The owner approved that x86-nxvm
change. The
production implementation and the MyOB300 EEh route use the same Port A
latch as 92h. The fixture covers set/clear readback, preservation of A20
when the keyboard source remains asserted, and EEh exclusion under MISCSET.
Production replays on both host widths showed EEh raising A20, passed the
previously failing load, reached the installed-RAM boundary at 200000h,
and showed the later EEh write dropping A20.

With an isolated read/write EEh experiment that also updated the existing
Port A latch, the original card advanced past the memory probe and through
later device checks. It reached a real-mode `STI; HLT` loop at `FC25Fh`
after the POST caller at `F2DC3h` executed `INT 10h`, `AH=02h`. The video
BIOS data area at `0449h` was uninitialized, and the visible trace contained
no ATA register accesses before this stop. The card contains a checksum-valid
32 KiB C&T 65510 video ROM at offset `7E0000h`. Testing an isolated C0000h
mapping alone produced the same INT 10h stop and no execution at `C000:0003`.
This leaves video ROM discovery and initialization open. It does not justify
presetting INT 10h or selecting a C-drive controller by assumption.

A read watchpoint at `C0000h` confirmed that the scratch mapping exposed
`55 AA 40`, but recorded no guest read at that address and no guest
execution in the mapped 32 KiB before the wait. The guest's INT 10h vector
at the stop was `F000:F065`; the path reached `F000:C25F` and halted there.
An execution trace from cold reset found no VGA register writes before
this path and confirmed earlier POST diagnostics `0308h` and `0543h` from
the saved diagnostic words. HP documents these as the keyboard-fuse
input and missing parallel port, respectively. Their relation to VGA
discovery is still being tested.

The original POST at `F1BB3h` reads the delayed 8042 `C0h` reply from port
60h and tests bit 2. The generic reply was `80h`, so the next branch
returned `0308h`. MyOB300 now sets that physical input high through the
existing board configuration (`84h`, retaining the existing bit 7).
The focused board and Machine fixture passed on x64 and x86. Its existing
controller polling exercises both `AAh -> 55h` and `C0h -> 84h`. In an
original-card replay with the A20 experiment, that change removed `0308h`.
A separate disposable `378h` parallel-latch test removed `0543h`. Neither
change caused the guest to scan C0000h or replace `INT 10h`. The parallel
trial is not part of the production implementation.

The first proven failure before video initialization was System Card
directory discovery. The original firmware programs indices 26h..28h
to F4h/81h/FDh, then scans the end of D0000h's 16 KiB window for
`OBCBIOS`. The directory starts at card offset 7F7FD0h and includes
`BIOS0` at 7F7FF0h. The old destination mask rejects the programmed
F4h value. A scratch decode matched the directory and reached the
original ROM's copier, which then selected source pages 203h and 204h
through the same destination window. The App now accepts both observed
common-memory control patterns through the same page decoder. This
models the combinations used by 1.0 ABA. General flag behavior and
physical slot priority remain unverified. The board smoke also exercises
the earlier F000/F4000 mappings and the modeled byte-wide attribute path.

The original firmware writes A5h and 5Ah to 378h and reads each value
back to detect its onboard parallel port. The board now exposes a
byte-wide data latch there and clears it on guest reset. The probe
confirmed this was the only parallel register the guest used before
leaving that POST check. Printer status, interrupts and data delivery
remain outside this latch model. Neither host-width replay needed a
temporary port override or a separate C0000h mapping to pass the ROM
directory check and enter the firmware's card copy loop. Both then
repeated a slot status query while BIOS ticks continued to advance.
Full ROM DOS and writable C have yet to be demonstrated.

The copied BIOS calls Socket Services GetStatus (8Fh) for its fourth
logical socket and tests DL bit 6. In the original firmware this
return bit comes from chipset index 33h bit 3. The register now reports
READY for the installed immutable System Card and preserves the other
observed configuration bits. Startup supplies this signal without a
modeled power-up delay. x64 and x86 left the observed polling loop,
continued initializing card windows, and later reached the original
BIOS keyboard wait at F000:CF37. The earlier polling loop reached
approximately one second of guest time in the bounded replay; its
original 0221h-tick timeout is about 30 seconds. This validates the
installed-card startup path, not the electrical timing or removable
socket state machine.

Original POST has option entry calls at E000:0003 and E800:0003. The
only 32 KiB C&T 65510 BIOS in the card is the checksum-valid block at
7E0000h. The E8000h trial installed INT 10h on both widths, then failed
the video table search. The guest reads its video parameter table
through absolute E000 segment pointers embedded in the C&T option
ROM. The E0000h trial reached the actual video initialization earlier
and advanced to a CRTC Input Status 1 poll. The E0000h C&T and E8000h
DOS option placement is being qualified in probes. The production
App keeps the previous single binding pending that verification.

With monochrome port decoding in the probe, the original ROM also
exercised a word IN from the CRTC pair at E000:0F2C. The existing CT
VGA receiver exposes the CRTC data register but rejects reads of
the CRTC index. The [C&T 65510 data sheet, page 46](https://www.nrjfwax.vgamuseum.info/images/doc/chips/f65510.pdf)
defines CRX as readable at the selected 3B4h or 3D4h base. This is a
distinct Shared defect from A20. Current production Shared source
includes only the approved A20 change. A disposable controller build
that returned the existing index latch made the original word read
complete. The same original ROM then passed the retrace check and
returned from the display initialization call, reaching the later
POST checkpoint at F000:295Ah. The experiment added no independent
index value and did not alter the System Card. Further full boot and
frame qualification require the proposed Shared correction and
verification of the App's selected I/O base.

An extended replay with the trial ROM placement, both VGA I/O
bases, and index readback reached an actual 80×25 text screen at
step 22,394,150 on both host widths. The unchanged 1.0 ABA firmware
reports invalid configuration information and waits for F1. This is
the first original VGA frame observed through the App capture API.
The matching trace digest is
`b8952ff12e84e38c00739d5cca088e75090519d7350a6becd2ae046d7f166445`.
The replay used production A20, card banking, System Card status and
parallel data. The experimental display changes were linked only
into the probe. No writable disk was connected.

Sending the native Set 2 F1 make and break through the existing
8042 path released that wait on x64. The guest continued into
post-acknowledgment firmware code and then repeatedly sampled at
F000:DC95h in the PIC fallback handler. The full instruction trace
shows that IRET returns to F000:9FA9h and repeats a divide exception.
The original routine forms `750 * (255 - IN(B4h)) / IN(B6h)`. Both
unclaimed board ports read as zero, so the guest divides by zero.
The firmware records both samples in CMOS and uses the result to
choose an entry in its supply-level table. This identifies an
App-owned power-sense gap. The trace does not establish the analog
transfer function or a measured hardware sample.
The bounded replay observed neither INT 19h nor a primary or
secondary ATA task-file access after acknowledgment. The C medium
protocol remains unconfirmed. The first replay is recorded by
`d82a3bec23e19e2ae183fafd58c949687681d79742e1b6505e6994e9fc9c562b`.
The instruction trace digest is
`216bdaa41a5117bb267acb6be49a12749f789d8a22b32b52c4f0a8152985ed55`.

A read-only probe returned 128 at B4h and 180 at B6h, yielding 529
under the guest's own conversion. That sits in the upper part of the
observed supply-level tables. The firmware continued to INT 19h and
executed the ROM DOS bootstrap at E800:0020h. Guest code subsequently
queried BIOS geometry for drive 80h through INT 13h AH=08h. This
shows a hard-disk BIOS view and does not yet identify the physical
controller behind it. The App now supplies the two fixed nominal
samples as an L1 stable-supply model, with no battery dynamics or
BIOS override. The focused board tests passed on both host widths.
The first probe digest is
`f336f0562a76bde3de64ab2dbdfdb9126816cc12440122e3b8c95bb555b3443b`.
The replay with the App's production power ports reached the Microsoft BIOS
services, MRCI and Flash File System 2.00 startup messages by step
33,999,999. The BIOS services driver reported logical drives A through D.
That report does not prove directory access or writable C. The display
placement and CRX readback still came from the isolated receiver build.
The replay digest is
`ffa631540328e9d51b0cb3125480f36dc71424b12d41bce95d0b05f6b1e41993`.
Later screen capture shows the original loader falling back to drive D
after failing to find CONFIG.SYS on C. The D path starts Microsoft
Power Manager and HIMEM and reports the High Memory Area available.
The owner C backup contains CONFIG.SYS, so it can be used to validate
the different C path after the guest-visible controller is confirmed.

The original ROM installs its disk interrupt at vector 76h from
F000:C29Eh. Its handler at F000:C475h reads 1F7h and acknowledges
both PICs. The disk routines issue ATA IDENTIFY, read sector words
from 1F0h, and assert software reset at 3F6h. In the no-medium
replay, the INT 13h geometry request for drive 80h instead reaches
the existing fallback routine and returns carry without touching
the task file. MyOB300 now binds the existing ATA PIO controller
and one primary slot through Machine construction. The focused
x64 and x86 fixture mounts a separate 512-byte image, submits a
CHS read, advances the actual Machine by one guest instruction,
then checks DRQ and the returned byte. This proves the stack from
the Machine configuration to a mounted ATA sector. It does not
assert the original 40 MB drive's physical geometry.

A disposable non-active FAT16 partition was populated from the matching C
archive with the existing ATA receiver's translated 16-head, 63-sector
geometry. An independent reader recovered all 43 files byte-for-byte.
The 41,803,776-byte image has digest
`6d3ea9626fb32787adf1703f5db36c7e108b50885e5fe127c6722eeb47cd9256`.
Only the disposable image was attached, in read-only mode. On the original
ROM replay, mounting it through MyOB300's real Machine constructor still
left the ROM loader on its D fallback. The original hard disk BIOS first
queries logical socket 3 at F000:8D33h via the Card BIOS, then configures
both ATA I/O ranges. It installs the INT 13h and IRQ14 vectors only after
that sequence succeeds. This distinguishes the complete sector-transfer
receiver from the still-missing guest slot detection and activation.
The next storage replay must capture the socket query and identify the
minimum hardware readback and configuration required for the installed C
device. The archive itself confirms the design: its CONFIG.SYS selects
COMMAND.COM on D, and AUTOEXEC.BAT normally starts Windows from D while
using the matching C data files.
The mounted replay digest is
`1c99fa373ec899b1a8e36db3070f803ac0b1d74ed13ecbde546e2dbd9e68e034`.
An instruction-level probe of the mounted machine narrowed the failure
to the first slot query. At F000:8D33h, the OEM Card BIOS receives
INT 1Ah AX=B000h with a socket-3 request. It returns carry set and
AX=0080h at F000:8D35h, and the firmware immediately skips both
ATA I/O window requests. The query trace digest is
`81bc2558b581ddbcf57b47f822990a0019f25decba135cdabec65b65d33a849b`.
A second probe added present and ready bits to the slot-3 status at
index 35h, with the mounted medium held fixed. The original query and
I/O window requests then succeeded. The firmware read 50h from 1F7h
and immediately rejected 00h from 1F4h because this card check expects
AAh at 1F4h and 55h at 1F5h. This identifies the next receiver
boundary: the OEM PC Card's initial task-file signature and its
activation, separately from ordinary CHS sector transfers. The probe
flags were not made unconditional in the App. Its log digest is
`2963eced913c9c1fd94d3e03d72a4e0562ffd7c613eac6dbe4e3d9296f91f7a1`.

The later D-boot stop repeatedly executes the HIMEM relocation
routine. The CPU trace shows DS and ES with 4 GiB limits after PE
is cleared, then shrinks those limits to 64 KiB when ordinary
`MOV DS,AX` and `MOV ES,AX` reload selector zero. At the restored
`REP MOVSD`, ESI is 00110000h, EDI is 00012357h and ECX is six.
The segment exception handler fixes the descriptors and returns,
but the same real-mode reload shrinks them again. The fault trace
digest is
`56e9e74f05f63ddcce722a8d01b7b13aac09da674f96de1431b21fe66d71490`.

A disposable Am386SXLV CPU build that retained the existing data
limit on real-mode reload got past the loop without changing the
guest. It reached OmniBook Message Manager, then exposed a read of
the missing SCAMP II SLPCTL register at index 13h. The App supplies
the awake-state readback using the deasserted sleep pin and clock
enable from [SCAMP II Table 23](https://bitsavers.computerhistory.org/components/vti/pc/VTI_VL82C316_VL82C323_SCAMP_II_199210.pdf).
This is an awake-only read model. Sleep controls and clock switching
are outside this batch. The CPU experiment digest is
`217a0d0053214a4986c85aa4d1a4be0bb9b5bbd295c774e69fbe9e5d09d96221`.
No Shared CPU change was made in the production tree.
With the App's SLPCTL readback in place and both experimental Shared
receivers linked in the x64 diagnostic, the original D fallback passed
the Message Manager and printed the original COM1 through COM4 assignment
text. It then stopped at linear 28212h. Neither this replay nor the C media
fixture constitutes interactive DOS qualification.

| Original-card diagnostic | SHA-256 |
| --- | --- |
| First protected-mode Machine breakpoint | `74ca83b6d9635a0a58860a97c0b21863bc7fe313aa1f597ff55235b374d3e8da` |
| Production A20 state and both descriptor reads | `d216eb6ed120c5f12f0c9de26413aeb36187f9511e7accafc1e9426bb47523c6` |
| Read-to-set EEh experiment, both host widths | `f15daf349d35c8e92663b53de46ea2990f5db7056b661ce3d43880507f580add` |
| Production EEh path, both host widths | `4199c5a7f99072e43801baa465c7d0bc01049b646eeeaf2e14f6c284497fe38b` |
| Extended read/write EEh experiment | `d13341664f1b97202a230ae4a278efa8217d27f8faba5d87dafa58daafe5766d` |
| Original INT 10h call site and wait | `c08cef4f7d99f8df310d351faa4ee00f50a7b3f722f04ea990588d338394af52` |
| C0000h visibility and guest read-watch trace | `15cb755aa25ba71a42b6d6a5b6621550468769708cd53d703a1b576f0fca792e` |
| KBC input-bit failure | `a58d6aa0a7663e931cb4fc823f34d3c4135dcae95b3c032dac87da1b11f6d309` |
| With fixed KBC input, without LPT experiment | `84f27c947c1d2a57f5066ca2153dd159fcfb0a4eee88366051a35c8cb57f6ada` |
| With fixed KBC input and LPT experiment | `3dea977f49e60001834bb103029325d0270768eb3a6a33fe9488bee6bf757718` |

The matching C backup `ob300a_system100_aba.zip` has digest
`4d3f6fbadfabfb056ad42d1dc82007f9d7ca57026fffb648903ae6135bf5715d`.
It contains 56 ZIP entries totaling 977,844 uncompressed bytes,
including AUTOEXEC.BAT and CONFIG.SYS. The archive contains no DOS
kernel, command interpreter, or sector image. The HP service procedure
presents the internal C medium as replaceable and distinct from the System
ROM. It lists both a 40 MB disk and a 10 MB flash model and documents preparing
the C medium with `OBSETUP /F` from System ROM. That establishes the media
separation, not the board protocol expected by this particular firmware.
[HP OmniBook service manual 5965-0244, product table and page 50](https://www.elhvb.com/mobokive/edwin/laptops/hp/omnibook/service/ob300-425-430-530_sm.pdf).

## Original option ROM routing and C-slot detection

The Card supplies the video option from offset 7E0000h at guest E0000h
and the DOS option from offset 7E8000h at guest E8000h. The video ROM uses
absolute pointers into E000h. MyOB300 now maps those two 32 KiB ranges
through the existing immutable firmware provider. Its board test gives
the two images distinct size bytes and verifies that guest writes change
neither range. The same fixture verifies that chipset index 35h reports
the C slot present and ready only when populated by the Machine.
Cached x64 and x86 board tests passed after both changes.

On the original-ROM trace, the BIOS checked for AAh at 1F4h and 55h at
1F5h and then wrote the reverse values to check the registers. A
disposable power-on task-file seed satisfied that check. The firmware
then issued 91h with a zero sector count, 10h and 90h. The zero count was
rejected, 10h succeeded and 90h aborted in the current implementation,
leading to POST 8310. An isolated ATA diagnostics implementation
returned 01h for the attached primary device. The firmware continued
through IDENTIFY and read the C volume. The AAh/55h startup check is
evidence about this original firmware and C device pairing.

The x86 replay read LBAs 1828 and 1829 successfully through the mounted
ATA backend. These LBAs contain CONFIG.SYS on the disposable FAT16
image reconstructed from the matching C backup. The image was mounted
read-only and its SHA-256 remained
`6d3ea9626fb32787adf1703f5db36c7e108b50885e5fe127c6722eeb47cd9256`.
The x86 replay log digest is
`350ca193210e47790661a058f2eec3ce8ab6ded5e5f685ae2e54056b70c78470`.
It used experimental CRTC, CPU and ATA receivers. It did not verify
writable media, an interactive shell, a native window or lifecycle
repetition.

The late stop at linear 28212h was a write of 04h to EDh after
selecting chipset index 3Fh via ECh. The App models this index as an
L1 read/write latch with reset value zero. A board test checks
readback. The real register's electrical effect remains unidentified.

## Approved CRTC, ATA and CPU receiver batch

The product now connects the mono CRTC index/data/status addresses to its
existing C&T controller. The shared chip provides index readback through
the same latch. An App regression exercises mono writes and observes the
same register through the color data port. Shared coverage checks initial
and masked index values and confirms that reading the index leaves data
access intact.

The ATA task-file config now carries the observed cylinder-latch values for
a hardware reset. The App selects AAh/55h, and ATA software reset still
clears both latches. ATA 90h reports diagnostic success for a present disk
on the selected task file in the modeled single-disk path, through the
existing command-completion interrupt. The chip fixture checks the signature,
software reset, IRQ, status and error result. The legacy WD and Xebec
paths keep their existing command implementations.

The CPU receiver retains the prior data-segment limit on Am386SXLV
real-address reloads with PE clear. The existing segment test verifies DS
and ES loads with a prior 4 GiB limit, checks the unchanged stack limit, and
compares the existing 80386 profile. This is a scoped L2 compatibility
model supported by original HIMEM execution. The 80386 microcode analysis
finds a real-mode base-only SBRM operation on that processor. It is useful
supporting evidence and does not establish Am386SXLV silicon completeness.
[80386 memory pipeline and decoded microcode](https://nand2mario.github.io/posts/2026/80386_memory_pipeline/).

The CRTC, HDC task file, CPU selector and MyOB300 board tests passed on
both widths in the cached Ninja graphs. An original-card replay used only
current production libraries and mounted the disposable C image read-only.
It passed the earlier CRTC stop, initialized HIMEM, and continued beyond the
3Fh latch write on both widths. The firmware displayed an invalid-primary-
partition prompt for the disposable non-active FAT16 fixture and then
stopped at F9B52h. The runtime trace confirmed index 13h and data 85h.
The App now latches writable SLPCTL fields, exposes the deasserted
-SLEEP pin, and keeps the existing clock rate for the observed
divide-by-one operation. The fixture checks that status bit 2 is
read-only, reserved bit 1 is not taken from the write, and clearing SLP
restores awake readback. Other clock and sleep modes remain outside
this model.

A second disposable image changed only the MBR active byte at offset
446 from 00h to 80h. Both headless production replays used that image
read-only, passed the primary-partition prompt, installed SHARE and
completed LapLink Remote Access initialization. They then stopped at
linear FDBADh in a REP MOVSD copy. The card and C backup were unchanged.
This establishes a usable C data path through original startup files
to that checkpoint. Interactive DOS and writable persistence remain
open.
[VLSI SCAMP II, Table 23](https://www.bitsavers.org/components/vti/pc/VTI_VL82C316_VL82C323_SCAMP_II_199210.pdf).

## High System Card aperture

The original ROM data copy at FDBADh used DS base 00802070h and ES base
0004D550h in protected mode, with 0BCCh doublewords remaining. The
low offsets of SI and DI were zero. The failed source address was outside
the mapped board memory. A read-only mapping of the card's high common
area to identical physical offsets let the x64 diagnostic pass that copy.
The x86 diagnostic independently recorded card reads beginning at
00802070h. The App now borrows those bytes from the same card lifetime
via a fixed 8–10 MiB read-only window. The upper decoding limit is a
model for this ROM-card image, supported by the boot replay and contents
of the image. The HP service manual's low memory map does not establish
this high limit.
[HP OmniBook service manual, memory map](https://www.elhvb.com/mobokive/edwin/laptops/hp/omnibook/service/ob300-425-430-530_sm.pdf).

The cached board tests passed on both widths and verify direct reads and
write protection through Core. Replays linked only production libraries
on both widths. The unchanged startup files reached Windows standard-mode
startup, which printed its insufficient-extended-memory diagnostic with
the fixed 2 MiB guest. This verifies progress into the original Windows
startup executable and does not qualify Windows itself. A separate
disposable DOS-focused medium removes Windows autostart to verify DOS
storage without requiring Windows startup.

| Replay | SHA-256 |
| --- | --- |
| `card-production-fault-x86.log` | `d6b43c91013be34aecb9d8ab6ddb27014197cf9920e8c777d5c5b494907f29cb` |
| `ob300-aperture-production-x86.log` | `60831fb22e03939c1c1cc9f66767c5646d8dfb4a24a98127ff835937c19612d4` |
| `ob300-aperture-production-x64.log` | `e13eec6212fac435e3e798ff141761febce13d0bd0c5030578e0a2dec5640b97` |

## Original DOS and writable C medium

The DOS-focused FAT16 fixture retained the original CONFIG.SYS, including
its D:\COMMAND.COM shell. Only the fixture's AUTOEXEC Windows launch line
was replaced with an ECHO redirection and a visible marker. Both builds
started the original ROM DOS, reached C:\>, and created the marker file on
independent disposable images mounted in direct mode. After Machine
destruction, the sector-level checker found the new FAT16 root entry,
allocated cluster and exact payload on both images. Both post-run image
digests were
`1b9b2eabba5407772baa7066ed9204a63964f17df7393fec9283b1112abac5e6`.

The DOS BIOS polls SLPCTL before enabling CPU interrupts. MyOB300 now
observes the existing PIC output and clears the modeled sleep request on
a pending IRQ. The board regression enters the poll loop with IF clear,
delivers a native key through the actual KBC, and verifies that the CPU
services the same IRQ and reads the key. Both board tests pass. The
original-ROM DOS probe accepted the injected native bytes but did not
show a VER response. The visible prompt and durable write are proven.
Original-ROM keyboard commands, repeated lifecycle, and a native window
remain open.

| Replay | SHA-256 |
| --- | --- |
| `ob300-dos-write-x86.log` | `57f6dfd5e0cd1a93249081b42c635f2e1533289ed79dd9d0a8469ea00dcb69ec` |
| `ob300-dos-write-x64.log` | `3bb315178464d3dc0cfd5dd8a92924e6a18bd1eb5a766087d6782425495f2758` |
| `ob300-native-dos-keyboard-x64.log` | `aa436088708ca9132b92bce662f17f00e87aadad692c1dce832093ac95ba4731` |
| `ob300-sleep-irq-regression-x64.log` | `baffed36a4ea45e83d8d8d0a7f3cd2950da02cc4e8c1d42f6dbe1c6664b845bf` |
| `ob300-sleep-irq-regression-x86.log` | `246f01cdd331de33538c856af7f42d0274a03f157a0708252463e4f56b1b2ad0` |

## Receiver qualification and publication

Shared production and test changes are in `c73e8898a` (13 files,
84 additions and 15 deletions including manifests). Both width-specific
unit graphs built from their existing Ninja trees. x86 passed 717/717.
The x64 suite passed 715/717 on its first run. The two failing manifest
checks read noncanonical lowercase hash entries. After restoring the
required uppercase format, the affected manifest tests and the updated
App test passed on x64. All four manifest files passed their standalone
verifiers. The current App also passed its standalone x86 board test.

The standard publisher rebuilt fourteen existing product executables.
The deployed file digests matched the publisher output and each PE field
matched its filename width. The release commits are `606f1b44d` (PC),
`7163670d7` (MyNES) and `595b7c97a` (MyRaidenII). No MyOB300 product
executable was published as part of this receiver batch.

| Artifact | PE machine | SHA-256 |
| --- | --- | --- |
| `assets/nxvm/nxvm_0_5_0582_x86.exe` | `014c` | `721e5e582472363d25bd19666b047a5b9488c60357f240622e812af2d41f61ae` |
| `assets/my5160/my5160_0_5_0582_x86.exe` | `014c` | `628afbc9a04011d99c4c6ef9cd194c01217662a028170ff9fd43ac201799a4a2` |
| `assets/my5170/my5170_0_5_0582_x86.exe` | `014c` | `784e641d214c22da8d2d064ff51f4cf32900e50ce7a9bf312733d2619ac355f8` |
| `assets/mydeskpro386/mydeskpro386_0_5_0582_x86.exe` | `014c` | `d30342e35d2cfb7644575343daced1cd99aaf0d9a47e52c1c945f2b562a6f253` |
| `assets/mypc110/mypc110_x86.exe` | `014c` | `cebf9742ef9b94a66254e91393aaaf27cdf84a9d9e5eb36a3128512669189595` |
| `assets/mynes/mynes_0_0_0044_x86.exe` | `014c` | `73f2f8c7a0c17dc1ee0bb771f3326aadf90317112bb463734de1b3581ed11e59` |
| `assets/myraidenii/myraidenii_0_0_0007_x86.exe` | `014c` | `54bfee1a28de491c39ccb56eee8b74d7f3907e2696712ff788f88bdf41437b05` |
| `assets/nxvm/nxvm_0_5_0582_x64.exe` | `8664` | `14a2677bc7db8d6e5c4834d6b2f12b98754920f8c7dbe8e41eebd58923836443` |
| `assets/my5160/my5160_0_5_0582_x64.exe` | `8664` | `b3c98acfecd8a7937ef70b3ba0c9688cb3cfeaa3054600c41adc934c7388be56` |
| `assets/my5170/my5170_0_5_0582_x64.exe` | `8664` | `09dfd10780263a5b4e684dec429ab53cabaa1effd7ecfcab5a6731eaf9fe1f6c` |
| `assets/mydeskpro386/mydeskpro386_0_5_0582_x64.exe` | `8664` | `3dba90dbff8344e17d8febf42dea6ad5cf6138f2ab42ba7a947f4896364e2092` |
| `assets/mypc110/mypc110_x64.exe` | `8664` | `5dfeb6a819027acff803396fa996897b6087f362aa9b75fa82588de37852d6e3` |
| `assets/mynes/mynes_0_0_0044_x64.exe` | `8664` | `772a09534573d9cf59aff79c43b670ff0ae74f59265ff899125199413163c44e` |
| `assets/myraidenii/myraidenii_0_0_0007_x64.exe` | `8664` | `4e4834328fb2dad32ea29cb3055fa4897564e84dabd1a81d43bc9827dbd45c17` |

## Shared A20 qualification and release

Shared P1 (`2617d595f`) changes three files, with 18 additions and six
deletions including the manifest. The Board-Base service and Port 92h
use the same producer of the fast A20 input. The existing KBC route
continues to supply the other input. The focused OB300 fixture verifies
that clearing EEh leaves effective A20 enabled while the KBC input
remains high. The EEh alias is registered only by the OB300 board.
No other Shared source pack changed.

Cached full repository-only unit suites passed 717/717 on x64 (267.56 s)
and 717/717 on x86 (139.06 s) for the A20 publication. The later
App-only card and parallel changes passed the focused x64 and x86 board
tests and original-card replay. The replay ran in the headless Core
harness using production Board providers. Tests did not use a desktop
window or establish interactive DOS qualification.

The cached publisher rebuilt and deployed all fourteen receiver EXEs.
Each deployed digest matched the publication log, and each PE machine
field matched its x86 (`014c`) or x64 (`8664`) suffix. Receiver commits
are `44a48813b` (PC), `3f3b1e266` (MyNES) and `625acaeec` (MyRaidenII).

| Artifact | PE machine | SHA-256 |
| --- | --- | --- |
| `assets/nxvm/nxvm_0_5_0582_x86.exe` | `014c` | `3418eb27b4b7d3db29a88f5ee8f0e5c4996b0c7219abf25b8dbe3bc2acf05318` |
| `assets/my5160/my5160_0_5_0582_x86.exe` | `014c` | `08c0ad5d03ba6f7df5ede1cfc23efc77298ac72a1b482c925ed300f4bbeabb07` |
| `assets/my5170/my5170_0_5_0582_x86.exe` | `014c` | `3fb44e1328372db9290ba67bdcebf5cea4662ebe7e74a1f1ef1872ca7bf2cd94` |
| `assets/mydeskpro386/mydeskpro386_0_5_0582_x86.exe` | `014c` | `7756ab0be91f5a4b05d58ee0b4a75c79bd9bc2840a70455d89ae1a7b41e0f718` |
| `assets/mypc110/mypc110_x86.exe` | `014c` | `050e4d1356d91b73a121378834c205de060ccf367088da96475711e43ca493f3` |
| `assets/mynes/mynes_0_0_0044_x86.exe` | `014c` | `83c4b5d7f078beb66956dff63ad4cac2914ec53cd54fc1fee24485634e0fdc5a` |
| `assets/myraidenii/myraidenii_0_0_0007_x86.exe` | `014c` | `f184e2005ed55e004ce300127eed9536eb118901e2796d935e8eae857130a8ca` |
| `assets/nxvm/nxvm_0_5_0582_x64.exe` | `8664` | `d0ec82c4d218d44098c32e9d336c48cbd746458f96ee3a7e16c63e2a1b2ff35e` |
| `assets/my5160/my5160_0_5_0582_x64.exe` | `8664` | `42449cc0e08988e6b0ab7d36573a42142c64e60fe606454327d84c6668456bd4` |
| `assets/my5170/my5170_0_5_0582_x64.exe` | `8664` | `0245a36e84f7832cd48c5bf59abe07496de41a13b86498375220f9fc4dd921bd` |
| `assets/mydeskpro386/mydeskpro386_0_5_0582_x64.exe` | `8664` | `b50d97be2d722fa644a9db2fab9c39764ebb41eaf0bd7681cfe0893bfc563355` |
| `assets/mypc110/mypc110_x64.exe` | `8664` | `55befc60389e7b537192f8f594e3d670d09cb7099b1b7a83df020d1e32fff4ce` |
| `assets/mynes/mynes_0_0_0044_x64.exe` | `8664` | `86657bd3a6ef1093a8fab773eeecc3fe530a1326ca8c7022e504051b136259d0` |
| `assets/myraidenii/myraidenii_0_0_0007_x64.exe` | `8664` | `b902a5ff65d00f25e7b5d83e0e413061a76f354282211eaecca2bb4a7c3ff6c9` |

| Test record | SHA-256 |
| --- | --- |
| `x64-a20-full-unit.log` | `8a80e3f2291168b4d17b9c4ba1fa67a6383245ed93fdc65611d82906fccc54b3` |
| `x86-a20-full-unit.log` | `fefe475b130b07ede046e6cda5dec3e163b289d3d8e1e691c061b1827fe98573` |
| `card-integrated-x64.log` | `cb98927e07e69b28ce8de116b95c08b476e5cfc2eb41de479f7d53bf13af63f1` |
| `card-integrated-x86.log` | `d3d68c92520ff5a270740ff9feca78528f9ab0a3b61bab45f04b1bcc6b9943c0` |
| `card-mono-stop-x64.log` | `411943dc25e13450bbcf31ddb0a29e5f01a12d5e93e64ae80ab582b7b630a438` |
| `card-crtc-x64.log` | `6c716ed4da79fefdf2f26ec527929bafdb4fa7568a3aa549102bd59e167d364f` |
