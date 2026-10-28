# T580 S77: PC110 inking boundary

The TFT real-guest checkpoint in the S76 evidence reached the handwriting
driver's first calibration prompt. That checkpoint does not establish that a
host input was consumed, or that the separate PS/2 mouse was detected.

The local Open-Source-PC110 `Discovery/Inking/readme.md` describes a separate
resistive digitizer. Its selected I/O window is configured through CMOS and
the guest discovers both its address and IRQ through its own firmware. The
document describes a three-byte sample delivered one byte per interrupt,
with a control-register acknowledgement between bytes. Its own bit-number
prose and decoding formula differ for the high coordinate bits; the packet
formula must be checked against the actual driver before claiming exactness.
Treat this reference-derived model as L2, not a complete hardware timing
specification.

The existing Machine profile binding accepts a physical-key callback; mouse
events instead follow the common PS/2 mouse path. Window currently emits
relative mouse events. Consequently, an inking implementation needs both a
board IRQ connection and a defined way to deliver pen input; merely decoding
the ports does not make calibration work. The ordinary PS/2 pointing device
and the signature pad must retain separate guest-visible semantics.

The board profile construction services now expose the existing borrowed PIC
endpoints. Sources remain owned by the board and use its existing lifetime
and interrupt handling. The Machine profile input hook now receives mouse as
well as key events; a declined event continues to its original device path.
The existing Machine construction test exercises the hook with a mouse and
passes on x86 and x64. PC110 construction also passes on both widths. These
checks do not exercise an inking device or its actual IRQ route.

An App-local L2 inking register and three-byte handshake model now has a
targeted test on both host widths. The test covers disabled/enabled status,
packet bytes, three successive interrupts through the actual PIC with EOI
for both IRQ5 and IRQ10 at each of the three documented port bases,
individual acknowledgements, queued pen transitions, coalesced movement and
explicit bounded-queue overflow. It does not establish physical
timing, input delivery, guest-driver compatibility or the configured IRQ.

The selected TFT firmware's query code was inspected locally: the CH=96h
selector is read via ports 74h/76h from SCAMP index 16h, rather than from the
ordinary CMOS array. The L2 cold SCAMP image has A1h at that index, encoding
15E0h and IRQ10. The device now reads the existing VL82C420 owner, accepts
only its selected port window, and binds its IRQ from that same value.
Existing tests cover the initial A1h selection delivering IRQ10 and changing
the selection to E2h to observe 25E0h/IRQ5, as well as initial device reads
via the assembled PC110 Machine;
both host widths pass. The same test checks that disabling the selected pad
returns open-bus reads and rejects new samples. On a guest write that changes
SCAMP 16h, the App
quiesces the preliminary device and deasserts its current IRQ; an existing
configuration-route test exercises a pending sample and real PIC source on
both widths, including preserving a pending sample when the same value is
written again. This is an L2 reconfiguration policy, not a measured physical
timing guarantee. The firmware's live execution still requires validation.

After rebuilding the PC110 unit targets and both variant preview dependencies
in the existing cached Ninja trees, the complete PC110 unit set passed:
13/13 on x64 and 13/13 on x86, including the later inactive-port
failure-handling correction. This does not cover real firmware, desktop or the
rest of the repository. The x86 inking replay predates that correction; the
new x64 inking replay includes it. Both predate a subsequent profile-factory
failure-path ordering cleanup, which has a targeted dual-width construction
test but still needs end-to-end qualification with the final binary.
After the lazy IRQ-binding cleanup, all 13 PC110 unit targets were relinked
against the current App library and the complete unit group passed again:
13/13 on x64 and 13/13 on x86.
The affected Shared X86-NXVM unit group also passed 233/233 on each of x86
and x64 after its cached rebuild. The later App-only inactive-port correction
does not change that Shared group.

The source now acquires the board-owned IRQ source when the first sample is
accepted, using the current SCAMP selection; construction no longer binds an
unused hard-coded IRQ. Configuration, construction and inking tests pass on
both widths after this cleanup. The active real-firmware replays predate it.
Both TFT and DSTN preview binaries were subsequently relinked on x86 and
x64, and their four product-composition tests passed. These build-tree
previews are not deployed or qualified PC110 releases.

