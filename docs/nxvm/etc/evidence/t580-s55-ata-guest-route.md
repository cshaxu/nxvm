# T580 S55 - Actual ATA bootstrap and selected guest route

Baseline: clean pushed `5e85b10de`, accepted S54. The frozen T580 ledger's
storage/guest member is still open. This S first observes the actual ATA
constructor, original BIOS disk/bootstrap and external selected guest path;
no production defect or repair is presumed from a prior description.

## Before-change design and input

The existing App plan already configures an ATA task file at 1F0--1F7,
alternate status/control 3F6 and IRQ14, through the sole Board/Machine path.
It has no inserted floppy drive and uses the normal optional fixed-disk
configuration. The existing external media manifest admits owner-provided
`media-nxvm/hdd_50m_win31.img`: 51,609,600 bytes, SHA-256
`61E5CDC0B76151CC65B73EB44094738B9DE86052B1B07F20FC03205984CD77E1`.
Both actual size and hash are rechecked. Its name does not prove a bootable
PC110 guest, CMOS setting, sector geometry or successful Windows execution.

The initial diagnostic extends only the existing external-BIOS harness:
`--ata-boot <image>`. It independently reads sector zero with a short-lived
readonly Lib Storage lease, closes it, then passes the real image through the
constructor with existing OVERLAY mode. The master is never opened writable.
The unchanged BIOS must execute its full cold/retained wake/RSM/POST/INT19
route, then actually execute at 7C00 with all 512 loaded bytes equal to the
independently read sector. No bytes are seeded into guest RAM by the host.
This first observation accepts only an actual bootstrap-sector boundary,
not a DOS/Windows or complete storage qualification. A failure selects the
complete BIOS/controller/enumeration/error class for further reconciliation,
not a patch tied to that PC.

Initial estimate: existing harness +60--120/-5--15 and eventual registration
+5--12/-0--4, no new executable, helper or production interface. The original
registered no-medium replay stays unchanged until the stronger selected-guest
predicate is established. One x64 diagnostic uses 420 s/100M containment,
with the existing sparse progress and no per-instruction observer. Further
execution is bounded to eight million instructions per new semantic phase.
The master hash is rechecked after the attempt. No external payload, raw
memory, third-party source or screenshot is committed.

## Static route reconciliation during the first observation

Open-Source-PC110 pin `02ccf2e8e19b298a2cf04c5d430278eb087cc368`,
`Discovery/Storage/readme.md`, SHA-256
`D526E3CADAA3F221C46168A8A86D5FA58F8DC3EEF6DDEE142946146B92F36604`,
records live/firmware evidence for CHS-only BIOS services and no IDENTIFY
command, with fixed/CMOS geometry rather than capacity discovery. This is a
reference research report, not an Intel timing specification or blanket
proof of this selected TFT firmware's runtime geometry.

Read-only original-firmware disassembly confirms INT19 F000:E6F2 delegates
to its F000:7DE4 bootstrap routine. It builds an ordered boot-device list
through a CMOS-dependent helper, tests candidate devices and attempts sector
loading; terminal error, retry and device-specific paths are distinct from
the real 7C00 handoff. The external PC110-QEMU pin
`bae390ebf21665ec97860d28e26ec46f1fe8f4e9` explicitly replaces that discovery
and INT13 path from the host. Those shortcuts are rejected, not copied.
Our existing plain port-8B latch, RTC and App ATA owners stay authoritative.

The selected image has a valid 55AA MBR signature and one active FAT16
partition: start LBA 63, length 100,737 sectors; other partition entries are
empty. Those structural facts do not alone establish that the BIOS selects
the drive or that its geometry matches the guest's filesystem. The real
constructor already loads the image through the existing fixed-disk media
owner, then resets the same machine; no second disk or CMOS route is added.
The partition BPB independently declares 512-byte sectors, 63 sectors/track,
16 heads, 63 hidden sectors and 100,737 volume sectors, matching the media
owner's current 16/63 translation but not proving BIOS-returned geometry.

## First actual observation and refined diagnostic

The first x64 observation terminates by its eight-million post-INT19
instruction containment, not wall timeout, at 64,813,775 instructions with
four delivered exceptions and no Core fault. It retains real wake/RSM and
enters INT19 F000:E6F2, but never executes 7C00. Later sparse points alternate
between the SMM entry and F000:E98A, whose original bytes branch into the
RTC IRQ handler. These samples alone cannot distinguish an interrupt storm,
an interrupted BIOS wait, or unfinished device discovery. No production fix
or failed-disk diagnosis is selected. Log SHA-256:
`817B3378416C3AC0818D644BFFFC047B8DCE016AD3E4763FF9AC1DFDDB2FEC73`.
The master retains its exact admitted hash afterward.

