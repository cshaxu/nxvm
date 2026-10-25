# Project Status

## Current Work

## M5 T580 — Open after accepted S48

S48 is closed after coordinator actual-change review of `368a765f2`.
Its [route audit](../etc/evidence/t580-s48-post-initialization-route.md) proves
the later ASMI/vendor/RSM path and identifies the absent FDC reset/IRQ receiver
plus the independent lower Flash toggle/copy/restore route. Both host widths
remain before INT 19h; all product/guest requirements remain in the ledger.
Temporary code is removed, App units pass 11/11 per width, no artifacts or
owner inputs change, and no active S packet remains. Next admission wires and
qualifies the existing FDC/PIC/DMA path, not a synthetic BIOS completion flag.

## M5 T580 — Open after accepted S47

S47 is closed after coordinator actual-change review of Shared `794380f69`,
NXVM `782f09ebb` and MyRaidenII `5ed407b4b`. The complete cold-initialization
receiver now has CPU-only SMRAM, SMIACT, independent locks, ASMI source/acknowledge/
rearm and actual vendor-handler/RSM proof. Both widths pass all 712 repository
unit cases and the enhanced unchanged-BIOS timer integration (delta 166C),
plus four real PC INI identities. The eight PC and two affected Core-linked
MyRaidenII EXEs are refreshed; MyNES, Lib, Emulator and owner inputs are unchanged.

The [S47 evidence](../etc/evidence/t580-s47-cold-initialization-contract.md)
records source grades, complete proof, actual diff, artifact hashes and limits;
the [history](../history/M5-T580-pc110-qualification.md) retains its packet.
S48's accepted audit is summarized above. T580 stays open: boot-only lower
Flash selection, later POST/Setup/guest, live panel/input/lifecycle and PC110
release qualification retain their [ledger](../etc/evidence/t580-s1-pc110-qualification-ledger.md)
receivers. Next admission reconciles the complete post-initialization firmware
and boot route; it must not count the successful prefix as product completion.

## M5 T580 S46 — Closed: SMI/SMRAM receiver audit, repair outstanding

Accepted audit `5ae0cfbd3` proves CPU SMI entry but a missing prepared handler
view, and identifies the skipped initialization branch with its actual SCAMP
writes. It rejects and removes the isolated trigger, constant and C-shadow
experiments. Restored x64 previews build and App units pass 11/11; retained
production/test delta is zero. The [S46 record](../etc/evidence/t580-s46-smi-smram-receiver-audit.md)
defines the complete cold-decode/SMRAM/SMI repair batch without a boot claim.

| Task | Progress |
| --- | --- |
| T580 S46 | Bounded receiver audit accepted; implementation and product qualification remain open. Next admission consumes the complete cold-memory/SMRAM initialization batch, not a single trigger patch. |

## M5 T580 S45 — Closed: post-timer reset and service wait

Accepted diagnostic `6b1df7a14` classifies the first post-timer reset and records
the subsequent 15EE/AX=5380/BH=84 memory-flag wait after three processor resets.
The run reaches its 200M containment without INT 19h or a terminal fault.
Temporary sampler code is removed; retained production/test delta is zero.
[S45 evidence](../etc/evidence/t580-s45-post-timer-reset.md) preserves the
copied observations, source limits and next complete receiver batch.

| Task | Progress |
| --- | --- |
| T580 S45 | Closed after actual-diff review; T580 remains open for the firmware service/wait receiver and all outstanding product qualification. |

## M5 T580 S44 — Closed: PC110 PIT/refresh clock relationship

Accepted implementation `1f30224c3` corrects the App-owned L2 clock ratios.
Both host widths pass the real timer integration and all 712 current unit
cases have passing evidence. Source/test/build-registration delta is +100/-119,
net -19. All Shared packages and owner assets remain unchanged. The
[S44 record](../etc/evidence/t580-s44-pit-refresh-clock.md) retains the scope,
source grades, actual review, verification and unqualified later milestones.

| Task | Progress |
| --- | --- |
| T580 S44 | Closed after App clock correction and dual-width timer proof; T580 remains open for later POST, panel, guest, lifecycle and product release qualification. |

## M5 T580 S43 — Closed: live delay-counter diagnosis

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S43. |
| Admission And Approval | Owner-directed autonomous PC110 implementation, reaffirmed on 2026-10-25. |
| Objective | Determine whether the real BIOS delay is progressing by recording PC, CX, DX and guest time at normal bounded execution boundaries. |
| Non-goals | No firmware mutation, register forcing, synthetic receiver, loop skipping or boot claim. |
| Reference Baseline | `1c7d27e82`, S33 outer-return proof and S36/S39 incomplete timeout observations. |
| Candidate Proposal | Current PC110 qualification proposal; consume its real-firmware progression batch. |
| Files And ABI Surface | Existing App-local diagnostic and NXVM evidence only; no public API or production change. |
| Applicable Rules | Execution bounded diagnostics and coverage ledger; Architecture single execution/time owner; Coding reuse existing fixture; source policy external BIOS read-only. |
| Verification | Build existing diagnostic in stable Ninja trees; run one persistent pollable x64 process with explicit stdout flush. Inspect counters before deciding on a receiver or performance change. |
| Expected Markers | PC/CX/DX/tick samples and either the actual delay return or an exact bounded disposition. |
| Asset Needs | Existing hash-pinned external TFT firmware and font. |
| Reporting Requirements | Distinguish actual counter progress, stopped execution and host throughput; correct unsupported prior conclusions. |
| Stop Conditions | 16 million retired instructions or 120 seconds; no output trace beyond 64 KiB. Stop owned process if containment expires. |
| Exit Criteria | Durable sampled facts, actual-change review and appropriate verification; temporary diagnostic mode removed after diagnosis. |
| Original Owner Request | Implement PC110 autonomously through the normal task lifecycle. |
| Similar-Issue Sweep | Inspect all callers of the shared delay and both DH/DL countdown stages; do not infer a protocol gap from an unsampled timeout. |

### S43 Closure

The unchanged TFT firmware reaches its PIT/refresh calibration, then deliberately
halts at `F000:6999`. IRQ0 arrives and sets its BIOS flag; the calibration delta
is `0x4EFF`, outside the firmware's `0x1614..0x1868` range. Earlier complete-delay
timeout claims did not establish throughput as the cause: their instruction
ceiling cannot terminate a halted guest that retires no instructions while the
harness keeps advancing to device deadlines. S44 owns the App clock correction.
The temporary sampler was removed. See [S43 evidence](../etc/evidence/t580-s43-real-pit-calibration-failure.md).

## M5 T580 S42 — Closed: MyPC110 runner-path containment

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S42. |
| Objective | Exercise the real, embedded TFT App through its normal Turbo runner for one bounded interval, proving only that the policy reaches the product execution path without an early fault or synthetic firmware action. |
| Scope | Task-local copied preview executable and INI only; no source, asset, external firmware or model change. Capture process liveness and output, then remove the temporary harness. |
| Verification | Run hidden and bounded; record process state and output. This is not a POST, Setup, DOS, display or artifact qualification. |
| Stop Conditions | Any need to alter firmware, fake input/device data, add an observer or extend the run merely to wait out the known delay closes the containment without a new claim. |

### S42 Closure

The command-pipe harness is not a valid observation mechanism for this native
cooked-console App: it exits without producing a console-reader transaction or
guest result. Retaining it would test redirected host input rather than the
real Window/Console path. The copied preview, INI and logs were removed with
the reviewed build-cleanup tool. S42 makes no runner or guest-progress claim;
the source-gated receiver boundary remains the only honest T580 disposition.

## M5 T580 S41 — Closed: MyPC110 execution-policy qualification

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S41. |
| Objective | Make the qualified PC110 App use the existing Turbo runner policy during BIOS POST, without changing the Core time axis, port receiver, firmware bytes or CPU semantics. |
| Scope | Add one validated optional initial-speed field to the existing X86-NXVM product binding, select Turbo only for MyPC110, and cover its factory propagation. S40's Core loop-compaction concept is explicitly deferred as a separate general-performance effort. |
| Verification | Build both host widths from the stable Ninja graphs; run affected X86-NXVM and MyPC110 unit tests; confirm the App binding chooses Turbo and the other bindings preserve zero/default Standard. |
| Stop Conditions | Any need for a BIOS address intercept, fake port value, additional guest clock, scheduler path or changed default for another App rejects this policy change. |

### S41 Closure

The App now selects the existing Turbo runner policy at construction.  Both
stable Ninja graphs built successfully, and the affected X86-NXVM/MyPC110 test
selection passed on x64 (116/116) and x86 (116/116).  This validates policy
propagation and preserves Standard as the default for every other binding.  It
does not claim that the external BIOS has reached a boot boundary: the
source-gated receiver qualification remains the T580 blocker.

## M5 T580 S40 — Closed: generic BIOS-delay acceleration design

S40 confirmed that Core cannot safely reuse its stopped/paused deadline advance
while a CPU instruction stream is active.  The selected BIOS loop repeatedly
observes port 61, and Core has no device-neutral contract that can predict when
an arbitrary I/O read changes; compacting it would require a new general loop
execution model, not a PC110 fix.  QEMU's address intercept is therefore still
rejected.  The immediate bounded action is S41's existing Turbo policy; any
generic trace/loop compiler is a separate Core performance task.  See the
[S40 evidence](../etc/evidence/t580-s40-generic-delay-acceleration-design.md).

## M5 T580 S39 — Closed: PC110 real-delay progress measurement

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S39. |
| Objective | Obtain a bounded, durable progress measurement for the selected TFT BIOS's real complete port-61 delay without installing a per-retirement observer or changing guest behavior. |
| Scope | One temporary mode in the existing App-local diagnostic; it uses the existing execute watchpoint at `F000:5405h` and reports only per-million executed-instruction progress. No production, Shared/Core ABI, firmware, asset or artifact change. |
| Verification | Build and run the existing diagnostic from the stable x64/x86 Ninja graphs in one persistent, pollable session per width. A hit, fault or exact instruction containment is recorded; raw output and mode are removed before P. |
| Stop Conditions | Any production change, guessed receiver, BIOS interception, source expansion, inconsistent width result or a wall/no-progress containment ends this S without implementation. |

### S39 Closure

The progress markers were stdout-buffered and the observer-free x64 process
still did not reach `F000:5405h` within 180 seconds. The temporary mode was
removed. This confirms an interpreter-throughput limitation rather than an
identified PC110 route gap; see the
[S39 evidence](../etc/evidence/t580-s39-delay-progress-containment.md).

## M5 T580 S38 — Closed: PC110 observer-free complete-delay boundary

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S38. |
| Admission And Approval | The owner directed autonomous continuation on 2026-10-25 after S36 showed the per-retirement observer made the complete delay impractical. |
| Objective | Determine whether the selected TFT BIOS reaches the already-static `F000:5405h` return boundary when no retirement or trace observer is installed. |
| Non-goals | No production change, firmware interception, timing shortcut, guessed receiver, Shared/Core ABI change, artifact, POST or boot claim. |
| Candidate Proposal | Temporarily add one existing-diagnostic mode that installs only the existing execute watchpoint at `F000:5405h`; run it with a finite instruction containment on both host widths. |
| Files And ABI Surface | Temporary `test/app-mypc110/diagnostic/real_firmware_post_diagnostic.c`, this packet and S-local evidence only. The mode and all raw output are removed before P. |
| Verification | Reuse the stable x64/x86 Ninja graphs; inspect the existing Core stop reason and copied CPU snapshot. A watchpoint hit is the only success marker; a budget/fault is a bounded result, not a product claim. |
| Stop Conditions | Any need for a receiver, device value, BIOS interception, production/ABI change, inconsistent host-width result or unbounded run ends this diagnostic without implementation. |
| Exit Criteria | Record a durable x64/x86 watchpoint or bounded no-hit disposition, remove temporary diagnostic material, and restore a clean source tree. |

### S38 Closure

The observer-free x64 containment did not reach `F000:5405h` in 180 seconds.
It was deliberately stopped; a duplicate x86 wait would not add a semantic
fact. The temporary mode was removed. This neither establishes a BIOS
non-return nor selects a missing receiver. See the
[S38 evidence](../etc/evidence/t580-s38-observer-free-delay-boundary.md).

## M5 T580 — Active: real-firmware product qualification

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S44 is active; T580 remains open. |
| Admission And Approval | The owner directed autonomous T580 execution on 2026-10-25. S36 established that another undifferentiated complete-delay execution budget has no practical diagnostic value. |
| Objective | Complete the PC110 qualification ledger through actual firmware and product behavior; S43 supersedes the delay-throughput assumption with a measured PIT calibration failure, repaired in S44. |
| Non-goals | No longer instruction-run probe, production route, guessed mailbox or power-MCU reply, BIOS interception, Shared/Core ABI change, firmware/media change, artifact or boot claim. |
| Reference Baseline | `3dceb3c42` plus S35 route inventory and S36 timeout evidence. |
| Candidate Proposal | S37 reconciled selected BIOS EC-A helpers and Open-Source-PC110 records; the only complete mailbox subset is already owned. |
| Files And ABI Surface | Documentation/evidence only; no source/test/build/asset change is admitted without new evidence. |
| Applicable Rules | `docs/rules/EXECUTION.md` coverage-ledger and diagnostic-containment rules; source policy; Architecture/Coding single-owner and no-synthetic-route constraints. |
| Verification | S37 cross-checked BIOS-visible EC-A helpers, existing App routes and source records. Documentation governance and diff checks apply. |
| Expected Markers | Explicit source-gated disposition and retained owner map. |
| Asset Needs | Existing hash-pinned external TFT BIOS and research repositories read-only; no download, derivation, movement or commit. |
| Reporting Requirements | Distinguish source facts from reference-emulator behavior; zero production/test/artifact diff is retained. |
| Stop Conditions | Required guessed value, BIOS interception, incomplete protocol, Shared/Core change or absent source keeps T580 source-gated. |
| Exit Criteria | New evidence defines a source-backed receiver or an owner explicitly authorizes a bounded L2 model. |
| Original Owner Request | Continue the whole PC110 qualification task without repeated routine approvals, while maintaining source-backed receiver boundaries. |
| Similar-Issue Sweep | S37 examined all selected-BIOS EC-A helper pairs; do not create an overlapping trace framework. |

### S37 Closure

The EC-A reconciliation found that the currently implemented `15E8h`/`15ECh`
ready/write mailbox is the only complete source-backed subset. `15EAh`/`15EBh`
and `15EEh`/`15EFh` lack reset and protocol semantics, so no receiver is added.
See the [S37 evidence](../etc/evidence/t580-s37-ec-a-contract-reconciliation.md).

## M5 T580 S36 — Closed: PC110 durable complete-delay observation

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S36. |
| Admission And Approval | The owner directed autonomous T580 execution on 2026-10-25. S34's 20M experiment lost its terminal output after the child exited, so its result is not evidence; this S repeats that one bounded observation with durable task-owned output. |
| Objective | Determine whether the selected BIOS's real `F000:53D0h` routine returns through `F000:5405h` and identify its real caller on both host widths. |
| Non-goals | No production receiver, timing shortcut, BIOS interception, guessed I/O reply, Shared/Core/App ABI change, firmware/media change, release artifact, or POST/boot claim. |
| Reference Baseline | `3dceb3c42` (S35), S33 outer-delay return, S34 no-result disposition, and selected TFT BIOS hash. |
| Candidate Proposal | Temporarily restore one unregistered diagnostic mode with a fixed 20M-retirement containment. Run under a hidden background process that persists stdout/stderr as S-local disposable evidence. Success requires actual entry, `F000:5405h` return and first caller; containment is never success. |
| Files And ABI Surface | Temporary `test/app-mypc110/diagnostic/real_firmware_post_diagnostic.c` only, its S36 evidence/status/proposal, and disposable `build/t580-s36/` logs. The temporary mode is removed before P regardless of result. No production/public ABI/CMake/manifest change. |
| Applicable Rules | `docs/rules/EXECUTION.md` coverage and diagnostic-containment rules; `ARCHITECTURE.md` single time owner; `CODING.md` no synthetic route; source policy. |
| Verification | Build each existing Ninja width, execute the named mode in a hidden background process, verify the process's terminal exit and both captured outputs, then remove temporary source/logs and run governance/diff checks. |
| Expected Markers | `PC110:POST:c000-port61-delay=entered:1 ... returned:1 caller=...` on both widths, or a retained bounded no-return marker. |
| Asset Needs | Existing hash-pinned external TFT BIOS only; no download, derivation, movement, or commit of firmware/media. |
| Reporting Requirements | Record x64/x86 exit/result, wall time, exact return/caller if any, temporary source removal, logs removed or retained only as summarized evidence, and no production/artifact claim. |
| Stop Conditions | Timeout containment, missing/ambiguous result, inconsistent width result, source/test scope expansion, need for a guessed device value, or any production change ends the mode with a no-result disposition. |
| Exit Criteria | A durable real return/caller fact replaces S34's no-result, or both bounded observations explicitly produce no return; no temporary code/log persists and no unapproved behavior is introduced. |
| Original Owner Request | Continue the whole PC110 qualification task without repeated routine approvals, while maintaining source-backed receiver boundaries. |
| Similar-Issue Sweep | Reuse exactly the one temporary complete-routine diagnostic and one output-capture mechanism; do not add overlapping trace frameworks or follow arbitrary post-return instructions. |

### S36 Closure

The two durable runs remained host-width-consistent for about 40 minutes but
did not return from the whole BIOS delay routine before the S-local wall-time
containment. They therefore provide no entry/return/caller fact and are not
treated as a no-return fact. All temporary diagnostic code, Scheduled Tasks
and logs were removed. S33 remains the retained dynamic boundary; S37 returns
to source-backed receiver-contract research. See the
[S36 evidence](../etc/evidence/t580-s36-complete-delay-timeout.md).

## M5 T580 S35 — Closed: PC110 post-delay receiver inventory

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S35. |
| Admission And Approval | The owner directed T580 to continue autonomously on 2026-10-25. S34 closed without retaining an unbounded whole-delay probe; the next step is a finite source/owner inventory, not another instruction-budget experiment. |
| Objective | Reconcile the selected TFT BIOS's post-delay interfaces with the currently implemented PC110 routes, and identify the earliest source-backed missing receiver or an explicit source-gated block. |
| Non-goals | No production route, BIOS interception, QEMU/PC110-EMU special case, guessed power/MCU reply, media import, Shared/Core ABI change, artifact, POST, Setup, or guest-boot claim. |
| Reference Baseline | `497e53e18` (S34), S33's retained outer-delay return, the selected external TFT BIOS identity recorded in S26, the existing App route map, and the PC110 programme ledger. |
| Candidate Proposal | Inspect only real host-visible routes: 8042 at `60h/64h`, admitted EC windows, C-segment/F-segment mappings, storage/LPT paths and their reset values. Cross-check Open-Source-PC110 wire/firmware evidence and treat PC110-EMU/QEMU interception as non-importable reference evidence only. |
| Files And ABI Surface | T580 status/proposal/evidence only unless a complete source-backed owner and minimal boundary are found; no source/test/build/asset file is in scope for this inventory. |
| Applicable Rules | `docs/rules/EXECUTION.md` coverage-ledger and diagnostic-containment rules; `ARCHITECTURE.md` one owner per route; `CODING.md` no synthetic firmware route; source policy research boundary. |
| Verification | Reconcile each inspected BIOS-visible route against App source and named external evidence; run documentation governance and diff checks. No build/test is claimed unless implementation is separately admitted. |
| Expected Markers | A finite table identifying each route as implemented/source-backed, source-backed but unimplemented with an owner, or insufficiently specified. Any external special case is explicitly rejected as a product implementation source. |
| Asset Needs | Read existing external sources and the hash-pinned TFT BIOS only; no download, derivation, movement, or commit of firmware/media. |
| Reporting Requirements | Record each route, source grade, existing owner, missing fact, and disposition; distinguish source facts from emulator reference behavior and report a zero production/test/artifact diff if none is warranted. |
| Stop Conditions | A necessary guessed register value, a need for BIOS interception or external code import, absent source-backed protocol, or a required Shared/Core change ends the inventory with an explicit transfer/block instead of implementation. |
| Exit Criteria | The finite post-delay candidate routes are reconciled with one source-backed next receiver or an explicit no-receiver disposition; no implementation claim is made from a trace budget. |
| Original Owner Request | Continue the whole PC110 qualification task without repeated routine approvals, while maintaining source-backed receiver boundaries. |
| Similar-Issue Sweep | Check all current PC110 App-installed I/O/memory routes and the corresponding external evidence; do not inspect only the first apparent BIOS delay or copy reference-emulator hooks. |

### S35 Closure

The finite inventory found no missing route with a complete source-backed
contract. The existing App owns the conventional 8042 path, EC/ED captured
bank, both admitted EC windows, C/F shadows, ATA, LPT and the PC110 plain
`80h`--`8Fh` file. Open-Source-PC110 confirms the wiring but leaves the U6
power protocol/thresholds and custom-ASIC register behavior incomplete.
PC110-EMU/QEMU cross the next stages with synthetic values and BIOS execution
intercepts, which are explicitly excluded. No production/test/asset change is
warranted. See the [S35 inventory](../etc/evidence/t580-s35-post-delay-receiver-inventory.md).

## M5 T580 S34 — Closed: PC110 complete delay-routine boundary

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S34. |
| Admission And Approval | The owner directed T580 to continue autonomously on 2026-10-25. S33 proved the inner `EA38h` loop returns quickly enough to make the containing BIOS routine the next and final useful delay boundary. |
| Objective | Determine whether the real `F000:53D0h` delay routine returns through `F000:5405h`, and record the actual return receiver without using an arbitrary retirement count as progress. |
| Non-goals | No production receiver, timing shortcut, Shared/Core/App ABI, firmware/media change, device model, release artifact or guest-qualification claim. |
| Reference Baseline | `4a2ab9d88` (S33), selected TFT BIOS hash recorded in S26, and static `F000:53D0h` control flow. |
| Candidate Proposal | Extend the existing one diagnostic binary with a temporary, unregistered complete-routine mode. Record entry `DX`, the `F000:5405h` `ret`, and its actual returned-to caller on both widths. Retain it only if it is bounded and semantic; otherwise remove it and record no-result evidence. |
| Files And ABI Surface | `test/app-mypc110/diagnostic/real_firmware_post_diagnostic.c`, `test/app-mypc110/CMakeLists.txt` only if the checkpoint qualifies, T580 status/proposal/evidence only. No production source, public ABI or component manifest. |
| Applicable Rules | `docs/rules/EXECUTION.md` S-contract/diagnostic containment/test-bloat rules; `ARCHITECTURE.md` single Core time owner; `CODING.md` diagnostic-copy/no-synthetic-route rules; source policy research boundary. |
| Verification | Both Ninja widths built the temporary mode. The 13M bounded run established entry `DX=0102h` but no routine return. The later 20M run produced no retained semantic result after its completed child output became unavailable, so its temporary source is removed. |
| Stop Conditions | Missing semantic return, inconsistent width fact, a need for guessed firmware/device behavior, a run beyond the declared containment, Core/Shared production change, or no source-backed receiver ends the mode and records no-result evidence. |
| Exit Criteria | Either a reproducible whole-routine return/caller fact replaces S33's narrower test, or the temporary mode is removed with a bounded no-result; no unapproved production behavior is introduced. |
| Original Owner Request | Continue the whole PC110 qualification task without repeated routine approvals, while maintaining source-backed receiver boundaries. |
| Similar-Issue Sweep | Expand the one existing diagnostic only to the smallest complete static routine; do not retain an overlapping inner-loop test or follow arbitrary post-return instruction counts. |

### S34 Closure

The 13M containment run enters the complete delay routine with `DX=0102h` and
does not reach `F000:5405h`. A later 20M temporary experiment could not yield
a retained semantic result after its child had exited but its terminal result
was unavailable. It is therefore not evidence of completion, duration, POST
progress, or a receiver. The mode is removed in full; S33 remains the sole
admitted dynamic delay checkpoint. No production, ABI, firmware/media, or
artifact changed. See the [S34 evidence](../etc/evidence/t580-s34-complete-delay-no-result.md).

## M5 T580 S33 — Closed: PC110 outer-delay completion correction

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S33. |
| Admission And Approval | The owner directed T580 to continue autonomously on 2026-10-25. S32's wall-time extrapolation used startup cost rather than measured loop cost, so this corrective semantic checkpoint is required before retaining its no-test disposition. |
| Objective | Run the already entered, real `BL=70h` BIOS outer delay to its actual `EA7Fh` return and record the real return receiver, without treating an instruction ceiling as success. |
| Non-goals | No production receiver, timing shortcut, Shared/Core/App ABI, firmware/media change, device model, release artifact or guest-qualification claim. |
| Reference Baseline | `4b6326c19` (S32), selected TFT BIOS hash recorded in S26, static `F000:53DAh` caller map and the existing post-C000 diagnostic. |
| Candidate Proposal | Replace the current caller CTest in place with one outer-return checkpoint. Record EA38 entry `BL`, EA55 iteration count, EA7F return and the first returned-to caller; retain one bounded containment ceiling only. |
| Files And ABI Surface | `test/app-mypc110/diagnostic/real_firmware_post_diagnostic.c`, `test/app-mypc110/CMakeLists.txt`, T580 status/proposal/evidence only. No production source, public ABI or component manifest exists on this App-local test tree. |
| Applicable Rules | `docs/rules/EXECUTION.md` S-contract/diagnostic containment/test-bloat rules; `ARCHITECTURE.md` single Core time owner; `CODING.md` diagnostic-copy/no-synthetic-route rules; source policy research boundary. |
| Verification | Build both Ninja widths; run the replacement named MyPC110 diagnostic on x64/x86; inspect entry/count/return facts; run the required repository-only unit suite before P. |
| Expected Markers | `PC110:POST:c000-port61-outer=... initial-bl=70 iterations=112 caller=...` on both widths. Success is semantic completion, never a retirement count. |
| Asset Needs | Existing external hash-pinned TFT BIOS only; no asset creation, download, movement or commit. |
| Reporting Requirements | Record exact caller/counter/return facts, actual loop run times, corrected S32 disposition, production diff (expected zero), x64/x86 results and unqualified limits. |
| Stop Conditions | Missing semantic return, inconsistent width fact, a need for guessed firmware/device behavior, Core/Shared production change, or a non-source-backed receiver stops implementation and records no-result evidence. |
| Exit Criteria | Real outer-loop entry and return are reproducible on both widths; the prior performance extrapolation is corrected; no unapproved production behavior is introduced; required diagnostic/unit evidence and documentation are complete. |
| Original Owner Request | Continue the whole PC110 qualification task without repeated routine approvals, while maintaining source-backed receiver boundaries. |
| Similar-Issue Sweep | Replace the sole caller case rather than retain overlapping slow diagnostics; distinguish measured loop work from preceding BIOS startup before deriving a test budget. |

### S33 Closure

The single replacement checkpoint completes the real outer delay on both
widths: it enters `EA38h` with `BL=70h`, observes all 112 `EA55h` tails,
returns through `EA7Fh`, and lands at `F000:53DDh`. The former caller-only
branch was removed. Final named diagnostics take 40.01 seconds on x64 and
45.73 seconds on x86; full repository-only units pass 667/667 on both widths.
S32's startup-inclusive hour-scale inference is superseded. This selects no
production receiver and does not qualify POST, setup, storage, panel behavior,
guest boot, or a MyPC110 artifact. See the
[S33 evidence](../etc/evidence/t580-s33-outer-delay-return.md).

## M5 T580 S32 — Closed: PC110 outer-delay counter fact

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S32. |
| Admission And Approval | The owner directed T580 to continue autonomously on 2026-10-25. This is a diagnostic-only semantic checkpoint following S31. |
| Objective | Record the real low `BL` counter value at S31's established post-`EA80h` caller (`F000:EA5Bh`) before deciding whether a complete outer-loop diagnostic is bounded and useful. |
| Non-goals | No production receiver, timing shortcut, Shared/Core/App ABI, firmware/media change, device model, release artifact or guest-qualification claim. |
| Reference Baseline | `8958da82e` (S31), selected TFT BIOS hash recorded in S26, and the existing `c000-port61-caller` diagnostic. |
| Candidate Proposal | Extend the existing caller checkpoint by copying the already supplied retirement CPU snapshot's low `EBX` byte at the first real caller. The existing CTest is strengthened in place; the former output/contract is not duplicated. |
| Files And ABI Surface | `test/app-mypc110/diagnostic/real_firmware_post_diagnostic.c` and T580 status/proposal/evidence. Full-suite verification also corrects the existing Lib/Emulator static-fixture time budgets and their manifests: `test/{lib,emulator}/CMakeLists.txt`, `test/{lib,emulator}/MANIFEST.sha256`. No production source or public ABI changes. |
| Applicable Rules | `docs/rules/EXECUTION.md` S-contract and test-bloat rules; `ARCHITECTURE.md` single Core time owner; `CODING.md` diagnostic-copy/no-synthetic-route rules; source policy research boundary. |
| Verification | Build both Ninja widths; run the existing strengthened named MyPC110 diagnostic on x64/x86; inspect the caller/counter fact; run the required repository-only unit suite before P. |
| Expected Markers | `PC110:POST:c000-port61-caller=... bl=..` on both widths. The counter fact is diagnostic evidence only, not a loop-completion or firmware-progress claim. |
| Asset Needs | Existing external hash-pinned TFT BIOS only; no asset creation, download, movement or commit. |
| Reporting Requirements | Record exact caller/counter, disposition for a possible later full-loop checkpoint, production diff (expected zero), diagnostic/doc line counts, x64/x86 results and unqualified limits. |
| Stop Conditions | A missing established caller, an inconsistent width result, a need for guessed firmware/device behavior, Core/Shared production change, or a non-source-backed receiver stops implementation and records no-result evidence. |
| Exit Criteria | The real caller has a reproducible counter fact on both widths; no unapproved production behavior is introduced; required diagnostic/unit evidence and documentation are complete. |
| Original Owner Request | Continue the whole PC110 qualification task without repeated routine approvals, while maintaining source-backed receiver boundaries. |
| Similar-Issue Sweep | Strengthen the sole caller diagnostic rather than adding a second counter probe or a second trace framework; retain only semantic facts used to choose a later receiver/checkpoint. |

### S32 Closure

The established real caller reports `BL=6Fh` on both widths, directly after
the BIOS `dec bl`; its prior loop value is `70h`. Its hour-scale wall-time
extrapolation was corrected by S33: it included pre-loop firmware startup, not
the loop's measured cost. S33 replaces this superseded caller checkpoint with
one outer-return checkpoint. No production source, ABI, firmware/media input
or release artifact changed. See the [S32 evidence](../etc/evidence/t580-s32-outer-delay-counter.md)
and the [S33 correction](../etc/evidence/t580-s33-outer-delay-return.md).

