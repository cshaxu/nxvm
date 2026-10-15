# T561 S2 Core Disposition And Contract Review

## Baseline And Proof Domain

Frozen input is `8db742b6d`, after accepted S1. The inventory accounts for
197 source-package files and 482 test-package files, including headers,
fixtures, CMake, checkers and manifests: **679 paths**, not just compiled C.
No source, test, build definition, public ABI or executable is changed by S2.
The five imported source/test pairs remain the accepted S1 prerequisite;
SoftPC's later worktree is not a moving extraction baseline.

This is the complete migration ownership inventory, not a new opcode semantic
audit, test qualification claim, runtime test selection or implemented split.
It does not supersede Current, the architecture authority or T560's CPU ledger.

Reproduce from the repository root:

```text
python docs/nxvm/etc/evidence/t561-s2/inventory.py --check
```

Python 3 and Universal Ctags 6.2.1 are used. The script reads Git-tracked inputs,
hashes actual bytes and rejects Core changes relative to the frozen baseline.
It reads both original Release Ninja graphs and their reachable generated
CTestTestfile.cmake registrations; it does
not configure another graph, compile, execute a test or touch the cache.
After extraction starts this historical check intentionally stops matching;
the frozen baseline remains reproducible through Git, not a permanent gate.

| Evidence | Meaning and limitations |
| --- | --- |
| [files.tsv](files.tsv) | Every tracked source/test path, SHA-256, line count, destination and decision. No path omitted by extension. |
| [symbols.tsv](symbols.tsv) | 15,158 Ctags declaration/definition records, including macros, enum values, types, members, functions, prototypes and variables; 2,872 occur in public interface headers. Repeated declarations and conditional definitions are intentionally retained with file/line/scope. These are not 15,158 unique exported APIs. Locals, parameters, comments and string words are not API symbols. Ctags is a lexical inventory, not a compiler or proof of semantic equivalence. |
| [includes.tsv](includes.tsv) | 2,326 quoted includes, including relative fixture includes. Receivers must resolve at their assigned owner; an include's spelling is not dependency approval. |
| [registrations.tsv](registrations.tsv) | 2,099 complete Core-related CMake statements, including declarations, aliases, conditions' bodies, source/link additions, test registration, generated dependencies and custom targets. Symbolic loop/macro arguments are preserved, not falsely presented as interpreted CMake. |
| [receivers.tsv](receivers.tsv) | 1,858 external source/test/build/tool reference lines to Core paths/targets. These remain edits of existing receiving owners, not files to import into a lower package. |
| [configured-targets.tsv](configured-targets.tsv) | 460 compiled targets per width with actual Core source/test object edges, destination owner and production-target rename. App targets using Core fixture objects remain App targets. Utility, alias and source-free declarations are in registrations.tsv. |
| [configured-dependencies.tsv](configured-dependencies.tsv) | The same 460 targets per width with Ninja order-dependency closures and explicitly marked App links. This is a generated dependency witness, not a claim that every transitive edge is a direct CMake link. |
| [configured-tests.tsv](configured-tests.tsv) | 656 currently registered cases per width, with exact names and labels; includes unit and non-unit routes. Listing is not execution. Unchanged Lib/Emulator/MyNES cases are included to make the configured universe explicit, not newly assigned to Core. |

The configured graph witness is the restored default profile. XT/AT/DeskPro
conditional registrations are retained by the complete source-statement index;
they are not claimed to have been separately reconfigured or executed in S2.
Future acceptance must compare all four configured profiles and both widths.
The S1 621-unit executions and its profile integrations remain the runnable
baseline, not tests rerun by this documentation prerequisite.

## File Dispositions

Destination braces denote explicit infrastructure partition, not an unassigned
file or permission to copy two implementations. New mixed-file fragments below
are additional receivers; the neutral remainder retains its listed destination.

