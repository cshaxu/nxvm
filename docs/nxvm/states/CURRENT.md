# Project Status

## Current Work

M5 T539 remains open. S1-S45 are accepted. S43 P1 `4ff59cd5c` establishes its
CPU-local descriptor receiver; the retained control-state source is assigned
only to S45. The former eleven-file, 7,000-plus-line arithmetic assignment is
split into S30-S35 under the existing automatic-S authorization. At S36 intake,
its oversized FLAGS/string/port row was divided into S36-S39. At S40 intake,
the 7,736-line descriptor/system row was divided into S40-S46 and the formerly planned
S41-S48 became S47-S54. At S48 intake, the former 6,724-line protected
transfer row was divided into S48-S55; S49 intake further divides the 1,181-line
control-transfer source into S49--S51; S68 intake divides the 20,417-line timing
corpus into S68-S73; the former oversized physical-relocation row is divided
into S76-S81 and final acceptance is S82. S78 intake found that the former
transfer/data row contains 19 files and 8,292 lines, so its unaccepted work is
split into S78-S86 before implementation; the remaining protected/system row
is then divided into S83-S100 and S100 is the final acceptance. S48-S100 are
accepted and T539 is closed. Earlier accepted
packets retain their historical prospective numbering; the linked
accepted packets retain their historical prospective numbering; the linked
work plan owns current numbers.
The [CPU work packages](../etc/architecture/t539-cpu-work-packages.md)
retain the remaining CPU work as pending, not accepted CPU extraction.

## S87 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | M5 T539 S87, automatically admitted continuation. |
| Admission And Approval | The owner approved automatic admission of bounded linear T539 S deliveries and required oversized CPU sources to be split by actual responsibility. This follows accepted S86. |
| Objective | Extract only the CPU bus, instruction-effect and copied retirement-observation rows from the mixed execution-context test into one Shared CPU receiver. |
| Non-goals | CPU create/reset/prepared-entry/lifecycle rows, NMI, prefetch, paging, INVLPG, public board wiring, CPU semantics/timing, production APIs, firmware, assets, INI and executable inputs. |
| Reference Baseline | `42ec9a014` after S86; the ledger assigns execution-context bus/observation rows to S87 and reserves lifecycle for S88 and signal/prefetch/paging for S89. |
| Candidate Proposal | [Independent Shared chips](../history/M5-T539-independent-shared-chips.md) and the revised [CPU work package ledger](../etc/architecture/t539-cpu-work-packages.md). |
| Files And ABI Surface | Split `cpu_execution_context_smoke.c` by existing static-function boundary. Move the already CPU-only `cpu_bus_fixture.h` to Shared and update its one retained NXVM FLAGS consumer to include it directly. Add one Shared successor with only bus/observation helpers and a minimal main; retain all other functions once in the NXVM residual source. Update registrations/manifests/static inventories; no production or public ABI change. |
| Applicable Rules | NXVM architecture and coding authorities; Shared architecture/coding/execution/documentation rules; one owner per test behavior and no duplicate CPU setup path. |
| Verification | Record the exact function allocation; prove the receiver uses only the existing CPU bus fixture; run focused x64/x86 receiver and residual tests, repository-only x64/x86 unit suites, T332 where applicable, Shared manifest/corpus, CPU/PIC authority, documentation governance and diff checks. Rebuild artifacts only if executable inputs change. |
| Expected Markers | One `x86-cpu` receiver, one Shared CPU bus fixture, a residual NXVM test with the remaining functions, no copied bus/observation function in both files, and no new fixture/API. |
| Asset Needs | None; repository-only test ownership split. |
| Reporting Requirements | Record moved and retained static functions, source line totals, static-gate impact, test evidence and artifact determination. |
| Stop Conditions | Stop and report if a candidate row needs public Core/board wiring, a new Shared fixture/API, or duplicated function/fixture setup. |
| Exit Criteria | All admitted bus/observation rows have one Shared receiver, every remaining execution-context row stays once in the residual source with S88/S89 allocation, and required verification is recorded. |
| Original Owner Request | Continue independent-chip extraction through strictly linear numeric S tasks with bounded, visible ownership. |
| Similar-Issue Sweep | S87 consumes only CPU bus/effect/retirement observation functions. S88 owns create/reset/prepared-entry/timing/lifecycle; S89 owns NMI, prefetch, paging-control and INVLPG. |

## S87 Acceptance

The former `cpu_bus_cases()` function is now solely the Shared
`cpu_execution_bus` receiver. Its one CPU-local fixture is likewise solely
Shared; NXVM's residual context and FLAGS-local tests include it directly, and
the former App fixture copy and function call are deleted. Lifecycle/timing,
signal/prefetch/paging and public-board rows remain once in NXVM for S88/S89.

Focused x64/x86 Shared and NXVM receivers, T332, Shared manifest/corpus and
CPU/PIC authority gates pass. The original terminal-hosted full-unit runs
misreported the long CMake negative gate as failed. Detached, process-owned
full-unit verification fixes that execution artefact: x64 **440/440** and x86
**440/440** both pass with exit code zero. This is test/CMake/documentation-only
work: no production/API, firmware, asset, INI or EXE input changed. See the
[S87 evidence](../etc/evidence/t539-s87-execution-bus-receiver.md). S87 is
accepted; T539 remains open for S88-S98.

## S88 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | M5 T539 S88, automatically admitted continuation. |
| Objective | Extract only CPU reset, prepared-entry, instance-lifecycle and timing rows from the residual execution-context test into bounded Shared receivers. |
| Non-goals | NMI, prefetch, paging, INVLPG, public board wiring, production APIs, firmware, assets, INI and executables. |
| Files And ABI Surface | Split only existing static functions from `cpu_execution_context_smoke.c`; reuse existing CPU-local fixtures or add no fixture/API unless the current code proves one is necessary. Retain every S89 row once in NXVM. |
| Verification | Record exact function allocation; run focused x64/x86 successor and residual tests, complete x64/x86 repository-only units, relevant static gates, manifests/corpus, authority, documentation governance and diff checks. |
| Exit Criteria | Every admitted lifecycle/timing row has one Shared receiver, all non-admitted rows remain once in NXVM, no duplicate fixture/setup path or public ABI is introduced, and both full unit runs pass. |

## S88 Acceptance

The CPU-only reset, prepared-entry, two-instance isolation and repeat-timing
rows now have one Shared lifecycle receiver. The NXVM residual no longer
contains a timing or lifecycle implementation; it retains only named S89 and
later work. Focused receivers and all static gates pass; detached process-owned
full units pass **441/441** on x64 and x86. No production/API, firmware, asset,
INI or EXE input changed. See [S88 evidence](../etc/evidence/t539-s88-execution-lifecycle-receiver.md).
S88 is accepted; T539 remains open for S89-S100.

## S89 Active Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | M5 T539 S89, automatically admitted continuation. |
| Objective | Move only the CPU-owned NMI mask/delivery and prefetch-reservation rows from the residual execution-context test. |
| Non-goals | Paging/INVLPG, protected fault/pending-event rows, public IRQ/board wiring, firmware, production APIs, assets, INI, executables and unrelated residual CPU rows. |
| Files And ABI Surface | Split existing static functions by actual CPU responsibility; reuse existing CPU-local fixture and create no shared framework, board adapter or public API. Retain any concrete board path once in NXVM under a named later receiver. |
| Verification | Record exact allocation; run focused x64/x86 successor and residual tests, complete x64/x86 units, applicable static gates, manifest/corpus, authority, documentation governance and diff checks. |
| Exit Criteria | Every admitted NMI/prefetch row has one Shared owner, every excluded row remains once in NXVM, neither width regresses, and full unit suites pass. |

## S89 Acceptance

`cpu_signal_case()` and `cpu_prefetch_case()` now have exactly one Shared
receiver, `cpu_execution_signal_prefetch`. The NXVM residual deletes both
functions and calls, retaining only named S90/S91 work. Focused x64/x86
receivers, T332, authority, manifest/corpus, documentation governance and
diff gates pass; detached full unit suites pass **442/442** on x64 and x86.
No production/API, firmware, asset, INI or executable input changed. See the
[S89 evidence](../etc/evidence/t539-s89-signal-prefetch-receiver.md). S89 is
accepted; T539 remains open for S90-S100.

## S90 Active Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | M5 T539 S90, automatically admitted continuation. |
| Admission And Approval | The owner approved automatic, bounded, numeric T539 continuation tasks; S90 is the next linear package after accepted S89. |
| Objective | Move only 80186 LGDT availability, paging-control and INVLPG rows from the residual execution-context test to bounded Shared CPU receivers. |
| Non-goals | Protected fault/pending-event rows, public paging/board wiring, firmware, production APIs, assets, INI, executables and unrelated residual CPU rows. |
| Reference Baseline | `30fa60856`, accepted S89; the residual source contains S90 paging/INVLPG and S91 fault/event rows only. |
| Candidate Proposal | [Independent Shared chips](../history/M5-T539-independent-shared-chips.md) and the [CPU work package ledger](../etc/architecture/t539-cpu-work-packages.md). |
| Files And ABI Surface | Split only `cpu_80186_lgdt_gate()`, `cpu_paging_prepare()`, `cpu_paging_control_gate()`, `cpu_paging_control_forms()`, `cpu_paging_invlpg_case()`, `cpu_paging_invlpg_rejection()` and `cpu_paging_cr0_mutable_controls()` to one Shared successor. Reuse the CPU-local fixture; no production, public API, framework or board adapter changes. |
| Applicable Rules | NXVM and Shared architecture/coding/execution/documentation rules; one owner per CPU-only test behavior, no duplicate fixture path and no public board assertion in Shared. |
| Verification | Record allocation; run focused x64/x86 successor and residual tests, complete x64/x86 units, T332, manifest/corpus, CPU/PIC authority, documentation governance and diff checks. Rebuild artifacts only if executable inputs change. |
| Expected Markers | One `x86-cpu` paging/INVLPG receiver, one NXVM residual source with only debug and S91 rows, no duplicate helper/function and no new fixture/API. |
| Asset Needs | None; repository-only ownership migration. |
| Reporting Requirements | Record moved and retained functions, focused/full test evidence, static-gate impact and artifact determination. |
| Stop Conditions | Stop and report if a row requires public Core paging, physical mapping, IRQ/board wiring, a new Shared API or a duplicate fixture path. |
| Exit Criteria | Every admitted paging/INVLPG row has one Shared owner, all excluded rows remain once in NXVM, neither width regresses, and full unit suites pass. |
| Original Owner Request | Continue independent-chip extraction through strictly linear numeric S tasks with bounded, visible ownership. |
| Similar-Issue Sweep | S90 consumes only CPU-local paging/INVLPG and 80186 gate rows. S91 retains fault/pending-event rows; public Core paging and board IRQ paths remain named NXVM receivers. |

## S90 Acceptance

The 80186 LGDT-gate, paging-control, INVLPG and CR0 mutable-control helpers
now have one Shared `cpu_execution_paging` receiver. NXVM deletes all seven
helpers and calls, retaining only debug and named S91 fault/event work.
Focused x64/x86 receivers, T332, authority, manifest/corpus, documentation
governance and diff gates pass; detached full units pass **443/443** on x64
and x86. No production/API, firmware, asset, INI or executable input changed.
See [S90 evidence](../etc/evidence/t539-s90-execution-paging-receiver.md).
S90 is accepted; T539 remains open for S91-S100.