The complete unit suite initially exposed three valid isolated CMake checks
whose inherited 30-second budget was below their measured duration. Their
existing time limit is raised to the approved 300 seconds; both widths then
pass 667/667. This alters no source/test assertion or product behavior.

## M5 T580 S31 — Closed: PC110 post-C000 port-61 caller boundary

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S31. |
| Admission And Approval | The owner directed T580 to continue autonomously on 2026-10-25. This S is a diagnostic-only continuation inside the established source/semantic-checkpoint rule. |
| Objective | Identify the actual caller reached after the admitted post-C000 `F000:EA80h` port-61 service return, then retain only a real semantic boundary that can select a source-backed receiver. |
| Non-goals | No production device route, Shared/Core/App interface, ROM/media mutation, timing shortcut, synthetic firmware action, release artifact, or guest-qualification claim. |
| Reference Baseline | `f4cfa9f25` (S30); existing `mypc110-real-firmware-post-diagnostic`, S28's static/observed EA38--EA92 path, and the selected TFT BIOS hash recorded in S26. |
| Candidate Proposal | Extend the existing one diagnostic binary only: after its established EA80 return, record the actual post-`ret` caller point. The static EA38 routine is not assumed to be this caller. A containment ceiling protects execution but is never success. |
| Files And ABI Surface | `test/app-mypc110/diagnostic/real_firmware_post_diagnostic.c`, its existing CMake registration and T580 evidence/status/proposal only. No production source, public ABI or component manifest exists on this App-local test tree. |
| Applicable Rules | `docs/rules/EXECUTION.md` S-contract, coverage and evidence rules; `ARCHITECTURE.md` single Core time owner; `CODING.md` diagnostic-copy and no synthetic-route rules; source policy research boundary. |
| Verification | Build both Ninja widths; run the existing and new named MyPC110 diagnostics on x64/x86; inspect emitted register/return/I/O facts; run the required repository-only unit suite before P. |
| Expected Markers | A successful checkpoint must prove the established EA80 return and record its actual caller. A bounded no-caller result is recorded as no receiver, not success. |
| Asset Needs | Existing external hash-pinned TFT BIOS only; no asset creation, download, movement or commit. |
| Reporting Requirements | Record exact observed caller, receiver disposition, production diff (expected zero), diagnostic/doc line counts, x64/x86 results and unqualified limits. |
| Stop Conditions | Missing dynamic semantic return, a need for guessed port values/device behavior, an arbitrary budget claim, Core/Shared change, or a non-source-backed receiver stops implementation and records a no-result. |
| Exit Criteria | The returned EA80 service has an actual-caller or bounded no-caller disposition on both widths; any selected receiver is separately source-gated; no unapproved production behavior is introduced; required diagnostic/unit evidence and documentation are complete. |
| Original Owner Request | Continue the whole PC110 qualification task without repeated routine approvals, while maintaining source-backed receiver boundaries. |
| Similar-Issue Sweep | Reuse the existing diagnostic runner rather than adding a second tracing framework; audit all newly captured trace paths for whether they are a real device receiver or an already-owned route. |

### S31 Closure

The real TFT path returns from `EA80h` to `F000:EA5Bh`, the BIOS's actual
`dec bl; jnz EA55h` outer delay tail, on both host widths. This is an
already-owned finite delay route, not a new receiver. The stronger caller
checkpoint replaces the former local-return diagnostic without increasing the
diagnostic count. Repository-only unit suites pass 667/667 on x64 and x86.
See the [S31 evidence](../etc/evidence/t580-s31-post-c000-port61-caller.md).

## M5 T580 S30 — Closed: PC110 EC/ED configuration gate

| Field | Required record |
| --- | --- |
| Objective | Restore the source-recorded `FBh` open / `F9h` close gate around the existing PC110 `ECh`/`EDh` configuration window. |
| Boundary | MyPC110 configuration adapter, firmware-owned reset state, its unit/diagnostic coverage and T580 evidence only. No Shared/Core contract, ROM mutation, release artifact or synthetic chipset behavior. |
| Evidence | Open-Source-PC110 records the gated protocol and a live sweep; the existing App is the sole owner of EC/ED index/data state and its C-segment shadow side effect. |
| Design | Store one reset-closed bit on the existing firmware binding. A closed window returns `FFh` for reads and ignores writes; `FBh` opens and `F9h` closes. |
| Verification | Exercise closed, open, closed-again and reset states through the actual ports; retain gate ports in the existing real-firmware trace filter. Build and run the affected x64/x86 X86-NXVM/MyPC110 corpus. |
| Limit | The established real-firmware checkpoint does not yet observe either gate port. This does not claim POST, setup, guest boot, panel qualification or publishable MyPC110 artifacts. |

### S30 Closure

The App no longer exposes EC/ED permanently. Its existing firmware binding
resets the gate closed and remains the only owner of the index, selected data
and shadow-write side effect. Both established host widths pass all 253
affected X86-NXVM/MyPC110 tests. See the [S30 evidence](../etc/evidence/t580-s30-eced-config-gate.md).

## M5 T580 S29 — Closed: PC110 DMA-page / Boot/IPL receiver

| Field | Required record |
| --- | --- |
| Objective | Replace the PC/AT DMA-page alias at PC110 ports `80h`--`8Fh` with the source-qualified PC110 plain read/write scratch file, including its `8Bh` Boot/IPL latch, and observe the TFT BIOS's real `out 8Bh,0; int 19h` boundary without treating an instruction count as success. |
| Boundary | `x86-nxvm` Board DMA wiring, MyPC110 plan/tests/diagnostic, and T580 evidence only. No Lib/Emulator/Core change, firmware mutation, external media import, host BIOS interception, nonzero latch invention or PC110 release artifact. |
| Evidence | Open-Source-PC110 identifies `8Bh` as the passive Boot/IPL latch; PC110-EMU and PC110-QEMU independently model `80h`--`8Fh` as zero-reset plain scratch. The selected TFT BIOS statically reaches `F000:52BA` only through its own real code, where it writes zero to `8Bh` before `INT 19h`. |
| Design | Add one copied Board configuration fact selecting the existing DMA-page range's plain-scratch personality. Existing PC-family boards retain their current DMA page mapping. PC110 receives one Board-owned 16-byte zero-reset file; no App side table, routing override, timer, retry path or new executor is introduced. |
| Verification | Exercise both DMA personalities, isolation and reset through their actual ports; build/test x64/x86 affected labels. The existing bounded real-firmware diagnostics establish the preceding post-C000 boundary, but do not yet observe the later `8Bh` write; that remains an explicit source-gated no-result. |
| Stop condition | Any need for a guessed latch value, a Shared Core/Lib/Emulator contract, a synthetic `INT 19h` route, protected media, or a budget-only firmware claim stops the implementation and records evidence. |

### S29 Closure

`x86-nxvm` now owns one configuration-selected plain scratch personality for
PC110 ports `80h`--`8Fh`; normal PC-family Board configurations retain their
DMA page routes. The second 8237 control range remains present in both
two-controller personalities. x64 and x86 each pass the 253-test affected
X86-NXVM/MyPC110 label, including the actual PC110 composition and existing
real-firmware diagnostics. This establishes the passive latch boundary only;
it does not claim that POST reached `out 8Bh`, that `INT 19h` booted media, or
that MyPC110 is artifact-qualified. See the
[S29 evidence](../etc/evidence/t580-s29-dma-page-boot-ipl-latch.md).

## M5 T580 S28 — Closed: post-delay execution boundary

| Field | Required record |
| --- | --- |
| Objective | Starting from the admitted real TFT BIOS return at `F000:EA84`, identify the next repeated execution boundary or actual device operation without treating an instruction ceiling as firmware progress. |
| Boundary | MyPC110 diagnostic and NXVM evidence only. No production receiver, Shared/Core change, firmware mutation, external media import or product artifact publication. |
| Method | Extend the existing bounded real-firmware diagnostic with a copied retirement ring only after the returned delay frame. Run both established host widths, classify the observed control flow, then remove any probe that does not select a concrete source-backed receiver. |
| Stop condition | Any required synthetic BIOS action, inferred device value, external model import, or lack of a reproducible semantic boundary stops this S with evidence rather than an implementation claim. |
| Delivery | The temporary trace showed a finite port-61 delay and only existing receiver routes after it; no reset value or new port was inferred. The trace was removed after x64/x86 observation. See [S28 evidence](../etc/evidence/t580-s28-post-delay-boundary.md). |

## Open Task — M5 T580

| Field | Required record |
| --- | --- |
| Latest accepted delivery | S25 records a traceable external DSTN panel-derived input. S24 adds the documented TFT EC Block-B static receiver. S20 rejects a non-semantic checkpoint and S21 rejects synthetic reference-emulator shortcuts; none establishes a product release qualification. |
| Evidence | [S5 C000 return](../etc/evidence/t580-s5-c000-return.md), [S6 port-61 sample](../etc/evidence/t580-s6-post-c000-port61-sample.md), [S7 delay return](../etc/evidence/t580-s7-post-c000-delay-return.md), [S8 display receiver](../etc/evidence/t580-s8-pc110-display-receiver.md), [S9 mode-12 capture](../etc/evidence/t580-s9-f65535-mode12-capture.md), [S14 post-C000 observation](../etc/evidence/t580-s14-post-c000-observation.md), [S19 cold image](../etc/evidence/t580-s19-cold-config-image.md), [S20 checkpoint](../etc/evidence/t580-s20-ram-test-checkpoint.md), [S21 model boundary](../etc/evidence/t580-s21-external-model-boundary.md), [S23 EC mailbox](../etc/evidence/t580-s23-ec-mailbox-receiver.md), [S24 EC Block B](../etc/evidence/t580-s24-ec-block-b-receiver.md), [S25 DSTN input](../etc/evidence/t580-s25-dstn-panel-derived-input.md), [S28 post-delay boundary](../etc/evidence/t580-s28-post-delay-boundary.md) and [S29 DMA-page/Boot/IPL](../etc/evidence/t580-s29-dma-page-boot-ipl-latch.md). |
| Remaining convergence | Real POST/setup/guest progression still lacks a demonstrated firmware transition beyond the established delay path and a complete power/MCU or later firmware contract; panel/power, storage/guest and lifecycle UX remain unqualified. A derived DSTN panel candidate now exists, but a factory-original DSTN BIOS/font input and independent DSTN qualification remain absent. |
| Next coordination rule | Admit only a source- or semantic-checkpoint-bounded receiver. Do not use arbitrary instruction counts, ROM-address behavior, QEMU postboot shortcuts, synthetic firmware paths or metadata-only MCU models as a substitute. |

## M5 T580 S25 — Closed: DSTN panel-input provenance

| Field | Required record |
| --- | --- |
| Objective | Establish whether the newly published, live-captured 18-byte stock-DSTN panel table can produce one explicitly derived external firmware candidate from the already admitted TFT image, without claiming a recovered factory ROM or a working PC110 product. |
| Boundary | External `nxvm-assets` candidate, MyPC110 provenance/evidence and narrowly necessary App build-input preparation only. No Shared/Core change, MCU/power/PCIC model, ROM-address behavior, product artifact publication or guest-qualification claim. |
| Evidence | The selected Open-Source-PC110 `Discovery/65535` stock-table record and Multi-Patcher source must agree byte-for-byte; the existing TFT master remains immutable. |
| Verification | Record input/output hashes, exact changed offsets, round-trip table identity and unchanged bytes outside the 18 documented locations. If an App build input is prepared, build and execute only its affected MyPC110 tests on both widths. |
| Limit | A panel-table-derived BIOS candidate is not an original raw DSTN dump, complete hardware model, POST/Setup/DOS proof or publishable executable. |

### S25 Closure

The external asset archive now contains a hash-recorded, explicitly derived
DSTN-panel candidate. It differs from the admitted TFT parent only at the 18
external-source-recorded F65535 table bytes; its provenance and mechanical
comparison are recorded in [the S25 evidence](../etc/evidence/t580-s25-dstn-panel-derived-input.md).
The App remains TFT-only because neither candidate has yet passed the separate
PC110 build, panel and guest qualification gates. No repository source, public
contract or product artifact changed.

## M5 T580 S26 — Closed: fixed panel build pairings

| Field | Required record |
| --- | --- |
| Objective | Replace the single TFT-only preview construction with two fixed MyPC110 build pairings: one uses the admitted TFT BIOS and one uses the S25 DSTN-derived input. |
| Boundary | `app-mypc110`, its unit tests, CMake input declarations and NXVM evidence only. No Shared/Core API, runtime model selector, product artifact deployment, MCU/power/PCIC model, guest claim or fallback path. |
| Design | Each generated firmware object has one selected immutable BIOS. Each product binding links exactly one such object and owns its matching fixed profile identity. The existing common board, font route and adjacent `mypc110.ini` are shared; no executable infers a panel from INI. |
| Verification | Verify each asset hash and both CMake input routes; build the two preview targets and run MyPC110 units on x64/x86. Then run the required repository unit suite before closure. |
| Limit | Building both pairings establishes only build identity. It does not qualify DSTN/TFT panel output, real POST/Setup/DOS, power/MCU/PCIC, lifecycle UX or artifact publication. |

### S26 Delivery

`app-mypc110` now builds independent `tft` and `dstn` preview pairings.  Each
links one generated immutable firmware object, has a fixed machine identity and
does not select a BIOS or panel at runtime.  The App-local firmware map was
renamed from `mypc110_tft_firmware` to `mypc110_firmware`, because the map has
no TFT-specific behavior.  The [S26 evidence](../etc/evidence/t580-s26-fixed-panel-pairings.md)
records both external input hashes and the qualification boundary.

Both established Ninja/ccache widths build the pairings and pass all 19
MyPC110 unit/diagnostic tests.  The repository unit suite also passes 667/667
on each width.  No PC110 release artifact was deployed: panel, POST, Setup,
DOS and lifecycle qualification remain T580 work.

## M5 T580 S27 — Closed: post-delay real-I/O observation

| Field | Required record |
| --- | --- |
| Objective | Observe the first actual I/O operation after the admitted post-C000 port-61 delay return, then select at most one source-backed receiver from that fact. |
| Boundary | App-local diagnostic and evidence only. No production port route, external model import, fixed instruction-count success condition or Shared/Core change. |
| Verification | Run the TFT real BIOS on both established host widths. Capture the first post-return I/O or explicitly retain a no-event result; do not call an instruction budget guest progress. |
| Limit | A trace point alone is diagnostic evidence, not POST, Setup, DOS, panel, MCU/power or release qualification. |

### S27 Delivery

The temporary x64 trace reached `F000:EA84` and ran through the existing
13,000,000-instruction containment limit without a later I/O event.  The
observation selected no receiver, so its temporary diagnostic path was removed.
In particular, it does not justify connecting FDC37C665IR or inventing a
guest-visible EC/ED gate.  The [S27 evidence](../etc/evidence/t580-s27-post-delay-no-receiver.md)
records the bounded result and retained limits.

## M5 T580 S24 — Closed: EC Block-B static receiver

| Field | Required record |
| --- | --- |
| Objective | Supply the documented TFT BIOS's Block-B EC index/data receiver through one App-local static L2 capture. |
| Boundary | MyPC110 source/test and evidence only; no Shared/Core interface, dynamic battery/power, MCU, second clock, firmware-address behavior, raw asset, or artifact publication. |
| Evidence | [S24 EC Block B](../etc/evidence/t580-s24-ec-block-b-receiver.md). |
| Verification | Build and execute the complete MyPC110 label on x64 and x86; exercise representative capture values, documented wrapping, unsupported data writes, and invalid byte-width input. |
| Limit | The static capture is compatibility data, not a physical EC/power model or POST/guest qualification. |

### S24 Closure

MyPC110 now installs the source-recorded Block-B receiver at `35EAh`/`35EBh`
in both its base and planned construction forms. The one-register receiver
accepts an index write and exposes only the captured 32-entry read file with
documented wrapping. It deliberately has no writable data register, dynamic
power state, busy interval, MCU execution or separate time owner. Both widths
pass the complete MyPC110 label: ten App units and eight bounded real-firmware
diagnostics. This does not prove a full firmware transition, POST, Easy-Setup,
panel/power, storage/guest lifecycle or DSTN qualification.

### S9 Brief

Implement the selected L2 F65535 standard-VGA mode-12 copied scanout:
640 by 480 indexed, four-plane graphics delivered through the existing
X86-NXVM Machine frame path.  The receiver owns only the standard VGA state
already retained by F65535.  It must not add a host renderer, front-LCD/panel
model, firmware-address behavior, a new Lib/Emulator contract, or an
alternative presentation route.  Extend existing F65535, Machine-frame and
MyPC110 board-route tests; preserve text capture and failure destination
semantics.  Verify both host widths and record the L2 limit before closure.

### S9 Closure

F65535 now copies only source-qualified standard VGA mode-12 state: the
existing four A0000h planes, 80-character CRTC width, 480 displayed lines,
40-word stride and 64 KiB A0000h map become one existing indexed graphics
snapshot. The existing Machine conversion route publishes that snapshot;
there is no panel renderer, physical panel timing, firmware-address behavior,
or Lib/Emulator contract change. x64 and x86 each pass the complete
X86-NXVM unit set (97 tests) and complete MyPC110 unit set (8 tests). This is
not proof that the admitted real firmware reaches mode 12, completes POST, or
has a qualified LCD/power or guest/storage path.

### S10 Brief

Connect the PC110's source-qualified standard ATA task-file receiver through
the existing X86-NXVM Board and Machine media route. The receiver is limited
to the confirmed 1F0h--1F7h task file and IRQ14 attachment; it reuses the
existing ATA PIO controller, media registry and fixed-disk owner. It must not
introduce a PCMCIA model, a synthetic disk, a new clock, or a guest-boot
claim. The construction path must also bind its media registry when ATA is
the only media consumer. Prove the resulting App board accepts its existing
ATA ports on both host widths and record the L2 limit.

### S10 Closure

MyPC110 now gives its existing Board plan the source-qualified ATA PIO task
file (`1F0h`--`1F7h`, alternate status/control at `3F6h`) and IRQ14, while
reusing the existing fixed-disk media owner. X86-NXVM construction now binds
the existing media registry whenever either a floppy or ATA controller is a
consumer, so an ATA-only profile has no hidden floppy prerequisite. The
MyPC110 construction contract verifies the resulting fixed-disk topology.

Both host widths passed the complete X86-NXVM set (234 tests), complete
MyPC110 set (8 tests), and the two relevant manifest checks. This is an L2
receiver only: it does not establish PC-Card behavior, disk geometry, ATA
mechanical timing, a guest boot, POST completion, or original-DSTN support.

## M5 T580 S14 — Closed: post-C000 observation

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S14. |
| Admission And Approval | The owner directed PC110 work to continue through T580 and grants standing approval for bounded App/Core implementation, verification, commit and push work. |
| Objective | Audit the real-firmware diagnostic's post-C000 observation boundary and determine whether it selects a further real port receiver without a synthetic port or firmware shortcut. |
| Non-goals | No LCD scanout, mailbox/power/PCMCIA model, host panel renderer, firmware-address special case, original-DSTN path, guest boot or release claim. |
| Reference Baseline | S13 commit `a6d17b967`; the selected TFT BIOS returns from `C000:0003` to `F000:3F6C`, but the existing trace discarded every port outside its preselected whitelist. |
| Candidate Proposal | [PC110 qualification](../proposals/m5-pc110-qualification.md), using its source-qualified receiver ledger. |
| Files And ABI Surface | Existing App-local real-firmware diagnostic during investigation, Current and one evidence record; no production or public-interface change. |
| Applicable Rules | NXVM guide; shared Execution, Architecture, Coding, Documentation and source/research policy; diagnostic code may observe existing routes only. |
| Verification | Widen the existing diagnostic only for the investigation, run its bounded x64/x86 observation, then remove an inconclusive probe rather than retaining a failing CTest. |
| Expected Markers | Either a two-width semantic receiver is selected, or the evidence records that no later receiver was observed and the probe is not retained. |
| Asset Needs | Existing external, hash-validated TFT BIOS and Kanji font under `nxvm-assets`; neither is tracked or modified. |
| Reporting Requirements | Record the retained boundary and whether a further receiver is actually selected. |
| Stop Conditions | A source/image mismatch, host divergence, missing asset, any need for synthetic hardware behavior, or any Shared/Core production change. |
| Exit Criteria | The evidence distinguishes an actual selected receiver from an inconclusive observation; no failing probe remains; documentation and actual-diff review are clean. |
| Original Owner Request | Continue the entire PC110 task, using the external PC110 projects and lawful external firmware as L2 evidence where appropriate. |
| Similar-Issue Sweep | All table registers are asserted together; no selective one-register claim, duplicate trace route or synthetic register injection remains. |

### S14 Closure

The temporary all-port observation was run on both widths after the established
`C000:0003` return. Within its bounded run, the TFT BIOS returned to the
existing port-61 wait loop and issued no later port operation. That does not
select a receiver, so the temporary failing CTest was removed rather than
mistakenly treating a retirement ceiling as a hardware requirement. No
production, public-interface, asset or release input changed. The evidence
records why live SCAMP values are not a cold-state basis. The next S must
start from documented C000 option-ROM/F65535 return semantics.

## M5 T580 S15 — Closed: executable C000 shadow

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S15. |
| Admission And Approval | The owner directed PC110 work to continue and grants standing approval for bounded App/Core implementation, verification, commit and push work. |
| Objective | Make the selected PC110 C000 video shadow one executable memory owner: instruction fetch must observe the same writable shadow already observed by data reads and writes. |
| Non-goals | No new memory map, BIOS special case, synthetic ROM content, F65535 panel model, Shared contract, original-DSTN path, guest boot or release claim. |
| Reference Baseline | S14 commit `971020a4a`; the App already makes the C000 shadow writable through the existing EC/ED route, but its instruction callback copies from immutable raw Flash. |
| Candidate Proposal | [PC110 qualification](../proposals/m5-pc110-qualification.md), using the selected TFT firmware map and its source-qualified writable-video-shadow contract. |
| Files And ABI Surface | `src/app-mypc110/pc110_firmware.c`, direct App firmware/config-route tests and Current; no public interface or component dependency change. |
| Applicable Rules | NXVM guide; shared Execution, Architecture, Coding, Documentation and source/research policy; one C000 memory owner for data and instruction access. |
| Verification | Test locked and unlocked C000 writes through the existing EC/ED control route, then execute from the C000 shadow through the ordinary Core instruction path on x64 and x86. |
| Expected Markers | Before unlock, a write cannot alter the selected shadow; after unlock, data read and executed instruction agree; reset restores the selected raw bank. |
| Asset Needs | No new asset. Existing TFT firmware remains external and unmodified. |
| Reporting Requirements | Record the owner mismatch, actual code/test diff, two-width result and the unchanged guest/panel qualification boundary. |
| Stop Conditions | A need for a second memory route, raw firmware mutation, shared-interface change, unproven ROM behavior or a failure outside the C000 shadow contract. |
| Exit Criteria | One App-local shadow owner serves both data and instruction access; direct two-width tests and relevant existing diagnostics pass; review is clean. |
| Original Owner Request | Continue the entire PC110 task, using external PC110 projects and lawful firmware as L2 evidence where appropriate. |
| Similar-Issue Sweep | Check every App-local firmware instruction callback for a data/instruction backing-store split; retain any intentional immutable route only when documented and tested. |

### S15 Closure

The selected C000 video shadow now has one App-local backing-store owner:
instruction fetch delegates to the same existing callback as data reads. The
direct configuration regression proves that a locked write cannot alter the
shadow, an EC/ED-enabled write is subsequently both readable and executable,
and reset restores the raw selected bank. The App-local sweep found no second
instruction callback. Both widths passed all eight MyPC110 unit tests and the
unchanged real-firmware C000-to-port-61 diagnostic. The evidence records the
boundary: this is not POST, panel, power, storage, guest, or DSTN
qualification.

## M5 T580 S16 — Closed: port-61 wait audit

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S16. |
| Admission And Approval | Standing approval for bounded PC110 research and any proven App/Core repair. |
| Objective | Determine whether the admitted real-TFT firmware's reached `F000:3F6C` port-61 wait is a correctly modeled finite delay or exposes one existing PIT/port-61 timing defect. |
| Non-goals | No arbitrary retirement success threshold, synthetic firmware path, guessed SCAMP state, LCD/panel/power/PCMCIA model, guest boot, or original-DSTN claim. |
| Files And ABI Surface | Initially evidence and direct diagnostics only; production scope is admitted only for a reproduced owner-local defect. |
| Verification | Compare the firmware loop's observed port contract with the existing board implementation; use both widths and existing diagnostics; retain a regression only for a concrete semantic boundary. |
| Exit Criteria | Record either a source/semantic-backed owner repair with two-width proof, or a no-repair disposition that preserves the reached boundary without a speculative model. |

### S16 Closure

The reached post-C000 loop is an existing finite firmware delay, not a stuck
receiver: the real TFT diagnostic sees port-61 bit 4 change and the existing
two-width regression observes the subsequent return. The MyPC110 plan's
three-tick Core-axis status source is the only required owner and matches the
Board implementation. No production repair or new diagnostic is justified;
the retained evidence records the values and the no-repair disposition.

## M5 T580 S17 — Closed: rejected post-DD05 checkpoint

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S17. |
| Admission And Approval | The owner directed PC110 work to continue and granted standing approval for bounded App/Core implementation, verification, commit and push work. |
| Objective | Verify the proposed `F000:DD05` post-reset checkpoint and, only if reached on both widths, identify its first subsequent real I/O receiver without retaining a budget-only success probe. |
| Non-goals | No guessed SCAMP cold image, synthetic firmware path, panel/power/PCMCIA model, host renderer, second executor, guest boot claim or artifact publication. |
| Reference Baseline | S16 commit `95dca6598`: real TFT firmware completes the C000-to-port-61 return and establishes a finite delay boundary on both widths. |
| Candidate Proposal | [PC110 qualification](../proposals/m5-pc110-qualification.md), consuming one source- or semantic-checkpoint-bounded receiver only. |
| Files And ABI Surface | Initially the existing App-local real-firmware diagnostic, Current and one evidence record; production/App changes are admitted only after a reproduced owner-local defect or receiver. |
| Applicable Rules | NXVM guide; shared Execution, Architecture, Coding, Documentation and source/research policy; diagnostic code observes existing routes only and Core remains the sole execution/tick owner. |
| Verification | Start a bounded all-port capture only after the real `F000:DD05` checkpoint; run both host widths and retain a test only if its success condition is a matching semantic event rather than retirement count. |
| Expected Markers | One matching I/O direction, port, value and execution point on x64/x86, or a documented no-receiver disposition with the temporary probe removed. |
| Asset Needs | Existing hash-validated external TFT BIOS/font only; no raw input is copied or modified. |
| Reporting Requirements | Record the selected receiver's provenance/owner or why no receiver can be selected; distinguish research evidence from product qualification. |
| Stop Conditions | Host divergence, fault, missing asset, a need for synthetic device behavior or Shared/Core contract expansion without a reproduced defect. |
| Exit Criteria | The capture's conclusion is two-width and semantic; no speculative production patch or failing/budget-only test remains; documentation and actual-diff review are clean. |
| Original Owner Request | Continue the complete PC110 task while using external PC110 research only as L2 evidence where appropriate. |
| Similar-Issue Sweep | Inspect the same post-checkpoint capture for every previously unmodeled port class before declaring one receiver unique. |

### S17 Closure

The temporary all-port capture was armed only after a proposed `F000:DD05`
checkpoint.  On both x64 and x86, eight million containment instructions did
not reach that point and ended in the known firmware delay region, without a
Core fault.  It therefore selected no receiver and was removed rather than
retained as a budget-success test.  No production, public-interface, asset or
artifact input changed.  See [the S17 evidence](../etc/evidence/t580-s17-post-dd05-port-observation.md).

## M5 T580 S18 — Closed: post-C000 first-I/O disposition

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S18. |
| Objective | Determine whether the first post-`F000:EA86..EA92` I/O selects an unmodeled receiver. |
| Non-goals | No panel/power/PCMCIA model, shared/Core interface change, raw-firmware commit or guest-boot claim. |
| Reference Baseline | S17 commit `c61584331`; S16 proves the finite port-61 loop returns on both host widths. |
| Files And ABI Surface | Current and one evidence record only. |
| Evidence Rule | The prior return boundary plus the verified BIOS bytes determine the immediate I/O; do not retain a duplicate diagnostic. |
| Verification | Confirm the returned code sequence uses only the existing port-61 receiver. |
| Exit Criteria | Record the disposition; no redundant probe remains. |

### S18 Closure

The first I/O after the established `F000:EA86..EA92` wait return is the
existing port-61 receiver (`F000:EA76` read, followed by `F000:EA7C` write).
It cannot select a new PC110 receiver, so no duplicate diagnostic is retained.
The source-qualified cold configuration image is recorded separately in the
[S19 evidence](../etc/evidence/t580-s19-cold-config-image.md).

## M5 T580 S19 — Closed: L2 chipset cold image

| Field | Required record |
| --- | --- |
| Objective | Supply the current 20 MiB TFT composition with an App-owned, source-qualified VL82C420 L2 cold image through the existing chip API. |
| Boundary | MyPC110 source/test and evidence only; no Shared/Core public change, no new hardware receiver, raw asset, or artifact publication. |
| Verification | Build both widths; assert representative SCAMP, block2 and EC/ED values through the actual routes and retain them across machine reset. |
| Limit | This is a compatibility baseline from external live captures, not a physical power-on/timing proof or POST/guest qualification. |

### S19 Closure

MyPC110 now initializes the existing VL82C420 through one App-local L2 cold
image, before either base composition path constructs the machine.  The image
does not add a Core contract or another route; warm reset continues to retain
the banks.  Both widths pass all eight MyPC110 units and reach identical
bounded real-firmware checkpoints without a fault.  The external captures
and the resulting qualification limit are recorded in the
[S19 evidence](../etc/evidence/t580-s19-cold-config-image.md).  This fixes
the invalid zero-filled baseline only; it does not claim POST completion,
panel/power behavior, storage boot or original-DSTN support.

## M5 T580 S20 — Closed: rejected RAM-test return checkpoint

| Field | Required record |
| --- | --- |
| Objective | Determine whether the observed `F000:6060..6068` RAM-test helper can select a later actual I/O receiver. |
| Boundary | Temporary existing MyPC110 diagnostic only; no production route, public interface, firmware or artifact input. |
| Evidence | [S20 RAM-test checkpoint](../etc/evidence/t580-s20-ram-test-checkpoint.md). |
| Exit | The temporary capture was removed because neither host width established the proposed semantic transition. |

