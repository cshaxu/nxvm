# MyRaidenII Status

## Current Work

M1 T2 S24 implementation is delivered at Shared P1 `ceab5712c` and NXVM
receiving P2 `1153273ac`. Full packet/evidence delivery and coordinator
actual-change review precede a separate governance acceptance. T2 stays open.

## T2 S24 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation, MyRaidenII M1 T2 S24 after accepted/pushed S23; no new T allocation. |
| Admission And Approval | Owner's durable single-person dual-role MyRaidenII implementation goal and standing V30 CPU authority. Shared X86 Core source/tests and MyRaidenII records; NXVM eight receiving artifacts/status for changed executable inputs. Lib/Emulator, other shared mechanisms, private App source and MyNES production remain unauthorized. |
| Objective | Consume V12 complete 256-slot documented/reserved/NEC-escape disposition and implement ordinary 8080 register/data/ALU/DAA/rotate, direct/indirect memory, DS:BP stack, conditional/unconditional call/branch/return/RST, IN/OUT, EI/DI/HLT. Reuse existing result/flag, memory/port, mode/entry/return, halt and host-failure owners. Qualify actual BRKEM-to-8080-to-native routes, fixed/retained PSW fields and shared native standby semantics where the NEC source requires them. |
| Non-goals | No undocumented Z80/8085 aliases, complete physical prefetch/clocks or Machine admission, unrelated native semantic gaps, general BRK/priority/LOCK, board/COP/video/audio/game. Reserved slots are explicit unsupported diagnostics, not assumed NOPs. No new public API/enum/layout, persistent CPU state, worker, clock or second interpreter. |
| Reference Baseline | Clean 35c6242ad; pinned NEC October 1986 manual sections 8/9 and existing S23 mode frames. Original Intel 8080 Rev B 1975 and later original Intel 8080/8085 flag clarification; original NEC 8080AF catalogue/table when available. Current 673 units/186 X86 Core, 498 strict rows, PC 0564 and MyNES 0044 release identities. |
| Candidate Proposal | [V30 proposal](../proposals/m1-v30.md), [V12/V13/V14 ledger](../etc/t2-v30-ledger.md). Full native/NEC/8080/time/board universe is unchanged. Neither a farther ROM checkpoint nor one repaired instruction substitutes for this batch. |
| Files And ABI Surface | Existing cpu_instructions.c private mode-format/execute/ALU/bus/entry/halt owners and manifest. CPU-local ordinary emulation units/fixture as needed, existing mode/native admission tests updated only for newly implemented forms, registration/manifest. MyRaidenII evidence/history/ledger/Current and conditional NXVM releases/status. Estimate production +500..900/-30..80, tests +800..1600/-20..80, excluding manifests/records/artifacts. No public contract or new stored state. |
| Applicable Rules | Product guide, proposal/ledger, architecture/layout/source policy, Contributing; shared Execution/Architecture/Coding/Document, architecture-governance then coding-governance and read-only PDF skill. Existing receiving NXVM embedded-firmware artifact exception applies; read its authorities before release. One owner/executor/bus, inward dependencies, canonical Lib Types and one target per pushed P. |
| Verification | Original x64/x86 Release Ninja/ccache graphs -j8 and full repository units -j4 after focused actual-memory/port/flags/flow/interrupt/failure matrices. New finite ordinary unit gets admitted TIMEOUT 90; old budgets stay unchanged. Run 498 strict rows, Types/dependency/ownership/negative/manifests/governance. Rebuild/deploy eight current PC EXEs, relevant four-profile receiving integrations, restore default graphs and verify all ten identities; MyNES unchanged. |
| Expected Markers | V30:EMULATION:OK and finite coverage counts/table for all slots, every legal form and defined flag/result class, actual register mapping/DS:BP versus SS:SP, full CPU/retained high registers, code/data/stack/vector aliases/wrap/lanes, preview purity, port acceptance/completion and every observed host-transfer refusal. Existing mode marker is revised only when its admitted ordinary-slot inventory changes. |
| Asset Needs | Already pinned NEC manual plus read-only primary Intel 8080 Rev B/clarification and NEC AF instruction pages. Cache public manual payloads only under ignored research, record URL/edition/SHA and inspect relevant rendered pages; no third-party implementation, games/ROM/media download or import. No traces, new graph or user INI/media changes. |
| Reporting Requirements | Report complete semantic batch and estimated/actual diff, source conflicts/grades and reuse decisions before implementation. Retain exact slot/form inventory, independent oracle, focused/full proof, initialization/reset/interrupt routes and source/artifact lineage. Preserve full sixteen-field packet before pure governance acceptance. |
| Stop Conditions | Unresolved primary semantics or a new undocumented model, public interface/state expansion, external source/license/protected-input need, second decoder/executor/state owner or out-of-family repair requires coordinator review before affected implementation. Do not silently alter the universe or weaken tests/budgets. |
| Exit Criteria | Entire admitted documented ordinary table passes both widths with source-backed flags and exact register/memory/port/control effects; reserved/escape disposition is complete, actual mode/IRQ/standby/return routes and failure boundaries pass. Full units/gates and affected receiving releases verified, Ps pushed and actual-change review accepted. Timing/Machine/board/game remain explicit non-eligible receivers. |
| Original Owner Request | Continue implementing MyRaidenII under single-person coordinator/executor governance toward the full playable goal, not a smaller CPU-test objective. |
| Similar-Issue Sweep | Replace the existing control-only mode sizer/handler at its sole owner; inspect native ALU/flags, memory/port and HLT/EI/DI counterparts before reusing them. Inventory every raw/context lexical and mode call site and all early-Intel/NEC interrupt/standby paths; retain native versus 8080 differences only for documented register/layout/flag/entry semantics. No duplicate state, copied memory/port path or silently surviving control-only production entry. |