The four existing PC products were rebuilt once per width using the same two
cached Ninja trees; each product/INI identity check passed. All eight output
PE machine types and SHA-256 hashes were independently checked and recorded in
`t580-s77-pc-artifacts.json`. This is not PC110 guest or desktop qualification.

The Machine profile now maps existing host pointer events to a bounded virtual
pad position and its left button to pen contact while the inking device is
enabled; the event also continues to the separate PS/2 pointing device. The
existing Machine construction test exercises host input through both press and
release into the guest-visible inking port on both host widths. The pending
sample queue and relative-pointer mapping are L2 interaction models, not claims
about the physical pad. Their practical calibration experience is unverified.
The existing Window release path emits a mouse-button release when capture or
focus is lost, so that event follows the same adapter. This is a static code
observation; a desktop focus-loss test has not been run for the new device.

Real guest calibration, both panel variants, the mouse driver,
desktop display and product publication remain to be validated under T580.

The existing real-firmware diagnostic now has an optional inking run: after
its original key checkpoint and only when the guest has enabled the digitizer,
it submits four corner press/release pairs through the Machine input driver,
with bounded guest execution between events. The x86 replay is running in the
background against the reviewed read-only media and overlay; no calibration
result is claimed before the guest display and execution are inspected. The
older x64 run continues independently with its pre-inking binary and is used
only to observe the original firmware path. After that run finished, a new
x64 inking replay was started against the corrected receiver, in parallel with
the existing x86 replay.

For future inking replays, the diagnostic now disables the per-retirement
observer after verifying the actual loader and restores it before sending the
driver input. Core's existing fault diagnostic provider continues recording
delivered exceptions in the interim. This is intended to reduce the long
boot's observation overhead; the currently running processes predate it, and
neither its performance nor its end-to-end equivalence has yet been measured.
The existing Core executor test, which exercises delivered exception
diagnostics without a retirement observer, passes on both widths. It checks
exception validity and kind, but does not independently establish that the
specific VM86 keyboard exception and its instruction bytes remain available;
that still requires the optimized real-guest replay.

The separate PS/2 path is present in PC110's board configuration and the
existing board test exercises auxiliary reset, identification, IRQ and packet
delivery. Those unit facts do not explain the guest's "mouse not found"
message. The future optimized replay additionally records a bounded set of
guest INT 15h/C2h and KBC 60h/64h operations after the original keyboard
checkpoint; this instrumentation compiles on both widths but has not yet run.

To avoid leaving the original panel variant unexamined, the existing firmware
diagnostic source also builds against the separately embedded DSTN BIOS. An
x64 `--fdc-reset` observation completed with exit 0 after 8,221,407 guest
instructions: the existing probe observed reset assertion and release, IRQ
return and all four SENSE results. The log SHA-256 is
`BFFB2FCD2A423CB3CF36CD94FAD7371484F161CE7890A3507A05C14646B99BE3`.
The x86 observation is running. This is a diagnostic, not an established DSTN
boot or product qualification test.

The older x64 diagnostic reached its actual late keyboard wait at 438,482,679
retired instructions, delivered its existing Enter input, and subsequently
observed the guest's `5380h/8300h` calls with `CH=96h` and writes `04h` and
`38h` to port 15E2h. This independently confirms the chosen register window
and enable operation for that run. Because this process was built before the
inking receiver, it does not establish IRQ delivery or calibration success.
The x64 run reached its 50-million-instruction post-input bound with status 0
and no Core fault; its final PPM SHA-256 is
`3A14F0E064CE15FF87A5DF205C46BA7938D024F1D71663C94FB47AE45B3A5DF4`,
identical to the prior late-key capture containing the first calibration
prompt. Its process exited 1 because the older diagnostic's unchanged-
exception-count assertion does not hold on this VM86 path; it is not a
passing integration test or evidence of a new Core fault.

