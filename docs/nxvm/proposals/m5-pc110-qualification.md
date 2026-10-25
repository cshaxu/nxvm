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

## Verification And Exit

Full required repository units and external integrations pass on x86/x64; affected prior Apps remain valid, releases/hashes/PE widths and retained EXE pairs are verified. Every PC110 domain is accepted or owner-disposed, manual limits are recorded, and owner acceptance closes the product claim.

Use direct owner tests plus the real production connection, full repository
unit gates and required external integrations under Execution. Reuse the two
existing Ninja/ccache graphs, rebuilding each shared target once per host
architecture/configuration. Rebuild and deploy every affected admitted App pair;
no PC110 artifact claims runnable qualification before real-firmware proof.
Record actual source/test diff, source grades, review, unresolved scope and
artifact identities; preserve owner INI, ROM masters, disks and snapshots.