The next diagnostic adds a bounded copied retirement observer only after
the proven INT19 boundary: at most 128 INT15 call/ATA/PCIC/IPL records,
plus a final copied 16-word guest stack. It never changes guest data or
devices. That new phase is limited to 500,000 instructions, enough to obtain
actual BIOS service callers without repeating the previous eight-million
uninformative wait. The unchanged cold prefix remains observer-free.
The public observer is installed only at the existing paused watchpoint;
the original no-medium mode is unchanged. This is one test-only helper and
no production state, API, clock or execution path. Revised total harness
estimate +90--150/-5--15, before rebuilding this refined observer.

The refined x64 observation reaches INT19 at 56,813,775 instructions and
exhausts its following 500,000 instructions at 57,313,592. It observes
INT15 C501, the real IPL-latch read/write, then INT15 90FD at F000:C79A
from the floppy bootstrap/motor-wait path. No ATA or PCIC access appears
in that sampled interval. The copied stack independently contains the
return to C79C and the firmware's A9CB/A9CE wait-helper chain. Original
bytes show this helper repeatedly obtains status with AX=5380/BH=84
through the configured ASMI port and checks a guest-owned timeout byte.
Thus the remaining class is the whole BIOS wait/interrupt/time progression,
not yet an ATA transfer failure. Four delivered exceptions remain unchanged.
Log SHA-256:
`00DEB7E876D1AB3A484F55581FBC7F68C942E2852DAE267B40E3D0B529A90198`.

The observer initially reused the configuration-output count for its bounded
print count; that produced four meaningless zero records in the final old
configuration dump. This diagnostic-only bookkeeping is corrected to its
own count before qualification; none of those zero records is hardware
evidence. To distinguish an unfinished finite wait from a broken timer, the
same existing observer now counts actual BIOS IRQ0/IRQ8/SMM entries, records
Core timeline ticks, and copies the final guest BDA timeout/timer values.
The full eight-million post-INT19 containment is restored only with these
new discriminating facts, not as an unchanged retry. No production change
is selected from a half-million-instruction sample.

The full timer observation terminates at 64,813,775 instructions, no new
exception and no Core fault; SHA-256
`C17ACE792C217112199D9D6690E4265135A93B942977F168F8880069CD048805`.
It observes completion of the first INT15 90FD wait, a following AH=86
delay, the BIOS's actual disk-read entry C504, then another 90FD/AH=86
attempt. Guest ticks advance from 324,055,887 at INT19 to 377,308,603:
53,252,716 ticks, about 1.61 modeled seconds on the declared 33 MHz axis.
The final guest timeout byte is zero, not frozen at ten. These facts show
finite wait/retry progress, not an established controller or timer defect.
No ATA access has occurred in this short prefix.

The preliminary IRQ0 counter used an assumed ROM stub and is not accepted
as an IRQ absence result. Before the next run both IRQ handler addresses are
copied from the actual guest IVT while paused; the observer counts those
dynamic entries. The next run retains the original 100M total instruction
and 420-second wall limits instead of ending early at the newly understood
eight-million post-INT19 prefix. There is no clock change, skipped wait,
forced device selection or new production patch. This correction is recorded
before execution rather than silently relabelling the earlier failed attempts.

## Resume and stronger storage-source reconciliation

The owner paused and resumed the goal during the first 100M observation.
Its native session is now missing, no diagnostic process remains, and its
redirected log is empty. This interrupted attempt has no retained execution
result: it is neither passing nor evidence of a guest failure. A single
replacement observation is justified by that verified termination, not an
observation-tool timeout.

The same pinned research repository's more specific `Discovery/Live-Dump/README.md`
section 14a, SHA-256
`27B888AAEA6037DE1D9DB979ACF80A1E50EAC562D341B07CCC739EBBCEDF5C0B`,
corrects its older Storage summary. Independent direct ATA IDENTIFY records
put the soldered SunDisk SDP3B-4 at primary master (123 cylinders, two heads,
32 sectors/track) and the removable CF at primary slave. The observed PCIC
socket is a multimedia/SCSI card, not that CF storage route. The old summary's
PCMCIA inference must not select a new PCIC storage implementation.

Current App wiring supplies only the primary medium; Machine's copied input
already has two disk slots, but preparation rejects slot 1 and allocation,
registry binding, startup insertion and reset implement only slot 0. Board's
ATA controller already accepts two media identities. This is a real complete
two-device construction gap to design before any repair, separate from the
still-unclassified ordinary empty-floppy waits. No second media owner or ATA
path is required. The current 50 MB diagnostic medium is not represented as
the original soldered 4 MB device or a complete CF topology qualification.

Read-only local external candidate inventory also finds PC110-EMU's Disk1.img
(8,011,776 bytes; raw FAT volume, not an MBR disk) and PC110-QEMU's privately
constructed Personaware-disk.img (4,194,304 bytes; MBR partition start 32,
8,160 sectors; BPB two heads/32 sectors). Their sizes/layouts do not equal
the observed 123-cylinder soldered device. Neither is silently substituted,
modified or committed; source identity and intended physical slot must be
reconciled before guest qualification.