## S91 Active Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | M5 T539 S91, automatically admitted continuation. |
| Admission And Approval | The owner approved automatic, bounded, numeric T539 continuation tasks; S91 is the next linear package after accepted S90. |
| Objective | Move only CPU-owned protected interrupt preparation, UD cache-preservation and pending-event rollback rows from the residual execution-context test. |
| Non-goals | Public Core paging, PIC IRQ delivery, board transactions, firmware, production APIs, assets, INI, executables and unrelated CPU rows. |
| Reference Baseline | `d8e3665ec`, accepted S90; the residual source contains debug and S91 fault/event rows only. |
| Candidate Proposal | [Independent Shared chips](../history/M5-T539-independent-shared-chips.md) and the [CPU work package ledger](../etc/architecture/t539-cpu-work-packages.md). |
| Files And ABI Surface | Split existing CPU-local static functions by responsibility, reusing the existing CPU bus fixture with no framework, board adapter or public API. |
| Applicable Rules | NXVM and Shared architecture/coding/execution/documentation rules; one owner per CPU-only behavior and no duplicate fixture path. |
| Verification | Record allocation; run focused x64/x86 successor and residual tests, complete x64/x86 units, T332, manifest/corpus, CPU/PIC authority, documentation governance and diff checks. Rebuild artifacts only if executable inputs change. |
| Expected Markers | One `x86-cpu` fault/event receiver, a residual NXVM test containing only debug, and no new fixture/API. |
| Asset Needs | None; repository-only ownership migration. |
| Reporting Requirements | Record moved/retained functions, test evidence, static-gate impact and artifact determination. |
| Stop Conditions | Stop and report if a row requires public Core paging, PIC/board wiring, a new Shared API or a duplicate fixture path. |
| Exit Criteria | Every admitted fault/event row has one Shared owner, excluded rows remain once in NXVM, neither width regresses, and full unit suites pass. |
| Original Owner Request | Continue independent-chip extraction through strictly linear numeric S tasks with bounded, visible ownership. |
| Similar-Issue Sweep | S91 consumes only artificial-IDT CPU fault/event rows; public IRQ and Core paging paths remain named NXVM receivers. |

## S91 Acceptance

The three protected fault/event helpers now have one Shared
`cpu_execution_fault_event` receiver. NXVM deletes the duplicate helpers and
transfers the T337 #UD inventory to that receiver; its independent debug API
test remains. Focused x64/x86 and all static gates pass; detached units pass
**444/444** on both widths. No production/API, firmware, asset, INI or EXE
input changed. See [S91 evidence](../etc/evidence/t539-s91-execution-fault-event-receiver.md).
S91 is accepted; T539 remains open for S92-S100.

## S92 Acceptance

The CPU-only protected data-access, far-transfer and outer-return receivers,
with their sole CPU-local fixtures, now have one Shared owner in
`test/x86/devices/cpu`. NXVM removes the duplicated targets and source copies.
Its retained PIC/board receivers continue to own interrupt delivery and use
the Shared fixtures without recreating CPU setup.

Focused x64/x86 receivers, T317/T332, CPU/PIC authority, Shared
manifest/corpus and documentation governance pass. Detached full unit suites
pass **447/447** on x64 and x86. This is test/CMake/documentation-only work:
no production/API, firmware, asset, INI or EXE input changed. See the
[S92 evidence](../etc/evidence/t539-s92-protected-receivers.md). S92 is
accepted; T539 remains open for S93-S100.

## S93 Acceptance

The CPU-only 16-bit task-switch, 32-bit task-switch decode and 32-bit
task-state receivers, with their sole fixture, now have one Shared owner in
`test/x86/devices/cpu`. NXVM deletes the duplicate targets and source copies.
The retained PIC board receiver consumes the Shared fixture while continuing
to own actual interrupt routing; cross-width, paging and TSS I/O paths remain
separate NXVM receivers.

Focused x64/x86 receivers, T317/T332, CPU/PIC authority, Shared
manifest/corpus and documentation governance pass. Detached full unit suites
pass **450/450** on x64 and x86. This is test/CMake/documentation-only work:
no production/API, firmware, asset, INI or EXE input changed. See the
[S93 evidence](../etc/evidence/t539-s93-task-state-receivers.md). S93 is
accepted; T539 remains open for S94-S100.

## S94 Acceptance

The eight CPU-only bit-scan/test, double-shift, IMUL2, MOVX, sign-extend,
SETcc and LEA receivers, with their sole operand-probe fixture, now have one
Shared owner in `test/x86/devices/cpu`. NXVM deletes the duplicate targets and
source copies. The named `core_machine_*` receivers remain NXVM board owners;
no production code, public API, firmware, asset, INI or executable input
changed.

Focused x64/x86 Shared and retained board receivers pass **16/16** on each
width. T317/T332, CPU/PIC authority, Shared manifest/corpus and documentation
governance pass. Detached full unit suites pass **458/458** on x64 and x86.
See the [S94 evidence](../etc/evidence/t539-s94-cpu-instruction-receivers.md).
S94 is accepted; T539 remains open for S95-S100.

## S95 Acceptance

The direct CPU-only operand/address and prefix-attribute receivers now have
one Shared owner in `test/x86/devices/cpu`, reusing the existing Shared
instruction fixture. NXVM deletes their duplicate targets and source copies.
The Core-machine operand/address and prefix-attribute board paths remain
separate NXVM owners; no production code, public API, firmware, asset, INI or
executable input changed.

Focused x64/x86 Shared and retained-board tests pass **3/3** on each width.
T317, CPU/PIC authority, Shared manifest/corpus and documentation governance
pass. Detached full unit suites pass **460/460** on x64 and x86. See the
[S95 evidence](../etc/evidence/t539-s95-operand-prefix-receivers.md). S95 is
accepted; T539 remains open for S96-S100.

## S96 Acceptance

The direct CPU-only legacy-LOCK and immediate-IMUL encoding receivers now
have one Shared owner in `test/x86/devices/cpu`, reusing the existing Shared
instruction fixture. NXVM deletes the duplicate targets and source copies.
`core_machine_legacy_lock_s1_smoke` remains NXVM because it owns real port and
IOPL board wiring; no blanket LOCK compatibility path was added.

Focused x64/x86 Shared and retained-board tests pass **3/3** on each width.
T317, CPU/PIC authority, Shared manifest/corpus and documentation governance
pass. Detached full unit suites pass **462/462** on x64 and x86. See the
[S96 evidence](../etc/evidence/t539-s96-lock-imul-receivers.md). S96 is
accepted; T539 remains open for S97-S100.

## S97 Acceptance

The three direct CPU-only control-transfer receivers now have one Shared owner
in `test/x86/devices/cpu`. S97 initially classified the IDT privilege receiver
as NXVM from its `device_support.h` include; S99 corrected that finding when it
proved the include supplied only a generic bit-test macro. No production code,
public API, firmware, asset, INI or executable input changed.

Focused x64/x86 successor and retained-IDT tests pass **4/4** on each width.
T317, CPU/PIC authority, Shared manifest/corpus and documentation governance
pass. Detached full unit suites pass **465/465** on x64 and x86. See the
[S97 evidence](../etc/evidence/t539-s97-residual-cpu-classification.md). S97
is accepted; T539 remains open for S98-S100.

## S98 Acceptance

Shared `src/x86/devices/cpu` remains the sole CPU timing implementation. The
timing/catalog runners remain their one NXVM owner because each composes an
App machine/profile, publishes board-time observations, or produces a
generated result contract. S98 adds no synthetic Shared machine fixture, no
second timing route and no copied formula table.

The CPU timing-manifest catalog, x64/x86 full units (**465/465** each), T317,
CPU/PIC authority, Shared manifest/corpus and documentation governance pass.
See the [S98 evidence](../etc/evidence/t539-s98-timing-catalog-boundary.md).
S98 is accepted; T539 remains open for S99-S100.

## S99 Acceptance

S99 corrected S97's include-only false boundary: the IDT privilege CPU
receiver now has one Shared owner. Its former App macro use is an equivalent
local bit test, not a new API. The retained PIC-board receiver directly
includes the canonical Shared fixture; the last NXVM fixture forwarding header
is deleted. IDT CPU semantics and PIC delivery remain separate one-owner tests.

Focused x64/x86 Shared IDT and retained PIC-board tests pass **2/2** on each
width. T317, CPU/PIC authority, Shared manifest/corpus and documentation
governance pass. Detached full unit suites pass **466/466** on x64 and x86.
See the [S99 evidence](../etc/evidence/t539-s99-final-cpu-path-cleanup.md).
S99 is accepted; T539 remains open for S100.

## S100 Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | M5 T539 S100, automatically admitted final continuation. |
| Admission And Approval | The owner approved automatic, bounded, strictly linear numeric T539 continuation tasks; S100 follows accepted S99. |
| Objective | Prove final CPU test ownership: each CPU-only receiver and fixture has one Shared owner; each machine, PIC, board-time and result-publication receiver has one named NXVM owner; no retired App CPU test path survives. |
| Non-goals | Further CPU behavior/timing changes, new Shared APIs/frameworks, profile changes, firmware, assets, INI and executable changes. |
| Reference Baseline | `dfe846cbc`, accepted S99. |
| Candidate Proposal | [Independent Shared chips](../history/M5-T539-independent-shared-chips.md) and the [CPU work package ledger](../etc/architecture/t539-cpu-work-packages.md). |
| Files And ABI Surface | Read-only inventory and evidence unless a real duplicate path is found. Verify no App target directly links `x86-cpu`, no App fixture forwards the Shared CPU fixture, and every retained App CPU-named receiver has a Core-machine/board/result owner. |
| Applicable Rules | One owner per fact and behavior; Shared owns chip semantics and CPU-only fixtures, NXVM owns composition, board signal routes and product result artifacts. |
| Verification | Full x64/x86 unit suites; full integration suite; T317/T332/static gates; Shared manifest/corpus; CPU/PIC authority; documentation governance; diff and inventory checks. |
| Exit Criteria | The complete receiver map is evidenced, both unit widths and integration pass, no duplicate path remains, and T539 can close without an unallocated CPU boundary. |
| Original Owner Request | Continue independent-chip extraction through strictly linear numeric S tasks with bounded, visible ownership. |

## S100 Acceptance

The final inventory found one remaining CPU-only EFLAGS receiver. It is now
the Shared `x86.cpu_eflags_local` receiver and uses the canonical CPU bus
fixture. No App target directly links `x86-cpu`; the remaining App sources
with CPU fixtures are named machine/board owners and include canonical Shared
fixtures directly rather than forwarding them. The timing ledger/manifests
remain NXVM because they compose machine/profile time, board inputs and
published result artifacts; no second Shared timing runner was created.

The native modal-window unit test was also correctly declared CTest-exclusive:
it owns a real nested Win32 message loop and was the only full-suite race. Its
coverage and assertions remain unchanged.

Full detached unit suites pass **467/467** on x64 and x86; the x64 external
integration suite passes **20/20**. T317, T332, CPU/PIC authority, Shared
manifest/corpus, documentation governance and diff checks pass. This is
test/CMake/documentation-only work: no production/API, firmware, asset, INI
or executable input changed, so no executable rebuild is required. See the
[S100 evidence](../etc/evidence/t539-s100-whole-cpu-acceptance.md). S100 is
accepted and T539 is closed.

## S86 Acceptance