The same local repository contains an unresolved reference inconsistency:
`Discovery/Inking/readme.md` attributes the inking query to APM `BX=8300`,
while `Discovery/PS2/DISASM.md` identifies `8300` as display and `8800` as
inking. The existing real-firmware diagnostic now records a bounded set of
relevant APM calls and inking port cycles. A background TFT replay has been
started to observe the guest before choosing the runtime selection mapping;
no conclusion about that mapping is yet claimed.

The reference's `Software/PS2GUI/PS2GUI.ASM` supplies an additional detail
omitted in the prose: its direct digitizer query uses `AX=5380`, `BX=8300`,
and `CH=96`. It derives the base from `CL & 3` and the IRQ from `CL & 70h`.
This supports a subselector-based interpretation of the apparent conflict,
but does not establish how CMOS bytes map to that result. Its ISR reads one
byte, pulses control bit zero, and then sends the PIC EOI, consistent with
the preliminary model's acknowledgement order.

The supplied Personaware disk's actual `INKDRV.COM` was independently read
through its FAT12 directory and cluster chain for a bounded disassembly. It
confirms that query uses `CH=96`, requires CL bit 7, and looks up the low two
bits in the four-entry 05E0h/15E0h/25E0h/35E0h table and bits 4–6 in the IRQ
table. The inspected temporary extraction (SHA-256
`772BB767F89475988289392A1798631AA5DF92144B30C7C802594989A8EF4374`)
was deleted; no guest binary is committed. This confirms the caller contract,
not the mapping between the firmware's live CMOS and its return value.

The initial TFT CMOS seed has byte 68h equal to 00h, which is not the register
read by the inspected TFT firmware query. Setup can modify configuration at
runtime; the running firmware trace is needed to validate the live path.

The separate reported Setup colour discrepancy was also checked statically:
F65535 forms palette colours as `0x00RRGGBB`, Machine resolves indices into
those values, and the Window renderer copies them into a 32-bit BI_RGB DIB.
This path contains no explicit red/blue exchange. S76's captured guest frame
is not a measurement of the actual Window or RDP presentation, so the user's
reported blue appearance is still unverified and is not closed by this check.
The local PC110-QEMU reference documents a blue initial Easy Setup theme in
its separate direct-entry path, which skips ROM pre-entry initialization and
patches a theme table in the loaded guest image. PC110-EMU likewise has a
direct-entry palette override. Neither workaround proves the cause in our
normal firmware path, and neither is applied here. The guest-selected DAC
values and the actual Window presentation still need to be compared at the
reported screen.
For the current x64 TFT real-firmware replay, the captured ERROR and
date/time screens are the two pre-input 640x480 RGB frames; they contain
`BE9E96` as their dominant non-black colour:
113,923 and 177,692 pixels respectively. These are Machine-frame PPM
captures, not screenshots of the Window or its RDP presentation; they do not
exclude a host display problem or prove every Setup page uses that colour.

The original DSTN firmware also completed the existing FDC reset diagnostic
on both host architectures: each executed 8,221,407 guest instructions and
observed reset assertion, release, the returning IRQ, and completed sense
commands. Both logs have SHA-256
`BFFB2FCD2A423CB3CF36CD94FAD7371484F161CE7890A3507A05C14646B99BE3`.
This is a controller checkpoint, not an original-panel DOS boot or visual
qualification.

An additional original DSTN x64 firmware replay completed its power-button
resume path and reached the BIOS INT 19h handoff after 64,286,985 guest
instructions (`PC110:POWER:result=1`, four delivered exceptions matching its
baseline, no fault). The log SHA-256 is
`A831A8137CA470FE38C905C8C0A5747A0D72BB488E88FD1DE7F240C6C1E7745A`.
The x86 replay independently reached the same handoff after the same number
of guest instructions with the same exception baseline and no fault; its log
SHA-256 is `10159899F56C9B73FE143488B085FF7339A45EB4F78A3553D2B5DF0269DE56CF`.
This checkpoint does not establish that DOS booted.

