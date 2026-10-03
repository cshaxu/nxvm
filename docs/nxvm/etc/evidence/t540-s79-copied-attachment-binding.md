# T540 S79 Copied Attachment Binding

## Assignment And Baseline

S78 P2 `f878753e1` admits the complete nineteen-callback receiver, not physical
relocation. The owner requested reusable Core and flat PC board integration
with unique state/dispatch/lifetime ownership. S79 consumes every callback in
that finite class; board-handle and test-owner relocation remain later work.
The executor confirms the CURRENT packet and the S78 contract before edits.

## Sole Production Mechanism

`attachment_interface.h` declares typed copied callback values and an opaque
context. Core retains one `core_machine_attachment`, replacing all nineteen
private callback fields and `board_owner`; no parallel compatibility fields
remain. Constructor code prepares one complete value and calls
`core_machine_bind_attachment()` once before fallible clock initialization.

Core validates context/finalization and its existing configuration-open
condition, rejects a second publication, then copies the value. Rejection
changes neither the current binding nor ownership. No runtime rebinding,
get-context operation, registry, extra clock or lifecycle queue is added.
Phase, shutdown and firmware callbacks now receive the same explicit context
as scheduler callbacks. The original chip bodies, sequence and time formulas
remain unchanged.

Core remains the lifetime owner: firmware context is cleared, attachment
finalization runs once, then CPU/FPU, routes, ROM/RAM and Core are released.
Board binding failure releases its unpublished allocation before destroying
the candidate; later failures use the established Core destructor. The board
finalizer no longer clears a private Core firmware slot. Firmware completion
still executes inside the existing publication/rollback transaction.

The current constructor context is the Core handle because the existing board
association still resides there. This is the explicitly retained S78 bridge,
not the final board-handle boundary. No public API exposes that association;
the six measured board owners and their direct tests still require the next
opaque-board receiver before Shared physical movement.

## Fixtures And Similar-Issue Sweep

The controller-authority test separates neutral phase-order instrumentation
from real controller construction/reset assertions. It verifies copied caller
storage, absent context/finalizer, null arguments, duplicate publication and
frozen publication with no prior binding. Reset/NMI/finalization callback
order and partial board cleanup remain covered.

The scheduler fixture substitutes one complete same-module instrumentation
bundle, forwarding production phases/firmware/shutdown with their original
context. Deadline probes are selected inside that fixture context, not by
overwriting a public binding or leaving phase callbacks with another owner's
pointer. D4 refresh-hold observes the real copied binding unchanged.

All tracked NXVM production/test/CMake callback declarations, publications,
calls and fixture substitutions were searched. Nine existing gates now inspect
the new contract. Controller authority checks every callback declaration and
constructor entry, the sole Core copy/guard, and the complete production tree
for old slots, direct binding writes and second production publishers.
An isolated positive source copy plus six injected negatives passes: old
slot, direct context assignment, second publisher, two private clock reads
and zero-time raw production port read are correctly rejected. The bounded
probe copy is removed after execution. Shared/MyNES have no attachment
consumer and are unchanged.

The full callback batch has one fixed disposition: all nineteen private slots
are replaced, not deferred. The five production consumers are board
construction, Core lifecycle, Core firmware, Core scheduler and CPU bus.
Sweep queries use `rg` over `src/app-nxvm`, `test/app-nxvm` and `cmake/nxvm`
for `->board_([a-z_]+_provider|owner)`, `core_machine_attachment`,
`core_machine_bind_attachment` and direct `->attachment` writes. Review
distinguishes legitimate same-module fixture substitution from production
publication. T317's Git inventory includes both cached and untracked source;
its explicit rerun covers the new public header (also staged before delivery).

## Size And Actual Source Review

Against S78 P2, `git diff --numstat` for NXVM source/test plus the new
83-line public header counts fourteen paths: +369/-218, net +151. Production
is +215/-157, net +58; tests are +154/-61, net +93. Documentation, artifacts
and nine build gates (+70/-26) are excluded from that source/test count.
The public contract and validation add durable ownership; test growth covers
rejected/copy publication and complete context-safe instrumentation, not a
second production path. No old type alias, slot or publication remains.

Executor hunk review covers every changed source, three fixtures, the new
contract and nine gates. Scheduler/CPU/firmware changes replace only dispatch
selection/context; phase ordering, guest time arithmetic and register bodies
are unchanged. Synthetic phase probes use neutral Core; real board/controller
assertions and the invalid-clock destruction case remain separately covered.
The scheduler uses private same-module instrumentation, restores the complete
production bundle before destruction and does not bypass the public bind guard
in any production caller.