### S20 Closure

The earlier `F000:6064` execute-watch observation was a stopped-at-entry
diagnostic boundary, not proof that the BIOS completed the surrounding helper.
The temporary two-width capture required retirement of `F000:6068`, departure
from that helper, and one subsequent port operation. Neither width met that
condition before returning to the already-qualified finite port-61 delay
loop. It therefore selects no receiver and leaves no failing diagnostic,
production change or PC110 release claim.

## M5 T580 S21 — Closed: external MCU and POST model boundary

| Field | Required record |
| --- | --- |
| Objective | Determine whether the available PC110 reference projects and external MCU images select a lawful, CPU-visible production receiver. |
| Boundary | Read-only source/asset audit and evidence only; no reference source import, ROM commit, address intercept, synthetic interrupt, or PC110 release. |
| Evidence | [S21 external-model boundary](../etc/evidence/t580-s21-external-model-boundary.md). |
| Exit | The reviewed references provide research leads but no implementation-grade machine contract; their synthetic paths are explicitly rejected. |

### S21 Closure

The external power and keyboard MCU images are available as BYOB inputs, but
the reviewed reference implementations do not execute either MCU through a
CPU-visible PC110 board contract. Their POST helpers complete loops at named
ROM addresses, synthesize disk/interrupt state, or expose firmware metadata.
Those mechanisms are useful research evidence only and cannot enter MyPC110.
The remaining qualification gates therefore remain explicit: a trace or
source-defined CPU-visible power/MCU contract, and an independently observable
real-firmware post-POST checkpoint. No production path, asset mapping or
release claim changed.

## M5 T580 S23 — Closed: EC mailbox receiver

| Field | Required record |
| --- | --- |
| Objective | Install the externally documented PC110 EC mailbox through the ordinary App/Core port route. |
| Boundary | App-local source, unit test and evidence only; no Shared/Core API, firmware-address behavior, MCU execution, APM emulation, clock, asset or release artifact. |
| Evidence | [S23 EC mailbox](../etc/evidence/t580-s23-ec-mailbox-receiver.md). |
| Exit | The documented `15E8h`/`15ECh` ready-and-write contract is tested on both host widths without claiming POST or power qualification. |

### S23 Closure

MyPC110 now owns one EC mailbox receiver, independent of the firmware map and
the standard board controllers. `15ECh` returns the documented ready state;
`15ECh` and `15E8h` accept the documented command/data byte sequence. The
receiver neither invents MCU behavior nor consumes a second clock. Both host
widths build the altered App board and pass the direct mailbox, base-composition
and construction regressions. This is a source-backed compatibility receiver,
not proof that the selected BIOS reaches it or completes POST.

### S11 Closure

The diagnostic probe recorded no `3D6h/3D7h` extension writes on either host
before the already-proved post-C000 port-61 return: every one of the 16
source-listed TFT table registers remained unseen. The identical result at
the same semantic checkpoint rules out treating the published TFT table as a
currently executed panel receiver. The temporary tracer was removed; no
production or test path changes remain from this result.

The next panel/power receiver must be selected from a later, source-defined
firmware event rather than extending a retirement budget or injecting the
table. The S10 x64/x86 16-test real-firmware regression remains the retained
proof for the reached path.

## M5 T580 S12 — Closed: post-C000 event source gate

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S12. |
| Admission And Approval | The owner directed PC110 work to continue through T580 and grants standing approval for bounded App/Core implementation, verification, commit and push work. |
| Objective | Identify the first real port operation after the established C000-to-port-61 return, so the next PC110 receiver has one observed owner rather than a guessed device. |
| Non-goals | No production device behavior, LCD/power/PCMCIA model, firmware injection, second executor, raw instruction-count success condition, guest boot or artifact claim. |
| Reference Baseline | S11 commit `97295cb2c`: both widths return from C000 to `F000:EA84`, while the F65535 panel table is still unexecuted. |
| Candidate Proposal | [PC110 qualification](../proposals/m5-pc110-qualification.md), consuming the next source-gated panel/power/storage/lifecycle receiver disposition. |
| Files And ABI Surface | `test/app-mypc110/diagnostic/real_firmware_post_diagnostic.c`, Current and one evidence record; no production or public-interface change. |
| Applicable Rules | NXVM guide; shared Execution, Architecture, Coding, Documentation and source/research policy; Core remains the sole execution/tick owner. |
| Verification | Rebuild the existing diagnostic and run the new semantic checkpoint on x64/x86, then run the complete MyPC110 x64/x86 diagnostic set if it reaches a source-defined event. |
| Expected Markers | One identical first post-return trace event (direction, port, value and instruction point) on both host widths; the fixed containment ceiling is never success. |
| Asset Needs | Existing external hash-validated TFT BIOS/font only. |
| Reporting Requirements | Record the exact two-width event and its candidate owner, or record why no event is reached; retain no speculative production patch. |
| Stop Conditions | Host divergence, fault, missing asset, a need for a second clock or synthetic firmware behavior, or an undetermined event class outside current App evidence. |
| Exit Criteria | Two widths establish one matching first event or a bounded source gate, all affected diagnostic tests pass, and actual-change review identifies no production path. |
| Original Owner Request | Continue the entire PC110 task using external PC110 projects and lawful external firmware as L2 research where suitable. |
| Similar-Issue Sweep | The tracer records any port direction, not a preselected device family; it has one post-return owner and no duplicate execution path. |

### S12 Closure

Both hosts left the same `F000:EA84` wait return and executed to the fixed
containment ceiling without one additional port operation. They instead
remained in identical ROM code at `F000:3F6C`, with no fault. The temporary
event tracer and its CTest were removed: an instruction ceiling cannot select
a new receiver, and no unowned port or panel/power operation was observed.

T580 therefore retains the reached firmware boundary but cannot honestly
advance panel, power, PCMCIA, guest or lifecycle qualification from this path.
The result is transferred to the qualification ledger rather than hidden as a
failed test or treated as evidence for a speculative device model.

## M5 T580 S13 — Closed: original-DSTN asset reconciliation

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S13. |
| Objective | Reconcile every available external PC110 firmware candidate against the required original-DSTN asset gate, so later work cannot mistake a modified or TFT image for the original model. |
| Non-goals | No byte derivation, reverse patch, firmware import, runtime loader, product build, hardware model or source-code change. |
| Evidence boundary | Compare only external hashes and the repositories' own source documentation; preserve every raw input outside this repository. |
| Exit Criteria | Record the identity and suitability of each discovered candidate, update the qualification ledger, and leave the original-DSTN receiver explicitly blocked unless a verified raw input exists. |

### S13 Closure

The three admitted research repositories provide no verified original-DSTN
firmware input. PC110-EMU's sole full BIOS matches the already admitted TFT
hash. Open-Source-PC110's only other full image is an LBA-modified BIOS whose
own README says it has not been run; its TFT documentation explicitly says a
confirmed unpatched DSTN dump is still required for a reverse patch. No image
was copied, derived or embedded. The four-artifact release target therefore
remains blocked by an external BYOB input rather than being silently narrowed
or filled with a guessed firmware image.

### S5 Closure

S5 is accepted as a finite return classification. Both host widths leave the
selected C&T bank at `F000:3F6C` and then reach the already-admitted port-61
wait without a Core fault or a new device access class. Its retained evidence
opens no hardware receiver. S6 therefore owns only the next finite status
sample; it may not use an arbitrary longer retirement budget as a
success marker.

### S5 Delivery

The two-width trace leaves C000 at the same `F000:3F6C` return point and then
reaches the already-known port-61 wait without a new fault or access class.
The finite result is retained in [the S5 evidence](../etc/evidence/t580-s5-c000-return.md).
It opens no new hardware receiver.

### S4 Closure

The C000 owner now distinguishes ordinary writable shadow data from immutable
selected-video-bank instruction fetches.  The F65535 compatibility owner
supplies only the register behavior reached by that path; the trace also
corrected generic Core lane masking and narrow-port latch width behavior.  The
x64 and x86 real-image CTests each cross the actual former C000 port boundary
after about 10.17 million ordinary instructions, with no Core fault.  This
bounded result is documented in [the S4
evidence](../etc/evidence/t580-s4-c000-video-progress.md); it is not a POST,
display, Setup or boot qualification.

The stable x64 and x86 Ninja trees each pass the complete repository-only
unit suite (663 selected tests).  The corresponding external-firmware C000
diagnostic passes again on both widths.  No executable publication is part of
this bounded diagnostic S.

### S3 Closure

The extended two-width real-TFT probe establishes that the previous port-61
cycle is not the current stop boundary: after 100 million ordinary
instructions it is in the real C000 video ROM, with no Core fault and over
one million C000 retirements.  The current F65535 path returns zero for every
input-status read.  The retained PC110-QEMU research documents this same BIOS
far-call to `C000:0003` and models the VGA `3DAh` vertical-retrace bit as a
changing status bit.  That is a finite X86-NXVM F65535 L2 receiver, not a
reason to copy its firmware-address intercepts.  The detailed boundary and
limits are in [`T580 S3 evidence`](../etc/evidence/t580-s3-f65535-status-gate.md).

### S2 Closure

MyPC110 now has one TFT-only App entry and immutable binding through the
existing X86-NXVM Product/Machine route.  Its adjacent `mypc110.ini` owns only
the shared runtime grammar; external TFT firmware/font selection remains a
build input.  The preview target is compile-only and was exercised through the
real Product factory on both widths.  It does not deploy an EXE, add a DSTN
path or claim firmware boot.  The exact surface and test set are retained in
[`T580 S2 evidence`](../etc/evidence/t580-s2-tft-product-preview.md).

### S1 Closure

The qualification universe is now frozen in the
[`T580 S1 ledger`](../etc/evidence/t580-s1-pc110-qualification-ledger.md).
The current App is a TFT-only board/diagnostic library: it has no process
entry, product binding, INI, profile selection or artifact route.  The
external TFT ROM and font are admitted BYOB inputs; the only PC110-EMU ROM has
the same TFT hash.  No verified original DSTN ROM or reversible byte patch is
available, so the original-DSTN artifacts cannot be honestly built yet.

S2 will add only the TFT product composition/build boundary and prove its
normal construction/lifecycle path without claiming that the real firmware
boots.  The later DSTN receiver will reuse that one route after a separately
admitted lawful, verified input arrives.

## Closed Task — M5 T579

| Field | Required record |
| --- | --- |
| Identifier Mode | T579 closed after committed S1--S3 deliveries; one session performed coordinator and executor roles. |
| Admission And Approval | Owner directed PC110 implementation to continue and grants standing approval for ordinary scoped diagnostic, implementation, verification, commit and push work. |
| Objective | Establish and replay the smallest source-labelled PC110 port-61/LPT progression boundary in the real TFT BIOS path, without fabricating a complete machine. |
| Non-goals | No synthetic BIOS/IVT/BDA path, firmware-address special case, borrowed PC/AT ROM, second clock, new Shared/Core API, raw asset commit, POST/display/Setup/DOS boot or release claim. |
| Reference Baseline | T579 S2: real BIOS reaches `F000:55B2` after its bounded 3BCh--3BEh LPT probe. |
| Candidate Proposal | [PC110 port-61 L2 progress](../history/M5-T579-pc110-port61-l2-progress.md). |
| Files And ABI Surface | MyPC110 board-plan/LPT model, bounded diagnostics, evidence and task records only. Shared/Core and raw assets remain excluded. |
| Applicable Rules | NXVM guide; shared Execution, Architecture, Coding and Documentation rules; NXVM source policy. One executor and Core's sole tick axis remain mandatory. |
| Verification | Dual-width App unit tests plus a bounded real-firmware post-reset progression diagnostic that proves retirement leaves the EA86--EA90 wait loop; documentation/whitespace gates. |
| Expected Markers | One L2 source/period record, ordinary port-61 transitions on Core's tick axis and a post-reset retirement beyond the wait loop. |
| Asset Needs | Read-only admitted TFT BIOS/font inputs only. |
| Stop Conditions | Need for a second clock, synthetic firmware behavior, an unproven Shared/Core repair, failure to leave the loop, or a claim beyond the verified checkpoint. |
| Exit Criteria | Met: the app-local L2 source is explicit, two-width diagnostics prove the port-61/LPT boundary and its later recurrence, and all boot/display/product claims remain deferred. |
| Similar-Issue Sweep | Port 61 read/write semantics, PIT wiring, A20/reset state, parity-memory exclusion, guest wait loops and board timing-source selection. |

### S1 Brief

Replace only MyPC110's unprogrammed PIT1 status selection with the existing
elapsed-tick status source.  Choose and document its finite L2 period from
actual real-image execution; do not claim physical timing accuracy.

### S1 Closure

MyPC110 now selects the existing three-tick elapsed-status source.  The real
TFT image enters and leaves the actual `F000:EA86--EA90` loop on both widths,
reaching `F000:DD05` with no delivered exception.  This is an App-local L2
gate only; its exact result and source limits are retained in
[`T579 S1 evidence`](../etc/evidence/t579-s1-pc110-port61-l2-progress.md).
The six App unit tests pass on both widths; the bounded external-firmware
diagnostic also passes on both widths.  No PC110 executable, POST, display,
Setup or guest-boot qualification follows from this checkpoint.

### S2 Brief

Starting from the proved post-reset progress point, extend only the existing
real-firmware diagnostic far enough to identify the next finite execution
boundary and its owner.  Do not preselect a video, storage, power or firmware
repair, and do not change product behavior until evidence isolates one.

### S2 Closure

The next owner is an App-local legacy LPT probe at ports `3BCh--3BEh`, not a
shared video or storage defect.  MyPC110 now supplies the bounded L2 control
and status behavior demonstrated by the actual BIOS path.  The real image
passes from the port-61 loop through `F000:55B2` on x64 and x86; the App unit
suite passes on both widths.  The retained evidence records the source grade,
finite output values and next address.  This still makes no POST, display,
Setup, guest-boot or artifact claim.

### S3 Brief

Starting after the now-proven LPT probe, extend the existing bounded firmware
diagnostic only until it reaches the next distinct recurring wait or concrete
unowned device boundary.  Reuse the sole executor and existing App routes;
do not guess a device model from an address alone.

### S3 Closure

The post-LPT path performs ordinary PC110 RTC/CMOS accesses through `70h/71h`
and the platform's `4Fh` index mirror, then returns to the already-admitted
port-61 wait at `F000:EA86--EA90`.  The existing board RTC route owns the
conventional pair; the observed `4Fh` write is not itself a wait or a proven
missing execution dependency.  A new bounded two-width diagnostic proves that
the real image reaches the LPT checkpoint and later re-enters the same
port-61 loop.  No new finite unowned boundary is therefore established, so
this task stops rather than guessing a chipset model.  The source grade,
trace result and deferred PC110 work are retained in
[`T579 S3 evidence`](../etc/evidence/t579-s3-lpt-cycle-source-gate.md).

### Task Closure

T579 closes as a bounded real-firmware progress/source-gate delivery, not as a
PC110 qualification.  S1 (`fbf63faf8`) replaced only the unprogrammed PIT1
assumption with an explicit three-tick L2 port-61 source.  S2 (`1492b8bb1`)
modelled the finite BIOS LPT probe.  S3 (`9c1614354`) proves the subsequent
RTC/CMOS path returns to the already-admitted wait rather than exposing a new
owned blocker.  The resulting MyPC110 test set passes on x64 and x86.  Product
and guest qualification remain in the queued PC110 qualification receiver;
this closure makes no runnable-artifact, POST, display, Setup or boot claim.

## Closed Task — M5 T578

| Field | Required record |
| --- | --- |
| Identifier Mode | New T578 S1 after closed T577; one session performs coordinator and executor roles. |
| Admission And Approval | Owner directed PC110 implementation to continue and grants standing approval for ordinary scoped diagnostic, implementation, verification, commit and push work. |
| Objective | Identify the earliest single source-qualified video or board initialization contract after the corrected real TFT BIOS reset, without presuming an old INT 10h checkpoint is reachable. |
| Non-goals | No synthetic IVT/BDA/BIOS path, guest-address special case, borrowed PC/AT video ROM, host fallback, second executor, raw asset commit, POST/display/Setup/DOS boot or release claim. |
| Reference Baseline | Closed T577 S1 `2340a68b6`: real 8042 reset classification, asserted A20 and bounded absent-memory route. |
| Candidate Proposal | [Video and board source gate](../proposals/m5-pc110-video-board-source-gate.md). |
| Files And ABI Surface | Initially MyPC110 diagnostic/evidence/current/proposal only. App wiring is eligible only after one source-proven owner; Core/Shared are excluded until a trace proves a generic contract. |
| Applicable Rules | NXVM guide; shared Execution, Architecture, Coding and Documentation rules; NXVM source policy. One executor and normal interrupt path remain mandatory. |
| Verification | Bounded dual-width real-firmware diagnostics, an explicit x64 reset trace, affected MyPC110 tests, and documentation/whitespace gates. |
| Expected Markers | Ordered post-reset initialization observations and either one owner disposition or a finite source gate. |
| Asset Needs | Read-only admitted TFT BIOS/font inputs only. |
| Stop Conditions | Ambiguous trace, missing lawful device source, broad compatibility behavior, synthetic firmware behavior or a Shared/Core scope not proven generic. |
| Exit Criteria | Met: the two-width standard diagnostic and explicit x64 reset trace record the first post-reset boundary; no unsupported owner repair or product claim was made. |
| Similar-Issue Sweep | Video config, C000/E000/F000 routes, EC/ED activity, F65535 routes, vector writes, reset restoration and normal interrupt delivery. |

### S1 Brief

Freeze the existing real-firmware diagnostic's first post-reset boundary.  Do
not preselect a video device model, a port-61 timing source, or an INT 10h
result.

### S1 Delivery

Closed as a finite source gate.  The real image makes its ordinary 8042 reset,
then remains in a port-61 bit-4 edge-wait loop before C000/E000/video ROM or
the formerly assumed INT 10h checkpoint.  Available research does not prove
the PC110 source or period for that bit.  The detailed two-width disposition,
explicit x64 trace, and limits are in
[`T578 S1 evidence`](../etc/evidence/t578-s1-pc110-video-board-source-gate.md).
No production code or raw asset was changed.  The next lawful receiver must
admit a clearly-labelled L2 port-61 timing decision before it can claim to
advance real firmware.

## Closed Task — M5 T577

| Field | Required record |
| --- | --- |
| Identifier Mode | T577 S1 after closed T576; one session performs coordinator and executor roles. |
| Admission And Approval | Owner requested continued PC110 implementation through closure and grants standing approval for ordinary scoped diagnostic, implementation, verification, commit and push work. |
| Objective | Capture the final real-firmware retirement window before the first shutdown/reset after the admitted data reload, then identify its smallest valid owner or source-gated disposition. |
| Non-goals | No CPU relaxation, firmware-address special case, BIOS/IVT/BDA injection, borrowed PC/AT ROM, broad compatibility mode, second executor, raw asset commit or boot claim. |
| Reference Baseline | Closed T576 S4 `732486b34`: corrected C000-shadow/E000-bank map, dual-width source gate and admitted external TFT BYOB image. |
| Candidate Proposal | [Protected-reset source gate](../proposals/m5-pc110-protected-reset-source-gate.md). |
| Files And ABI Surface | Initially MyPC110 diagnostic/tests/evidence/current/proposal only. Core is eligible only after the trace isolates one complete generic CPU contract; Shared and raw assets are excluded. |
| Applicable Rules | NXVM guide; shared Execution, Architecture, Coding and Documentation rules; NXVM source policy. One executor and one diagnostic route remain mandatory. |
| Verification | Bounded dual-width reset trace, affected MyPC110 tests, full repository-only x64/x86 unit suites and documentation/whitespace gates. |
| Expected Markers | One ordered final retirement window, the actual shutdown/reset fact and an owner disposition; no fabricated advancement. |
| Asset Needs | Read-only admitted TFT BIOS/font assets only. |
| Reporting Requirements | Record exact two-width facts, source grade, actual changed surface, owner disposition and all unqualified endpoints. |
| Stop Conditions | Ambiguous trace, needed broad compatibility behavior, missing source for an App device, shared scope beyond a proven generic contract or any artificial guest continuation. |
| Exit Criteria | Met: the first reset is classified on both widths, scoped verification passes and no product/boot claim is made. |
| Original Owner Request | Continue PC110 implementation to closure using the external PC110 projects as L2 research and external usable firmware. |
| Similar-Issue Sweep | Protected far call/return, IRET, segment reload, stack transition, exception delivery, reset delivery, map routes and existing fault observation without adding a parallel execution path. |

### S1 Brief

The already-corrected map is not the immediate execution blocker. Extend only
the existing MyPC110 diagnostic so it preserves the last bounded retirement
window before the ordinary shutdown/reset. Do not select an implementation
owner until that window proves one.

### S1 Delivery

The bounded diagnostic disproved the assumed CPU shutdown: the actual reset is
the firmware's ordinary `FEh` command to the existing 8042.  Its trace exposed
two App-owned omissions, now repaired without changing Core or any shared
component: reset begins with the 8042 A20 output asserted, and the exact
64-KiB firmware memory-limit probe above fixed RAM is an L2 open-bus route.
The retained two-width proof and its limits are in
[`T577 S1 evidence`](../etc/evidence/t577-s1-pc110-protected-reset-source-gate.md).

T577 is closed at this corrected reset boundary.  It does not qualify PC110
POST, display, Setup, DOS boot, or a published PC110 product.

## Closed Task — M5 T576

| Task | Status |
| --- | --- |
| T576 | Closed at the corrected, source-qualified PC110 video-initialization gate. No synthetic BIOS/IVT path or PC110 product claim was made. |

## M5 T576 S4 — Closed: MyPC110 video-bank map correction

| Field | Required record |
| --- | --- |
| Identifier Mode | Corrective S4 of the immediately preceding T576 closure after stronger already-admitted source evidence contradicted S3's C000 bank conclusion. |
| Admission And Approval | Owner directed PC110 implementation through closure and has standing-approved ordinary App-local implementation, diagnostics and verification. This corrective scope is limited to the source-proven map; no new shared/Core authority is used. |
| Objective | Restore the proven E000 bank-2 / F000 bank-3 / C000-shadow topology and trace the next ordinary real-image boundary. |
| Non-goals | No IVT/BDA/BIOS injection, QEMU behavior copy, PC/AT ROM, loose CPU mode, new public API or product claim. |
| Reference Baseline | S3 commit `76a09778b`, admitted TFT BYOB image and the already-reviewed Open-Source-PC110 live-map record. |
| Candidate Proposal | [S4 map correction](../proposals/m5-pc110-video-bank-map-correction.md). |
| Files And ABI Surface | MyPC110 provider, its App-owned tests/diagnostic, evidence/current/proposal only. Shared/Core and raw assets are excluded. |
| Applicable Rules | NXVM guide; shared Execution, Architecture, Coding and Documentation rules; App source policy. The firmware image remains external BYOB input. |
| Verification | Two-width map assertions, bounded real-image diagnostic, complete repository-only x64/x86 units and documentation/whitespace gates. |
| Expected Markers | C000 and E000 show the bank-2 option-ROM header; F000 shows bank 3; diagnostic reports no C/E execution before its unchanged INT 10 source gate. |
| Asset Needs | Read-only admitted TFT BIOS/font assets only; no raw asset commit or PC110 executable publication. |
| Reporting Requirements | Record actual map, two-width checkpoints, retained source gate, changed files and all unqualified endpoints. |
| Stop Conditions | Need for a synthetic vector/BIOS, borrowed PC/AT video path, broad CPU relaxation, a shared/Core change or a boot claim. |
| Exit Criteria | The contradicted C alias is removed, one correct map owner is tested, the next trace fact is recorded, and no boot/display claim is made. |
| Original Owner Request | Continue the PC110 programme to closure using PC110-EMU, PC110-QEMU and Open-Source-PC110 only as research, with external usable firmware. |
| Similar-Issue Sweep | C000/D000/E000/F000/reset map, EC/ED route state, font aperture precedence and the real image's C/E retirement before the terminal interrupt. |

### S4 Brief

Open-Source-PC110's live source record places bank 2 at E000 and identifies
C000 as its runtime shadow destination. Correct the App map before pursuing
any further CPU or video theory.

### S4 Delivery

The provider now loads the C000 reset shadow from bank 2 while immutable flash
continues to expose that bank at E000 and bank 3 at F000. The dual-width map
tests prove the topology. The real-image checkpoint still reaches `F000:4CA9`
with an all-zero INT 10 vector before executing either C000 or E000. This
closes the mapping correction and records the source gate without a fabricated
video initialization path or a PC110 boot claim.

## M5 T576 S3 — Superseded by S4 map correction

| Field | Required record |
| --- | --- |
| Identifier Mode | S3 follows committed T576 S2; one session performs coordinator and executor roles. |
| Admission And Approval | The owner directed PC110 implementation through closure and accepts bounded L2 external behavior where L3 evidence is unavailable. Ordinary diagnostic, implementation and verification work is standing-approved. |
| Objective | Identify the first causal protected control/stack boundary after the resolved `F000:605F` data reload and before the real firmware's later invalid INT 10 dispatch. |
| Non-goals | No broad loose mode, selector/address exceptions, CS/SS/far/interrupt relaxation without a traced generic contract, BIOS/IVT/BDA interception, PC/AT fallback, second executor, raw-ROM commit, boot claim or product artifact. |
| Reference Baseline | T575/S1/T576 S2 evidence and the [retained control-transfer proposal](../history/M5-T576-pc110-control-transfer-source-gate-proposal.md); Open-Source-PC110, PC110-EMU and PC110-QEMU are L2 research only. |
| Files And ABI Surface | Initially MyPC110 diagnostic/evidence/current/proposal only. X86 Core is eligible only after a trace isolates one generic CPU contract. Public/shared layers and unrelated Apps are excluded. |
| Applicable Rules | NXVM guide; Execution, Architecture, Coding, Documentation and source-policy authorities; one executor/segment owner; BYOB assets and no third-party source copying. |
| Verification | Two-width ordered retirement/fault observations through the first causal boundary, then scoped tests only if a valid owner repair exists. |
| Stop Conditions | Ambiguous trace, source-unproved hardware behavior, need for a broad compatibility scaffold, firmware-address special case, or changes beyond one isolated owner. Record and transfer rather than fabricate. |
| Exit Criteria | One finite causally ordered boundary and owner disposition are recorded on both widths; all scoped tests/evidence pass; no false boot or release claim. |
| Similar-Issue Sweep | Examine far call/jump/return, IRET, stack segment reload, exception delivery, reset boundaries, IVT reads and C-shadow route state without adding parallel execution. |

### S3 Brief

Extend the existing real-firmware diagnostic only enough to preserve a bounded
ordered control/fault window after `F000:6060`.  Do not preselect the QEMU or
PC110-EMU broad compatibility design; decide ownership only from the trace.

### S3 Delivery

S3's initial C000 bank-2 conclusion was superseded before a PC110 product
claim: the stronger live-map record identifies E000 as bank 2 and C000 as the
shadow destination. S4 corrects that App route and repeats the bounded trace.

## M5 T576 S2 — Closed: MyPC110 protected unreal-data reload

| Field | Required record |
| --- | --- |
| Identifier Mode | S2 follows committed T576 S1; one session performs coordinator and executor roles. |
| Admission And Approval | The owner directed the PC110 T through closure, accepts labelled L2 external emulator/research behavior where no L3 manual contract exists, and gives standing approval for ordinary implementation and verification. |
| Objective | Admit the smallest strict-by-default CPU L2 policy needed by the real TFT firmware's next protected-mode data reload at `F000:605F`, then determine its next finite boundary through the ordinary executor. |
| Non-goals | No code/stack/far-transfer relaxation, hardware-specific selector base, firmware-address dispatch, BIOS/interrupt interception, PC/AT fallback, second executor, raw-ROM commit, guest boot claim or PC110 release artifact. |
| Reference Baseline | T575 protected-cache evidence; T576 S1 C-shadow evidence; [unreal-data proposal](../proposals/m5-pc110-protected-unreal-data.md); external Open-Source-PC110, PC110-EMU and PC110-QEMU as L2 research only. |
| Files And ABI Surface | X86 Core CPU policy/config/tests, MyPC110 configuration/diagnostic/tests and current/evidence/proposal records only. Lib, Emulator, X86-NXVM and unrelated Apps are excluded. |
| Applicable Rules | NXVM guide; Execution, Architecture, Coding, Documentation and source-policy authorities; strict default and one CPU descriptor owner; external protected bytes stay BYOB; no third-party source copying. |
| Verification | Dual-width Core policy tests and real-TFT diagnostic must advance beyond `F000:605F`, plus affected MyPC110 and structural gates. |
| Expected Markers | Strict rejects zero descriptors; the opt-in policy first retains valid matching visible cache then uses real-address data attributes only for an all-zero descriptor; no PC110-named CPU branch. |
| Asset Needs | Read-only admitted TFT firmware/font and manifest. |
| Reporting Requirements | Record exact pre/post data segment state, source grade, policy boundary, dual-width facts and all unqualified endpoints. Do not call advancement a boot or product qualification. |
| Temporary Output | `build/t576-s2/` only for bounded diagnostics; remove it after compact evidence is recorded. |
| Stop Conditions | Any need to relax CS/SS/far transfer, introduce hardware selector special cases, fabricate device behavior, or leave the stated CPU/App boundary. |
| Exit Criteria | The data-reload boundary is verified or rejected on both widths, the strict default is preserved, all scoped gates pass, and the next receiver is explicit. |
| Similar-Issue Sweep | All data segment reload paths, invalid descriptor handling, existing policy consumers, Core configuration validity, snapshot/diagnostic observations and MyPC110 policy selection. |

### S2 Brief

Implement only the declared data-reload policy.  Use existing segment-loader
and real-address attribute routines; do not duplicate descriptor preparation.
Update the real-firmware diagnostic only to move its finite checkpoint past the
now-resolved instruction and report the next actual boundary.

### S2 Delivery

X86 Core now has one explicit L2 policy that preserves the T575 visible-cache
receiver and otherwise uses the existing real-address initializer for an
all-zero protected data descriptor.  MyPC110 is the sole consumer.  Both
widths execute the real BIOS through `F000:605F` and stop at `F000:6060` with
`DS=0008`, base `00000080`; strict and cache-only behavior remain covered.