| Decision | Files | Exact action |
| --- | --- | --- |
| C1 | 114 | CPU's eight source/header files and 106 matching test files become x86-core/cpu. Keep all CPU profiles, instruction tables, timing models and current warning exceptions. Two CPU tests currently include device_support_interface.h: replace their bit/mask spelling with equivalent Types expressions, not a CPU-to-machine dependency or copied compatibility header. |
| C2 | 4 | Three FPU files and its contract test become x86-core/fpu. Preserve the actual CPU-to-FPU dependency and partial-x87 qualification. |
| N1 | 82 | 35 execution files and 47 neutral-machine test files become x86-core/machine, after P2--P8 separation below. Do not wholesale reclassify their present public contract as neutral. |
| T1 | 6 | exception, reset-alias, boot and memory-registration fixture files become test/x86-core/machine/support. Private Core storage remains usable only by its own owner tests. Higher board tests must use their own board setup/public observation, never import these private fixtures across the new package. |
| T2 | 2 | The 80286 and 80386 timing manifest runners currently in test/core/x86 include the real board constructor; keep their board-composition qualification in test/x86-nxvm/board-base/qualification. Their CPU timing source keys/grade assertions remain unchanged. Do not pretend they are standalone CPU tests. |
| T3 | 2 | video_topology_fixture.c/h remain PC board test support, under test/x86-nxvm/board-base/support. |
| P1 | 448 | All remaining chip/board/machine/product source and corresponding tests move to x86-nxvm at the same relative owner. The 12 non-CPU/FPU chip leaves retain independent targets and interfaces. setup/integration remain external-asset harnesses, not independent-unit fixtures. |
| G1 | 6 | Address-span, decode-admission, host-arithmetic and stack-frame checks/negative checks follow CPU/execution into test/x86-core. Repair their scanned root and source owner, retaining their violation predicates. |
| G2 | 6 | CPU-bus/FDC composition negatives, family DAG, App support and route-partition checks follow NXVM integration into test/x86-nxvm; root infrastructure remains responsible for repository-wide routing. Independent package gates cannot scan peer Apps as if they owned them. |
| B1 | 7 | Partition root CMake, manifests, corpus/manifest checks and verifier-negative registration into the two package owners. Generate each manifest from its exact package root; remove old Core manifests only after both new inventories pass. Do not duplicate the entire former Core dependency policy into each package. |
| B2 | 2 | Partition x86_tests.cmake by actual assertion/link owner; board_tests.cmake belongs to NXVM tests. Replace old aggregate registration with the two independent entries, retaining every route and assertion. |

The current Core support location was correct for one package. The split creates
a new boundary: it does not justify placing lower fixtures in the higher package
or retaining a cross-package test-header shortcut. No fixture migration alone
proves that a test's actual setup is independent.

Tests under board-base/composition which execute CPU instructions using actual
PIC, DMA, memory wiring or a board reset remain PC integration unit tests. Their
existence does not remove CPU's own direct instruction tests. Do not delete
either category based on similar opcode names. Machine prefetch tests named
`d4_cecg_memory_class_contract` and `cecg_port_wait_contract` use only neutral
executor configuration: retain their range/wait assertions but use descriptive
range-filter names in x86-core, not a fictitious dependency on D4 hardware.

## Public Contract And Symbol Decisions

One spelling has one canonical owner. Forward declarations in another header
follow that canonical declaration, not the directory where Ctags found a repeat.
Members and opcode-local names need not be mechanically renamed. Header guards,
public prefixes, build targets and package tests must agree with their owner;
no retired Core-path alias is a permanent compatibility surface.

| Family | Receiving owner and namespace |
| --- | --- |
| core_machine_cpu_* / CORE_MACHINE_CPU_* | x86-core/cpu; x86_core_cpu_* / X86_CORE_CPU_*. Preserve enum values and instruction/timing tables. |
| x86_fpu_* / X86_FPU_* | x86-core/fpu; existing stable FPU prefix already names the capability and can remain. |
| remaining neutral core_machine_* / CORE_MACHINE_* | x86-core/machine; x86_core_machine_* / X86_CORE_MACHINE_*, subject to the explicit PC exceptions below. |
| core_machine_board_*, PC board plans/controller bindings | x86-nxvm/board-base; x86_nxvm_board_* / X86_NXVM_BOARD_*. XT/AT-specific interfaces qualify their own leaf. |
| independent x86_pit_*, x86_pic_*, x86_video_* and other chip prefixes | x86-nxvm/chips/<chip>; retain existing unambiguous chip prefixes rather than rewriting algorithms merely to lengthen names. No chip may depend on board, adapter or Product. |
| vm_machine_* adapter and vm_app_* family Product | x86-nxvm/machine and product respectively; x86_nxvm_machine_* and x86_nxvm_product_* when their public paths migrate. Keep machine and entry/factory lifetimes distinct. |
| existing x86_nxvm_product_machine_* information/speed contract | Absorb into the one full x86-nxvm/product; do not create a second descriptor, enum translation, driver or implementation alongside it. |
| neutral test helper prefixes | Follow x86-core test owner; higher board fixtures do not borrow private headers. App-only D4 assertions and their objects remain MyDeskPro386. |

