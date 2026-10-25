# T580 S46 — Software-SMI / SMRAM receiver audit

## Question and baseline

S45 reaches the real firmware's memory-flag wait and repeated output to 15EE.
This audit uses accepted baseline `99999d585`, the same immutable TFT firmware
and the three already pinned PC110 references. No new asset is acquired.

The finite receiver batch comprises the trigger and gating, cold SMRAM
preparation, active memory view, source acknowledgement/rearm and RSM return.
An isolated successful SMI entry cannot accept this batch.

## Existing owners

| Link | Actual implementation / evidence | Disposition |
| --- | --- | --- |
| CPU SMI/RSM | X86 Core CPU already latches SMI, saves its image, fetches SMBASE+8000 and restores through RSM. | Existing production mechanism; no replacement CPU executor or handler is needed. |
| Core signal and SMIACT | `x86_core_machine_set_smi()` and attachment `system_management_active` already exist. | Reuse; no Lib/Emulator gap is demonstrated. |
| Board-to-App SMIACT wiring | Board-Base's profile binding does not forward the existing output. | A future isolated SMRAM decoder needs a reviewed optional profile callback, not another Core execution API. No Shared edit is made here. |
| App software trigger | Existing EC-A model owns only 15E8/15EC; 15EE is not connected to SMI. | A plain unconditional SMI pulse is insufficient, as shown below. |
| Cold/active SMRAM view | App firmware installs C and F shadows and the reset alias, but no separate SMRAM decode. | Reconcile initialization and active mapping before implementing a trigger. |
| Clear/rearm | Firmware and the pinned Chipset record identify block2 B7 source handling and FA commit/rearm. | The hardware relation to the software trigger and initial decode remains to be established; do not invent register effects. |

## Bounded x64 experiments

All experiments used the existing App integration executable and normal Core
CPU execution, never a forced firmware PC, register return or memory flag.
Temporary pin injection is an experimental input, not an accepted hardware
model. Each run stops after its semantic sample or the existing containment;
fixed-size state and output remain below 64 KiB. All owned processes exited.

1. A temporary App 15EE write route pulsed the existing SMI input. The first
   request reaches the real CPU SMM entry at 38000 after 6,836,966 retirements,
   but its bytes are zero. No RSM executes; the sample stops after one quantum
   (4,096 retirements), not after an invented successful return. The incoming
   register request is AX=5380/BX=8300. The inspected E900:0006 initialization
   and copy checkpoints have not executed.
2. Clearing the live-capture SCAMP 01 bit 7 in the temporary cold image does
   not change that result. That proposed constant patch is rejected.
3. A diagnostic-only SMI stimulus delayed until the actual timer pass also
   reaches zero code at 38000. The request is AX=5380/BX=8424, at 59,995,247
   total retirements. It records no E900:0006 or E9118 initialization/copy.
   This delayed stimulus is not a production policy.
4. The grouped initialization checkpoints identify the real conditional:
   F000:4CEF calls 432D; at 4340 the BIOS reads SCAMP 01 as 82; the conditional
   at 4CF2 continues to 4CF7 and skips the 341B initialization call. The retained
   I/O facts show the BIOS itself writing 00, then 02, then 82 to this register.
   A different initial constant therefore cannot repair the mechanism.
5. A separate candidate removes the host-prepared C000 shadow contents so
   the BIOS sees RAM rather than a prefabricated Option ROM. It changes the
   early path and first request (AX=5380/BX=8701 at 3,013,959 retirements), but
   SMM entry is still zero and the initialization markers remain absent.
   This is not proof that C shadow prefill is the sole root cause or that
   removing it is a sufficient repair. It too is withdrawn.

Several runs refined the same sampler. That repeated startup cost was avoidable:
the successor must collect the complete initialization/control batch in one
planned observation, rather than adding one branch or register per replay.
No new permanent test or trace framework is justified by these experiments.

## Static reconciliation and next repair boundary

The immutable BIOS contains a genuine initializer at flash 29006/2907B.
Its 29118 preparation and 2912F copy prepare the 38000-based handler; flash 29000 begins with
a far jump to 3800:5B00. The handler, source loop and RSM are present in the
original firmware. Having those bytes in the ROM does not establish how the
board makes them available at CPU SMM entry.

The same initializer accesses port 22/23 controls and 302 bit 3. Other ROM
bank-copy code toggles port 98 bit 2 and changes EC/ED decode. Current App code
does not implement the complete relationship among those controls, the
initialization branch and active SMRAM. The earlier C-shadow diagnosis is thus
a candidate-path difference, not a proven complete root cause.

The next cohesive board repair must reconcile all of:

- cold ROM/RAM and C/E shadow visibility, including what the guest copies;
- the actual initialization route and its SCAMP/CMOS/board preconditions;
- SMRAM preparation/open/closed/active views and processor-reset preservation;
- the qualified 15EE trigger, CPU SMIACT, source clear/rearm and RSM return.

Use the existing CPU and Core signal contracts. If Board-to-App SMIACT needs
one optional callback, review that exact Shared surface before editing it.
Do not preload an invented handler, inspect the BIOS PC to enable SMI, return
success from AX=5380 in the host, or mutate the waited-on flag. Reference
emulators' intercepted services do not resolve these hardware decode facts.

## Actual-change and verification review

Every temporary production and test change is removed. The final source/test
diff against the accepted baseline is +0/-0; no interface, helper or production
state is retained. The x64 stable graph rebuilds the restored integration
binary and both fixed preview variants. The restored App units pass 11/11.
S44's unchanged-code dual-width timer and complete unit evidence remains
applicable; these experiments were x64 only, not x86, desktop, RDP or guest
qualification. No product EXE is deployed from an experiment.

All Shared packages, other Apps, owner INI, external ROM/font/media and
published artifacts remain unchanged. This audit establishes an exact
board-initialization/decode gap and rejects incomplete repairs; it does not
qualify POST, Setup, DOS, panel output or the four PC110 release artifacts.
