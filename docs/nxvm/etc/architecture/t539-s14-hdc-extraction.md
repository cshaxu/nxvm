# T539 S14: Fixed-Disk Controller Boundary

Baseline b2cbaf74c. This batch consumes the ledger's complete hdc.c/hdc.h row
and its construction, scheduling, diagnostics and test consumers. It extracts
existing controller personalities, not a claim of complete ATA, WD or Xebec
silicon. The S1 design remains authoritative for this boundary.

## Inspected Source And Ownership

The 1,188-line implementation has four frozen personalities: ATA PIO,
Compaq/WD 40MB, WD1003/ST-506 and Xebec XT. Task-file variants share sector
buffer/command machinery; Xebec has a distinct DCB/response/DMA state machine.
Keep these explicit protocols, not an ATA fallback or plugin registry.

- Shared `x86/devices/hdc`: registers, command capture versus live task file,
  CHS/LBA translation and progression, one 512-byte transfer buffer, Xebec
  DCB/sense, IRQ/DRQ causes, reset and service deadlines.
- NXVM `devices/hdc`: PC port-to-register mapping, media-ID/provider adaptation,
  PIC/DMA wiring and borrowed callback lifetime. The existing machine board
  retains registration/checkpoint rollback and Compaq/FDC wired-OR 3F7 reads.
- Profile retains geometry selection, board clock and resolved service inputs.
  Shared receives hardware values, never a machine name, IRQ number, DMA token,
  PC port, media registry, host file or profile configuration structure.

Keep one cohesive private implementation and header unless the actual cutover
demonstrates a separate owner. Public `hdc_interface.h` exposes an opaque
instance; owner-local tests may use private declarations. No product state
mirror or permanent forwarding compatibility implementation.

## Contract Decisions

1. A neutral personality tag selects exactly one implemented protocol at
   creation. Existing restrictions remain: task-file master/slave differences,
   unsupported IDENTIFY/commands, Xebec zero block count and limited command
   completions. No new ESDI capability is inferred.
2. Register operations identify a controller register/role, not its PC address.
   Task-file DATA is a 16-bit value; Xebec DATA is byte-sized. Distinguish normal
   versus alternate status acknowledgement and WD head-extension control.
   NXVM maps existing addresses and shared read ports to these roles.
3. A copied medium description supplies presence, write protection, known
   geometry and sector capacity. Bounded read/write operations transfer one
   512-byte sector and report absence/range/protection/failure explicitly.
   The controller owns translation, command failures and its buffer; the
   adapter owns external media identity and persistence, not another cursor.
4. IRQ and DRQ are synchronous level outputs with a borrowed context whose
   lifetime exceeds chip destruction. DMA byte service and TC are independent
   inputs. Construction failure publishes no instance; reset/destruction
   withdraw every active output before the context can be released.
5. Time uses the existing Core axis. Prefer caller-supplied absolute time,
   matching FDC and the scheduler's existing due_tick. No host clock, second
   board accumulator or diagnostic-snapshot-driven production clock. Due-now
   and no-event are distinct even when now is zero; repeated service at one
   timestamp must not invent ticks or replay a completed transition.
6. Service durations and WD step-selector durations are resolved construction
   values. Preserve accepted formulas/grades; the adapter derives them from
   its existing board input. The chip selects the guest-programmed rate, not
   a host/profile frequency. Keep the existing logical per-byte Xebec behavior.
7. Copied observations cover only current diagnostic/test consumers. Production
   never reads private phase, transfer buffer, task-file or DCB fields. Remove
   the old test-only immediate-advance helper from production and migrate its
   callers to a bounded fixture over the public deadline/advance contract.

Providers execute on the sole machine execution owner, without reentrant
reset/advance/destruction. Callback failures must become existing bounded
controller outcomes, never silent successful data or an instant-mode fallback.

## Findings To Prove Before Cutover Acceptance

