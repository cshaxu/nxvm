# M5 T592: OmniBook 300 product-stage closure

The owner directed T592 closure and admission of performance investigation
on 2026-11-21. S1 through S3 are closed for the bounded product stage.
[S1 integrated the App](M5-T592-S1-ob300-product-entry.md) and
[S2 qualified the native release](M5-T592-S2-ob300-native-qualification.md).
The coordinator reviewed S3 through `597aad29d`, including the final
App-only path change and the hashes of the final deployed pair.

| Product obligation | Disposition |
| --- | --- |
| Launch and working setup on both widths | The [release and external setup record](../etc/evidence/t592-s3-ob300-local-setup.md#colocated-production-system-card-and-c-image) identifies the deployed 0.5.0592 pair and verifies direct startup with the colocated System Card and C image. |
| Native lifecycle, media, and input | The [S3 replay record](../etc/evidence/t592-s3-ob300-local-setup.md) covers Console and Window lifecycle, CMOS restart, media leases and DOS writes. The owner verified menu navigation, exiting Windows to DOS, and mouse movement. |
| Power switch | Original DOS switch and wake are proven on both widths. The [Program Manager probe](../etc/evidence/t592-s3-ob300-local-setup.md#reported-no-response-in-program-manager) proves that an addressed x64 Window chord reaches the board and generates one SMI after guest input is responsive. |
| Release qualification | The Shared hotkey release passed 717 repository-only unit tests on each width and all registered integration cases after the documented isolated retries. The later App-only path update passed its board and LF checks and direct launches on both widths. Closure changes no runtime or build input and retains the published pair. |

## Open receivers

- [T593 performance](../proposals/m5-ob300-performance.md) receives the original
  DOS boot and DIR budget misses, interactive Windows readiness latency, and the
  time from a delivered switch event to a visible power response. The 50.156 s
  between observed Program Manager and the first observed File selection is an
  interval between checkpoints, not an isolated input-latency measurement.
- The [ROM Windows receiver](../proposals/m5-ob300-rom-windows.md) retains final
  original installation and persistence, native user/RDP endpoint verification,
  and original Windows Standard/Off follow-through on x86. Word and Excel are
  outside the owner's acceptance scope. Existing physical-panel, detachable
  controls and PC Card limitations retain the boundaries in the S3 ledger.

The hotkey report remains a latency lead. The injected Window probe did not
exercise the owner's RDP delivery or wait for visible screen change. Closure
does not establish the cause of the owner's observed response.

[Archived proposal](M5-T592-ob300-product-qualification-proposal.md).

## Archived S3 packet

# M5 T592 S3 - Active: OmniBook product acceptance and remaining limits

[S2 is accepted and closed](M5-T592-S2-ob300-native-qualification.md).
The 0.5.0592 x64/x86 Product reaches original ROM DOS through native Console.

S3 has a rebuilt dual-width candidate. The original 2 MiB guest passed its
former Windows memory error and both widths drew Program Manager. The
[original Windows evidence](../etc/evidence/t592-s3-ob300-rom-windows-baseline.md)
records the mapping, protected-mode and RTC fixes, and distinguishes the
captured desktop from input and application qualification. The
[external media record](../etc/evidence/t592-s3-ob300-local-setup.md)
covers repeatable C-drive preparation and native process entry.

The owner verified Windows menu navigation, exit to DOS and usable mouse input. The
[pop-out mouse evidence](../etc/evidence/t592-s3-ob300-popout-mouse.md)
records the fixed OEM input path and an original guest frame after motion.
The owner removed Word and Excel from the later [ROM Windows acceptance
scope](../proposals/m5-ob300-rom-windows.md).

Both published widths now wake original DOS APM through Ctrl+Alt+O.
Both widths display the chord in Monitor help and the Window Console
status page. See the [product hotkey verification](../etc/evidence/t592-s3-ob300-local-setup.md#product-hotkey-extensions-and-native-help).
A [dual-width DOS switch observation](../etc/evidence/t592-s3-ob300-local-setup.md#native-dos-switch-press-and-wake)
also records one real SMI, the original firmware wake poll and fresh VER.
The [replacement binding evidence](../etc/evidence/t592-s3-ob300-local-setup.md#ctrlalto-power-switch-binding)
records the native Console input route. The prior release proved the original DOS power cycle through F13
input. The [power evidence](../etc/evidence/t592-s3-ob300-local-setup.md#production-f13-suspend-and-wake)
records new DOS command execution after wake and the remaining power limits.
Both widths also restore the original Windows Program Manager in the same
run generation and respond to a fresh Alt+F after F13 wake. See the [original
Windows power checkpoint](../etc/evidence/t592-s3-ob300-local-setup.md#original-windows-power-checkpoint)
for the replay limits. Final Windows installation, persistence and performance
qualification remain with their queued receivers.

The [configuration restart replay](../etc/evidence/t592-s3-ob300-local-setup.md#configuration-warning-after-process-restart)
now passes through the published product on both widths. Clean monitor exit
saves the firmware's RTC and CMOS beside the EXE. A second process loads the
file and reaches original ROM DOS without F1. The evidence also covers
malformed input and a refused save.

The [disk-sharing replay](../etc/evidence/t592-s3-ob300-local-setup.md#published-windows-disk-sharing)
confirms dual-width Windows denial of conflicting C-image access and release
at monitor exit in readonly, overlay and direct modes.
The [published DOS write replay](../etc/evidence/t592-s3-ob300-local-setup.md#published-dos-writes-in-overlay-and-direct-modes)
verifies overlay isolation and direct persistence on both widths.

The [native Window replay](../etc/evidence/t592-s3-ob300-local-setup.md#published-native-window-lifecycle)
verifies paint, pause freeze, resume in the same window and normal exit of
both releases on a private local desktop. The [published Program Manager
captures](../etc/evidence/t592-s3-ob300-local-setup.md#original-program-manager-on-the-published-window-path)
also show original Windows drawing through that route on both widths.
The [x64 Window input replay](../etc/evidence/t592-s3-ob300-local-setup.md#published-x64-window-keyboard-response)
observed the guest File menu selection and clean exit after original Windows.
The [x86 follow-up](../etc/evidence/t592-s3-ob300-local-setup.md#published-x86-guest-selected-file-label)
also captured a guest File selection, populated Main group and clean exit.
The addressed messages leave physical input focus and RDP outside that proof.

The [published DOS handoff](../etc/evidence/t592-s3-ob300-local-setup.md#published-dos-performance-handoff)
measures current native startup and directory response on both widths.
Direct dual-width sampling places branch-target decode and board deadline
observation at comparable cost. Neither region alone explains the DIR gap.
A [scratch preview prototype](../etc/evidence/t592-s3-ob300-local-setup.md#read-only-preview-boundary-prototype)
cut the cold preview sample in one pair without improving DIR latency.
[Call-site counters](../etc/evidence/t592-s3-ob300-local-setup.md#timing-versus-observation-preview-calls)
attribute every sampled workload preview to the timing formula, with no
additional target read for retirement metadata.
The [fractional clock prototype](../etc/evidence/t592-s3-ob300-local-setup.md#exact-fractional-clock-prototype)
reduced conversion cost in isolation and in one original DOS x86 pair.
The [original
Power control replay](../etc/evidence/t592-s3-ob300-local-setup.md#original-windows-power-control-path)
confirms that x64 Standard and Off both run POST and load DOS after two F13
presses without needing F1. On x86, both modes enter a new POST and pass the
extended CMOS check. A shorter disposable x86 Standard startup again
verified POST but did not reach the DOS driver under the existing cap.
The [aligned wake traces](../etc/evidence/t592-s3-ob300-local-setup.md#cross-width-restart-trace-comparison)
match through every captured x86 ordinary POST CMOS operation. The
later x86 DOS endpoint still lacks direct evidence.
The [published x86 acceptance procedure](../etc/evidence/t592-s3-ob300-local-setup.md#published-x86-windows-power-acceptance-procedure)
specifies the observable Standard and Off endpoints and RDP evidence.
The replacement chord passes x64 Windows [Standard](../etc/evidence/t592-s3-ob300-local-setup.md#windows-standard-with-the-replacement-power-chord)
and [Off](../etc/evidence/t592-s3-ob300-local-setup.md#windows-off-with-the-replacement-power-chord)
through fresh POST and DOS driver load in the instrumented guest replays.
The [x86 Standard replay](../etc/evidence/t592-s3-ob300-local-setup.md#x86-windows-standard-with-the-replacement-power-chord)
confirmed the new keyboard chord and restarted firmware CMOS reads.
The queued performance and Windows gates remain in force.

| Field | Required record |
| --- | --- |
| Identifier Mode | M5 T592 S3 after ordered S2 publication. |
| Admission And Approval | Continue the owner's ordered OmniBook implementation and single-session coordinator/executor review. Necessary X86-NXVM and X86 Core changes are owner-approved. The owner also approved the optional X86-IBMPC Product hotkey extension and MyOB300 help entry. |
| Objective | Reconcile remaining power, peripheral, storage failure, desktop and documented setup obligations against the original Machine and released Product. Close confirmed gaps within scope and record explicit acceptance limits. Owner requests replacing the MyOB300 F13 power binding with Ctrl+Alt+O for manual acceptance. Owner also requests colocating the external System Card and test C image under media-nxvm/ob300. |
| Non-goals | No firmware bytes in Git, duplicate release or input route, speculative Windows fix, or claim of unobserved power behavior. |
| Reference Baseline | S2 published 0.5.0592 and S3/S4 of T591 for Machine lifecycle, C persistence and timing. |
| Candidate Proposal | proposals/m5-ob300-product-qualification.md and m5-ob300-architecture.md, especially O07-O09 and O12. |
| Files And ABI Surface | MyOB300 App, assets guidance, corresponding fixtures and NXVM evidence. X86-NXVM or X86 Core only when a confirmed defect requires their ownership. The hotkey delivery affects Shared X86-IBMPC Product and its tests, MyOB300, and Release qualification for all eight Apps. |
| Applicable Rules | NXVM guide, Contributing, Roadmap, Architecture/Layout, source and UI policy, shared Execution/Documentation/Architecture/Coding. |
| Verification | Review the product/device ledger, repeat actual DOS lifecycle across both widths where not already covered, cover detectable storage failures and collect native Window and power evidence when supported. Keep performance and Windows qualification separate. |
| Expected Markers | Proven supported behavior, explicit unsupported capabilities and a repeatable setup. No unnecessary receiver rebuild if only evidence changes. |
| Asset Needs | Read-only owner card/backup and disposable C medium. build/t592-s3 owns transient probes, capped at 200 MiB and 300 seconds per probe. |
| Reporting Requirements | Separate compiled, observed, limited and unverified capabilities. Record any scope decision required for T592 acceptance. For the replacement binding verify neutral input, guest-visible switch levels, both published widths, and existing App regressions. |
| Stop Conditions | Do not deem original guest suspend, native desktop UX or Windows qualified by monitor pause or raw Console alone. |
| Exit Criteria | Every T592 product obligation has direct evidence or an explicitly accepted limitation, with application and build instructions agreeing with the released pair. |
| Original Owner Request | Complete the OmniBook in hardware, board, Machine, Product and verification order. |
| Similar-Issue Sweep | Guest suspend versus monitor pause, reset/stop across different media states, clean failure, display routing and BYOB ownership. |
