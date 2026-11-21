# T592 S3 — Restore the external OmniBook media

The published files are byte-identical to the S2 verification inputs. In the
repository deployment directory, both executables returned exit code 1 before
composing the monitor. The external System Card and C image expected by the
adjacent INI layout were absent. The S2 native boot evidence used a populated
disposable layout and did not establish that the owner's local published
directory contained its runtime inputs.

The offline NXVM tool restores the owner-supplied C ZIP to an active
FAT16 partition on a 32-cylinder raw disk with 16 heads and 63 sectors per
track. The tool is independent of build, Product startup and ATA execution.
It reads the backup, preserves the 8.3 hierarchy and contents, and creates a
new output exclusively. It does not alter the archive or install the System
Card. The [external input guide](../../../assets/myob300/README.md) names the
matching archives, geometry, startup and access-mode boundaries.

Independent readback checked the signed MBR/BPB, the active partition,
both identical FATs, allocated cluster chains, directory parent links,
all 13 directories and all 43 files byte-for-byte. The System 1.0 ABA backup
SHA-256 stayed `4d3f6fbadfabfb056ad42d1dc82007f9d7ca57026fffb648903ae6135bf5715d`.
The restored raw image SHA-256 is
`1b146fb6d17c3025a571699217715f21d80dcd4e23005319595ef5cb76d2b325`.
A second restore to the same path refused to overwrite it, and a backup with
a traversal entry was rejected before creating an output.

The x64 and x86 Release copies launched as ordinary processes in a separate
local asset layout with the unmodified published INI. A foreign working
directory did not affect configuration lookup. Each ran `info`, reported
the 16,515,072-byte disk connected, and accepted `exit`. The source archives
and the sibling asset checkout were unchanged.

With the exact external System Card present, both binaries rejected missing,
511-byte and zero-byte C images and an invalid INI access mode before the
monitor appeared. The valid source image remained byte-identical. These
start-time cases did not exercise a storage fault after guest startup.

The local setup verification reaches the monitor with attached media. The
original, unmodified C backup proceeds into a separate ROM Windows path after
`start`. The original-C Windows boot, Window/RDP interaction and guest power
qualification remain outside this launch result.

## Original DOS write rejection after startup

The x64 and x86 replays used the original ROM and a disposable variant of
the restored C image. The variant kept the original `CONFIG.SYS` and
replaced the `AUTOEXEC.BAT` Windows launch with a DOS file creation
attempt. The Machine attached that disk in `readonly` mode.

After POST and the original DOS startup, both guests reached the write
attempt and displayed `Seek error writing drive C`. A neutral key event
selected Fail at the DOS prompt. DOS reported `File creation error` and
displayed the subsequent batch marker, confirming recovery to guest
command execution. Both runs completed with the disk and backup unchanged.

| Item | SHA-256 |
| --- | --- |
| Disposable readonly C image | `020de47933a0d1845f827889ea77530b53c055eb42b38f9bfe25a27ba94d5588` |
| Owner C archive | `4d3f6fbadfabfb056ad42d1dc82007f9d7ca57026fffb648903ae6135bf5715d` |
| x64 guest output | `2de405256ce38c7dcfc4944a3db23ed98cea628e98addb92edd14a76b8fc67bf` |
| x86 guest output | `dad05be11bd3a8abed4c2c7c17ab710460c70c4eeaa8779eff2fc99aa81caeb4` |

These instrumented runs establish the original DOS and ATA read-only
path. The earlier guest write/readback verification covers writable
media separately. Hot removal and backing-file loss were not tested.

## Guest power qualification boundary

The original *HP OmniBook Operating Guide* (`OmnibookOperatingGuide.pdf`,
pages 1-6 and 9-8) describes an On/Off key and distinct power modes.
Advanced mode preserves the working session on ordinary power-off.
Standard and Off discard it and boot again at the next power-on.
The HP Windows package includes `OBPOWER.DLL` and the `OBMGM` management
program. The current board models the SCAMP idle request clearing on
PIC wake and the SMI memory route. The physical On/Off key sequence,
Advanced suspend and resume, and the Standard/Off transition remain
open product evidence. Monitor pause and resume are already qualified
as a separate lifecycle path.