The pre-extraction lifecycle regression reproduced both source findings on
x64: all four protocols omitted an immediate command's tick-zero deadline,
and Xebec DMA remained asserted after finalize. Reset was the passing control.
The current working correction uses pending phases rather than a zero-time
sentinel, permits zero elapsed service without inventing ticks, and withdraws
DMA before clearing the connection. These are not yet an accepted delivery.
The regression also checks both task-file sector directions across all three
personalities, no deadline while awaiting data, no repeated completion at the
same timestamp, and reset removing pending work. It and the existing HDC,
Compaq and Xebec checks passed on both widths before extraction. The working
cutover now has one opaque Shared owner and a board adapter with checked
construction. Production scheduling uses absolute time; test-only service and
copied diagnostics replace direct state access. The initial Shared contract
and existing board regressions pass on x64; this is not final verification.
Cold reset starts a new epoch, while guest reset-register writes preserve the
current epoch. Remaining work includes the complete original-case ownership
review, provider/allocation failure proof, full suites and eight artifacts.
The initial extracted x64 full unit run passed 356/356 on 2026-09-28.
Subsequent source review found that replacing Xebec's registry sector operation
with a neutral record callback had omitted the registry's capacity check.
An accepting provider reproduced incorrect read/write admission for a zero-capacity
medium on both command directions. The chip now checks capacity and its fixed
512-byte record size before transferring; Write Data still admits DMA before
checking media completion, preserving the original phase order. The regression
also rejects 256/1024-byte descriptions without invoking a record callback.
Owner-local allocation injection covers all four personalities, no instance or
signal publication on failure, successful retry, destruction and invalid protocol.
Both new Shared HDC tests pass on x64/x86. These later changes still require the
final full-suite run and receiving artifacts; they are not accepted delivery.
The shared contract test now owns the former Compaq S5 restore, initialize,
diagnostic and rejected IDENTIFY/22h command assertions; it also exercises the
WD1003 differences directly. The App test retains media-ID master/slave binding,
port decode, alternate-status acknowledgement, 3F7 wired-OR and absent binding.
Its two constructor callers now check allocation status before registering ports.

Sector addressing is now a single `lib_u64` sector index inside the chip;
only the media adapter converts it to a byte offset. This removes the prior
host-sized `lib_size` byte-offset intermediate (which truncated at 4 GiB on
x86), not a new addressing mode. Read/write regressions use LBA 00800000h and
0FFFFFFFh against a code-owned sparse provider; no large image is allocated.
They pass on both widths. The tools-off standalone build passes all 20 runtime
cases plus corpus/negative controls; both manifest checks passed after updating
the two changed hashes.

The original ATA assertion sequence now also runs independently in
`test/x86/devices/hdc/hdc_taskfile_smoke.c`, using local registers and a
code-owned two-sector provider. It retains reset defaults, NIEN and status
acknowledgement, read/write results, command-time media queries, range and
write-protection failures, no pre-read on writes, seven-tick command and
three-tick sector delays, busy/DRQ command rejection, both multi-sector
directions, IDENTIFY and reset during a pending command. The 160-line pure
phase/timing block is removed from the App test; its remaining cases prove
PC port decode, media registry/generation/result adaptation and WD board-clock
conversion. The removed ATA-SERVICE output marker has no build/script caller;
the independent runtime test now owns that proof. Both the independent and
retained App tests build and pass on x64/x86. This changes the unit inventory
and requires refreshed manifests and final full-suite results. After this
migration, x86 full units pass 358/358 in 38.94 seconds. The tools-off suite
passes 25/25 (21 runtime tests and four corpus/manifest checks); all six
manifests, the ATA static boundary and documentation governance also pass.
The corresponding x64 full run passes 358/358 in 202.83 seconds. Default
external integrations pass 20/20 on x86 (21.06 seconds) and x64 (18.35
seconds), rebuilding their embedded-ROM products. The specialized aggregate
passes, including the 379-row compilation audit and negative controls.
Vendor boot results and final artifact status are recorded below.
The full extracted unit suites subsequently passed 357/357 per width: x86 in
35.05 seconds and x64 in 202.28 seconds. Later board rollback review expanded
the former single registration-failure-17 test to all four protocols: 18 ATA,
18 WD1003, 19 Compaq (including wired-OR) and seven Xebec registration failures,
plus a failed chip constructor for each. Each of these 66 cases checks cleared
HDC state/topology, removed new ports and successful retry. Compaq preserves
the existing FDC read route; Xebec leaves no DMA request. Both widths pass.
The same matrix passed before and after consolidating the three duplicated
HDC construction cleanup blocks into one failure exit. The x86 ATA, Compaq
and Xebec integration-at-the-board unit cases also pass after that cleanup.
The final 358/358 runs above include this rollback cleanup; the earlier
357/357 results are intermediate evidence, not the final delivery baseline.
Sweep command,
next-read/write-sector, Xebec DCB, reset, TC, destruction and construction-failure
variants. Do not paper over these with caller sleeps or a board-only workaround.