The 257-line CPU-only debug-state receiver—MOV-DR, debug exceptions and data
breakpoints—now has one Shared `cpu_debug_state` owner using the established
instruction fixture. Its NXVM source and target are removed;
`machine_debug_state_board_smoke` remains the named public-board receiver.
T332 now maps the debug receiver through the canonical `devices/cpu/` entry.

Focused successors and T332 pass on x64 and x86. Both current full
repository-only unit logs contain 439 passing tests and zero failed-test
records. CPU/PIC authority, Shared manifest/corpus, documentation governance
and diff checks pass. This is test/CMake/documentation-only work: no
production/API, firmware, asset, INI or EXE input changed, so no executable
rebuild is required. See the [S86 evidence](../etc/evidence/t539-s86-debug-state-receiver.md).
S86 is accepted; T539 remains open for S87-S98.

## S85 Acceptance

The 392-line CPU-only CLTS/SMSW/LMSW/MOV-CR receiver now has one Shared
`cpu_control_state` owner using the established instruction fixture. Its NXVM
source and target are removed; `machine_control_state_board_smoke` remains the
named public board receiver.

The same intake found that T332's static lifecycle gate still resolved S83/S84
Shared receivers as obsolete App paths. Its source resolver now recognizes the
canonical `test/x86/devices/` inventory prefix, so the 44-owner gate verifies
the real Shared files without a parallel inventory. Focused receivers and T332
pass on x64/x86; each 438-case unit suite was executed. The x64 run recorded
the existing `unit.vm-runner-error-propagation-smoke` flake and x86 recorded
the existing `x86.cpu_movs` flake; both pass immediately in isolated reruns.
Shared manifest/corpus, CPU/PIC authority, documentation governance and diff
checks pass. This is test/CMake/documentation-only work: no production/API,
firmware, asset, INI or EXE input changed, so no executable rebuild is
required. See the [S85 evidence](../etc/evidence/t539-s85-control-state-receiver.md).
S85 is accepted; T539 remains open for S86-S98.

## S84 Acceptance

The four CPU-only system-table receivers now have one Shared owner:
`cpu_descriptor_system`, `cpu_dttr_s61`, `cpu_lgdt_lidt`, and
`cpu_sgdt_sidt`. The former NXVM copies (1,068 source lines) are retired; no
board path moved. The descriptor receiver no longer includes an App header:
its two generic CR0 bit-macro uses are equivalent local expressions, without
a Shared API addition.

Each successor passes on x64 and x86, the 437-case unit suite was executed on
both widths, and every migration receiver passes in focused reruns. The x64
parallel run recorded five pre-existing string-test flakes and the x86 runs
recorded one `cpu_movs` flake; all six passed immediately in isolation. The
x64 serial run recorded the existing runner-error-propagation flake, which
also passed immediately in isolation. Shared manifest/corpus, CPU/PIC
authority, documentation governance and diff checks pass. This is
test/CMake/documentation-only work: no production/API, firmware, asset, INI
or EXE input changed, so no executable rebuild is required. See the
[S84 evidence](../etc/evidence/t539-s84-system-table-receivers.md). S84 is
accepted; T539 remains open for S85-S94.

## S40 Acceptance

Actual pushed NXVM P1 `e4a7615d1` has exactly nine scoped paths, passes
`git show --check`, and equals `origin/master` at actual-commit review. The
two original ARPL sources (910 lines) are retired; every base and S53 case
has a CPU or public-board receiver, including register/memory forms, illegal
encodings, protected faults and PIC IRQ delivery. Eight code/test/build/gate
paths add 689/remove 941 lines (net -252); evidence is separate. CPU tests
link only `x86-cpu`; board tests use public machine and real PIC operations.
Complete x64/x86 builds and units pass 413/413 per width; all 66 specialized
gates and the 412-row T344 matrix pass on both widths. Six unchanged Shared
manifests, documentation governance and diff checks pass. No production/API,
Shared, firmware, INI or EXE input changed. See [S40 evidence](../etc/evidence/t539-s40-arpl-migration.md).
S40 is accepted; 56 original direct-private `.c` consumers plus the common
fixture header remain assigned to S41-S51. T539 stays open.

## S39 Acceptance

Actual pushed NXVM P1 `07019f588` has exactly fourteen scoped paths, passes
`git show --check`, and equals `origin/master` at review. The 170 original
scalar IN/OUT contexts and sixty original INS/OUTS contexts retain CPU or
public-board receivers; the separate port-ownership behaviors retain their
board receiver. The thirteen code/test/build/gate paths add 1,239 and remove
1,564 lines (net -325); the 44-line evidence report is separate. There is no
production/API or EXE input change. Complete x64/x86 builds and repository-
only units pass 413/413 each. All 66 specialized gates pass per width,
including T317/T332/T337/T344, CPU/PIC authority and the 412-row direct
matrix. Six unchanged Shared manifests pass within eleven manifest tests;
documentation governance passes. See [S39 evidence](../etc/evidence/t539-s39-port-io-migration.md).
S39 is accepted; 58 direct-private `.c` consumers plus the common fixture
header remain assigned to S40-S51. T539 stays open.

## S31 Acceptance

Actual pushed NXVM P1 `38bc5b10c` has exactly nine scoped paths, passes
`git show --check`, and equals `origin/master` at review. All 335 original
contexts retain one receiver: 324 CPU-owned and eleven board-owned. Code,
test, build and gate changes add 1,289/remove 1,468 lines (net -179); the
49-line evidence report is separate. Two CPU tests link only `x86-cpu` and
compile with warnings as errors. Complete x64/x86 units pass 397/397 each;
T317/T332/T344 and CPU/PIC gates, 396-row direct matrix, six unchanged
Shared manifests, and documentation governance pass. No production/API or
executable input changed. [S31 evidence](../etc/evidence/t539-s31-imul-group2-migration.md)
contains the original-case receiving map. The S31 packet is closed; 74
original private-test consumers remain assigned to S32-S42. S43-S45 retain
lifetime, physical relocation and whole-CPU acceptance.

## S30 Acceptance

Actual pushed NXVM P1 `442088410` has exactly 23 scoped paths, passes
`git show --check`, and equals `origin/master` at review. All 549 original
contexts retain one receiver: 539 CPU-owned and ten board-owned. The 19
code/test/build paths add 1,429/remove 1,506 lines (net -77); six CPU tests
link only `x86-cpu`, while six board tests retain real faults or IRQ through
Core-machine. Complete x64/x86 units pass 395/395 each, 66 specialized gates
pass per width, six unchanged Shared manifests verify, and documentation
governance passes. No production/API or executable input changed. See
[S30 evidence](../etc/evidence/t539-s30-bit-condition-extension-migration.md).
The S30 packet is closed; 76 original private-test consumers remain assigned
to S31-S42. S43-S45 still own lifetime, physical relocation and whole-CPU
acceptance.

## S32 Acceptance

Actual pushed NXVM P1 `9f785a551` has exactly eight scoped paths, passes
`git show --check`, and equals `origin/master` at independent review. All 775
original contexts retain one receiver: 767 CPU-owned and eight board-owned.
Code/test/build/gate changes add 1,536/remove 1,589 lines (net -53); the
57-line P1 evidence draft is separate. Two CPU tests link only `x86-cpu` and
compile with warnings as errors; board tests use public machine operations.
Complete x64/x86 builds and units pass 399/399 on each width; all 66
specialized gates pass per width, including T317/T332/T337/T344, CPU/PIC
authority, the 398-row direct matrix, six unchanged Shared manifests and
documentation governance. No production/API or executable input changed.
[S32 evidence](../etc/evidence/t539-s32-legacy-alu-lock-migration.md) contains
the original-case receiving map. The S32 packet is closed; 72 original
private-test consumers remain assigned to S33-S42. S43-S45 retain lifetime,
physical relocation and whole-CPU acceptance.

## S33 Acceptance

Actual pushed NXVM P1 `6ca7f61ac` has exactly six scoped paths, passes
`git show --check`, and equals `origin/master` at independent review. The
original 19 first-group functions retain one receiver each: 200 CPU-only
instruction executions and 23 public board contexts. Code/test/build/gate
changes add 1,056/remove 1,041 lines (net +15); the 43-line evidence report
is separate. The CPU test links only `x86-cpu` and compiles with warnings as
errors. The board test uses public machine operations for 11 protected
faults and twelve real-mode divide deliveries. Historic T316/T401 first-group
success markers moved to the CPU receiver. Complete x64/x86 builds and units
pass 401/401 on each width; all 66 specialized gates pass per width, including
T317/T332/T337/T344, CPU/PIC authority, the 400-row direct matrix, six
unchanged Shared manifests and documentation governance. No production/API
or executable input changed. [S33 evidence](../etc/evidence/t539-s33-inc-dec-first-group-migration.md)
contains the original-case receiving map. The S33 packet is closed; 72
original private-test consumers remain assigned to S34-S42, including the
unmigrated S34/S35 functions in `core_machine_inc_dec_smoke.c`. S43-S45 retain
lifetime, physical relocation and whole-CPU acceptance.

## S34 Acceptance

Actual pushed NXVM P1 `3b2d17d97` has exactly seven scoped paths, passes
`git show --check`, and equals `origin/master` at independent review. All 13
original second-group functions retain one receiver each: 111 CPU instruction
executions and eight public board faults. Two originally mixed functions were
split at their fault sections. Code/test/build changes add 843/remove 881
lines (net -38); the 44-line evidence report is separate. One CPU test links
only `x86-cpu`; one board test uses public machine operations. The shared
test-only board-fault helper removes duplicate S33/S34 checking logic while
preserving the 23 S33 board cases. Complete x64/x86 builds and units pass
403/403 per width; all 66 specialized gates pass per width, including
T317/T332/T337/T344, CPU/PIC authority, the 402-row direct matrix, six
unchanged Shared manifests and documentation governance. No production/API
or executable input changed. [S34 evidence](../etc/evidence/t539-s34-test-add-adc-sbb-migration.md)
contains the receiving map. S34 is closed; the last 19 `inc_dec` functions
remain assigned to S35. The original private-test consumer inventory remains
at 72 until that file is retired. S43-S45 still own lifetime, physical
relocation and whole-CPU acceptance.

## S35 Acceptance

Actual pushed NXVM P1 `0fc194460` has ten scoped paths, passes
`git show --check`, and equals `origin/master` at independent review. All 19
remaining functions have one receiver: 325 CPU instruction executions and
twelve public board contexts. The mixed source is retired; the CPU receiver
links only `x86-cpu`, and the board receiver uses public machine operations.
The shared test-only divide fixture removes repeated S33/S35 delivery checks
while preserving S33's cases. Code/test/build changes add 463/remove 733
lines (net -270); the 45-line evidence report is separate. Complete x64/x86
builds and units pass 404/404 per width, all 66 specialized gates pass per
width, including T317/T332/T337/T344, CPU/PIC authority, the 403-row direct
matrix, six unchanged Shared manifests and documentation governance. No
production/API or executable input changed. [S35 evidence](../etc/evidence/t539-s35-final-inc-dec-migration.md)
contains the receiving map. S35 is closed; 71 original private-test consumers
remain assigned to S36-S45. S46-S48 retain lifetime, physical relocation and
whole-CPU acceptance.

## S36 Acceptance