A bounded no-watchpoint investigation proceeds past that data reload but later
re-enters the existing uninitialized `INT 10h` vector path without executing
C000 or programming EC/ED.  The next legitimate work would require a wider
code/stack/far-control L2 model, which is expressly outside S2.  It is
transferred rather than hidden behind a broad compatibility mode.  The exact
proof and limits are in [the S2 evidence](../etc/evidence/t576-s2-pc110-protected-unreal-data.md).

## M5 T576 S1 — Closed: MyPC110 C-segment video-ROM shadow

| Field | Required record |
| --- | --- |
| Identifier Mode | Successor to closed T575; one session performs coordinator and executor roles. |
| Admission And Approval | The owner directed continued PC110 implementation through closure and gave standing approval for ordinary PC110 implementation, verification and task sequencing. The owner also accepts labelled L2 external emulator/research behavior when no L3 manual contract is available. |
| Objective | Implement the smallest App-owned C0000 video shadow using the observed TFT EC/ED transition, then prove its route state and reset behavior on both widths. |
| Non-goals | No Core chipset special case, firmware-address dispatch, BIOS/interrupt interception, PC/AT fallback, BDA/IVT mutation, second executor, raw-ROM commit, guest boot claim or PC110 release artifact. |
| Reference Baseline | Closed T575 evidence; [C-segment video-shadow proposal](../proposals/m5-pc110-c-segment-video-shadow.md); App-local firmware composition; Open-Source-PC110 `Discovery/65535/readme.md`, `Discovery/BIOS-Flash/readme.md` and `Discovery/Chipset/readme.md` as L2 research only. |
| Files And ABI Surface | `src/test/app-mypc110` firmware owner, diagnostic, evidence/current/queue/proposal records and their structural registration. Core routes are consumed unchanged. Lib, Emulator, X86 Core/X86-NXVM and unrelated Apps are excluded. |
| Applicable Rules | NXVM guide; Execution, Architecture, Coding, Documentation and source-policy authorities; one firmware/decode owner; external protected bytes stay BYOB; no third-party source copying. |
| Verification | Unit C000/F000/reset map coverage and the existing two-width real-TFT protected-bootstrap checkpoint, plus affected diagnostic/structural/manifest gates. |
| Expected Markers | Raw C000 before the observed sequence, writable shadow after it, raw C000 after reset, and the retained post-bootstrap checkpoint. No product-qualification claim. |
| Asset Needs | Read-only `NXVM_PROFILE_ASSETS_ROOT/profiles-nxvm/ibm-pc110-tft` BIOS/font and manifest only. |
| Reporting Requirements | Record L2 source grade, mapping model, actual changed surface, two-width facts and unqualified endpoints. Do not call this POST, Setup, DOS boot or PC110 release qualification. |
| Temporary Output | `build/t576-s1/` only for bounded diagnostics; remove it after recording compact evidence. |
| Stop Conditions | Mapping conflicts with the actual image, real execution still never reaches C000, a needed change leaves the App boundary, or implementation would fabricate guest behavior. Record and transfer rather than fabricate. |
| Exit Criteria | One App-owned video shadow route and its reset behavior are directly tested; both widths show the finite next protected-segment boundary before C000 execution; evidence and scoped gates are complete. |
| Original Owner Request | Continue the PC110 programme to closure using PC110-EMU, PC110-QEMU and Open-Source-PC110 only as research, with external usable firmware. |
| Similar-Issue Sweep | Inspect C000/D000/E000/F000/reset mappings, font overlap precedence, reset restoration, provider lifetime and all existing firmware routes; reject duplicate owners and static PC/AT aliases. |

### S1 Brief

Use the existing MyPC110 firmware provider and existing Core replacement routes.
Model only the observed EC/ED C-shadow transition, retaining raw immutable
flash before the guest opens and fills the shadow.  The real-firmware
diagnostic retains T575's exact finite protected-bootstrap checkpoint; a later
S must resolve that independent CPU boundary before it can prove C execution.

### S1 Delivery

The App now owns exactly one 48 KiB `C0000–CBFFF` shadow.  It remains hidden
behind immutable flash at reset, accepts the guest's copy only while the
observed EC/ED ROMSET/CAXS/CCBL sequence permits it, becomes visible after
relock, and is discarded by reset.  `CC000–CFFFF` is deliberately outside this
route.  No Core, Lib, Emulator, X86-NXVM or ROM byte changed.

All seven MyPC110 unit/diagnostic tests pass in both persistent Ninja graphs.
The real TFT diagnostic still completes T575's `F000:6044` checkpoint but then
stops at a separate protected-segment reload at `F000:605F`; it has not reached
C000.  That finite boundary is transferred to the next T576 S rather than
covered by an invented descriptor or execution shortcut.  The source grade,
mapping model and exact verification are in
[the S1 evidence](../etc/evidence/t576-s1-pc110-c-segment-shadow.md).

## M5 T575 S1 — Closed: MyPC110 protected-bootstrap segment-cache boundary

| Field | Required record |
| --- | --- |
| Identifier Mode | Successor to closed T574; one session performs coordinator and executor roles. |
| Admission And Approval | The owner directed continued PC110 implementation through closure and granted standing approval for ordinary implementation, diagnosis, verification and task sequencing. |
| Objective | Reproduce and classify the first protected-bootstrap transition that causes the admitted TFT firmware to leave F000 and fault before it reaches its video BIOS. |
| Non-goals | No static C-segment alias, BDA/IVT write, BIOS/interrupt interception, PC/AT facade, guessed SCAMP protocol, second executor, raw-ROM commit or PC110 release claim. |
| Reference Baseline | T574 two-width trace; [protected-bootstrap proposal](../proposals/m5-pc110-protected-bootstrap.md); App-local PC110 composition; existing CPU exception, memory and trace contracts; approved external TFT firmware/font assets. |
| Files And ABI Surface | App diagnostic/evidence plus the minimal Core/X86-NXVM configuration contract proved by the ordinary trace. Lib, Emulator, unrelated Apps and raw assets remain excluded. |
| Applicable Rules | NXVM guide; Execution, Architecture, Coding, Documentation and source-policy authorities; one firmware/decode owner; external bytes remain BYOB; PC110-QEMU/PC110-EMU are L2 references only. |
| Verification | Both widths produce a bounded instruction/exception/memory/port trace through the fault boundary. The strict default must still reject a zero descriptor; MyPC110 alone must complete the traced data-segment reload through its selected configuration. |
| Expected Markers | One reproducible pre-fault execution sequence, one exact before/after segment-cache result and one explicit owner disposition. No artificial transition to C000. |
| Asset Needs | Read-only `NXVM_PROFILE_ASSETS_ROOT/profiles-nxvm/ibm-pc110-tft` firmware/font and manifest only. |
| Reporting Requirements | Record source grade, exact two-width trace, owner decision, actual changed surface and all unqualified endpoints. Do not claim POST, DOS, Setup or release qualification. |
| Temporary Output | `build/t575-s1/` only for bounded logs/probes; delete after compact evidence is recorded. |
| Stop Conditions | The trace remains ambiguous, a required protocol is unproved, implementation would synthesize guest behavior, or a Shared contract is not minimal/owned. Record and transfer rather than fabricate. |
| Exit Criteria | The first protected-bootstrap failure is classified on both widths, its smallest evidence-backed owner fix is verified, and the next real boundary is recorded without fabricated continuation. |
| Original Owner Request | Continue the PC110 programme to closure using PC110-EMU, PC110-QEMU and Open-Source-PC110 only as research, with external usable firmware. |
| Similar-Issue Sweep | Compare protected-mode entry, segmentation, exception delivery, A20/KBC state, RAM/ROM map, controller reset state and configuration-port access; reject generic changes that merely make the BIOS proceed farther. |

### S1 Brief

Extend the existing real-firmware diagnostic only enough to capture the
instruction, descriptor/segment state and relevant memory/port events that
precede the first `F000:6044` exception and its reset aftermath.  Cross-check
the exact path against documented CPU semantics and labelled L2 external
behavior.  Select an implementation owner only after the trace isolates one.

### S1 Delivery And T575 Closure

The ordinary TFT-firmware trace identified a protected-mode `POP DS` at
`F000:6044`: selector `0048` has an all-zero GDT descriptor while `ES` already
holds a valid data cache for that same selector. Core retains strict loading by
default and admits one explicit, board-selected L2 policy that copies only that
existing visible data cache. MyPC110 is its sole consumer. Both widths execute
the real instruction through `F000:6048`; no descriptor, BIOS service,
C-segment route or PC/AT fallback is fabricated.

Both stable Ninja/ccache graphs completed the 661-test repository-only unit
suite and the 27-test external-ROM/disk integration suite. The Core/X86-NXVM
change also rebuilt and verified all four receiving PC App pairs as 0.5.0575;
their identities and hashes are recorded in the [S1 evidence](../etc/evidence/t575-s1-pc110-protected-bootstrap.md).
MyPC110 remains an unqualified, non-published diagnostic App: this bounded
receiver does not claim POST, Setup, DOS boot or a PC110 product release.

## Closed Task — M5 T575

| Task | Status |
| --- | --- |
| T575 | Closed after the strict-by-default protected data-cache receiver, two-width real-firmware trace, full repository qualification and receiving-PC artifact refresh. |

## M5 T574 S1 — Closed: MyPC110 C-segment ROM/shadow source gate

| Field | Required record |
| --- | --- |
| Identifier Mode | New task after closed T573; one session performs coordinator and executor roles. |
| Admission And Approval | The owner directed continued PC110 implementation through closure and granted standing approval for ordinary PC110 implementation choices. T573 transferred this exact source-backed mapping receiver. |
| Objective | Capture the real TFT firmware's VL82C420 mapping writes, then implement the smallest App-owned C-segment bank/shadow route that lets unchanged firmware reach its own video BIOS. |
| Non-goals | No BIOS/interrupt interception, synthetic POST/Setup/DOS service, PC/AT facade, second executor, raw-ROM commit, external runtime ROM loader, or PC110 release claim. |
| Reference Baseline | T573 trace evidence; [C-segment contract proposal](../proposals/m5-pc110-c-segment-shadow.md); App-local PC110 composition; existing X86 Core route contracts; approved TFT BIOS/font assets. |
| Files And ABI Surface | `src/test/app-mypc110` diagnostic/unit code and App-local `pc110_firmware`/`pc110_config`/composition are eligible. A minimal X86 Core VL82C420 observation addition is eligible only if the App cannot otherwise learn the selected EC/ED register. Lib, Emulator, unrelated Apps and raw assets remain excluded. |
| Applicable Rules | NXVM guide; Execution, Architecture, Coding, Documentation and source-policy authorities; one firmware/decode owner; external protected bytes remain BYOB; PC110-QEMU/PC110-EMU are labelled L2 behavior references only. |
| Verification | Both widths capture the actual C-segment configuration sequence using the ordinary executor; App tests prove reset/default map and each admitted mapping transition; both widths prove genuine C000 execution follows firmware configuration. Run affected unit/diagnostic, structural and documentation gates. |
| Expected Markers | One App-owned dynamic mapping owner, one recorded BIOS transition sequence and a real C000 VBIOS entry after firmware configuration. No static alias, host IVT/BDA write or firmware-address special case. |
| Asset Needs | Read-only `NXVM_PROFILE_ASSETS_ROOT/profiles-nxvm/ibm-pc110-tft` BIOS/font bytes and manifest only. Raw inputs remain outside Git and generated sources remain ignored. |
| Reporting Requirements | Record source grade, reset/default map, observed port values, exact changed ownership, both-width results and remaining unqualified endpoints. Do not call an execution checkpoint a bootable PC110 or publish an EXE. |
| Temporary Output | `build/t574-s1/` only for bounded diagnostic logs; delete it after copying the necessary compact facts into evidence. No reusable cache or user asset is touched. |
| Stop Conditions | The firmware emits no reproducible mapping sequence, required mapping needs a guessed protocol, a necessary change would add synthetic guest behavior, or a Shared contract expansion cannot be minimal and owned. Record and transfer rather than fabricate. |
| Exit Criteria | A source-labelled reset/default and runtime C-segment transition is implemented through one App-owned route, both widths execute real C000 code through the ordinary path, scoped regression/gates pass, and the next product-qualification receiver is updated with only proven facts. |
| Original Owner Request | Continue the entire PC110 programme to closure using PC110-EMU, PC110-QEMU and Open-Source-PC110 only as research, with usable firmware drawn from approved external assets. |
| Similar-Issue Sweep | Compare all C000/F000 mappings, reset overlays, EC/ED configuration writes, firmware provider lifetime and aliases; reject duplicate memory owners, static VBIOS aliases, PC/AT decode and duplicate selection state. |

### S1 Brief

Use the existing runtime trace facility to capture every PC110 configuration
port write and the surrounding C-segment execution boundary before changing
production code. If the observed transition is sufficient, put its mapping and
shadow state entirely behind the App's existing firmware/configuration owner.
Keep Core's routing mechanism neutral; do not make it recognize PC110 banks.

### S1 Delivery And T574 Closure

The normal real-firmware executor was traced for 8,000,018 instructions on
both x64 and x86.  It reached one processor reset and the same later CPU
exception, but emitted neither a captured candidate configuration-port access
nor a C000 entry.  Consequently the required VL82C420 C-segment mapping
protocol is not proved and is not implemented.  The App retains only the
source-backed mutable F-segment shadow; it does not write a BDA warm-reset
signature or add a static C-segment alias.  The exact source grades, commands,
results and transfer limit are in
[the S1 evidence](../etc/evidence/t574-s1-pc110-c-segment-trace.md).

T574 is closed by the candidate's explicit source/proof stop condition.  This
is not a POST, Setup, DOS-boot or release qualification claim; MyPC110 product
qualification remains blocked on a lawful, reproducible protected-bootstrap or
mapping contract.

## Closed Task — M5 T574

| Task | Status |
| --- | --- |
| T574 | Closed after S1's two-width source-gate trace; it rejected an unproven C-segment mapping rather than fabricating one. |

## M5 T573 S1 — Closed: MyPC110 real-firmware boundary trace

| Field | Required record |
| --- | --- |
| Identifier Mode | New task after closed T572; one session performs coordinator and executor roles. |
| Admission And Approval | The owner's standing instruction, “老规矩实现PC110”, continues the next queued MyPC110 qualification receiver. T572's accepted transfer requires this first finite trace before any feature or product claim. |
| Objective | Identify, reproduce and classify the first remaining CPU-visible service, interrupt or device boundary reached by the admitted real TFT PC110 firmware after T572's bounded checkpoint. |
| Non-goals | No synthetic POST/Setup/DOS service, BIOS interception, PC/AT facade, guessed controller protocol, PC110 executable release, raw asset commit, external runtime loader or broad product-qualification claim. |
| Reference Baseline | Closed T572 implementation/evidence; queued [MyPC110 qualification proposal](../proposals/m5-pc110-qualification.md); App-local PC110 composition; X86 Core/X86-NXVM existing execution and trace contracts; approved external TFT BIOS/font inputs. |
| Candidate Proposal | [MyPC110: product and guest qualification](../proposals/m5-pc110-qualification.md). |
| Files And ABI Surface | Initially App-local diagnostic/evidence and matching tests only. Existing X86 Core/X86-NXVM owner code is eligible only when the trace proves a generic defect or a complete, source-backed device contract; Lib, Emulator and unrelated Apps remain excluded. |
| Applicable Rules | NXVM guide; Execution, Architecture, Coding, Documentation and source-policy authorities; one executor/frame/firmware owner; external protected inputs remain BYOB; a bounded trace is diagnostic evidence, not guest qualification. |
| Verification | Reproduce the real firmware path on both widths through normal Machine execution; retain a finite trace with instruction/address, exception/interrupt or port/memory operation evidence; distinguish an existing generic defect from an absent source-backed PC110 endpoint; run affected diagnostic/unit and structural gates. |
| Expected Markers | One finite, reproducible failure boundary and one owner disposition: existing-owner fix with evidence, source-backed next receiver, or explicit blocker. No silent continuation or fake success marker. |
| Asset Needs | Read-only `NXVM_PROFILE_ASSETS_ROOT/profiles-nxvm/ibm-pc110-tft` BIOS/font bytes and manifest only. Raw inputs remain outside Git and generated sources remain ignored. |
| Reporting Requirements | Record both-width trace facts, the exact boundary, source grade, owner decision, actual changed surface and unexecuted qualification rows. Do not call an instruction budget or diagnostic trace a bootable PC110. |
| Stop Conditions | Missing lawful source for the boundary, a required unapproved Shared contract change, ambiguous/non-reproducible trace, proposed synthetic guest behavior or an external asset mismatch. Record and transfer rather than fabricate. |
| Exit Criteria | The first post-T572 boundary is reproduced and classified on both widths, its owner disposition is explicit, and all in-scope diagnostics/evidence/gates pass. Any actual implementation remains a later bounded S. |
| Original Owner Request | Continue the entire PC110 programme to closure using PC110-EMU, PC110-QEMU and Open-Source-PC110 only as research, with usable firmware drawn from approved external assets. |
| Similar-Issue Sweep | Compare the traced boundary against existing PC110 composition, KBC/RTC/PIT/port-61/F65535/firmware routes and current generic Core exception/trace mechanisms; reject duplicate traces, second executors, hidden PC/AT routes and firmware-address special cases. |

### S1 Brief

Start with the existing real-firmware diagnostic and normal X86-NXVM executor.
Extend only an existing diagnostic observation route if needed to make the
first remaining boundary finite and reproducible; do not add a production
fallback. Establish the complete observed batch before selecting any hardware
repair. A diagnosis that merely advances another instruction budget is not a
valid result.

### S1 Delivery And T573 Closure

The normal real-firmware executor reaches a finite, reproducible boundary on
both host widths: after a genuine processor-only reset, the BIOS executes
`INT 10h` at `F000:4CA9` with no executed C000 video BIOS and a zero IVT
target. The bounded diagnostic, rejected warm-signature and static-bank probes,
source grades, and explicit owner disposition are in [the S1 evidence](../etc/evidence/t573-s1-pc110-warm-reset-trace.md).

T573 closes its first-boundary source gate without a fabricated guest service,
PC/AT fallback, firmware-address special case, or PC110 release artifact. The
new [C-segment ROM/shadow contract candidate](../proposals/m5-pc110-c-segment-shadow.md)
owns the only admissible next receiver. The later product-qualification
candidate remains blocked on that contract and its existing source/asset gates.

## Closed Task — M5 T573

| Task | Status |
| --- | --- |
| T573 | Closed after S1's finite real-firmware boundary classification and explicit transfer; this is not a POST, Setup, DOS-boot or product-qualification claim. |

## M5 T572 S1 — Closed: PC110 real-firmware POST admission

| Field | Required record |
| --- | --- |
| Identifier Mode | New task after closed T571; one session performs coordinator and executor roles. |
| Admission And Approval | The owner directed continued PC110 implementation through to closure, with real external firmware and no synthetic BIOS, DOS or device fallback. |
| Objective | Connect the admitted MyPC110 board composition to the existing Emulator/Machine/Product execution path, embed the approved TFT BIOS and font inputs at build time, and execute the guest from reset through a bounded real-firmware POST diagnostic. |
| Non-goals | No POST completer, BIOS interception, fake storage/audio/power/PCMCIA device, external-ROM runtime loader, PC/AT facade, raw-firmware commit, or claim of completed DOS boot before the real checkpoint passes. |
| Reference Baseline | Closed T565--T571 PC110 receivers; `app-mypc110` base composition; X86-NXVM Machine/Product construction; external PC110 research and the external TFT asset manifest. |
| Candidate Proposal | [PC110: real firmware POST, setup and guest boot](../history/M5-T572-pc110-firmware-boot-proposal.md). |
| Files And ABI Surface | `src/test/app-mypc110`, applicable X86-NXVM Machine construction only if its existing profile seam cannot express the App-local route, selected CMake/artifact wiring, manifest and current PC110 evidence/docs. Shared Lib/Emulator/X86 Core and unrelated Apps stay excluded unless evidence proves a real contract gap. |
| Applicable Rules | NXVM guide; Execution, Architecture, Coding, Documentation and source-policy authorities; one execution/frame/firmware owner; external protected payloads remain BYOB. |
| Verification | Validate frozen external asset hashes before generation; execute the real reset vector through the normal driver on both widths; prove no synthetic handler participates; run affected App/X86-NXVM units and structural gates. Publish no PC110 EXE until its actual product qualification is separately earned. |
| Expected Markers | One App-owned PC110 machine binding, one X86-NXVM execution adapter, immutable embedded firmware bytes, one standard copied-frame path and a finite POST diagnostic that records the first real unsupported boundary rather than replacing it. |
| Asset Needs | Read-only `NXVM_PROFILE_ASSETS_ROOT/profiles-nxvm/ibm-pc110-tft` BIOS/font bytes and manifest only. Raw inputs remain outside Git and generated embedding sources remain under ignored build output. |
| Reporting Requirements | Record hashes, actual owner/diff, reset/POST result, source grades, executed versus unexecuted checkpoints, and any transferred hardware gap. Do not call a diagnostic boot a usable PC110 product. |
| Stop Conditions | Invalid asset pairing/hash, a required undocumented device protocol, an unavoidable Shared contract change, or any proposed synthetic guest path. Record the exact first boundary and transfer it. |
| Exit Criteria | Met for the bounded admission checkpoint: App-local product execution is real and bounded; both widths pass affected tests; generated artifact inputs are provenance-checked; the real-firmware checkpoint transfers the remaining guest boundary to the next receiver. |
| Original Owner Request | Continue the entire PC110 programme to closure using PC110-EMU, PC110-QEMU and Open-Source-PC110 only as research, with usable firmware drawn from approved external assets. |
| Similar-Issue Sweep | Compare all existing fixed PC App bindings, firmware embedding, driver/lifecycle/frame routes and PC110 board routes; reject duplicate Core creation, second frame paths, external firmware file access and PC/AT-only assumptions. |

### S1 Delivery And T572 Closure

The existing base composition created Core directly, while the normal product
path created that same Core through `x86-nxvm/machine`. S1 makes the existing
profile-construction seam carry the PC110-owned firmware, F65535, font and
VL82C420 routes so the normal single executor owns reset, lifecycle and copied
frames. The real TFT image passed the bounded eight-million-instruction and
one-shutdown/reset checkpoint on both widths. It did not complete POST, Setup
or DOS boot; no PC110 executable is published. The next [MyPC110 product and
guest qualification](../proposals/m5-pc110-qualification.md) receiver owns a
finite trace of the first remaining CPU-visible service, interrupt or device
boundary. The provenance, generic CPU/Board fixes, exact checkpoint and
transfer are recorded in [the S1 evidence](../etc/evidence/t572-s1-real-firmware-post.md).

## M5 T571 S1 — Closed: PC110 audio and auxiliary admission

| Field | Required record |
| --- | --- |
| Identifier Mode | New task after closed T570; one session performs coordinator and executor roles. |
| Admission And Approval | The ordered PC110 programme continues. This source gate separates independently implementable audio/auxiliary devices from physical inventory and external placeholders. |
| Objective | Admit only PC110 audio and remaining peripheral behavior with a complete CPU-visible decode, configuration, reset, IRQ/DMA, ownership and failure contract. |
| Non-goals | No speaker-only completion claim, host-audibility test, synthetic interrupt, host-side modem/serial/dock behavior, raw firmware/media import, product EXE or guest-boot claim. |
| Reference Baseline | PC110 audio/auxiliary proposal; T570 source-gate outcome; current MyPC110 composition; Open-Source-PC110, PC110-EMU and PC110-QEMU read-only research; existing X86-NXVM audio/peripheral contracts. |
| Candidate Proposal | [PC110: audio and remaining peripherals](../proposals/m5-pc110-audio-aux.md). |
| Files And ABI Surface | S1 changes documentation/evidence and task/Queue disposition only. A later receiver may name App-local PC110 routes and an existing independent owner; no interface is presumed. |
| Applicable Rules | NXVM guide; Execution, Architecture, Coding, Documentation and source-policy authorities; one device/lifecycle/media owner and no App-to-peer dependency. |
| Verification | Inventory actual PC110 devices and source-qualified CPU-facing contracts; classify direct, model-only, blocked and conflicting paths against existing mechanisms. |
| Stop Conditions | An endpoint lacks lawful source or reset/configuration/IRQ/DMA semantics, conflicts with existing ownership, requires a Shared contract or depends on host output. Transfer it without a placeholder. |
| Exit Criteria | Every selected audio/auxiliary row is admitted with a bounded owner/proof or transferred to a named receiver; no physical connection is misrepresented as a device protocol. |
| Original Owner Request | Continue implementing PC110 in ordered receivers. |
| Similar-Issue Sweep | Compare every candidate against current PC speaker, ESS/OPL, serial/parallel/IR/modem/dock and board routes for accidental PC/AT assumptions or duplicated ownership. |

### S1 Delivery And T571 Closure

The [audio/auxiliary source gate](../etc/evidence/t571-s1-audio-aux-source-gate.md)
preserves the real ES488 SB2 reset/version and OPL2-status observations while
rejecting an incomplete PCM/DMA/IRQ/output model and all physical-only
Super-I/O/dock endpoints. Neither PC110-EMU's explicit placeholders nor
PC110-QEMU's PC-speaker route is a valid substitute. T571 therefore leaves
public audio and the App composition unchanged, and transfers configuration
and observed transactions to real-firmware execution.

## M5 T571 — Closed: PC110 audio and auxiliary disposition

T571 closes its source gate without misrepresenting discovery-level wiring as
a runnable audio or peripheral protocol. The PC110 program proceeds to real
firmware POST, setup and guest-boot evidence.

## M5 T570 S1 — Closed: PC110 storage and flash admission

| Field | Required record |
| --- | --- |
| Identifier Mode | New task after closed T569; one session performs coordinator and executor roles. |
| Admission And Approval | The ordered PC110 programme continues. This S freezes implementable PCMCIA, flash and boot-storage facts before touching device routes. |
| Objective | Establish a finite storage/flash cohort with complete CPU-visible decode, media, reset, removal/error and ownership contracts. |
| Non-goals | No synthetic INT 13h, host decompression, implicit first floppy, raw firmware/media commit, flash-master mutation, generic card manager, runnable PC110 EXE or guest-boot claim. |
| Reference Baseline | PC110 storage proposal; current MyPC110 base and VL82C420 routes; Open-Source-PC110, PC110-EMU and PC110-QEMU read-only research; existing X86-NXVM media contracts. |
| Candidate Proposal | [PC110: PCMCIA, flash and boot storage](../history/M5-T570-pc110-storage-proposal.md). |
| Files And ABI Surface | Documentation/evidence and task/Queue disposition only during S1. Later work may name App-local PC110 routes and already owned Core/X86-NXVM storage mechanisms; no interface is presumed. |
| Applicable Rules | NXVM guide; Execution, Architecture, Coding, Documentation and source-policy authorities; one media/flash owner and no App-to-peer dependency. |
| Verification | Inventory current Core/X86-NXVM storage contracts and each researched PC110 decode; classify direct, model-only, blocked and conflicting paths. |
| Stop Conditions | Required behavior lacks lawful source/asset, needs a Shared contract, conflicts with existing controller ownership or cannot express errors/removal. Transfer it without a placeholder. |
| Exit Criteria | Every selected storage/flash row is admitted with a bounded owner/proof or transferred to a named receiver; no guessed boot path remains. |
| Original Owner Request | Continue implementing PC110 in ordered receivers. |
| Similar-Issue Sweep | Compare all PC110 storage/flash candidates against current FDC/HDC/media/firmware routes for duplicate ownership or implicit PC/AT assumptions. |

### S1 Delivery And T570 Closure

The [storage/flash source gate](../etc/evidence/t570-s1-storage-source-gate.md)
confirms that flash read mapping is already singular and correct, while the
known PCMCIA, CF/ATA and dock-floppy facts do not establish the controller
registers, decode, reset, IRQ/DMA, removal or error contracts required for a
truthful route.  PC110-EMU's corresponding port handlers are explicit
placeholders and PC110-QEMU's software boot completer is outside the machine
boundary.  T570 therefore adds no PC/AT-shaped fallback, no synthetic boot
path and no production/test code.  The real-firmware boot receiver owns the
required traces; the PC110 program remains active.

## M5 T570 — Closed: PC110 storage and flash disposition

T570 preserves the one-owner immutable firmware mapping and transfers the
unproven PCMCIA, ATA/CF and dock-floppy behavior to real-firmware execution.
It closes a source gate, not the PC110 product goal.

## M5 T569 S1 — Closed: PC110 APM and power protocol admission

| Field | Required record |
| --- | --- |
| Identifier Mode | New task after closed T568; one session performs coordinator and executor roles. |
| Admission And Approval | The owner-directed PC110 programme continues in Queue order. T567 transferred power/APM rather than fabricating it; this S is the required source-gate audit. |
| Objective | Establish whether any PC110 CPU-visible APM, suspend, power-off or wake behavior has a complete enough contract for a bounded implementation receiver. |
| Non-goals | No U6 emulator, `0ECh`/`0EDh` power alias, BIOS interrupt interception, host lifecycle alias, always-ready mailbox, raw firmware import/execution, runnable product or PC110 EXE claim. |
| Reference Baseline | T567 S3 power disposition; Open-Source-PC110 current BIOS/chipset research; PC110-EMU/QEMU as read-only L2 references; current MyPC110/VL82C420/Core source. |
| Candidate Proposal | [PC110: power management and APM](../proposals/m5-pc110-power-apm.md). |
| Files And ABI Surface | Documentation/evidence and task/Queue disposition only unless a complete CPU-visible contract admits one existing owner. No public interface is presumed. |
| Applicable Rules | NXVM guide; Execution, Architecture, Coding, Documentation and source-policy authorities; one lifecycle/frame/CPU-event owner. |
| Verification | Trace each candidate call/register to current source ownership; record request, reply/error, reset, lifecycle and wake semantics with source grade; prove any proposed receiver has one owner and no conflict with `VL82C420`. |
| Stop Conditions | Incomplete protocol, reliance on host lifecycle, protected firmware execution, or a needed Shared contract. Transfer the exact missing behavior rather than invent success. |
| Exit Criteria | Every inspected APM/power row is either admitted with a bounded owner and proof or transferred to a named receiver; evidence, Queue and task history remain truthful. |
| Original Owner Request | Continue implementing PC110 in ordered receivers. |
| Similar-Issue Sweep | Inspect all current PC110, VL82C420 and Core power/suspend hooks; distinguish CPU-visible semantics from physical wiring and host lifecycle. |

### S1 Delivery

The [power/APM source gate](../etc/evidence/t569-s1-power-apm-source-gate.md)
confirms that existing VL82C420 routing is correct and that no complete
guest-visible suspend/wake or APM device contract is available.  It admits no
invented MCU, I/O window or lifecycle alias.  The real-firmware boot receiver
owns the missing execution traces and any later App-local power implementation.