## Whole ordinary-boot result and Setup prerequisite

The retained replacement 100M run terminates normally with the diagnostic's
non-success status at 100,035,528 instructions; a final bounded quantum may
cross the containment ceiling. There is no wall timeout, new exception or
terminal Core fault. Log SHA-256:
`925B2BC68ED94115DC9FFA3BCEEFFDFED3C857A9B5BA7B9E4012523D09271D47`.
The actual IVT entries are IRQ0 F000:FEA5 and IRQ8 F000:E98A; their observed
counts are 129 and 2,565, with 35,894 SMM entries. Guest BDA time reaches
84h and the wait byte reaches zero. This rejects the preliminary timer-hang
hypothesis and the invalid assumed IRQ0 counter. No timer repair is warranted.

Actual service/port facts show the empty-floppy wait/retry path completes,
then the BIOS probes PCIC identification and calls CA00/CA01. It does not
issue an ATA task-file transfer in this interval. The last eight million
instructions execute the original lower-Flash LZW loader copied to 9000.
Its actual bytes match original ROM offset 011Ah, not a fabricated host
payload. Raw original bytes independently initialize its destination to
5000:0000 and patch the eventual far transfer. This proves the lower-bank
loader is a real consumer, not yet that decompression or Setup UI succeeds.

Before modifying storage or initialization, the next same-harness mode
`--ata-setup <image>` uses an ordinary F1 make/break through the existing
Machine driver immediately after genuine retained RSM. It watches actual
5000:0000 execution while the guest copies and decompresses its own Flash;
the host does not load or patch a Setup image, seed CMOS, force a PC or
perform guest decompression. The original 100M/420 s containment is retained.
This establishes the actual configuration/loader prerequisite before guessing
why the ordinary boot list selected no ATA drive. A Setup entry alone is not
UI, storage, DOS or T qualification. Revised diagnostic-only harness estimate
+140--220/-10--20; production and public interfaces remain unchanged.

Offline research may run the pinned reference repository's existing
`tools/unpack-easysetup.py` against the same immutable BIOS to obtain only
an expected output size/hash under S55 scratch. This is an independent
reference oracle, not copied implementation, a runtime dependency, guest
decompression by the host, or proof that the emulated machine loads Setup.
The generated protected output must never be supplied to the machine or
committed, and is hash-reviewed before scratch cleanup.

The F1 attempt also reaches 100,035,745 instructions with no new exception,
at the identical 9000:011A decompression state. It does not prove the F1
selection branch was consumed or Setup entry occurred. Log SHA-256:
`1EE3C05F18460FE1D32DE254264482BBDF2833661F6BA25CBABCF4B1518F5950`.
The offline pinned decoder produces 232,185 bytes, SHA-256
`7128F7C5D4907A8C7F1D7B8C0D64F8617CDA43464F36599D21ED7BBCCAB8FE8B`;
the tool's hash is
`FDD14C43E9DE222316DD61118491EC43C8480BFBCFF7F32FE9E56B32F45FA3BC`.

Before any production repair, a temporary direct-Core fixture isolates the
original lower-Flash loader using only public Core operations. It copies
the original compressed input to ordinary fixture RAM at 70000, initializes
the fixture CPU at that input's actual entry and watches 50000, then compares
its guest-produced output with the host reference buffer. The decoded buffer
is never written into guest RAM. This is an explicitly synthetic initial
CPU/test-memory arrangement, not the App's BIOS path, a runtime intercept or
boot qualification. A passing result selects the actual board mapping/copy
consumer for investigation; a failing result selects a finite CPU/decoder
class. It cannot advance a product gate. No new public entry is added.
Source/executable/log stay in S55 scratch and are removed after evidence;
use the existing compiled Core libraries, at most 20M instructions/120 s,
no new build tree or production/test-package source change for this fixture.

The direct Core reaches Setup after 9,821,892 instructions. Its first output
comparison differs at only one byte, offset 013Bh: the first actual Setup
instruction stores the incoming SP into CS:013A before the execution
watchpoint reports. A second diagnostic retains that exact one-byte finding;
it is not a decompression defect. The watch is therefore moved to the
loader's final far JMP at 9000:009E, whose retired target must be 5000:0000,
so all 232,185 output bytes can be compared before Setup legitimately
modifies itself. The decoded reference is still never supplied to the guest.

This also explains why the 100M full-machine containment cannot yet prove a
loader hang: the real loader starts near 92M, leaving about eight million
instructions, while the isolated original loader needs about 9.82M without
interrupt overhead. No CPU repair, byte normalization or weakened comparison
is justified. The next real-machine observation will retain the native time
limit but provide enough instruction containment for the complete workload,
only after this corrected full-byte oracle succeeds.