The TFT x86 real-driver replay enabled the selected 15E0h device, then the
guest INKDRV read injected packet bytes at 15E0h and acknowledged them with
39h/38h writes at 15E2h. Eight press/release events for four corners were
delivered at one-million-instruction intervals. The final 640x475 frame
still asks the user to touch the upper-right corner: only delivery and guest
register consumption are demonstrated, not completed calibration. The run
ended at its instruction limit with status OK and no Core fault, but its
legacy power checkpoint returned failure because delivered exceptions no
longer equalled the earlier baseline. The large post-input exception count
also requires interpretation; it must not be assumed to be a CPU fault or
ignored as proof of calibration. Event timing and phase transitions require
further investigation.
The TFT x64 replay independently reached the same final state: its final
frame is byte-identical to the x86 frame (SHA-256
`2A40C72702D8E640AFC1985971F0F3DD086E70C5225DC396452CA900187D891D`)
and its corresponding legacy power checkpoint also failed. A subsequent
diagnostic records the actual frame before each press and spaces samples more
widely, rather than inferring successful stage transitions from delivery.
The second x64 run captured the guest prompting for upper-left, upper-right,
and lower-left in sequence, but it remained at lower-left after the third
sample. A bounded read-only disassembly of the supplied INKDRV.COM (the
previously recorded hash) shows that calibration compares the returned
coordinates against 80h to require the expected quadrant. The probe's lower Y
value of 125 was below that boundary, so the guest correctly rejected it.
The probe now uses Y=250 for the lower corners. This corrects test input and
is not evidence for changing the device model. The temporary extraction was
removed.

The corrected x64 replay captured the guest's prompts for upper-left,
upper-right, lower-left and lower-right before the corresponding samples.
After all eight press/release events the final 640x480 Machine frame shows
the actual Personaware launcher, including its DOS and application entries;
its SHA-256 is
`9F5A17B4583F30B07A889D75CC25881F0329BD6A04FFBFCC6B48ACA2243296B7`.
The test delivered inputs from guest instruction 448,482,835 through
482,606,010 and stopped at 538,482,886 with status OK and no Core fault.
Its legacy power-checkpoint predicate nevertheless reported `result=0`
and the process exited 1, since it requires the delivered-exception count
to remain at the earlier baseline even during the subsequent VM86 guest.
This is real x64 guest progression evidence, not a passing diagnostic or
a desktop or mouse-driver qualification. The x86 replay independently
captured the same four pre-press frames and reached a byte-identical final
Personaware frame at the same 538,482,886 guest-instruction checkpoint.
It likewise recorded status OK, no Core fault, and exit 1 from the same
legacy exception-count predicate. Neither replay is counted as a passing
registered integration test.

After the current diagnostic source was rebuilt, the registered TFT
`mypc110.integration/real-firmware-fdc-reset` test passed on x86 and x64:
1/1 in 53.96 s and 1/1 in 37.30 s respectively. These are targeted
controller checks; they do not turn the optional inking replays' exit 1
into a passing result or qualify the full repository.
The registered TFT `mypc110.integration/real-firmware-boot-handoff` also
passed on x86 in 318.37 s and x64 in 222.22 s.
The NXVM documentation-governance target and `git diff --check` passed;
both Shared X86-NXVM source and test manifests verified.