## M5 T569 — Closed: PC110 power management and APM disposition

T569 closes its source gate without a production device.  The currently known
APM status route belongs to the real BIOS, `0ECh/0EDh` remain VL82C420
configuration, and the observed Block2 suspend write lacks a complete wake and
resume contract.  The task therefore preserves the existing one-owner routes
and transfers power behavior to the real-firmware boot receiver rather than
fabricating a mailbox or host-lifecycle mapping.

## M5 T568 S5 — Closed: PC110 F65535 standard-VGA and capture contract

| Field | Required record |
| --- | --- |
| Identifier Mode | S5 follows committed S4; one session performs coordinator and executor roles. |
| Admission And Approval | S4 established that F65535 cannot be a VADP personality. The owner permits L2 models derived from external emulators when original manuals do not establish behavior, provided they are labelled as such. |
| Objective | Establish the smallest separate standard-VGA register/memory/capture contract that can make a truthful F65535 frame available through the existing single copied-frame route. |
| Non-goals | No VADP personality, host font replacement, panel timing or compensation guess, power model, second renderer/frame path, raw-ROM commit or runnable-product claim. |
| Reference Baseline | S3 manual/live ledger, S4 foundation, C&T F65535 Revision 2.0, read-only PC110-EMU/PC110-QEMU research. |
| Candidate Proposal | [PC110: F65535 display and font ROM](../proposals/m5-pc110-display.md). |
| Files And ABI Surface | Candidate `x86-nxvm/chips/f65535` and matching test/App route only. Any Board/Machine bridge need is first recorded as an explicit receiver, not silently coupled to VADP. Lib/Emulator/X86 Core remain excluded. |
| Applicable Rules | NXVM guide; Execution, Architecture, Coding and Documentation rules; source policy; one frame publication owner. |
| Verification | Register/planar-memory vectors, text/graphics capture vectors and proof that a later machine bridge has one owner. Both cached widths run affected units/manifests/gates. |
| Stop Conditions | A capture requires a Shared frame/API change, relies on a guessed panel value, or needs a VADP/PC-AT facade. Record the exact receiver rather than manufacture a shortcut. |
| Exit Criteria | A labelled L3/L2 contract, implementation only for bounded behavior, direct dual-width tests and an explicit remaining product bridge/panel receiver. |
| Original Owner Request | Continue the PC110 programme after T567 closure. |
| Similar-Issue Sweep | Compare existing video register/memory/capture logic only to identify facts and reusable shape; do not copy PC/AT personality or source code. |

### S5 Delivery

S5 adds an L2 standard-VGA text subset to the independent F65535 owner:
standard register storage, planar/odd-even CPU aperture access and a direct
copied text snapshot.  MyPC110 routes the documented standard port range and
aperture through Core without using VADP or a private frame path.  The
[S5 VGA/capture record](../etc/evidence/t568-s5-f65535-vga-capture.md)
separates the L3 interface observations from the compatibility model and
records the deliberately unimplemented graphics/panel/product bridge.  Both
cached widths pass the direct chip, App route and X86-NXVM structural proof,
and the full repository-only unit corpus passes 660/660 per width.
No PC110 executable or firmware-boot claim is made by this delivery.

## M5 T568 — Closed: PC110 F65535 display and font ROM

T568 closes the bounded F65535/font receiver, not a runnable PC110 product.
S1--S5 establish one App-local font window and one independent F65535 owner:
the sourced `XR00=C1h` identity, 512 KiB VRAM, extension-index access and an
L2 standard-VGA text capture path.  MyPC110 owns its Core routes; neither the
PC/AT VADP nor a private host frame path is used.

Both stable cached Ninja widths passed the complete repository-only unit corpus
(660/660 each), then the owner-managed integration corpus (27/27 each).
Documentation governance and component manifest/dependency gates passed.
The F65535 addition reaches no admitted runnable PC App, so the generated
timestamp-only NXVM 0564 binary changes were deliberately discarded instead
of publishing a false PC110 release.

Graphics/palette scanout beyond the current shared-frame capacity, physical
panel behavior, real firmware POST, storage, power and a product frame bridge
remain with their already queued PC110 storage, firmware-boot and product
qualification receivers.  They are not approximated here.

## M5 T568 S4 — Delivered: PC110 F65535 register and VRAM foundation

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation of T568 after closed S3; one session performs coordinator and executor roles. |
| Admission And Approval | S3's C&T/manual and PC110 live-observation ledger admits only the independent controller's directly sourced identity, extension-index path, reset state and 512 KiB logical-VRAM foundation. |
| Objective | Implement the smallest standalone F65535 chip mechanism and MyPC110 route binding: resettable 512 KiB VRAM, `3D6h`/`3D7h` extension index/data behavior and observed `XR00=C1h` identity. |
| Non-goals | No VGA-compatible display engine, panel timing/compensation, panel power, unqualified register defaults, guest BIOS boot, raw ROM commit, product EXE or Lib/Emulator/X86 Core/VADP modification. |
| Reference Baseline | T568 S3 ledger, C&T F65535 Revision 2.0 register manual, PC110 live-dump identity observation, Core port/memory route contracts and App-local MyPC110 base composition. |
| Candidate Proposal | [PC110: F65535 display and font ROM](../proposals/m5-pc110-display.md). |
| Files And ABI Surface | New `src/test/x86-nxvm/chips/f65535` implementation/unit component and App-local MyPC110 route/composition test binding only. No Lib, Emulator, X86 Core, VADP or raw-asset change. |
| Applicable Rules | NXVM guide; Execution, Architecture, Coding and Documentation rules; source policy; current architecture/coding authorities; no second guest-frame or renderer owner. |
| Verification | Both widths prove exact VRAM bounds/reset, `XR00`, seven-bit index masking, read-only identity and App/Core route ownership. Existing MyPC110 and X86 NXVM units retain behavior. |
| Expected Markers | One F65535 lifetime and register/VRAM owner, one App wiring owner, no VADP route, no second frame/renderer and no inferred panel behavior. |
| Asset Needs | Unit bytes only; all real ROM/media remains external and uncommitted. |
| Reporting Requirements | Record actual source/test diff, L3/L2 distinction, rejected controller scope and unexecuted guest/panel/desktop validation. |
| Stop Conditions | A required behavior needs a Shared change, normal-VGA display semantics cannot be bounded from the foundation, or a panel value would be guessed. |
| Exit Criteria | Both widths pass direct chip/App integration units and relevant static/manifest/docs gates; actual changes are reviewed and pushed; the next capture/panel receiver stays explicit. |
| Original Owner Request | Continue the PC110 programme after T567 closure. |
| Similar-Issue Sweep | Check all App-local device routes for an external ROM path, VADP dependency, guessed auxiliary behavior or duplicate memory owner. |

### S4 Delivery

S4 adds a bounded F65535 owner with sourced 512 KiB VRAM, `3D6h`/`3D7h`
extension access, seven-bit index masking and the observed immutable
`XR00=C1h` identity.  MyPC110 routes only the normal CPU aperture and those
ports through Core; it neither impersonates VADP nor publishes a second frame.
The [S4 foundation record](../etc/evidence/t568-s4-f65535-foundation.md)
captures the L3 facts, direct dual-width tests and deliberate exclusions.
The next receiver owns a separately admitted standard-VGA/capture audit; no
guest, panel, BIOS or product-EXE claim is made by this delivery.

### S3 Delivery

The [F65535 controller contract](../etc/evidence/t568-s3-f65535-controller-contract.md)
records the original C&T register/manual facts, the PC110 live identity and
the one truthful ownership split.  It admits a later standalone
`x86-nxvm/chips/f65535` mechanism with 512 KiB logical VRAM and App-local
wiring; it rejects VADP impersonation, reconstruction-board capacity and
unsourced panel defaults.  Both cached widths pass documentation governance.
S3 is closed after its documentation P; S4 is the bounded chip/route
implementation receiver.

### S2 Delivery

The [font-ROM window record](../etc/evidence/t568-s2-font-rom-window.md)
captures the App-local `1160h`/`DE000h` route, its required overlap priority
and the corrected stable composition lifetime. Both cached widths pass all
four MyPC110 units. The unresolved F65535 chip/panel receiver remains next;
no raw ROM, executable or guest claim is introduced.

### S1 Delivery

The [F65535/font admission record](../etc/evidence/t568-s1-f65535-font-admission.md)
separates the directly observed App-local font window from the still
source-qualified F65535 receiver. It confirms that existing VADP is not a
truthful F65535 owner and that no Shared frame change is presently justified.
S1 changes only documentation and task evidence; it makes no display, ROM,
boot or artifact claim.

## M5 T567 S3 — Closed: PC110 power and suspend/wake disposition

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation of T567 after closed S2; one session performs coordinator and executor roles. |
| Admission And Approval | The owner directed the admitted PC110 T to closure. S1/S2 transfer only the still-unresolved power, status and wake rows here. |
| Objective | Decide whether the PC110 U6 power-controller CPU protocol is sufficiently evidenced for one deterministic guest-visible model, or transfer it without a placeholder to a newly bounded evidence receiver. |
| Non-goals | No fabricated mailbox, always-ready reply, host power shortcut, Emulator lifecycle alias, MCU firmware import/execution, rail simulation, display/storage/audio/product EXE or Shared Lib/Emulator modification. |
| Reference Baseline | S1 audit; S2 KBC/TrackPoint composition; original U6 power-sequence evidence, Pluto port observations and current App/Core lifecycle ownership. |
| Candidate Proposal | [PC110: keyboard, pointer and power](../proposals/m5-pc110-input-power.md). |
| Files And ABI Surface | Documentation/evidence and task disposition unless an already-sourced CPU-visible protocol establishes an App-local port model. No interface is presumed. |
| Applicable Rules | NXVM guide; Execution, Architecture, Coding and Documentation rules; source policy; one lifecycle owner and no fabricated-success invariants. |
| Verification | Inspect all cited physical/firmware protocol facts against current Core lifecycle and I/O abstractions; enumerate request, reply, ready, error, reset and wake semantics before admitting code. |
| Expected Markers | A finite supported contract or an explicit no-device decision plus a separately queued receiver; no overlap with Emulator machine lifecycle. |
| Asset Needs | Read-only research only. No raw MCU payload, ROM or external code is copied. |
| Reporting Requirements | State exact evidence grade, rejected models, code surface if any, unexecuted physical proof and the route that owns unresolved work. |
| Stop Conditions | CPU-visible protocol remains incomplete, conflicts with one-owner lifecycle, or requires a new Shared contract. Close the row as unsupported and transfer it rather than guessing. |
| Exit Criteria | The power/suspend/wake row is resolved into bounded implementation or explicit transfer, records are updated and the S P is pushed. |
| Original Owner Request | Continue the PC110 programme to the real product end state after T566 closure. |
| Similar-Issue Sweep | Check every current App/Core power/reset shortcut for accidental reuse or second lifecycle ownership. |

### S3 Delivery And T567 Closure

The [power disposition](../etc/evidence/t567-s3-power-disposition.md) confirms
physical U6 and APM facts but not a complete CPU-visible power contract.  The
historical `0ECh/0EDh` attribution conflicts with the source-qualified chipset
route, so no speculative power device is admitted.  The source-gated
[PC110 power management and APM](../proposals/m5-pc110-power-apm.md) receiver
owns the unresolved row.  [T567 closure](../history/M5-T567-pc110-input-power.md)
records the completed neutral input transport and this explicit transfer.

## M5 T567 S2 — Closed: PC110 keyboard and TrackPoint composition

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation of T567 after closed S1; one session performs coordinator and executor roles. |
| Admission And Approval | The owner directed the entire admitted PC110 T to its real closure. S1's evidence admits only the proven keyboard/TrackPoint cohort. |
| Objective | Compose the proven PC110 `60h`/`64h` 8042-style keyboard transport and standard PS/2 TrackPoint through the neutral PC110 Board without installing PC/AT facade behavior. |
| Non-goals | No inking, power MCU, suspend/wake, display, storage, firmware import, product EXE or guest boot claim; no Lib/Emulator/Public-Shared interface change. |
| Reference Baseline | T566 App-local base, S1 input/power audit, current X86 NXVM KBC/keyboard/PS2 mechanisms and the cited measured PC110 sources. |
| Candidate Proposal | [PC110: keyboard, pointer and power](../proposals/m5-pc110-input-power.md). |
| Files And ABI Surface | `src/app-mypc110`, `test/app-mypc110`, and one additive X86 NXVM Board keyboard-topology enum. New routes must stay App-local; no PC/AT Board adapter reuse and no Lib/Emulator or CPU interface change. |
| Applicable Rules | NXVM guide; Execution, Architecture, Coding and Documentation rules; NXVM architecture/coding/source policy; S1 evidence and component dependency direction. |
| Verification | Directly prove `60h`/`64h` command/data behavior, OBF/IBF, guest-facing make/break, AUX reset/ID/status and atomic three-byte pointer reports; prove no PC/AT facade route appears. |
| Expected Markers | One PC110-owned composition and one input route; real protocol failures propagate; power and inking remain absent rather than falsely succeeding. |
| Asset Needs | Research sources only. No firmware/media payload enters Git or an artifact. |
| Reporting Requirements | Record direct/composed proofs, exact reuse boundary, unavailable guest/physical proof and the remaining power transfer. |
| Stop Conditions | Required 8042 side effect cannot be isolated from the PC/AT facade, or a route requires unsourced power/inking semantics. Transfer the row instead of approximating it. |
| Exit Criteria | PC110 can compose/destroy the proven input transport with direct tests on both widths and unchanged existing Apps; manifests/docs pass and the implementation P is pushed. |
| Original Owner Request | Continue the PC110 programme to the real product end state after T566 closure. |
| Similar-Issue Sweep | Check all PC110 assembly/test paths for a hidden PC/AT board dependency, second KBC ownership or direct host-to-BIOS input path. |

### S2 Delivery

The [keyboard and TrackPoint composition record](../etc/evidence/t567-s2-keyboard-trackpoint-composition.md)
captures the neutral 8042 transport boundary, direct `60h`/`64h` and AUX
proof, and its deliberate exclusions.  Existing PC Apps retain their PC/AT
selection; MyPC110 alone uses the new neutral selection.  S2 is closed after
both widths pass the direct App tests plus X86 NXVM structural/manifest gates.

## M5 T567 S1 — Closed: PC110 input/power contract audit

| Field | Required record |
| --- | --- |
| Identifier Mode | New M5 T567 from the first ordered PC110 endpoint receiver after closed T566; one session performs coordinator and executor roles. |
| Admission And Approval | The owner’s persistent instruction, “老规矩实现PC110”, and the prior approved PC110 programme admit the next ordered receiver. This S is an audit only; it does not authorize speculative hardware behavior. |
| Objective | Freeze the implementable keyboard, pointer and power-control batch by reconciling current Core/X86 NXVM mechanisms, the admitted PC110 source ledger and the PC110 App base composition. |
| Non-goals | No product EXE, firmware/ROM import, MCU execution, host-key shortcut, synthetic power mailbox, reset/default inference, display/storage/audio implementation, Shared Lib/Emulator change or peer-App dependency. |
| Reference Baseline | Closed T566 composition `3ff3982cd`; PC110 input/power proposal; T558 receiver checklist IN-1/IN-2/PM-1/PM-2; current X86 NXVM keyboard/mouse mechanisms; pinned research checkouts and external asset manifests read only. |
| Candidate Proposal | [PC110: keyboard, pointer and power](../proposals/m5-pc110-input-power.md). |
| Files And ABI Surface | Documentation/evidence, Current and Queue disposition only during S1. A later S may nominate App-local PC110 source/test files or a proven existing X86 NXVM mechanism; no public interface is presumed. |
| Applicable Rules | NXVM guide; Execution, Architecture, Coding and Documentation rules; NXVM architecture/coding/roadmap; source-and-research policy; one-owner/timeline and inward-dependency invariants. |
| Verification | Inspect all current KBC, PS/2 pointer, keyboard ingress, reset and lifecycle call chains; classify every PC110 input/power route as reusable, App-local receiver, source-model candidate or blocked; record source grade and direct/composed proof. |
| Expected Markers | One finite IN/PM disposition ledger, no duplicate controller or power lifecycle owner, no host-key simulation mistaken for guest protocol, and one bounded next implementation cohort only where evidence supports it. |
| Asset Needs | Read-only external PC110 research and owner-managed asset metadata only. No protected firmware, MCU bytecode, media or ROM payload crosses into Git or an artifact. |
| Reporting Requirements | Record concrete route/ownership findings, source grades, rejected shortcuts, candidate source/test surface, missing proof and all unexecuted guest/hardware validation. |
| Stop Conditions | Required route/reset/power semantics lack a lawful source or require a Shared contract; conflicting evidence cannot be resolved into a labelled model; another task owns the mechanism. Transfer that exact row rather than fabricate an always-ready device. |
| Exit Criteria | The finite input/power batch is exhausted into directly implementable rows, named later receivers or explicit blockers; evidence and Queue disposition are updated, documentation checks pass and the audit P is pushed. |
| Original Owner Request | Continue the PC110 programme to the real product end state after T566 closure. |
| Similar-Issue Sweep | Compare every existing X86 NXVM keyboard/pointer/power ingress and all App construction paths for accidental PC/AT endpoint assumptions or duplicate guest-input/lifecycle ownership. |

### S1 Delivery

The [input/power contract audit](../etc/evidence/t567-s1-input-power-audit.md)
establishes a bounded direct next cohort: the observed `60h`/`64h` 8042-style
keyboard transport and live-probed standard PS/2 TrackPoint.  It explicitly
rejects the existing PC/AT board facade, BIOS key injection and an invented
power mailbox.  Power, suspend/wake and inking remain named later receivers
because their CPU-visible contracts are not yet source-qualified.  S1 changes
only the task record, Queue disposition and evidence; it makes no product,
asset or runtime claim.

## M5 T566 S3 — Closed: PC110 App-local base composition

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation of T566 after closed S1--S2; one session performs coordinator and executor roles. |
| Admission And Approval | The owner directed T566 to completion. S1 and S2 explicitly transfer the typed firmware and configuration-route bindings to this App-local assembly cohort. |
| Objective | Create the one minimal PC110 assembly root: 486SX Core, neutral PIC/PIT/DMA Board substrate, TFT firmware map and VL82C420 route adapter. |
| Non-goals | No PC110 executable, reset alias, panel/display, keyboard, pointer, storage, RTC seed, PCMCIA, power, audio, POST or boot claim. |
| Files And ABI Surface | `src/app-mypc110`, matching `test/app-mypc110`, build registration and PC110 evidence only. No Lib, Emulator, Core or public Shared interface change. |
| Verification | Both stable Ninja/ccache widths build and run the three PC110 App units; direct Board neutral-controller regression remains green. |
| Exit Criteria | One App owner creates and tears down the complete admitted base atomically; the test proves firmware mapping and absence of forced PC/AT endpoints; scope/evidence accurately transfer every unimplemented endpoint. |
| Stop Conditions | Any required reset, input, display, storage or firmware behavior would need an invented PC/AT fallback or a new Shared contract. Transfer it to its named later receiver. |

### S3 Delivery And T566 Closure

S3 adds the one App-local base composition and its direct regression. The
composition owns only the admitted 486SX, neutral PIC/PIT/DMA substrate, TFT
firmware map and VL82C420 route binding. It explicitly has no PC/AT facade,
input, display, storage or executable publication path.

T566 is closed as the minimal PC110 base-composition task. Its three
implementation commits are `a6179bd67`, `201fff250` and `aef783205`; the
[T566 closure](../history/M5-T566-pc110-base-composition.md) transfers every
remaining endpoint and guest/product qualification to the ordered PC110
receiver tasks. Both stable Ninja/ccache widths pass the four affected direct
units, documentation governance and the MyPC110 dependency gate. No release
artifact is due because the resulting App is deliberately non-runnable.

## M5 T566 S2 — Closed: neutral X86 NXVM controller substrate

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation of M5 T566 after closed S1; one session performs coordinator and executor roles. |
| Admission And Approval | The owner directed the whole PC110 task to completion and S1 identified this bounded implementation as its next cohort. Standing approval covers X86 NXVM/Core-facing construction work that does not modify Lib or Emulator. |
| Objective | Make the existing X86 NXVM Board lifetime capable of instantiating its reusable PIC/PIT/DMA substrate without forced PC/AT keyboard, VADP, port-92/A20 or automatic reset-alias endpoints. |
| Non-goals | No PC110 guest boot, executable, ROM embedding, RTC seed, panel, keyboard, pointer, storage, PCMCIA, power, audio, PC/AT descriptor reuse, Shared Lib/Emulator change or second board/device manager. |
| Files And ABI Surface | `src/test/x86-nxvm/board-base` configuration/constructor/test surface and their manifests; task evidence/Current only. Existing numeric choices retain behavior; new no-endpoint selections are additive. |
| Verification | x64/x86 direct no-endpoint construction test proves PIC/PIT remain available while 60h, 92h and 3DAh remain uninstalled; existing X86 NXVM unit corpus, manifests and dependency gates retain behavior. |
| Exit Criteria | One Board object and one Core attachment own the controller lifetime; no forced AT facade under explicit no-endpoint selections; existing Apps retain defaults; required two-width tests and review pass, then P is pushed. |
| Stop Conditions | A controller requires an absent endpoint, or PC110 needs a reset/RTC/device behavior not represented by the selected neutral substrate. Transfer that behavior to its named receiver rather than installing an AT substitute. |

### S2 Delivery

The [neutral-controller substrate record](../etc/evidence/t566-s2-neutral-controller-substrate.md)
documents the three explicit selections and the one new direct regression.
Focused x64/x86 construction tests and both manifests pass.  The change is
limited to X86 NXVM's internal Board configuration and test corpus; Lib,
Emulator, X86 Core, firmware assets and existing App selections do not change.
S2 is closed after its implementation P; the next S may compose these selected
mechanisms into the App-local PC110 root.

## M5 T566 S1 — Closed: PC110 minimal base-composition audit

| Field | Required record |
| --- | --- |
| Identifier Mode | New M5 T566 S1 from the next Queue candidate, after closed T565; one session performs coordinator and executor roles. |
| Admission And Approval | The owner’s persistent goal, “老规矩实现PC110”, directs continuation after T565 closure. Standing in-scope approval admits this source/test/design audit. |
| Objective | Freeze the exact PC110 base-composition universe: identify the reusable standard controller mechanisms, every forced PC/AT side effect in the current X86 NXVM board constructor, the smallest legal extraction boundary, and all remaining PC110 endpoint receivers. |
| Non-goals | No production implementation, executable, firmware embedding, AT descriptor reuse, reset/default inference, guest boot claim, external asset change, Shared Lib/Emulator change or third-party source import. |
| Reference Baseline | Closed T565 history and S8 transfer audit; current `x86-nxvm/board-base`, `x86-core`, `app-mypc110` and their tests; pinned PC110 research checkouts as read-only evidence. |
| Candidate Proposal | [PC110: minimal base composition](../proposals/m5-pc110-base-composition.md). |
| Files And ABI Surface | Documentation/evidence, Queue/Current and proposal refinement only in S1. Any later source surface must be nominated by the ledger; no public ABI is eligible now. |
| Applicable Rules | NXVM guide; Execution, Architecture, Coding and Documentation rules; NXVM goal/architecture/coding; source/research policy; component dependency direction and one-owner construction invariants. |
| Verification | Trace the concrete constructor, attachment, device, route and reset call graph; compare it with source-qualified PC110 port/device facts; classify every candidate as reusable, extraction-required, endpoint-owned or blocked; run documentation governance and link/whitespace review. |
| Expected Markers | One finite composition ledger, no hidden AT endpoint, one future construction owner, named source/test owner for every admitted mechanism, and an explicit next implementation cohort with bounded stop conditions. |
| Asset Needs | Read-only use of external PC110 manifests/research only. No raw ROM, media, generated byte source or executable is created. |
| Reporting Requirements | Record source grades, forced-side-effect inventory, accepted/excluded mechanisms, direct test strategy, proposed files and unexecuted guest/hardware proof. |
| Stop Conditions | A required composition needs a Shared contract, undocumented reset or controller semantics, conflicting source facts, or a device task’s proprietary behavior. Stop and transfer that row; do not substitute an AT behavior. |
| Exit Criteria | A reviewed convergence ledger and implementation-ready next S plan exhaust S1’s composition audit universe; docs checks pass and the audit P is pushed. |
| Original Owner Request | Continue implementing PC110 after completing the preceding T. |
| Similar-Issue Sweep | Inspect all X86 NXVM board creation paths and four existing Apps for implicit dependencies that a PC110 extraction would accidentally reverse or duplicate. |

### S1 Delivery

The [minimal base-composition audit](../etc/evidence/t566-s1-base-composition-audit.md)
traces the current `x86-nxvm-board-base` constructor and separates its reusable
PIC/PIT/DMA/optional-RTC substrate from its forced PC/AT endpoints.  It
authorizes the smallest next cohort: explicit no-endpoint selections for
keyboard, display and reset/A20, preserving the existing values and behavior
for all four shipped PC Apps.  PC110 remains App-local and does not yet claim
an executable, POST, input, panel, storage, power or RTC image.

Documentation governance and whitespace review pass.  The actual diff is
limited to T566 admission, its queue disposition, this packet, the proposal
clarification and the finite evidence ledger; it contains no source, asset or
artifact claim.  S1 is closed; T566 continues with its bounded source
implementation cohort.

## M5 T565 S1 — Closed: PC110 board and firmware admission baseline

| Field | Required record |
| --- | --- |
| Identifier Mode | New M5 T565 from the first admitted Queue candidate; one session performs coordinator and executor roles. |
| Admission And Approval | Owner explicitly admitted the Queue head on 2026-10-23. The approved task is `m5-pc110-board.md`; S1 is the proposal's bounded construction/mapping prerequisite. |
| Objective | Freeze the exact PC110 board/firmware input batch and the concrete App/Core construction boundary required before implementing board, chipset or ROM mapping. Produce the coverage ledger and admission disposition for BRD-1--3/SIO-1--3 without claiming a bootable machine. |
| Non-goals | No production chip/device implementation, no guessed reset constants, no external ROM acquisition, no firmware patch/application, no PC110 EXE, no INI/runtime model selector, no guest boot claim, no Shared-six change and no modification to the existing four PC Apps. |
| Reference Baseline | T558 research delivery `7ea9084fe`, the PC110 programme and receiver checklists, the current four-App/Core source map, and the external BYOB asset/provenance policy. T559/T563 provide the closed 486 prerequisites; this task does not requalify them. |
| Candidate Proposal | [PC110: board, chipset and firmware mapping](../history/M5-T565-pc110-board-proposal.md), consuming its first construction/mapping boundary and the programme's board rows. |
| Files And ABI Surface | Initially NXVM documentation/evidence only: `docs/nxvm/states/CURRENT.md` and new PC110 evidence records. Any later production S must explicitly name its App/Core surface after this S freezes it. No public ABI is eligible in S1. |
| Applicable Rules | NXVM guide; Execution, Architecture, Coding and Documentation rules; NXVM architecture/coding authorities; source/research policy; PC110 programme, board proposal, T558 S1 ownership audit, T558 S2 source-availability record and receiver checklists. Evidence must distinguish original manual, observed dump, model/reference and unresolved fact. |
| Verification | Inventory current Core/App capabilities and direct consumers; verify all cited asset/manuscript identities against the external asset index without copying protected payloads; create a finite board/firmware ledger with owner, evidence grade, reset/decode status, direct/composed proof and next receiver for each required row; run documentation governance and link/path checks. |
| Expected Markers | One declared PC110 construction root, two build-fixed DSTN/TFT firmware pairings, no runtime model selection, no second SMSC/configuration owner, and explicit bounded dispositions for unknown EC/ED, 4Fh, ROM/shadow and panel/firmware facts. |
| Asset Needs | Read-only inspection of owner-managed `O:\repos.hobby\nxvm-assets` and pinned research references only. No ROM/media/manual payload is copied into the repository, emitted to an artifact or modified. |
| Reporting Requirements | Record the complete S1 ledger batch, source grades and hashes/references where available; distinguish facts from unresolved choices; name every later receiver and block condition; report no source/artifact claim and all unexecuted hardware/guest validation. |
| Stop Conditions | Missing lawful asset/provenance, unresolved board/firmware identity needed to select an implementation owner, required Shared-six change, conflicting primary evidence, or any demand to make an undocumented fallback succeed. Stop the affected batch and report/transfer it rather than guessing. |
| Exit Criteria | A reviewed, finite construction/firmware/board ledger assigns each consumed BRD/SIO row to direct proof, an explicit later receiver, or a named blocking decision; no hidden AT-layout assumption remains; documentation gates pass and the admission record is pushed. |
| Original Owner Request | "准入队列首位T任务". |
| Similar-Issue Sweep | Review all current PC App construction/firmware declarations for an accidental AT-layout dependency or duplicate ROM/config owner; classify, do not modify, any unrelated issue. |

### S1 Delivery

The [board and firmware admission ledger](../etc/evidence/t565-s1-pc110-board-admission-ledger.md)
reconfirms all three research revisions, candidate hashes and the absence of a
PC110 input set in the approved external asset root.  It assigns the consumed
BRD/SIO rows to explicit Board, Storage, Display, Input/Power or Firmware
receivers.  No source, ROM, media, INI or artifact changed; no firmware or boot
claim is made.

Coordinator actual-change review accepts S1 P1 `52d83344b`: it changes only
the Queue, active packet and a bounded evidence ledger. The reviewed ledger
preserves the external ROM/license boundary, names the unresolved reset and
firmware choices, and does not create a hidden placeholder implementation.
Documentation governance and whitespace checks pass. S1 is closed; T565
remains open for its next bounded board-evidence receiver.

