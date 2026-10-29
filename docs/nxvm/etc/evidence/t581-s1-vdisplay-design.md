# T581 S1 — PC110 stopped-state display selection

## Scope and ownership

The owner admitted the first queued task on 2026-10-29. The starting revision
was `e769bc579` (T580 closed after accepted S90). This task implements one
MyPC110 product with DSTN default and `vdisplay=dstn|tft` while STOPPED.
Selection is process-local, not an INI field or a remembered startup choice.
Both BIOS inputs are the already admitted BYOB images; no new ROM, copied
third-party source or model accuracy claim is introduced.

The App owns the command, selected/active variant, immutable firmware references
and two CMOS images. `requested` is the control-to-worker atomic value; `active`
and CMOS images are worker-owned until shutdown. Session remains the sole
monitor/control dispatcher. The extension checks both Session and Machine
STOPPED facts and never mutates live firmware. It rejects running, paused,
reset-completed, starting and error states. Shared code knows no panel names.

The existing Machine worker invokes the optional profile `before_reset` hook
before its normal cold reset. Construction's initial reset uses the already
prepared DSTN assets; it does not invoke this worker hook. The App captures
current CMOS/RTC, stages the candidate BIOS, verifies its reset bank and changes
the existing provider view with decode invalidation. The same board then restores
the selected configuration and optional valid RTC image before cold reset.
There is no replacement Machine, medium, worker, display path or clock.

The two validated BIOS files differ at 18 bytes in E0000--E3FFF. The immutable
F0000/high reset bank is identical. Selection explicitly rejects a different
reset bank. The S90 L1 POST compatibility patch is applied by the same copy
helper for initial construction and later selection; its meaning is unchanged.
A failed allocation/validation leaves the old ROM view untouched. After capture
has validated this same extended-RTC board, restoration has exact fixed sizes
and infallible register writes; no external callback can change the board
between these steps on the sole executor. An unexpected hook failure follows
the existing Machine ERROR path rather than claiming a completed reset.

Generic X86-NXVM additions are limited to the profile pre-reset hook, CMOS
restore symmetric to capture, and optional Product construction-context and
extension callbacks. Factory attachment failure releases the prepared profile
once and never constructs a Machine. Configuration save receives the same
borrowed run-scoped context after shutdown/unbinding. Existing bindings with
absent callbacks keep their original route. Lib, Emulator, X86 Core, X86 Debug
and X86 IBMPC source/test trees are unchanged.

The initial prepare retains the existing executable-adjacent CMOS loader;
the run context loads both images for future switching. Active state is captured
from the board at each actual switch and shutdown, not inferred from an old
file. Only dirty variants are saved on normal teardown. The original
`mypc110_cmos_save` remains used by existing diagnostics and tests; both it and
the new saved-image path share the single binary writer.

## Build and test boundary review

The board library does not link embedded BIOS payloads. App composition supplies
borrowed immutable variant references and owns the selectable-firmware library.
The common Kanji font is embedded only once. The original single-variant
firmware targets remain for the existing external-firmware diagnostics; they
are not another product execution path or extra released EXEs.

Two tests depending on real BIOS bytes are under App `integration`, not unit:
`firmware_variants_smoke.c` and `product_composition_smoke.c`. No new test
framework, production test entry point, thread or polling recovery is added.
Synthetic command and Shared owner tests remain independently buildable units.

## Requirement-to-proof