## Complete Caller And Verification Batch

The initial private/immediate-advance sweep finds fourteen test/diagnostic
consumers: controller-authority, base HDC, Compaq HDC S5, Xebec wiring, VM HDC
ports, Model40 HDC S26/integration S8, 5170 and XT composition, HDD boot,
BYOB boot and three Windows31 probes/checkpoints. Production outside hdc.c/h
has no direct data/xebec/connection access; its public construction, scheduler,
DMA and port callers must still be reconnected and verified.

Move command/record/status/deadline cases to independent chip tests; retain
PC port wiring, media registry, DMA/PIC, 3F7 and machine assembly in NXVM.
Map every removed case to its new owner and preserve expected status/data,
including each sector's delayed BSY/DRQ boundary. Add allocation, registration
rollback, callback failure and output-release proof without a public test hook.

Run full NXVM units x64/x86, standalone tools-off x86 suite, all six manifests,
corpus/static boundary checks with negative controls, default integrations
both widths and each other profile/width boot once. Rebuild all eight 0539
EXEs; inspect MyNES link inputs and rebuild its 0043 pair only if affected.
Preserve user INIs, external masters and firmware packaging. Record actual diff,
code size and positive-growth justification; use complete target-separated
Shared/NXVM P deliveries followed by coordinator acceptance.

## Final Receiver Verification Progress

Final caller review includes execution ownership, not only private-header
removal. The Windows checkpoint and setup diagnostic readers still sample
HDC/CPU while their worker can mutate it. Their terminal diagnostic capture
must first confirm pause and complete the existing Common shutdown/join.
Stopping while Core is running can cold-reset its diagnostic counters. The
checkpoint waits for the published DIR file summary before pause, retaining
the nonzero-command predicate without racing on that counter.
No production API or new checkpoint is introduced. Rebuild these callers and
rerun the affected default integration suites before delivery.

The six vendor boot cases all passed once each, using the selected product
INI, external media and compiled firmware objects. XT x64/x86 took 18.70/24.39
seconds, AT 37.60/44.02 seconds and Model 40 61.82/79.36 seconds. Both reused
trees were restored to default configuration after the runs. These are named
boot checkpoints, not exhaustive manual UX qualification.

Final actual-source review corrected an obsolete Xebec comment that claimed
every DCB returned drive-not-ready despite the implemented READ/WRITE path.
The public contract now explicitly documents missing callback behavior already
implemented: unavailable media for no query, failed transfer for no read/write,
and an unconnected output for no signal callback. Both changes are comments
only; they do not alter the tested command logic. Six manifests and the
documentation gate pass after this cleanup. All eight product targets are
refreshed and their PE architecture and hashes verified in the
[receiver evidence](../evidence/t539-s14-hdc-extraction.md), before delivery
review. No user INI or external master changes are part of this batch.