## M5 T565 S2 — Closed: PC110 construction, reset and decode contract

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation of open M5 T565 after accepted S1; one session performs coordinator and executor roles. |
| Admission And Approval | Owner approved automatic in-scope S admission for the admitted Queue-head task. S1's ledger explicitly transfers the construction/reset/decode batch here. |
| Objective | Reconcile the exact existing Core construction/lifetime route with PC110 board evidence, and produce a minimal construction/reset/decode contract that states what can be implemented next, what must remain App-local, and which unresolved facts block a behavior. |
| Non-goals | No production code, App directory, public API, ROM copy/embed/patch, executable, reset-value guess, synthesized chipset, QEMU/EMU source import, BIOS interception or guest boot claim. |
| Reference Baseline | T565 S1 ledger; T558 ownership/source-availability/checklist records; current Core and four-App construction paths; the three pinned research checkouts read only. |
| Candidate Proposal | [PC110: board, chipset and firmware mapping](../history/M5-T565-pc110-board-proposal.md), specifically BRD-1--3 and SIO-1--3 construction/mapping preparation. |
| Files And ABI Surface | Documentation/evidence only during S2. The result may nominate later `src/app-mypc110` and `test/app-mypc110` files but creates neither now. No public ABI is in scope. |
| Applicable Rules | NXVM guide; Execution, Architecture, Coding and Documentation rules; NXVM architecture/coding; source/research policy; T558 source availability, ownership audit and receiver checklist. Existing Core must remain the sole CPU/executor/controller timeline owner. |
| Verification | Inspect every current constructor/reset/ROM-map call chain used by the four PC Apps; trace board evidence to named source pages/observations; classify each future PC110 operation as direct proof, L2 model candidate or blocked; verify no proposed owner requires a peer/App reverse dependency; run documentation governance and path/link review. |
| Expected Markers | One build-fixed composition root; a typed firmware-binding boundary rather than AT slot reuse; a reset-reason map with no inferred register defaults; one claimed decode owner for each port/window; explicit deferrals for panel/font/storage/input endpoints. |
| Asset Needs | Read-only external manual/research/ROM inspection only. No external payload crosses into the repository or approved asset archive. |
| Reporting Requirements | Provide an evidence table naming the current construction entry points, proposed later owner, source grade, exact unknown and direct/composed falsification test. State all blocked choices and unexecuted guest/desktop proof. |
| Stop Conditions | Any required PC110 behavior cannot be represented without an unapproved Shared-six/API change, primary/reconstructed evidence conflicts without a bounded L2 choice, or a firmware-dependent behavior lacks an admitted source input. Stop that row and transfer it instead of creating a placeholder. |
| Exit Criteria | A reviewed construction/reset/decode contract exhausts this S2 batch into implementation-ready rows or named blockers, maintains one owner/timeline and no invented firmware/device behavior, and passes documentation governance. |
| Original Owner Request | Admit and continue the first queued PC110 board/chipset/firmware mapping task. |
| Similar-Issue Sweep | Check the four current Apps' construction/reset/ROM mapping paths for assumptions that would accidentally become a PC110 requirement; record only actual reuse candidates and constraints. |

### S2 Delivery

The [construction, reset and decode contract](../etc/evidence/t565-s2-pc110-construction-contract.md)
confirms one legal future PC110 composition route: an App-local typed board and
firmware binding publishes through the neutral `x86-nxvm` construction path,
while `x86-core` remains the sole executor, reset and ROM-map owner. The
existing AT preparation helper is explicitly excluded. Each BRD/SIO row now
has one later owner, evidence grade, falsification test and named blocker;
unknown chipset/reset facts remain blocked rather than inferred. The owner has
now admitted the local PC110-EMU TFT candidates into the external asset archive;
only their future typed binding/build validation remains, while stock DSTN
firmware is still unavailable.
No repository source, executable or runtime behavior changed. Documentation
governance and whitespace checks are the only applicable verification; guest,
desktop and hardware validation are not claimed. Coordinator review accepts the
bounded contract and asset-ledger update. S2 is closed; T565 remains open.

## M5 T565 S3 — Closed: VL82C420 configuration mechanism

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation of open M5 T565; S3 follows closed S1--S2. |
| Admission And Approval | Owner directed that T565 continue through closure. Earlier standing approval admits bounded in-scope S work automatically. |
| Objective | Extract the source-backed, address-neutral VL82C420 configuration mechanism into X86 Core: four-read unlock/relock, indexed banks, warm-reset preservation and opaque register storage. Establish only the mechanism required for later PC110 board wiring. |
| Non-goals | No port registration, App executable, firmware boot, guessed DRAM bit meaning, ROM-shadow behavior, QEMU/EMU source import, synthetic successful POST, display/storage/input implementation or Shared Lib/Emulator change. |
| Reference Baseline | T565 S1/S2 records; Open-Source-PC110 Chipset sections 13j/13h and checked live-dump/config files; PC110-QEMU/EMU behavior is L2 comparison only. |
| Files And ABI Surface | New `src/x86-core/chips/vl82c420/*` and matching `test/x86-core/chips/vl82c420/*`, Core CMake/manifest and task evidence only. Its interface must remain port/address neutral; later App wiring owns actual I/O decode. |
| Applicable Rules | NXVM guide; Architecture, Coding, Execution and Documentation rules; source/research policy; X86 Core dependency direction and L2/L3 evidence rules. |
| Verification | Direct Core tests cover fresh state, four exact unlock reads, gated/ungated index-data access, relock, independent bank retention and warm reset. Run affected Core x64/x86 unit suites and the complete repository unit suite before closure; no artifact refresh without a runnable product-input change. |
| Expected Markers | One chip state owner, no guest-IP shortcut, no board port constants in Core, no duplicate config bank, and explicit L2 labeling for any behavior not directly observed. |
| Asset Needs | Read-only use of the admitted external TFT asset manifest and research checkouts. No firmware bytes enter this repository or a product artifact. |
| Reporting Requirements | Record primary/reference evidence, exact interface and behavior table, test boundaries, actual diff and unexecuted guest/hardware proof. |
| Stop Conditions | A required operation cannot be stated without a guessed reset/default or requires a Lib/Emulator contract; stop that operation and retain it for the App wiring/firmware receiver. |
| Exit Criteria | One tested, address-neutral configuration mechanism with source-grade labeling, no PC110 App port decode or firmware claim, required Core/repository units and static/doc gates passed, reviewed commit pushed. |
| Original Owner Request | Continue the entire admitted PC110 board task through closure. |
| Similar-Issue Sweep | Inspect existing Core chips for a matching indexed/unlock mechanism; reuse only a semantic match, not a board-specific port facade. |

### S3 Delivery

`x86-core-vl82c420` now owns the address-neutral SCAMP, Block2 and EC/ED
indexed storage mechanism. Its only gate is the source-backed four-read
Block2 unlock sequence; actual I/O addresses, register semantics, cold-reset
images and PC110 wiring remain outside this component. The Core test proves
the exact unlock order, relock, SCAMP bit-seven alias, bank independence and
warm retention. Focused x64/x86 builds and tests pass. The two complete
parallel repository unit executions each reached all 653 cases; their only
failures were pre-existing timeout-sensitive Lib/Emulator self-checks under
load, and each failed case passed when immediately rerun alone. No product
input changed, so no executable artifact refresh is applicable. S3 is closed;
T565 remains open for the App-owned PC110 board binding.

## M5 T565 S4 — Closed: PC110 composition inventory and proven base routes

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation of open M5 T565 after closed S3; one session performs coordinator and executor roles. |
| Admission And Approval | Owner directed T565 to continue to closure and standing approval admits bounded in-scope S work. |
| Objective | Turn the S1/S2 board ledger into a concrete, evidence-backed PC110 composition inventory. Reuse only neutral X86 Core/X86 NXVM construction and controller facilities whose ownership and route facts match the PC110; implement direct, isolated route tests only where a fully proven board endpoint can be attached without inventing configuration defaults. |
| Non-goals | No AT profile reuse, executable, firmware boot claim, ROM shadow model, guessed chipset/SIO values, display/font/storage/input/power endpoint, QEMU/EMU source import, Shared Lib/Emulator change or App-to-App dependency. |
| Reference Baseline | T565 S1/S2/S3 evidence; PC110 board/chipset source ledger; current `x86-nxvm` board/controller construction and four App plans. |
| Files And ABI Surface | Evidence, task packet and, only if a direct verified route exists, App-local `src/app-mypc110`/`test/app-mypc110` construction files plus an existing neutral X86 NXVM construction receiver. No Core API change is presumed. |
| Applicable Rules | NXVM guide; Execution, Architecture, Coding and Documentation rules; source/research policy; Core/X86 NXVM dependency direction and source-grade policy. |
| Verification | Build a per-endpoint reuse/disposition ledger, prove all nominated route owners from current code and primary/reconstructed evidence, run direct tests for any admitted implementation, and run appropriate x64/x86 units/static/manifest gates. |
| Expected Markers | One future PC110 plan/binding root, one route owner per admitted endpoint, no AT descriptor/slot reuse, no duplicate config bank and a named receiver/blocker for every non-admitted endpoint. |
| Asset Needs | Read-only use of owner-managed PC110 TFT manifest and research checkouts. No external firmware enters Git or an executable during this S. |
| Reporting Requirements | Record actual route evidence, retained/blocked endpoints, source grades, exact diff and all missing guest/desktop/hardware proof. |
| Stop Conditions | Any proposed route needs an unqualified reset/decode default, an AT facade, a Shared contract, or an unresolved firmware identity. Stop that route rather than modeling a plausible substitute. |
| Exit Criteria | Every PC110 base endpoint is either attached through a tested single owner or explicitly retained for its named later receiver; no hidden AT assumption or firmware claim; required gates pass and the scoped result is pushed. |
| Original Owner Request | Continue the complete admitted PC110 board/chipset/firmware mapping task to closure. |
| Similar-Issue Sweep | Compare each candidate route with every existing four-App plan to catch accidental descriptor, firmware-slot or controller-state reuse. |

### S4 Delivery

The [composition inventory](../etc/evidence/t565-s4-pc110-composition-inventory.md)
proves the narrow reusable construction surfaces and rejects the tempting but
incorrect AT-board shortcut. It records three directly attributable VL82C420
configuration windows, their owner boundary and every endpoint still blocked
on PC110-specific facts. No production input or asset changed; x64/x86 S3
Core route tests remain the applicable executable verification. S4 is closed.

## M5 T565 S5 — Closed: PC110 App-local firmware and configuration-route binding

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation of open M5 T565 after closed S4; one session performs coordinator and executor roles. |
| Admission And Approval | Standing owner approval admits bounded in-scope work; S4 names this as the first legal production receiver. |
| Objective | Add one App-local, non-runnable PC110 construction foundation: typed immutable firmware binding plus a tested configuration-route adapter that delegates to S3's VL82C420 mechanism. |
| Non-goals | No executable, firmware embedding, DSTN fallback, guest boot, AT preparation, reset-default table, ROM-shadow behavior, display/storage/input/power endpoint, Shared Lib/Emulator change or external-source import. |
| Files And ABI Surface | New `src/app-mypc110` and matching `test/app-mypc110` foundation files, build/test registration, manifests and PC110 evidence only. Existing public Shared interfaces remain unchanged. |
| Verification | x64/x86 direct route and firmware-map units; App/Core dependency, manifest and static gates. No artifact publication because no runnable App input is admitted. |
| Exit Criteria | One App-local construction/route owner proves its exact delegation and rollback behavior, has no AT or sibling-App dependency, and leaves every unknown endpoint explicitly absent. |

### S5 Delivery

`app-mypc110` now contains the one admitted configuration-route adapter and
matching unit root. The [S5 record](../etc/evidence/t565-s5-pc110-config-route-binding.md)
captures the exact four delegated routes, S3 ownership, omitted Block2 reason
and Core lifecycle proof. x64/x86 CMake configuration, build and direct route
test pass. No ROM is embedded, no executable is produced, and no boot claim is
made. S5 is closed.

## M5 T565 S6 — Closed: PC110 TFT firmware binding

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation of T565 after closed S5. |
| Objective | Add the typed TFT firmware binding at the qualified immutable BIOS range, without giving an incomplete board a false construction/publication path. |
| Non-goals | No asset-path loading, manifest parsing, DSTN substitution, executable publication, firmware boot claim, AT profile helper, fabricated reset/shadow defaults, display/storage/input/power endpoint or Shared change. |
| Required Proof | The binding accepts only an exact TFT image; Core owns the copied mapping; null/wrong-size input rejects and the ordinary firmware reset path remains valid. |
| Stop Conditions | Any need to infer ROM aliases/shadow/reset data, load a machine-local asset at runtime, or publish an empty board construction stops that behavior and transfers it to the appropriate endpoint receiver. |

### S6 Delivery

`app-mypc110` now provides a construction-only `mypc110_tft_firmware` binding.
It accepts one exact 256 KiB TFT BIOS byte image and publishes only the
evidence-supported `000C0000h..000FFFFFh` immutable mapping through the
existing Core firmware contract.  Core copies the bytes while binding; no
external path, manifest, asset or executable enters this delivery.  The
[S6 evidence](../etc/evidence/t565-s6-pc110-tft-firmware-binding.md) records
the L2 source basis and the deliberately unresolved behavior.

Both Ninja/ccache widths configure, build and pass the direct PC110
configuration-route and firmware-binding tests.  A source-qualified complete
board composition is still required before X86 NXVM construction publication;
S6 correctly does not emit a fake empty machine plan.  S6 is closed and T565
continues with the remaining board-composition receivers.

## M5 T565 S7 — Closed: PC110 Block2 configuration gate

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation of T565 after closed S1--S6. |
| Objective | Bind the source-backed PC110 Block2 port gate to the existing address-neutral VL82C420 protocol without adding a second unlock state or exposing a Core-internal failure to the guest. |
| Non-goals | No reset-image inference, board construction publication, BIOS boot, ROM shadow model, endpoint implementation, external asset loading, executable publication, or Shared Lib/Emulator change. |
| Files And ABI Surface | App-local PC110 config-route adapter, direct App test, manifest and evidence only. The existing VL82C420 public contract is reused unchanged. |
| Required Proof | Closed reads return `FFh`; an out-of-order read sequence remains closed; the exact `FC23h/F023h/C023h/0023h` read sequence enables indexed Block2 access; a `22h` bit-8 write relocks it. |
| Stop Conditions | Any need for undocumented Block2 reset contents, ROM/shadow semantics or a generic PC/AT construction route transfers that behavior to a later receiver. |

The route adapter maps only the physical PC110 I/O addresses.  `x86_vl82c420`
remains the sole owner of unlock progress and lock state; the adapter forwards
the four observation reads and the relock write.  It translates the chip's
closed-state result to the observed guest-facing `FFh`/ignored-write gate,
rather than duplicating protocol state.  The direct route test passed on both
established Ninja/ccache widths together with the existing TFT firmware-binding
test.  This verifies a configuration path only, not a bootable PC110.

## M5 T565 S8 — Closed: PC110 board mapping closure audit

| Field | Required record |
| --- | --- |
| Identifier Mode | Final bounded S of M5 T565 after S1--S7. |
| Objective | Reconcile the original board/mapping universe against actual source and construction owners, close only the accepted mapping foundation, and explicitly transfer every remaining controller/device/boot item. |
| Non-goals | No production change, board publication, executable, external asset mutation, boot claim or Shared change. |
| Required Proof | Every in-scope row is either accepted with a direct owner/proof or transferred to a named Queue receiver; no existing AT assembly is silently reused as PC110. |
| Exit Criteria | Closure audit, Queue transfer and task history preserve all residual work; worktree remains clean and earlier committed direct dual-width proof is linked. |

The S8 audit transfers the unresolved standard-controller construction root to
the explicit `m5-pc110-base-composition.md` candidate, and retains endpoint and
firmware/guest responsibilities at their existing PC110 receiver proposals.
T565 is closed as a mapping foundation only; it does not claim complete board
or guest qualification.  See the closure history and S8 evidence for the full
disposition.

## M5 T564 — Closed: public-component qualification

The owner accepted final qualification on 2026-10-23.  S15 retained the
modal/frame-scale integration test while returning host mouse-capture proof to
its dedicated Unit owner.  Clean serial qualification passes: x64 and x86
unit suites each pass 697/697, and x64 and x86 integration suites each pass
21/21.  The [S15 evidence](../etc/evidence/t564-s15-window-modal-test-ownership.md)
records the RDP `ClipCursor` failure, retained coverage, no-artifact decision
and exact results.  `ad5c532a8` records the final target-scoped repair and
qualification; no executable input changed.

## M5 T564 S15 — Closed: Window modal integration test ownership correction

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation of open T564; S1--S14 concluded under its declared stop condition; single-session coordinator and executor. |
| Admission And Approval | Owner approved the minimal repair after diagnosis: keep the modal/frame-scale integration test, remove only its unrelated real mouse-capture exercise. |
| Objective | Restore environment-independent ownership of the native Window modal/frame-scale integration test without changing Window production behavior or weakening the actual modal/geometry proof. |
| Non-goals | No Lib production/API change; no mouse-capture semantic change; no removal or reclassification of the integration test; no artifact refresh without production inputs. |
| Reference Baseline | S14 clean x64 integration run: 20/21 passed; `lib.integration/kvm_window_modal` reproducibly failed only after its synthetic `WM_LBUTTONDOWN` attempted RDP-restricted `ClipCursor` (`ERROR_ACCESS_DENIED`). Existing `kvm_window_capture_contract_smoke` covers capture/release outcomes with controlled native-call fixtures. |
| Candidate Proposal | [T564 public-component quality program](../proposals/m5-softpc-rgb-frame-import.md), final qualification requirement. |
| Files And ABI Surface | `test/lib/integration/kvm_window_modal_smoke.c`, its manifest, task evidence/state only. No production source, public header, CMake registration or receiver App input changes. |
| Applicable Rules | NXVM guide; Execution, Architecture, Coding and Documentation rules; strict test ownership, no assertion weakening and one production path. |
| Verification | Run the focused modal integration and capture-contract unit test on x64/x86; then run clean serial repository unit and integration suites on both widths, static/manifest/documentation gates, and inspect no EXE input changed. |
| Expected Markers | Modal test still proves real Window frame scaling, all sizing edges, maximize/restore, native modal-loop admission, freeze/title control and teardown; capture remains proved only by its dedicated contract test; both qualification suites pass. |
| Asset Needs | Reuse stable Ninja/ccache graphs; preserve all INIs, ROMs, media, snapshots and released binaries. No EXE refresh is expected because the executable input graph is unchanged. |
| Reporting Requirements | Record the native failure evidence, the retained and removed assertions, per-width focused/full results, manifest state, actual diff and unchanged artifact determination. |
| Stop Conditions | Any failure outside the removed host-policy capture segment, a need to alter production capture behavior/API, unexpected artifact mutation, or a failure in a required qualification suite. |
| Exit Criteria | Narrow test correction reviewed; required tests and governance pass; target-scoped commit is pushed; final qualification is truthfully recorded for owner acceptance. |
| Original Owner Request | Approve this correction, then compile, test, commit and push according to the established process. |
| Similar-Issue Sweep | Confirm no other modal/frame-scale integration test invokes host mouse capture; retain all dedicated capture-contract coverage. |

### S15 Delivery

The integration test continues to exercise its native Window/modal behavior;
only its unrelated host-policy-sensitive capture branch was removed.  The
dedicated capture contract continues to cover capture and release semantics.
Focused x64/x86 tests, full x64/x86 unit suites and full x64/x86 integration
suites pass.  No production or artifact input changed.  Commit and push record
this delivery; T564 awaits owner review rather than claiming closure.

## M5 T564 S14 — Closed: qualification stop-condition diagnosis

S14 ran the x64 serial integration suite. Twenty of twenty-one tests passed;
the remaining `lib.integration/kvm_window_modal` failure was reproduced alone.
Debugger evidence identifies `LIB_STATUS_IO_ERROR` from its synthetic
mouse-capture branch, while Win32 reports `ERROR_ACCESS_DENIED` from the RDP
desktop's `ClipCursor` policy. The test's stated scope is modal-loop and
frame-scale behavior; its real capture assertion duplicates the dedicated
`kvm_window_capture_contract_smoke` unit suite. No production defect or
receiver-App fault was found. The declared stop condition therefore admitted
the owner-approved narrow S15 correction; S14 does not claim T-level
qualification or closure.

## M5 T564 S13 — Closed: X86 NXVM verifier-negative efficiency and four-PC App test ownership

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation of open T564; S1--S12 closed; single-session coordinator and executor. |
| Admission And Approval | Owner accepts S12's no-repair audit disposition and directs admission of the next planned S. |
| Objective | Retain the effective X86 NXVM negative boundary assertions while auditing every four-PC App test use of X86 NXVM private headers; move or adapt only tests whose real behavior belongs to an inner owner. |
| Non-goals | No raw production getter, copied private layout, test-only public API, assertion weakening, qualification regression, production-path change or unrelated Shared work. |
| Reference Baseline | Accepted S7 repair and S8--S12 audit dispositions. |
| Candidate Proposal | T564 public-component quality program, S13; [PC App test adapter encapsulation](../proposals/m5-pc-app-test-encapsulation.md). |
| Files And ABI Surface | `test/app-my5160`, `test/app-my5170`, `test/app-mydeskpro386`, `test/app-nxvm`, and their CMake registration; X86 NXVM test owners only if a classified assertion belongs there. Production interfaces remain unchanged unless evidence demonstrates no existing public operation can prove an App-owned behavior. |
| Applicable Rules | NXVM guide; Execution, Architecture, Coding and Documentation rules; strict inward test ownership, no private-layout facade, one production path and no test-only compatibility contract. |
| Verification | Inventory every private-header include and classify it as hardware observation, inner-component proof, actual App integration or fault injection; preserve assertion meaning, run focused x64/x86 tests, and expand only for actual moves. |
| Expected Markers | Complete use ledger, per-use disposition, retained negative assertions, no new raw getter/layout mirror and evidence that moved tests retain the original behavioral proof. |
| Asset Needs | Reuse stable Ninja/ccache trees; preserve INIs, media, ROMs, snapshots, existing artifacts and unrelated work. |
| Reporting Requirements | State each private-header use and its owner, distinguish a safe test-local fixture from an illegal cross-owner dependency, report actual diff/test results and unexecuted integration or desktop work. |
| Stop Conditions | A required production API, a behavior that cannot be observed through an existing owner boundary, a proposed assertion weakening, or an unrelated Shared/App change. |
| Exit Criteria | Every scoped use has a documented disposition; any justified move/adaptation preserves coverage; required tests, manifests and governance pass; reviewed commits/push and owner acceptance. |
| Original Owner Request | Close S12 and admit the next S under the established public-component program. |
| Similar-Issue Sweep | Search all four PC App test roots and CMake registrations for equivalent private-header or private-layout access; classify every hit rather than repairing only the first include. |

### S13 Accepted Delivery

The [S13 ownership audit](../etc/evidence/t564-s13-test-ownership-audit.md)
classifies all 81 four-PC-App private Machine-header uses.  Every use is an
App-selected hardware, firmware, media, integration or explicit diagnostic
observation; none is a generic X86 NXVM behavior misplaced beneath an App.
No raw getter, test-only public API, layout mirror or assertion weakening is
warranted.  The X86 NXVM negative verifier now reuses one restored corpus per
probe family and independently resets each nested-CMake bypass case, preserving
all assertions while eliminating repeated full-corpus copies.  Focused tests
pass on x64 and x86, and the clean serial repository unit suite passes 697/697
on both widths.  No executable is refreshed because no production input changed.
The owner accepted this delivery.  Shared P1 `dcc8e98ca` contains the verifier
change; NXVM P2 `a864cf7fd` contains the evidence and queue receiver.  S13 is
closed; S14 owns final integration qualification and T564 closure preparation.

## M5 T564 S12 — Closed: X86 NXVM peripheral and timing-boundary audit

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation of open T564; S1--S11 closed; single-session coordinator and executor. |
| Admission And Approval | Owner accepts S11's no-repair audit disposition and directs admission of S12. |
| Objective | Inventory selected X86 NXVM board peripheral requirements and beyond-digital timing boundaries; repair only a reproduced gap in an already selected device, otherwise explicitly defer unselected or analogue capability. |
| Non-goals | No speculative device, analog model, second timing mechanism, product profile expansion, unrelated Shared change or weakened regression. |
| Reference Baseline | Accepted S7 implementation and accepted S8--S11 audit dispositions. |
| Candidate Proposal | T564 public-component quality program, S12. |
| Files And ABI Surface | X86 NXVM chips, board and Machine routes plus direct tests only where actual selected-board evidence requires; manifests and records only for approved repairs. |
| Applicable Rules | NXVM guide; Execution, Architecture, Coding and Documentation rules; source policy before external research; inward dependency and one Core guest-time owner. |
| Verification | Establish profile/device ownership and timing-evidence ledger before modifying code; reproduce a selected-device defect before repair; focused x64/x86 owner and receiver tests, expanded only for actual changes. |
| Expected Markers | Selected-device inventory, evidence grades, existing owner/test map, explicit unselected/analog dispositions or a minimal owner-local repair design. |
| Asset Needs | Reuse existing Ninja and ccache trees; preserve owner configuration, ROMs, media and unrelated work. |
| Reporting Requirements | Separate digital behavioral correctness from physical/analog timing claims; name any public surface precisely; do not call an inventory full hardware qualification. |
| Stop Conditions | Unapproved new device/profile/API, protected/external source need without policy admission, unproven hardware-model claim, unresolved safety issue or unrelated change. |
| Exit Criteria | Required audit, approved necessary repairs, verification, reviewed commits/push and owner acceptance. |
| Original Owner Request | Close S11 and admit the next S for brief, audit and design. |
| Similar-Issue Sweep | Review equivalent selected-board peripheral/timing consumers without making a second clock, signal route or board owner. |

Owner accepted the S12 no-repair audit.  The [peripheral and timing-boundary audit](../etc/evidence/t564-s12-peripheral-timing-audit.md) traces all four selected PC profiles through the complete copied Board timing table into the one Core deadline owner.  It found no selected-device defect and no basis for a speculative model; 11 focused X86 NXVM profile/Board/chip tests passed on each architecture.  No source, test, ABI or artifact change was warranted.  S12 is closed and S13 is active.

## M5 T564 S11 — Closed: X86 Core x87 and virtual-interrupt audit

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation of open T564; S1--S10 closed; single-session coordinator and executor. |
| Admission And Approval | Owner accepts S10's no-repair audit disposition, directs S11, then accepts its no-repair disposition and directs S12. |
| Objective | Establish the selected CPU/FPU profiles and source-backed coverage for X86 Core x87 execution and virtual-interrupt extensions; qualify current claimed behavior and explicitly disposition unselected variants. |
| Non-goals | No speculative hardware implementation, profile expansion, timing claim beyond evidence, duplicate interrupt route, unrelated Shared change or weakened regression. |
| Reference Baseline | S7 delivery and accepted S8--S10 public-component audits. |
| Candidate Proposal | T564 public-component quality program, S11. |
| Files And ABI Surface | X86 Core CPU/FPU/Machine and direct tests only where an actual selected-profile contract requires them; manifests and records only for approved repairs. |
| Applicable Rules | NXVM guide; Execution, Architecture, Coding and Documentation rules; source policy before external research; inward dependencies and evidence-tiered hardware claims. |
| Verification | First inventory selected profiles, current execution paths and existing regressions; reproduce any claimed gap before modification; use focused x64/x86 tests and expand verification only with actual changes. |
| Expected Markers | Profile/feature ledger, evidence grades, coverage ownership, no-repair dispositions or an owner-local repair design with actual test plan. |
| Asset Needs | Reuse existing Ninja and ccache trees; preserve owner configuration, media and unrelated work. |
| Reporting Requirements | Separate implemented selected behavior from unselected variants and from source evidence; identify any proposed public surface precisely; do not present an audit as universal x87 qualification. |
| Stop Conditions | Unapproved public ABI or profile expansion, protected/external source need without policy admission, unproven hardware model claim, unresolved safety issue or unrelated change. |
| Exit Criteria | Required audit, approved necessary repairs, verification, reviewed commits/push and owner acceptance. |
| Original Owner Request | Close S10 and admit the next S for brief, audit and design. |
| Similar-Issue Sweep | Review x87 and virtual-interrupt owner paths across selected profiles without adding parallel interrupt, timing or FPU state. |

Owner accepted the S11 audit.  The [x87 and VME record](../etc/evidence/t564-s11-fpu-vme-audit.md) establishes that all four current PC Apps select no FPU, retains the one CPU/FPU/deadline boundary and verifies the selected VME condition without claiming broad x87 or virtual-interrupt completeness.  Sixteen focused tests passed on each architecture; no source, test, ABI or artifact change was warranted.  The complete x87 program is now a Queue candidate.  S11 is closed and S12 is active.

## M5 T564 S10 — Closed: X86 Core descriptor, Debug CR and CPU-fault audit

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation of open T564; S1--S9 closed; single-session coordinator and executor. |
| Admission And Approval | Owner accepts S9's no-repair disposition, directs S10, then accepts its no-repair disposition and directs S11. |
| Objective | Inventory X86 Core descriptor vocabulary, Debug control-register mutation and CPU fault reporting; reproduce any concrete gap and design only the owner-local repair. |
| Non-goals | No speculative CPU implementation, duplicate fault channel, raw private-machine access, unrelated Shared change or weakened regression. |
| Reference Baseline | S7 delivery and full qualification through 197076521; S8 and S9 accepted read-only dispositions. |
| Candidate Proposal | T564 public-component quality program, S10. |
| Files And ABI Surface | X86 Core CPU/Machine and direct X86 Debug/Emulator receivers only where actual callers require them; manifests and records only for approved repairs. |
| Applicable Rules | NXVM guide and source policy; Execution, Architecture, Coding and Documentation rules; inward dependencies, explicit timing/fault ownership and minimal public contracts. |
| Verification | Trace descriptors, CR mutation and fault facts through actual execution and receiving code; reproduce a concrete defect before modifying source; use focused x64/x86 tests and expand verification only with actual changes. |
| Expected Markers | Evidence-backed owner map, normal/failure-path disposition, test-ownership assessment, repair design and estimated diff. |
| Asset Needs | Reuse existing Ninja and ccache trees; preserve owner configuration, media and unrelated work. |
| Reporting Requirements | Distinguish source-confirmed behavior from hardware-evidence limits, identify any proposed public surface precisely, and report no-repair dispositions honestly. |
| Stop Conditions | Unapproved public ABI expansion, unproven hardware model claim, unresolved safety issue or unrelated change. |
| Exit Criteria | Required audit, approved necessary repairs, verification, reviewed commits/push and owner acceptance. |
| Original Owner Request | Close S9 and admit S10 for brief, audit and design. |
| Similar-Issue Sweep | Examine descriptor, CR and CPU-fault receivers without creating a second exception or status path. |

Owner accepted the S10 audit.  The [descriptor, Debug CR and fault record](../etc/evidence/t564-s10-core-debug-fault-audit.md) confirms one copied descriptor boundary, one paused raw-Debug register patch owner and one CPU-fault publication path.  Fourteen focused X86 Core/Debug/NXVM tests passed on each architecture; no source, test, ABI or artifact change was warranted.  S10 is closed and S11 is active.