The corrected direct-Core fixture passes: 9,821,891 instructions, actual
retired far JMP at 9000:009E to 5000:0000 and zero differences across the
full 232,185 bytes. Log SHA-256:
`C85AA42DC35BADBAEFC4920C5966237D4B13BC6ADA96638C9D11D7E87C119414`.
This rejects a generic CPU LZW failure and selects a complete-workload
observation, not a speculative production patch.

The real-harness mode is now `--setup <reference-file>`, using the normal
no-medium App configuration rather than misrepresenting a 50 MB primary
disk as original hardware. The reference is opened read-only into host
comparison storage only. Original BIOS copying/decompression and the final
far transfer must produce the same complete bytes at 50000. The final far
JMP watch avoids comparing after Setup's legitimate self-modifying first
instruction; the retired target must still be 5000:0000. Ordinary driver F1
is attempted but no F1-selection claim is made without a separate consumer
fact. The observed automatic recovery route remains legitimate evidence.

Containment is revised explicitly from 100M to 110M total instructions:
the real cold/empty-drive route needs about 92M before a loader that itself
needs 9.82M without interrupt overhead. The native budget stays 420 seconds.
This is an evidence-based full-workload budget correction, not a guest-time
change, test weakening or claimed completed Setup UI. Original registered
no-medium boot/FDC tests and all production code remain unchanged.

The real BIOS comparison succeeds at 101,917,229 retired instructions. The
original final far JMP reaches 5000:0000 and all 232,185 bytes match before
Setup's first self-modifying instruction. Four resets/four exceptions remain
the known cold prefix, with no Core fault. Log SHA-256:
`ECAC3291E1C243A54C638BD03FF97BA4CFD2DA05A488E88021023D2CFEB40689`.
This establishes real loader completion, not Setup UI or guest boot.

The same diagnostic may now continue for two million instructions after that
verified handoff and capture through the existing Machine display publisher
and driver frame-copy path. An optional fourth argument owns a scratch-only
PPM output. Its screenshot and CPU/frame facts select the next real Setup
consumer; execution count alone is not a UI assertion. Estimated additional
test-only delta +45--75/-3--8. Native containment stays 420 seconds; no
production/API, firmware, reference injection or new execution path is added.

That observation completes normally at 103,947,666 instructions with a valid
640x480 graphics frame, but every pixel is black. It is not UI success. Log
SHA-256 `C23F26C7C3DF5653E38C2EACCDBA8254BA07E3A4CA6AB4785B43DED1B673DE24`.
The actual final 5000:C0A5 instruction compares the BDA timer at 0040:006C
with its previously sampled value; original Setup C098--C0AC implements one
tick wait, called four times by C08B--C097. Two million instructions cover
only about 0.15 guest seconds here, shorter than four ordinary BIOS ticks.
This is another insufficient observation, not proof of a timer or VGA hang.
The same diagnostic's post-entry containment becomes 20M instructions for
Setup initialization and normal delays, within the unchanged 420 s x64
native ceiling; no timing model or guest memory is changed.

Independent source audit identifies missing VGA read-latch and write-mode
semantics in F65535's direct plane-copy memory access. The local original
65535 Register Reference revision 2.0, printed pages 65--71 (GR00--GR08),
SHA-256 `ACB95E9BD56786227B6B9201B3BFF317ADBD224D1A587FBF8DB515FB5A648A91`,
defines read latch loading, read-map/compare, rotate/logical operations,
set/reset and bit masking for four write modes. PDF page 72/printed 68 was
rendered and visually checked. This remains a concrete source gap, not the
proven cause of a frame captured while the guest is still delaying. No
production change is selected until the actual consumer is reconciled.

The same display-class audit also finds that both F65535 scanout branches
use a fixed RGBI palette even though the component already owns Attribute,
DAC and PEL-mask registers. A later admitted repair must reconcile the whole
CPU planar access/scanout colour class, including side-effect-free debug
inspection and unchanged text addressing, rather than patching only one
observed pixel. This does not establish that either gap caused the first
all-black delay sample. The selected external 50 MB medium's hash is
unchanged after these observations; only readonly/overlay access was used.

The 20M continuation completes at 121,928,529 instructions without new
exceptions/faults and delivers a 640x480 frame with six distinct RGB colours.
Visual inspection shows the real Setup ERROR dialog, codes 08081, 00173,
00301 and 00163, with OK/Cancel controls. This proves real execution and
rendered error UI, not normal configuration, input consumption or guest boot.
The terminal CPU is in the BIOS C503 service, not the earlier tick wait.
Log SHA-256 `1A7B0F1044B996FEDCA9CC1F7E44877CEB6B9060A06E4A4C5D956BB8B64B0711`.
The original reference screenshot is only a visual comparison and uses
separate BIOS-completer shortcuts; none are imported or used here.

