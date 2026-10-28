# T580 S78 — PC110 controller restoration investigation

## Confirmed baseline

S77's genuine x64 firmware/guest trace shows INT 15h C201h fails because the
original SMM code tests its own byte at 3000:0004h, finds bit 0 set and
returns carry before its EC mailbox transaction. This byte is already 01h
at the original BIOS INT 19h handoff, so DOS is not its initial owner.

At suspend, the implemented EC 10h completion AAh is available. The BIOS
restoration consumes it, sends 0Ah with data 9Bh and then 11h, and preserves
the previous SMM lock. Both registered restoration callbacks actually run;
the second reaches its later 89h/00h operation without executing the known
lock-clear instruction. See the S77 evidence for trace hashes and scope.

## Controller and reference distinction

The selected keyboard-controller image's reconstructed command table maps
11h to a return instruction; it does not by itself establish a reply. Its
0Ah handler reads one parameter, stores it in its control byte, tests bit 6
and writes either 0Fh or 00h to controller address 12h. The latter address
is not yet identified for this specific controller; treating that write as
a proven timer or assigning an exact delay would be unsupported. The
instruction interpretation is checked against the published M740 opcode
description, not imported as implementation: [MAME M740 opcode map](https://github.com/mamedev/mame/blob/master/src/devices/cpu/m6502/dm740.lst).

PC110-EMU's indexed-port placeholder writes the last command into its status
byte and later clears its busy bit. For 11h this leaves the availability bit
set; its data port returns its existing latch. That is a possible L2
behavioral reference, not proof that the selected controller's 11h handler
emits a byte. Copying its status echo would also change the mailbox's existing
single-OBF ownership for unrelated commands. No such fallback is adopted.

The next step is to identify the actual controller-side completion/event and
its ownership. A candidate must preserve the existing one-reply contract,
not turn 11h into a fabricated command reply, patch the SMM byte, or add a
second guest-input path. Qualification requires the unmodified guest to
clear its lock and initialize and use its pointing device; a boot-handoff or
mailbox unit alone does not establish that outcome.

## Bounded hypothesis experiment

To test the event hypothesis rather than assert it, a temporary App mailbox
model scheduled a single control-data notification after a nominal 200,000
guest ticks when 0Ah selected bit 6 clear. Its value followed PC110-EMU's
data-latch behavior. The interval and controller address 12h's purpose are
not established hardware facts; this is an L1 experiment, not an approved
product model. One genuine x64 `--resume-boot` run is executing in the
background and reports the original firmware lock after handoff. The
temporary model and observation print were removed from the working source
immediately after launch. A cleared lock would be causal evidence only;
controller semantics, behavior across reset, other commands and real guest
mouse input would still need design and verification before any implementation
could be accepted.

The experiment completed the original x64 firmware handoff with status OK,
no Core fault and its original exception baseline. Its observation reported
SMM lock `00h`, versus `01h` on the unchanged mailbox. The unmodified guest
therefore can clear its own lock when a later mailbox event becomes available;
this is a causal result, not proof of the nominal delay or of mouse input.
It also shortened the bounded run from 64,230,771 to 47,345,511 instructions.
The experimental log SHA-256 is
`2393838E01542F10D4C5BA08A8F793484F0C4599AF3806FFF349A3C7621E3607`.
One separate bounded x64 real-guest run using the same temporary model is
now observing the DOS driver's INT 15h C2h request and return. It is stopped
after that observed request, and does not by itself prove pointer movement or
desktop behavior. As before, the temporary model and observer are absent
from the working source while the prebuilt probe runs.

## Implementation design to review against the guest result

If the guest result supports the hypothesis, the narrow candidate is one
App-owned deferred control notification behind the mailbox's existing reply
latch. A 0Ah control transaction with the observed bit-6 condition arms it;
the first subsequent mailbox read supplies Core's existing guest tick and
starts a nominal deadline. When a later read reaches the deadline and the
reply latch is free, the notification becomes available once. A subsequent
control transaction replaces or cancels the pending notification. CPU-only
reset must retain controller-owned state; no guest memory, separate worker,
polling loop or public interface is needed.

The nominal delay and returned data are L1 compatibility choices informed by
the real firmware sequence and PC110-EMU's existing latch behavior, not a
physical timing or MCU-protocol claim. Starting the deadline on the first
post-write read avoids assuming a stale pre-write time: the existing port
write callback does not provide a timestamp. This model would need explicit
tests for empty and occupied reply latch, one-shot delivery, cancellation,
CPU reset and unrelated commands, followed by both-width real firmware and
actual driver/input verification. No production model is selected yet.

The candidate is now present in the working tree for verification. Its
one-shot control notification is deferred behind the existing reply latch;
control replacement and cancellation do not create a second queue. The
count restarts if the Core tick value moves backwards across CPU reset.
The existing mailbox smoke test exercises occupied-latch ordering,
single delivery, replacement, cancellation and reset using the real Core
port routes and guest execution. Both x86 and x64 targeted builds and that
test passed. All 13 PC110 unit tests also passed on each architecture after
rebuilding their targets. The genuine x86 firmware resume test passed with
47,345,511 executed instructions, four baseline exceptions and no fault.
This is not yet a mouse or release qualification. The separate x64
DOS-driver probe continues with the previous experimental
binary; the two candidate timing implementations are not identical.

The x64 experimental DOS-driver trace reached C205h and C201h with carry
clear (`flags=00027256`), unlike the prior C201h carry-set result. Later
repeated C206h returns had carry set, and the diagnostic recorded an internal
Core fault after 251,313,914 instructions (`detail=80000000`,
`code=eb1f6a7c`); its exit is not a passing end-to-end result. The watchdog
reported `STOPPED_AFTER_PROBE`, so that run was intentionally bounded and
cannot establish whether the guest would otherwise recover. Its log SHA-256
is `1B4BEE5A6F14CC8C87773EF4F4199C29B20FAAA062B0917B1B24DF1210962846`.
The reason for the C206h failures and the Core fault still needs investigation.
The reported `80000000h` is the CPU's internal case-error exception, and
one path assigns its code from a failed physical memory access. The value
`eb1f6a7ch` may therefore be an address, but the current log does not prove
which error path fired or why the guest reached it; it is not evidence for
changing the shared memory-map behavior.
The external PC110-EMU reference notes that this ROM waits for an auxiliary
IRQ before consuming the reset response, but its placeholder writes a BIOS
data-area flag directly. That shortcut is not a model for this project and
does not establish whether the IRQ, controller command byte, device response
or guest state is responsible for the C206h failure. The new real-guest
probe records the BIOS request registers and controller I/O instead.

The current-candidate x64 probe reproduced a successful C201h return and
identified the C206h failure: BIOS writes 8042 command D4h followed by
mouse command E6h, and the auxiliary device replies FEh instead of FAh.
The existing PS/2 mouse model already reports unscaled movement but did not
accept the standard 1:1 scaling command. The candidate adds the E6h ACK
without claiming support for the separate E7h 2:1 scaling mode. Both-width
mouse command tests and the existing 8042-to-auxiliary IRQ/port test pass,
including the actual D4h/E6h route. Both affected Shared manifests were
updated. A further bounded real-guest probe is running against this candidate;
its outcome, including the later Core fault, is not yet known.
After the E6h change, all 13 PC110 unit tests and all 219 registered
X86-NXVM tests passed separately on x86 and x64. These are component
results, not a full-repository or real-guest qualification.

The subsequent x64 real-guest diagnostic with the current candidate observed
both C201h and C206h return with carry clear. In C206h, the unchanged BIOS
wrote D4h to 64h and E6h to 60h, then read FAh from 60h. The probe was
intentionally stopped after the observed return; it cannot establish full DOS
startup or absence of the later Core fault. Its log SHA-256 is
`449DD6838088BA65BE4A60E87F4EE1FF45BF65F1B55A1D2BC68A72112BB0E3F4`.
Both-width real guest and inking flows subsequently delivered all eight
corner press/release events, reached matching 640x480 Personaware launcher
frames and stopped at their prescribed instruction budgets with no recorded
Core fault. Both diagnostics nevertheless returned 1: their inherited
power-check predicate requires no new delivered exceptions, whereas these
real DOS runs delivered 164,539 VM86 exceptions against a baseline of eight.
The image SHA-256 is
`85A35680232CA0AD623DB7DD18D23A79396931ACB133791581D40C37048AEF41`
on both architectures. This is real guest and frame evidence, not a passing
automated integration test or proof of interactive mouse movement.

The existing diagnostic's power-only no-new-exception assertion was scoped
to the power-only cases. The OS path now instead requires the successful
power/boot chain, all four input press/release pairs, a generated final frame,
and no Core fault; delivered VM86 exceptions are not themselves classified
as an emulator failure. The two observed examples included VM86 software
interrupt dispatch (`INT 1Ch` and `INT 16h`), not a recorded Core fault.
The same diagnostic now captures frames before and after a relative host
mouse event. The two-architecture run is in progress; the two image captures
must be inspected for actual guest response rather than assuming that a
successful driver callback proves it.

The x64 run completed with the new input checks (`POWER:result=1`, no Core
fault). Its immediate pre/post input frames were captured while the launcher
was still drawing, so their differences are not valid pointer evidence. At
the same final instruction budget, the earlier run without the added mouse
event had the pointer near the image centre; this run has it over the right
menu. The final-frame SHA-256 values are respectively
`85A35680232CA0AD623DB7DD18D23A79396931ACB133791581D40C37048AEF41`
and `6149FFD194A8B24F4C772C719AB3793E40FE6E9AC2829BA2A55C32E859004A62`.
The display proves guest-visible host-input response in this reproducible
comparison; it does not distinguish PS/2 movement from the parallel inking
path. The x86 run also completed with the same successful result and exact
final-frame SHA-256 as x64, with no Core fault. Both runs used the TFT guest;
they do not prove the DSTN variant or desktop/RDP presentation.

To distinguish the shared PS/2 path from parallel inking, the same guest
diagnostic now has an explicit PS/2-only input checkpoint that calls the
existing board receiver with the already mapped relative movement and checks
its returned status. It introduces no production interface. The x64 run is
completed successfully and its final frame matches the moved-pointer frame
from the host-event run, rather than the no-motion baseline; no Core fault
occurred. This proves guest-visible response to the board's PS/2 path on x64.
The x86 PS/2-only run also passed without a Core fault and produced the same
`6149FFD194A8B24F4C772C719AB3793E40FE6E9AC2829BA2A55C32E859004A62`
final-frame hash as x64. This diagnostic uses the existing Machine-private test dependency,
whose cleanup remains a separate T580 exit item.

The existing `machine_construction_smoke.c` already exercises the actual
PC110 plan through its Emulator driver and debug request contract. It offers
a concrete migration destination for port and memory coverage currently held
by the test-only duplicate base composition; this has only been inspected,
not yet migrated or qualified.
The duplicate tests also cover deadline-sensitive 8042 ordering and F65535
read-latch behavior. A later cleanup must preserve those assertions at the
actual driver boundary or in the owning chip tests; simply deleting the
duplicate construction would lose evidence.

After both runs, the final diagnostic source and all four PC110 preview
targets were rebuilt on their respective x86/x64 cached trees. The three
focused mailbox, mouse-chip and board-route tests passed again on each
architecture. The four existing PC profiles were rebuilt for both widths
from the shared cached trees, and all eight product/INI identity tests passed.
PE architecture and deployed file hashes are recorded in
[the artifact record](t580-s78-pc-artifacts.json). The PC110 builds remain
previews, not final released PC110 products. Shared source/test manifests,
the X86-NXVM dependency gate, NXVM documentation governance and
`git diff --check` passed.

The changed production code comprises 17 additions and one deletion in the
PC110 mailbox plus three retained App-local fields, and three added lines in
the existing PS/2 mouse chip. Tests extend the existing mailbox, chip, board
route and real-firmware diagnostic; no new public Shared API, thread or
second input pipeline was introduced. The E7h 2:1 scaling command remains
unsupported; acknowledging it without implementing its behavior would be
incorrect. Incomplete PC110 release, separate DSTN qualification, real
desktop/RDP presentation remain open.
The real-firmware FDC/reset integration and firmware resume test both passed
on x86 and x64 with the current working-tree candidate. These checks do not
exercise the DOS driver's later C206h failures or Core fault.
The separately embedded derived DSTN firmware's power/resume and boot
handoff also passed on both host widths with four baseline exceptions and no
Core fault. This does not qualify DSTN DOS, input or visible presentation.