## M5 T564 S9 — Closed: X86 IBM PC receiving-contract audit

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation of open T564; S1--S8 closed; single-session coordinator and executor. |
| Admission And Approval | Owner accepts S8 and requests S9 audit with a detailed report. |
| Objective | Audit X86 IBM PC receiving contracts and tests against the preceding Lib, Emulator and Debug dispositions; reproduce any suspected defects before proposing repairs. |
| Non-goals | No speculative defect, duplicate inner-component mechanism, unrelated Shared change or weakened test. |
| Reference Baseline | S7 delivery and full qualification through 197076521; S8 checkop audit and focused x86/x64 verification. |
| Candidate Proposal | T564 public-component quality program, S9. |
| Files And ABI Surface | X86 IBM PC Product, its tests and affected records; extend to receivers only if evidence requires. Review Shared changes before implementation. |
| Applicable Rules | NXVM guide and source policy; Execution, Architecture, Coding and Documentation rules; inward dependencies, minimal behavior and existing ownership. |
| Verification | Inspect actual call and failure contracts and direct tests; reproduce concrete gaps; focused x64/x86 component and receiver tests. Full repository units and affected integration and binaries follow actual implementation when required. |
| Expected Markers | Evidence-backed disposition of Lib, Emulator and Debug receiving paths, warning exceptions and test ownership. |
| Asset Needs | Reuse existing Ninja and ccache trees; preserve owner configuration, media and unrelated work. |
| Reporting Requirements | Detailed factual audit, normal and failure paths, real test coverage, proposed repairs and estimated diff; distinguish hypotheses and unverified behavior. |
| Stop Conditions | Unapproved Shared or public interface expansion, unresolved safety issue or unrelated change. |
| Exit Criteria | Required review and necessary approved repairs and verification complete; reviewed commits pushed and owner acceptance. |
| Original Owner Request | Close S8, admit S9 and audit its receiving contracts. |
| Similar-Issue Sweep | Review equivalent receiving paths without creating a second owner, redundant tests or speculative fixes. |

## M5 T564 S8 — Closed: X86 Debug assembler checkop audit

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation of open T564; S1--S7 closed; single-session coordinator and executor. |
| Admission And Approval | Owner directs S7 closure after cached full build, unit and integration qualification and admission of the next S. |
| Objective | Audit the assembler checkop path, its real callers and contracts, and establish whether a reproducible defect warrants repair. |
| Non-goals | No speculative reassembly framework, duplicate execution path, unrelated Shared changes or weakened tests. |
| Reference Baseline | S7 delivery through 366eb14b6 and its subsequent qualification record. |
| Candidate Proposal | T564 public-component quality program, S8. |
| Files And ABI Surface | X86 Debug assembler and existing tests only if justified; affected manifests, receivers and records as needed. Review any Shared interface expansion before implementing. |
| Applicable Rules | NXVM guide and source policy; Execution, Architecture, Coding and Documentation rules; inward dependencies and minimal behavior. |
| Verification | Reproduce any claimed defect before modifying code; extend existing tests only as warranted; full repository units on both architectures and relevant integration and receiver qualification for actual changes. |
| Expected Markers | Evidence-backed checkop disposition, real callers and retained or repaired behavior. |
| Asset Needs | Reuse current Ninja and ccache trees; preserve owner configuration and media. |
| Reporting Requirements | Explain findings, proposed scope and estimated diff before implementation; distinguish actual defects from assumptions. |
| Stop Conditions | Unapproved Shared API expansion, unresolved safety issue or unrelated change. |
| Exit Criteria | Evidence-backed disposition, necessary changes and verification completed, actual diff reviewed and applicable delivery pushed; await owner acceptance. |
| Original Owner Request | Close S7 after complete cached compilation and unit/integration tests; admit the next S. |
| Similar-Issue Sweep | Inspect comparable assembler operand checking and its existing test coverage without inventing duplicate machinery. |

Owner accepted the X86 IBM PC receiving-contract audit: X86 Debug's valid-result-on-output-failure
contract, the Machine closed-input result and the shared Window formatting contract all reach
their sole PC adapter correctly.  The direct adapters do not introduce a second lifecycle owner,
queue, parser or presentation path.  Twenty-five relevant Emulator/X86 IBM PC tests passed on
each architecture; no source, test or interface change was warranted.  S9 is closed and S10 is
active.

Owner accepted the read-only checkop audit: the historical optional runtime
disassemble/reassemble check is not present, no concrete current defect was
identified, and all 12 X86 Debug tests passed on each architecture. No source,
test or interface changes were warranted. This disposition does not qualify
all x86 instructions or close the other planned public-component audits. S8
is closed and S9 is active.

## M5 T564 S7 — Closed: Emulator Machine wake failure contracts

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation of open T564; S1--S6 closed; single-session coordinator and executor. |
| Admission And Approval | Owner accepted S6 and admitted the next S on 2026-10-22; Shared changes remain subject to the established review boundary. |
| Objective | Audit Emulator Machine asynchronous wake and event-operation failure handling for lifecycle, input, debug cancellation and shutdown; correct proven defects under the existing contract. |
| Non-goals | No speculative fix, duplicate queue, recovery thread, new framework, unrelated product change, Linux or browser qualification. |
| Reference Baseline | NXVM 9c455da4d and accepted T564 S6 evidence. |
| Candidate Proposal | T564 public-component quality program, S7; inspect current code and historical evidence before deciding whether a defect exists. |
| Files And ABI Surface | Emulator Machine and its tests if justified; relevant manifests and receiver tests only as needed. No public ABI expansion assumed; review Shared changes before implementation. |
| Applicable Rules | NXVM guide and source policy; Execution, Architecture, Coding and Documentation rules; inward dependencies, single state owner, minimal failure handling and honest coverage. |
| Verification | Inspect all scoped paths and existing failure tests; inject failures only where a real gap justifies them; x64/x86 full repository units, relevant integrations and static gates; qualify receivers and rebuild affected binaries if production changes. |
| Expected Markers | Evidence-backed disposition of accepted work, wake failure, pending ownership, cancellation and shutdown paths; no false success or duplicate delivery and no weakened assertions. |
| Asset Needs | Existing Ninja/ccache graphs and disposable test data; preserve owner INI, media and unrelated work. |
| Reporting Requirements | Begin with an evidence-based brief; distinguish proven failure from hypothesis and report proposed scope, actual diff, tests, desktop limitations and affected artifact identities. |
| Stop Conditions | Unresolved safety failure, unapproved Shared or public API expansion, external change, or inability to preserve existing behavior. |
| Exit Criteria | Scoped wake paths and Emulator strict-compilation exceptions dispositioned with evidence; justified repairs and tests complete; required review, verification and affected artifacts committed and pushed; owner acceptance. |
| Original Owner Request | Close S6, admit the next S and first provide its task brief. |
| Similar-Issue Sweep | Inspect event signal, reset and wait results across lifecycle, input, debug cancellation and shutdown, distinguishing synchronous accepted work from actual failures. |

S7's bounded Machine and Session repair and its verification are recorded in
the [event failure review](../etc/evidence/t564-s7-event-failure-review.md).
The owner accepted leaving simultaneous native synchronization failures as a
documented limitation rather than adding recovery machinery. Following the
owner's closure direction, both cached builds, full dual-width units and the
four-profile integration matrix were rerun; the evidence preserves the initial
native desktop failure and successful real-session retry. S7 is closed; T564
remains open and S8 is active.

The owner extended open T564 on 2026-10-22 to plan the public-component TODO
classes and former PC App test-encapsulation candidate as S6--S13. The
[updated proposal](../proposals/m5-softpc-rgb-frame-import.md) owns the planned
sequence and conditional dispositions, reordered by owner direction as Lib,
Emulator, X86 Debug, X86 IBM PC, X86 Core, then X86 NXVM. S1--S5 remain closed;
The owner accepted and closed S6 on 2026-10-22; its [evidence](../etc/evidence/t564-s6-lib-console-review.md) records the audit and verification. S14 is active. Model40 throughput, DevBox exploration,
Linux runtime verification and future browser storage remain in NXVM TODO;
the latter two are excluded from T564 by owner direction. T564 is open.

M5 Td S183 is closed: the owner directed correction of the two Shared documentation
items omitted from Td S182. Shared documentation rules now identify the six
current shared source/test package pairs, and Execution records the four PC
Apps' lowercase product-specific EXE/INI layout. These are documentation-only
corrections; no binaries, INIs or source code change. The two entries are
removed from NXVM TODO; documentation governance checks passed for NXVM,
MyNES and MyRaidenII. Shared delivery is `004169618`. T564 remains open.

## M5 Td S182 — Closed: NXVM documentation governance

| Field | Required record |
| --- | --- |
| Identifier Mode | Governance, next NXVM M5 Td S182; single-session coordinator and executor. T564 remains open. |
| Admission And Approval | Owner requests S5 closure and a new NXVM documentation governance Td on 2026-10-22. |
| Objective | Audit current NXVM documentation and identify needed governance corrections. |
| Non-goals | No implementation, Shared or other product changes; no assumption that all TODO entries must be queued or deleted. |
| Reference Baseline | S5 delivery through abb2b10f2, followed by MyNES governance commit 14d12458a. |
| Candidate Proposal | Owner-directed documentation governance; detailed remediation scope remains to be established. |
| Files And ABI Surface | NXVM documentation only; no production, test, binary or public API changes. |
| Applicable Rules | NXVM guide and shared Execution and Documentation rules; preserve historical records and single authority. |
| Verification | Audit actual current documentation, links and state; run NXVM documentation governance gate and review actual changes. |
| Expected Markers | Clear findings and approved, consistent documentation disposition. |
| Asset Needs | No media, runtime assets or temporary build output. |
| Reporting Requirements | Distinguish verified stale descriptions, historical evidence and unapproved proposed changes. |
| Stop Conditions | Scope requiring Shared or another product, or an owner choice affecting task order or deletion. |
| Exit Criteria | Approved scope completed, checks and actual-diff review passed, target-scoped commit pushed. |
| Original Owner Request | Close preceding S and admit a Td to govern NXVM project documentation. |
| Similar-Issue Sweep | Review related current authorities and references; retain history rather than rewriting it. |

The [20-item audit](../etc/evidence/td-s182-todo-audit.md)
distinguishes verified current facts, historical observations and conditional
future work. The PC App test encapsulation audit is queued at the tail without
reordering existing candidates; the old verifier timeout statement is corrected.
Shared issues are not repaired here. Two existing candidate proposals now use
current App, component and INI names. Unproven historical issues remain explicitly
conditional; reproducing or fixing them is outside this documentation Td. The
owner's requested audit and queue disposition are complete; Td S182 is closed.

## M5 T564 S5 — Closed: verbatim SoftPC shared-package import

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation of open T564 after S4 audit; single-session coordinator and executor. S2 was subsequently closed by owner direction. |
| Admission And Approval | Owner authorizes verbatim import, build, release, testing, commit and push on 2026-10-22. |
| Objective | Import the committed SoftPC Lib, Emulator, X86 Debug and X86 IBM PC source/test pairs byte for byte and qualify affected products. |
| Non-goals | No sibling repository writes, uncommitted upstream import, user INI/media changes, or unrelated feature changes. |
| Reference Baseline | NXVM 0ee06e444; SoftPC 898e40546f6182cc0f6649c5dfaf8aab6b414d6e, whose eight roots match the S4 audited 9ee4af66. |
| Candidate Proposal | T564 shared import proposal and S4 audit evidence; owner adds this committed refresh. |
| Files And ABI Surface | Eight shared source/test roots and required NXVM/MyNES receiving build, tests and release EXEs; task records. Preserve unrelated changes. |
| Applicable Rules | NXVM guide and source policy; shared Execution, Architecture, Coding and Documentation rules; byte identity, inward dependency, test ownership and artifact scope apply. |
| Verification | Compare all eight trees to the committed source, check manifests and boundaries, build x64/x86 with reusable Ninja/cache, run all shared and affected product units and required integrations, verify released EXE identity and hashes. |
| Expected Markers | Eight matching roots; passing applicable double-width builds and tests; deployed current PC and MyNES EXEs with evidence. |
| Asset Needs | Existing reusable Ninja/ccache graphs and external owner-provided firmware and media; do not mutate owner INI or media. |
| Reporting Requirements | Record source commit, actual diff, result counts, binaries, omissions and actual-change review; distinguish desktop and RDP verification. |
| Stop Conditions | Unapproved external mutation, unresolvable ownership conflict, missing required asset or unexplained required test failure. |
| Exit Criteria | Complete scoped delivery and evidence pushed; await owner acceptance; T564 remains open. |
| Original Owner Request | Admit a new S to import the latest committed SoftPC eight components, build, publish, test, commit and push for acceptance. |
| Similar-Issue Sweep | Check changed Console/PCM worker termination, UI/Product ownership and all actual consumers for retained-resource assumptions. |

S4 audit was accepted for follow-up by the owner; its evidence is
[retained](../etc/evidence/t564-s4-softpc-shared-audit.md). No concrete
incompatibility was found that blocks import; qualification is still required.
S5 imports the committed eight shared roots byte for byte. Both architectures
passed 697 unit tests and the specialized gates; all profile integration
cases passed, although the x64 DeskPro batch had one MyNES native Console
failure which passed on separate retry. Five NXVM integration receivers were
updated for the S3 unready-frame contract. Ten EXEs were rebuilt. The
[S5 evidence](../etc/evidence/t564-s5-softpc-shared-import.md) records the
failure, retest and verification limits. Two pre-existing owner INI changes
were subsequently restored at the owner's request. The owner accepted and
closed S5 on 2026-10-22; T564 remains open. The intermittent MyNES native
Console failure remains a separately queued investigation, not a claimed fix.
T564 S3 was accepted by the owner and closed on 2026-10-22. S4's audit was
accepted for S5 import and is closed. The
[task history](../history/M5-T564-softpc-rgb-frame-import.md) records its
delivery, corrective verification and closure. T564 remains open.

## M5 T564 S2 — Closed

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation of open T564; S2 follows accepted S1. Single-session coordinator and executor. |
| Admission And Approval | Owner requests investigation then approves repair, build, deployment, tests, commit and push on 2026-10-22; Shared x86-nxvm FDC and affected consumers are in scope. |
| Objective | Repair FDC absolute time on machine cold reset and verify reset/resume and stop/start boot from the configured floppy. |
| Non-goals | No unrelated Window geometry change, protected media mutation, CMOS policy change, or claim based solely on a state notification. |
| Reference Baseline | Repository HEAD 2b54b51a5 and deployed NXVM 0564; preserve owner INI and disk images. |
| Candidate Proposal | Bounded owner-directed investigation in open T564; compare first boot against reset/resume and stop/start using existing tests and isolated read-only or disposable media. |
| Files And ABI Surface | Shared x86-nxvm FDC, its existing tests/manifests; NXVM boot regression, task records and affected product binaries. No public API change. |
| Applicable Rules | Execution and documentation authorities; inward architecture and coding boundaries; owner-managed media must be protected. |
| Verification | Prove full versus controller reset clock semantics and guest-visible floppy boot on both paths; build both architectures, run focused and full repository units, relevant integrations, manifest and documentation gates; deploy and verify affected EXEs. Separate real desktop from machine-level evidence. |
| Expected Markers | Cold reset rebases FDC time; controller port reset preserves it; both sequences reach A:\> following a nontrivial prior run. |
| Asset Needs | Existing deployed binary and INI are read-only; task-owned temporary diagnostics under build/t564-s2, 1 hour, 100 MiB, 10-minute no-progress budget; no protected media writes. |
| Reporting Requirements | Explain each sequence, evidence, differences from initial boot, affected ownership, uncertainty and next action. |
| Stop Conditions | Protected media mutation, unrelated Shared/API change, or unexplained failing required validation. |
| Exit Criteria | Repair and regression complete, affected products rebuilt and tested, actual diff reviewed and scoped commits pushed; await owner acceptance; T564 remains open. |
| Original Owner Request | Investigate reset then resume and stop then start hanging at cursor. |
| Similar-Issue Sweep | Compare initial start, reset/resume, stop/start and other machine states without assuming they share a cause. |

Closed by owner direction on 2026-10-22 after delivery; this status correction
does not add desktop, RDP, or other verification beyond the evidence below.
Delivery: owner clarified that both paths eventually start,
but are slow, and reset/resume enters the hard disk/date prompt instead of the
initial floppy boot. x64 deployed 0564 with the adjacent Window/overlay
INI produced a 640x400 first Window, then a 9600x6000 client after
reset/resume in two observations. Reducing that client to 640x400 exposed DOS
waiting at its date prompt; the large client could hide the rest of the guest
screen. A stop/start observation showed a 640x400 client and the same date
prompt, so its reported symptom is not yet explained. A direct Machine probe
reached A:\> after both transitions, but bypassed Session and Window and cannot
qualify the deployed interaction. Root cause and x86/RDP behavior remain under
investigation. Core cold reset clears its elapsed ticks, whereas full FDC reset
preserves the FDC's old absolute elapsed tick and ignores earlier ticks. A
standalone probe confirmed 100 -> full reset 100 -> advance to 1 still 100;
software controller reset correctly preserves the same timeline. The BIOS
tries floppy first then hard disk when floppy boot fails. This identifies a
credible timing defect and failure path; its complete causal chain still needs
a real product regression. Machine-level counterfactual using the same INI:
after an initial A:\> and 15 seconds of running, the original reset/resume
and stop/start each failed to reach A:\> within 30 seconds in separate runs;
an otherwise identical transient FDC object clearing its clock on full reset
reached A:\> in 2.50 and 2.56 seconds respectively. This does not exercise
Session or the real Window. HDC full reset already clears its time. Full FDC
reset now rebases its clock; controller-only reset continues to preserve it.
The deployed x64 application reached A:\> after both reset/resume and
stop/start; the x86 and x64 lifecycle regressions also reached that prompt.
All 697 unit tests passed on each architecture, and the DOS prompt and FDC
read-track integrations passed on each. All eight PC executables were rebuilt.
The anomalous Window geometry remains a separate unqualified observation, not
part of this repair; RDP and deployed x86 desktop behavior were not verified.
The initial large captures exceeded
the temporary size budget; the two generated 230,400,054-byte BMPs were
removed after making a small diagnostic copy. Owner media and INI were unchanged.

## Suspended governance packet: M5 Td S181

The owner requested the PC lifecycle investigation first. The previously
delivered Td S181 remains unaccepted and is suspended, not closed; its packet
and existing delivery are preserved below for later owner review.

M5 Td S181 follow-up P5 onward is delivered for owner review after removal of
audited duplicate tooling operations and isolation of cache-test statistics.
The previous delivery was not accepted/closed. This standalone Shared task does not advance
T564, MyNES or MyRaidenII T2. Existing product work and artifacts stay intact.

| Field | Required record |
| --- | --- |
| Identifier Mode | Governance: M5 Td S181 follows latest NXVM Td S180; one session performs coordinator/executor roles. No implementation T/S or artifact revision is allocated. |
| Admission And Approval | Owner's 2026-10-18 original request and subsequent audit approval authorize this same Td follow-up: four tooling cleanups and related proof/records. All three products retain the same policy; no sibling repository import. |
| Objective | Retain the delivered single CMake/cleanup owners; remove duplicate filesystem checks, inherited preset directories and MyNES standalone options. Replace global cache-hit deltas with invocation-local checked statistics, preserving real compile and safety proof. |
| Non-goals | No emulator behavior, Shared corpus source/API/manifest, ROM/media/INI mutation, global-cache deletion, product binary refresh, device qualification, new parser/service/manager or copied SoftPC implementation. |
| Reference Baseline | Follow-up clean b85336f39, after the recorded initial 17af5609a cleanup. Preserve both stable compiler graphs and all existing product inputs. Recheck task processes before deleting follow-up outputs. |
| Candidate Proposal | ../proposals/m5-build-cache-governance.md; audit/evidence ../etc/evidence/td-s181-build-governance.md. |
| Files And ABI Surface | Follow-up: CMakePresets.json, tools/shared/Clear-Build.ps1, test/tools/build_environment.ps1, tools/mynes/Verify-SharedStandalone.ps1 and this task's records. Estimate scripts/checks +15--30/-20--40; actual +27/-31/net-4. No source corpus, ABI, rule or product input change. Prior delivery +383/-47/net336 remains historical. |
| Applicable Rules | Guides and Execution/Architecture/Coding/Document authorities; architecture/coding governance skills. CMake owns configuration, cleanup tool owns safety, tests call actual owners, documentation gate stays documentation-only. Owner explicitly replaces old cross-task build-tree retention policy. |
| Verification | Real x64/x86 stable Ninja builds, optional launcher off/on, direct build under hostile cache environment, no-op increment, actual local cache stats; reject outside Ninja trees; preview/apply/traversal/link/Git/cache/protected-file/all-before-delete safety tests. Applicable three product doc gates, actual diff and artifact hashes unchanged. No physical/native-device tests required. |
| Expected Markers | Stable .ninja trees, fixed three CCACHE variables, no repeated compile on unchanged build, default preview does not delete, dangerous targets refuse before any deletion; final build contains only .ccache/.ninja or explicitly reported protected exceptions. |
| Asset Needs | Existing external inputs and ten artifacts unchanged. Temporary owner: this Td session, build/td-s181; total2 hours, no progress5 minutes, diagnostic output128 MiB. Cleanup fixtures build/cleanup-contract-PID and build/build-contract-width-PID have180-second and8 MiB budgets and self-clean. Stable build metadata/objects stay only in .ninja. |
| Reporting Requirements | Initial brief and diff estimate before implementation; concrete cleanup list before Apply; actual code/docs counts, bytes freed, retained files, cache/increment/safety proof, failures and unexecuted checks; pushed complete target-scoped delivery awaiting owner review. |
| Stop Conditions | Live unrelated process, unchecked Git data, ambiguous/protected input without preservation/identity, path/link escape or source/behavior scope expansion. Resolve or retain explicitly; never recursively delete build root. |
| Exit Criteria | Follow-up actual diff review complete; each width passes2/2 governance tests with2 invocation-local cache hits and no-op increments. Four standalone configurations, three doc gates,1398 Types inputs and ten unchanged artifact identities pass. Remove only owned16 temporary files /171030 bytes; only both cache roots remain. Shared P5 f15f5a77b and MyNES P6 31c1c3ca1 are pushed; this NXVM P7 receipt awaits acceptance. Prior P1--P4 evidence remains intact. |
| Original Owner Request | Uniform optional ccache/Ninja and temporary-output lifecycle with safe existing build cleanup, minimal single-owner tooling and no unnecessary EXE refresh. |
| Similar-Issue Sweep | Search tracked presets/tools/current operational docs for old tree paths/launchers, task-keyed build roots and duplicated cleanup ownership. Historical evidence remains historical; stale product-only script contracts are repaired or explicitly retired. |

## Retained Numeric Task

M5 T564 remains open. The owner accepted S1 on 2026-10-17 and explicitly
requested its closure. S1 is closed after coordinator actual-change review
of Shared `d30b69b2a`, MyNES `906e61b0c` and NXVM `b9433b31d`.
There is no active S packet and no successor task is admitted.

| Task | Progress |
| --- | --- |
| T564 | S1--S5 closed; S4 audit led to the S5 import. T-level closure has not been requested. |

The complete S1 packet is retained in `b9433b31d`. The
[proposal](../proposals/m5-softpc-rgb-frame-import.md),
[task history](../history/M5-T564-softpc-rgb-frame-import.md) and
[review evidence](../etc/evidence/t564-s1-shared-frame-import.md) retain the
frozen scope, similar-issue sweep, initial failures and final proof.
Each width passed 666 units, 48 product integrations, four shared native
integrations, specialized gates and 498 strict rows. The accepted
[results](../etc/evidence/t564-s1-results.json) and
[ten artifact hashes](../etc/evidence/t564-s1-artifacts.json) remain unchanged:
eight PC 0564 and both MyNES 0044 EXEs.

Closure rechecked all ten deployed hashes and the 297 canonical imported files
against the frozen export. The eight source/test roots match the reviewed
SoftPC dfc85aac bytes, also unchanged at its 5e76dd49 closure baseline.
No source, test, build, configuration or artifact input changes in this
governance-only acceptance; no rebuild is required. Manual RDP, physical
audio/controller and whole guest GUI proof remain unclaimed, with prior
MyNES TODOs unchanged. The task queue is not advanced.

The owner's subsequent selection of former queue entry 10 is governed by
[MyRaidenII M0](../../myraidenii/states/CURRENT.md), not an admitted NXVM T.
The [NXVM Queue](QUEUE.md) records the transfer; other candidates remain pending.

## Receiving Release Update From MyRaidenII

MyRaidenII M1 T3 S10 refreshes the eight current0564 PC EXEs after Shared
`f2ca9ae01` fixes callback-fault publication at exact run budgets. Both full
repository unit suites pass687/687, and14 selected PC receiving integrations
per width pass with the original predicates/budgets. Eight built/deployed
hashes and PE widths are checked; all profile switches reuse CPU objects.
Both MyNES0044 EXEs,33 protected tracked asset files and six selected external
media-role identities remain unchanged. No NXVM source, INI, firmware or media
change and no new NXVM task/S is allocated; the active Td S181 is untouched.
The [S10 result/identity record](../../myraidenii/etc/evidence/t3-s10-results.json)
and [scope/review](../../myraidenii/etc/evidence/t3-s10-cop-transfer-review.md)
distinguish the first incomplete x64 unit attempt from final qualification.
No complete external integration, desktop/RDP or gameplay pass is claimed.

MyRaidenII M1 T2 S49 refreshes the eight0564 PC EXEs after Shared P1
`a07c37107` elapsed-time and genuine minimum Machine receiver delivery. No NXVM
source, configuration, firmware or media changes, and no NXVM task/S is
allocated. Both MyNES0044 EXEs and all33 protected tracked asset files remain
unchanged. This receiving update does not alter the active Td S181 packet.

Both widths pass684/684 full repository units. The final indentation-only test
replacement passes30/30 per width and produces twelve byte-identical objects.
Each width passes498 strict rows,44 CPU compile-command checks,536 unique
declarations and1401 Types inputs. Four PC receivers pass14 selected
configuration/boot/lifecycle integrations per width. Eight PE widths, absence
of compiler debug sections and deployed/build SHA-256 match. Both original
Ninja/ccache graphs finish at default; each profile switch recompiles zero CPU
objects. Six selected external media preserve size/SHA/access mode.

The [S49 evidence](../../myraidenii/etc/evidence/t2-s49-elapsed-time.md),
[results](../../myraidenii/etc/evidence/t2-s49-results.json) and
[ten release identities](../../myraidenii/etc/evidence/t2-s49-artifacts.json)
supersede the PC hashes in the [S48 receipt](../../myraidenii/etc/evidence/t2-s48-artifacts.json).
The explicit V30 serialized-fetch L2 and minimum deterministic Machine route
do not qualify silicon-exact time or complete Machine/gameplay. S50--S51 remain
in the same open MyRaidenII T2. No complete external suite, manual desktop/RDP
or physical device qualification is claimed.

### Prior S46 Receiving Update

MyRaidenII M1 T2 S46 refreshes the eight current 0564 PC EXEs after
Shared V30 escape/bus and instruction-entry mode P1 `128204d91`.
NXVM production source, configuration, firmware and media are unchanged;
one receiving static assertion now checks the CPU-private prefix helper's
actual context-bearing signature. No assertion is removed or weakened.
This receiving update allocates no NXVM task/S.
MyNES 0044 hashes remain unchanged. This receiving publication supersedes
only the PC hashes in the historical T564 snapshot above; it does not rewrite
T564's accepted evidence or open task status. See the
[current receiving evidence](../../myraidenii/etc/evidence/t2-s46-time-accounting.md)
and [ten release identities](../../myraidenii/etc/evidence/t2-s46-artifacts.json).
Each width passes680 units (193 X86 Core),498 strict rows,1391 Types inputs
and14 selected PC receiving integrations plus a restored-default identity,
not a new full integration-suite claim. Both original Ninja/ccache Release
graphs finish at default. Profile switches/restores recompile zero CPU
objects, verified with both Ninja path-separator spellings. INIs and MyNES
assets are unchanged; six selected external media originals retain actual
before/after SHA-256/size/access-mode identities, not an entire archive audit.
Earlier receiving proof remains in MyRaidenII history, not rewritten.
S46 is a bounded prerequisite stage, not complete V30 timing, Machine
admission or MyRaidenII gameplay qualification.

## Latest Task Closure

M5 T563 is closed after actual-change review of S10 Shared **a936e554b**,
MyNES **621b98d15** and NXVM **80bc1ac92**, plus the complete CPU closure audit.

| Task | Closure |
| --- | --- |
| T563 | S1--S10 accepted. Original C01--C17, F01--F20 and R1--R10 are reconciled: selected 486SX functionality, complete runtime/preview timing and real Core/Board production admission are implemented and qualified. Both widths pass 655 units (166 X86 Core), 48 product integrations, four shared native integrations and 498 strict rows. Ten current releases are verified. |

The [closure audit](../etc/evidence/t563-final-closure.md),
[task history](../history/M5-T563-486sx-timing-production-completion.md) and
[archived proposal](../history/M5-T563-486sx-timing-production-completion-proposal.md)
retain the requirement-to-proof mapping, source/model grades and acceptance.
S8's sole transferred native MIDI obligation is resolved by actual successful
S9/S10 opens/releases; reclassification alone is not treated as its resolution.

[S10 results](../etc/evidence/t563-s10-results.json),
[canonical inventory](../etc/evidence/t563-s10-canonical.json) and
[ten artifact hashes](../etc/evidence/t563-s10-artifacts.json) are the accepted pre-T564
receiving baseline: PC 0563 and MyNES 0044 pairs. Initial x64 timeout/native
Console failures and unchanged successful full reruns remain explicit in
[S10 evidence](../etc/evidence/t563-s10-shared-import.md) and existing TODOs.

Closure does not promote approved L2 models to silicon-exact timing or claim
broader x87, every 486 variant, PC110 board/firmware, manual RDP, physical audio
or full guest GUI qualification. The next queued candidate is PC110 Board;
no successor T is admitted automatically. This governance-only closure changes
no source/test/build/configuration/EXE input and needs no artifact rebuild.

## Previous Task

M5 T562 is closed after S4 coordinator review of pushed delivery `131e7d65c`.
Its complete final packet is retained in that commit;
[archived proposal](../history/M5-T562-shared-audio-import-quality-proposal.md)
and [task history](../history/M5-T562-shared-audio-import-quality.md) retain scope.

| Task | Closure |
| --- | --- |
| T562 | S1--S4 accepted: pinned import, approved shared repairs and equivalent-path consolidation, final 631/631 units and 48/48 integration executions per width, published CLI checks and specialized gates. Eight PC 0562 artifacts refreshed; MyNES 0044 pair remains current. |