The initial setup diagnostic attempted F1 during resumed POST without proving
its actual consumer. Remove that unnecessary injected event from this
automatic error-recovery route before classifying 00301 as a production
keyboard fault. A subsequent no-F1 replay must distinguish a diagnostic
interaction from an existing controller/self-test defect; no keyboard
production change is justified merely by this error number. x86's complete
replay uses the same 20M post-entry observation, with 540 native seconds
based on S54's measured 302.96/224.16 host-width ratio and the new x64
approximately 392-second whole route. No guest clock or predicate changes.

For the subsequent keyboard classification, the existing copied port trace
can select only data reads at 60h and command/data writes at 60h/64h, without
retaining the repeated status polls. It reuses the existing 128-entry array,
reports the retained count and makes no claim of a complete transcript if
that capacity is exhausted. Estimate +8--16/-2--5 test lines, no new array,
observer API, host input source or guest state. The unmodified no-F1 x86
observation already running is allowed to terminate before another replay.

## Error-UI receiver classification

Ricoh source: local original application manual EA-028-9804, SHA-256
`7EB037B495D7ED3F4334A1CDC6C6582C8FA9CEADF0FD680EBADEC2575B60C7EC`.
It is a research input only; no manual or third-party source is imported.

| Observed item | Proven fact and next owner |
| --- | --- |
| Recovery entry | Original BIOS executes the complete LZW loader and transfers to its own Setup. No decoded reference or success state enters guest RAM. |
| Black early frame | Two-million sample ends in the original BDA tick delay. The later ordinary continuation draws an error dialog; the early sample is not a proven hang. |
| 08081 / PCIC candidate | Earlier real BIOS probes indexes 00/40/80/C0 through 3E0/3E1 and reads zero. Current App has no PCIC route. Ricoh EA-028-9804, printed page 27 and register summary page 44, specifies readonly ID/revision 83h for the actual two-slot RB5C396. This establishes a missing controller receiver independently of assigning the error number; do not fake only one successful ID read at a BIOS PC. |
| 00173 / 00163 | Current construction intentionally has no successful CMOS seed. Existing RTC reset preserves calendar/NVRAM. Generic IBM error references associate these codes with configuration/date, but no PC110-specific error-code table has been established; preserve this distinction and exercise real Setup before selecting an RTC repair. |
| 00301 | Keyboard/controller is present through the normal 8042-compatible path. Early diagnostic F1 is not evidence of consumed F1 selection. No-F1 replay and actual data/command transcript select whether this is diagnostic-induced or a real protocol gap. No guessed ACK is admitted. |
| Colour correctness | The dialog is visibly rendered, but fixed RGBI scanout ignores retained Attribute/DAC/PEL state. Existing UI output is not yet qualified against actual guest-programmed colours. |

None of these facts is DOS, normal Setup, complete PCMCIA/card support,
live input, panel identity, whole-T acceptance or PC110 artifact publication.

The no-F1 x86 run reaches the same successful final far transfer and full-byte
comparison. Its later UI continuation reaches at least 115,835,343 instructions
without a new exception, then the 540-second native job deadline terminates it.
No final frame or keyboard-error disposition is available. Preserve that failed
UI observation, log SHA-256
`D680FAC837F1D62B8A8CD6AB0F0CA9BFF58DFDF3CECA992CF5864C04D443BABD`.
It is not a passing whole replay and does not justify a guest timing repair.

The next coherent diagnostic combines the actual error UI with one ordinary
Enter make/break after its measured 20M initialization phase, then 10M further
instructions to observe configuration entry or the complete input failure.
Capture the before/after frames and print/reset the same bounded copied keyboard
transcript at phase boundaries; do not introduce another input path or array.
Only the real driver delivers the key, with no forced guest branch or state.
Extract the existing frame-file writer into one test-local helper because the
same capture is now needed twice. Revised additional test estimate +45--85/-5--15,
including moved code rather than two renderers. The x64 native ceiling is 480 s
(measured approximately 392 s plus the additional execution/trace); x86 is 660 s
(measured 540 s through 115.8M plus the remaining approximately 16M). These are
host containment estimates, not changed guest clocks or success predicates.

The first input-follow-through attempt is interrupted with native supervisor
exit 1073807364. Its log is empty and neither child nor supervisor remains;
no guest result is claimed. A single replacement runs the same frozen
diagnostic and 480-second bound through the existing supervisor in a hidden,
detached PowerShell process, so tool/session interruption does not itself
destroy the owned job. Its task script and terminal result stay only in S55
scratch; this is not a new permanent runner or watchdog.

The detached x64 observation completes at 131,959,677 instructions. Both
before/after frames are valid 640x480 output: the initial ERROR dialog still
contains 00301 without early F1; ordinary Enter moves to the genuine Date/Time
panel showing 1994.01.01/00:00:00. This directly proves that key's consumption,
not all input or a completed configuration save. Four known exceptions/resets
remain; no Core fault appears. Log SHA-256:
`2B7C092BEA9B247BB8DBCF7147581C10D0B0E9E02149753C4A4F4E904FFB0F22`.

