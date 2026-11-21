# T592 S3 — Original C startup and Windows memory boundary

The published MyOB300 x64 0.5.0592 executable was started on an isolated
Windows desktop, with the original System 1.0 ABA card and the complete
restored C file tree. Both original archives remained read-only and the
disposable C image was mounted as overlay. The probe acknowledged the
firmware F1 prompt with ordinary host key input. The original Windows
standard-mode loader then reported insufficient extended memory, matching
the owner's Window observation. The BIOS reported 1792 KiB during POST.
The earlier independent DOS probe measured 959 KiB of free extended
memory after HIMEM loaded. These measurements are at different layers.
[DOS and memory evidence](t591-s3-ob300-machine-assembly.md).

HP's service manual lists 2 MiB RAM and Windows 3.1 as the factory
configuration of the OmniBook 300. The [VL82C316 reference](https://bitsavers.computerhistory.org/components/vti/pc/VTI_VL82C316_VL82C323_SCAMP_II_199210.pdf)
defines both the low RAMMAP bits that select the DRAM bank map and bit 4,
which can remap the A0000h–FFFFFh DRAM behind the top of supported installed
memory. The current MyOB300 stores RAMMAP writes in a startup latch and
uses a fixed flat Core memory backing/decode. It does not implement the
register's bank selection or remap. The previous original-ROM trace
includes a write of 10h. Table 1 lists map code 0 as a 512 KiB configuration
without 384 KiB remapping. The installed board's bank model and actual
firmware selection therefore need to be reconciled. An unconditional
bit-4 alias or increasing the configured RAM would not prove factory
hardware behavior.

The isolated native Window check observed a 640×400 client area, but
the background capture returned an essentially uniform compositor
surface. The owner screenshot independently demonstrates text visible in
Window mode. Host Window input, desktop/RDP handoff and preserved guest
state across an OmniBook power cycle remain separate verification gates.

## 384 KiB alias experiment

A disposable x64 build installed a disabled Core RAM alias for
200000h–25FFFFh backed by the existing A0000h–FFFFFh DRAM bytes.
Register 03h bit 4 enabled it without changing installed RAM, firmware,
media or loader. This intentionally ignored the low map code solely as
a diagnostic. The POST value increased by 384 KiB to 2176 KiB, but the
unmodified Windows loader reported the same memory failure at 96.110 s.
The unmodified release reported it at 95.187 s. A different POST count
therefore does not establish that Windows can use the space. No production
decode change was made.

A second diagnostic inserted a DOS query immediately before the original
Windows launcher in a disposable C image. The alias variant returned 053Fh
(1343 KiB) for both total and largest free XMS, with no XMS query error.
This is 384 KiB more than the earlier 959 KiB DOS reading of the released
model. Windows still reported the same failure. The added upper range is
visible to HIMEM's free-space query. This observation does not show that
the loader accepted a block or that protected-mode access to it completed.
The stock leg of this paired probe did not reach its input milestone.

An isolated allocation probe immediately before the original `D:\WIN`
call allocated and released a 1024 KiB XMS block with AX=1 in each case.
Windows then displayed the same error. A subsequent in-guest tracer observed
the original code using the normal HIMEM entry point. It changed no XMS return
values and the App loaded the original System Card. This run used the existing
Emulator Machine worker without a native desktop frontend.

| Observed loader sequence | Result |
| --- | --- |
| AH=08h, query available memory | Largest free and total both 053Fh (1343 KiB) |
| AH=09h, allocate 053Fh KiB | AX=1, handle A902h |
| AH=0Ch, lock A902h | AX=1, physical address 00110400h |
| AH=0Bh, memory moves | Four calls returned AX=1 |
| AH=08h, query later in startup | Largest and total zero, BL=A0h |
| AH=09h, allocate 0868h KiB (2152 KiB) | AX=0, BL=A0h |

No successful free of A902h was observed between these calls. The trace
establishes actual allocation, locking and later exhaustion. A second run
also recorded the original DOS EXEC calls. LapLink completed its startup
before `C:\WINDOWS\OBWIN.COM` began. OBWIN queried XMS with 1343 KiB
available, and the allocation of the full block followed the EXEC of
`C:\WINDOWS\SYSTEM\RSWAP.EXE`. The later allocation came through
`F4F7:0BAC` (physical F5B1Ch). The original Card bytes at this location
match a far-call wrapper around the HIMEM entry pointer. Thus the failure
lies after the original swap stage in the ROM/DOSX memory handoff.
The exact owner that should release or reuse A902h remains unresolved.
[Microsoft XMS
3.0](https://ps-2.kev009.com/basil.holloway/ALL%20PDF/Microsoft_XMS_3%5B1%5D.0_Specification.pdf)
defines BL=A0h on the free-memory query when all extended memory is allocated.

The first tracer substituted the INT 2Fh function-4310 entry. Its dispatch
lacked the patchable five-byte prologue required by XMS 3.0. Microsoft
documents that the [standard-mode swapper hooks that dispatch](https://ftp.zx.net.nz/pub/Patches/ftp.microsoft.com/MISC/KB/en-us/83/008.HTM)
to share XMS with DOSX. Thus the exhaustion sequence above applies to the
instrumented version and cannot, by itself, attribute the stock failure to
an unreleased RSWAP block. The unmodified original-media error and the
simple allocate/free diagnostic remain independent observations. Parent
return addresses from the instrumented run confirm the embedded DOSX
query and failing request at F4F7:1CF7 and F4F7:1D38.

The next tracer kept the original INT 2Fh vector and hooked the actual HIMEM
dispatch using the documented chainable prologue. RSWAP did participate:
its own call sites handled the intercepted query and made a zero-length
allocate/free pair before forwarding the DOSX request to HIMEM. HIMEM still
reported exhaustion. This trace captured a failed request of 085Eh
(2142 KiB). At the failing arithmetic, DOSX's data segment was 4D8Bh
in conventional RAM. Its descriptor-table size input was 1FFEh entries
and the 32-bit work-area field was 64207470h. The original routine turns
these inputs into 085Eh after 16-bit truncation. This is evidence of an
unexpected work-area value. A subsequent CPU bus probe confirmed the
writer. DOSX zeroes this field in conventional RAM. The original firmware
then writes across it from the protected-mode copy at linear FDBADh.
The bytes around the write match original card data at offset 804E5Eh.
DOSX's later alignment changes the low byte to 70h, producing the observed
64207470h. The write address was already in conventional RAM at the CPU
bus boundary, before board memory applies A20 masking. The original copy
contract and why it reaches that part of RAM remain under investigation.

## High System Card window correction

The observed firmware copy starts with DS base 00802070h, a conventional-RAM
ES base, SI and DI low words zero, and CX=0BCCh at the doubleword copy.
The firmware itself supplies the 00802070h source and the corresponding
end at 00804FA0h. The address and 12080-byte range reach the Core bus as
requested. The App had treated that physical address as a card-file
offset. At file offset 804E5Fh, the bytes landing in DOSX's work-area field
are 64207465h instead of the structured value 000008F0h found at card
offset 4E5Fh.

With the original ROM, the same C image, 2 MiB RAM and no XMS tracer or
RAM alias, subtracting the high window base before reading the card passes
the former out-of-memory checkpoint. The original mapping reproducibly
reports the memory error. The candidate run did not establish the Windows
desktop. The corrected App retains the existing modeled address extent,
window priority and immutable-card ownership. The board test supplies
different values at offsets 2070h and 802070h, checks that the lower offset
is read via the high physical window, and retains its read-only assertion.
Cached x86 and x64 App builds and the board unit passed. The same x64
test object failed the byte comparison when linked against the previous
card implementation. A separate x64 replay linked the corrected production
libraries and observed the unmodified guest for 205 seconds. It reproduced
HIMEM and completed LapLink initialization with no insufficient-memory
diagnostic. A second x64 replay armed frame capture after LapLink completed
installation and observed a mostly black 640×480 graphics frame. After 155
seconds, the regular Machine pause/debug interface sampled CS:EIP 2F98:0613
in real mode. The bytes match LLREMOTE.EXE in the original card file view
at offset 1213h, in a loop testing its byte countdown at DS:065Bh. The
extracted file contains a separate decrement at offset 2467h. This sample
narrows the follow-on investigation and does not establish whether a
timer, interrupt delivery or another execution condition prevents progress.
The Windows desktop was not observed.

### LapLink timing observation

A repeat used the existing Machine pause interface at three separated
points after the first 640×480 frame. The CPU sampled different locations
in real mode. The final sample was again at LapLink's countdown, with
interrupts enabled and the countdown value still 2. The BIOS data-area
tick count advanced from 33 to 34 before holding at 34 in the final sample.
INT 08h consistently pointed to 3347:1F10h at the three boundaries.
The card's original file view shows that LapLink registers an INT 08h
handler and contains a routine that decrements the countdown. These
observations justified tracing PIT, PIC and vector dispatch separately.

A bounded CPU bus and PIT-output trace raised the last IRQ0 at about 98
seconds of wall time. The processor acknowledged vector 08h from
02F7:A543h. The last master PIC mask was B8h, which leaves IRQ0 unmasked.
No master EOI was observed after that acknowledge. The PIT continued to
generate 164 further rising edges through the end of the replay at 155
seconds. CPU writes still set LapLink's countdown to 2, but no data-area
tick write or subsequent interrupt acknowledge was observed. This
excludes a stopped PIT clock for the reproduced stall. The next check is
the actual instruction path and PIC reads after the last acknowledge.

### Protected-mode interrupt return

Instruction snapshots from the last IRQ0 show the protected-mode handler
attempting a 16-bit RETF from 0070:124Fh after copying the interrupted
task's LDT selector (21AFh) into the outgoing stack frame. The next
executed instruction is at the #GP entry 0070:1147h. When execution
returns to LapLink, its OCW3 ISR read returns 01h. No master EOI appears
after this IRQ0. The subsequent PIT rising edges cannot preempt the
IRQ0 still in service.

The CPU was rejecting an LDT selector for an outer RETF stack before the
existing descriptor-table lookup. The same early restriction existed for
outer IRET's code and stack selectors. The fix removes those three early
rejections, preserving the existing selector/descriptor validation and
task-specific rules. The Intel 80386 manual defines these checks in
[RET](https://read.seas.harvard.edu/~kohler/class/aosref/i386/RET.htm)
and [IRET](https://read.seas.harvard.edu/~kohler/class/aosref/i386/IRET.htm).

The existing outer-return unit now exercises an LDT stack for RETF and
independent LDT code/stack cases for IRET. Both widths passed this unit
and the MyOB300 board unit. The Core dependency check and both changed
manifest checks passed. The stable x86 and x64 Release trees also each
passed all 717 registered unit tests, with 135.45 and 339.81 seconds
respectively of wall time. Both widths then passed all 48 registered
integration tests through the existing repository aggregate, including
native-window and device cases in that label. The x64 route took 247.30
seconds and x86 took 272.39 seconds. This aggregate does not run the
original OmniBook Windows desktop scenario. Final publication is tracked
separately.

A corrected x64 replay passed the exact IRQ0 checkpoint at the same
guest tick. RETF entered 0053:0E38h with SS=21AFh, the BIOS updated
its tick count and sent the master EOI, and a later LapLink ISR read
returned 00h. The replay recorded 296 IRQ0 acknowledgments from this
checkpoint through the 210-second run. With the stock C backup and 2 MiB,
the 640×480 frame at 154.937 seconds renders the original Microsoft Windows
Version 3.1 splash. At the end of the run, the firmware's idle routine
was still receiving and acknowledging timer interrupts. No desktop or
input/Office acceptance is implied by the splash.

A separate x64 replay without CPU/PIT bus instrumentation rendered the
splash at 161.717 seconds. At the end of 280 seconds the capture still
showed the splash. The paused CPU was at F000:9B69h, inside the firmware's
SCAMP sleep-control polling loop. This loop was already decoding guest
timer interrupts in the instrumented run, so a single idle-boundary
snapshot does not establish why Program Manager has not appeared.

The follow-on diagnostic used the same guest media and wrapped only the
existing CT-VGA write and capture entry points. From wall-clock 167.700
through 279.620 seconds, all measured 640×480 captures succeeded.
The VGA generation, CRTC start, graphics memory map and cumulative write
counts stayed constant while guest retrace count advanced by 1,351.
The App made 6,117 further captures of that unchanged generation. This
shows that a newly drawn desktop is not being dropped at the
presentation boundary during the measured interval. The stable frame is
also a measured source of repeated planar conversion. A temporary
comparison suppressed only unchanged graphics captures. The comparison
still displayed the splash and did not reach the desktop in 280 seconds.
During the first minute on the same stable splash generation, modeled
retraces advanced at 12.60 per host second with suppression versus 12.20
in the normal run. These are single runs with uncontrolled background
variation. The eliminated raster work is real, but the rate comparison
does not establish a large speedup or explain the missing desktop.
Suppression was confined to the temporary experiment.

### First protected-mode exceptions at splash

The runtime CPU diagnostic callback placed four additional exceptions
within 15 milliseconds of the splash marker in one x64 replay.
The sequence begins with #GP(038Ch) at 03A7:016Fh, followed by
#GP(00E0h) at 02F7:62B2h, #NP(0404h) at 041F:004Bh and #GP(00E0h)
at 02F7:6451h. Protected execution was still sampled immediately after
those exceptions. All 1,429 periodic CPU samples from 135 seconds onward
were in real mode, with most inside the BIOS sleep-control poll.
A follow-on capture observed the same sequence and recorded the faulting
instruction bytes. The leading #GP(038Ch) arose from the actual two-byte
`INT 2Fh` in the Windows path. It identifies an LDT selector rather than the
interrupt vector. The protected INT code was rejecting LDT code targets
before checking the descriptor, the same category of error as the earlier
RETF failure. The [Intel 80386 INT reference](https://read.seas.harvard.edu/~kohler/class/aosref/i386/INT.htm)
requires a valid code descriptor within the selected table. The existing
CPU test now dispatches INT 2Fh via both 16-bit and 32-bit interrupt gates
to an LDT code descriptor, using the existing ring-change fixture. The
case fails against the baseline and passes with the scope-limited fix on
both host widths. No TSS, PIC, task-gate or firmware behavior was altered.
The corrected x64 replay no longer delivered the splash-coincident
`INT 2Fh` #GP. It drew a distinct 640×480 HP OmniBook frame with an
hourglass at 148.642 seconds. In contrast to the earlier real-mode-only
tail, protected and real-mode execution continued after the splash.
The last periodic CPU sample arrived at 161.598 seconds. Frame capture
also ceased during the same interval, and the subsequent pause request
did not yield a snapshot. The first replay did not identify the Machine
state or reason for the stop. The next diagnostic observed the existing
Machine state and Core execution result directly.

Two further x64 replays recorded a terminal Machine error after the HP
screen. The first placed it at `2616:1953`. The second identified a byte
read from port `0xED` while index `0x3E` was selected via port `0xEC`.
The board returned `LIB_STATUS_UNSUPPORTED`. The original `OBMGM.COM`
executes this access and the observed caller discards the returned byte.
MyOB300 now returns zero for that read as an L1 compatibility value.
Unrecognized indexes still use the existing dispatch. The board smoke
test exercises the indexed byte read through the normal Core bus.
Both x64 and x86 passed the board and outer-return tests with the chipset
read fix. The next x64 replay passed that read and turned up a separate
fault after three graphics captures, at CS:EIP `02FF:0EA9` with an internal
access exception carrying `0x00C2CE34`. It did not show the original
insufficient-memory message and did not reach the desktop.
A separate x64 replay wrapped the existing physical memory calls. It
reproduced `02FF:0EA9` and confirmed a two-byte read at `0x00C2CE34`,
returning `LIB_STATUS_INTERNAL_ERROR`. The address is outside the current
high Card route. The HP service manual's page 68 memory map covers the
conventional upper-memory area and does not establish the high aperture's
full extent. Any larger route needs separate evidence before adoption.
An x64 diagnostic-only link extended the range to `0x01000000` with the
same window-relative addressing. The original guest continued executing
after its former fault, with protected-mode samples through 269.659 seconds.
The bounded replay still showed the HP screen when paused. A second replay
captured 64 successful guest byte and word reads in the `0xC2CExx` range.
Its card-relative offsets contain structured data next to HP Phone Book
resources. Skipping unchanged graphics conversion did not avert the later
slow phase, which still took about 12–14 seconds per 65,536 refresh samples.

The App now exposes the existing high common route across the upper half
of its modeled 24-bit bus as an L1 decode. This preserves the observed
window-relative offset and the separate raw reset route. It is not a
claim about the full physical decoder. The existing board regression
checks distinct synthetic bytes at the fault address as well as cold
reset and the earlier low offset. Both host widths passed their focused
board and Core tests, and both Core manifests verified.

The production-library x64 replay reproduced the successful high reads
and then a marked slowdown inside `x86_core_machine_run`. A representative
4,096-instruction quantum took 911 ms while advancing 21,777 guest ticks
(about 1.09 ms at 20 MHz). The quantum completed on its instruction
budget and normal frame conversion ran outside the timed call. The
bounded run remained on the HP screen. No desktop or application
acceptance follows from passing the address fault.

## RTC alarm deadline cost during Windows initialization

Splitting the same protected-mode quanta put about 98% of elapsed time
inside `x86_core_machine_publish_elapsed_ticks`. The host counter ran at
10 MHz. In one 757 ms quantum, clock publication consumed 735.6 ms and
CPU refresh consumed 19.2 ms. A separate link measured the RTC deadline
query itself. At the slowdown, the guest enabled only the alarm in BCD
24-hour mode (`A=0x26`, `B=0x22`), with the programmed alarm `24:00:00`.
The calendar was at `00:00:22`. Each query returned invalid state after
a complete 25-hour search, with sampled query costs around 0.16–0.21 ms.
The board asks for that deadline on the ordinary Core scheduling path.

The RTC now skips hours and minutes whose alarm fields cannot match and
jumps to the next candidate second inside a matching minute. Rollover
continues through the existing calendar increment, including the DST
skip and repeat. The preview still returns the earliest supported IRQ
deadline without changing clock state. No new scheduler state or public
interface was added. The existing calendar fixture now includes a distant
alarm and a noncanonical BCD alarm. Eight focused tests passed on each
host width. A temporary reference check compared the previous and new
IRQ status and tick results across 8,192 fixed-seed cases, including DST
dates, wildcards, binary and BCD clocks, and out-of-range fields.

A production-library replay with the optimized RTC used the same original
Card, restored C image, 2 MiB of guest RAM and execution observer. Between
refresh samples 80,019,456 and 80,084,992, both runs reported identical
sampled guest registers. Host time fell from 14.040 s to 0.250 s. The
updated replay reached 124.6 million samples inside the 280-second budget
and observed two additional graphics signatures after the HP splash. The
observer did not save these frames. At pause, the guest was at `F000:9B5D`,
near a poll of the SCAMP sleep-control register. Screen contents and the
purpose of the polling loop required direct follow-up.

A separate x64 replay preserved the new frames. At 181.767 seconds the
original 640×480 guest image contains the HP wallpaper, Windows Program
Manager and the Main group. At 201.400 seconds it shows the hourglass
inside that group. The keyboard and application paths still need direct
verification. The BIOS poll was observed reading both `0x85` with PIC
output low and `0x05` with PIC output high, confirming that requests can
wake during the late run. Repeated visits to that loop are therefore not
evidence of a stuck sleep latch by themselves.

A following x64 replay detected the Program Manager title three times
and delivered Alt+F make and break events through the Emulator Machine
queue. It observed further frame changes, but the retained frames show
only initial Program Manager drawing. Queue acceptance and a changing
frame do not prove that the Windows menu processed the keys.

An x86 replay also drew the original Program Manager. It recognized the
title frame at 219.057 seconds and captured the actual Main window at
252.058 seconds. Four live reads from the emulated keyboard data port
returned `38`, `21`, `A1` and `B8` after the second Alt+F request. Thus
the original guest consumed both make/break pairs. Its captured window
still showed the hourglass and no File menu. The 280-second diagnostic
ended with continued guest execution. Neither a Win32 input defect nor
a completed Windows interaction follows from these results.

## Local receiving artifacts

The RETF/IRET and Card baseline, before the newer INT fix, passed the
full unit and integration results above. On that baseline, the
supported CMD entry rebuilt and deployed all 16 executables using
the two stable Release trees. Each App directory contains its expected
x86/x64 pair with the correct PE machine identifiers. Both deployed
MyOB300 byte sequences match their respective Ninja outputs. On a
separate native Console, the exact x86 and x64 bytes launched from a
foreign working directory, reported the restored C image as connected
and completed `info` and `exit` successfully. These checks cover the
process entry and external media lookup. They do not qualify the Windows
desktop or imply remote publication.

The later Card, INT and RTC candidate was rebuilt and deployed locally
through the CMD entry for all eight products on both widths. The
sixteen PE architectures were inspected. Fourteen files matched the
publication log. The x64 MyNES and MyPC110 files matched the subsequent
Ninja relink caused by the unit build's existing copy steps. Both MyOB300
files matched their publication SHA-256 and were copied to the separate
owner-media verification layout. Their actual native Console launches
reported the matching C disk connected and completed `info` and `exit`.
The ROM, disk and INI digests stayed unchanged during this check.

The final cached qualification passed all 717 registered unit and all 48
registered integration tests on each host width. The x64 runs took 307.73
and 254.00 seconds; the x86 runs took 126.14 and 285.67 seconds. These
results cover the registered suites. The separate native-release checks
and the owner-media guest replays provide their own evidence above.

After the suites exited, the CMD entry published the same source revision
once more to refresh release timestamps and capture the final files.
All 16 repository assets matched the final publication SHA-256 and their
expected PE architecture. The exact MyOB300 pair once more completed the
native Console launch and disk attachment check with the external media.

| Capture | SHA-256 |
| --- | --- |
| Published Console with original C | `ec64807dfcb893e78391df513721a34b04b47d54d2771f3694c084a86fdc16fc` |
| Published Window sizing check | `a1278327750f24de65bae5db90f369709abc29065ac5aed0dda36ed40dbc65e9` |
| Diagnostic RAMMAP alias with original C | `f577a68f051f839200cfdf47516b9194f282717e78a61f34be8b3474b6cfe112` |
| DOS XMS allocate/free probe | `6bdd4c20827705ce184a8d99cb566344d69f60ff3c6067dc31662f4f3a4387ed` |
| Original Windows XMS calls | `fb6f697faa32df7ff862ee6b53eb657c42635e5ab676d0a89feb1b0ab8c5239f` |
| Original Windows frame observations | `f0d815ebbf29d587b56c3740c538cea6e853c61a3c5733098441564e9310c105` |
| Original DOS EXEC and XMS call sites | `0dc423d93a3384c5eafce365ec81f532de20b7365ec527ed28c0808da8bda68f` |
| Frame observations with call sites | `77b188e62680fabe72215ff19cd5ddeaf0d9e365d976d3b246ef0a97a40199c2` |
| Chain-compatible XMS calls | `db0630527f09a8973ecda6fd3483133e98100a60dd7c7dd426d695a60833bdb5` |
| Chain-compatible frame observations | `d5e1ff3c918fd9a58053d13571cd6c517a87349946c670f312fa9a049b0f6f9c` |
| Chain-compatible DOSX input fields | `05c7ad63fb6d2488b205156697a44c966b31bd1fb76d80112ed98234e824b008` |
| DOSX work-area write observations | `5e4af5b3484387060002c63b13d28c8e24e9170071b2290bee96b874d8fb7f89` |
| Copy registers at the original write | `9ac63968037b1a96ea471c0052f0f3707d364a6d141b147d7c42a5ae50101eb9` |
| Stock ROM and C with window-relative card addressing | `00b9a7336319835e5c4fefccd5f23655c43d16a309a34c36cba65251a03a48b4` |
| Corrected production libraries, stock ROM and C | `72376cf7ee7e0920e3f9514ae7abd32de9a3423864e9d514ad4014e18666f54f` |
| Corrected production libraries, post-checkpoint frame and paused PC | `75aa907da44d776d2369611158e3dc24c1cc1addbda1b6b20b1789724e79e4b8` |
| First post-LapLink graphics frame | `66db5294d37aa673b0da76b7e14aec2e2a8b7f69d21f1cf1b1df3244c0ff1803` |
| Three post-transition CPU, IVT and countdown snapshots | `acf6af2f28c255b9889cc9d7b9fce651b2595d230bf0ac60a951fd1d1af10cd6` |
| Bounded PIT outputs, port writes and CPU acknowledge trace | `db7d2f16884ad330dac5991212beec72948395e01fe0466dbdbb53fe60905eba` |
| Original failure, protected IRQ0 instruction trace | `2c4988bc18040f41b7ca325c11b28cb7d08d24da1152b2565cd4e2dfa43052a1` |
| Original failure, master PIC ISR readback | `4ccc2a3e8101e1f296826663b486f8b0c926ed2b6692155232ca526be0392f7d` |
| Corrected x64 replay with Windows 3.1 splash | `b44e6361708a47f360c49b33df9c9bc59af339cc2ad5874fb51d1b69f2c272ce` |
| Corrected x64 timer trace | `d5d7d68950856a48d4a0ac5b454a3540258674e3c73b810975a7f0929c5789f7` |
| First sampled Windows 3.1 splash frame | `a525fe24e19d1374f325a0ce5d43a5661476f6eccf4f7bc8ae8d02f1b8913804` |
| x64 full registered unit label | `3e1cc7a75f7e649449eadd3846d083463e1b077a19f1c7f3580e5b971c80b650` |
| x86 full registered unit label | `2fb74aff803f3e8193d1d6a58fad9b429651c0315c55ded50f8b1412653f0200` |
| 280-second replay without CPU/PIT instrumentation | `d699440d50ead4e37e15be1e2ce4e4f3b51d708fb411cdbdefe818a6d95a436a` |
| Splash frame from independent replay | `477f76c026f8948d68ad8f4ec98650293a388b0d1ff1cc7f653b5c5b29cb8a15` |
| x64 and x86 registered integration aggregate | `61f0bf062322195c3e671faff46404a30cf16558dd152fc4489be62703a66e29` |
| Original-media VGA write and capture observation | `1b542eada5b299614c3b92b5f3fa1f73aa500cca8271debe5b1880453d16438f` |
| Original-media VGA replay display and paused PC | `9cdaee09a7f16cb7a5f69202fd3b34a24f195e17a43b40c7ba1f6d06ba118b52` |
| Unchanged graphics conversion comparison | `ef0faedc6e4dddbdece2bc712d29c8a0b470a30242ffc77feb854bb6651b9cc9` |
| Local CMD build and deployment log | `9482b5c1400152b2754d96b33c1e089080480aa4ebfc6867046ba51c3cd7d85b` |
| Local 16-EXE PE and SHA-256 inventory | `8d4efceafe7be870b6feecde392a440381d2511048e5fc18162746a3ef7e6c1b` |
| Exact deployed MyOB300 native entry, both widths | `51ef6760e94ba212eca184e519241a0e54d53dc2d945e2940681b76188f39586` |
| Splash-coincident exceptions and later CPU mode samples | `30b6d9dae72471854d04b298290ded24b2e6ded8b9eee6c5237d6b3bb105bd31` |
| CPU-diagnostic replay display and paused PC | `c53fa09d5bdd19c34212e30d2016ad5c350d2c24a61bad4057705759607bdfd4` |
| Fault instruction bytes, selectors and descriptor tables | `d1930b8d7eeaf109d0415f2bad3d8ddbfe9ae3a93fe354217982ac7aea0dbf01` |
| Corrected INT replay through the HP OmniBook screen | `b9ed7c7a207c6528f8bcfe890f03bf18b08a6c7bb4d573171f615647f868c806` |
| CPU progression after the INT 2Fh fix | `3927825f57a821d4523351e22eb60cb9b4cd63d246ab23d406ed500defe65ca1` |
| HP OmniBook and hourglass frame | `da9c0853f17c40adf8418522591e6f6e74cbdcfaca2b8cc5d04bd1d7e4b70fcf` |
| Terminal Machine state after the HP frame | `45c8b9465be38c6386bc022df50b21e7b7017a45ecbdecd914fe3cf266bdc8be` |
| Failing byte read at indexed port EDh | `08410a2d0ec4e706de8cda7b538d18e4cfecd460471d9c1416ad269e30e22bf4` |
| Original replay after the indexed read fix | `0d6da930db44ea2907ebedc2aed369f2bf8637fe98a881db161b642e4e9fb8c2` |
| CPU samples and later access fault | `feb913bd7973e69d68ec37f807aed1d9dc08fb0aa62163b6888e70386cc8a0bc` |
| Physical read failure beyond the current Card window | `4b3fb3f1a66d5d295efe32d1b0f5c6c1a62d04ae495f654ddae1bbb1a7af3cfb` |
| Diagnostic-only expanded window display log | `f18b1cf954c57837877276ba97aba28df7d0ed611a725de12cc717d422386b21` |
| Diagnostic-only expanded window CPU samples | `08872c43b24aabfee0f43dc0bb2bdef735cd63134df9c11559cce4b8a100af8f` |
| Original guest reads in the extended common window | `7940f3722f68311299cd769ee04e7f0ca302ec92aba7453d082fc995e619e5db` |
| Final high-window change, dual-width focused tests and Core manifests | `8c5d18af28ca89f4cd1716e6d73713afee6ae74cbd60fbbf89b90a534b61b1c6` |
| Production-library Core quantum time after the high-window correction | `f810fd7d649ac39f5f588fe38403e832c34582feac75aaa51c05b6ba72215ad5` |
| CPU and clock-publication split | `f5e376eac1509c515960a20317cb9145fcd1a0a797c1cd99b52f2c4890586789` |
| Original guest RTC alarm registers and per-query timing | `5357391835b72a6f51653356bbfff60eaa477e49f7dc94f2e178eef0495a8ac6` |
| RTC deadline comparison against the prior implementation | `036ab7b12bb681c0ad7cb4ee64bac34e2b8535d131b121e209f35c96d84f3ecc` |
| Dual-width RTC, board and MyOB300 tests | `e818bd587d87e43e549a248b94acc2e2e77406decbb264dbaec59f96a4dc0804` |
| RTC-optimized original Windows CPU replay | `5e78c87fa03b5ca274d1b00082b51a383ad47d73081913c0be38506429ef949f` |
| RTC-optimized original Windows frame signatures and pause point | `806384334ad7fbe13d3c773760d1577c30003724d61564f1c5ca120eca2d4768` |
| Program Manager frame at 181.767 s | `c6d59b3d4f347bae325cb4e598ccb99f7eb961ac1ffd35f2ba6d5df1c450d31e` |
| Program Manager and hourglass at 201.400 s | `9d10e05ae96c05954b042203904fc0d08930fdac8fea621d863b3878320cac88` |
| x64 desktop input queue observations | `6f1c68f66becd462cd65c15cb7367644998899d59a14bb8683be97ba34805582` |
| x86 guest reads of Alt and F make and break | `a84b105913069d001d7eef25e9a9415bc3badc4008f5f86a4b370fd8001abfec` |
| x86 Program Manager after the second key request | `4fbceba09ffbc51d712bba31f3d8aa6724abf0175565138b631f1f593ad63da9` |
| Current PE inventory and native MyOB300 launches | `31b8b01bc7438db44929c38f531b52365803aff9ee2ec349db04f6eddbcbeceb` |
| x86 original Windows frame and execution checkpoints | `47c38479d7b662de242a223630e2b248d6c70085812b62d081ff6d6b7bea6e1b` |
| S3 latest 16-file build and complete registered dual-width qualification | `867780fc50589736182719ece4d0480b139c00d0aadcad804a32341aaac91f90` |
| Final 16-file publication | `57dba5f27ff2e0f47540ba3486b74db58136943a68a74aefdc758dfdf1361c5d` |
| Final PE inventory and exact MyOB300 native launch | `f2379eb3e01d8e827eb68dd9c2daeadd2e2e71773761ebeb8bb89ecd58882a42` |