Actual pushed NXVM P1 `607fdea7a` contains exactly 17 scoped paths, passes
`git show --check`, and equals `origin/master` at independent actual-commit
review. Four private-state FLAGS tests are retired, including the PUSHF/POPF
source includer. All original case families have one CPU or public-board
receiver; the six replacement tests pass on x64 and x86. Code/test/build/gate
changes add 1,562/remove 1,982 lines (net -420). Complete x64/x86 units pass
406/406 each; both specialized gate aggregates, T317/T332/T337/T344,
CPU/PIC authority, 405/404-row direct matrices, six unchanged Shared
manifests, and documentation governance pass. No production/API or executable
input changed. [S36 evidence](../etc/evidence/t539-s36-flags-migration.md)
records the receiving map. S36 is accepted; 66 original direct-private `.c`
test consumers plus one shared fixture header remain assigned to S37-S45.

## S37 Acceptance

Actual pushed NXVM P1 `3c826c32d` has exactly thirteen scoped paths, passes
`git show --check`, and equals `origin/master` at independent actual-commit
review. The two original mixed-owner MOVS/LODS sources are retired; all 119
original contexts have a CPU or public-board receiver, with complementary
protected-fault assertions rather than duplicate production paths. Eleven
code/test/build/gate paths add 1,065/remove 1,327 lines (net -262); the
evidence and inventory documentation are separate. Both CPU receivers link
only `x86-cpu`; board receivers use public machine operations and real PIC,
guest descriptors, faults and interrupt frames. Complete x64/x86 units pass
408/408 each; both 66-target specialized gate aggregates, the 407-row T344
direct matrix, six unchanged Shared manifests, documentation governance and
diff checks pass. No production/API, Shared source/test, firmware, INI or EXE
input changed. [S37 evidence](../etc/evidence/t539-s37-string-transfer-migration.md)
records the receiving map. S37 is accepted; 64 original direct-private `.c`
test consumers plus one fixture header remain assigned to S38-S45.

## S38 Acceptance

Actual pushed NXVM P1 `ae76a7c84` has exactly fifteen scoped paths, passes
`git show --check`, and equals `origin/master` at independent actual-commit
review. All 203 original STOS/SCAS/CMPS contexts retain CPU or public-board
receivers. The three mixed-owner sources are retired; six replacement tests
preserve historical success markers. Thirteen code/test/build/gate paths add
1,682/remove 2,034 lines (net -352); the two evidence documents are separate.
Complete x64/x86 builds and units pass 411/411 per width. Specialized gates
pass 66 x64 and 68 x86 targets, including T317/T332/T337/T344, CPU/PIC
authority and direct matrices of 410/409 rows. All eleven selected manifest
tests, including six unchanged Shared manifests, documentation governance and
diff checks pass. No production/API, Shared, firmware, INI or EXE input
changed; the regenerated x86 EXE build side effect was removed. The
[S38 evidence](../etc/evidence/t539-s38-string-scan-compare-migration.md)
records the receiving map. S38 is accepted; 61 direct-private `.c` test
consumers plus one fixture header remain assigned to S39-S45.

## S41 Acceptance

Actual pushed NXVM P1 `99ab4c002` contains exactly eighteen scoped paths,
passes `git show --check`, and equals `origin/master` at actual-commit review.
The 899-line mixed BOUND source is retired. Its 247-line CPU receiver links
only `x86-cpu`; its 335-line board receiver uses public machine operations,
guest table construction and the real PIC. All original families retain one
receiver: width/profile, size attributes, invalid forms, segment routes,
signed boundaries, SIB/SS, VM86, real/protected faults and IRQ delivery.
The BOUND-local predecode fixes the previously reproduced 32-bit register-only
form from internal CPU error to terminal #UD without a new path or API.

Eight code/test/build/gate paths add 609/remove 922 lines (net -313); evidence
and task state are separate. Complete x64/x86 builds and units pass 414/414
each. Both 67-target specialized gate aggregates pass per width, including
T317/T332/T337/T344, CPU/PIC authority and the 413-row direct matrix. Six
unchanged Shared manifests, documentation governance and diff checks pass.
Because CPU production changed, all four runnable profiles have rebuilt,
optimized, stripped T539 x64/x86 EXEs; their hashes and no-INI-change proof
are recorded in [S41 evidence](../etc/evidence/t539-s41-bound-migration.md).
S41 and S42 are accepted. Fifty-two direct-private `.c` consumers plus the
shared fixture header remain assigned to S43-S51. T539 remains open.

## S42 Acceptance

Coordinator actual-change review accepts pushed NXVM P4 `2935b5886`: exactly
the three named table-register smoke sources and registrations are retired;
CPU-only forms remain in their three `x86-cpu` receivers, while real guest and
board contexts remain in `machine-table-register-board-smoke`. Test sources
add 109/remove 1,370 lines (net -1,261); gate declarations add 33/remove 44.
No production, Shared, firmware, INI or executable input changed. Complete
x64/x86 repository-only units pass 415/415 per width; specialized gates pass
66/66 per width. T317 has 35 strict receivers; T332 recognizes the three
CPU-local instruction fixtures; T344 classifies the one public board
constructor (123 total). Documentation governance and diff checks pass, and
the reviewed commit equals `origin/master`. See [S42 evidence](../etc/evidence/t539-s42-table-register-migration.md).

## S43 Acceptance

P1 `4ff59cd5c` moves every descriptor/table/cache case from the mixed source
to the CPU-only `cpu_descriptor_system_smoke.c`; its retained original source
now contains only S45's `SMSW/LMSW/CLTS/MOV CR` control-state cases. The new
receiver links only `x86-cpu`; no descriptor case remains in the S45 input.
The nine scoped P1 paths pass `git show --check` and equal `origin/master`.
No production/API, Shared, firmware, INI or executable input changed.

Complete repository-only units pass 457/457 on both x64 and x86. Both widths'
specialized aggregate passes, including T317 (36 strict CPU receivers), T332
(36 fixture owners), T337, T344, T345, CPU/PIC authority, direct-matrix,
manifest and documentation-governance gates. See
[S43 evidence](../etc/evidence/t539-s43-descriptor-system-migration.md).
S43 is accepted; S44 owns descriptor queries and S45 alone owns the retained
control-state source portion. T539 remains open.

## S44 Acceptance

S44 retires the two 1,748-line mixed LAR/LSL and VERR/VERW sources. The
CPU-only receivers retain every instruction-local selector, visibility,
operand, prefix, rollback, VM86 and LDT result without a `core_machine`
dependency. The retained 80386 board timing runner owns the four real
page-granularity LSL rows (register/memory: 21/25/22/26 ticks); public Core
receivers retain descriptor-table and IRQ delivery behavior. No production,
Shared, firmware, INI or executable input changed.

Repository-only units pass 416/416 on x64 and x86. Both specialized aggregates
pass, including T317 (36 strict CPU receivers), T332 (36 fixture owners),
T337, T344, T388, CPU/PIC authority, direct matrix, manifest and documentation
governance gates. The focused timing runner passes on x86; S44 evidence records
the receiver map and verification. S44 is accepted; S45 owns control state and
T539 remains open.

## S45 Acceptance

| Field | Contract |
| --- | --- |
| Identifier / mode | M5 T539 S45, accepted implementation. |
| Admission and approval | The owner granted automatic admission for each bounded T539 S. This packet admits the next bounded CPU-control-state batch. |
| Objective | Retire direct-private CPU access from CLTS/SMSW/LMSW/MOV-CR control-state tests while retaining one CPU-local or public-board receiver for every original case. |
| Precise scope | Consume `core_machine_clts_s62_smoke.c`, `core_machine_msw_s63_smoke.c`, and only `dt_test_msw_and_control_registers()` from `core_machine_descriptor_system_smoke.c`. Cover CR0 TS/PE effects, CR2/CR3 reads and writes, real/protected/VM86 and CPL behavior, register and memory operands, prefixes/LOCK, faults and IRQ delivery. |
| Non-goals | No CPU production change, new public API, Shared change, firmware/asset/INI/EXE update, timing reinterpretation, or migration of any remaining descriptor-system case. |
| Reference baseline | `CURRENT.md`; `t539-cpu-work-packages.md`; `t539-cpu-incremental-inventory.md`; S43/S44 evidence; the existing x86 CPU and public Core-machine test contracts. |
| Candidate implementation | Move instruction-local control semantics to an `x86-cpu` receiver using CPU-only fixtures. Retain actual fault delivery, memory boundary and PIC IRQ observations in public Core-machine receivers. Delete the old private sources only after the exact map is complete. |
| Files and ABI surface | Test, CMake gate, task-state and evidence paths only. Production and ABI surface remain unchanged. |
| Applicable rules | NXVM architecture/coding/documentation guides and shared execution, architecture, coding and documentation rules named by `docs/nxvm/README.md`. |
| Verification | Both affected targets build and pass on x86/x64; repository-only units pass 416/416 per width; T317/T332/T337/T344/T345, CPU/PIC authority, direct-matrix, manifest, documentation and diff gates pass. |
| Expected markers | Retain `M5:T316:S68:CLTS:OK` and `M5:T316:S69:MSW:OK`, or record their exact successor receiver markers in the evidence map. |
| Asset needs | None; repository-only unit fixtures only. No executable rebuild is required. |
| Reporting requirements | Record a complete original-case-to-receiver map, counted test-path delta, retained owner path, focused and full verification, and a similar-issue sweep. |
| Stop conditions | Stop for a new production/ABI requirement, an unmapped original case, a disagreement between retained behavior and the CPU authority, or any necessary scope beyond the named control-state cases. |
| Exit criteria | All named original cases have exactly one receiver; no named source retains direct-private CPU access; no duplicate production path is introduced; required verification and actual-change review pass. |
| Original owner request | Split CPU migration into small, traceable S tasks; automatically admit each, preserve clean ownership boundaries, and avoid patch-on-patch extraction. |
| Similar-issue sweep | Audit CLTS/SMSW/LMSW/MOV CR across all supported CPU profiles, operand and prefix forms, CR0/CR2/CR3 effects, privilege/VM86/fault rollback, memory and PIC delivery. |

The two complete mixed sources and the retained descriptor-system block are
retired with no duplicate execution path.  The CPU receiver owns only
instruction-local state; the public board receiver owns actual PIC delivery,
interrupt frames and the existing early-80386 board option.  The exact map and
dual-width evidence are recorded in
[S45 evidence](../etc/evidence/t539-s45-control-state-migration.md). T539
remains open for S46 and later packets.

## S46 Acceptance

