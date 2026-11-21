# OB300 implementation design from T588

This is the planned OB300 target, not current implemented hardware. It refines
[T588](m5-ob300-design.md); Current owns admission and delivery status.
The [S1 ledger](../etc/evidence/t588-ob300-convergence.md) and
[S2 proof](../etc/evidence/t588-s2-reset-proof.md) separate facts from hypotheses.

## Product and component shape

Use `MyOB300`, `src/app-myob300`, `test/app-myob300`, `assets/myob300`.
Follow the flat MyPC110 App pattern, with cohesive files rather than invented
subcomponent directories. Do not create empty future files.

| App file responsibility | Contract |
| --- | --- |
| main | Fixed product name, myob300.ini and one existing X86-NXVM Product entry |
| product_binding/context | Prepare fixed construction/assets, register genuine extra commands only when needed, save App-owned persistent configuration |
| ob300_plan | Own HP wiring and lifetime; build one candidate, publish through existing Machine construction; unwind failed construction once |
| ob300_config | Validate supported memory/media choices; use the existing INI grammar, no App-selected CPU or alternate machine loader |
| ob300_firmware | Validate external fixed System Card identity; install/reset board-owned windows using existing Core routes |
| ob300_system_card | Own common/attribute decode and bank registers; keep immutable source bytes and mutable decode state separate |
| ob300_platform | Own HP control registers, including the evidenced FBh route, RAM/window and board signal connections |
| ob300_display/input/power | Only HP connection/panel/control behavior; reusable chip semantics stay with their actual owner |

App symbols use `myob300_` with responsibility-specific suffixes. Headers expose
only what other App files need; do not add a public getter to expose private
registers. `test/app-myob300/unit` covers owned state/transactions;
`integration` covers external ROM and real component composition; a `diagnostic`
file exists only for a retained diagnostic, not as a second implementation.
Documentation, build tooling and MTSP remain in the NXVM family sequence.
Final artifacts are one current x86/x64 pair and myob300.ini directly under
assets/myob300. Versioning follows the active implementation task, not T588
permanently. No build or empty App scaffold is part of this design delivery.

## Single execution and state ownership

X86 Core owns CPU/FPU, the existing executor, memory/port dispatch and guest
clock. X86-NXVM owns reusable peripherals/board mechanisms, Machine adaptation
and family Product. X86-IBMPC and X86 Debug retain their existing interaction
and Debug roles. Emulator retains Session, lifecycle, generic commands and UI
composition. Lib remains the sole host window/input/audio/storage service.

The App imports no private Machine/CPU layout. Construction uses the existing
plan, provider and profile-binding contracts, as PC110 does. Device callbacks
run on the sole executor clock; no second worker, polling thread or copied
CPU state machine. App card/register writes update one decode owner and use
the existing route; no CPU-PC-specific interception, BIOS-call replacement
or host FFS/Windows execution. Shared code never branches on MyOB300 identity.

start/stop/pause/resume/reset retain Emulator's fixed semantics. Reset resets
volatile HP registers and bank selection, not persistent disk contents or the
immutable card. Stop/start re-enters original firmware; no special fast boot.
Construction owns loaded external bytes until its existing destruction path
has stopped execution and released borrowers. Required asset failure returns
the existing status before publishing a candidate.

## Hardware decisions and limits

The following is a finite implementation contract, not a register specification
invented from a schematic picture. Unknown values receive explicit L1 models
only after the implementation batch identifies their observable use. A manual
constant/formula is L3; an external reference model is L2. Do not downgrade an
existing accurate shared device or silently call a generic CPU SXLV-exact.