The zero copied keyboard records are not absence-of-I/O evidence. Core's
transaction trace is intentionally compiled out in Release even when a
provider is installed. Correct only this diagnostic to select the already
available copied retirement I/O observation, using the same bounded array
and phase boundaries. Do not change production trace policy, Core API,
build configuration or guest behavior. Estimate +12--24/-5--10 test lines.
Runtime overhead must be measured; this is not permission to call an
uninstrumented run a complete keyboard transcript.

The runtime observer completes with the same 131,959,677 instructions,
known four exceptions/resets and Date/Time follow-through. Phase counts
45/33/24/6 are below the 128-entry capacity. Final entries show guest reads
1Ch/9Ch for the delivered Enter make/break. Log SHA-256:
`C74648E4FDA56B109D9E247344E8612453230E3CC6BCB79EEDA69D2AA861A400`.
Frozen harness SHA-256:
`4552ABE48C01BB7520EE2E73330F4E25A56233F57E2E0792CCBD08280E3761F4`;
x64 diagnostic EXE SHA-256:
`FED52A973207C99CABA02141FF385CE6CED85E2E8878DDC575D97500CD7CC9B6`.

## Selected keyboard ordering repair

The actual cold transcript reads FA at BIOS F000:659B, then AA at 65C0 and
takes the 6615 failure cleanup, instead of completing reset/identity checks.
The original 657F--65C5 helper drains early OBF while it tests IBF, then waits
for the real command response. Current synchronous input completion clears
IBF and exposes ACK before that first status observation. This is the exact
ordering class already handled by shared KBC's existing
command_response_status_polls; its source comment names the same 5170
command pattern. PC110 leaves the option zero in both fixed constructions.

Select one status poll through that existing contract. The first status
sample sees the completed write without early ACK; the shared owner then
releases ACK for the subsequent response wait. Do not add MCU/BIOS-PC
special cases, sleeps, host time, success data or another response queue.
This is an L2 ordering model, not a measured MCU service duration.

Before implementation: pc110_plan.c and pc110_machine.c expected +6--10/-0
source lines; existing base-composition/machine-construction tests +20--45/
-0--8. Test the drain-before-wait sequence through actual Core ports,
existing immediate input and AUX behavior, then replay the original BIOS
and inspect its error list. Four unpublished PC110 previews receive the
App change; no Shared code/API or other App executable input changes.

Direct x64 regression passes after draining the keyboard's distinct initial
BAT before issuing reset. The same test fails with only the App's old zero
poll value restored (status 15h exposes OBF too early), then both affected
App tests pass after restoring one poll. Negative-control log SHA-256
`13E6977FC70E2B8D273D8C58044E6F5BB8CB93DBD985F8683D6BC5794CA28CF5`;
restored two-test log
`5363926C7883B5BBAD66B9A7F5430E4341D4DC2B5E6BD96753EBD06EF0D1DCBE`.
The temporary zero configuration is removed before any artifact build.

The repaired real-BIOS replay also sends a second ordinary Enter after the
already observed Date/Time follow-through and captures each pre-input frame.
It observes whether the guest commits configuration or requests another step;
no successful save is presumed. This adds only a second stage to the same
bounded test input, not an input framework. Native containment becomes 540 s
x64/780 s x86 for the 20M initialization plus two 10M follow-throughs: the
instrumented 131M x64 run measured about 475 s, versus 425 s without the
observer, and x86's failed 540 s run reached 115.8M. A correctly shortened
keyboard path may finish much sooner; budgets never stand in for behavior.

The one-poll real replay fixes the observed early-reset read sequence: FA is
now read at 65C0, BAT AA at 663C and ID AB/41 at 6601/660C. It nevertheless
ends at the 110M instruction containment inside the original LZW loader,
without a new exception/fault; the changed POST path begins that loader later.
This is a failed Setup gate, not proof of a new CPU fault. Log SHA-256
`DCE735DF07DA4E9D57D6E2108E8C5115FB855B78596758FDBBA1B224F583B045`.

Broader happy-path review rejects shipping the poll-dependent candidate:
the existing visibility gate intentionally cannot release a reply or advertise
a deadline until the guest polls. An IRQ-only command consumer need not do
so. Prefer the other existing controller capability: a positive response
duration and zero visibility polls, advanced solely by Core time. No shared
implementation changes are needed. Retire the one-poll App choice, not the
shared capability used by other declared models.

Use a nominal 100 microsecond L2 response interval: 86Box source pin
`4fef696a4eead1d55a28d6ac0e5bd2864e5454da`, unchanged
`src/device/kbc_at.c`, polls both controller and attached device at that
cadence. This is an emulator-derived service quantum, not an asserted PC110
physical response time or imported implementation. Convert it to 3,300 ticks
on the declared 33 MHz KBC/Core axis in App clock facts. Extend the existing
port regression to prove publication via the actual Core deadline without
another status read, and preserve AUX/native input assertions. Source estimate
remains two App initializers plus the existing clock header, +8--12/-2--4;
tests add at most 15 lines beyond the admitted port regression.