## Task Progress

| Task progress | Accepted result / remaining work |
| --- | --- |
| T2 S23 | Accepted/closed mode-transition baseline; S24 ordinary 8080 delivery is complete and awaiting coordinator actual-change review. |

[S24 evidence](../etc/evidence/t2-s24-emulation.md),
[results](../etc/evidence/t2-s24-results.json) and
[ten identities](../etc/evidence/t2-s24-artifacts.json) retain proof.
[T2 history](../history/M1-T2-v30.md) and the
[convergence ledger](../etc/t2-v30-ledger.md) preserve the full task domain.

S24 implements all 244 documented ordinary 8080 slots, retains the constrained
NEC escape and rejects eleven reserved slots. Register/ALU/DAA/rotate,
DS:BP stack, memory/port, absolute flow and EI/DI/HLT use the existing CPU
executor, ALU, bus, flag, entry/return and rollback owners. Execution/context
preview share one private format owner. No new public API, enum, layout,
stored CPU state, worker or clock; Lib/Emulator and App production are unchanged.

## Final Verification

Both original Release Ninja/ccache graphs build all targets. Full repository
unit suites pass 674/674 per width, including 187 X86 Core units, in
306.73/136.84 seconds. Ordinary matrices pass 4,328,948 inputs per width in
7.09176/8.18136 seconds; existing mode matrices retain 328,192 inputs.
Both specialized targets retain 498 strict rows. Types/dependency/ownership/
negative/manifests/governance/whitespace checks pass. Historical CPU warning
exceptions remain, not exemption-free strict qualification.

Eight PC 0564 EXEs are rebuilt/deployed; fourteen selected integrations plus
one restored-default identity check pass per width. DeskPro groups finish in
122.81/175.90 seconds within old budgets. Both original graphs are restored;
receiving loops compile zero CPU objects. All ten PE/name/width/size/hash
identities pass. MyNES 0044 and all user INIs/media/ROMs remain unchanged.

Production +394/-23/net +371 and tests/registration +653/-4/net +649 give
code +1,047/-27/net +1,020, excluding manifests/records/artifacts. Nine private
helpers own real mode-format/register/layout/flag/condition differences; old
control-only functions are replaced, not retained in parallel. Source-backed
AF/DAA/PSW differences and manual typo resolutions are indexed in evidence.
Self-review corrected word-operand lexical component counts; final complete
suites follow that correction. The earlier partial qualification is explicitly
superseded, not counted as final proof. All final handles have exited.

## Remaining T2 Boundary

Ordinary 8080 architecture is implemented and tested, not full V30 eligibility.
Packed-adjust/non-ten and CHKIND signedness source questions, general
priority/step/BRK, full prefixes/WAIT/LOCK, physical bus/prefetch/clocks and
neutral Machine/board/game remain original receivers. Native data-tail/BRK
sampling grades and V15 DE/BR retirement accounting remain explicit.
No fresh complete external/native integration suite, manual desktop/RDP/audio,
silicon-exact V30 timing or gameplay qualification is implied.
No runnable MyRaidenII EXE exists yet.

## Previous Work

T1 source/board/set and integration-contract freeze closed at `7f7d23938`;
[T1 history](../history/M1-T1-board-and-source-freeze.md) retains thirteen
runtime image identities and F01--F13, not game execution.
M0 Td S1 closed at `f21a22997`; M1 remains open toward the full playable goal.
Accepted NXVM T564 supplies RGB32/four-slot host publication; actual RAIDEN II
board palette/frame/rotation integration remains unqualified.

## Queue

[Queue](QUEUE.md) contains five unadmitted receivers after open V30 T2.