| Field | Required record |
| --- | --- |
| Identifier Mode | M5 T539 S46, continuation implementation. |
| Admission And Approval | The owner granted automatic admission for each bounded T539 S. This packet admits the next finite debug-state migration batch on 2026-09-30. Allowed target: NXVM only. |
| Objective | Retire direct-private CPU access from MOV DR and TF/#DB test receivers while retaining one CPU-local or public-board receiver for every original semantic case. |
| Non-goals | No CPU production change, new public API, Shared/MyNES change, firmware, asset, INI or EXE update, timing reinterpretation, or migration of any S47-plus source. |
| Reference Baseline | This Current packet; [CPU work packages](../etc/architecture/t539-cpu-work-packages.md); [incremental inventory](../etc/evidence/t539-cpu-incremental-inventory.md); S45 evidence; existing x86 CPU fixture and public Core-machine contracts. |
| Candidate Proposal | Move MOV-DR register semantics, profile/operand/prefix/privilege rejection and synthetic state rollback to CPU-only fixtures. Retain actual exception vector delivery, #DB/#UD/IRQ ordering, trap frames, PIC acknowledgement and protected-IDT observation in public Core-machine receivers. Delete the old mixed sources only after the complete receiving map is proven. |
| Files And ABI Surface | Consume only `core_machine_debug_mov_s59_smoke.c` and `core_machine_tf_db_s60_smoke.c`; add bounded CPU and board receivers plus CMake/gate/evidence/state paths as needed. Production and ABI surface remain unchanged. |
| Applicable Rules | NXVM architecture and coding guides; shared execution, architecture, coding and documentation rules named by `docs/nxvm/README.md`. The tests retain one owner per fact and no parallel execution path. |
| Verification | Build affected x86/x64 unit targets; run complete `ctest -L unit -j8` once per width; run T317/T332/T337/T344/T345, CPU/PIC authority, direct-matrix, manifest and documentation gates; run `git diff --check`. |
| Expected Markers | Preserve the original test success coverage or record exact successor markers and original-case-to-receiver mapping in S46 evidence. |
| Asset Needs | None; repository-only unit fixtures only. No executable rebuild is required. |
| Reporting Requirements | Record every original case family, receiving owner, test-path delta, retained public board path, full verification and a similar-issue sweep. |
| Stop Conditions | Stop for a new production/ABI requirement, an unmapped original case, a disagreement between retained behavior and CPU authority, or necessary scope beyond MOV DR and TF/#DB. |
| Exit Criteria | All named original cases have exactly one receiver; neither named source retains direct-private CPU access; no duplicate production path is introduced; required verification and actual-change review pass. |
| Original Owner Request | Implement chip/device architecture extraction in small automatically admitted S tasks with explicit ownership, preservation and no patch-on-patch duplication. |
| Similar-Issue Sweep | Audit MOV DR and TF/#DB across supported CPU profiles, register forms, prefixes/LOCK, real/protected/VM86 privilege, rollback, exception vectors, trap frames and PIC ordering. |

The two named direct-private sources are retired. `cpu-debug-state-smoke`
owns MOV-DR transfer/rejection and CPU-local debug-register behavior through
the shared instruction fixture; `machine-debug-state-board-smoke` owns real
and protected exception delivery, interrupt frames and PIC ordering through
the public Core-machine boundary. Complete x64/x86 unit suites pass 416/416
per width, and the current specialized-gate aggregate passes per width. The
T332 fixture inventory classifies the new CPU receiver explicitly; no
production/API, Shared, firmware, INI or executable input changed. The
[S46 evidence](../etc/evidence/t539-s46-debug-state-migration.md) retains the
complete receiver map and validation record. S46 is accepted; T539 remains
open for S47 and later packets.

## S47 Acceptance

Actual pushed NXVM P1 `583ba9a6c` retires the five S3--S7 direct-private
protected-mode smoke consumers.  Their official targets now share one public
bootstrap fixture and preserve the five historical markers.  Architectural
gate, external/NMI, outer-stack, IRET/RETF and call-gate contracts retain
their public-board receivers; non-architectural cache mutations are retired,
not recreated as a test seam.  The 80286/80386 timing runners retain only the
renamed timing recipe for S50.

The commit changes 18 NXVM paths, adding 1,028 and removing 1,297 lines
(net -269), with no production/API, Shared, firmware, INI or executable input.
Complete repository-only units pass 512/512 on x64 and x86; both widths'
current specialized gates, the T344 constructor classification, documentation
governance and diff checks pass.  The actual commit equals `origin/master` at
review.  [S47 evidence](../etc/evidence/t539-s47-protected-16-fixture-migration.md)
contains the receiver map and retained timing boundary.  S47 is accepted;
T539 remains open for S48--S67.

## S48 Acceptance

Actual pushed NXVM P1 `a61178820` retires the 776-line private call-gate
privilege-entry smoke and replaces it with one public 80386 board receiver.
The receiver reaches ring 3 through guest GDT/LTR/IRET instructions, tests
32-bit outer call-gate parameter frames, and observes DPL, gate-type and target
descriptor rejection through real #GP delivery.  The shared bootstrap gains
one initial default-32 construction path; it does not expose a mutable CPU
seam.  Cache corruption, forced exception-delivery and shutdown-flag rows are
retired because they are not architectural guest states.  The smaller
call-gate smoke remains solely with the 80286 timing includer in S63.

Nine NXVM paths add 346/remove 814 lines (net -468), with no production/API,
Shared, firmware, INI or executable input change.  The affected marker passes
on x64 and x86; complete repository-only units pass 416/416 per width; both
specialized-gate aggregates, T344 classification, documentation governance and
diff checks pass.  No EXE rebuild is required for test/CMake/docs-only input.
See [S48 evidence](../etc/evidence/t539-s48-call-gate-privilege-entry-migration.md).
S48 is accepted; T539 remains open for S49--S67.

## S49 Acceptance

Actual pushed NXVM P1 `19ea7597e` splits the former 1,181-line mixed
control-transfer smoke at its instruction-family boundary.  The new
CPU-local receiver owns all short/near Jcc, direct short/near JMP,
LOOP/LOOPE/LOOPNE and JCXZ/JECXZ forms, their 16/32-bit code/address forms,
the four real-mode CPU profiles, 80286 near-Jcc #UD and target-limit
atomicity.  It links only `x86-cpu` and uses the CPU instruction fixture; no
Core private machine field is exposed.  The renamed retained source contains
only the pending S50 near-call/return and S51 far-transfer families, each
once.

Eight NXVM paths add 366/remove 398 lines (net -32), with no production/API,
Shared, firmware, INI or executable input change.  Complete repository-only
units pass 417/417 on x64 and x86.  Both specialized-gate aggregates,
including the 36-owner T332 lifecycle check and the 105-row retained T344
matrix, documentation governance and diff checks pass per width.  No EXE
rebuild is required for test/CMake/docs-only input.  See [S49
evidence](../etc/evidence/t539-s49-control-transfer-branch-migration.md).
S49 is accepted; T539 remains open for S50--S69.

## S50 Acceptance

Actual pushed NXVM P1 `64c261cf7` retires the direct-private near-transfer
families into one `x86-cpu` receiver: direct and register-indirect CALL,
RET/RET-immediate, 16/32-bit forms, indirect JMP and target-limit rollback.
The CPU fixture explicitly supplies stack and instruction-retirement inputs;
the retained Core source has no S50 helper, invocation or marker and now
contains only S51 far-transfer families.

Six NXVM paths add 203/remove 185 lines (net +18), with no production/API,
Shared, firmware, INI or executable input change.  Complete repository-only
units pass 418/418 on x64 and x86.  Both specialized-gate aggregates pass,
including T317 strict compilation, the 37-owner T332 lifecycle check and the
105-row retained T344 matrix; documentation governance and diff checks pass.
No EXE rebuild is required for test/CMake/docs-only input.  See [S50
evidence](../etc/evidence/t539-s50-control-transfer-near-migration.md).
S50 is accepted; T539 remains open for S51--S69.

## S51 Acceptance

Actual pushed NXVM P1 `50cd002c1` retires the final mixed direct-private
control-transfer source.  One `x86-cpu` receiver retains protected immediate
and indirect far CALL/JMP, same-CPL RETF selector rejection and rollback, all
four real-mode far/near forms, boundary far-pointer behavior and terminal
reserved-`FF` #UD behavior.  It links only the CPU instruction fixture; no
Core machine or board-private field is exposed.  The source is deleted, so no
parallel control-transfer path remains.

Ten NXVM paths add 462/remove 649 lines (net -187), with no production/API,
Shared, firmware, INI or executable input.  The two protected-16 board callers
only receive their already-required `default32 = false` test-helper argument,
which restores the current helper contract without changing their scenarios.
Fresh x64/x86 builds and complete repository-only units pass 418/418 per
width.  The specialized gates, 38-owner T332 lifecycle check, 104-row T344
inventory, documentation governance, direct-private sweep and diff checks
pass.  No EXE rebuild is required for test/CMake/docs-only input.  See [S51
evidence](../etc/evidence/t539-s51-control-transfer-receiver-map.md).  S51 is
accepted; T539 remains open for S52--S69.

## S52 Acceptance

Actual pushed NXVM P1 `bec9e0a70` retires both direct-private IDT and
privilege-entry sources.  Their cases now have exactly three receivers:
software `INT` DPL/gate/frame and rollback semantics are CPU-local;
external-IRQ DPL bypass is an independent PIC board receiver; and full-Core
delivery plus handler continuation uses only public Core operations.  No
receiver borrows executor or shared-PIC fields, and no parallel private setup
path remains.

Ten NXVM paths add 487/remove 398 lines (net +89), with no production/API,
Shared, firmware, asset, INI or executable input change.  Focused x64/x86
receivers and complete repository-only units pass 419/419 per width.  Both
specialized-gate aggregates, the 39-owner T332 lifecycle inventory, the
103-row T344 matrix, documentation governance, direct-private sweep and diff
checks pass.  No EXE rebuild is required.  See [S52
evidence](../etc/evidence/t539-s52-idt-privilege-receiver-map.md).  S52 is
accepted; T539 remains open for S53--S69.

## S53 Acceptance

P1 `89eeef93c` retires the 934-line protected far/data pair into two CPU-local
receivers and two PIC board receivers.  The CPU cases retain descriptor/cache,
data-access and all-or-nothing fault assertions; the board cases additionally
prove real PIC IRR-to-ISR acknowledgement and interrupt-frame publication.
No production/public API, Shared, firmware, asset, INI or EXE input changes.

The focused receivers and complete repository-only x64/x86 unit suites pass.
T317 confirms 41 strict CPU compile commands, T332 confirms 41 fixture owners,
T344 confirms 101 direct constructors; direct-private, documentation-governance
and diff checks pass.  The [S53 receiver map](../etc/evidence/t539-s53-protected-far-data-receiver-map.md)
records the exact allocation.  S53 is accepted; T539 remains open for S54-S69.

## S54 Acceptance

P1 `6d65ce0cc` retires the 1,003-line outer-return pair into exactly two
receivers. `cpu_outer_return_smoke` owns 80286/80386 outer RETF and IRET
frames, prefix and width forms, cached segment restoration and exact
all-or-nothing exception observations. `machine_outer_iret_pic_board_smoke`
alone owns real PIC IRR-to-ISR acknowledgement and the externally delivered
outer-IRET interrupt frame. The CPU receiver constructs neither a Core machine
nor a PIC; the board receiver uses only the public CPU-bus/PIC contract.

Ten NXVM paths add 575/remove 1,019 lines (net -444); no production/API,
Shared, firmware, asset, INI or executable input changes. Focused receivers
and complete repository-only x64/x86 units pass 421/421 per width. All 66
specialized gates pass, including the 41-owner T332 lifecycle check, 41-command
T317 strict compilation audit and 101-row T344 fixture-shape inventory;
documentation governance and diff checks pass. No EXE rebuild is required.
The [S54 receiver map](../etc/evidence/t539-s54-outer-return-receiver-map.md)
records the exact allocation. S54 is accepted; T539 remains open for S55-S69.

## S55 Acceptance