| Requirement | Actual proof |
| --- | --- |
| DSTN default; only stopped accepts selection | App command test covers parsing, both choices, unknown/invalid text, Session states and stale STOPPED versus actual STARTING. Real-process CMOS tests send the actual command. |
| Provider/fetch and reset alias agree | Real BIOS integration compares differing bytes, tests instruction execution after route invalidation, rejects incompatible reset banks without mutation and resets shadow to the selected view. |
| Single Machine and repeat switching | Real App composition attaches the runtime, performs four alternating driver resets and checks stable Machine identity and active/dirty variants. |
| Failure ownership | Existing Shared factory test covers attach failure/no creation/one release; construction test covers hook failure/no core reset/no destruction followed by successful retry. |
| CMOS and RTC stay separate | Real product process writes different CMOS values for DSTN/TFT, switches back and reads the earlier value, then checks both saved files. Existing calendar save/reload and lifecycle checks remain. |
| Media retained | App integration retains an exclusive DIRECT lease through repeated selections and releases it only on destruction. Existing Shared disk test writes dirty overlay bytes, resets through the same hook and checks object identity and retained bytes. No media close/open operation exists in the selection path. |
| Both firmware images in two EXEs | Artifact checks verify both complete BIOS byte sequences, common font, PE width, built/deployed identity and rejection of an absent firmware fixture. |
| Normal receivers retained | Full dual-width unit suites plus real PC Product entry/INI/info/speed/help checks and default-PC lifecycle/full-session integration. |

The final unit suites passed **712/712 on x64 and 712/712 on x86** (70.92 and
73.34 seconds). The earlier 714 count included the two real-ROM tests now
correctly classified as integration; no coverage was removed.
Nine targeted integration cases per width passed: the six PC110 product,
CMOS/firmware/artifact cases and three default-PC lifecycle/session/entry
cases. My5160, My5170 and MyDeskPro386 also passed the existing real-process
INI/standard-command receiver on both widths, using the deployed EXEs and
isolated test configuration.

The three existing real-firmware integrations passed on both widths:
FDC reset, resume boot handoff and cold boot handoff (x64 223.69 seconds;
x86 267.56 seconds combined).

A task-owned public-factory probe used the actual Machine worker, configured
CMOS copies and overlay disk copies. In each architecture it started DSTN,
stopped the same Machine, submitted `vdisplay=tft` and started again. Each
variant received 36 ten-second frame observations, without key injection.
All four final frames show the complete Personaware menu and have SHA256
`85A35680232CA0AD623DB7DD18D23A79396931ACB133791581D40C37048AEF41`.
Both probes exited successfully. Master and slave file hashes remained equal
to their inputs (`F27518A03119C26D0CA2721CBC14935E764E2F7D7F2DE91A2EC91CA12E04793B`
and `5149DB391D13CFEAB330016FCF0EDBE6B0D379CBB66A3AED91DBF7684142D52E`).
This is actual guest execution through the production factory and frame API;
it is not a native Window/RDP test or a claim about booting empty CMOS.

Temporary evidence remains in the active task-owned `build/t581` until owner
acceptance: probe source, final frames and bounded run logs. All owned build,
test and probe processes have exited. No background retry or monitor remains.
The two stable Ninja trees and project ccache are retained for reuse.

## Release and review

Use the existing cached Ninja x64/x86 trees and `-j 8`; no task-specific build
tree was created. All five PC products share version **0.5.0581**. The four
existing PC Apps use their versioned names; PC110 retains `mypc110_x86.exe`
and `mypc110_x64.exe`. Both obsolete TFT EXEs and replaced 0580 PC pairs are
removed only after replacement artifact checks. MyNES/MyRaidenII EXEs are
unaffected. Owner INI, CMOS, disks and snapshots remain unmodified.

[Artifact hashes](t581-s1-artifacts.json) identify the ten rebuilt/deployed
EXEs. Source/test changes are confined to MyPC110 and X86-NXVM; root presets
and release checks follow the admitted product identity. Shared and NXVM
changes are delivered in separate target-scoped P commits.

The coordinator reviewed actual source, tests, build graph, manifest and docs,
not just test totals. Existing APIs were reused for media, storage, lifecycle,
frame snapshots and debug. The generic hook and context are the necessary
ownership seam; panel policy stays in the App. No Lib/Emulator changes or
unrelated formatting sweep is included.

Native Window/RDP rendering and human input acceptance are not established by
background frame capture. Empty DSTN CMOS retains its previous first-setup
behavior; tests using configured copies do not certify automatic boot from
empty CMOS. T580's already transferred timing, hardware and guest qualification
limits remain unchanged. Owner acceptance is required before closing S1/T581.