This table and per-declaration records define the migration rule, not an
instruction to rewrite string literals, assembler tables or historic evidence.
New public contract details receive actual-change review in the implementing S;
S2 neither invents public API declarations nor reserves an unimplemented ABI.

## PC Assumptions: Required Separation

| ID and current evidence | Frozen receiving decision and proof obligation |
| --- | --- |
| P2: machine_interface.h timing_capability/disposition/seam/declaration, machine.c get/validate/install helpers and machine.h 30-entry array | The 30-item whole-PC qualification universe includes PIC, disk media, host wait, Session and Product Debug. Its table, one-time installation and provenance remain board-base qualification data, not generic CPU capability requirements. Move the declarations and storage together. Neutral retirement eligibility, physical-time admission and CPU source-unallocated checks remain in x86-core. Board deadline code uses its own frozen qualification state rather than Core mirroring another table. Retain original IDs/values and T433/T560 evidence; moving a table does not upgrade any grade. |
| P3: trace_interface.h PC-specific advancement tags | Core owns trace buffering, sequence, guest timestamp and transaction/CPU events. NXVM board owns PIT/PIC/RTC/KBC/video/FDC/HDC advancement tags and their emission. Preserve the current copied event identity/order and compile-time Release no-op. Use the existing scalar event envelope with board-owned tag declarations if necessary; no second trace buffer or renderer, no product-name tags in Core. Review the smallest tag representation change with its actual callers, not a new telemetry framework. |
| P4: machine_scheduler.c arbitration_advance, dma_grant_advance and readiness_advance; attachment_interface.h PIT pair, rtc/media/DMA/refresh callbacks | Core retains sole guest elapsed time, timeline ordering, CPU retirement and bus begin/commit/cancel/HOLD ownership. Board receives PIT/PIC order, RTC/media service, DMA channel readiness, refresh production and their clock ratios. The existing chronological order must survive: due timeline work, refresh/bus arbitration, PIT/PIC arbitration, media/FPU/wait/RTC readiness, then peripheral advance. Earlier readiness cannot be silently promoted to the same-tick interrupt arbitration. A bounded advance/deadline board boundary replaces named PIT/RTC/media slots, not a callback registry or second scheduler. CPU/FPU internal deadlines remain Core-owned. Physical bus HOLD/grant waits stay with the sole transaction owner; moving PC sequencing must not create a board-local competing bus owner. |
| P5: machine.c resolve_cpu_profile/resolve_memory_bytes; machine_interface.h default/minimum/maximum memory constants | CPU-model choice and zero-memory-to-16MiB convenience belong to NXVM board construction; neutral construction consumes explicit values. The current 2MiB reconfigure admission is not the allocator's minimum: memory_initialize_for accepts any nonzero backing. Preserve PC admission in the board, validate generic allocation/range safety separately and prove small synthetic non-PC RAM. Never infer guest-visible RAM from allocator capacity. Preserve all actual CPU models and the intentionally closed production 486SX constructor (currently valid_cpu_profile ends at 80386). |
| P6: cpu_bus.c pic_pending/pic_acknowledge slots | CPU sees INTR and interrupt acknowledgement/vector delivery, not a PIC object. Neutralize those two names while retaining one request/ack transaction path; PIC cascade/wiring remains board-owned. Keep WAIT TEST, escape-trap, cache pins, special-cycle RDY and SMIACT as real architectural signals, not PC-device policy. |
| P7: memory.c wrap_a20 and memory_interface.h two A20 board policies | Board selects A20/physical decode policy. Core retains route resolution, atomic map installation, read-only ROM/aliases and side-effect-free observation. Preserve pre-A20 reset overlay priority and provider UNSUPPORTED fallthrough versus real failure. Do not add an address callback on every byte merely to hide an enum: first evaluate the existing route mechanism and keep CPU physical-bus masking in CPU. Both current PC policies require identical regression. |
| P8: machine_firmware.c, firmware_interface.h, entry_plan_interface.c and machine.c firmware_operation_active guards | Immutable byte mapping and bounded construction/reset/after-run callbacks contain no host BIOS interception and can remain neutral. ROM bytes, reset aliases, board initialization, BIOS image count and boot strategy stay outer. Preserve atomic binding/configuration failure, borrowed context lifetime and recursive-run exclusion. Entry plans are explicit construction mechanisms, not permission to skip actual PC firmware. |