The unchanged reference file SHA-256 is
`4753D4E0759B7BFEB84218A593736C319FCBBCBB0A62E2BB59CB53CAFAEE6DC7`.
The resulting source change is six added lines in the two App constructors
and clock header. The Core/KBC scheduler, controller and all Shared files
remain unchanged. Both direct regressions pass on x64 and x86. Restoring
only the base constructor's original zero delay makes the reset-order
regression fail with status 15h; restoring the timed configuration returns
both tests to passing. Negative-control log SHA-256:
`C1A355EC82C72592B197C146054BF2826A2777451DCB3DC0CBFDC5D5F18831B8`.

All eleven PC110 units pass on each width with the final timed configuration.
The four unpublished TFT/DSTN previews are rebuilt and their PE widths/hashes
are in [the S55 inventory](t580-s55-previews.json). No production inputs of
the other Apps change. Documentation governance and whitespace checks pass.
The full repository-unit regression is separately running, with Types layout
alone and the remaining 711 cases at j8; these pending results are not passes.

The timed-response original-BIOS replay uses 120M instructions to reach the
unchanged loader handoff, then the same 20M initialization and two 10M input
stages. The previous 110M run ended inside a progressing original loader;
its independently measured 9.82M decompression work must fit after the later
POST path. Native containment is 660 s x64/900 s x86, scaled from the
instrumented 475 s/132M observation, not a guest-timing adjustment or success
condition. An attempted relink of the running integration executable is
rejected by Windows; the original loaded replay is retained, and independent
unit/preview targets are used until that native process exits. No process,
OS setting or source behavior is changed to bypass that file ownership.

## Final timed-response observations and bounded disposition

Both x64 and x86 execute the same unchanged BIOS to all 232,185 independently
compared Setup bytes, render the same error list without 00301, consume two
ordinary Enter make/break pairs, and stop at the same real firmware request
for software reset. Both end at 144,097,172 instructions, four processor
resets, the same four delivered exceptions, and no Core fault. FA is now
read at F000:65C0, AA at 663C, and AB/41 at 6601/660C. This qualifies the
reset/identity command ordering and the observed keyboard input, not all
PC110 input or product interaction.

| Evidence | SHA-256 |
| --- | --- |
| Timed x64 real-BIOS log | `25F890A392B0CADEEBEE744281E70DD4B09D6F3ABA257245FC5242B9F74573D5` |
| Timed x86 real-BIOS log | `BD8F361670E634396295F77655E2764DF6BE7F84228A2A6AEE6F8915E3141CDF` |
| Error frame, identical on both widths | `996EB2DA9495551F2808C41C6AF2C07FBD9DCF6F2D7587A9C9DDE37DC8D9998E` |
| Date/Time frame and retained final frame, identical on both widths | `37770F9C230B2B372426A8DDB392418D02E196AC236E51C4F0D92E832C95C321` |
| x64 isolated Types result | `35505F1C39954C52123D12F8BEB7C5F13F5EEBA29269047F68F70B8ECF9D0947` |
| x86 isolated Types result | `422E905A4AB7A21075161FDC93ACA150C15BBA02335F175AF110F41A87CC4A61` |
| x64 remaining 711 units | `BDF69D19AC0D066F29A1D796E29D474272A377507ACDAD140452C7FB132F3130` |
| x86 remaining 711 units | `19EC3A87BA2D1A8E6649387DDD7E93136DA47896AD7759E73EBF5A0F4FE0E9E2` |

All 712 distinct repository units pass per width: Types 28.44/27.73 s,
the remaining 711 at j8 in 596.89/102.85 s (x64/x86). No failed unit case is
discarded or replaced by an App-only pass. The focused negative control and
eleven App-unit results remain separate evidence. The registered firmware
integration regression is reported separately after its two-width completion;
the manual Setup diagnostic's exit status does not establish a normal Setup,
saved configuration, DOS, desktop or live product qualification.

Readonly disassembly of the original F bank establishes the final path:
F000:ABAD disables interrupts; ABB3 reads 92h; ABB9 writes bit 1 set and
bit 0 clear; ABBF writes bit 0 set; ABC1 executes HLT if no reset occurs.
The captured CPU is at ABC2 with IF clear. The App declares no 92h route;
its existing base test explicitly asserts UNSUPPORTED there. This is a real
missing system-control receiver, not an unexplained CPU hang or a reason to
force the instruction pointer. Board-Base's existing 92h implementation also
only writes the final A20 mask and does not implement reset or distinct
keyboard/fast-gate inputs. The earliest owner of that reusable mechanism is
Board-Base; the PC110 App owns selecting its physical presence.

