# MyPC110 Product And Guest Qualification

## Admission And Dependencies

Unnumbered M5 candidate in the [NXVM queue](../states/QUEUE.md), created by the
owner-requested PC110 task split. Allocate a numeric T only at admission.
Depends on: Real-firmware boot task and all required feature-ledger dispositions.
Use the [PC110 programme contract](m5-pc110-evidence-and-implementation.md)
and its convergence ledger; the task packet freezes the exact batch and source
baseline before implementation. Research references are not implementation imports.

## Objective And Scope

Consume [S2 qualification rows QUAL-1--3](../etc/evidence/t558-s2-receiver-checklists.md#board-and-qualification-receivers)
and all preceding feature proofs. Product integration is the existing
Core/Product and Emulator route, not a parallel PC110 execution framework.
Qualify original DSTN and upgraded TFT independently in both host widths.
Publish exactly `mypc110_x86.exe`, `mypc110_x64.exe`,
`mypc110_tft_x86.exe` and `mypc110_tft_x64.exe` under `assets/mypc110`.
The owner-selected filenames omit version text; verify internal version,
PE width and compiled firmware/panel identity for each artifact. Both models
use the adjacent `mypc110.ini` runtime options, not runtime model/BIOS loading.

Complete lifecycle/debug/config/Console-Window integration and a frozen guest corpus: DOS shell/storage/input, Easy-Setup, Personaware and selected Windows checkpoints only where lawful media and explicit admission establish them. Audit all feature, source, quality and release ledgers.

Expected owners: PC110 App/build/tests/assets/docs; actual Core regression owners when defects are demonstrated.

## Non-goals And Stop Conditions

No moving guest-compatibility goal, broad all-software claim, cosmetic success marker or benchmark threshold substituting for behavior. Optional guest expansion is a later candidate.
Stop and report source/model conflicts, missing lawful assets needed by this
batch, or a required Shared-six modification; do not invent an undocumented
fallback. Continue independent in-scope work where possible.

## Suggested S Boundaries

Lifecycle/failure and UX qualification; finite guest suite; complete regression/release/quality audit.

These are planning boundaries, not allocated S identifiers. Split an oversized
batch before admission; never use letter-suffixed S numbers or an endless
first-failing-BIOS-instruction repair sequence.

## S28 Closure — Post-Delay Execution Boundary

S28 records the real TFT BIOS retirement sequence immediately after the
established `F000:EA84` delay return, on both host widths.  It proves that the
apparent stall is a finite BIOS port-61 delay and that the subsequent executed
routes are already owned.  The source record identifies a later `8Bh` boot/IPL
latch candidate but does not define its reset state, so no receiver was added.
The temporary probe was removed.  Full evidence is in
[the S28 record](../etc/evidence/t580-s28-post-delay-boundary.md).

## S29 — DMA-page / Boot/IPL receiver

The PC110's `80h`--`8Fh` range is not admitted as the normal PC/AT DMA-page
alias: the reviewed PC110 sources identify `8Bh` as a passive Boot/IPL latch,
while two independent reference implementations model the entire range as a
zero-reset plain read/write scratch file.  The selected TFT BIOS contains its
own `out 8Bh,0` followed by `INT 19h` at `F000:52BA`; that is a semantic
observation target, not a host shortcut.

The allowed implementation is one Board configuration fact selecting the
plain-scratch personality and one Board-owned 16-byte store. Existing boards
retain the standard DMA page mapping. This does not create a PC110 App-owned
overlay, a second route, a guessed nonzero reset image, or any synthetic boot
service. The real-firmware diagnostic may stop only on the observed write.

### S29 Closure

The S29 Board-Base correction selects a PC110-only, zero-reset plain byte
file for `80h`--`8Fh`, while retaining normal DMA page registers and both
8237 control ranges for ordinary PC-family configurations. Both host widths
pass the 253 affected X86-NXVM/MyPC110 tests. The selected firmware's later
`out 8Bh,0; int 19h` is statically established but not yet dynamically
observed, so this remains a bounded receiver correction rather than POST,
boot, or artifact qualification. The source grades and exact limits are in
[S29 evidence](../etc/evidence/t580-s29-dma-page-boot-ipl-latch.md).

## S30 — EC/ED configuration gate

The existing MyPC110 EC/ED receiver now follows the source-recorded access
gate: `FBh` opens, `F9h` closes and reset starts closed.  Its one bit belongs
to the existing firmware binding because that binding already owns the selected
EC/ED register, data effects and C-segment shadow behavior.  A closed window
reads `FFh` and ignores writes, preserving normal hardware I/O without turning
an unavailable config window into a Core fault.

Both host widths pass the 253-test affected X86-NXVM/MyPC110 corpus.  The
existing real-firmware trace is extended only to record gate ports if reached;
it does not yet reach them.  No Shared/Core contract, ROM byte, new timer,
host service, product artifact or POST/guest claim is introduced.  Full
provenance and limits are in [S30 evidence](../etc/evidence/t580-s30-eced-config-gate.md).

## S31 — post-C000 port-61 caller boundary

S31 consumes the next real-firmware checkpoint after S28/S30. The established
diagnostic proves one `F000:EA80h` port-61 service return but intentionally
stops at its local `ret`. Static `EA38h` code is not assumed to be the active
caller. S31 may extend that same diagnostic to record the actual next
instruction after the real service return. The instruction ceiling remains
containment only. No production receiver, timing shortcut, ROM mutation,
artifact or guest claim is admitted; a no-caller or already-owned-route result
closes the S with evidence rather than inventing a device model.

### S31 Closure

The actual caller is `F000:EA5Bh` (`dec bl; jnz EA55h`), confirmed on both
host widths. It is the existing BIOS outer delay loop and does not select a new
device receiver. The stronger caller test replaces the former local-return
test; detailed output and limits are in [S31 evidence](../etc/evidence/t580-s31-post-c000-port61-caller.md).

## S32 — outer-delay counter fact

S32 strengthens the one admitted caller diagnostic rather than speculating
about the static outer-loop count. It records the low `BL` value from the
retirement snapshot at the first established `F000:EA5Bh` caller on both host
widths. That fact determines whether completing the loop is a finite,
semantic next checkpoint; it is not itself a completion, receiver, POST or
guest-boot claim. No production code, timing model, ROM/media input or public
contract is in scope.

### S32 Closure

Both host widths report `BL=6Fh` at the established `EA5Bh` caller, after the
routine's `dec bl`; the preceding value is `70h`. S32's hour-scale wall-time
extrapolation was later corrected: it included pre-loop startup time rather
than the measured outer-loop duration. S33 replaces the caller checkpoint with
one outer-return checkpoint. See [the S32 record](../etc/evidence/t580-s32-outer-delay-counter.md)
and [the S33 correction](../etc/evidence/t580-s33-outer-delay-return.md).

## S33 — outer-delay completion correction

S32 established the counter value but incorrectly extrapolated full-loop wall
time from pre-loop firmware startup. S33 replaces the one caller test with one
real outer-return checkpoint. It must observe EA38 entry, all `EA55h` loop
tails, `EA7Fh` return and the actual returned-to caller on both widths. The
instruction limit is containment only. No timing shortcut or production device
model is admitted.

## S34 — complete delay-routine boundary (closed: no retained receiver)

S33 proves the `EA38h` subroutine returns to `F000:53DDh`; static firmware
shows that this point remains inside the enclosing `F000:53D0h` delay routine,
which returns at `F000:5405h`. S34 temporarily extended the one diagnostic to
record the routine's `DX`, `ret`, and returned-to caller. At the prior 13M
containment it established `DX=0102h` and no routine return; the later 20M
experiment did not yield a retained semantic result after its terminal session
lost the completed child output. The temporary mode was removed rather than
treated as evidence. S33 remains the one admitted dynamic delay checkpoint;
there is no source-backed receiver or product change. See
[S34 evidence](../etc/evidence/t580-s34-complete-delay-no-result.md).

## S35 — post-delay receiver inventory

S35 consumes a finite inventory rather than following more instructions: the
real 8042, admitted EC windows, firmware mappings, storage and LPT routes are
reconciled against the selected TFT BIOS and PC110 source records. Open-Source-
PC110 can establish wiring and protocol facts. PC110-EMU and PC110-QEMU are
reference evidence only: their BIOS interceptions and synthetic boot paths are
not implementation sources. A route without a complete source-backed contract
is transferred as blocked rather than filled with a guessed reply.

### S35 Closure

The inventory confirms that every currently installed PC110 route has its
named source and owner, but no next receiver has a complete protocol. The
remaining power/ASIC paths are source-gated: their known wiring cannot define
the guest-visible values, sequencing, or failure behavior needed to model
them. This is an explicit product-qualification block, not a missing test or
a license to copy PC110-EMU/QEMU synthetic values. See
[S35 evidence](../etc/evidence/t580-s35-post-delay-receiver-inventory.md).

## S36 — durable complete-delay observation

S36 repeats S34's one complete-routine observation only because S34's extended
run lost its completed terminal output. The identical 20M containment runs in
an observable background process, preserving its result long enough to decide
whether `F000:53D0h` actually returns at `F000:5405h`. It retains no test code
or output log afterward. No guessed interface follows from a no-return result.

### S36 Closure

The durable x64/x86 observation ran for about 40 minutes with CPU-time parity
and no fault or early exit, but neither completed the diagnostic's run loop
before wall-time containment. Since its result is emitted only at loop exit,
there is no entry/return/caller fact; the result is not a BIOS no-return claim.
The temporary mode and all task-local launch/output files were removed. The
whole delay routine is unsuitable as a practical diagnostic boundary; S37
returns to static, source-backed receiver-contract research. See the
[S36 evidence](../etc/evidence/t580-s36-complete-delay-timeout.md).

## S37 — EC-A guest-visible receiver contract (closed: source-gated)

S37 reconciled the selected BIOS EC-A helper family with current App routes and
Open-Source-PC110 evidence. The `15E8h`/`15ECh` ready/write mailbox is already
implemented. The BIOS-visible `15EAh`/`15EBh` and `15EEh`/`15EFh` pairs lack
the reset, operation and failure semantics needed for a receiver. No guessed
indexed file, zero-response route, QEMU synthetic register bank or BIOS
intercept is admitted. See the
[S37 evidence](../etc/evidence/t580-s37-ec-a-contract-reconciliation.md).

## S38 — observer-free complete-delay boundary

S36's retirement observer invokes App diagnostic work for every instruction,
so its wall time cannot establish normal execution cost or firmware
non-return. S38 temporarily adds one mode to the existing real-firmware
diagnostic. It installs the existing execute watchpoint at static return
address `F000:5405h`, deliberately installs neither trace nor retirement
observers, and retains only the existing machine result and copied CPU
snapshot. The same finite containment runs on x64 and x86. A hit proves only
that this BIOS control-flow boundary is reached under the existing machine;
it neither qualifies POST nor authorizes a synthetic receiver. The temporary
mode and raw output are removed before P.

### S38 Closure

The observer-free x64 run did not reach the static return boundary during its
180-second containment. Repeating the same long wait on x86 would add no
semantic fact, so it was not run. The temporary mode was removed and the
source-gated qualification boundary remains; see
[the S38 evidence](../etc/evidence/t580-s38-observer-free-delay-boundary.md).

## S39 — real-delay progress measurement

S39 replaces S38's silent no-observer containment with one temporary,
App-local diagnostic mode that reports only each completed million instructions
while continuing to execute the original BIOS unchanged. It reuses the existing
Core execute watchpoint at `F000:5405h`; no address intercept, device reply,
timing shortcut or observer is installed. The reports identify whether the
remaining work is finite interpreter throughput or a semantic stop, and are
removed with the mode before P.

### S39 Closure

The progress markers were buffered until process exit and the x64 observer-free
process still did not hit `F000:5405h` within 180 seconds. No duplicate x86
wait was run. Together with S38 this identifies interpreter throughput, not a
missing PC110 device route; see
[the S39 evidence](../etc/evidence/t580-s39-delay-progress-containment.md).

## S40 — generic BIOS-delay acceleration design

The next useful path is not another PC110 route or QEMU-style firmware
intercept. S40 audits whether the existing X86 Core can identify and compact a
strictly generic, interrupt-disabled counted I/O delay loop while preserving
the Core-owned clock, the actual port read contract and architectural register
effects. Any design tied to a BIOS address, PC110 profile, artificial port
value, duplicate scheduler or extra execution path is rejected. This S is
design-only until a single owner and proof obligations are established.

### S40 Closure

The audit rejects a PC110-local acceleration.  `x86_core_machine_run()`
publishes elapsed guest time after every retired instruction; its only
deadline-advance API is intentionally invalid while running.  The BIOS loop
reads port 61 on every candidate iteration, so a generic shortcut also needs a
device-neutral prediction contract and a CPU-side loop execution model that
preserves NMI, I/O and register boundaries.  Neither exists today.  This is a
separate X86 Core performance feature, not a firmware or board repair.

## S41 — MyPC110 Turbo execution policy

MyPC110 may use the existing X86-NXVM Turbo runner policy from construction:
it increases only the host rendezvous quantum and removes host pacing; Core
continues to retire instructions and publish the one guest time axis normally.
The policy is explicit in the App binding, preserves Standard as every other
binding's zero/default value, and remains user-selectable through the existing
`speed` command.  It adds no firmware-address rule, guessed device value, new
clock, scheduling path or Core behavior.

### S41 Closure

The constructor binding now validates and propagates an existing initial
runner-speed selection.  MyPC110 selects Turbo, while zero-valued bindings
remain Standard.  The x64 and x86 stable Ninja graphs built, and the relevant
X86-NXVM/MyPC110 suite passed 116/116 on each width.  This is an execution
policy verification only; the external BIOS remains source-gated rather than
declared boot-qualified.

## S42 — MyPC110 runner-path containment

S42 exercises the actual embedded-TFT preview through the normal Product,
Emulator and Turbo runner path for one fixed, hidden interval.  It may establish
only that the policy reaches the running process without an early fault; it
cannot turn elapsed wall time into a POST, Setup, DOS, display or release
claim.  The copied executable, adjacent INI and captured output are task-local
build outputs and are removed after the observation.  No firmware, receiver,
Core or Shared behavior is changed.

### S42 Closure

The native cooked-console reader does not accept the attempted redirected
command-pipe harness as a real monitor interaction. It terminated without a
reader transaction or guest result, so it cannot qualify the normal product
path. The task-local executable copy, INI and logs were removed after the INI
hash was verified. This does not alter S41's construction-policy proof or the
outstanding source gate.

## Verification And Exit

### S50 lower-Flash receiver

The [S50 receiver](../etc/evidence/t580-s50-lower-flash-selection.md) adds
App-owned port-98 selection to the sole external E/F decoder. Existing
fixtures prove lower/upper banks, independent shadow, guest-executed copying,
restoration and reset behavior. This is a source-labelled L2 functional model;
actual normal firmware has not yet reached its Setup bulk-copy/loader path.
A bounded sample ends in the later SMM checksum loop, not INT 19h and not a
proven deadlock. The real Setup/boot consumer and all original guest/live
product gates remain required; receiver implementation alone does not qualify
publication.

### S49 integrated-controller proof

The [S49 receiver](../etc/evidence/t580-s49-integrated-fdc.md) uses the existing
Board/controller/PIC/DMA path and original SMC fixed-status evidence. Both
actual TFT BIOS widths complete the FDC reset/IRQ/four-SENSE transaction with
no inserted drive or medium, retaining the earlier initialization/timer proof.
Shared trees and previously published Apps are unchanged. This is not full
SMC, POST, Setup, storage boot or PC110 product qualification; the lower Flash
and remaining ledger receivers continue under this T's original predicate.

### S48 post-initialization reconciliation

The [S48 audit](../etc/evidence/t580-s48-post-initialization-route.md) separates
working later ASMI/vendor/RSM from the missing FDC reset/IRQ receiver and the
independent lower Flash selection sequence. Both unchanged-firmware widths
remain before INT 19h. The next implementation batch reuses existing Board
FDC/PIC/DMA contracts, proving reset completion and empty-drive behavior;
it cannot fabricate the BIOS wait flag or inherit another board personality.
Flash/Setup, selected storage/guest, live panel/input/lifecycle and release
remain original T580 obligations.

### S47 receiver reconciliation

Actual unchanged-BIOS execution supersedes the S30 hard EC/ED gate assumption:
the caller uses the bank after F9 without FB. S47 removes that Boolean and
uses source-labelled, register-qualified C--F shadow access; the guest performs
all copies. Initializer entry, handler copy, return and timer pass are proved
on both host widths. This does not close the complete receiver: CPU-only
SMRAM visibility, SMIACT forwarding, trigger/acknowledge/rearm, RSM and the
boot-only lower Flash bank view retain their qualification requirements.
The [S47 record](../etc/evidence/t580-s47-cold-initialization-contract.md)
preserves sources, superseded observations and exact proof limits. Historical
S30 closure is not rewritten and no partial proof permits PC110 publication.

The final S47 continuation implements CPU-only active SMRAM, Board SMIACT
forwarding and the registered ASMI source/acknowledge/rearm protocol. Both
widths now execute the real vendor handler and RSM, then pass the existing
timer milestone with delta 166C; full unit and affected-artifact qualification
is recorded in the same evidence. Coordinator actual-change review accepts
this complete cold-receiver batch. Boot-only lower Flash selection and later
POST/Setup/guest/panel/product qualification remain T580 members; this does
not authorize PC110 publication or treat those rows as accepted.

S43 supersedes S36--S42's assumed complete-delay throughput diagnosis. A
sampled normal execution reaches the actual PIT calibration and an intentional
error halt. Its count difference, `4EFF`, violates the BIOS's `1614..1868`
range. IRQ0 delivery and its BIOS flag are functioning.

S44 corrects the App's existing PIT/RTC clock ratios and nominal refresh
period on the one 33 MHz Core axis. Both host widths now execute the unchanged
TFT BIOS through its real timer pass branch with delta `1668`. This is a
source-labelled L2 board model and one qualified POST stage; remaining POST,
panel, storage, guest and product-release requirements remain open. See the
[S43 diagnosis](../etc/evidence/t580-s43-real-pit-calibration-failure.md) and
[S44 proof](../etc/evidence/t580-s44-pit-refresh-clock.md).

Full required repository units and external integrations pass on x86/x64; affected prior Apps remain valid, releases/hashes/PE widths and retained EXE pairs are verified. Every PC110 domain is accepted or owner-disposed, manual limits are recorded, and owner acceptance closes the product claim.

Use direct owner tests plus the real production connection, full repository
unit gates and required external integrations under Execution. Reuse the two
existing Ninja/ccache graphs, rebuilding each shared target once per host
architecture/configuration. Rebuild and deploy every affected admitted App pair;
no PC110 artifact claims runnable qualification before real-firmware proof.
Record actual source/test diff, source grades, review, unresolved scope and
artifact identities; preserve owner INI, ROM masters, disks and snapshots.