`device_support_interface.h` contains arithmetic/bit expressions rather than a
device service. Its production users are memory.c and machine_board.c; a few
tests also consume it. Keep necessary neutral implementation expressions private
at their owner and update board/test uses directly; do not promote a catch-all
shared helper API or make lower CPU tests include machine internals.

### Lifetimes And Failures

CPU owns its execution object; Machine stores an opaque pointer, never mirrored
register/decoder state. FPU owns its own state. `x86_fpu_create(NONE)` currently
allocates a real profile-NONE object: retain that established absence model, not
an emulated guest coprocessor or a new fake implementation. The build currently
links CPU privately to FPU and execution Machine to both. Independent FPU
selection is a real profile choice, not a claim that CPU has no link dependency.

Board attachment is copied once before freeze; its context lives through the
one finalize callback. Reset and stop do not free it. Candidate construction
failure cancels only unpublished routes/resources, and accepted device effects
are not fictitiously rolled back. Adapter owns board lifetime until all Core
routes/providers stop borrowing it. Product's private-machine descriptor lives
through the complete synchronous run; wrapper unbinding precedes destruction.
No new host thread, lifecycle queue, pending flag mirror or cleanup manager is
needed for any of these moves.

## Targets, Registration And Non-PC Proof

Production target mapping is explicit in configured-targets.tsv:
core-chip-cpu/fpu become x86-core-cpu/fpu; core-x86 and its observable build
become x86-core-machine and x86-core-machine-observable; other chip targets,
board-base/observable, board-XT/AT and machine/media/conversion receive the
x86-nxvm prefix. core-product is absorbed by the existing x86-nxvm-product
contract target, which becomes the one implementation target. It is not a new
parallel library. Preserve source-local warning exceptions and trace build
variants; these are not blanket strict-warning or timing-accuracy upgrades.

Source-free aggregates core-verify, core-product-tests, core-x86-tests and
core-board-tests partition into their real package verification/test owners.
The old root x86-cpu alias is retired after callers use the canonical CPU target;
no compatibility alias is introduced. cpu-timing-manifest-catalog is currently
generated from NXVM evidence/tools: retain the qualification dependency in the
repository receiver until its data is self-contained, not an undeclared
standalone x86-core prerequisite. The manifest runners' exact source keys,
generated-result contracts and original ledgers survive; path renames are not
evidence-key renames.

The large root NxvmProduct.cmake mixes package-owned tests, App wiring and
artifacts. registrations.tsv retains every affected statement, including loop
expressions, direct source ownership ledgers, observable/scheduler test objects
and all profile-conditional integration registrations. Move package-owned
registration into its test package; retain four-App selection, ROM embedding,
deployment and external-profile registration in the family receiver. An App
target recompiling a Core scheduler under test defines is not a second production
execution path; its owned regression must still resolve after the source moves.

The generated witness also exposes Core-owned board/CPU tests linked through
`mydeskpro386-d4`, for example core-machine-legacy-alu-smoke and
core-machine-cpu-fault-diagnostic-smoke. Their source uses Core/board contracts,
not App headers; retain them at the assigned package owner and replace the
outer dependency with actual inward board/observable targets in S6. Do not
ship standalone tests that succeed only because an App happens to supply
transitive board libraries. Genuine App tests and integration-session-ini-support
remain outer consumers; their App dependencies are not inherently violations.
The per-target table records every such edge instead of treating all marked
App links as bugs or erasing them without a link/behavior regression.