The unrelated PC DOS mouse-driver message was checked against the current
assembly: the PC110 board selects the 8042 topology, leaves `kbc_aux_absent`
false, creates the auxiliary device and binds its IRQ12 source. This is only
static connection evidence; it does not establish the BIOS or driver reset
handshake. The prior late trace was saturated by ordinary 64h status polls and
cannot identify why detection failed. Inspection of the actual Release build
also showed that its general bus trace is compiled out, so a proposed earlier
bus-trace addition was removed rather than treating it as working evidence.
The existing retirement observer now filters ordinary status polls and keeps
auxiliary-ready status, 60h operations and 64h writes starting at loader
handoff for OS diagnostics that retain the retirement observer. The inking
mode deliberately disables that observer until Enter to reduce diagnostic
overhead, and the active phase-3 executables predate the filter. A separate
`--setup-pair-os-late-key` run retains observation during driver startup; it
is still needed to identify the actual mouse-detection attempt.
The subsequent x64 probe has now reached that attempt: it observed guest
`INT 15h` C205h followed by C201h, and repeated 8042 commands ADh/A7h/20h/60h
with command-byte reads 75h and writes 65h. It has not yet recorded an
auxiliary D4h command in that bounded trace. The unchanged BIOS's nearby
helper can retry when a keyboard output byte is pending; this is a possible
explanation, not an established cause, because that trace filtered ordinary
status reads. The diagnostic source now records output-full status reads
after the first C2h call as well. Its current running executable predates that
last refinement; no behavior change is justified from the incomplete trace.
The next run observed the driver-side returns: C205h resumed with carry clear,
whereas C201h resumed with carry set. The guest's subsequent detection failure
is therefore reproduced. The trace did not yet locate which firmware branch
set carry. The next diagnostic observes the bounded real-ROM controller-reset
branches and changed output-full status; these are test observations only.
That branch probe reached the real controller helper three times during
C205h and once during C201h. At `F000:B871` the firmware takes the successful
branch with carry clear; at `F000:B879` carry is still clear; on return to
`F000:B621` carry is set. The intervening firmware calls `F000:DECA`, which
issues an `AX=5380h`, `BX=8A02h` service through the existing SMI route.
The trace therefore narrows the next investigation to that service and its
return, but does not yet distinguish a legitimate firmware-reported failure
from an SMI/board/CPU implementation defect. The bounded probe intentionally
stopped after the observed guest call; its log SHA-256 is
`3446C9FD25FFEF6CEF8D713EEE0FF66DFED44D126474DA4F5264E46D278AD321`.

PC110-EMU's `Sources/PC110Firmware/BIOS_FIRMWARE.md` models an INT 15h
AX=5380h request as a no-event timeout with carry set. This is a reference
model, not evidence that the private SMI call observed here has identical
semantics or that a mouse must be absent. A short synthetic register-entry
probe reached the SMI handler but was interrupted by a debug stop before the
helper returned; it does not resolve the real guest failure. Its temporary
code was removed. The unchanged-firmware guest replay remains the authority
for the actual call and return.

The subsequent bounded real-guest replay captured the SMI return: with
AX=5380h and BX=8A02h at the request, execution entered the SMI handler at
38000h, then resumed at F000:DD11 with AX=A580h and carry set. The C201h
caller ultimately received carry set. This establishes where the result
changes, not whether the firmware result or underlying emulation is wrong.
The probe was stopped after that observation; its log SHA-256 is
`8F670C500926D6D62C388A73DE822B46C0031580DCA18A4AA21B2BF785DEDCD0`.

A synthetic post-initialization execution of the same helper reached the
vendor handler at 3B28Fh and showed it writing FFh to the EC command/status
port 15ECh after checking bit 1. In that isolated state the handler returned
without carry; the actual DOS path returned carry. An experimental FFh
mailbox response was withdrawn: the observed isolated branch waits for input
buffer availability, not for the proposed response, so that response has no
established causal basis. The next bounded real-guest trace records the EC
ports and SMI instructions during C201h to identify the difference.
The original TFT firmware's handler first tests bit 0 of its own SMM data
byte at DS:0004h; when set, it returns carry before touching the mailbox.
The same firmware has power-transition routines that set and clear that bit.
This identifies an additional guest-state predicate, not yet proof that the
actual C201h failure took that branch or that the bit was incorrect.
The subsequent unmodified-mailbox real guest trace did take that branch:
at 3E5D1h the test leaves ZF clear, and the JNE at 3E5D7h jumps directly to
the error return at 3E5EBh. The BIOS never attempts the 15ECh write in that
call. This verifies the observed cause of its failure, but not whether the
new 10h completion will clear the lock on the actual wake path.
The bounded baseline log SHA-256 is
`EE3CC2C41CCE213FB07114D64C5E27BA730F995964DB896AC85CAA0CEEC7D385`.
The previously reconciled keyboard-MCU command dispatcher maps command
11h to handler D897h, whose selected-image byte is RTS. Thus the existing
model's lack of a direct 11h reply is consistent with that selected image;
the firmware wake loop's expected input cannot be justified as a fabricated
reply to that command. This does not rule out a separately generated event.