S55 retires the 16-bit/task-gate half of `core_machine_task_switch_smoke.c`.
The CPU-only receiver covers direct and indirect task JMP, task CALL/GDT task
gate, LDT, nested return, IDT/double-fault task gates and every named selector,
presence, busy, short-TSS, stack and LOCK failure route. The sole board
receiver covers a real PIC IRQ0 IRR→ISR acknowledgement after a task switch.
All S55-only private helpers are deleted; the residual legacy source contains
only S56's four 80386 operand/address-size forms. Complete units pass 423/423
on x64 and x86; all 66 specialized gates, the repaired CPU-boundary negative
test, Lib manifest and documentation governance pass. No production/API,
Shared, firmware, asset, INI or EXE input changed, so no EXE rebuild is
required. The [S55 receiver map](../etc/evidence/t539-s55-task-switch16-receiver-map.md)
records the exact allocation. S55 is accepted; T539 remains open for S56-S69.

## S75 Acceptance

S75 replaces the board's embedded CPU, decoder and execution layout with one
`core_machine_cpu_execution_context *` lifetime. `cpu.c` owns its allocation
and destruction; the board only creates it with its CPU-bus provider, binds its
existing profile/FPU/diagnostic inputs, and passes that same opaque context to
prepared entry, reset, execution and destruction. No public mutable CPU
accessor, layout mirror or compatibility path was added.

The CPU test fixture is the only private-layout consumer: it borrows the
opaque owner's CPU/decoder exclusively to prepare historical instruction-state
inputs. Production headers now import the opaque CPU interface rather than the
private decoder definition. Complete repository-only units pass 426/426 on
both x64 and x86; lifetime, CPU/PIC authority and historical-fixture gates pass
on both widths. Because production executable inputs changed, all four
admitted NXVM profiles have rebuilt 0539 x64/x86 Release artifacts. The
[S75 evidence](../etc/evidence/t539-s75-opaque-cpu-lifetime.md) records the
boundary and artifact hashes. S75 is accepted; T539 remains open for S76-S82.

## S81 Acceptance

S81 moved the five 1,804-line CPU-only `MOVS`/`LODS`/`STOS`/`SCAS`/`CMPS`
test suites to one Shared CPU owner and deleted the NXVM duplicates. Real Core
memory, interruptibility, and PIC/IRQ observations remain NXVM board tests.
Complete repository-only units pass 427/427 on x64 and x86. Shared
manifest/corpus, CPU/PIC authority, documentation governance, and diff checks
pass. This test/CMake-only scope changes no firmware, asset, INI, or EXE
input; the 0539 artifacts remain current. See the
[S81 evidence](../etc/evidence/t539-s81-string-receivers.md). S81 is
accepted; T539 remains open for S82-S86.

## S82 Acceptance

S82 moved the two 642-line CPU-only scalar/string port suites and their sole
68-line CPU-bus fixture to one Shared owner, then deleted NXVM duplicates.
Actual port routing, permission, and PIC/IRQ observations remain named NXVM
board tests. Complete repository-only units pass 429/429 on x64 and x86.
CPU/PIC authority, Shared manifest/corpus, documentation governance, and diff
checks pass. This test/CMake-only scope changes no firmware, asset, INI, or EXE
input. See the [S82 evidence](../etc/evidence/t539-s82-port-receivers.md).
S82 is accepted; T539 remains open for S83-S94 after the protected/system
work-package split.

## S83 Acceptance

S83 moved the four 1,143-line protected descriptor-operand suites and their
sole 121-line descriptor-query fixture to one Shared CPU owner. NXVM removes
the duplicate tests and its static inventories use the same Shared targets.
The named ARPL/BOUND public-board receivers retain real board semantics.
Complete repository-only units pass 433/433 on x64 and x86. CPU/PIC authority,
Shared manifest/corpus, documentation governance, and diff checks pass. This
test/CMake-only scope changes no firmware, asset, INI, or EXE input. See the
[S83 evidence](../etc/evidence/t539-s83-descriptor-operands.md). S83 is
accepted; T539 remains open for S84-S94.

## S80 Acceptance

S80 moved three 2,105-line CPU-only segment/data tests to one Shared owner and
deleted NXVM duplicates. Public Core memory/fault/IRQ observations remain NXVM
tests. See the [S80 evidence](../etc/evidence/t539-s80-segment-data-receivers.md).

Complete x64/x86 repository-only units pass 427/427. Shared manifest/corpus,
CPU/PIC authority, documentation governance, and diff checks pass. This
test/CMake-only scope leaves the S76 0539 artifacts current. S80 is accepted;
T539 remains open for S81-S86.

## S79 Acceptance

S79 moved five 1,883-line CPU-only segment-stack/far-pointer tests to one
Shared owner and deleted NXVM duplicates. Board-specific descriptor,
memory/fault, and IRQ observations remain NXVM tests. See the
[S79 evidence](../etc/evidence/t539-s79-segment-stack-receivers.md).

Complete x64/x86 repository-only units pass 427/427. Shared manifest/corpus,
CPU/PIC authority, documentation governance, and diff checks pass. This
test/CMake-only scope leaves the S76 0539 artifacts current. S79 is accepted;
T539 remains open for S80-S86.

## S78 Acceptance

S78 moved the four 1,858-line CPU-only basic-stack tests to one Shared owner
and deleted the duplicate NXVM paths. Real Core stack/fault/IRQ cases remain
explicit NXVM board tests. The [S78 evidence](../etc/evidence/t539-s78-basic-stack-receivers.md)
records receiver ownership and verification.

Complete repository-only units pass 427/427 on x64 and x86, and Shared
manifest/corpus, CPU/PIC authority, documentation governance, and diff checks
pass. Test/CMake-only scope leaves S76's 0539 product artifacts current. S78 is
accepted; T539 remains open for S79-S86.

## S77 Acceptance

S77 established the first Shared CPU-only test receiver group: ten arithmetic,
FLAGS, rotate, and direct-register sources plus their one real fixture now live
under `test/x86/devices/cpu/`.  The historic NXVM fixture is only a forwarding
include for explicitly assigned S79--S85 callers; it carries no state or test
registration.  Board paths stayed NXVM and are listed in the
[S77 evidence](../etc/evidence/t539-s77-cpu-arithmetic-receivers.md).

The final 427-test repository-only unit suite passed on both x64 and x86.
Shared corpus/manifest, CPU/PIC authority, documentation governance, and diff
checks pass. No runtime CPU or product input changed, so the S76 0539 artifacts
remain current. S77 is accepted; T539 remains open for S78--S82.

## S76 Acceptance

S76 established `src/x86/devices/cpu/` as the canonical nine-file CPU corpus.
`x86-cpu-shared` is the sole implementation target; NXVM's historic
`x86-cpu` spelling is a CMake alias, not a wrapper or duplicate object path.
All active NXVM consumers and static authority gates now use the canonical
Shared headers/sources. The retained App CPU files are inert historical source
only; S81 owns their physical deletion.

Shared corpus, opaque contract, CPU-bus negative controls and focused timing
runners pass on both widths. Complete repository-only NXVM units pass 427/427
on each width. All eight 0539 profile/width artifacts were rebuilt and their
PE architectures plus hashes are recorded in the [S76 evidence](../etc/evidence/t539-s76-shared-cpu-cutover.md).
S76 is accepted; T539 remains open for S77-S82.

## S74 Acceptance

`machine_cpu_profile_gate_smoke.c`, `machine_fpu_escape_smoke.c` and
`machine_fpu_interface_s65_smoke.c` are now the sole Core-machine receivers
for the remaining profile/FPU inputs.  `support/machine_cpu_fixture.h` is the
one private prepared-state fixture; all 31 test-only consumers use it, with no
compatibility include or second fixture path.  The CPU-boundary, T332 lifecycle
and T344 shape gates recognize the successor names and still reject the private
fixture outside its intended boundary.

Focused successor receivers and the CPU-boundary negative gate pass on x86 and
x64.  All affected consumers rebuilt on both widths.  Complete repository-only
unit suites pass 426/426 on x86 in 100.66 seconds and 426/426 on x64 in 99.72
seconds; Types, T344, T332, VM lifecycle, CPU/PIC authority, T388,
documentation-governance and diff checks pass per width.  No production/API,
Shared, firmware, asset, INI or executable input changed, so no product binary
rebuild is required.  See the [S74 receiver map](../etc/evidence/t539-s74-remaining-consumer-fixture-map.md).
S74 is accepted; T539 remains open for S75-S77.

## S73 Acceptance

`machine_80386_timing_manifest_runner.c` is the sole Core-machine receiver
for the 80386 timing-manifest context. It retains source formulas,
manifest/catalog rows and measured results without adding a generated
production dependency or a second recipe path.

Focused x64/x86 receivers pass; complete repository-only unit suites pass
426/426 on x86 in 98.16 seconds and 426/426 on x64 in 98.08 seconds. T344
registration and historical fixture shapes, T332 lifecycle, VM-machine
lifecycle, Core CPU/PIC authority, T388 lexeme and physical eligibility,
documentation governance and diff checks pass. No Shared source, firmware,
asset, INI or EXE input changed, so no product binary rebuild is required.
The [S73 receiver map](../etc/evidence/t539-s73-80386-timing-receiver-map.md)
records the exact scope. S73 is accepted; T539 remains open for S74-S77.

## S72 Acceptance

`machine_80286_timing_manifest_runner.c` and its direct
`machine_call_gate_smoke.c` includer are the sole Core-machine receiver/include
closure for the 80286 timing-manifest context. They retain source formulas,
manifest/catalog rows, measured results and the existing public call-gate
construction without adding a generated production dependency or a second
recipe path.

Focused x64/x86 receivers pass; complete repository-only unit suites pass
426/426 on x86 in 105.59 seconds and 426/426 on x64 in 105.52 seconds. T344
registration and historical fixture shapes, T332 lifecycle, VM-machine
lifecycle, Core CPU/PIC authority, T388 lexeme and physical eligibility,
documentation governance and diff checks pass. No Shared source, firmware,
asset, INI or EXE input changed, so no product binary rebuild is required.
The [S72 receiver map](../etc/evidence/t539-s72-80286-manifest-call-gate-receiver-map.md)
records the exact scope. S72 is accepted; T539 remains open for S73-S77.

## S71 Acceptance

`machine_80286_instruction_timing_ledger_smoke.c` and
`machine_80386_protected_io_timing_smoke.c` are the sole Core-machine
receivers for their 80286 ledger and 80386 protected-I/O timing contexts.
They retain the source formulas, protected-port rows and measured results
without adding a generated production dependency or a second recipe path.

Focused x64/x86 receivers pass; complete repository-only unit suites pass
426/426 on x86 in 103.32 seconds and 426/426 on x64 in 103.50 seconds. T344
registration and historical fixture shapes, T332 lifecycle, VM-machine
lifecycle, Core CPU/PIC authority, T388 lexeme and physical eligibility,
documentation governance and diff checks pass. No Shared source, firmware,
asset, INI or EXE input changed, so no product binary rebuild is required.
The [S71 receiver map](../etc/evidence/t539-s71-80286-protected-io-timing-receiver-map.md)
records the exact scope. S71 is accepted; T539 remains open for S72-S77.

## S70 Acceptance

`machine_80186_instruction_timing_ledger_smoke.c` and
`machine_80186_timing_manifest_runner.c` are the sole 80186 Core-machine
timing receivers. They retain the source formulas, generated manifest catalog
and measured results without adding a generated production dependency or a
second recipe path.