[Final evidence](../etc/evidence/t562-s4-final-qualification.md),
[integration inventory](../etc/evidence/t562-s4-integration-inventory.tsv) and
[ten artifact hashes](../etc/evidence/t562-s4-artifacts.json) record results.
S4 changes no source/test code. Earlier native-console intermittency remains
in MyNES TODO; manual RDP, physical MIDI/audio and complete guest GUI proof are
not claimed. Remaining 486SX C16/C17/final qualification is now admitted
separately as T563 above; it was not completed by T562 closure.

## T562 S2 — Accepted Shared Quality Cleanup

S2 is closed after one-session coordinator actual-commit review of Shared
`e5b346309` and NXVM `8f3280726`. S3 is also accepted as recorded above.
The complete S2 implementation packet is retained in P2.

The five admitted findings are repaired: obsolete port dispatch, empty
composition wrapper, test ownership, stale names and checker blind spots.
[Evidence](../etc/evidence/t562-s2-shared-boundary-cleanup.md) and
[eight artifact hashes](../etc/evidence/t562-s2-artifacts.json) record 630/630
units per width, 15 selected integration executions per width, eight final
published CLI checks, all specialized gates and the reviewed actual diff.

Eight PC 0562 EXEs are rebuilt and published. MyNES, owner INI/media/ROM/
snapshots and sibling repositories are unchanged. Automated desktop-labelled
tests pass; manual RDP/physical audio and fresh full external qualification
are not claimed. Queue order and the unresolved 486SX successor are unchanged.

## Completed M5 Td S180 Shared Eight-Tree Audit

Owner-requested audit is complete; no source/test/build/artifact was changed.
[Report](../etc/evidence/td-s180-softpc-eight-tree-audit.md) and
[hash inventory](../etc/evidence/td-s180-eight-tree-hashes.tsv) pin NXVM
12e79911b versus SoftPC 44cad548: only committed Lib source/test differ;
the remaining six roots match. Upstream's subsequently edited Emulator
heartbeat work is explicitly separate, not a fixed import baseline.
Manifest and static boundary/Types checks pass for the frozen pair;
no fresh dynamic or desktop qualification is claimed.
Confirmed shared findings and receiver implications were repaired in T562 S1.
Standalone governance actual-change review and documentation checks pass.

M5 T561 is closed after S9 coordinator actual-change review of pushed Shared
`ee7fe06ab` and NXVM `034f3eb77`. No S is active. The
[archived proposal](../history/M5-T561-core-package-split-proposal.md) and
[convergence ledger](../history/M5-T561-core-package-split.md) retain the
complete I0--I5 scope and acceptance.

| Task | Closure |
| --- | --- |
| T561 S1--S9 | Accepted: Core split into X86 Core and X86 NXVM, mirrored source/test owners, real non-PC consumer and all four PC receivers. Both widths pass independent Core 165/NXVM 215 tests, root 626 units and 48 profile integration executions, specialized gates and eight published artifact checks. |

[Final evidence](../etc/evidence/t561-s9-final-qualification.md),
[679-path disposition](../etc/evidence/t561-s9-disposition.tsv) and
[eight artifact hashes](../etc/evidence/t561-s9-artifacts.json) record the
qualified 0561 releases. All eight PC EXEs were rebuilt and published.
INI/media/ROM/snapshots and MyNES artifacts are unchanged.

The final review verifies actual checker changes, all disposition destination
hashes, eight artifact hashes, preserved source owners and independent link
graphs. S9 production delta is zero; root test enforcement is +10/-4.
Its complete execution packet remains in delivery `034f3eb77`.
Existing App tests' NXVM adapter-private coupling is explicitly retained in
TODO; it is not neutral X86 Core private-layout borrowing.

Manual physical/RDP and fresh diagnostic-ROM qualification are not claimed.
The next queued 486SX successor retains C16/C17/final qualification and awaits
owner admission; this structural completion does not claim those CPU goals.

## Previous Staged CPU Closure

| Task | Closure |
| --- | --- |
| T560 | Closed as an owner-approved bounded stage. S1--S8 remain accepted; S9 implementation `b906c7138` passes coordinator actual-change review. Complete C16 timing/runtime/preview, C17 production admission and final CPU qualification remain unmet and transfer intact to the CPU successor after T561. |

## Retained T560 Stage Baseline

[Final S9 evidence](../etc/evidence/t560-s9-selected-timing.md#final-frozen-stage-qualification):
607/607 complete repository units per width; complete specialized gates;
48 profile integration executions plus restored-default identity per width;
eight rebuilt, deployed, PE/hash-verified and debug-info-stripped 0560 PC EXEs.
Both original Release Ninja/ccache graphs were restored to default. The
PC artifact target at that delivery was `vm-0-5-0560`; its source/artifact baseline is
implementation `b906c7138`. Shared-six, MyNES 0044 and owner INI/ROM/media/
snapshot inputs are unchanged. Automated desktop-labelled tests pass; manual
RDP, full Windows GUI and PC110 qualification are not claimed.

The [T560 history](../history/M5-T560-complete-486sx-cpu.md) and
[original convergence ledger](../etc/evidence/t560-s1-cpu-convergence.md)
preserve accepted mechanisms, source/model restrictions and all residuals.
The sole selected-SX instruction recipe remains explicitly compatibility/
unallocated, and production 486SX construction remains closed.

The former [Queue](QUEUE.md) placed
[Core refactoring](../history/M5-T561-core-package-split-proposal.md) first and
[complete remaining 486SX qualification](../history/M5-T563-486sx-timing-production-completion-proposal.md)
second. Core refactoring is now admitted as T561; CPU work
stays paused until the refactor is accepted and its successor is admitted.

## Retained Historical Packets (Not Active)

### Accepted Delivery — M5 T559 S6 CPUID And Flags

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation T559 S6 after coordinator acceptance of S5 P1 8e6159de4. |
| Admission And Approval | Owner-admitted T559 and automatic sequential Core-only implementation; group I7/I8 together. No Shared or MyNES changes. |
| Objective | Freeze source-qualified selected SX CPUID/ID behavior and implement it with the existing flags owner; reconcile PUSHF/POPF, IRET and task-load masks without duplicated profile state. |
| Non-goals | No production 486 admission, invented zero-feature silicon, unproved VME advertisement, SMM/cache/TLB hardware, PC110 board, Shared or MyNES changes. Real AC/VIF/VIP hardware effects remain mandatory system gates. |
| Reference Baseline | 8e6159de4; S1 I7/I8, pinned selected-SX data sheets and original Intel manuals. Audit exact stepping applicability and current flags paths before implementation. |
| Candidate Proposal | [Selected 486SX instructions](../proposals/m5-pc110-486-instructions.md), [finite ledger](../etc/evidence/t559-s1-instruction-inventory.md), and [system receiver](../history/M5-T560-complete-486sx-cpu-proposal.md). |
| Files And ABI Surface | Core CPU existing instructions, flags definitions and profile reset only where source-qualified, directly affected CPU tests/registration, Core manifests and NXVM evidence. Preliminary production +60--160/-10--60 and tests +200--400/-0--30; refine after source/form audit. No planned public API or persistent-state addition. Rebuild eight PC EXEs if executable inputs change. |
| Applicable Rules | NXVM guide; Execution lifecycle/coverage/artifacts; Document; Architecture/Coding and their skills; NXVM architecture/layout and source/research policy. |
| Verification | Full I7 leaf/GPR/flags/privilege/LOCK/fetch/old-profile matrix; I8 PUSHF/POPF, real/protected/VM86 IRET and task-load masks, 16-bit upper-half preservation and privilege rules. Focused whole batch, actual-change review, then full x64/x86 repository units and affected artifact/INI identity checks. |
| Expected Markers | CPUID feature and ID behavior match the named candidate source contract; one flags-mask owner, retained earlier-family semantics and no production admission before system qualification. |
| Asset Needs | Existing original manuals and admitted external build inputs only; preserve owner INI/media/ROM/snapshot and MyNES. |
| Reporting Requirements | Before edits report source/stepping contract, existing flags owners, complete form matrix and refined diff. Delivery records actual diff, direct proof, system gates, artifacts and whole-batch qualification. |
| Stop Conditions | Required Shared/new cross-owner API, unresolved source contradiction or absent evidence needed to choose feature values. Record exact prerequisite and continue independent scoped work; do not guess silicon capability. |
| Exit Criteria | I7/I8 applicable instruction contracts have direct proof and retained-family regression, system-only effects remain explicit, full dual-width units and applicable eight artifacts pass, actual diff reviewed and complete P pushed. No full CPU/PC110 qualification claim. |
| Original Owner Request | Implement 486SX instruction semantics under governance; implement related groups together and avoid per-opcode full validation. |
| Similar-Issue Sweep | Audit every reserved/writable flags-mask consumer, CPUID dispatch/metadata/preview and profile-specific identity path; classify reset/control/VME effects at their real owner. No global enum widening or CPU rewrite. |

S5 is accepted and closed after actual review of pushed 8e6159de4. Its
[evidence](../etc/evidence/t559-s5-system-instruction-boundaries.md) records
retained TR transfer/form/privilege/fetch proof and 596/596 units per width.
Complete I4--I6 unavailable instruction/effect contracts remain with the system
receiver, including the existing passive 486 TR6/TR7 limitation. No production
source changed; eight S4 0559 EXEs remain current. Shared-six, MyNES and owner
inputs are unchanged. S6 is admitted for the complete CPUID/flags batch; source
review precedes edits. T559 and production 486 qualification remain open.

The proposal's batch-efficiency order applies: complete family implementation,
focused CPU validation and actual-change review before final whole-suite
qualification. External guest boot remains a T-level exit requirement.

S6 executor delivery is complete, pending coordinator acceptance. The candidate
CPUID handler and one profile-aware flags mask have focused x64/x86 proof.
The new CPU-owned test covers leaf outputs, independent code-size/prefix
combinations, old-profile rejection (without confusing 8086 POP CS with a
two-byte CPUID), truncated preview, failed instruction fetch without register
mutation, reset ID/signature, PUSHF/POPF and real/VM86 IRET AC/ID transfer.
Existing protected IRET and five task-switch completion paths now include
486 AC/ID patterns; their updated tests pass on both host widths. Existing
IRET/POPF/task regressions also passed in the preceding focused batch.
The [S6 evidence](../etc/evidence/t559-s6-cpuid-flags.md) records source-contract
reconciliation, direct proof and actual-diff review. All 597 unit cases per
width now have passing evidence: x64 first passed 594 and then passed its three
affected rechecks after correcting one old metadata assertion and its manifest;
x86 passed 597/597 in one complete run. Eight PC EXEs are rebuilt/deployed and
pass real-product INI identity, PE-width and version checks. Core manifests,
documentation and whitespace gates pass. Shared-six, MyNES and owner inputs
remain unchanged. No production 486 admission, VME hardware qualification,
manual RDP or external firmware boot result is claimed. Whole-T S7 remains.

## Closed Task — M5 T558

Owner accepted S2 delivery 7ea9084fe and approved T closure on 2026-10-13.
The nine source/checklist receivers and their bounded unresolved decisions are
accepted as research delivery. This does not assert fully measured hardware:
CPU feature/source decisions transfer to T559 S1 and the system candidate;
board, input/power, display, storage, audio and asset decisions remain mandatory
gates in their named queued receivers. Production and ten EXEs are unchanged.
The retained [history](../history/M5-T558-pc110-evidence-and-implementation.md)
records the closure review and accepted limits.

### Accepted Delivery — M5 T558 S2 PC110 Implementation Evidence And Contract Freeze

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation T558 S2, after accepted S1 research/planning delivery feadb8235. |
| Admission And Approval | Owner directs continued PC110 audit/design so every later implementation task has evidence, a checklist and original manuals or reference code. Owner subsequently selects original DSTN and upgraded TFT as two compiled variants, with mypc110_x86.exe, mypc110_x64.exe, mypc110_tft_x86.exe and mypc110_tft_x64.exe; this supersedes the briefly proposed INI model/BIOS selection. All ROMs remain external build inputs embedded in EXEs. This admits design/research, not production implementation or source import. |
| Objective | Establish per-receiver implementation evidence, finite capability/checkpoint inventories, hardware/firmware baseline and explicit unresolved decisions; audit completion against all nine planned receivers. |
| Non-goals | No production/test/Shared changes, EXE refresh, external ROM acquisition/import, BIOS bypass, guessed hardware qualification or silently excluded device. |
| Reference Baseline | NXVM feadb8235; three external pinned references in S1 ledger, revalidated unchanged at S2 admission. |
| Candidate Proposal | [PC110 programme](../proposals/m5-pc110-evidence-and-implementation.md), its nine linked receivers and S1 ownership audit. |
| Files And ABI Surface | NXVM planning, evidence, status and history only; no ABI. |
| Applicable Rules | Guide, Execution coverage/closure rules, Document rules, architecture ownership and source/research policy; architecture-governance skill. |
| Verification | Inspect source bytes and reference code, original manual identity and relevant sections; reconcile every receiver to sources, current implementation, finite checklist and proof criteria; run documentation/link/diff checks. |
| Expected Markers | No receiver relies solely on a repository name or boot screenshot; unavailable raw evidence and conflicts are explicitly classified; every unresolved decision has an owner and admission condition. |
| Asset Needs | Read existing external candidates and record roles/size/hash; no copies into source or release admission inferred from availability. |
| Reporting Requirements | Report concrete evidence changes, exact unresolved facts, completeness per receiver and unexecuted hardware/runtime verification. |
| Stop Conditions | Incompatible source derivation, unapproved acquisition, production/Shared changes or essential hardware decision lacking defensible evidence; continue independent research. |
| Exit Criteria | All nine receivers have reviewed evidence/checklists/proof gates; exact baseline choices are justified or explicitly blocked with bounded resolution, without claiming unknown behavior implemented. T closure separately reviews the programme's hardware/contract freeze requirements. |
| Original Owner Request | Execute PC110 audit/design so each later implementation task has evidence, a checklist and original manuals or reference code. |
| Similar-Issue Sweep | Verify referenced raw evidence exists; distinguish original versus reconstructed manuals, implemented versus placeholder models, firmware dumps versus runtime shadows, CPU variant claims and actual bytes. |

S1's inventory, queue plan and ownership review are accepted as design delivery,
not complete hardware freeze. S2 completed its research goal. Current ten artifacts remain
unchanged because no executable inputs change.

S2 research delivery: all nine proposals consume the reviewed
[source record](../etc/evidence/t558-s2-source-availability.md) and
[receiver reconciliation](../etc/evidence/t558-s2-receiver-checklists.md#final-research-reconciliation).
Their source/checklist/ownership/proof coverage is reviewed; exact unresolved
hardware and asset choices retain named admission gates. No implementation,
firmware preparation, runtime qualification or new artifact is claimed.
The owner accepted this bounded delivery; remaining hardware decisions transfer
explicitly to the corresponding receivers rather than being counted as proven.

### Accepted Delivery — M5 T558 S1 PC110 Research And Gap Inventory

| Field | Required record |
| --- | --- |
| Identifier Mode | New T558 S1 after T557 closure; next NXVM numeric task. |
| Admission And Approval | Owner selected PC110 on 2026-10-13, assigns all current work to M5, then requests a multi-T queue plan. T558 is revised to research/contract freeze; nine explicit queued receivers retain its implementation/product goal. Owner also authorizes external reference checkout updates and external ROM embedding into committed PC110 EXEs. |
| Objective | Establish a source-tiered PC110 hardware, asset and current-code gap ledger, current ownership map and dependency-ordered multi-T implementation plan. |
| Non-goals | No production/test code, Shared-six changes, external source/ROM import, BIOS interception, asset edits or claims of a working PC110. |
| Reference Baseline | NXVM 8f00b1bce; external research revisions and dirty-source distinction recorded in the ledger. |
| Candidate Proposal | [PC110 evidence and implementation](../proposals/m5-pc110-evidence-and-implementation.md). |
| Files And ABI Surface | NXVM proposal, status, queue, roadmap, history and research evidence only; no ABI or executable input changes. |
| Applicable Rules | Guide, Execution lifecycle/coverage rules, Document rules, architecture ownership and source/research policy; architecture-governance skill. External sources are read-only research after the explicitly authorized checkout refresh; no code import. |
| Verification | Inspect current CPU/profile/device interfaces and external source provenance; distinguish original authority from reconstruction; check documentation structure, links, actual diff and clean scope. Design-only work does not manufacture runtime tests. |
| Expected Markers | Every inventoried domain has source status, current owner, gap, next receiver and proof gate; no M6 allocation among current candidates. |
| Asset Needs | Inventory external filenames and references only; lawful profile asset set is not yet admitted. Do not copy firmware/media or treat research dumps as release inputs. |
| Reporting Requirements | State facts versus hypotheses, exact CPU absence, external-reference limitations, proposed batches, unknown revision/assets and unexecuted hardware/runtime validation. |
| Stop Conditions | Source licensing/import, undocumented hardware behavior requiring an owner choice, Shared change or expanding S1 into implementation. Continue safe research on unresolved sources. |
| Exit Criteria | Reviewed initial source/gap ledger and bounded next research/implementation gates; each implementation receiver has a proposal, dependency, owner, proof and stop condition; M5 queue reconciled, documentation checks passed and complete documentation P pushed. T558 still needs hardware/contract freeze; no PC110 runtime qualification claimed. |
| Original Owner Request | Admit entry 3: IBM PC110 research and implementation; all currently queued work belongs to M5, not M6; begin research. |
| Similar-Issue Sweep | CPU enum versus historical 486 claims; stale Common/x86/chips/ibmpc paths in PC110 plan; roadmap/queue milestone allocations; reconstructed/emulator evidence versus original sources; owner-requested four-App/Core boundary audit and PC110 ownership receivers. |

The [initial ledger](../etc/evidence/t558-s1-pc110-source-and-gap-ledger.md)
is the T convergence inventory, not a complete register specification. Original
manual, board-revision and lawful asset qualification remain research gates.
Current artifacts remain the verified T557 PC pairs and MyNES 0044 pair;
S1 changes no executable input and creates no PC110 artifact.
The [ownership audit](../etc/evidence/t558-s1-core-app-ownership.md) adds actual
four-App/Core inspection and the proposed PC110 split without admitting code
changes; current manifest and dependency checks pass, with no dynamic retest.

The owner accepts T556 S11 and requests the new PC publication/INI task.
T556's NXVM work is closed. The owner subsequently closed S10's diagnostic
delivery with its unresolved MyNES input repair transferred to
[MyNES TODO](../../mynes/states/TODO.md#rdp-short-input-repair).
`3dc750be2` is not a production fix. No MyNES implementation is
part of T557's original S1; S2's approved receiver exception is recorded below.
Prior S9/S11 packets below are historical delivery evidence.

## Closed Task — M5 T557

| Task | Status |
| --- | --- |
| T557 | Closed after accepted S2 and task-level evidence review, before owner-selected T558 admission on 2026-10-13. |

Accepted delivery: Shared `5f5f43c8c`, MyNES `08bdaf06d`, NXVM `97d3cf948`.
Both widths passed 592 repository units, 36 PC integrations and 12 MyNES
integrations, with all ten EXEs deployed. The [S2 evidence](../etc/evidence/t557-s2-cursor-ownership.md)
retains the contract, review, hashes and verification limits. This closure only
updates governance; production and executable inputs are unchanged, so the
verified binaries remain current without a rebuild. The [proposal](../history/M5-T557-pc-publication-and-cursor-proposal.md)
is archived; both original naming scope and cursor extension are covered.

## Delivered Task — M5 T557 S1 PC Publication And INI Identity

| Field | Required record |
| --- | --- |
| Identifier Mode | New T557 S1, next numeric NXVM task after accepted T556 S11 and explicit MyNES residual transfer. |
| Admission And Approval | Owner requests four product-specific lowercase EXE/INI identities, explicit App INI injection and full affected qualification. This naming contract supersedes the old machine-key artifact naming in Execution for these four Apps only. |
| Objective | Publish my5160, my5170, mydeskpro386 and nxvm release pairs and load each App's own adjacent lowercase INI through the existing Core parser. |
| Non-goals | No SoftPC access/import, Shared-six changes, MyNES changes, configuration-content/media/ROM/snapshot mutation, fallback names or machine-key renaming. |
| Reference Baseline | ed75cf572, clean after separate MyNES diagnostic publication; accepted PC implementation 6b0cb13e8. |
| Candidate Proposal | [PC publication and INI identity](../history/M5-T557-pc-publication-and-cursor-proposal.md). |
| Files And ABI Surface | Four App build/main declarations; Core Product startup signature and version; NXVM build/deployment/gates; affected Core/App tests, current documents and eight assets. Root PC preset target version changes use a separate Shared commit, with no MyNES preset change. |
| Applicable Rules | NXVM guide and Architecture/Coding/UI/source policy; shared Execution/Architecture/Coding/Document rules; architecture-governance and coding-governance skills. |
| Verification | Existing x64/x86 Ninja/ccache graphs; affected startup/config/Console tests, all repository units, four-PC integrations and static/manifest/docs gates. Prove explicit INI selection, no wrong-name fallback, INI-relative media paths, lowercase release identities, PE widths and deployment hashes. |
| Expected Markers | Eight version 0.5.0557 release files with product-specific prefixes; four matching lowercase INIs, byte-identical contents; unchanged display names and internal machine keys. |
| Asset Needs | Read existing external inputs only; create and verify new eight artifacts before removing replaced PC EXEs; preserve MyNES and owner media/configuration bytes. |
| Reporting Requirements | Per-file scope/diff and naming map, actual dual-width results, release identity proof, missing manual/RDP verification; never infer desktop acceptance from unit tests. |
| Stop Conditions | Required Shared implementation, format/behavior change beyond naming, unexplained input mutation or conflict with other work. |
| Exit Criteria | Source/build/runtime/test/docs identities agree, required tests and eight deployments pass, old duplicate releases removed, reviewed target-scoped commits pushed; await owner review. |
| Original Owner Request | Close current S and admit a new T for four PC product-named EXE/INI publication and explicit runtime configuration loading. |
| Similar-Issue Sweep | All four App declarations/callers, integration/setup fixtures, release gates/scripts/presets and current normative documents; historical text stays historical. |

### S1 Delivery

Implementation and qualification are complete: 592/592 repository units and
36/36 four-PC integration executions per width. Eight 0.5.0557 product-named
EXEs are deployed and the replaced eight filenames removed. Four renamed INIs
retain their original hashes. Both specialized gate aggregates, manifests,
registration, INI/publication and documentation checks pass. The
[delivery evidence](../etc/evidence/t557-s1-publication-ini-identity.md) records
actual diff, final hashes and limits. Shared-six and MyNES are unchanged.
S1 is delivered for owner verification; T557 remains open.

## Accepted Task — M5 T556 S11 Default Text Raster

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: next unused S11 after the MyNES S10 admission; do not execute or modify that product's pending work. |
| Admission And Approval | Owner requests default NXVM 80x25 full 8x16 text, DeskPro verification only unless a defect is proven, dual-width qualification and deployment. |
| Objective | Repair the default guest BIOS text-mode register/font geometry so boot and graphics-to-mode03 return produce 640x400, with correct cursor and raster timing; establish whether DeskPro 640x350 is correct. |
| Non-goals | No Shared change, forced Window height, host BIOS interception, external ROM or INI/media mutation; preserve MyNES and other pending changes. |
| Reference Baseline | Current master 7bfefb2fc, plus preserved S10 documentation/diagnostic work; review b2ee99b1c and b25e2ca8d and subsequent firmware changes. |
| Candidate Proposal | [T556 proposal](../proposals/m5-app-component-layout.md), owner-added S11. |
| Files And ABI Surface | Default firmware/video tests and native size diagnostic; owner-added Core CPU retirement selection and existing capture boundary, timing tests/gates and Core manifests. Timing selector becomes CPU-private; no Shared API change. Docs and affected four-PC dual EXEs. |
| Applicable Rules | NXVM guide, architecture, coding, UI and source policy; shared Execution/Architecture/Coding/Document rules; architecture-governance and coding-governance skills. |
| Verification | Real guest execution at boot and return from graphics; complete font halves, cursor, vertical active/retrace/exit; snapshot-to-Window 640x400; actual client dimensions. Both existing Ninja/ccache graphs, full repository units, all four PC profile integrations including default DOS/Windows and required static/manifest/doc gates. Actual RDP DPI proof is separately reported, never inferred from a synthetic frame. |
| Expected Markers | Default 80x25, glyph height 16 and 640x400; coherent CRTC overflow/total/display/blank/retrace and cursor registers. DeskPro retains 14/350 if sources and selected board agree. |
| Asset Needs | Existing external manuals and build inputs read-only; refresh affected four-PC 0556 x64/x86 pairs and validate PE identity after the Core repair. Preserve MyNES, adjacent owner inputs and unrelated EXEs. |
| Reporting Requirements | Record sources and timing grade, before/after reproduction, actual diff, dual-width results and release hashes; distinguish native client, outer frame and untested RDP scaling. |
| Stop Conditions | Required Shared/API change, unsupported EGA register semantics, unproven DeskPro defect, or overlap with unrelated work. Report before extending scope. |
| Exit Criteria | Correct default guest BIOS and regression proof, required qualification/deployment complete, actual-diff review and scoped commits pushed; await owner desktop acceptance, T remains open. |
| Original Owner Request | Investigate/fix default Window text geometry from guest BIOS and verify rather than normalize DeskPro 640x350. |
| Similar-Issue Sweep | Check all default text-mode initialization/restore paths, glyph upload, BDA geometry, cursor and raster counters; inspect selected DeskPro BIOS/board without changing external ROM. |

S11 implementation and final automatic qualification are complete: 592/592
units and 32/32 four-PC profile integration executions per width. All eight
PC EXEs are rebuilt and deployed; MyNES and public six remain unchanged.
The default BIOS now presents all 16 font rows. Its exposed Core TIME fault
is repaired by selecting completed-instruction timing before interrupt entry
reuses decoder state. The actual-Core regression fails against admission
objects and passes with the repair. DeskPro retains its correct 14/350 mode.
Native remote-session observations no longer show the TIME failure, but the
diagnostic pause acknowledgement and owner phone-client DPI/interaction are
not qualified by automation. The owner has accepted this scoped delivery;
S11 is closed. Preserve the separately retained MyNES S10 pending work. See the
[S11 evidence](../etc/evidence/t556-s11-default-text-raster.md).

## Delivered Task — M5 T556 S9 Audit Repairs And Qualification

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: S9 follows completed S8 audit and accepted S1--S7. |
| Admission And Approval | Owner's 2026-10-13 request assigns necessary audit repairs to S9. Targets are NXVM and MyNES, with separate commits. The proposal's existing Shared root-build receiver covers only three PC preset target names changing from 0555 to 0556; public six and shared rules remain excluded. |
| Objective | Flatten DeskPro source and matching unit tests under the owner's S9 amendment; repair A1--A5 under that revised ownership, retain A6's separate Td receiver and complete five-App qualification. |
| Non-goals | No Shared-six or shared-rule edit, unrelated hardware rewrite, new runtime manager, persisted-format change or owner-input mutation. |
| Reference Baseline | S8 audit `a28cac691`; accepted S7 implementation `307cf1bd6`. |
| Candidate Proposal | [Five-App component layout](../proposals/m5-app-component-layout.md). |
| Files And ABI Surface | Core test registration/owner gates; all DeskPro source moved to its App root and units to its unit root, with includes/guards/symbols/build/gates/docs repaired; moved MyNES Machine symbols/callers; corresponding product version/artifact receivers. No Shared API change. S9 actual diff is recorded against the amended flat file map. |
| Applicable Rules | Product guides; shared Execution, Architecture, Coding and Document rules; both product architecture/layout authorities; source policy; architecture-governance and coding-governance skills. |
| Verification | Correct labels/names without reducing cases; reject incomplete D4 attachment with rollback proof; run dual-width full units and every admitted profile/MyNES integration in original Ninja/ccache trees; verify manifests, dependency/doc gates and affected release pairs. |
| Expected Markers | No absolute-path or mislabeled Core tests; no DeskPro chips/machine/product source directories or matching unit nesting; one D4 state owner and one Core attachment owner; missing alias rejects atomically; documents match current source; complete per-route results and ten qualified artifacts. |
| Asset Needs | Existing BYOB inputs read-only; update affected executable pairs only, preserving INIs, media, ROM originals and snapshots. |
| Reporting Requirements | Report per-target actual added/deleted/net lines, retained owners, exact test coverage, artifact identities/hashes, failures and unexecuted desktop checks. |
| Stop Conditions | Required Shared-six/rule change, incompatible saved data, unexplained behavior change or newly expanded hardware scope. Report rather than patch outside this packet. |
| Exit Criteria | A1--A5 repaired with evidence; A6 explicitly transferred; required dual-width builds/tests and artifact checks complete; actual changes reviewed, per-target P commits pushed, clean worktree; T closure still requires final acceptance. |
| Original Owner Request | Admit S8 for comprehensive governance, documentation and code-quality audit; admit S9 for necessary repairs. |
| Similar-Issue Sweep | Search all five App owners and Core plus current authorities for the same stale ownership, registration, vocabulary and contract defects; retain historical terminology as history. |

### S8 Audit Delivery

The [audit ledger](../etc/evidence/t556-s8-quality-audit.md) records six
confirmed classes and their S9 disposition. Both widths pass the 18 selected
static gates; both documentation checks and the production dependency DAG pass.
No new full dynamic qualification is claimed. Production, tests and assets
remain unchanged. Shared-rule vocabulary retains a separate Td receiver.

| Task | Progress |
| --- | --- |
| T556 S9 | Delivered for owner review: 592/592 units and 44 profile/MyNES integration executions per width, both specialized gate targets, documentation/dependency/manifest checks and ten release artifacts. T556 remains open. |

## Retained S9 Delivery

The [S9 delivery report](../etc/evidence/t556-s9-quality-repair-delivery.md)
records exact scope, actual diff, logs, artifact hashes and unperformed manual
acceptance. Four PC Apps retain 0.5.0556 under their flat asset roots; MyNES
retains its independent 0.0.0044 pair. The PC target is `vm-0-5-0556`.
MyNES receiving commits are `7fcd7b1d9` and `c5ca06e85`; the NXVM commit
containing this packet/report identifies its delivered source and eight EXEs.
Root preset names use a separate Shared-target commit; the public six are unchanged.

## Historical Evidence

Prior packets remain in the [retained status archive](../history/M5-T556-retained-status.md).
They are not concurrent active tasks. The [T556 history](../history/M5-T556-app-component-layout.md)
records accepted layout work, S11 closure and the explicit MyNES residual.