The whole remaining class stays inside T580 with explicit receivers:

| Member | S55 disposition and retained receiver |
| --- | --- |
| Original loader and reset/identity ACK/BAT/ID ordering | Accepted with complete byte comparison, original-firmware consumption, negative control and direct deadline/AUX regressions. |
| Ordinary Setup interaction/save/restart | Error confirmation and Date/Time input observed; restart cannot complete because 92h is absent. Next T580 system-control receiver must supply the physical route and one Board-owned A20/reset mechanism, then re-run the original save/restart path. No CMOS-success seed or PC interception. |
| Two-device ATA construction and selected guest | Open at X86-NXVM Machine and App composition. Existing slot-zero-only allocation/bind/insert/reset and global geometry must become one declared per-slot path before claiming the real SunDisk/CF topology. The selected 50 MB image is an input, not proven PC110 geometry or boot. |
| Empty PCIC controller and inserted-card capability | Open at the PC110 controller/composition receiver, with Ricoh source and actual missing 3E0/3E1 route identified. A documented empty controller is distinct from CF/PC-Card mapping and must not fabricate card presence. |
| VGA aperture/colour and panel identity | Open at the existing F65535 owner and App observation adapter. Standard planar latches/read/write modes, Attribute/DAC/PEL scanout and side-effect-free inspection must be reconciled together; no fixed host palette or second rendering path. Panel-specific qualification stays independent. |
| Other original T580 lifecycle, DSTN identity, guest and four-artifact gates | Unchanged and unaccepted at their existing ledger receivers. Four rebuilt previews are not published products. |

This is the complete observed failure-class disposition permitted by S55's
packet, not a claim that ATA bootstrap, guest boot or T580 is complete. No
remaining member is removed from the original universe. The next admission
consumes a named remaining receiver instead of inventing success for a later
BIOS PC.

Actual tracked C/H delta across six paths is +310/-23, net +287: production
is +6/-0 in three App configuration files; tests are +304/-23 in the existing
harness and two existing units. The increase retains the real sector/loader
oracle, bounded retirement transcript, original driver input and copied frame
evidence in one harness; there is no new test executable/framework. Three
test-local helpers own transcript formatting, frame export and ATA retirement
observation. The test uses the harness's existing internal Machine seam plus
its existing publisher and public driver-copy result; this is not claimed as
a private-free external integration. No production helper, mutable state,
public interface, Shared source/test file or manifest changes. Final actual-
change review and owned-scratch cleanup are still required before delivery.

The registered regressions pass without changing their assertions or budgets:
FDC reset 52.29 s x64 / 54.92 s x86 (180 s budget); real boot handoff
236.77 s x64 / 352.18 s x86 (360 s budget). Result-log SHA-256 values are
`987ACF8607D839E34309647E8A59B4B7B869E3BB07925F1DA5324B067C2909C5`
and `9792F4176B5EAEF29915E6D12FC7E57BD9E7F5E62BC2075ACDD1D9E98870A3E3`.
These are separate from the longer manual Setup path and do not prove a
selected external guest. They each run once per width.

Reproduction uses the existing two stable GCC release graphs and j8 builds:
`cmake --build <tree> --target build-unit-tests -j8`, isolated
`ctest --test-dir <tree> --output-on-failure -j1 -R '^lib.types-layout-selftest$'`,
then `ctest --test-dir <tree> --output-on-failure -j8 -L '^unit$' -E '^lib.types-layout-selftest$'`.
The registered pair uses `-j1 -R '^mypc110.integration/real-firmware-(fdc-reset|boot-handoff)$'`.
The existing firmware executable's `--setup <comparison-only-file> <frame.ppm>`
mode executes the real loader and ordinary input scenario under the existing
hidden bounded native runner. Raw protected references, screenshots and logs
are scratch-only; the source pins, inputs, result predicates and SHA-256 facts
above are retained evidence.

After every owned task exits, the TFT master still hashes to
`232101C88466F311BCC32FBC215A4D7569F695CE19F9C07CA67CE2AEE5232312`
and the selected external disk to
`61E5CDC0B76151CC65B73EB44094738B9DE86052B1B07F20FC03205984CD77E1`.
No owner INI, snapshot, external repository or media is modified.

Executor actual-change review confirms six counted C/H paths, no Shared or
public-contract change, and the retained one Core clock/KBC scheduler. The
ten published identities from S54 remain exact; the four preview identities
match the new inventory. After OS process verification finds no owned task,
the official cleaner first previews and then removes only S55's 128 files,
14,332,369 bytes. Its sole protected comparison file is explicitly hash-
reviewed. Both stable Ninja graphs and ccache remain untouched. No raw input,
frame or third-party implementation enters the commit. The registered and
manual qualifications above do not replace unexecuted desktop/RDP, DSTN,
panel, selected guest, configuration-save or final product gates. Executor
delivery is ready for coordinator review; T580 stays open.