## Complete Verification

Both final complete unit suites pass 470/470, with zero failed tests: x64
72.24 seconds and x86 73.73 seconds. Both complete specialized gates pass,
including T344's 402 rows (377 strict, 25 declared deferred). Six injected
negative checks and documentation governance pass. All eight product builds,
independent neutral-Core executions and one-shot real-INI boots pass. No boot
was launched before native unit execution ended. Each retained profile/width
row ran once; no retry budget substitutes for a checkpoint.

| Real INI profile | x64 checkpoint | x86 checkpoint |
| --- | --- | --- |
| Default PC/AT, 1.44 MB + HDD | `dos-prompt` | `dos-prompt` |
| IBM 5160 Model 268, 360 KB | `installer-running` | `installer-running` |
| IBM 5170 Model 339, 1.2 MB | `installer-running` | `installer-running` |
| DeskPro Model 40, 1.2 MB + HDD | `installer-running` | `installer-running` |

Ignored verification records are `build/s79-unit-{x64,x86}.log`,
`build/s79-gates-{x64,x86}.log`, the eight
`build/s79-<machine>-<width>-{build,neutral,boot}.log` files and
`build/s79-artifacts.log`. Complete units/gates used the existing S79 unit
harness; each product executed its neutral-link test and the existing
`vm-profile-floppy-boot-matrix` with its actual unchanged INI and 180000 ms
containment. Checkpoints, not that timeout, are the success predicate. Final
diff, local links, scope and artifact-hash checks pass. This bounded proof
does not establish indefinite absence of intermittent faults or T closure.

One early build used a nonexistent shortened target name; corrected targets
were rebuilt before the four executions. A complete-unit launch overlapped
the still-live specialized build and exited on Ninja dependency-log access;
that terminal failed launch is not verification evidence. The known original
build handle was polled to exit zero, then complete units were relaunched
serially in their own trees. No unrelated process was stopped or tree reset.

The early product harness used Windows PowerShell and misclassified CMake's
normal stderr architecture-OK message as a native-command failure. Its handle
and build processes were confirmed terminated before relaunching the existing
script under PowerShell 7; no source workaround was added. After final fixture
cleanup, x64 complete units/gates were explicitly reverified; x86's build had
already consumed that final source. Native desktop suites did not overlap each
other or the boot runs. Separate product trees may compile concurrently.

## Product Artifact Identity

All eight product targets were rebuilt from S78 P2 plus this S79 source
delivery, using their existing Release recipes and approved embedded BYOB
inputs. PE width, absence of `.debug` sections, `0.5.0540` banner and freshness
against the attachment input are checked. These are current developer/product
artifacts, not release or complete hardware/timing qualification evidence.
Their implementation source commit is the S79 P1 recorded at acceptance.

| Product path | Bytes | SHA-256 |
| --- | ---: | --- |
| `assets/nxvm/compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x64.exe` | 1335479 | `D39478EC41CC8DE241FDD454D8D296A9CD0311644D6B04C88CCF15C4FC7DB66F` |
| `assets/nxvm/compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x86.exe` | 1505286 | `5BA1E395407597E9720760B18046F13238648D95CE5CD44C875DA2BBF31A60BC` |
| `assets/nxvm/default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x64.exe` | 1351796 | `CF43D9116F4B491FB54D580E30DDC39CE9EC82B0DB8A3261834943EBC30504BC` |
| `assets/nxvm/default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x86.exe` | 1521601 | `5A35FE8910F7300392BF1A15865097E29F7BBB2E32A399B6A445D617C24B6CDD` |
| `assets/nxvm/ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x64.exe` | 1351764 | `3782A80B5D50F1237AF76D63AA057AEA8F39DD37C08F7671B41B7A52D49D5310` |
| `assets/nxvm/ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x86.exe` | 1521568 | `FE82889FF5FD29CEAF258059F73662DEC33B0DF4B7E9A4797D1DA5DE7A3EFB9F` |
| `assets/nxvm/ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x64.exe` | 1351830 | `F04C8CC09C70E15E61769543C4341544CA7558EFE2CC9AE267D470F5724D5AFD` |
| `assets/nxvm/ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x86.exe` | 1521636 | `85047F340318D81C78ECF154673F3A174A315FC5F81625257D192779DF9FF1B6` |

Owner INI contents are unchanged. Their media entries all use overlay mode;
external masters are not written or copied into the repository. No protected
original, generated firmware byte source or MyNES artifact is imported.
Ignored incremental trees and compact logs remain needed by the immediate
opaque-board receiver; the isolated negative copy is already removed. No
recording or unbounded trace is produced.