Further inspection of the same selected controller image resolves a separate
bounded omission. Its command-group table selects DA4Ch for command 10h;
the handler's branches converge at DAA6h on `LDX #AAh` followed by a call
to the same DBB1-output routine used by the known 05h reply. The App mailbox
previously ignored 10h. The candidate receiver now publishes AAh through
its existing reply latch, graded L2 because the functional response is
reconstructed from the supplied controller image and its other physical
effects and timing are not modeled. The x86 mailbox unit passes. The actual
post-resume guest and mouse result still require verification; no inference
from the unit test alone is a successful mouse qualification.
The existing mailbox unit also checks that a subsequent 11h no-op and a
CPU reset do not discard the pending 10h completion; polling does not consume
it and the data read does. This is the modeled retention contract, not a
claim that all power-management side effects are implemented.
With the candidate receiver, the registered x86 real-firmware FDC/reset
integration also passes in 59.77 seconds; this verifies that earlier boot
checkpoint, not the later mouse path.
The corresponding x64 FDC/reset test passed in 38.74 seconds.
The separately embedded DSTN firmware diagnostic was also rebuilt with the
candidate and completed `--fdc-reset` on both x86 and x64, each observing
reset assertion, release, IRQ return and completed SENSE transactions.
The x64 DSTN `--resume-boot` diagnostic also completed successfully: its
unchanged guest reached the INT 19h handoff after 64,286,985 instructions,
with no Core fault and the expected restored exception baseline. Its log
SHA-256 is `F1F06A1E2BDD7694C8C387EF1C6C41CF485B7FA4FC3172A2695177B21A936E3F`.
This is a boot-handoff check, not DSTN DOS or desktop qualification.
The x86 DSTN `--resume-boot` diagnostic independently reached the same
handoff after 64,286,985 instructions and passed its existing result and fault
checks; log SHA-256 is
`C53F048B3661745E4F2D36237AF7A136BA67778056EDC0FFB81C3C703BD38B9B`.
The registered real-firmware boot-handoff integration also passed with the
candidate in 350.72 seconds on x86 and 251.94 seconds on x64. The mailbox
unit passed on x86 and x64.
A separate x64 real-guest mouse trace with the candidate reached INT 15h
C205h/C201h, but C201h still returned carry set. Both visits to 3E5D7h had
ZF clear, proving the guest's EC lock remained set. The candidate therefore
does not fix the mouse failure. The probe was stopped after the observed
return; its log SHA-256 is
`884625436269941E6814C6C97D49CE4939554DA1456050A95FC32559ECD17CE4`.
The 10h reply remains a separately sourced bounded controller behavior, not
evidence of successful wake or mouse initialization.
To locate when the lock is retained, a separate bounded `--resume-boot`
diagnostic reads the App-owned SMRAM backing byte after the genuine BIOS boot
handoff. This read is observation only; it does not patch guest RAM, modify
firmware or change the release path. Its short-lived print was removed from
the source after starting the diagnostic.
The x64 handoff completed successfully at INT 19h and reported the SMM byte
as `01h`; thus the lock is already set before DOS starts. This narrows the
remaining investigation to firmware power restoration, not the DOS driver.
The log SHA-256 is
`CB991D768681E91241E4B6EF0A28D0C0494049B0CAFDF1F819245891C51C9D46`.
A further bounded handoff probe now observes that backing byte and the
mailbox's non-consuming status at suspend and the existing firmware wake
watchpoints. It does not synthesize an input or change firmware state; the
temporary observation code was removed after starting the probe.
With the 10h response enabled, the x64 probe observed the firmware lock as
01h and mailbox status as 01h both at suspend and at 39336h wake entry.
At 3DBC5h, immediately before RSM, the mailbox status was 00h but the lock
remained 01h; it was still 01h at the later INT 19h handoff. Thus the
response was consumed during restoration without the lock being cleared.
The bounded log SHA-256 is
`5C03D041F2BC4474C00B7520B2EC107766DB05F01643E1A606ABA56B96D4A1C3`.
The consumer trace then completed the same genuine x64 boot handoff. During
restoration, firmware instruction 3E5DDh read AAh from 15E8h while its lock
was temporarily zero, issued command 0Ah, read the stale data latch again,
and issued 11h. No execution of the observed lock-clear points 39121h or
3C691h occurred during this bounded restoration. The firmware subsequently
restored the previous lock value. The trace does not establish which external
event should invoke a lock-clear handler; it rules out loss of the 10h reply
as the complete explanation. Its log SHA-256 is
`3B6F1BA9766E4D282A3C947771176FCEB8966FE34EDDA931EBEDA4B57C3EC8FB`.
The subsequent resume-only trace shows both firmware callback entries execute.
The first sends 0Ah/9Bh and 11h while temporarily clearing the lock, then
restores the old lock. The second is entered with AH=01h, reaches its later
89h/00h transaction, but never reaches the 3C691h clear. Thus it is incorrect
to characterize the problem as a missing callback invocation. The observed
sequence is consistent with its mailbox-ready wait expiring, but does not yet
establish the controller event or response value required to satisfy it.
Log SHA-256:
`0F95271C291C31566E339F97C5A3BB0FA1FD6D1C08CD212616DE7FD570B6A153`.
Both DSTN and TFT preview executables were relinked in each stable x86/x64
build tree with the candidate mailbox. `objdump -f` identifies the two x86
previews as i386 and the two x64 previews as i386:x86-64. These are build-tree
previews only, not deployed or guest-qualified PC110 release artifacts.
All 13 PC110 unit executables were relinked against the current board on each
host architecture and then passed (13/13 x86, 13/13 x64).

