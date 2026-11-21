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

## Original SMM handler backing

After a probe linked against the P11 production board code resumed
a disposable DOS session, an ordinary F13 press and release left DOS active. An
explicit SMI pulse in the diagnostic then entered the CPU's original
reset-vector SMM path. The firmware did not return to DOS in the
observation window and was executing the card boot path. The pulse
diagnostic does not establish that the real On/Off switch uses SMI.

System 1.0's card initializes the handler through the ordinary A/B
shadow path. It selects chipset index 0Eh, enables shadow access,
writes its marker at A0210h and copies the handler beginning at
A0800h. The ROM's SMM entry checks the same marker at 60210h before
jumping to 60800h, with BASIZ=7 selected. The App instead had an
independent 128 KiB backing for SADS. This was sufficient for a
synthetic write followed by read within the same alias, while the
original setup write and SMM read reached different bytes.

The candidate now maps the SCAMP A/B SADS selections to the existing
ordinary shadow bytes. The standard VL82C316 table identifies the
selected backing ranges and translation. Selected C/E paths retain
their prior private storage. The board fixture checks that an SMM
write at 60210h becomes visible at A0210h in A/B mode and is separate
from ordinary DOS RAM at 60210h. Both cached board tests passed.
[VLSI VL82C316, Table 40 and Shadow Control Effects During SMM](https://bitsavers.computerhistory.org/components/vti/pc/VTI_VL82C316_VL82C323_SCAMP_II_199210.pdf).

| Original P11 diagnostic | SHA-256 |
| --- | --- |
| SMM instruction-space read samples | `a8216d18d1fd76898e332cfd828d371bff6fc15802da74129389cc28fc2a069b` |
| Guest output and final CPU sample | `dc0b471a974be676da82140abbc184e1b63d6690741d9fcdb7985be6ca82ad8b` |

The x64 candidate read the original marker through SADS at 60210h
and fetched instructions at 60800h. The next observed failure was at
6080:0171h while the original handler accessed ordinary RAM at
9C4B6h. The current Core dispatch returns the board's unsupported
status on that non-translated address. Neither the ordinary DOS RAM
address nor the fallback is covered by the synthetic alias test.
The App alias correction is still a local candidate.

| Candidate diagnostic | SHA-256 |
| --- | --- |
| Original marker and first installed-handler fetch | `457059d46a88076e4110c01833b499ebfa98ee3b0a2a86571ca0624659b415bd` |
| Guest output and first non-translated SMM access failure | `7662c8e11004670a461ac194bb0ef21e125b5a4b76bb5fff80d36c588f773130` |

A separate x64 diagnostic used ordinary Core memory routing for
SMM accesses declined by the board. The handler passed the fault at
9C4B6h and the Machine remained active for the remainder of the
8.5-second post-resume window. Its final sample was in the original
BIOS PIT-based delay routine at F000:ADCBh. The read-only test image
was unchanged. The observation did not include a completed second
power cycle.

The existing Core attachment contract deliberately rejects an
unsupported SMIADS access, and its test requires that rejection.
A production change therefore needs an explicit distinction
between routing to ordinary memory and rejecting a private access.
The diagnostic's blanket unsupported fallback is not the proposed
production contract.

| Ordinary-route diagnostic | SHA-256 |
| --- | --- |
| Guest output and final CPU sample | `dfb270a74010a5ae2e6c3806931b85903a086a98ab7eb3217d98c737117028b9` |

## Explicit SMIADS decode and receiving checks

The existing Core attachment result now distinguishes an ordinary-memory
route from an access refused by the board. Core runs the existing physical
decode only for the explicit ordinary route. SMIADS provenance stays visible
to transaction tracing. A provider error is returned unchanged and an
observational read still bypasses transaction and device side effects. MyOB300
uses this result for accesses wholly outside its enabled management window.
The SADS backing remains separate from ordinary DOS addresses with the same
numbers, and a cycle partially overlapping the private window is refused.

On x64 and x86, the cached Core executor fixture verifies ordinary SMIADS
read, write and observation, and retains the hard refusal check. The cached
MyOB300 board fixture executes a guest handler that both reads private memory
and writes ordinary RAM through the real App/Core wiring, then executes RES3.
Both source/test Core manifest checks also passed on both widths.

An x64 replay linked the production CPU bus with the immutable original card
and a read-only diagnostic DOS image. The guest entered APM suspend and F13
through the normal Machine input FIFO resumed DOS with event 0003. A separate
diagnostic SMI pulse at guest tick 268342488 then progressed past the old
9C4B6h failure, without a terminal Core error, and the final sampled PC was
F000:A316h in the original BIOS. This run did not issue a second resume after
that observation. The disposable DOS image SHA-256 remained
`24f235b0955949fc2c1d9a353a7a97e9483d1adf6268239012164d42750c03ff`.

| Production-link x64 diagnostic | SHA-256 |
| --- | --- |
| Guest output and final CPU sample | `e9c49746f623cc2408624a8cba94436d11efe2719f978880aa1fe6c3d3a1e45e` |
| Core execution trace | `4a4cb4dd06f370660e8538de7b4206c2c095bd2b8d8130e05795c8449599f15f` |

A second x64 replay logged successful production physical reads at
9C4B6h after the diagnostic SMI. Its attempted second F13 did not restore
interactive DOS. Comparing the sampled F000:A316h with the original ROM
instructions showed that the probe signalled during a delay loop. The
firmware checks the power key only later when it reads port EDh at
F000:A337h. This unsuccessful replay does not qualify a second wake. Its
read-only DOS image was unchanged.

| Early-wake diagnostic | SHA-256 |
| --- | --- |
| Guest output and final CPU sample | `ff23d6c24db8d0355d76842d921073d1988a67342d1c4b7adcebab684b49be0f` |
| Successful ordinary-memory read samples | `0af08004a5228f57f12b2f2ab4eaf6bcf2610db08195cd96c7a2b2f936c6ac65` |

In the next x64 run the diagnostic watched the original EDh reads at
F000:A337h and waited for that observation before sending F13 through
the normal FIFO. After F13 release it sent the keyboard scan codes for
`VER` and Enter. The guest displayed `C:\>ver`, emitted
`Hewlett-Packard MS-DOS 5.00 - E.00.00`, and drew a fresh `C:\>` prompt.
The replay itself returned 3 because its exit predicate expected the
generic `MS-DOS Version` string. The guest framebuffer and unchanged
read-only image prove the recorded DOS interaction. The SMI was still
supplied by the diagnostic, so this run does not qualify awake F13 as
the initiator of the suspend transition.

| Power-poll diagnostic x64 | SHA-256 |
| --- | --- |
| Guest output including fresh DOS command and prompt | `34d93258e5ae195408dfc41e34f011a8b6f7dfa8e609b5a33ae3117a01a6dbe3` |
| Original firmware power-port reads | `8cd75cc5c006eb61c8360806c475712420802a7f07158cf6854cbd7b1db2d5f6` |
| Successful ordinary-memory reads after the pulse | `f4c34917e7cc3ef51c3e18e8bd7ca007831ba20bd0b96fe7f372cf6684e90461` |

The x86 replay used the original EDh read trigger and matched the actual
HP DOS version string. It sent the second F13 through the same Machine
FIFO at 121,547 ms and released it at 121,804 ms. After the new `VER`
input the guest printed `Hewlett-Packard MS-DOS 5.00 - E.00.00`.
The probe exited successfully, and the read-only image was unchanged.
The focused x86/x64 checks and Core dependency and test Types gates all
passed. The diagnostic SMI, guest boot timing and separate desktop UX
still retain their stated limits.

| Power-poll diagnostic x86 | SHA-256 |
| --- | --- |
| Guest output with the fresh DOS command response | `806466bbea158ab40c02bbc1260100a48d0c4718bb1f9006d41c181bc1fae4a4` |
| Original firmware power-port reads | `ccb6e37da90f50aaffd2a9ef9a20c7f6474f8a8bdf8a97fe5b5e234726c98943` |
| Successful ordinary-memory reads after the pulse | `892a2a54708ed1e29c77edf76ded89ea106ec78d5fd62860602b321dcaf673f7` |

## SMM enable at the DOS idle boundary

An additional x64 replay observed the firmware's indexed SCAMP writes
without substituting register values. After the original APM call had
returned to DOS, the last successful SMMCTL (index 18h) write was A7h at
the diagnostic SMI boundary. The original DOS prompt again handled a
fresh `VER` after the diagnostic SMM/wake cycle. This fixes the input
condition for further work: in this awake DOS case the firmware had
enabled SMI, with BASIZ=7. A separate hardware event source for the
production F13 key is still open. The tracing replay did not test the
actual OmniBook wiring or an On/Off transition initiated by F13 alone.

The VL82C316 signal description and Table 40 document that an external
SMI request is gated by SMMEN. The related VL82C323 describes a rising
push-button event while awake and a separate wake role while suspended.
These are design references for the App, not identification of the
OmniBook's actual PMU wiring.
[VLSI SCAMP II chipset technical reference](https://bitsavers.computerhistory.org/components/vti/pc/VTI_VL82C316_VL82C323_SCAMP_II_199210.pdf).

| Awake-register diagnostic x64 | SHA-256 |
| --- | --- |
| SCAMP writes and last SMMCTL value at the diagnostic SMI | `6962f0e204d6004a64193471d0798cf68059f3a00c17de7d68049fcab127ab09` |
| Guest output and completion | `80f4add1e3c1c43b466a9e2afe76c33375e31c366feddb01dab7a74539dd09a7` |

A paired observation read the firmware's power poll at F000:A337h before
the first APM wake. The last SMMCTL write was 27h during software suspend
(tick 255000410) and A7h at the later diagnostic SMI from awake DOS
(tick 269044619). The trace SHA-256 is
`9d5add30d29e7e287b8d5d9175bcc6820fb178f274f3840c856d91a1d2dbb283`.
SMMEN can therefore gate the new App-local F13 rising-edge signal for
this firmware. The board keeps the existing power sense level, suppresses
new SMI requests inside SMM and uses Core's existing SMI input. The model
is L2 based on the chipset reference and on the observed ROM behavior.

## SMM release qualification

The Core receiver is Shared commit `f9b5eab867ad840262ba8bb31ab45ee20581f724`.
The paired NXVM delivery updates the MyOB300 board, board fixture and all six
PC releases. MyNES and MyRaidenII receive separate artifact commits. The
artifact bytes below match the cached Ninja publication or the final relink
in the same build tree. Both exact MyOB300 binaries launched in the isolated
native Window configuration, attached the original card and read-only C
image, and completed monitor `info` and `exit`. The original media hashes
were unchanged. This check does not exercise the F13 gesture in the native
Window; the original DOS wake observations above use the Machine input FIFO.

- x64 and x86 Release unit tests: 717/717 each.
- x64 integration: 47/48 in the first aggregate. The single MyNES snapshot
  transaction failure passed on its exact isolated rerun. The test polls raw
  RUNNING state before queue admission is necessarily open. It links MyNES
  and Emulator, independently of the changed X86 Core. No MyNES code or test
  was changed.
- x86 integration: the 300-second aggregate budget expired after 38 passing
  cases while starting the next MyNES case. The remaining ten registered cases
  passed in the continuation. No original test timeout or aggregate policy was
  changed. The resulting per-case coverage is 48/48 on each width, with the
  failed and incomplete first aggregates retained as such.
- Targeted Core dependency, Types and source/test manifest checks passed in
  both trees. The NXVM documentation governance check and diff review passed.

| Released executable | PE machine | SHA-256 |
| --- | --- | --- |
| nxvm_0_5_0592_x86.exe | 014Ch | `3d76d55d2a616eafb5f1f33f64bd29ad396e9a848a66cd50b690ce1bad042bba` |
| nxvm_0_5_0592_x64.exe | 8664h | `168571b05d38e94a5c675e1d4f464a923c9bded12c35b5b81b0e5baa15f870aa` |
| my5160_0_5_0592_x86.exe | 014Ch | `8a221a4dc285a00d469eefae88a2efe9e68504d0ea153fa9c0348a2cc6e08a00` |
| my5160_0_5_0592_x64.exe | 8664h | `bd514a35913aecbe4b80eb1c3079de326667a010a0eec4f0709e3b5f4cfca260` |
| my5170_0_5_0592_x86.exe | 014Ch | `13e06a595b7dc331f05a29d30e28bbf9ba74db73fe7a1862b585d1c57dfd1998` |
| my5170_0_5_0592_x64.exe | 8664h | `5f6d36b70affe4c504ac315e70b7949028e56c913baf4907444b790f1cd70dd4` |
| mydeskpro386_0_5_0592_x86.exe | 014Ch | `45681d997aa6328af1ab83199600de13e3c78d66019b369560c9bf4dd5fd9154` |
| mydeskpro386_0_5_0592_x64.exe | 8664h | `36a80c339a121447483b3a1e686640ecea0071401f452510bd2ab9fa91601f32` |
| mypc110_x86.exe | 014Ch | `e80f21d8d4d8688b1363d7aa71bbe6223f60a1bf7c2927ef85af18f78d95e4b6` |
| mypc110_x64.exe | 8664h | `71fc48a01f1e2cb4cc352873d97a4be6ede66b7a90c31856cf613b63e7b1a520` |
| myob300_0_5_0592_x86.exe | 014Ch | `51237e1543e27e6410a1d08fc1867c08612958d3fec43a257958136882d0c6f0` |
| myob300_0_5_0592_x64.exe | 8664h | `2a42f5015843bd8900f731f1a16597a614775b826d728d785a2e4479ccb9ca5a` |
| mynes_0_0_0044_x86.exe | 014Ch | `e362a56e74c6f1b60b864aec7f99d7c6fe57fa6d86d50391fc09391ec9a2f0f5` |
| mynes_0_0_0044_x64.exe | 8664h | `697d63cda216948a5e1fd6f6e121c6da951f5940d0b314971315c658772458b7` |
| myraidenii_0_0_0007_x86.exe | 014Ch | `4e0d1ce0b939d0632d4d83f2c03e630c344298b12e8b0cbae7b34a9ea1f39d64` |
| myraidenii_0_0_0007_x64.exe | 8664h | `30e671e17800906a8454536abc04c8331e4b73a24d27ea2ff2f2024cff96a1bd` |

## Production F13 suspend and wake

Both original System 1.0 ABA runs used the production App board and
normal Machine input FIFO. The DOS diagnostic first entered software
APM suspend. F13 resumed it and the original BIOS reported event 0003h.
A later F13 from awake DOS caused exactly one production SMI assertion
and its paired deassertion. The observer forwarded the real signal and
did not inject an SMI. At the subsequent original wake poll, a new F13
resumed the original session. Typed `VER` produced the HP DOS version
string on both widths. Both probes exited successfully and the
read-only C image retained its SHA-256. The guest executes its original
power handler throughout.

The pre-existing board smoke passed on both architectures with the
power input in place of its diagnostic SMI call. It checks disabled SMI,
entry on an enabled rising edge, no nesting on a new press in SMM and
no retrigger while the key remains held after RES3. It also preserves
the existing private and ordinary memory assertions. The production
change is confined to MyOB300. This qualifies the original DOS power
cycle under the L2 model. Windows session preservation and the physical
machine's complete power modes remain separate evidence.

| Production power diagnostic | SHA-256 |
| --- | --- |
| x64 guest output | `e22b28fc4305c7778ee3ef0d974d311d28df7baf21187d9f1a35f67ee653fe52` |
| x64 production SMI edge | `3b5fba67d3621ee9e83a37400ac4b8158de71558bd8c7ea3f092d91bdfd1c6a5` |
| x86 guest output | `c933c00981d73c41f1b59fe7cc9b524bed390494dac5b3ce1f9c7a9de7477e0c` |
| x86 production SMI edge | `05264d955f41e32d7303617a2f4d10f6c517d1ddf61a63e86cba56595996b737` |

The cached MyOB300 publication built both widths. Each exact release
started as an isolated native process, attached the external System Card
and read-only C image, and completed monitor `info` and `exit`. The
published INI and the external source hashes matched before and after.
No Shared source or receiver executable changed in this App delivery.

| Released MyOB300 executable | PE machine | SHA-256 |
| --- | --- | --- |
| myob300_0_5_0592_x64.exe | 8664h | `188a5d262237a16983b0d331c98fc73d40226933591d8277ce8a30c29b5de53c` |
| myob300_0_5_0592_x86.exe | 014Ch | `93c55bd0e8a73333dc00687092e83a5e02f65b040219d0e36e8d5b0f52bd6db6` |

## Original Windows power checkpoint

An x64 replay used the published P17 source graph, the original System 1.0
ABA card and the unmodified restored C backup in overlay mode. The runner
entered Program Manager, observed a broad on-screen region change after
Alt+F and then requested suspend through production F13 input. The App emitted exactly
one SMI pulse and the original handler executed. The original firmware power
poll at F000:A337h appeared about 65 seconds after the suspend request.

The runner then sent F13 to wake the guest. The replay ended without seeing
the expected fresh frame with the original menu, so it never sent its
planned post-wake Escape. The run generation stayed at 1 and the backing
C image hash was unchanged. This proves the Windows suspend entry and
guest wake-poll checkpoint. It leaves wake-key recognition, SMM exit and
preservation of interactive Windows state to separate checks.

| Original Windows x64 diagnostic | SHA-256 |
| --- | --- |
| Program Manager, F13 requests and terminal predicate | `729c6e51133948ddef66faf52143ba0ac737c30b031eb79b719c6d3775e65ddf` |
| Original firmware wake-poll observations | `c5d9b61fa2bfceb8cf88b8471e0ec1b6e3d36fc82e8d9ae83681f7176f179cf` |

The second x64 trace observed the first F13's SMM entry at tick 433793030
and its exit back to the protected-mode Windows execution address at tick
433798527. Later, the firmware read the second F13 as 10h at the original
F000:A337h poll. The resumed Program Manager title reappeared in generation
1. After a new Escape event, Windows published another graphical update.
The earlier checksum covered a changing part of the Main group beneath the
menu, so the differing full-region values do not decide whether the File
menu was preserved or whether Escape closed it. The next probe uses only
the File menu label and requires a fresh Alt+F and Escape round trip.

| Traced Windows x64 diagnostic | SHA-256 |
| --- | --- |
| Windows frame, Escape and terminal predicate | `575bfea22d5c20296b52c7f840e8805325d59c616a7fb73a736013e9701ab52e` |
| CPU SMM execution edges | `117314a90354393e6bb1a4e70c53ece89a428adeb7977b8f47cf919713a0a0cf` |
| Firmware wake check, including the pressed key bit | `0227b51ebf935c42afc663b7ccac041cb7291b765dd9c1d6f608dc79506a590e` |

The File-label replay established the closed label fingerprint from
an earlier captured desktop, then observed a new value after Alt+F.
The original BIOS read F13 at tick 579559059. On returning to the same
generation, the title and File-label pixels matched their pre-suspend
state. The first graphical output appeared about 1.5 seconds after
delivery of the wake key. The label did not return to the expected
closed fingerprint after Escape, so the post-wake input round trip
remains unverified. This run did not change the backing C image.

| File-label Windows x64 diagnostic | SHA-256 |
| --- | --- |
| Observed label restoration and Escape attempt | `097eacf3d64f8a79aafbfca5063c4234dd0be0513857c7e7e553ec93874efa07` |
| CPU SMM execution edges | `551cc6b4ce2cbc824dbed107d6f4be362c49e45a822df40475e6f658d78e88f5` |
| Original firmware wake check | `9531017de3ee6ef06c77170719ca728f139051daf00d84dc8471c888431c54e9` |

A lower-overhead replay omitted the per-instruction timing probes and
started the F13 cycle with the File menu closed. The original BIOS sampled
the second F13 at tick 580628866. Program Manager republished its original
closed File label (`d317880c`) in the same run generation 1 about 1.3 s
after the wake input. The probe then sent a new Alt+F sequence. The File
label changed to its active value (`3aa68941`) about 1.6 s later. This
establishes new Windows keyboard interaction after wake. The subsequent
Escape did not satisfy the expected closed-label predicate, so the probe
returned 1 and does not establish that additional interaction. The backing
C image remained byte-identical.

| Low-overhead Windows x64 diagnostic | SHA-256 |
| --- | --- |
| Guest frames and new Alt+F after wake | `0c1efb5a7356dfdf7a57dfc1c40e5ea509534c986087484f0a08595190c9ee0f` |
| Original firmware wake sampling | `1e5b16569d828eddf36cc409b076b108bedb3b1e5a417d66e57c5473169b4979` |
| CPU SMM execution edges | `2a2320f23368e4236a458fa627219d3021656a84c0b9b5bf6e1605df4a2376f3` |

The x86 low-overhead replay also ran the original card and C backup in
overlay mode. The BIOS sampled the second F13 at tick 580587724. Program
Manager restored the closed File label in generation 1 about 2.0 s after
wake input. A new Alt+F changed the label about 2.5 s after delivery.
The next frame had the same active label fingerprint (`3aa68941`) as
x64. The 280-second observation ended 235 ms after that frame, before
the separate Escape predicate completed. The backing C image remained
byte-identical. The Windows wake and new input checkpoint has direct
evidence on both widths. This is a guest frame/input check and its short
post-wake intervals are diagnostic, outside the controlled performance gate.

| Low-overhead Windows x86 diagnostic | SHA-256 |
| --- | --- |
| Guest frames and new Alt+F after wake | `b97f183209ed3f64eccd7f8cc855358fe7f5563fb3241a681d561dad63daf732` |
| Original firmware wake sampling | `7a9653326da239592e8e9da4c105c3bcd545cf7ef80fe44db2be416b2afcac4f` |
| CPU SMM execution edges | `b86fb9dc3ac5671018042be8585e07e00e4e015aa4fb77bb14f9ed696e479270` |

## Configuration warning after process restart

The original x64 BIOS displayed `Invalid configuration information` on
an unseeded process and continued to original DOS startup after F1. The
probe captured all 128 CMOS bytes through the existing Machine interface
after shutdown and unbind. The standard checksum was valid (`00E7h`). A
new process seeded with those exact guest-written bytes still requested F1.
Both processes used the matching original C backup in overlay mode and
left its backing image unchanged.

An observational port trace of the seeded process identified the separate
HP check. The original code at F000:B98Ah sums indices 40h through 64h.
The observed bytes sum to 023Ch, while the saved value at 7Eh/7Fh was
003Ch. During POST the firmware sets bit 6 at index 78h. The later
configuration-validation path reads 40h from index 78h and prints the
warning even though the preceding ordinary checks reach that point. Raw
CMOS persistence at this capture boundary would preserve the warning.
The probe used no fabricated checksum or production NVRAM file.

| Configuration-restart diagnostic | SHA-256 |
| --- | --- |
| First-run CMOS bytes captured after unbind | `66e94142ee67b587e0b78ffc742772d20de8a59b9dfcbff02e2d11175fb6a1be` |
| Unseeded POST, F1 and original DOS startup | `88fc93e05e1641f730f47c8bcda09c555ee425c34552a7e0f7cbe4c859c57dec` |
| Seeded POST still requests F1 | `00890526fc13f3f08e67e490d56e7503ef65a1f101acbb00d0d573185318dd7d` |
| Seeded POST CMOS port trace | `725ae7b73a059ba815a88525e5fa9e327e2fc1ffd4df132a33c9ad2f3ab72331` |

Repeating the capture at the original Windows Program Manager, with the
File menu label visible, produced the same extended sum (023Ch) and
stored checksum (003Ch). The next process again requested F1.

A separate cold trace identified how the inconsistency first appears.
The BIOS writes the extended checksum 003Ch before the later writes to
indices 40h–43h from F000:A031h through A04Dh. Those four bytes were all
zero during the earlier checksum calculation. The later writes contribute
0200h. The ROM wraps that record's writes in an index-14h bit-3 set/clear
pair. Its earlier read of that same four-byte record follows the same
switch sequence and verifies the signature and an eight-bit sum. The POST
checksum reader accesses the same indices without that switch. This
supports testing distinct storage for the four-byte record as an App board
model. The generic SCAMP II manual calls bit 3 CACHEN, so the original
OmniBook electrical decode remains a model inference.
[VLSI SCAMP II technical reference, Table 51](https://www.bitsavers.org/components/vti/pc/VTI_VL82C316_VL82C323_SCAMP_II_199210.pdf).

| Later configuration diagnostics | SHA-256 |
| --- | --- |
| CMOS captured at original Windows Program Manager | `fc2a766324d75cc12ebb39d88dbf6b8418c444131edbbf1a3c6028dd8dc6316b` |
| Seeded POST with Windows-captured CMOS | `6435f6694308d7647fcb39b830e0b3633dce16833ce1afbeaa3bba915700cb58` |
| Unseeded boot writes through original DOS startup | `0602b32ede5b9fba0b437393b7fa884b37d3b127730f97ee25d320812658afe7` |

An isolated I/O experiment separated the four selected bytes. The unseeded
guest still requested F1 on first boot. At DOS startup the captured extended
sum and the stored checksum both read 003Ch. A second process seeded with
that unmodified capture reached original DOS startup without receiving F1.
The backing C image hash remained unchanged.

| Isolated separation experiment | SHA-256 |
| --- | --- |
| Guest-written CMOS after first startup | `81d8ea2a0c20fa62dc502771dc5ab0575c0a8264571b0e7b40ca2fe62dabdcf9` |
| Unseeded first startup | `ff8a984b9cfffb41d74ede05afcc9001b0f915fd389e7ae9a0b9318241e96a5c` |
| Seeded second startup | `9eb543a624444eaa1ce53c7a29317e64d9828e8d6a8e5ee43d59e5e1dadabf9e` |

The production MyOB300 binding now selects the record's storage on the
existing Board-Base RTC data route. The existing MyOB300 board test exercises
both directions, selection changes, and an adjacent ordinary CMOS location.
The record belongs to the board instance. Cross-process CMOS storage remains
a separate product persistence obligation.

An x64 replay linked to the production board code repeated the result
without replacing the RTC port handlers. The first process reached original
DOS startup at 79.454 s after one F1. Its guest-written image was identical
to the isolated result. The second process reached the original DOS
checkpoint at 78.210 s without F1. These times identify the replay events
and do not replace the controlled DOS performance baseline.

| Production-path x64 replay | SHA-256 |
| --- | --- |
| Unseeded first startup | `200a2fb6570cf7a5b962ced25514d27d98926cacfaa59e489097b8fd07fd3581` |
| Seeded second startup | `ea61387fa8f9a51628c9b1abdd4476a4e23735af5b7bf2704accb608994a29da` |

The production-path x86 replay also reached DOS in two separate processes.
The first reached the checkpoint at 103.344 s after F1 and produced the
same 128 bytes as x64. The second used those bytes unchanged and reached
the checkpoint at 103.844 s without F1. Both widths matched the backing
C image hash before and after replay. The successful second boot on each
width was a diagnostic seed injection through the production construction
contract. The release launcher still starts without a persisted seed.

| Production-path x86 replay | SHA-256 |
| --- | --- |
| Unseeded first startup | `3da9876903a16816c69e0225c2d5d430eb8cacc6c8c3a5286edbd6a0965efe68` |
| Seeded second startup | `3c032c71a8fc0f8b8d9481feab58caf43d62b442359eaa3a109e017f85a47c1d` |

### Configuration-routing qualification and artifact identities

Cached Release rebuilt and deployed both widths for all eight applications
from the same reviewed source changes. An independent read verified PE
014Ch for x86 and 8664h for x64. When the test graph relinked the default
NXVM, MyPC110 or MyNES executable, its deployed copy matched the final
build-tree bytes. The production changes are +33/-4 and the existing
MyOB300 board test is +14/-0. Manifest entries were refreshed.

| App | Width | Final SHA-256 |
| --- | --- | --- |
| nxvm | x86 | `80b59aad64ede7457e3c30d01fd8a3a096d836add665f2082481a566e735e3f3` |
| my5160 | x86 | `8666de36662334b22bc8ac67a7d0d671140d3cfa9d3df657d2cbb9a79a7fd4ba` |
| my5170 | x86 | `d2190d001e35ec2d47091bcb65b525e8fa9dc04915595eb21f5af8b6b3187357` |
| mydeskpro386 | x86 | `6fd6312f1ad25fad816f057b64610a0269d472c7cadc357e8cd55b71434e78cc` |
| mypc110 | x86 | `9680851f7a2c79fc1876fd63dc5912b10e3ab11b155fa341f9a43867e01e4397` |
| mynes | x86 | `30f564d6a328cd56f491071598e48944ec4ad4618191e353789fae6494b35bf5` |
| myraidenii | x86 | `b9cb1ce7a054f05c9ec6c5233be2b21a66934177124c468fc162f0f26aec00f3` |
| myob300 | x86 | `2d79cf8dd7033347f7e131f354ff225802d6f26f75b9b99608a243198fe0a22b` |
| nxvm | x64 | `a9614a44650b3c843a157944c433b02cd815a302ab0df27347a27417c518de1a` |
| my5160 | x64 | `aa90db86ae4acac27a1b288832c29e397d488f2022566e9e9ff02ba1ebc8de16` |
| my5170 | x64 | `c72ba86560ed1f72cc8b01621334f753f0f4c62af66481aff4840cf574166f18` |
| mydeskpro386 | x64 | `e8edb9fe748bd0fd6495bfe512caee58226a60eaae923778d07851432bb86a3f` |
| mypc110 | x64 | `6be3d4a8051fed49d09cf8c5a196eba5a198a54be54ae90eb002d17b072f00d0` |
| mynes | x64 | `6a7c32b8b2ded1f6295805de65173fe4096c4fe370eb23ed8a09570b377395cd` |
| myraidenii | x64 | `082951fd0a414b48d916805d1065d42e48eea05098a5ed72b89d5f7b90a7edf3` |
| myob300 | x64 | `10f695a9f1fa44dabdaf5637e8b09dcb4d80a2be4fbf8e7b1d8a7da85e9e14d3` |

- x64 and x86 unit runs passed 717/717 each.
- x64 integration passed 48/48 in the registered aggregate.
- x86 integration passed 35 before the 300-second aggregate deadline
  interrupted PC110 cold boot. The exact remaining 13 passed in a bounded
  run, including PC110 cold boot at 77.28 s. No test budget was changed.
- Both published MyOB300 files accepted native Console `info` and `exit`
  against an isolated card copy and readonly C image. Original inputs and
  the tracked INI were unchanged. No new MyOB300 interactive Window/RDP
  test was run for this routing change.

### Product CMOS persistence

MyOB300 now loads `myob300.cmos` beside the executable and captures the
ordinary CMOS and guest RTC on clean monitor exit, after the worker has
shut down and the Machine is unbound. The file holds 128 bytes and is
independent of the storage attachment mode. The first missing or empty
file uses the board defaults. Invalid lengths and I/O failures are reported.
The implementation uses the existing storage writer. A single instance owns
this file while running. Save is a clean-exit guarantee, without an atomic
replacement or power-loss guarantee.

The native replay drives the actual published EXE from a different working
directory. It starts with an absent CMOS file, sends F1 only after the
original warning appears, observes the original DOS HP Card Installation
Client, pauses the guest, and sends `exit`. A second process starts with
the unchanged file from that exit and must reach the same DOS checkpoint
without seeing or receiving F1. Each run uses the original System 1.0 ABA
ROM and a readonly disposable C disk. No diagnostic wrapper supplies the
seed to either process.

Both widths produced the same 128-byte CMOS image on the first clean exit,
including the original extended checksum 003Ch. Both started a second
process, reached the DOS checkpoint without F1, and exited successfully.
Both rejected a short existing file before startup and left its contents
unchanged. In a separate x64 check the existing CMOS file was made readonly
after the monitor prompt. The save failed with process exit code 1 and the
previous file remained byte-identical.

| Verified output | SHA-256 |
| --- | --- |
| MyOB300 0.5.0592 x64, PE 8664h | `476e3113c372c31d15f3267bc33a02d1ea223cbb4b8f9b24e270311881854b2a` |
| MyOB300 0.5.0592 x86, PE 014Ch | `1bd3c62e7f97dd5472424d5ef5480c5bbbbfa35c804a34d7dfc9259ef9941206` |
| First clean exit CMOS, both widths | `81d8ea2a0c20fa62dc502771dc5ab0575c0a8264571b0e7b40ca2fe62dabdcf9` |
| Second clean exit CMOS, both widths | `090696bb0f7121116cb66948c271fa47db1e445de8c2fed2534cac049d07cb3e` |
| Readonly C disk before and after replay | `24f235b0955949fc2c1d9a353a7a97e9483d1adf6268239012164d42750c03ff` |

The cached MyOB300 board unit test passed on both widths. The diff is
App-local, +71/-6 production lines including the storage link declaration,
with no shared-corpus source change. The original card, owner configuration
and unrelated product artifacts were unchanged. No new Windows or RDP run
was performed for this persistence change.

### Published DOS performance handoff

The follow-up native Console replay used the exact P23 release executables,
original System 1.0 ABA card, a newly absent CMOS file and Turbo. Both host
widths run serially. The disposable C image retained the original CONFIG.SYS
and AUTOEXEC setup until the original Windows launch, then printed one
diagnostic marker and returned to DOS. Its readonly SHA-256 was
`34d5597e7b040ea7a8b09d84234eea118d7f2cd8934cc4fc6c5eceaadf90b67f`.

The probe observes the active native Console buffer. It sends F1 only after
the actual warning, then checks the DOS prompt, HP's VER output and a DIR
result containing CONFIG.SYS followed by a ready prompt. DIR timing begins
just before injecting Enter, after the full command has been typed. An
earlier attempt observed both DOS prompts but injected the command keys in
one burst and failed to complete VER. It is excluded from the command
comparison. The successful input feeds each make and break separately.

| Published EXE measurement | x64 | x86 | Gate |
| --- | ---: | ---: | --- |
| Start to F1 submission | 35.312 s | 43.016 s | Diagnostic checkpoint |
| Start to DOS prompt | 78.640 s | 87.500 s | Diagnostic checkpoint |
| Start to DOS VER | 79.015 s | 87.875 s | 60 s x64, 90 s x86 |
| DIR CONFIG.SYS through ready prompt | 6.015 s | 7.344 s | 1 s on both widths |

Both EXEs passed the fresh-input checks and quit successfully. The disk hash
was unchanged. x64 missed the cold threshold in this run and both widths
missed the DIR threshold. The single x86 cold result is under its budget;
the previous controlled x86 result was over budget, so its repeatable cold
qualification remains open. The successful Console trace SHA-256 is
`517ced3a70a9f7e8bdbd222aaddd466a29511fb0e855a0c195c3c331067f3bf2`.

The [T591 controlled worker measurement](t591-s4-ob300-dos-baseline.md)
remains the performance receiver's controlled baseline. The native checks
include actual Console transfer and display and do not measure guest-side
instruction cost in isolation. Core's Turbo runner skips wall-clock pacing
and still advances the guest scheduler at instruction retirement. A useful
first split for the receiver is CPU execution versus elapsed-time publication,
then board deadline observation versus device advance inside that publication.
The following worker sampling measures those paths as a diagnostic.

#### Original DOS execution-cost sample

The probes linked the same Release production libraries as P27 and used the
same original card, CONFIG.SYS and readonly C fixture as the native handoff.
A host-only observer measured each Core quantum and one call in every 257
for the nested routines, with the monotonic performance counter. The
observer queued normal KVM input and succeeded only after the real DOS
prompt and a new `CONFIG.SYS` directory entry followed by a ready prompt.
Cold-stage counters include the time while typing `DIR`, up to submission
of Enter. The probes did not modify the storage image or advance guest time.

| Measured on the worker | x64 cold | x86 cold | x64 DIR | x86 DIR |
| --- | ---: | ---: | ---: | ---: |
| Complete Core calls, directly timed | 89.31 s | 98.23 s | 6.88 s | 8.36 s |
| CPU refresh, estimated | 66.87 s | 70.40 s | 5.48 s | 6.20 s |
| Elapsed-time publication, estimated | 23.71 s | 31.29 s | 1.70 s | 2.45 s |
| Board deadline observation inside publication, estimated | 9.66 s | 11.23 s | 0.68 s | 0.89 s |
| Retired instructions | 45.31 M | 45.06 M | 3.29 M | 3.27 M |
| Guest ticks in Core results | 251.13 M | 249.65 M | 14.20 M | 14.09 M |

The sampling estimates are inclusive and approximate. The sums slightly
exceed the direct Core durations because of measurement and extrapolation
error. Use the observed ranking without treating estimates as exact
percentages. The ordinary release runs above remain the performance
comparison. A higher-overhead preliminary x86 probe also reached DOS and
DIR and was excluded from the table.

The CPU refresh path is the largest measured region in both workloads.
It includes decode, instruction boundary handling and memory bus work.
The result does not yet assign time among those subpaths. Clock publication
is the next measured region, with a material part inside the Board deadline
scan. The DIR results show that removing only the measured clock publication
cost would still leave the observed command well above the one-second goal.
The next causal measurement should separate the actual guest's retirement
work from decode/fetch, memory routing and boundary bookkeeping. Preserve
existing retirement, device wake and fault semantics in any comparison.

| Sampled replay | SHA-256 |
| --- | --- |
| x64 worker and guest output | `f35412b06c505f64c9c34439a0db708c823a3dc62d357758ad33913e70233ef8` |
| x86 worker and guest output | `5832d602aa81930ef8eb1549775fb515c2fdd8a6dca314629106a81baf7b5eb9` |

#### CPU timing and preparation detail

A serial follow-up substituted only an instrumented CPU object in the scratch
replay above. Both widths saw the original DOS prompt and a fresh
CONFIG.SYS entry followed by a ready prompt. Both exited normally and left
the readonly C image unchanged. The guest retired about 45.1 million
instructions before DIR and 3.28 million during DIR on each width.

| Worker region | x64 cold | x86 cold | x64 DIR | x86 DIR |
| --- | ---: | ---: | ---: | ---: |
| Complete Core calls, directly timed | 85.70 s | 106.79 s | 7.75 s | 9.24 s |
| CPU refresh, estimated | 65.18 s | 77.65 s | 6.11 s | 7.08 s |
| Instruction execution, inclusive estimate | 42.87 s | 50.58 s | 3.91 s | 4.49 s |
| Timing selection, inclusive estimate | 22.91 s | 28.14 s | 2.88 s | 2.72 s |
| CPU restart-state assignment, estimated | 4.60 s | 5.94 s | 0.42 s | 0.51 s |

The scratch object times one out of 1,021 internal calls. The x64 run
staggers sampler offsets. On x86 the instruction preparation, boundary and
copy samples overlap, so their individual totals cannot be used to estimate
exclusive preparation cost. Nested samples and performance-counter overhead
also prevent adding the estimates or comparing the two widths as a benchmark.
The production readings remain the acceptance comparison.

Instruction preparation ran about 53.8 million times for 45.1 million
retirements, and about 3.78 million times for 3.28 million during DIR.
Preparation is also called by branch-target lexical preview and exception
entry, so those differences cannot be assigned to one caller. Inspection of
the timing path found that formula metadata can preview a taken branch
target. That preview copies private CPU and decoder state, then calls the
same instruction preparation as execution. Keep the regular restart
checkpoint. For the performance receiver, first compare a narrowly scoped
reduction in branch-target preview work against unchanged guest ticks,
retirement metadata and faults. Reuse the existing timing-preview and
retirement-observation tests. This sample quantifies the timing region, but
does not quantify how much of it that follow-up would save.

| Sampled replay | SHA-256 |
| --- | --- |
| x64 worker and guest output | 05b9bbb916ac15a83d9f85bff0b5e8dd7301095e07b5d98de6c5eb8e89314e8e |
| x86 worker and guest output | 0636d504d262944bf436920ca0c81e2d9c0b49a77ad49edf11d92c349680d31d |

#### Direct branch-target decode sample

A link-only observer sampled the existing branch-target lexical preview
entry point from the production CPU archives. Both widths used the original
card and the same readonly disposable C image as the preceding replays.
They reached the original DOS prompt, returned to a fresh prompt after
DIR, exited normally and left the image unchanged.

| Worker region | x64 cold | x86 cold | x64 DIR | x86 DIR |
| --- | ---: | ---: | ---: | ---: |
| Complete Core calls, directly timed | 84.87 s | 105.84 s | 7.26 s | 8.01 s |
| Branch preview calls | 8,647,212 | 8,622,386 | 508,533 | 506,051 |
| Branch preview duration, estimated | 12.38 s | 15.06 s | 1.18 s | 1.23 s |
| Board deadline observation, estimated | 10.68 s | 14.17 s | 0.88 s | 0.92 s |

Every counted preview returned an available lexeme. The timing row uses
one in 1,021 calls and includes any fetch, decoder preparation and counter
overhead within that sample. It does not measure their individual costs or
prove an achievable speedup. The two estimates place preview and deadline
observation in the same order of magnitude. Even removing the entire
measured preview cost would leave DIR far above its one-second budget.
The performance receiver should bound a same-fetch-path preview change,
then measure release cold start and DIR before deciding whether to change
clock publication and board deadline handling. Preserve branch-target
lexical observations, code-fetch behavior, fault handling and exact guest
deadlines in both experiments.

| Sampled replay | SHA-256 |
| --- | --- |
| x64 worker and guest output | 047976d44b0583ffb2fd2b27fccb50c786e5194818102c987154cc0bac456068 |
| x86 worker and guest output | adfca132cbf712c3ee5759f1c4381440815eb79a7cbda375847514a98eb2851c |

### Product obligation review

| Design row | Evidence at P32 | Remaining boundary |
| --- | --- | --- |
| O07 input | Product DOS make/break, native monitor handoff and owner-tested original Windows keyboard and built-in pointer | Formal Windows interaction and redraw are part of the Windows receiver. |
| O08 power | Original DOS and original Windows resume via F13 have direct dual-width replay. Clean Product exit and restart preserve RTC/CMOS. Windows Power Standard and Off both initiate a fresh POST with valid configuration on both widths. | x64 reaches the DOS driver in both modes. On x86 the 300-second probe cap expires after the configuration check and before the visible driver. See the original Windows Power control path below. |
| O09 storage | DOS C/D access and writable readback are recorded in T591. An original DOS readonly write fails and returns to DOS on both widths. | Windows installation and file persistence remain with the Windows receiver. Live backing-file loss and card hot removal are unverified. |
| O12 product | Native Console release startup, monitor lifecycle and packaging are recorded in S2. P23 passed fresh real DOS startup, clean exit and CMOS reload. | Usability thresholds remain open in the performance receiver. No new independent desktop/RDP qualification was run. |

### Original Windows Power control path

The original restored SYSTEM.INI identifies `POWER.DRV` and selects the HP
`D:\OMNIBOOK\OBPOWER.DLL` OptionsDLL. The original Main group names Control
Panel and `C:\WINDOWS\CONTROL.EXE`. An x64 replay with the unmodified guest
startup saw the Program Manager title at 176.456 s and the original File
menu respond to Alt+F at 231.974 s. The first desktop frame alone did not
establish input readiness.

A second x64 probe used a disposable C image with only `shell=progman.exe`
changed to `shell=control.exe`. It loaded the same original System Card and
production Machine. The guest displayed the stock Control Panel and its
Power icon. Existing neutral key input moved focus from Color to Keyboard
and across to Power. The sampled frame after Enter showed Power selected,
its status description and the busy cursor. The replay reached its budget
before the Power dialog could be observed. Neither setting was changed, and
no power switch was injected in this run. The next observable milestone is
an opened Power dialog, followed by choosing a setting and checking the
actual F13 power-off and reset sequence.

| Power control diagnostic | SHA-256 |
| --- | --- |
| Disposable Control Panel startup disk, before and after | `790340b8f294a5a0e61b80a7481b9767bec395c0a2f2cdef26c3bf51fb8ffa68` |
| Stock Control Panel and Power icon | `4a9f04339fafb34525ff16cba904e51f05635fe4cd751085c99254ca7ac9abf8` |
| Selected Power after Enter | `c2d2c79cabb9baf5cf16fc08a3bcf2630ba18481733a8ceba3e66a2b3d75a1c1` |
| Control Panel replay | `3046b699ed3981edbc01e5aadd96ed2bc2544ad7fdec8e556d28c8c049fc4134` |

The next bounded x64 replay used the observed icon to open the actual Power
dialog. It showed Advanced initially. Keyboard navigation selected Standard;
confirmation closed the dialog and returned to Control Panel. The first F13
reached the existing guest power input, emitted an accepted SMI pulse, and
the firmware reached its wake-poll instruction at F000:A337h. This run
reached its limit before a second F13. The old session's retention or loss
on power-on remains unproven at this checkpoint.

| Standard selection and first F13 diagnostic | SHA-256 |
| --- | --- |
| Original dialog, default Advanced | `a16c04d34b4183fcabebd8a6d23256b7e68dfcefab3252891f0e6b72d16bb62e` |
| Standard visibly selected | `fda5157247a6a3955e05c33442d73790e37c56b72ad22bc237b3fa494fafd298` |
| Control Panel after confirmation | `33aa08ae796d48ccc42fe1b31e5b873ca8a446418a62a7bc04498738287d3b6b` |
| Original BIOS wake poll | `9147ca38d4e8d0e06dee6e55ef68ca95ba19b198661ed7ac2a24dacf074fa9e4` |

The automated x64 Standard replay observed the selected mode at 190.812 s,
confirmed the dialog closed, and sent the first F13 at 191.079 s. It sent the
second F13 at the actual BIOS wait at 201.130 s. The BIOS read the asserted
0x10 switch bit and drew a new OmniBook ROM BIOS 1.0 POST at 242.529 s.
The guest then loaded the original DOS BIOS Services driver. This confirms
that the old Windows session was discarded and firmware ran again. The
probe returned 1 because its exit predicate also expected a Core reset
notification. None was captured, so the reboot mechanism remains open.
The new POST asked for F1 with invalid configuration information despite
the initial process starting with known-good persisted CMOS. The probe
acknowledged F1. The trigger for the new warning remains to be traced.

| Standard power-on diagnostic | SHA-256 |
| --- | --- |
| Actual Windows input, BIOS POST and DOS driver | `7dd8182644be30b80b23cef64b40c484d649f710c93296df92f90fb9e51b1a7a` |
| Firmware sampled both wake switch levels | `bf5a8932684102c188c9a080c4cce0ba37cdef842f58f6e57624cebfeb3caf17` |

The x64 Off replay separately captured the original dialog displaying Off.
It confirmed the dialog closed and sent F13 at 230.632 s. The second F13
was sent at 245.981 s after the original wake poll was observed. The BIOS
sampled the asserted switch. The replay ended at 288 s without a POST or
return to the old Control Panel, leaving the result of Off power-on open.
The old C image and the isolated Control Panel variation stayed unchanged.

| Off power-on diagnostic | SHA-256 |
| --- | --- |
| Off visibly selected | `2e386b2a3cc165de5e680a29695b52c198baf8a106858a418df4fd04bdb1406f` |
| Actual Windows input and bounded terminal | `cadf8a9dc9c1efceee40345df35c048e281d80180fa881f0395a60a9c4470dfc` |
| Firmware sampled both wake switch levels | `f4c9bdbd9cbb9eb233d5afbf4cd84d34b37ee3f57af6861e61945a34b5c5bc39` |

The earlier Standard run that ended in the BIOS wake wait captured RTC/CMOS
with an extended data sum of 036Eh against the saved 003Ch. The mismatch
was present before the wake key and new POST. Its changed bytes included
44h–5Bh, outside the already isolated 40h–43h control record. This
identifies an earlier point to trace. The capture alone cannot identify the
writer, the SCAMP selector during the writes, or the intended physical bank.
Its SHA-256 is `e1f488b867ac87c087f7c8c0c61294b77cfb2fce08c15cc993b47c31aff0d356`.

The Standard trace confirmed that System 1.0 writes every index from 40h
through 79h with SCAMP MISCSET bit 3 set. F000:A516h enables the selector
for the rolling power record. The ROM shifts the existing record from
44h–70h into 4Dh–79h and writes fresh values into 44h–4Ch. F000:A5B7h
clears the selector. The later POST reads unselected 40h–64h and sees
056Dh against stored 003Ch, then writes bit 6 at index 78h. The same
selected routine reads time/calendar indices below 40h from the RTC.
Trace SHA-256: `950ed8947ba9aafa48804fa7dbf0eae96db8abb7c4ae725ca008eb7dbd61b117`.

MyOB300 now gives the observed 40h–79h power record one board-owned bank
selected by the existing SCAMP bit. The ordinary RTC remains the owner
of all unselected accesses, the time/calendar and the adjacent unobserved
indices. This extends the existing L2 board route and adds no shared
interface. The x64 and x86 board regressions check 44h and 79h in each
bank and 7Ah immediately outside the modeled range.

The x64 candidate repeated the complete Standard sequence. The POST read
003Ch and the stored 003Ch, left the index 78h diagnostic flag clear and
loaded the original DOS BIOS Services driver. The probe terminated
successfully and never acknowledged F1. The backing C and isolated Control
Panel variation were unchanged. The result verifies this firmware and
model route on x64. The x86 Power panel replay and an Off terminal remain
separate checks.

| Candidate Standard after bank separation | SHA-256 |
| --- | --- |
| Visible ROM POST and DOS driver | `1f1634e546d2889663e393de9d1a4b0a866ffbfd75b17e017bc2ccf861f2d0a3` |
| RTC bus routing and valid POST checksum | `4ade67e57f1e372ce86c8155e70c2491f4426e78bb6ad8ad803288d82c516d03` |

Cached Release rebuilt and published the two MyOB300 0.5.0592 executables.
PE checks and publication receipts match the on-disk hashes. Only MyOB300
production and board tests changed. The production diff is +9/-8 and the
board test diff is +13/-1. The existing INI and external inputs were preserved.

| Updated release | PE | SHA-256 |
| --- | --- | --- |
| MyOB300 x64 | 8664h | `44ed3a49c0cbe740854b6bf8072340cb619027dde5592a827599d4e7e4218971` |
| MyOB300 x86 | 014Ch | `974f1553a28fcf496753ed73624879b871c7a237eacc4adf07402cd37c62a5d2` |

An x64 replay on the same production revision independently confirmed Off.
The original dialog displayed Off and closed after confirmation. The first
F13 was sent at 192.430 s and the second at 205.713 s after the original
wake poll. Both switch levels were observed. The firmware drew fresh
OmniBook ROM BIOS POST at 248.846 s and loaded the original DOS BIOS
Services driver. No F1 was sent and the probe completed successfully.
This qualifies the cold-boot behavior in both Standard and Off on x64 using
the controlled Control Panel startup. The media hashes stayed unchanged.

| Off after bank separation, x64 | SHA-256 |
| --- | --- |
| Original dialog Off selection | `2e386b2a3cc165de5e680a29695b52c198baf8a106858a418df4fd04bdb1406f` |
| Original POST and fresh DOS driver | `f585669cf41b1423ecd3466bebb3b7d8e7748db5063d4808961fda1ff3e82102` |
| Actual wake switch levels | `0fda059204381356df4c3b921a51133f0f2b435deeada167c7de8b6f088d884b` |

On x86, the first Standard replay reached the actual original BIOS wake
poll and read the switch after the second F13 at 256.113 s. Before the
288-second terminal, the new POST routine read 003Ch from the ordinary
CMOS data and the same stored 003Ch, and cleared the index 78h diagnostic
flag. The probe had no time to reach its visible DOS driver terminal.
This is direct evidence of POST and the corrected configuration check,
with the remainder of DOS startup still unobserved in that run.

A second pair of x86 runs used the same original Control Panel shell with
the optional `WIN.INI` desktop startup programs omitted from a disposable
copy. The original DOS startup, Power driver and HP OptionsDLL remained in
use. The unmodified C backup and test disk did not change. In Standard and
Off, the actual mode was observed in the dialog, F13 reached the original
BIOS wake poll, and a new POST read the extended data sum and stored value
as 003Ch. Index 78h stayed clear. The visible DOS driver terminal lay beyond
the probe limit on both runs. These tests establish x86 reboot and the
configuration check, while x64 alone reached the visible DOS driver in
these Windows power flows. The general probe cap and performance gates
remain unchanged.

| x86 Windows power diagnostic | SHA-256 |
| --- | --- |
| Original-shell-variation Standard RTC trace | `5c82424f325098a583762ea3004dfaaaf0ce863823743fa09e78b16c6051c4d6` |
| Shorter x86 test disk before and after both runs | `0b79d2caf21ad4a424b609e69dbb4c0cdcda53e2be89230b526e36ac4c5a52c3` |
| Shorter Standard RTC trace | `9e8d70ec07cd3baaa867925eaacf2a1c673df6c55f0568c2b259029344d1222e` |
| Shorter Off RTC trace | `66a712b0d63120ba9c6187d591cec4b4f63ef3e1124c0ec316b42465d9c94f4f` |

To observe a later DOS checkpoint within the same limit, the follow-up
x86 fixture used Windows 3.1's documented `CONTROL.INI` `[don't load]`
setting to hide unrelated Control Panel icons. The original Power icon,
Power driver, OptionsDLL, DOS startup, ROM and power path remained present.
The first two probes reached the one-icon Control Panel but had assumed
the old left-column position. The next probes used the Power icon's
coordinates measured from that actual frame. The resulting disk hash is
`f6c53ca2ec36bdf963b0543d140bc7654b09388776012f091cb9b7076fb75088`.
[Microsoft Knowledge Base Q84709](https://jeffpar.github.io/kbarchive/kb/084/Q84709/).

| One-icon Control Panel, x86 | Standard | Off |
| --- | ---: | ---: |
| Original Power icon selected and Enter sent | 227.460 s | 228.075 s |
| First F13 after confirmation | 234.608 s | 235.493 s |
| Second F13, accepted by the original BIOS | 249.859 s | 251.093 s |
| Visible DOS driver at 288-second probe terminal | Not observed | Not observed |

The altered panel saved only a few seconds versus the complete panel.
Both probes advanced through the valid POST configuration check and
later RTC/CMOS work after the second F13. No F1 was sent and both disk
hashes stayed unchanged. This checks the same x86 firmware behavior with
a smaller startup fixture. It does not qualify the DOS driver terminal
or meet a performance threshold. The 300-second outer budget is unchanged.

| One-icon evidence | SHA-256 |
| --- | --- |
| Standard UI and wake | `8999d0fa5bb58110cfeaa8b2e60a0dd924058ae4f3493d782317417522d8132f` |
| Standard POST RTC trace | `eb42bb5776277aeab229d6673bbd2689c5f664134f3d2e2324df7ce37567c404` |
| Off UI and wake | `af8d4f4f0aa54431f82f49956540ff85cf7c5df075af58d627579352e120ff17` |
| Off POST RTC trace | `d364d36ab912e7e08aff275077c0ad6cbc1efbb7f5e4b360c595cdf92f6acd97` |