| Ledger | Design disposition and owner | Required receiving proof |
| --- | --- | --- |
| O01 CPU | Reuse the 80386 executor as the initial functional research baseline, no FPU. A board-compatible 386SXLV selection needs a documented bus/address/reset difference audit in X86 Core before production admission; do not rename 486SX | ROM DOS: CPU identity/reset/bus-width decision and old-profile regression if Shared changes |
| O02 reset | App firmware owns mappings; Core alias API is sufficient for the demonstrated first transfer. Diagnostic whole-F mapping is not final | ROM DOS: original F000:E05B entry, final BIOS/VGA split, reset and mapping-boundary regression |
| O03 card | App owns System Card windows/banks/CIS; master remains immutable. No assumption that archive length equals physical decoder width | ROM DOS: bank read/fetch boundaries, reset bank, attribute/common distinction, observed writes and read-only behavior |
| O04 RAM | Existing Core provider/decode mechanism; HP selection registers App-local. Start research with S2's 2 MiB, validate accepted product sizes from local manual | ROM DOS: RAM/ROM transitions, aliases, cross-boundary access; review known masked-span debt if route uses it, no blanket waiver |
| O05 board | Reuse PIC/PIT/RTC/DMA and other compatible mechanisms, App owns HP port decode and signal wiring; FBh meaning remains to be traced | ROM DOS: original board-init batch, interrupts/timer progress, reset ordering; no silent successful unknown-port writes |
| O06 display | C&T 65510 semantics are a distinct chip requirement, not renamed F65535. Its reusable core belongs in X86-NXVM chips after scoped review; panel wiring belongs to App | ROM DOS: original video BIOS, text/graphics register behavior, VGA RAM mapping and 640x480 frame; Windows: vgarom3 drawing |
| O07 input | App connects compatible keyboard/mouse mechanisms after tracing HP routes; use existing neutral KVM events and Machine admission | ROM DOS: real make/break and pause/reset; Windows: pointer and application interaction |
| O08 power | HP control/power registers App-local; one guest-time model, explicit L1/L2 where necessary | ROM DOS: boot readiness and reset; product qualification: suspend/resume and persistence, with any unsupported mode explicit |
| O09 C storage | Model guest-visible PCMCIA storage, not host files. Select a supported disk/controller route from actual firmware; use the matching C-file backup to populate disposable media | ROM DOS: partition/boot setup documented, C/D directory and read/write/readback; no claim file ZIP is bootable raw media |
| O10 software | Execute original ROM DOS and split ROM Windows plus OBSETUP/C files, no replacement system | Windows: original loader reaches interactive Program Manager, with persistent C-drive operations; matching backup preserved |
| O11 performance | Preserve one timing path; profile actual workload after missing hardware waits are resolved | Performance: numeric gates below; any Shared optimization requires existing consumers' qualification |
| O12 product | Reuse family Product, Emulator lifecycle and existing artifact conventions | Product qualification: both widths, clean launch/error, reset/resume, stop/start, media safety and reproducible BYOB release |

The first implementation should cover the initial board/control/card batch,
not repair one unsupported port per S indefinitely. Collect the reachable
initialization accesses and their dependencies before choosing a batch. A
bounded diagnostic may record unknown ports, but invented successful replies
are not qualification evidence. Missing hardware documentation is not an
indefinite blocker: state a minimal observable L1/L2 contract and test it.

## Asset and publication contract

System ROM 1.0 ABA is the fixed card; 1.01 and 425/430 are not runtime variants.
The full card contains Microsoft software and stays external. Do not embed it
in a committed EXE. App firmware loads the fixed external BYOB asset at prepare
time and verifies the S1 size/hash, using existing storage and path capabilities.
No fallback image or guessed filename. Use a portable path relative to the
EXE directory, for example the external sibling collection's
`media-nxvm/ob300/system10-aba-common.bin`, not a machine-local drive path.
The exact relative deployment path belongs to the implementation asset manifest
and App binding; do not add firmware selection to the shared INI grammar.

Keep owner masters untouched. Build disposable writable C media from the
matching file backup using an explicit geometry/filesystem/controller plan;
retain source hash, creation recipe and access mode. INI selects writable
guest media through existing semantics. It must not change CPU identity or
silently replace the System Card. No requirement to extract files from the
card at runtime: guest firmware sees the original bytes.

## Usability gates and measurement

These are engineering acceptance targets chosen under the owner's autonomous
goal-setting direction, not HP timing constants or predicted current results.
S2 establishes that the existing executor runs and gives a controlled cost
envelope; it cannot establish full guest speed. Hardware readiness and spin
waits must be separated from host instruction throughput.

| Measurement on the same recorded host | Target for each host width |
| --- | --- |
| start accepted to interactive ROM DOS, with fixed clean C media | at most 60 seconds x64; 90 seconds x86 |
| DOS key accepted to visible echo, ten scripted keys after boot | each at most 250 ms; record individual samples |
| simple DIR command to next ready prompt on the fixed C image | at most 1 second |
| normal pause accepted to completed paused state | at most 1 second |
| reset/resume and stop/start on the same defined media | reach the same usable DOS checkpoint within the respective boot budget |

Use cached Release, fixed CPU/model/media, and record host, commit, media hashes,
retired work, guest ticks, stage times and chosen pacing policy. Measure input
at Machine admission and frame publication separately from native paint; report
desktop/RDP effects rather than calling either measurement the other. One
complete run per width is the qualification baseline; repeat only on new
evidence, not automatically three times. Fixed-work synthetic CPU tests do not
substitute for these guest tests. Windows budgets are frozen by its receiving
task before qualification, including Windows launch and desktop interaction.

ROM DOS first proves correctness and records any budget shortfall. The next
performance T must meet these gates or seek an explicit scope/budget decision
before full Windows qualification; it may not silently weaken them. No JIT,
timing downgrade or App fast path is a prerequisite. Prefer measured duplicate
work/complexity reductions over a new framework.

## T588 completion boundary

The twelve domains now each have an owner, honest evidence/model limit and
named receiving proof. S2 supplied the real reset experiment and fixed-work
baseline. This completes the design deliverable, not hardware implementation.
The first ROM DOS receiver must resolve HP registers and card decode; the
remaining performance, Windows and final qualification receivers remain queued.
No unresolved domain is declared non-applicable merely to close this design T.