## Implementation review and remaining work

Actual source/test counts use `git diff --numstat` for tracked C, headers and
CMake files plus line counts of the three new files; manifests, documentation
and binaries are excluded. Shared X86-NXVM source is +14/-7 and tests +14/-4;
PC110 App source is +260/-6 and tests +314/-6. The net increase introduces
the previously absent physical inking receiver, its bounded packet queue and
the corresponding direct/real-guest coverage. The EC 10h completion reuses
the existing single reply latch; no additional production event queue, worker,
firmware interception or guest-memory patch is introduced. Lib and Emulator
are unchanged. Shared source and test manifests and NXVM documentation gate
pass; all eight affected PC artifact hashes match the deployment record.

The actual inking driver reached the Personaware launcher on both widths, as
described above. The optional long-running observation still exits 1 because
its inherited exception-count predicate rejects expected VM86 deliveries;
it is not represented as a passing integration test. Mouse C201h, both
desktop presentation and the wider T580 PC110 release qualification remain
open. Mouse restoration belongs in the next bounded S; the duplicate
test-only construction path and private diagnostic dependency remain T580
cleanup requirements, not waived or counted as complete here.

An independent quality review also found an existing second construction
path in `src/app-mypc110/pc110_machine.c`: its base-composition entry is
used only by `base_composition_smoke.c` and `display_route_smoke.c`, while
the actual Product and firmware diagnostic use `pc110_plan.c`. Its copied
board configuration can drift, and those two tests do not by themselves
verify the real Product path. This is not changed during the current inking
receiver validation; it remains a T580 cleanup item to migrate meaningful
assertions to the real construction and remove the test-only production path.
The real-firmware diagnostic also includes `x86-nxvm/machine/machine_private.h`
and accesses its Core directly. That pre-existing dependency crosses the opaque
Machine boundary; it is diagnostic evidence rather than a clean independent
App integration test. It must be reassigned to the component that owns that
private state or rewritten around existing public observation contracts before
T580's architecture/test-quality closure. Do not add a raw-Core getter merely
to retain the current harness.