Focused x64/x86 receivers pass; complete repository-only unit suites pass
426/426 on x86 and x64. T344 registration and historical fixture shapes, T332
lifecycle, VM-machine lifecycle, Core CPU/PIC authority, T388 lexeme and
physical eligibility, documentation governance and diff checks pass. No
firmware, asset, INI or EXE input changed, so no product binary rebuild is
required. The [S70 receiver map](../etc/evidence/t539-s70-80186-timing-receiver-map.md)
records the exact scope. S70 is accepted; T539 remains open for S71-S77.

## S69 P1 Acceptance

P1 corrected the private Shared `x86_video_active_ega_aperture()` failure
contract before the timing receiver rename. The helper now reports invalid
arguments and each of its planar-offset, write-observer and containment
callers has one explicit failure result. Valid aperture maps, offsets and
dirty observation are unchanged. The P1 commit is `429407e84`; dual-width EGA
focused tests, complete 426/426 unit suites, all applicable gates and
documentation governance passed. No public API, asset, INI or EXE input
changed. P2 is recorded by the S69 acceptance below.

## S69 Acceptance

P1 is the approved Shared precondition and P2 makes
`machine_8086_instruction_timing_ledger_smoke.c` and
`machine_8086_timing_manifest_runner.c` the sole 8086 Core-machine timing
receivers. The latter is compiled once per explicit 8086/8088 profile, without
duplicating its recipe executor or generated-catalog path. Formula rows,
timing results and decoder inventory remain unchanged.

Focused x64/x86 8086/8088 ledger, manifest, result and decoder-ledger tests
pass; complete repository-only unit suites pass 426/426 on x86 and x64. T344
registration and historical fixture shapes, T332 lifecycle, VM-machine
lifecycle, Core CPU/PIC authority, T388 lexeme and physical eligibility,
documentation governance and diff checks pass. No firmware, asset, INI or EXE
input changed, so no product binary rebuild is required. The
[P1 Shared evidence](../etc/evidence/t539-s69-p1-shared-video-aperture.md) and
[P2 receiver map](../etc/evidence/t539-s69-8086-timing-receiver-map.md)
record the exact scope. S69 is accepted; T539 remains open for S70-S77.

## S68 Acceptance

`machine_instruction_timing_smoke.c`, `machine_instruction_timing_ledger_smoke.c`,
`machine_legacy_timing_normalization_s2_smoke.c` and
`machine_t359_s2_timing_smoke.c` through `machine_t359_s6_timing_smoke.c`
are the sole named Core-machine receivers for the common timing baseline.
They retain the original formula rows, execution-provider observations and
normalization assertions without changing a timing algorithm or production API.

Focused x64/x86 receivers pass and retain `M5:T265:S3:INSTRUCTION-TIMING:OK`,
`M5:T357:S3:INSTRUCTION-TIMING-LEDGER:OK`,
`M5:T362:S2:LEGACY-TIMING-NORMALIZATION:OK`, and all T359 S2-S6 markers.
Complete repository-only unit suites pass 426/426 on x86 and x64. T344
registration and historical fixture shapes, T332 lifecycle, VM-machine
lifecycle, Core CPU/PIC authority, T388 lexeme and physical-eligibility gates,
documentation governance and diff checks pass. This is test/CMake/documentation
work only: no production/API, Shared, firmware, asset, INI or EXE input changed,
so no EXE rebuild is required. The [receiver map](../etc/evidence/t539-s68-timing-baseline-receiver-map.md)
records the complete allocation. S68 is accepted; T539 remains open for S69-S77.

## S67 Acceptance

`machine_vm86_delivery_smoke.c`, `machine_vm86_iret_smoke.c`,
`machine_vm86_lgdt_lidt_s5_smoke.c` and
`machine_hardware_delivery_s3_smoke.c` are the sole named public Core-machine
receivers for the VM86 dependency group. They retain VM86 interrupt frames,
LGDT/LIDT validation, paging faults and real hardware IRQ delivery; direct
includers reuse the same fixture rather than introduce another execution path.

Focused x64/x86 receivers pass and emit `M5:T539:S67:VM86:OK` together with
their retained historical VM86 markers. Complete repository-only unit suites
pass 426/426 on x64 and x86. T344 registration and historical fixture shapes,
T332 lifecycle, VM-machine lifecycle, Core CPU/PIC authority, documentation
governance and diff checks pass on both widths. This is test/CMake/documentation
work only: no production/API, Shared, firmware, asset, INI or EXE input changed,
so no EXE rebuild is required. The [receiver map](../etc/evidence/t539-s67-vm86-receiver-map.md)
records the complete allocation. S67 is accepted; T539 remains open for S68-S72.

## S66 Acceptance

`machine_interrupt_entry_smoke.c` is the unique named Core-machine receiver
for GDT/IDT gate construction, software-INT entry, PIC/NMI delivery and
fault escalation.  Its direct S66 software-INT includer reuses this setup;
the S67 hardware-delivery includer receives only the mechanical new filename
and retains all VM86 behavior for its own package.

Focused x64/x86 receiver and both includers pass and emit
`M5:T539:S66:INT-ENTRY:OK`.  Complete repository-only unit suites pass 426/426
on both widths.  T332 lifecycle, T344 registration and historical fixture
shapes, VM lifecycle, Core CPU/PIC authority, documentation governance and
`git diff --check` pass on both widths.  This is test/CMake/documentation-only
work: no production/API, Shared, firmware, asset, INI or EXE input changed, so
no EXE rebuild is required.  The [receiver map](../etc/evidence/t539-s66-int-entry-receiver-map.md)
records the allocation.  S66 is accepted; T539 remains open for S67-S72.

## S65 Acceptance

`machine_protected_iret_smoke.c` is the unique named Core-machine receiver for
the protected IRET dependency group.  It preserves real/protected/outer IRET
frame construction, validation and all-or-nothing fault delivery through public
machine setup.  Its sole direct includer,
`core_machine_iret_s51_smoke.c`, reuses that receiver rather than duplicating
the descriptor and frame setup.

Focused x64/x86 receiver and includer runs pass and emit
`M5:T539:S65:PROTECTED-IRET:OK`.  Complete repository-only unit suites pass
426/426 on both widths.  T332 lifecycle, T344 registration and historical
fixture shapes, VM lifecycle, Core CPU/PIC authority, documentation governance
and `git diff --check` pass on both widths.  This is test/CMake/documentation-
only work: no production/API, Shared, firmware, asset, INI or EXE input changed,
so no EXE rebuild is required.  The [receiver map](../etc/evidence/t539-s65-protected-iret-receiver-map.md)
records the allocation.  S65 is accepted; T539 remains open for S66-S72.

## S64 Acceptance

`machine_cli_sti_interrupt_smoke.c` is the unique named Core-machine receiver
for real/protected/VM86 CLI/STI construction, IF/shadow, PIC IRQ/mask and
guest-visible frame behavior. Its direct S64 includers—80286 CLI/STI, HLT and
INT-to-IRET-to-IRQ composition—reuse that receiver rather than rebuilding a
second execution-provider or IRQ setup. Later IRET and software-INT sources
received only the required mechanical filename update; their behavior remains
allocated to S65 and S66.

Focused x64/x86 runs emit `M5:T539:S64:CLI-STI-INTERRUPT:OK`; final serial
repository-only units pass **426/426** on x64 and x86. T332 lifecycle, T344
registration and fixture shape, VM lifecycle, Core CPU/PIC authority,
documentation governance and `git diff --check` pass. This is test/CMake/
documentation-only work: no production/API, Shared, firmware, asset, INI or
EXE input changed, so no EXE rebuild is required. The [receiver map](../etc/evidence/t539-s64-cli-sti-receiver-map.md)
records every context. S64 is accepted; T539 remains open for S65-S72.

## S63 Acceptance

`machine_tss_iomap_port_authorization_smoke.c` is now the sole named
receiver for TSS I/O-map authorization. It preserves real protected entry,
`LTR`, bitmap bounds, CPL/IOPL decisions, #GP delivery and actual public port
provider effects. No task-switch fixture or second authorization path was
introduced.

Focused x64/x86 runs emit `M5:T539:S63:TSS-IOMAP:OK`; final serial
repository-only units pass **426/426** on both widths. T344 shape and
registration, Core CPU/PIC authority, VM lifecycle, documentation governance
and `git diff --check` pass. This is test/CMake/documentation-only work: no
production/API, Shared, firmware, asset, INI or EXE input changed, so no EXE
rebuild is required. The [receiver map](../etc/evidence/t539-s63-tss-iomap-receiver-map.md)
records every authorization context. S63 is accepted; T539 remains open for
S64-S69.

## S62 Acceptance

The final residual task-switch receiver is now
`machine_task_switch_cross_width_smoke.c`, replacing the misleading mixed
`core_machine_task_switch_smoke.c` name and target. It alone executes the
eight 16→32 and 32→16 direct/nested/task-gate/return cases, preserving each
old/new TSS image, TR and busy state, backlink/NT state, and nested IRET
return. The existing 80286 and 80386 timing runners include that same source
under their private `main` rename; no duplicate fixture was introduced.

Focused x64/x86 runs pass and emit `M5:T539:S62:TASK-CROSS-WIDTH:OK`. Final
serial repository-only unit suites pass **426/426** on both x64 and x86.
T317/T332, T344 shape/declaration/direct-compilation/registration, Core
CPU/PIC authority and machine lifecycle gates pass on both widths;
documentation governance and `git diff --check` pass. This is
test/CMake/documentation-only work: no production/API, Shared, firmware,
asset, INI or EXE input changed, so no EXE rebuild is required. The
[receiver map](../etc/evidence/t539-s62-task-cross-width-receiver-map.md)
records all eight contexts. S62 is accepted; T539 remains open for S63-S69.

## S61 Acceptance

The ordinary TSS32 far-JMP and nested TSS32 far-CALL rows with a pending IRQ
now have one board/Core receiver, `machine_task_switch32_paging_smoke.c`.
It uses guest I/O to initialise and unmask the master PIC, injects a real
keyboard byte through the public Core board input, and observes the resulting
KBC-to-PIC-to-CPU delivery while executing the real task transfer. The
legacy mixed runner no longer contains its direct-private pending-IRQ helper.

Focused x64/x86 receivers and the retained corpus pass. Complete serial unit
suites pass **426/426** on x64 and **426/426** on x86. The directly invoked
T344 historical-shape and Core lifecycle scripts pass; documentation
governance and `git diff --check` pass. The CMake/Ninja batch invocation for
the remaining static targets stalled without CPU progress on this host, so it
is deliberately not recorded as a passing result. This is test and
documentation-only work: no production/API, Shared, firmware, asset, INI or
EXE input changed, so no EXE rebuild is required. The [receiver map](../etc/evidence/t539-s61-task-switch32-pending-irq-receiver-map.md)
records the two contexts and their sole delivery route. S61 is accepted;
T539 remains open for S62-S69.

## S60 Acceptance

The eleven nested CALL/JMP, task-gate, nested-return and rejection rows now
have one CPU-local receiver, `cpu_task_switch32_state_smoke`.  It executes
real guest table loads and task transfers over copied CPU memory, including a
correctly aligned task-gate descriptor and the actual LDT descriptor limit.
The mixed legacy runner retains only S61's two pending-IRQ contexts.