Each package independently checks its own source/test root, Types and permitted
inward edges. Root registration/DAG owns the repository consumer map; the inner
verifier must not inspect App contents to satisfy its standalone contract.
External setup/integration observers stay NXVM-family owned and require their
real App binding/ROM/media; they cannot be used to qualify standalone units.

The first real non-PC consumer is a ROM-free synthetic board test in
test/x86-core/machine. It supplies small RAM, a project-authored reset program,
ROM alias, MMIO read/write/observe, an interrupt source and bounded device
deadline. It must demonstrate real fetch/retire, read-only ROM, unmapped and
alias behavior, IRQ entry, HALT progression, budget/pause, reset and teardown.
It links only x86-core/Types, not board chips, x86-debug, x86-ibmpc, Apps or
private PC fixtures. This is a new independence proof, not Raiden II support.

## Bounded Remaining S Plan

These are proposed next linear identifiers, not concurrently admitted packets.
Every implementation batch includes receiver repairs and remains buildable;
no commit intentionally leaves broken includes. Re-evaluate exact line estimates
against each admitted brief. Mechanical moves report rename-aware statistics,
not hundreds of thousands of apparent delete/add algorithm lines.

| Next S | Scope and estimated non-mechanical delta | Exit |
| --- | --- | --- |
| S3 | CPU/FPU sources, 107 test-package files including fixtures, canonical targets/public names and all receivers; roughly +150--300/-150--300 registration/include lines beyond mechanical symbol/path substitution. | Independent dual-width CPU/FPU owner build/tests, full units, preserved instruction/timing data and eight affected PC artifacts; no CPU-to-machine/PC dependency. |
| S4 | P2/P3/P5 qualification/default/trace separation while paths still compile; roughly +180--320/-220--380 lines, primarily moving existing declarations/state to board receivers. | Complete timing table provenance and grades preserved, no duplicated state, constructor/trace/default regressions and receiving Apps pass. No C17 admission. |
| S5 | Neutral execution extraction and P4/P6/P7/P8 boundaries; roughly +250--450/-220--400 non-mechanical implementation/registration lines plus +180--300 non-PC proof lines. | Real non-PC public consumer and PC arbitration/reset/alias/exception/deadline regressions pass both widths; one timeline, transaction and executor. |
| S6 | Remaining NXVM chips/boards/driver/product and test/support/registration consolidation; roughly +180--320/-220--400 registration/receiver lines beyond mechanical moves. | All four Apps use the sole package, private fixtures stay local, imported information/speed contract absorbed, old Core roots/aliases removed, artifacts qualified. |
| S7 | Final complete qualification and actual-diff audit; no planned functional expansion. | Independent package builds, all repository units/gates, all four profile integrations and eight current PC EXEs on both widths; docs/manifests/DAG and T560 residual ledger reconciled. |

If an interface requires a larger implementation than this bounded plan, revise
the next S before implementation rather than silently growing a batch. Shared
changes require their own declared target/commit; no Lib or Emulator change is
currently justified. MyNES has no new dependency on either x86 package.

## Verification And Retained Limits

Inventory regeneration and byte comparison verify all eight TSVs against the
same current input universe. Declaration records are cross-referenced by path,
line, scope and explicit mixed-contract decisions; root registrations and
configured-object edges supplement, rather than replace, file coverage.
Actual-change review must confirm only NXVM documentation changed, with no
Core/Shared/App/asset diff. Documentation governance is additionally required.

No x86/x64 rebuild, dynamic unit, profile boot, non-PC runtime, desktop or RDP
verification is claimed for S2. Existing 0561 artifacts are retained because
no executable input changed. Independent build/qualification, physical
extraction and non-PC proof remain I2--I5, not discharged by a green inventory.
The T560 successor still owns C16 complete timing/runtime/preview, C17 production
admission and final CPU qualification. T561 remains open.

Recorded checks: inventory regeneration/byte comparison passes; current source
and test Core manifests pass; current Core corpus boundary passes; documentation
governance passes. These are static/documentation checks. A later listing attempt
stalled inside CTest without running tests; only the identified audit-owned
processes were stopped. The final reproducer reads existing reachable generated
registrations directly, producing the same 656-case universe per width without
launching CTest. No timeout, graph, host setting or production file was changed.