## Original APM BIOS query

The HP guide names APM and the Advanced mode, and the original System
Card file inventory ties the Windows Power panel to the OEM manager.
The Win16 OBPOWER module imports OBMGM. The original DOS OBMGM module
directly calls INT 15h with AX=530Ah and BX=0001h. This identifies the
guest interface to probe without intercepting it in the emulator.

A disposable startup variant kept the original CONFIG.SYS, HP resident
programs and DOS. After SHARE it ran a small project-authored DOS query
instead of launching Windows. C was mounted read-only and both runs
completed with the same disk hash. The x64 and x86 logs agree:

| Original ROM request | Observed return |
| --- | --- |
| Installation check, 5300h | Carry clear, AX=0101h, BX=504Dh, CX=0000h |
| Real-mode connect, 5301h | Carry set, AH=08h |
| System power status, 530Ah | Carry clear, BX=0000h, CX=013Ch |
| Next event, 530Bh | Carry set, AH=80h |

The installation signature advertises APM 1.1. The no-event response
matches the [Intel and Microsoft APM interface specification](https://intel-vintage-developer.eu5.org/IAL/POWERMGM/APMV12.PDF).
The connect result is kept raw because the later specification does
not list AH=08h as a real-mode connection result. The diagnostic also
issued a disconnect before returning to DOS. This run establishes
working stock BIOS query and event polling under idle conditions, not
a suspend request, switch event, or resume.

| Item | SHA-256 |
| --- | --- |
| Query program | `d49b652b141d6168d68687581f7ceda219295aa694187429bacd7543e8c9b714` |
| Disposable C image | `1fd14d4126d1ad5cf85caa711db261317eb4f4ab1ad62cb59767fec3b6818ed0` |
| x64 guest output | `133eaf08b4d85b6b5ea6318dc830edf322e3a8d5e377c61723ae16c8b1c1276f` |
| x86 guest output | `caa26ad48a6ebb3395cc65f2dce8188bf9558a35f586b6db17a4e83f07069234` |

## Original APM suspend request

A second disposable DOS replay issued INT 15h AX=5307h with BX=0001h
and CX=0002h after the original HP startup, without reconnecting or
disconnecting APM. The replay traced the real I/O dispatch and captured
the failed CPU access. The original BIOS reached its device state save
code and selected the SCAMP index range 32h through 48h.

On the unmodified tree, x64 reached F000:A0ECh and failed reading ECh's
selected index 3Ah through data port EDh. The board already accepted
writes to the keyboard row selection latch. Reading back that same latch
is sufficient for this register. The two cached builds and existing
board unit fixtures passed with the readback and a real Core I/O
round-trip assertion. This four-line local candidate is not a release.

The patched x64 replay read through index 44h and then hit the same
unsupported I/O result at index 45h. The BIOS save loop reads the whole
range. Its restore code also reads 45h and 46h to write UART FIFO control,
then restores the other saved external device registers. This is a
separate App board obligation. The next implementation must determine
the serial register owners and routing before assigning values to these
indexes. No suspend return, wake, or session preservation was observed.

| Item | SHA-256 |
| --- | --- |
| Suspend request program | `d27b9a682083b64c9bc59742fe90270491dbc0f237f4ab1cdaaecf7f38fee779` |
| Disposable C image, unchanged in both attempts | `24f235b0955949fc2c1d9a353a7a97e9483d1adf6268239012164d42750c03ff` |
| Original failure trace | `b5a955024ee580ad2c8b8d05b1877ccd9b8bf40740d1ae37de0db802eb18a0eb` |
| Candidate failure trace | `b67b20466a3cb1bfeca878b4745e83ab89ffdc4050723af4d381b2b43cd4993f` |
| Candidate indexed writes | `86a202fe3d1555873bd932b5e45fd4085254d4696280dfe9a7d77bbd951f17c1` |

The original BIOS reads extension register 47h before restoring the
two FIFO controls. Bit 3 selects 3FAh or 2EAh for the value from 45h.
Bit 1 selects 2FAh or 3EAh for the value from 46h. HP documents the
four logical COM address and IRQ pairs in the *OmniBook Operating
Guide*, page 4-6. The selected destinations are offsets 2 in those
UART ranges. That supports an App-owned connection to a UART FIFO
control register, but does not by itself establish the backing latches'
power-on values or what updates them.

An x64 diagnostic supplied zero for just the two unsupported FIFO
control reads. It entered the original state restore loop and failed
on a write to index 3Bh at F000:A14Eh. This proves the needed bus
behavior of the keyboard column input: accept its restore write while
continuing to derive the value from pressed keys. Both column bytes
now ignore writes in the local candidate. These probe results are
separate from hardware suspend qualification: the trace shows the BIOS
executing its restore path, without an external On/Off event.

| Item | SHA-256 |
| --- | --- |
| FIFO-read diagnostic display | `9ec6e47d7f9b61a30328f2c50d57a420b745347ec98efab1c0e90398c9a5887b` |
| FIFO-read diagnostic indexed ports | `c16d3e41d14844f652941e0f4b94d9a671bc8d65d2fc9bb04d531b583334dfbb` |
| Restore-write failure | `06664d41be742e6f6635435365a63502e78a422945342e8f73b11463c4e6b899` |

With the column write behavior in place, both cached App board tests
passed again. A second x64 diagnostic also accepted the unsupported
FIFO restore writes and the zero-value write to the existing 3Eh probe
register inside the scratch runner only. The original BIOS completed
the indexed restore sequence through 48h and progressed into a polling
loop. The latest x64 diagnostic therefore checks a later firmware
boundary, not a product code path or an observed wake. The stock BIOS
also contains a separate status-gated branch that selects keyboard
matrix row 0, reads column bit 4, and may reset the processor. That
branch needs a live instruction sample before assigning the physical
On/Off key or changing the current matrix contract.

The restore-only diagnostic exhausted its bounded window. A paused
instruction sample was inside the real BIOS delay routine at F000:ADB0h
and the indexed trace showed repeated 02h/32h/47h queries. The original
loop tests bit 7 of I/O port B5h at F000:A3C6h, then checks the master
PIC before continuing. A separate diagnostic pulsed only that input at
the actual read, at guest tick 255000370. It demonstrates the branch's
input location and does not establish which physical switch or latch
drives it on the original board.

The local production candidate now gives 45h and 46h distinct, writable
FIFO shadow bytes. The current board has no emulated UART endpoints,
so both use disabled reset defaults as an L1 model. The guest's observed
3Eh restore writes back its inert zero probe value. That write is now
acknowledged. The x64 and x86 existing board tests passed with the
candidate, including both shadows and the matrix restore checks.

| Item | SHA-256 |
| --- | --- |
| Restore-only display and paused sample | `d7d0d8e503ad745a11c633695e7235cd29a0418b2a33f976c34c7b4c6969ff7a` |
| Restore-only indexed trace | `a4ab18a87ee610c3df065b25ecfaeb1a4b4176e92ec421acbd76dc3e7fa4fc14` |
| B5h bit-7 diagnostic marker | `fbd3e13c3743598f92be6e8f185f1195e6b3d3ea0e56f692442edc7ad4341f54` |

The one-shot B5h pulse did not return to DOS within the 280-second
bound. The paused sample was again in the BIOS delay routine, at
F000:ADC6h. The probe supplies a diagnostic input only and cannot
distinguish a sustained level from a missing secondary wake signal.
The display and paused trace SHA-256 is `9bbda32a2966538777b421ad73da18ad52a8df6d92bfccb18e1bc479891d99c0`.

An unmodified-ROM x64 replay of the production register candidate
completed the same save and restore sequence without an I/O bypass.
Its indexed trace matched the earlier restore-only diagnostic byte
for byte. It still remained in the BIOS wait with no host-operated
power input. In a separate test, one pulse of the status bit 0 at
index 02h reached F000:A337h. This diagnostic also reached the time
bound without returning to DOS. The App does not emulate either
diagnostic input in production.

| Item | SHA-256 |
| --- | --- |
| Production register candidate display and paused sample | `3585c3819aacb4feb232439d265e339ca35afd977b938a30b55b582664a28120` |
| Index 02h diagnostic marker | `397cfc05f23851c230892f278f4bca95631866843533051d04ac258d36dc2514` |
| Index 02h diagnostic display and paused sample | `e7f25eb25fb6a15917f13a40e0b2df82585ce5fd2418f3bf41c2dbf77d829474` |

The verified dual-width register build is published under assets/myob300.
Both native executables started with the external card and read-only
C drive in an isolated layout. Their monitor reported the attached disk
and handled exit, and the ROM, disk and checked-in INI were unchanged.
The previously running owner copy was left in place.

| Release | PE machine | SHA-256 |
| --- | --- | --- |
| myob300_0_5_0592_x64.exe | 8664h | `42867875243c9e9778a216d4823da16a8e0e94e3ad4f24364fae39514ae5782f` |
| myob300_0_5_0592_x86.exe | 014Ch | `0ec934f71e28c3f158c1ff80a9c80e9c974bf88ef9ec391ef43d9a60614d3e53` |

The register change is App-local. No Shared source changed. The
focused release gate covers cached x64/x86 build, board tests, native
monitor launch, external media attachment and exit. The real x64
instrumented variant traces the firmware register restore sequence.
This batch did not repeat the full repository or desktop test suites.

## Firmware wake cause and repeated suspend

A bounded trace of the index 02h bit-0 diagnostic kept the first two
samples of each instruction address so the F000:A4C7h loop could not
consume the entire capture. It shows that the original BIOS reaches
its resume epilogue. F000:A4DEh and A4E2h restore both PIC masks,
A4ECh restores index 48h, and A4F3h restores port 61h. The APM caller
then reaches the code after the BIOS routine. The original HP software
reads B5h again and the BIOS enters the save routine a second time.
The diagnostic still does not reach the DOS completion marker.

That distinguishes a rejected or insufficient wake reason from a failure
to execute the first resume epilogue. Static inspection shows that the
APM handler tests bits 1, 3 and 4 of its recorded wake reason after the
epilogue. The injected bit 0 does not satisfy that test. Separately, the
ROM selects UART modem-status port 3FEh or 2FEh and tests its ring input
at F000:A360h. The *HP OmniBook Operating Guide*, page 11-21, identifies
serial and internal-modem ring as auto-on sources and excludes PCMCIA
modems. This documents a firmware wake input with an independent
source. It does not identify the On/Off button's signal.

| Item | SHA-256 |
| --- | --- |
| Index 02h deduplicated effect trace | `765d389e9b09e2e912b7b76ed7f015ff295dd1b6a7d68e24013b92e04a489265` |
| Index 02h diagnostic display and paused sample | `28cbcfc9d528a55c3f4f52434cd5a4460e53686491094051adc184e4da521860` |

The first serial-ring diagnostic asserted RI for one modem-status read.
It did not return from 5307h. The ROM's loop advances while RI is high
and falls through to wake only after 1,000 consecutive high samples.
A second x64 diagnostic held RI through those reads, all at 02FEh.
Samples 1 and 1,000 were separated by 2,190,807 guest ticks, about
110 ms on the board's nominal 20 MHz axis. The actual BIOS then
returned from the original 5307h call with carry clear, and the next
530Bh returned BX=0003h with carry clear, the normal resume event.
The DOS diagnostic printed its completion marker. The read-only disk
retained its recorded SHA-256.

The x86 replay completed the same diagnostic from the same read-only
disk. Both runs sampled RI at the same two guest ticks and returned
the same APM registers, including BX=0003h from 530Bh. This checks
the original firmware behavior across the two host widths.

The injection targeted the observed modem-status I/O reads in the
diagnostic executable. It demonstrates original guest continuation
after a known wake cause. The production App still has no modem ring
or On/Off signal source, and this test does not claim preservation of
an open Windows session or Standard/Off cold start.

| Item | SHA-256 |
| --- | --- |
| One-sample ring trace, no DOS completion | `861afd7383de15e2956b2920bdd8db6173587f81f6b6a03d02c4f60276cc1e6b` |
| Continuous RI sample window | `dac3ed5f1c0cdcba0555006728733bdd25985360b58a34c96fca25f2a36a2621` |
| Continuous RI indexed and interrupt trace | `8d40e5a1a43770e0a551aee33f99d56ac365851f52e3514d4b77508042085b00` |
| Continuous RI DOS BIOS results and completion marker | `7565ef70cb75617566d99d7ac692865f805d693bd447fc7b2d6ef5499c917295` |
| x86 continuous RI BIOS results and completion marker | `6b16a2d087e8687f6e73b66e76b0582d9b7da38a77426692efd7cda46cf5219c` |

## HP power-key sense candidate

A separate x64 diagnostic asserted index 02h bit 4 for 1,000,000
guest ticks, then released it. The original firmware read the active
level at F000:A337h and A43Ch, selected keyboard matrix row 0 and
observed the relevant column released. It repeatedly checked bit 4
until the level cleared. It then returned to DOS with carry clear and
reported the normal APM resume event 0003h. This identifies a
guest-visible press-and-release contract for a wake key independently
of the serial ring path. The HP troubleshooting bulletin bpi01075
documents the separate Esc plus On/Off diagnostic combination.

The current candidate maps neutral F13 down/up to this App-local
level. The existing Machine input queue still owns delivery, and
the indexed port observes the level without a BIOS address check.
The cached board fixture passed on x86 and x64, including delivery
through the real Machine driver and the resulting 02h values.
Cold reset clears the pressed level. This is an L1 sense model.
The exact physical decode and behavior that initiates suspend
when the system is awake remain open.

| Item | SHA-256 |
| --- | --- |
| Diagnostic switch level and hold interval | `3e023a646a8f8fb7f29c959d24ad1f2488bd20cce907ce6c5170764ccfa0da40` |
| Firmware polling, matrix scan and release trace | `b8b35cfbf4b272a81bb77c5585728237e48bfb62f12dc9c2975cb6f71c509dd1` |
| DOS completion after diagnostic release | `78531e5c54f2ca80e0c10f74791ede80135c90008b89f8c258357a3c92797a01` |

The x64 original-DOS replay then used the production board and Machine
input path. The I/O observer only marked that the guest was waiting at
F000:A337h. The runner sent neutral F13 press and release through
emulator_machine_enqueue_input, at 88,822 and 89,077 ms respectively.
No port values were injected. The original BIOS returned carry clear
from 5307h, reported the normal 0003h resume event, and the DOS program
continued to its completion marker. The read-only disk was unchanged.
The x64 output SHA-256 is `da2d04633d930cce0a9b2b8edc5530d19e6f9b19cf37e28dda64f4e481d36d37`.

The x86 run sent the same production FIFO transitions at 97,252 and
97,508 ms. The original DOS diagnostic returned carry clear from
5307h, reported 0003h and reached its completion marker. The read-only
C image remained unchanged. Its output SHA-256 is
`c5035e270e54ea75005293c4a636fcce920e0af126fdb24ef95f12133b47c1e3`.

The published pair passed native launch, external card and read-only
disk attachment, monitor info and exit. The App source and its
existing board fixture are the only implementation diff. Shared
sources, other Apps, external media and owner INI are unchanged.
The native monitor launch check did not simulate a desktop F13 press.

| Release | PE machine | SHA-256 |
| --- | --- | --- |
| myob300_0_5_0592_x64.exe | 8664h | `dacf14d52b78168345dafbdff695ce898a218970cea69f0954e39c8fa3fc3d3d` |
| myob300_0_5_0592_x86.exe | 014Ch | `02fb6921da05410f2c6f5cfd529eaedaa6ed820edb04bd30c57a935c14786cbe` |