Focused x64/x86 receivers and the retained corpus pass.  The complete x64
suite passes 426/426.  The x86 suite's 424 unaffected rows passed in the full
serial run; its two transient failures each passed immediately when rerun in
isolation, including the fixed-path CPU-boundary negative gate. T317/T332,
T344, Core CPU/PIC authority and lifecycle checks, documentation governance
and `git diff --check` pass.  This is test/CMake/documentation-only work; no
production/API, Shared, firmware, asset, INI or EXE input changed, so no EXE
rebuild is required. The [receiver map](../etc/evidence/t539-s60-task-switch32-nesting-receiver-map.md)
records the exact allocation. S60 is accepted; T539 remains open for
S61-S69.

## S59 Acceptance

The two direct TSS32 paging contexts now have one public-Core receiver,
`machine_task_switch32_paging_smoke.c`.  It builds real guest page tables,
GDT, IDT and TSS images through public physical-memory writes, performs guest
`LGDT`/`LIDT`/`LMSW`/`LTR`, and observes a mapped target transition plus an
unmapped target-TSS `#PF` through the public diagnostic and copied snapshot
contracts.  The retained mixed source contains neither S59 page-table state
nor private IDTR mutation.  The [receiver map](../etc/evidence/t539-s59-task-switch32-paging-receiver-map.md)
records the exact boundary.

Focused x64/x86 receivers and the retained task-switch corpus pass. Complete
repository-only unit suites pass 426/426 on x64 and x86 when serially run;
T317, T332, T344, Core CPU/PIC authority and lifecycle gates, documentation
governance and `git diff --check` pass on both widths. This is
test/CMake/documentation-only work: no production/API, Shared, firmware,
asset, INI or EXE input changed, so no EXE rebuild is required. S59 is
accepted; T539 remains open for S60-S69.

## S56 Acceptance

The four functional 80386 `66h`/`67h` task-JMP contexts now have one
CPU-local receiver, `cpu_task_switch32_decode_smoke.c`, sharing the established
S55 TSS16 fixture. The legacy mixed source retains only the encoding recipes
needed by the S65 timing-runner includer; it no longer invokes these functional
rows. The [receiver map](../etc/evidence/t539-s56-task-switch32-decode-map.md)
records the boundary.

Focused x64/x86 receivers and the retained mixed runner pass. Complete units
pass 424/424 on x64 and x86; all 66 specialized gates, documentation governance
and `git diff --check` pass. This is test/CMake/documentation-only work: no
production/API, Shared, firmware, asset, INI or EXE input changed, so no EXE
rebuild is required. S56 is accepted; T539 remains open for S57-S69.

## S57 Acceptance

Direct TSS32 baseline, operand/address forms, descriptor/LDT state and fault
rows now have one CPU-local receiver, `cpu_task_switch32_state_smoke.c`. Its
fixture executes `LGDT` and `LTR`; it does not fabricate a TR cache. The
[receiver map](../etc/evidence/t539-s57-task-switch32-state-map.md) records
the exact 17 contexts. Its then-residual mixed source retained only the later
exceptional and cross-width rows.

Focused x64/x86 receivers and the retained mixed runner pass. Complete units
pass 425/425 on x64 and x86; documentation governance and `git diff --check`
pass. This is test/CMake/documentation-only work: no production/API, Shared,
firmware, asset, INI or EXE input changed, so no EXE rebuild is required. S57
is accepted; T539 remains open for S58-S69.

## S58 Acceptance

The direct TSS32 debug-trap word and LOCK direct/indirect task-JMP cases now
have one CPU-local receiver, `cpu_task_switch32_state_smoke.c`; its genuine
LGDT/LTR setup supplies the task state without a fabricated TR cache. The
residual mixed source no longer invokes any of these five contexts. A first
principles fixture check established that paging does not belong in that
receiver: its CPU bus has no Core physical-translation binding, so S59 owns
the paging rows through a public Core receiver instead.

Focused x64/x86 receivers and the retained mixed runner pass. Complete x64/x86
units pass 425/425 each. T317 and T332 pass on both widths with 44 strict
CPU receivers/fixture owners; documentation governance and `git diff --check`
pass. This is test/CMake/documentation-only work: no production/API, Shared,
firmware, asset, INI or EXE input changed, so no EXE rebuild is required.
S58 is accepted; T539 remains open for S59-S69.

## S29 Acceptance

Actual pushed NXVM P1 `86fe95201` has exactly 13 scoped paths, passes
`git show --check`, and equals `origin/master` at review. All 28 original
operand contexts and all twelve original prefix groups retain CPU or board
receivers; two fault contexts have complementary observations. x64 and x86
builds and complete units pass 389/389 each, along with 66 specialized gates,
extended CPU-boundary negatives, six unchanged manifests and documentation
governance. The nine test/build paths add 1,514/remove 1,311 lines; no
production/API or executable input changed. See
[S29 evidence](../etc/evidence/t539-s29-operand-prefix-migration.md). S29 is
accepted; 82 original private consumers remain assigned to S30-S42. S43 owns
opaque lifetime, S44 physical Shared relocation and S45 whole-CPU acceptance.

## S28 Acceptance

Actual-commit review accepts pushed NXVM P1 `fa092b95e`: the 244 original
execution contexts and three metadata queries retain CPU/board receivers;
the inverted 286 test result and incorrect EAX expectation are corrected.
Full x64/x86 units pass 387/387 each, with 66 specialized gates, six unchanged
manifests and documentation/diff checks. Ten test/build paths add 2,065/remove
1,530 lines, net +535; no production or executable input changed. See
[S28 evidence](../etc/evidence/t539-s28-segment-migration.md). The S28 packet
is closed, not carried into S29.

CPU extraction itself is not accepted. The
[inventory](../etc/evidence/t539-cpu-incremental-inventory.md) assigns the
remaining 56 original direct-private `.c` test consumers and one fixture
header to S41-S51. Embedded CPU lifetime remains until S52; physical Shared
relocation is S53; whole CPU acceptance is S54. S41 owns the unresolved
32-bit BOUND observation. None is silently closed or transferred to the next T.

## Accepted Progress

| Task | Progress |
| --- | --- |
| T539 S44 | Accepted: descriptor-query cases move to CPU-local LAR/LSL and VERR/VERW receivers; the retained 80386 board timing runner covers LSL 21/25/22/26 source ticks. Units 416/416 per width. No production or asset change. |
| T539 S40 | Accepted: NXVM P1 e4a7615d1 migrates ARPL ownership; original base/S53 cases retain CPU/board receivers; units 413/413 per width. No production or asset change. |
| T539 S39 | Accepted: NXVM P1 07019f588 migrates port I/O ownership; all 230 original scalar/string contexts retain CPU/board receivers; units 413/413 per width. No production or asset change. |
| T539 S38 | Accepted: NXVM P1 ae76a7c84 migrates STOS/SCAS/CMPS ownership; all 203 original contexts retain CPU/board receivers; units 411/411 per width. No production or asset change. |
| T539 S31 | Accepted: NXVM P1 38bc5b10c migrates immediate IMUL and Group-2 test ownership; all 335 original contexts retain CPU/board receivers; units 397/397 per width. No production or asset change. |
| T539 S30 | Accepted: NXVM P1 442088410 migrates bit/condition/extension ownership. All 549 original contexts retain CPU/board receivers; units 395/395 per width. No production or asset change. |
| T539 S29 | Accepted: NXVM P1 86fe95201 migrates operand/address and S64 prefix ownership. All 28 operand contexts and twelve prefix groups retain receivers; units 389/389 per width. No production or asset change. |
| T539 S28 | Accepted: NXVM P1 fa092b95e migrates segment selector/SREG MOV test ownership. All 244 original contexts and three queries retained; units 387/387 per width. No production or asset change. |
| T539 S27 | Accepted: NXVM P1 42d6c86e0 migrates far-pointer test ownership. All 117 original contexts retained; units 385/385 per width. No production or asset change. |
| T539 S26 | Accepted: NXVM P1 587a91af9 migrates segment-stack test ownership. All 164 original contexts retained; units 382/382 per width. No production or asset change. |
| T539 S25 | Accepted: NXVM P1 2cb8b64f7 migrates ENTER/LEAVE test ownership. All 53 original contexts retained; units 380/380 per width. No production or asset change. |
| T539 S24 | Accepted: NXVM P1 ff09b22a3 migrates GPR stack test ownership. All 198 original contexts retained; units 379/379 per width. No production or asset change. |
| T539 S23 | Accepted: NXVM P1 5c2835936 migrates XCHG test ownership. All 101 original instruction contexts retained; units 376/376 per width. No production or asset change. |
| T539 S22 | Accepted: NXVM P1 9e5382872 migrates GPR MOV/MOFFS test ownership. All 277 original contexts retained; units 375/375 per width. No production or asset change. |
| T539 S21 | Accepted: NXVM P1 1049b9021 migrates LEA/MOVX test ownership and divides the oversized instruction batch. Units 373/373 per width. Production, EXEs and INIs unchanged. |
| T539 S20 | Accepted: NXVM P1 af06a6259 qualifies copied observations, debug/reset adapters and board access. Units 371/371 per width. |
| T539 S19 | Accepted: NXVM P1 f1b43af46 qualifies CPU bus transactions, failure effects and imports. Units 371/371 per width. |
| T539 S18 | Accepted: NXVM P1 0067d80c4 restores the incremental baseline and preserves pending migrations. Units 370/370 per width; default integration 20/20 per width; tools-off 45/45; six vendor boots once. |

The [proposal](../proposals/m5-shared-chip-extraction.md),
[finite ledger](../etc/evidence/t539-chip-migration-ledger.md),
[CPU boundary](../etc/architecture/t539-s18-cpu-extraction.md) and
[task history](../history/M5-T539-independent-shared-chips.md) retain the
complete scope, original requirements, earlier acceptance and receiving proof.
The historical S18-S20 prospective numbering is superseded only for unadmitted
packages by the current work plan.

## Current Technical Baseline

Four fixed products remain XT, AT, Model40 and default PC/AT; PC110 is not
runnable. Eight optimized compiler-debug-stripped 0539 EXEs are committed in
0067d80c4 with unchanged owner INIs. S18 evidence records hashes, PE architecture
and verification limits. S19-S39 changed no executable inputs and require no
new artifact. Both reusable NXVM trees remain configured for default; the three
bounded build/t539-s3 trees and S18 recovery patch remain needed for later CPU
batches. Run native desktop test suites without cross-tree overlap.

Lib/Common retain 268464d49. Shared x86 PIT is accepted at 24162ac93, RTC at
8a8435648, PIC at d6dc6ca3a, DMA at 53b4be21d, AT keyboard at eb1e2e208,
XT PPI/keyboard at 0f9c6b1a8, FDC at 1d6dc5876, HDC at 9020d8bba, video at
cadaf0990 and FPU at 5fa831a2b. MyNES retains its unchanged 0043 pair: its link
inputs do not include these x86 chip targets.

The owner-approved S12 firmware route remains: project BIOS source lives in
app-nxvm/firmware; commercial originals stay external BYOB; selected ROM bytes
are embedded into the committed EXEs. No runtime ROM-file fallback, host BIOS
service, external-master or owner-INI change is introduced by the CPU batches.

The seven [Queue](QUEUE.md) candidates retain dependency order. Acceptance does
not claim indefinite absence of intermittent faults.

## Historical Context

[Earlier status](../history/M6-T41-and-prior-status-archive.md) preserves prior
deliveries. [T538 history](../history/M5-T538-deployed-boot-pairs.md) retains
earlier boot qualification. [S12 evidence](../etc/evidence/t539-s12-fdc-drive-status.md)
owns preceding FDC qualification and embedded-firmware recovery.
